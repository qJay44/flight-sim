#include "Shader.hpp"

#include "utils/clrp.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "utils/utils.hpp"

fspath Shader::directory = "";

Shader::Shader(const fspath& vsPath, const fspath& fsPath, const fspath& gsPath)
  : Shader(vsPath, fsPath, "", "", gsPath) {}

Shader::Shader(const fspath& vsPath, const fspath& fsPath, const fspath& tescPath, const fspath& tesePath, const fspath& gsPath) {
  program = glCreateProgram();
  GLuint shaders[5];
  u8 idx = 0;

  shaders[idx++] = compile(vsPath, GL_VERTEX_SHADER);
  shaders[idx++] = compile(fsPath, GL_FRAGMENT_SHADER);

  if (!tescPath.empty()) {
    if (tesePath.empty())
      error("[Shader::Shader] Provided TCS shader [{}] but no TES shader [{}]", tescPath.string(), tesePath.string());

    shaders[idx++] = compile(tescPath, GL_TESS_CONTROL_SHADER);
    shaders[idx++] = compile(tesePath, GL_TESS_EVALUATION_SHADER);
  }

  if (!gsPath.empty())
    shaders[idx++] = compile(gsPath, GL_GEOMETRY_SHADER);

  for (int i = 0; i < idx; i++) glAttachShader(program, shaders[i]);
  link(program);
  for (int i = 0; i < idx; i++) glDeleteShader(shaders[i]);
}

Shader::Shader(const fspath& compPath) {
  program = glCreateProgram();
  GLuint shader = compile(compPath, GL_COMPUTE_SHADER);
  glAttachShader(program, shader);
  link(program);
  glDeleteShader(shader);
}

void Shader::setDirectoryLocation(const fspath& path) { Shader::directory = path; }

GLint Shader::getUniformLoc(const std::string& name) {
  const auto it = locs.find(name);
  if (it != locs.end())
    return it->second;

  GLint loc = glGetUniformLocation(program, name.c_str());
  locs.emplace(name, loc);

  #ifndef NDEBUG
    if (loc == -1)
      warning("[Shader::getUniformLoc] Didn't found location [{}]", name);
  #endif

  return loc;
}

void Shader::use() const { glUseProgram(program); }

void Shader::printUniforms() const {
  std::string head = std::format("==================== Program: {} ====================", program);
  printf("\n%s\n\n", head.c_str());

  for (const auto& [u, l] : locs)
    printf("%s: %d\n", clrp::format(u, clrp::ATTRIBUTE::BOLD, clrp::FG::CYAN).c_str(), l);

  puts("");
  for (size_t i = 0; i < head.length(); i++) printf("%s", "=");
  puts("");
}

void Shader::setUniform1f (GLint loc, const GLfloat& n) { glProgramUniform1f (program, loc, n); }
void Shader::setUniform2f (GLint loc, const vec2& v)    { glProgramUniform2f (program, loc, v.x, v.y); }
void Shader::setUniform3f (GLint loc, const vec3& v)    { glProgramUniform3f (program, loc, v.x, v.y, v.z); }
void Shader::setUniform4f (GLint loc, const vec4& v)    { glProgramUniform4f (program, loc, v.x, v.y, v.z, v.w); }
void Shader::setUniform1i (GLint loc, const GLint& v)   { glProgramUniform1i (program, loc, v); }
void Shader::setUniform1ui(GLint loc, const GLuint& v)  { glProgramUniform1ui(program, loc, v); }
void Shader::setUniform2i (GLint loc, const ivec2& v)   { glProgramUniform2i (program, loc, v.x, v.y); }
void Shader::setUniform1fv(GLint loc, GLsizei count, const GLfloat* v) { glProgramUniform1fv(program, loc, count, v); }
void Shader::setUniform3fv(GLint loc, GLsizei count, const GLfloat* v) { glProgramUniform3fv(program, loc, count, v); }
void Shader::setUniformMatrix3f(const GLint& loc, const mat3& m) { glProgramUniformMatrix3fv(program, loc, 1, GL_FALSE, value_ptr(m)); }
void Shader::setUniformMatrix4f(const GLint& loc, const mat4& m) { glProgramUniformMatrix4fv(program, loc, 1, GL_FALSE, value_ptr(m)); }

void Shader::setUniform1f (const std::string& name, const GLfloat& n) { setUniform1f (getUniformLoc(name), n); }
void Shader::setUniform2f (const std::string& name, const vec2& v)    { setUniform2f (getUniformLoc(name), v); }
void Shader::setUniform3f (const std::string& name, const vec3& v)    { setUniform3f (getUniformLoc(name), v); }
void Shader::setUniform4f (const std::string& name, const vec4& v)    { setUniform4f (getUniformLoc(name), v); }
void Shader::setUniform1i (const std::string& name, const GLint& v)   { setUniform1i (getUniformLoc(name), v); }
void Shader::setUniform1ui(const std::string& name, const GLuint& v)  { setUniform1ui(getUniformLoc(name), v); }
void Shader::setUniform2i (const std::string& name, const ivec2& v)   { setUniform2i (getUniformLoc(name), v); }
void Shader::setUniform1fv(const std::string& name, GLsizei count, const GLfloat* v) { setUniform1fv(getUniformLoc(name), count, v); }
void Shader::setUniform3fv(const std::string& name, GLsizei count, const GLfloat* v) { setUniform3fv(getUniformLoc(name), count, v); }
void Shader::setUniformMatrix3f(const std::string& name, const mat3& m) { setUniformMatrix3f(getUniformLoc(name), m); }
void Shader::setUniformMatrix4f(const std::string& name, const mat4& m) { setUniformMatrix4f(getUniformLoc(name), m); }

GLuint Shader::load(fspath path, int type) {
  path = directory.empty() ? path : directory / path;
  std::string shaderStr = readFile(path);
  const char* shaderStrPtr = shaderStr.c_str();
  GLuint shaderId = glCreateShader(type);
  glShaderSource(shaderId, 1, &shaderStrPtr, NULL);

  return shaderId;
}

GLuint Shader::compile(const fspath& path, int type) {
  GLuint shaderId = load(path, type);
  GLint hasCompiled;
  char infoLog[1'024];

  glCompileShader(shaderId);
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &hasCompiled);

  // if GL_FALSE
  if (!hasCompiled) {
    glGetShaderInfoLog(shaderId, 1'024, NULL, infoLog);
    std::string fmt = clrp::prepare(clrp::ATTRIBUTE::BOLD, clrp::FG::RED);
    std::string head = std::format("\n===== Shader compilation error ({}) =====\n\n", path.string().c_str());
    printf(fmt.c_str(), head.c_str());
    puts(infoLog);
    for (size_t i = 0; i < head.length() - 3; i++)
      printf(fmt.c_str(), "=");
    puts("");
    exit(1);
  }

  return shaderId;
}

void Shader::link(GLuint program) {
  GLint hasLinked;
  char infoLog[1'024];

  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &hasLinked);

  if (hasLinked == GL_FALSE) {
    glGetProgramInfoLog(program, 1'024, NULL, infoLog);
    std::string fmt = clrp::prepare(clrp::ATTRIBUTE::BOLD, clrp::FG::RED);
    printf(fmt.c_str(), "\n===== Shader link error =====\n\n");
    puts(infoLog);
    printf(fmt.c_str(), "=============================\n\n");
    exit(1);
  }
}

