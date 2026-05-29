#pragma once

#include "engine/Camera.hpp"
#include "engine/Shader.hpp"
#include "engine/texture/TextureCubemap.hpp"
#include "other/Sun.hpp"

struct Environment {
  Sun sun{};
  TextureCubemap skybox;

  static Environment createDefault(const char* skyboxImagePath) {
    Environment env;
    env.sun.pitch = glm::radians(20.f);
    env.sun.updateDir();

    env.skybox = TextureCubemap::loadFromImage(skyboxImagePath, {GL_TEXTURE_CUBE_MAP});

    return env;
  }

  void draw(const Camera* cam, Shader& shader) const {
    mat4 viewRot = mat4(mat3(cam->getView())); // No translation part;

    shader.setUniformMatrix4f("u_proj", cam->getProj());
    shader.setUniformMatrix4f("u_viewRot", viewRot);
    skybox.bind(0);

    Mesh::drawScreen(cam, shader);
  }
};

