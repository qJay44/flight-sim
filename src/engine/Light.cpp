#include "Light.hpp"

#include "mesh/meshes.hpp"
#include "global.hpp"

Light::Light(vec3 position, float radius, vec3 color)
  : Mesh(meshes::plane(2, GL_TRIANGLES, global::forward)), position(position), radius(radius), color(color) {}

const vec3& Light::getPosition() const { return position; }
const vec3& Light::getColor() const { return color; }

void Light::update() {
  setTranslationMat(position);
}

void Light::setUniforms(Shader& shader) const {
  shader.setUniform1f("u_lightRadius", radius);
  shader.setUniform3f("u_lightPos", position);
  shader.setUniform3f("u_lightColor", color);
}

void Light::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  setUniforms(shader);

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Mesh::draw(camera, shader, forceNoWireframe);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

