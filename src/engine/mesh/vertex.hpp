#pragma once

namespace vertex {

struct Attribute {
  GLuint size;
  GLenum type;
};

struct Layout {
  const Attribute* attribs;
  size_t count;
  size_t stride;
};

// ----- P --------------------------------------------------------------------------------- //

struct P {
  vec3 position;
};

static constexpr Attribute P_ATTRIBS[] = {
  {3, GL_FLOAT}
};

static constexpr Layout P_LAYOUT = {P_ATTRIBS, 1, sizeof(P)};

// ----- PT -------------------------------------------------------------------------------- //

#pragma pack(push, 1)
struct PT {
  vec3 position;
  vec2 texture;
};
#pragma pack(pop)

static constexpr Attribute PT_ATTRIBS[] = {
  {3, GL_FLOAT},
  {2, GL_FLOAT}
};

static constexpr Layout PT_LAYOUT = {PT_ATTRIBS, 2, sizeof(PT)};

// ----- PCTN ------------------------------------------------------------------------------ //

struct PCTN {
  vec3 position;
  vec3 color;
  vec2 texture;
  vec3 normal;
};

static constexpr Attribute PCTN_ATTRIBS[] = {
  {3, GL_FLOAT},
  {3, GL_FLOAT},
  {2, GL_FLOAT},
  {3, GL_FLOAT}
};

static constexpr Layout PCTN_LAYOUT = {PCTN_ATTRIBS, 4, sizeof(PCTN)};

// ----------------------------------------------------------------------------------------- //

} // vertex

