#pragma once

#include "FBO.hpp"
#include "mesh/Mesh.hpp"
#include "Shader.hpp"
#include "Light.hpp"
#include "texture/ImageDescriptor.hpp"
#include "texture/Texture.hpp"

namespace gfx {

class Renderer {
public:
  struct ClearCommand {
    FBO* fbo = nullptr;
    ivec2 viewPort;
    vec4 clearColor{0.f, 0.f, 0.f, 1.f};
    GLbitfield clearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
  };

  struct RenderCommand {
    Shader* shader;
    Mesh* mesh;
    bool enableCullFace = true;
    bool enableDepthTest = true;
    bool enableBlend = false;
    bool enableDepthMask = true;
    std::vector<Texture*> textures{};
    int priority = 0;
    std::pair<GLenum, GLenum> blendFunc{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
  };

  struct ComputeCommand {
    Shader* shader;
    uvec3 numWorkGroups;
    std::vector<ImageDescriptor> images;
    std::vector<Texture*> textures{};
  };

  Renderer() = default;

  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = default;

  Renderer& operator=(const Renderer&) = delete;
  Renderer& operator=(Renderer&&) = default;

  ~Renderer() = default;

  void init() const;
  void memoryBarrier(GLbitfield barriers) const;

  void clearFrame(const ClearCommand& cmd);

  void setGlobalLight(const Light* light);
  void setDrawBuffers(const std::vector<GLenum>& drawBuffers);

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

