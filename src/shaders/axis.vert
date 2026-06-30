#version 460 core

layout(location = 0) in vec3 a_pos;

out vec3 v_color;

uniform mat4 u_camPV;
uniform mat4 u_model;

const vec3 colors[6] = vec3[](
  vec3(1.f, 0.f, 0.f),
  vec3(1.f, 0.f, 0.f),
  vec3(0.f, 1.f, 0.f),
  vec3(0.f, 1.f, 0.f),
  vec3(0.f, 0.f, 1.f),
  vec3(0.f, 0.f, 1.f)
);

void main() {
  v_color = colors[gl_VertexID];
  gl_Position = u_camPV * u_model * vec4(a_pos, 1.f);

}

