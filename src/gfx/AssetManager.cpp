#include "AssetManager.hpp"

#include "tiny_obj_loader.h"
#include "../gfx/mesh/vertex.hpp"
#include "utils/clrp.hpp"
#include "utils/utils.hpp"

namespace gfx {

namespace {

// A custom hasher for the tinyobj index struct
struct IndexHasher {
  size_t operator() (const struct tinyobj::index_t& i) const {
    size_t h = 0;
    // Standard hash combine logic
    auto hash_combine = [](size_t& seed, int v) {
      seed ^= std::hash<int>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };

    hash_combine(h, i.vertex_index);
    hash_combine(h, i.normal_index);
    hash_combine(h, i.texcoord_index);
    return h;
  }
};

// Equality check for the map
struct IndexEqual {
  bool operator() (const tinyobj::index_t& a, const tinyobj::index_t& b) const {
    return a.vertex_index == b.vertex_index &&
           a.normal_index == b.normal_index &&
           a.texcoord_index == b.texcoord_index;
  }
};

} // namespace

AssetManager::AssetManager() {
  {
    std::vector<vertex::P> vertices = {
      {{0.f, 0.f, 0.f}}, {{1.f, 0.f, 0.f}},
      {{0.f, 0.f, 0.f}}, {{0.f, 1.f, 0.f}},
      {{0.f, 0.f, 0.f}}, {{0.f, 0.f, 1.f}},
    };

    MeshData data(vertices);
    data.mode = GL_LINES;

    meshes.emplace("Axis", std::make_unique<Mesh>(Mesh(data)));
  }
}

std::string AssetManager::createMeshPlane_Triangles(size_t resolution, bool skirts, bool instanced) {
  std::string skirtsSuffix = skirts ? "_Skirts" : "";
  std::string instancedSuffix = instanced ? "_Instancied" : "";
  std::string name = std::format("MeshPlane_Triangles{}{}{}", resolution, skirts, instancedSuffix);

  if (meshes.contains(name)) {
    warning("[AssetManager::createMeshPlane_Triangles] Mesh ({}) already created", name);
    return name;
  }

  size_t res1 = resolution - 1;
  float invRes1 = 1.f / float(res1);

  size_t baseVertexCount = resolution * resolution;
  size_t skirtVertexCount = skirts ? 4 * 2 * res1 : 0;
  std::vector<vertex::P> vertices(baseVertexCount + skirtVertexCount);

  size_t baseIndexCount = res1 * res1 * 6;
  size_t skirtIndexCount = skirts ? res1 * 4 * 6 : 0;
  std::vector<GLuint> indices(baseIndexCount + skirtIndexCount);
  size_t triIndex = 0;

  for (size_t z = 0; z < resolution; z++) {
    float v = z * invRes1;

    for (size_t x = 0; x < resolution; x++) {
      size_t idx = x + z * resolution;
      float u = x * invRes1;

      vertices[idx].position = vec3(u, 0.f, v) * 2.f - 1.f;

      if (x != res1 && z != res1) {
        indices[triIndex + 0] = idx + resolution + 1;  // 0       2 -------- 1
        indices[triIndex + 1] = idx + 1;               // 1       |          |
        indices[triIndex + 2] = idx;                   // 2       |          |
                                                       //    CCW  |          |
        indices[triIndex + 3] = idx;                   // 2       |          |
        indices[triIndex + 4] = idx + resolution;      // 3       |          |
        indices[triIndex + 5] = idx + resolution + 1;  // 0       3 -------- 0

        triIndex += 6;
      }
    }
  }

  if (skirts) {
    size_t skirtVertexIdx = baseVertexCount;

    const auto addSkirt = [&](size_t origIdxA, size_t origIdxB) {
      vertices[skirtVertexIdx] = vertices[origIdxA];
      vertices[skirtVertexIdx + 1] = vertices[origIdxB];

      indices[triIndex + 0] = origIdxB;
      indices[triIndex + 1] = skirtVertexIdx + 1;
      indices[triIndex + 2] = skirtVertexIdx;

      indices[triIndex + 3] = skirtVertexIdx;
      indices[triIndex + 4] = origIdxA;
      indices[triIndex + 5] = origIdxB;

      triIndex += 6;
      skirtVertexIdx += 2;
    };

    for (size_t i = 0; i < res1; i++) {
      addSkirt(i, i + 1);
      addSkirt(res1 * resolution + i + 1, res1 * resolution + i);
      addSkirt((i + 1) * resolution, i * resolution);
      addSkirt(i * resolution + res1, (i + 1) * resolution + res1);
    }
  }

  auto data = MeshData(vertices, indices);
  data.instanced = instanced;

  meshes.emplace(name, std::make_unique<Mesh>(Mesh(data)));
  return name;
}

std::string AssetManager::createMeshPlane_Patches(size_t resolution, bool instanced) {
  std::string instancedSuffix = instanced ? "_Instancied" : "";
  std::string name = std::format("MeshPlane_Patches{}{}", resolution, instancedSuffix);

  if (meshes.contains(name)) {
    warning("[AssetManager::createMeshPlane_Patches] Mesh ({}) already created", name);
    return name;
  }

  std::vector<vertex::P> vertices;
  std::vector<GLuint> indices;
  size_t triIndex = 0;

  vertices.resize(resolution * resolution);
  indices.resize((resolution - 1) * (resolution - 1) * 4);

  float invRes1 = 1.f / (resolution - 1.f);

  for (size_t z = 0; z < resolution; z++) {
    float v = z * invRes1;

    for (size_t x = 0; x < resolution; x++) {
      size_t idx = x + z * resolution;
      float u = x * invRes1;

      vertices[idx].position = vec3(u, 0.f, v) * 2.f - 1.f;

      if (x != resolution - 1 && z != resolution - 1) {
        indices[triIndex + 0] = idx + resolution + 1; // 0
        indices[triIndex + 1] = idx + 1;              // 1
        indices[triIndex + 2] = idx;                  // 2
        indices[triIndex + 3] = idx + resolution;     // 3

        triIndex += 4;
      }
    }
  }

  MeshData data(vertices, indices);
  data.mode = GL_PATCHES;
  data.instanced = instanced;

  meshes.emplace(name, std::make_unique<Mesh>(Mesh(data)));
  return name;
}

void AssetManager::loadFromObj(fspath filepath, bool printInfo) {
  std::string filename = filepath.filename().string();

  if (meshes.contains(filename)) {
    warning("[AssetManager::loadFromObj] Mesh ({}) already loaded", filename);
    return;
  }

  tinyobj::ObjReaderConfig readerConfig;
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(filepath.string(), readerConfig)) {
    std::string msg = "[AssetManager::loadFromObj] ParseFromFile error";
    if (!reader.Error().empty())
      msg = "TinyObjReader: " + reader.Error();

    error(msg);
  }

  if (!reader.Warning().empty())
    warning(std::format("TinyObjReader: {}", reader.Warning()));

  const tinyobj::attrib_t& attrib = reader.GetAttrib();
  const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
  // const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

  std::vector<vertex::PTNC> vertices;
  std::vector<GLuint> indices;
  std::unordered_map<tinyobj::index_t, uint32_t, IndexHasher, IndexEqual> uniqueVertices;

  for (const auto& shape : shapes) {
    for (const auto& idx : shape.mesh.indices) {
      auto [it, inserted] = uniqueVertices.emplace(idx, vertices.size());

      if (inserted) {
        vertex::PTNC vertex;

        vertex.position = {
          attrib.vertices[3 * idx.vertex_index + 0],
          attrib.vertices[3 * idx.vertex_index + 1],
          attrib.vertices[3 * idx.vertex_index + 2]
        };

        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
        if (idx.texcoord_index >= 0) {
          vertex.texture = {
            attrib.texcoords[2 * idx.texcoord_index + 0],
            attrib.texcoords[2 * idx.texcoord_index + 1]
          };
        }

        // Check if `normal_index` is zero or positive. negative = no normal data
        if (idx.normal_index >= 0) {
          vertex.normal = {
            attrib.normals[3 * idx.normal_index + 0],
            attrib.normals[3 * idx.normal_index + 1],
            attrib.normals[3 * idx.normal_index + 2]
          };
        }

        // Optional: vertex colors
        vertex.color = {
          attrib.colors[3 * idx.vertex_index + 0],
          attrib.colors[3 * idx.vertex_index + 1],
          attrib.colors[3 * idx.vertex_index + 2]
        };

        vertices.push_back(vertex);
      }

      indices.push_back(it->second);
    }
  }

  // ============ Print info ============ //

  if (printInfo) {
    clrp::clrp_t cfmt{
      .attr = clrp::ATTRIBUTE::BOLD,
      .fg = clrp::FG::CYAN
    };
    std::string cname = clrp::format(std::format("[{}]", filepath.string()), cfmt);
    std::string infoLoad = std::format("[load]\nvertices: {}\ncolors:   {}\ntextures: {}\nnormals:  {}", attrib.vertices.size() / 3, attrib.colors.size() / 3, attrib.texcoords.size() / 2, attrib.normals.size() / 3);
    std::string infoFinal = std::format("[final]\nvertices: {}, indices: {}\n", vertices.size(), indices.size());
    printf("\n==================== %s ====================\n\n%s\n\n%s\n\n", cname.c_str(), infoLoad.c_str(), infoFinal.c_str());

    std::string end = "============================================";
    for (u32 i = 0; i < filepath.string().size(); i++)
      end += "=";
    end += "\n";

    puts(end.c_str());
  }

  // ==================================== //

  meshes.emplace(filename, std::make_unique<Mesh>(Mesh(MeshData(vertices, indices))));
}

void AssetManager::addShader(const std::string& name, Shader&& shader) {
  if (shaders.contains(name)) {
    warning("[AssetManager::createShader] Shader ({}) already created", name);
    return;
  }

  shaders.emplace(name, std::make_unique<Shader>(std::move(shader)));
}

void AssetManager::addTexture(const std::string& name, Texture&& texture) {
  if (textures.contains(name)) {
    warning("[AssetManager::addTexture] Texture ({}) already created", name);
    return;
  }

  textures.emplace(name, std::make_unique<Texture>(std::move(texture)));
}

void AssetManager::addCamera(const std::string& name, core::Camera&& camera) {
  if (cameras.contains(name)) {
    warning("[AssetManager::addCamera] Camera ({}) already created", name);
    return;
  }

  cameras.emplace(name, std::make_unique<core::Camera>(std::move(camera)));
}

void AssetManager::addLight(const std::string& name, Light&& light) {
  if (lights.contains(name)) {
    warning("[AssetManager::addLight] Light ({}) already created", name);
    return;
  }

  lights.emplace(name, std::make_unique<Light>(std::move(light)));
}

void AssetManager::checkShaders() {
  for (auto& [name, shader] : shaders)
    if (shader->needsReload())
      shader->reload();
}

Mesh* AssetManager::getMesh(const std::string& name) const {
  assert(contains(meshes, name));
  return meshes.at(name).get();
}

Shader* AssetManager::getShader(const std::string& name) const {
  assert(contains(shaders, name));
  return shaders.at(name).get();
}

Texture* AssetManager::getTexture(const std::string& name) const {
  assert(contains(textures, name));
  return textures.at(name).get();
}

core::Camera* AssetManager::getCamera(const std::string& name) const {
  assert(contains(cameras, name));
  return cameras.at(name).get();
}

Light* AssetManager::getLight(const std::string& name) const {
  assert(contains(lights, name));
  return lights.at(name).get();
}

bool AssetManager::contains(const auto& map, const std::string& name) {
  auto it = map.find(name);
  if (it == map.end())
    error("[AssetManager::getMesh] Didn't find ({})", name);

  return true;
}

} // namespace AssetManager

