#pragma once

#include "MeshData.hpp"
#include "Transformable.hpp"
#include "VAO.hpp"
#include "BufferObject.hpp"
#include "../Camera.hpp"
#include "../Shader.hpp"
#include "vertex.hpp"

class Mesh : public Transformable {
public:
  Mesh(const Mesh&) = delete;
  Mesh(Mesh&&) = default;

  Mesh& operator=(const Mesh&) = delete;
  Mesh& operator=(Mesh&&) = default;
  ~Mesh() = default;

  // NOTE: Only for attributes having all same primitve (e.g. all float (no float, float, int))
  static void linkAttributes(const vertex::Layout& layout);
  static void drawScreen(const Camera* camera, Shader& shader);

  void updateBufferVBO(const MeshData& data);

  virtual void draw(const Camera* camera, Shader& shader) const = 0;
  virtual void draw(const Camera* camera, Shader& shader, const mat4& model) const = 0;

protected:
  GLenum mode;
  GLsizei elementCount;
  VAO vao;
  BufferObject vbo{GL_ARRAY_BUFFER};

protected:
  Mesh() = default;

  static void setGlobalUniforms(Shader& s);
};

