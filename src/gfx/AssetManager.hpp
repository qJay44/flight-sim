#pragma once

#include "mesh/Mesh.hpp"
#include "Shader.hpp"
#include "texture/Texture.hpp"
#include "../core/Camera.hpp"
#include "../gfx/Light.hpp"

namespace gfx {

class AssetManager {
public:
  AssetManager();

  AssetManager(const AssetManager&) = delete;
  AssetManager(AssetManager&&) = default;

  AssetManager& operator=(const AssetManager&) = delete;
  AssetManager& operator=(AssetManager&&) = default;

  ~AssetManager() = default;

  std::string createMeshPlane_Triangles(size_t resolution, bool skirts = false, bool instanced = false);
  std::string createMeshPlane_Patches(size_t resolution, bool instanced = false);

  void loadFromObj(fspath filepath, bool printInfo = false);
  void addShader(const std::string& name, Shader&& shader);
  void addTexture(const std::string& name, Texture&& texture);
  void addCamera(const std::string& name, core::Camera&& camera);
  void addLight(const std::string& name, Light&& light);

  void checkShaders();

  Mesh* getMesh(const std::string& name) const;
  Shader* getShader(const std::string& name) const;
  Texture* getTexture(const std::string& name) const;
  core::Camera* getCamera(const std::string& name) const;
  Light* getLight(const std::string& name) const;

private:
  std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
  std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
  std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
  std::unordered_map<std::string, std::unique_ptr<core::Camera>> cameras;
  std::unordered_map<std::string, std::unique_ptr<Light>> lights;

private:
  static bool contains(const auto& map, const std::string& name);
};

};

