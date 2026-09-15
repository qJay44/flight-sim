#pragma once

#include "mesh/Mesh.hpp"
#include "Shader.hpp"
#include "texture/Texture.hpp"
#include "../core/EngineContext.hpp"
#include "../core/Camera.hpp"
#include "../core/Light.hpp"

namespace gfx {

class Renderer {
public:
  struct RenderCommand {
    Shader* shader;
    const Mesh* mesh;
    const core::Camera* cam;
    const vec3& camPos;
    const mat4& model;
    std::vector<Texture*> textures;
  };

  Renderer() = default;

  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = default;

  Renderer& operator=(const Renderer&) = delete;
  Renderer& operator=(Renderer&&) = default;

  ~Renderer() = default;

  void init(const core::EngineContext *ctx) const;

  void beginFrame(ivec2 viewPort);

  void setProjectionMat(const mat4& proj);
  void setViewMat(const mat4& view);
  void setGlobalLight(const core::Light* light);

  void submit(const RenderCommand&& cmd);
  void endFrame(const core::EngineContext& ctx);
private:

  std::list<RenderCommand> renderQueue;
  mat4 renderProj;
  mat4 renderView;
  const core::Light* globalLight{};
};

} // namespace gfx

