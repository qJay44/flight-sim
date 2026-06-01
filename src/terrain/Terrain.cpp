#include "Terrain.hpp"

#include "../engine/mesh/Mesh.hpp"

Shader Terrain::shaderBufferA;
Shader Terrain::shaderBufferB;

Terrain::Terrain(ivec2 bufferSize) : bufferSize(bufferSize) {
  if (!shaderBufferA.initialized()) {
    shaderBufferA = Shader("terrain/bufferA.comp");
    shaderBufferB = Shader("terrain/bufferB.comp");
  }

  bufferA = Texture2D(bufferSize, TextureDescriptor{
    .internalFormat = GL_RGBA32F,
    .format = GL_RGBA,
    .minFilter = GL_LINEAR,
    .magFilter = GL_LINEAR,
    .wrapS = GL_REPEAT,
    .wrapT = GL_REPEAT,
  });

  bufferB = Texture2D(bufferSize, TextureDescriptor{
    .internalFormat = GL_RGBA32F, // Using alpha just make it to work with ImGui without extra handling
    .format = GL_RGBA,
    .minFilter = GL_LINEAR,
    .magFilter = GL_LINEAR,
    .wrapS = GL_REPEAT,
    .wrapT = GL_REPEAT,
  });

  constexpr uvec3 localSize{16, 16, 1};
  numWorkGroups = (uvec3(bufferSize, 1u) + localSize - 1u) / localSize;
}

void Terrain::update() {
  updateBufferA();
  updateBufferB();
}

void Terrain::draw(const Environment& env, const Camera* cam, Shader& shader) const {
  env.sun.setUniforms(shader);
  shader.setUniform2i("u_bufferSize", bufferSize);

  bufferA.bind(0);
  bufferB.bind(1);
  env.skybox.bind(2);
  Mesh::drawScreen(cam, shader);
}

void Terrain::updateBufferA() {
  shaderBufferA.use();
  glBindImageTexture(0, bufferA.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numWorkGroups.x, numWorkGroups.y, numWorkGroups.z);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Terrain::updateBufferB() {
  shaderBufferB.use();
  glBindImageTexture(0, bufferB.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numWorkGroups.x, numWorkGroups.y, numWorkGroups.z);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

