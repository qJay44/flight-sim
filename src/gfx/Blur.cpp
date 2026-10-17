#include "Blur.hpp"

#include "Renderer.hpp"

namespace gfx {

Shader* Blur::shaderExtract{};
Shader* Blur::shaderBlur{};
Mesh* Blur::meshScreen{};

TextureDescriptor Blur::getDefaultBlurDescriptor() {
  static TextureDescriptor desc{
    .internalFormat = GL_RGB16F,
    .format = GL_RGB,
    .type = GL_HALF_FLOAT,
    .minFilter = GL_LINEAR,
    .magFilter = GL_LINEAR,
  };

  return desc;
}

Blur::Blur(ivec2 resolution, const TextureDescriptor& blurDesc)
  : resolution(resolution), uvOffset(1.f / vec2(resolution))
{
  texBlurVertical.initStorage(resolution, blurDesc);
  texBlurHorizontal.initStorage(resolution, blurDesc);

  fbo.gen();
  fbo.attach2D(GL_COLOR_ATTACHMENT0, texBlurVertical);
  fbo.attach2D(GL_COLOR_ATTACHMENT1, texBlurHorizontal);
}

Texture2D* Blur::blur(Renderer& renderer, Texture2D* texHDR, int amount) {
  Renderer::ClearCommand cmdClear{
    .fbo = &fbo,
    .viewPort = resolution,
    .clearMask = GL_COLOR_BUFFER_BIT
  };

  Renderer::RenderCommand cmdRenderExtract{
    .shader = shaderExtract,
    .mesh = meshScreen,
    .enableCullFace = false,
    .enableDepthTest = false,
    .textures = {texHDR},
    .drawBuffers = {GL_COLOR_ATTACHMENT1},
  };

  renderer.clearFrame(cmdClear);
  renderer.submit(cmdRenderExtract);
  renderer.renderFrame();

  Renderer::RenderCommand cmdRenderBlurV{
    .shader = shaderBlur,
    .mesh = meshScreen,
    .enableCullFace = false,
    .enableDepthTest = false,
    .textures = {&texBlurHorizontal},
    .drawBuffers = {GL_COLOR_ATTACHMENT0},
  };

  Renderer::RenderCommand cmdRenderBlurH{
    .shader = shaderBlur,
    .mesh = meshScreen,
    .enableCullFace = false,
    .enableDepthTest = false,
    .textures = {&texBlurVertical},
    .drawBuffers = {GL_COLOR_ATTACHMENT1},
  };

  amount += amount & 1;

  for (int i = 0; i < amount; i++) {
    shaderBlur->setUniform2f("u_uvOffset", vec2(0.f, uvOffset.y));
    renderer.submit(cmdRenderBlurV);
    renderer.renderFrame();

    shaderBlur->setUniform2f("u_uvOffset", vec2(uvOffset.x, 0.f));
    renderer.submit(cmdRenderBlurH);
    renderer.renderFrame();
  }

  return &texBlurHorizontal;
}

} // namespace gfx

