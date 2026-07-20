#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::uvec2;
using glm::uvec3;
using glm::uvec4;

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

using glm::dvec2;
using glm::dvec3;
using glm::dvec4;

using glm::mat3;
using glm::mat4;

#ifdef _WIN32
  #include <direct.h>
  #include "glad/glad.h"
  #define CHDIR(p) _chdir(p);
#else
  #include <unistd.h>
  #include "glad/gl.h"
  #define CHDIR(p) chdir(p);
#endif

#include "GLFW/glfw3.h"
#include "defines.hpp"

#define UTILS_ENABLE_GLM
#include "utils/utils.hpp"
#include "utils/types.hpp"

// STL
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <functional>
#include <future>
#include <fstream>
#include <list>
#include <map>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "glm/gtx/norm.hpp"

template<typename T>
inline T normalizeSafe(const T& v) {
  float lenSq = glm::length2(v);
  if (lenSq > 1e-6f)
    return v * glm::inversesqrt(lenSq);

  return T(0.f);
}

