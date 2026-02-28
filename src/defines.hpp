#pragma once

#define PI 3.141592265359f
#define PI_2 (PI / 2.f)
#define PI_3 (PI / 3.f)
#define PI_6 (PI / 6.f)

#define IM_VEC2_CLASS_EXTRA                                             \
  constexpr ImVec2(const vec2& f) : x(f.x), y(f.y) {}                   \
  operator vec2() const { return vec2(x,y); }                           \
                                                                        \
  ImVec2 operator+(const ImVec2& rhs) const {                           \
    return ImVec2(x + rhs.x, y + rhs.y);                                \
  }                                                                     \

#define IM_VEC3_CLASS_EXTRA                                             \
  constexpr ImVec3(const vec3& f) : x(f.x), y(f.y), z(f.z) {}           \
  operator vec3() const { return vec3(x,y,z); }

#define IM_VEC4_CLASS_EXTRA                                             \
  constexpr ImVec4(const vec4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}   \
  constexpr ImVec4(const vec3& f, const float& w) : x(f.x), y(f.y), z(f.z), w(w) {} \
  operator vec4() const { return vec4(x,y,z,w); }

