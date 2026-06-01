#pragma once

#include "../engine/texture/Texture2D.hpp"
#include "../engine/Camera.hpp"
#include "../Environment.hpp"

class Terrain {
public:
  Terrain(ivec2 bufferSize);

  void update();
  void draw(const Environment& env, const Camera* cam, Shader& shader) const;

private:
  friend struct gui;

  static Shader shaderBufferA;
  static Shader shaderBufferB;

  ivec2 bufferSize;

  Texture2D bufferA; // heightmap (X), normals (YZ) and erosion mask (W)
  Texture2D bufferB; // Noise to add diffuse/normal detail
  uvec3 numWorkGroups;

private:
  void updateBufferA();
  void updateBufferB();
};

