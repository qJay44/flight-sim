#pragma once

class Shader {
public:
  Shader() = default;

  [[nodiscard]] Shader(
    const fspath& vsPath,
    const fspath& fsPath,
    const fspath& gsPath = ""
  );

  [[nodiscard]] Shader(
    const fspath& vsPath,
    const fspath& fsPath,
    const fspath& tescPath,
    const fspath& tesePath,
    const fspath& gsPath = ""
  );

  [[nodiscard]] Shader(const fspath& compPath);

  static void setDirectoryLocation(const fspath& path);

  GLint getUniformLoc(const std::string& name);

  // NOTE: Call this before any GPU run (glDispatchCompute, glDrawElements, etc.)
  void use() const;
  void printUniforms() const;

  void setUniform1f (GLint loc, const GLfloat& n);
  void setUniform2f (GLint loc, const vec2& v);
  void setUniform3f (GLint loc, const vec3& v);
  void setUniform4f (GLint loc, const vec4& v);
  void setUniform1i (GLint loc, const GLint& v);
  void setUniform1ui(GLint loc, const GLuint& v);
  void setUniform2i (GLint loc, const ivec2& v);
  void setUniform1fv(GLint loc, GLsizei count, const GLfloat* v);
  void setUniform3fv(GLint loc, GLsizei count, const GLfloat* v);
  void setUniformMatrix4f(const GLint& loc, const mat4& m);

  void setUniform1f (const std::string& name, const GLfloat& n);
  void setUniform2f (const std::string& name, const vec2& v);
  void setUniform3f (const std::string& name, const vec3& v);
  void setUniform4f (const std::string& name, const vec4& v);
  void setUniform1i (const std::string& name, const GLint& v);
  void setUniform1ui(const std::string& name, const GLuint& v);
  void setUniform2i (const std::string& name, const ivec2& v);
  void setUniform1fv(const std::string& name, GLsizei count, const float* v);
  void setUniform3fv(const std::string& name, GLsizei count, const float* v);
  void setUniformMatrix4f(const std::string& name, const mat4& m);

private:
  static fspath directory;
  GLuint program = 0;
  std::unordered_map<std::string, GLint> locs;

private:
  static GLuint load(fspath path, int type);
  static GLuint compile(const fspath& path, int type);
  static void link(GLuint program);
};

