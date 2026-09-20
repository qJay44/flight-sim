#pragma once

#include "mesh/Mesh.hpp"
#include "Shader.hpp"
#include "Light.hpp"
#include "texture/Texture.hpp"
#include "../core/EngineContext.hpp"

namespace gfx {

class Renderer {
public:
  struct RenderCommand {
    Shader* shader;
    const Mesh* mesh;
    const std::vector<Texture*> textures;
  };

  Renderer() = default;

  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = default;

  Renderer& operator=(const Renderer&) = delete;
  Renderer& operator=(Renderer&&) = default;

  ~Renderer() = default;

  void init(const core::EngineContext *ctx) const;

  void newFrame(ivec2 viewPort);

  void setGlobalLight(const Light* light);

  void submit(const RenderCommand&& cmd);
  void renderFrame();
private:

  std::list<RenderCommand> renderQueue;
  const Light* globalLight{};
};

} // namespace gfx

