#include "MeshElements.hpp"

#include "Mesh.hpp"
#include "MeshData.hpp"
#include "vertex.hpp"
#include "utils/status.hpp"
#include "utils/clrp.hpp"
#include "utils/utils.hpp"
#include "tiny_obj_loader.h"
#include <cassert>

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

MeshElements MeshElements::loadFromOBJ(const fspath& file, bool printInfo)  {
  tinyobj::ObjReaderConfig readerConfig;
  tinyobj::ObjReader reader;

  status::start("Loading", file.string());

  if (!reader.ParseFromFile(file.string(), readerConfig)) {
    std::string msg = "ParseFromFile error";
    if (!reader.Error().empty())
      msg = "TinyObjReader: " + reader.Error();

    status::end(false);
    error(msg);
  }

  if (!reader.Warning().empty())
    warning(std::format("TinyObjReader: {}", reader.Warning()));

  const tinyobj::attrib_t& attrib = reader.GetAttrib();
  const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
  // const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

  std::vector<vertex::PCTN> vertices;
  std::vector<GLuint> indices;
  std::unordered_map<tinyobj::index_t, uint32_t, IndexHasher, IndexEqual> uniqueVertices;

  for (const auto& shape : shapes) {
    for (const auto& idx : shape.mesh.indices) {
      auto [it, inserted] = uniqueVertices.emplace(idx, vertices.size());

      if (inserted) {
        vertex::PCTN vertex;

        vertex.position = {
          attrib.vertices[3 * idx.vertex_index + 0],
          attrib.vertices[3 * idx.vertex_index + 1],
          attrib.vertices[3 * idx.vertex_index + 2]
        };

        // Check if `normal_index` is zero or positive. negative = no normal data
        if (idx.normal_index >= 0) {
          vertex.normal = {
            attrib.normals[3 * idx.normal_index + 0],
            attrib.normals[3 * idx.normal_index + 1],
            attrib.normals[3 * idx.normal_index + 2]
          };
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
        if (idx.texcoord_index >= 0) {
          vertex.texture = {
            attrib.texcoords[2 * idx.texcoord_index + 0],
            attrib.texcoords[2 * idx.texcoord_index + 1]
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
    std::string cname = clrp::format(std::format("[{}]", file.string()), cfmt);
    std::string infoLoad = std::format("[load]\nvertices: {}\ncolors:   {}\ntextures: {}\nnormals:  {}", attrib.vertices.size() / 3, attrib.colors.size() / 3, attrib.texcoords.size() / 2, attrib.normals.size() / 3);
    std::string infoFinal = std::format("[final]\nvertices: {}, indices: {}\n", vertices.size(), indices.size());
    printf("\n==================== %s ====================\n\n%s\n\n%s\n\n", cname.c_str(), infoLoad.c_str(), infoFinal.c_str());

    std::string end = "============================================";
    for (u32 i = 0; i < file.string().size(); i++)
      end += "=";
    end += "\n";

    puts(end.c_str());
  }

  // ==================================== //

  status::end(true);

  return MeshElements(MeshData(vertices, indices));
}

MeshElements::MeshElements(const MeshData& data)  {
  assert(data.vertices && data.indices);

  mode = data.mode;
  elementCount = data.indicesSize / sizeof(data.indices[0]);

  vao.gen();

  vao.bind();
  vbo.allocate(data.vertices, data.verticesSize, data.usage);
  ebo.allocate(data.indices, data.indicesSize, data.usage);

  vbo.bind();
  ebo.bind();

  linkAttributes(data.layout);

  vao.unbind();
  vbo.unbind();
  ebo.unbind();
}

void MeshElements::draw(const Camera* camera, Shader& shader) const {
  draw(camera, shader, getModel());
}

void MeshElements::draw(const Camera* camera, Shader& shader, const mat4& model) const {
  assert(mode && elementCount);
  vao.bind();

  camera->setUniforms(shader);
  setGlobalUniforms(shader);
  shader.setUniformMatrix4f("u_model", model);

  shader.use();
  glDrawElements(mode, elementCount, GL_UNSIGNED_INT, 0);

  vao.unbind();
}

void MeshElements::drawMultiIndirect(const Camera* camera, Shader& shader, const BufferObject& ibo, GLsizei drawCount) const {
  assert(mode && elementCount);
  vao.bind();
  ibo.bind();

  camera->setUniforms(shader);
  setGlobalUniforms(shader);
  shader.setUniformMatrix4f("u_model", getModel());

  shader.use();
  glMultiDrawElementsIndirect(mode, GL_UNSIGNED_INT, nullptr, drawCount, 0);

  ibo.unbind();
  vao.unbind();
}

