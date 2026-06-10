#pragma once

#include "../engine/texture/Texture2DArray.hpp"
#include "../engine/Shader.hpp"

namespace terrain {

class TextureManager {
public:
  TextureManager() = default;
  TextureManager(GLsizei slots, ivec2 size);

  void generate(vec2 offset, int slot);
  void bind() const;

private:
  static Shader shaderBufferA;
  static Shader shaderBufferB;

  Texture2DArray bufferA; // heightmap (X), normals (YZ) and erosion mask (W)
  Texture2DArray bufferB; // Noise to add diffuse/normal detail
  uvec3 numWorkGroups;

private:
  void updateBufferA(vec2 offset, int slot);
  void updateBufferB(vec2 offset, int slot);
};

} // terrain

