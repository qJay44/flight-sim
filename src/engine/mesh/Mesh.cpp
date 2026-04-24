#include "Mesh.hpp"

#include "global.hpp"
#include "utils/status.hpp"
#include "utils/clrp.hpp"
#include "VAO.hpp"
#include "utils/utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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

Mesh Mesh::loadObj(const fspath& file, bool printInfo) {
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

  std::vector<VertexPCTN> vertices;
  std::vector<GLuint> indices;
  std::unordered_map<tinyobj::index_t, uint32_t, IndexHasher, IndexEqual> uniqueVertices;

  for (const auto& shape : shapes) {
    for (const auto& idx : shape.mesh.indices) {
      auto [it, inserted] = uniqueVertices.emplace(idx, vertices.size());

      if (inserted) {
        VertexPCTN vertex;

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

  return Mesh(vertices, indices, GL_TRIANGLES);
}

Mesh::Mesh(const std::vector<VertexPCTN>& vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage) : Mesh(std::span(vertices), indices, mode, usage) {}
Mesh::Mesh(const std::vector<VertexPT>&   vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage) : Mesh(std::span(vertices), indices, mode, usage) {}
Mesh::Mesh(const std::vector<VertexPC>&   vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage) : Mesh(std::span(vertices), indices, mode, usage) {}

Mesh::Mesh(const std::vector<VertexPCTN>& vertices, GLenum mode, GLenum usage) : Mesh(std::span(vertices), {}, mode, usage) {}
Mesh::Mesh(const std::vector<VertexPT>&   vertices, GLenum mode, GLenum usage) : Mesh(std::span(vertices), {}, mode, usage) {}
Mesh::Mesh(const std::vector<VertexPC>&   vertices, GLenum mode, GLenum usage) : Mesh(std::span(vertices), {}, mode, usage) {}

void Mesh::screenDraw(const Camera* camera, Shader& shader) {
  static const VAO& vao = VAO::getEmpty();
  setCamUniforms(camera, shader);
  setGlobalUniforms(shader);

  vao.bind();
  glDrawArrays(GL_TRIANGLES, 0, 6);
  vao.unbind();
}

void Mesh::draw(const Camera* camera, Shader& shader, bool forceNoWireframe) const {
  assert(count);
  assert(mode);

  vao.bind();

  setCamUniforms(camera, shader);
  setGlobalUniforms(shader);
  shader.setUniformMatrix4f("u_model", getModel());

  if (global::drawWireframe & !forceNoWireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  shader.use();
  drawFunc(mode, count);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  vao.unbind();
}

void Mesh::setCamUniforms(const Camera* c, Shader& s) {
  s.setUniform1f      ("u_camNear"   , c->getNearPlane());
  s.setUniform1f      ("u_camFar"    , c->getFarPlane());
  s.setUniform1f      ("u_camFov"    , c->getFov());
  s.setUniform3f      ("u_camPos"    , c->getPosition());
  s.setUniform3f      ("u_camRight"  , c->getRight());
  s.setUniform3f      ("u_camUp"     , c->getUp());
  s.setUniform3f      ("u_camForward", c->getForward());
  s.setUniformMatrix4f("u_camProj"   , c->getProj());
  s.setUniformMatrix4f("u_camView"   , c->getView());
  s.setUniformMatrix4f("u_camPV"     , c->getProjView());
}

void Mesh::setGlobalUniforms(Shader& s) {
  s.setUniform1f("u_time", global::time);
}
void Mesh::drawElements(GLenum mode, GLsizei count) {
  glDrawElements(mode, count, GL_UNSIGNED_INT, 0);
}

void Mesh::drawArrays(GLenum mode, GLsizei count) {
  glDrawArrays(mode, 0, count);
}

