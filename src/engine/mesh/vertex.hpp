#pragma once

namespace vertex {

struct Attribute {
  GLuint index;
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

  static const Layout& getLayout() {
    static constexpr Attribute attribs[] = {
      {0, 3, GL_FLOAT}
    };

    static constexpr Layout layout = {attribs, 1, sizeof(P)};

    return layout;
  }
};

// ----- PT -------------------------------------------------------------------------------- //

struct PT {
  vec3 position;
  vec2 texture;

  static const Layout& getLayout() {
    static constexpr Attribute attribs[] = {
      {0, 3, GL_FLOAT},
      {1, 2, GL_FLOAT}
    };

    static constexpr Layout layout = {attribs, 2, sizeof(PT)};

    return layout;
  }
};

// ----- PCTN ------------------------------------------------------------------------------ //

struct PCTN {
  vec3 position;
  vec3 color;
  vec2 texture;
  vec3 normal;

  static const Layout& getLayout() {
    static constexpr Attribute attribs[] = {
      {0, 3, GL_FLOAT},
      {1, 3, GL_FLOAT},
      {2, 2, GL_FLOAT},
      {3, 3, GL_FLOAT}
    };

    static constexpr Layout layout = {attribs, 4, sizeof(PCTN)};

    return layout;
  }
};

// ----------------------------------------------------------------------------------------- //

template<typename T>
concept IsVertexType =
  std::is_same_v<T, P>    ||
  std::is_same_v<T, PT>   ||
  std::is_same_v<T, PCTN>;

} // vertex

