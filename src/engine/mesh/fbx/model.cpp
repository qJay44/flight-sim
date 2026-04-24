#include "model.hpp"

#include <print>

#include "glm/gtc/type_ptr.hpp"
#include "ufbx.h"
#include "utils/utils.hpp"

namespace {

glm::mat4 ufbxToGlm(const ufbx_matrix& m) {
  glm::mat4 res{1.f};

  res[0] = {m.m00, m.m10, m.m20, 0.f};
  res[1] = {m.m01, m.m11, m.m21, 0.f};
  res[2] = {m.m02, m.m12, m.m22, 0.f};
  res[3] = {m.m03, m.m13, m.m23, 1.f};

  return res;
}

}

namespace fbx {

Model load(const fspath& file, bool printInfo) {
  // NOTE: Creating only from vertices (no indices)

  ufbx_load_opts opts = { 0 }; // Optional, pass NULL for defaults
  ufbx_error err; // Optional, pass NULL if you don't care about errors
  ufbx_scene *scene = ufbx_load_file(file.string().c_str(), &opts, &err);
  if (!scene)
    error("[Mesh::loadFbx] Failed to load: {}", err.description.data);

  size_t printInfoTitleLen = 2;
  if (printInfo) {
    std::string title = std::format("\n==================== {} ====================\n", file.string().c_str());
    printInfoTitleLen = title.size();
    puts(title.c_str());
  }

  Model model;

  for (size_t i = 0; i < scene->nodes.count; i++) {
    ufbx_node *node = scene->nodes.data[i];
    if (node->is_root) continue;

    ufbx_mesh* mesh = node->mesh;

    if (printInfo) {
      std::print("Object: {:<20}", node->name.data);
      if (mesh) {
        std::print(" -> {:>5} faces | colors: {:3} | uv: {:3} | normal: {:3}",
          node->mesh->faces.count,
          node->mesh->vertex_color.exists  ? "yes" : "no",
          node->mesh->vertex_uv.exists     ? "yes" : "no",
          node->mesh->vertex_normal.exists ? "yes" : "no"
        );
      }
      else if (node->light)
        printf(" (light)");
      else if (node->camera)
        printf(" (camera)");
      else if (node->bone)
        printf(" (bone)");
      else
        printf(" attrib_type: (%d)",  node->attrib_type);
      puts("");
    }

    std::vector<VertexPCTN> vertices;

    if (mesh) {
      for (size_t f = 0; f < mesh->faces.count; f++) {
        ufbx_face face = mesh->faces.data[f];
        u32 tris[64]; // Big enough for most N-gons
        size_t numTris = ufbx_triangulate_face(tris, 64, mesh, face);

        for (u32 i = 0; i < numTris * 3; i++) {
          u32 idx = tris[i];
          VertexPCTN vertex{};

          ufbx_vec3 position = ufbx_get_vertex_vec3(&mesh->vertex_position, idx);
          vertex.position = glm::make_vec3(position.v);

          if (mesh->vertex_color.exists) {
            ufbx_vec4 color = ufbx_get_vertex_vec4(&mesh->vertex_color, idx);
            vertex.color = glm::make_vec3(color.v);
          }

          if (mesh->vertex_uv.exists) {
            ufbx_vec2 uv = ufbx_get_vertex_vec2(&mesh->vertex_uv, idx);
            vertex.texture = glm::make_vec2(uv.v);
          }

          if (mesh->vertex_normal.exists) {
            ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, idx);
            vertex.normal = glm::make_vec3(normal.v);
          }

          vertices.push_back(vertex);
        }
      }
      model.meshes.push_back({node->name.data, Mesh(vertices, GL_TRIANGLES, GL_STATIC_DRAW)});

    } else if (node->attrib_type == UFBX_ELEMENT_EMPTY) {
      Socket s;
      s.name = node->name.data;
      s.transform = ufbxToGlm(node->node_to_world);

      model.sockets.push_back(s);
    } else {
      error("[fbx::load] Unhandled attrib_type []", (int)node->attrib_type);
    }
  }

  ufbx_free_scene(scene);

  if (printInfo) {
    puts("");
    for (size_t i = 0; i < printInfoTitleLen - 2; i++) printf("=");
    puts("\n");
  }

  return model;
}

} // namespace fbx

