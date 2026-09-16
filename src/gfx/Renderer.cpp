#include "Renderer.hpp"

#include "../core/EngineContext.hpp"
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
}

} // namespace

void Renderer::init(const core::EngineContext* ctx) const {
  ivec2 winSize = ctx->getWinSize();

  glViewport(0, 0, winSize.x, winSize.y);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
}

void Renderer::beginFrame(ivec2 viewPort) {
  renderQueue.clear();
  glViewport(0, 0, viewPort.x, viewPort.y);
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setProjectionMat(const mat4& proj) { renderProj  = proj; }
void Renderer::setViewMat(const mat4& view)       { renderView  = view; }
void Renderer::setGlobalLight(const Light* light) { globalLight = light; }

void Renderer::submit(const RenderCommand&& cmd) {
  renderQueue.push_back(std::move(cmd));
}

void Renderer::endFrame(const core::EngineContext& ctx) {
  renderQueue.sort([](const RenderCommand& a, const RenderCommand& b) {
    return a.shader < b.shader;
  });

  assert(globalLight);

  Shader* currBoundShader = nullptr;
  const Mesh* currBoundMesh = nullptr;
  const core::Camera* currBoundCamera = nullptr;
  const Texture* currBoundTextures[MAX_TEXTURES]{};

  for (const auto& command : renderQueue) {
    if (command.shader != currBoundShader) {
      currBoundShader = command.shader;
      currBoundShader->use();
      currBoundShader->setUniformMatrix4f("u_proj", renderProj);
      currBoundShader->setUniformMatrix4f("u_view", renderView);
      currBoundShader->setUniform3f("u_lightColor", globalLight->color);
      currBoundShader->setUniform3f("u_lightDir", globalLight->direction);
      currBoundShader->setUniform1f("u_time", ctx.time);
      currBoundShader->setUniform1f("u_lightAmbient", globalLight->ambient);
      currBoundShader->setUniform1f("u_lightSpecular", globalLight->specular);
    }

    if (command.mesh != currBoundMesh) {
      currBoundMesh = command.mesh;
      currBoundMesh->vao.bind();
      glPolygonMode(GL_FRONT_AND_BACK, currBoundMesh->polygonMode);
      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
    }

    if (command.cam != currBoundCamera) {
      currBoundCamera = command.cam;
      currBoundShader->setUniform1f("u_camNear", currBoundCamera->nearPlane);
      currBoundShader->setUniform1f("u_camFar", currBoundCamera->farPlane);
      currBoundShader->setUniform1f("u_camFov", currBoundCamera->fov);
      currBoundShader->setUniform3f("u_camUp", currBoundCamera->up);
      currBoundShader->setUniform3f("u_camForward", currBoundCamera->orientation);
      currBoundShader->setUniform3f("u_camPos", command.camPos);
    }

    for (size_t i = 0; i < command.textures.size() && i < MAX_TEXTURES; i++) {
      const Texture*& currTex = currBoundTextures[i];
      const Texture* cmdTex = command.textures[i];

      if (currTex != cmdTex) {
        currTex = cmdTex;
        currTex->bind(i);
      }
    }

    currBoundShader->setUniformMatrix4f("u_model", command.model);

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
}

} // namespace gfx

