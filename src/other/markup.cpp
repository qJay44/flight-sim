#include "markup.hpp"

#include "global.hpp"
#include "../engine/mesh/meshes.hpp"
#include <memory>

namespace {
  std::unique_ptr<MeshArrays> lineH = nullptr;
  std::unique_ptr<MeshArrays> lineV = nullptr;
}

namespace markup {
  void init() {
    vec2 winSize = global::getWinSize();
    vec2 winCenter = winSize * 0.5f;
    lineH = std::make_unique<MeshArrays>(meshes::line({0.f, winCenter.y, 0.f}, {winSize.x, winCenter.y, 0.f}));
    lineV = std::make_unique<MeshArrays>(meshes::line({winCenter.x, 0.f, 0.f}, {winCenter.x, winSize.y, 0.f}));
  }

  void onResize() {
    vec2 winSize = global::getWinSize();
    vec2 winCenter = winSize * 0.5f;

    vertex::P vertices[] = {{{0.f, winCenter.y, 0.f}}, {{winSize.x, winCenter.y, 0.f}}};

    MeshData data;
    data.vertices = (float*)vertices;
    data.verticesSize = sizeof(vertices);
    data.layout = vertices[0].getLayout();
    data.mode = GL_LINES;

    lineH->updateBufferVBO(data);

    vertices[0] = {{winCenter.x, 0.f, 0.f}};
    vertices[1] = {{winCenter.x, winSize.y, 0.f}};

    lineV->updateBufferVBO(data);
  }

  void drawCross(const Camera* cam, Shader& shader) {
    shader.setUniformMatrix4f("u_proj", global::getScreenProjection());
    shader.setUniform3f("u_color", crossColor);

    lineH->draw(cam, shader);
    lineV->draw(cam, shader);
  }
}

