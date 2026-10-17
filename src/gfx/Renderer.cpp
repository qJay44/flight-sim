#include "Renderer.hpp"

#include "Blur.hpp"
#include "utils/clrp.hpp"

namespace gfx {

namespace {

constexpr size_t MAX_TEXTURES = GL_TEXTURE30 - GL_TEXTURE0 + 1;

void GLAPIENTRY MessageCallback(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam
) {
  static const clrp::clrp_t clrpError{clrp::ATTRIBUTE::BOLD, clrp::FG::RED};
  static const clrp::clrp_t clrpWarning{clrp::ATTRIBUTE::BOLD, clrp::FG::YELLOW};

  clrp::clrp_t clrpFinal = clrpError;
  bool stop = true;

  switch (source) {
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      return; // Handled by the Shader class itself
  }

  // Suppress annoying SIMD32 callback
  if (type == GL_DEBUG_TYPE_PERFORMANCE) {
    clrpFinal = clrpWarning;
    stop = false;
  }

  fprintf(
    stderr, "GL CALLBACK: %s source = 0x%x, id = 0x%x type = 0x%x, severity = 0x%x, message = %s\n",
    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), source, id, type, severity, clrp::format(message, clrpFinal).c_str()
  );

  if (stop)
    exit(EXIT_FAILURE);

  (void)length;
  (void)userParam;
}

} // namespace

void Renderer::init(AssetManager& assetManager) const {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  assetManager.addShader("BlurExtract", Shader("screen.vert", "blur/extract.frag"));
  assetManager.addShader("BlurBlur", Shader("screen.vert", "blur/blur.frag"));

  Blur::shaderExtract = assetManager.getShader("BlurExtract");
  Blur::shaderBlur = assetManager.getShader("BlurBlur");
  Blur::meshScreen = assetManager.getMesh("ScreenRect");
}

void Renderer::memoryBarrier(GLbitfield barriers) const {
  glMemoryBarrier(barriers);
}

void Renderer::clearFrame(const ClearCommand& cmd) {
  renderQueue.clear();
  computeQueue.clear();

  if (cmd.fbo)
      cmd.fbo->bind();
  else
    FBO::unbind();

  glViewport(0, 0, cmd.viewPort.x, cmd.viewPort.y);
  glClearColor(cmd.clearColor.r, cmd.clearColor.g, cmd.clearColor.b, cmd.clearColor.a);
  glClear(cmd.clearMask);
}

void Renderer::setGlobalLight(const Light* light) { globalLight = light; }

void Renderer::submit(RenderCommand cmd) {
  renderQueue.push_back(std::move(cmd));
}

void Renderer::submit(ComputeCommand cmd) {
  computeQueue.push_back(std::move(cmd));
}

void Renderer::renderFrame() {
  renderQueue.sort([](const RenderCommand& a, const RenderCommand& b) {
    return a.priority == b.priority ? a.shader < b.shader : a.priority < b.priority;
  });

  assert(globalLight);

  Shader* currBoundShader = nullptr;
  const Mesh* currBoundMesh = nullptr;
  const Texture* currBoundTextures[MAX_TEXTURES]{};

  for (const auto& command : renderQueue) {
    if (command.shader != currBoundShader) {
      currBoundShader = command.shader;
      currBoundShader->use();
      currBoundShader->setUniform3f("u_lightColor", globalLight->color);
      currBoundShader->setUniform3f("u_lightDir", globalLight->direction);
      currBoundShader->setUniform1f("u_lightAmbient", globalLight->ambient);
      currBoundShader->setUniform1f("u_lightSpecular", globalLight->specular);
      currBoundShader->setUniform1f("u_lightIntensity", globalLight->intensity);
      currBoundShader->setUniform1f("u_lightFocus", globalLight->focus);

      if (!command.drawBuffers.empty())
        glDrawBuffers(command.drawBuffers.size(), command.drawBuffers.data());
    }

    if (command.mesh != currBoundMesh) {
      currBoundMesh = command.mesh;
      currBoundMesh->vao.bind();
      glPolygonMode(GL_FRONT_AND_BACK, currBoundMesh->polygonMode);

      if (command.enableCullFace) glEnable (GL_CULL_FACE);
      else                        glDisable(GL_CULL_FACE);

      if (command.enableDepthTest) glEnable (GL_DEPTH_TEST);
      else                         glDisable(GL_DEPTH_TEST);

      if (command.enableDepthMask) glDepthMask(GL_TRUE);
      else                         glDepthMask(GL_FALSE);

      if (command.enableBlend) {
        glEnable(GL_BLEND);
        glBlendFunc(command.blendFunc.first, command.blendFunc.second);
      } else {
        glDisable(GL_BLEND);
      }
    }

    for (size_t i = 0; i < command.textures.size() && i < MAX_TEXTURES; i++) {
      const Texture*& currTex = currBoundTextures[i];
      const Texture* cmdTex = command.textures[i];

      if (currTex != cmdTex) {
        currTex = cmdTex;
        currTex->bind(i);
      }
    }

    std::visit([](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, ArraysDraw>) {
        glDrawArrays(arg.mode, 0, arg.vertexCount);
      } else if constexpr (std::is_same_v<T, ElementsDraw>) {
        glDrawElements(arg.mode, arg.indexCount, arg.indexType, arg.indicesOffset);
      } else if constexpr (std::is_same_v<T, ElementsInstancedDraw>) {
        glDrawElementsInstanced(arg.mode, arg.indexCount, arg.indexType, arg.indicesOffset, arg.instanceCount);
      }
    }, currBoundMesh->drawCmd);
  }

  renderQueue.clear();
}

void Renderer::dispatch() {
  computeQueue.sort([](const ComputeCommand& a, const ComputeCommand& b) {
    return a.shader < b.shader;
  });

  Shader* currBoundShader = nullptr;
  const Texture* currBoundImages[MAX_TEXTURES]{};
  const Texture* currBoundTextures[MAX_TEXTURES]{};

  for (const auto& command : computeQueue) {
    if (command.shader != currBoundShader) {
      currBoundShader = command.shader;
      currBoundShader->use();
    }

    for (size_t i = 0; i < command.images.size() && i < MAX_TEXTURES; i++) {
      const ImageDescriptor& imgDesc = command.images[i];
      const Texture*& currImg = currBoundImages[i];
      const Texture* cmdImg = imgDesc.texture;

      if (currImg != cmdImg) {
        currImg = cmdImg;
        currBoundShader->setUniform1i("u_layer", imgDesc.layer);
        glBindImageTexture(i, currImg->getId(), imgDesc.level, imgDesc.layererd, imgDesc.layer, imgDesc.access, imgDesc.format);
      }
    }

    for (size_t i = 0; i < command.textures.size() && i < MAX_TEXTURES; i++) {
      const Texture*& currTex = currBoundTextures[i];
      const Texture* cmdTex = command.textures[i];

      if (currTex != cmdTex) {
        currTex = cmdTex;
        currTex->bind(i);
      }
    }

    glDispatchCompute(command.numWorkGroups.x, command.numWorkGroups.y, command.numWorkGroups.z);
  }

  computeQueue.clear();
}

} // namespace gfx

