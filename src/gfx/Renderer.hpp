#pragma once

#include "mesh/Mesh.hpp"
#include "Shader.hpp"
#include "Light.hpp"
#include "texture/ImageDescriptor.hpp"
#include "texture/Texture.hpp"
#include "../core/EngineContext.hpp"

namespace gfx {

class Renderer {
public:
  struct RenderCommand {
    Shader* shader;
    Mesh* mesh;
    std::vector<Texture*> textures;
  };

  struct ComputeCommand {
    Shader* shader;
    uvec3 numWorkGroups;
    std::vector<ImageDescriptor> images;
    std::vector<Texture*> textures;
  };

  Renderer() = default;

  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = default;

  Renderer& operator=(const Renderer&) = delete;
  Renderer& operator=(Renderer&&) = default;

  ~Renderer() = default;

  void init(const core::EngineContext *ctx) const;
  void memoryBarrier(GLbitfield barriers) const;

  void newFrame(ivec2 viewPort);

  void setGlobalLight(const Light* light);

  void submit(RenderCommand cmd);
  void submit(ComputeCommand cmd);

  void renderFrame();
  void dispatch();

private:
  std::list<RenderCommand> renderQueue;
  std::list<ComputeCommand> computeQueue;
  const Light* globalLight{};
};

} // namespace gfx

