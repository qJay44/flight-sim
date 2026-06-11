#pragma once

#include "../engine/texture/Texture2DArray.hpp"
#include "../engine/Shader.hpp"

struct gui;

namespace terrain {

class GenerationManager {
public:
  GenerationManager() = default;
  GenerationManager(GLsizei slots, ivec2 size);

  const float& getHeightmapScale() const;

  void setHeightmapScale(float s);
  void generate(vec2 offset, int slot);
  void bindTextures() const;

private:
  friend ::gui;

  static Shader shaderBufferA;
  static Shader shaderBufferB;

  Texture2DArray bufferA; // heightmap (X), normals (YZ) and erosion mask (W)
  Texture2DArray bufferB; // Noise to add diffuse/normal detail
  uvec3 numWorkGroups;
  float heightmapScale = 1.f;

private:
  void updateBufferA(vec2 offset, int slot);
  void updateBufferB(vec2 offset, int slot);
};

} // terrain

