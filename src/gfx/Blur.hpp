#pragma once

#include "FBO.hpp"
#include "texture/Texture2D.hpp"
#include "Shader.hpp"
#include "mesh/Mesh.hpp"

namespace gfx {

class Blur {
public:
  static TextureDescriptor getDefaultBlurDescriptor();

  Blur(ivec2 resolution, const TextureDescriptor& blurDesc = getDefaultBlurDescriptor());

  [[nodiscard]]
  Texture2D* blur(class Renderer& renderer, Texture2D* texHDR, int amount = 4);

private:
  friend class Renderer;

  static Shader* shaderExtract;
  static Shader* shaderBlur;
  static Mesh* meshScreen;

  ivec2 resolution;
  vec2 uvOffset;
  FBO fbo;
  Texture2D texBlurVertical;
  Texture2D texBlurHorizontal;
};

} // namespace gfx

