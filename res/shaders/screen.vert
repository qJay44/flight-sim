#version 460 core

#include "common.glsl"

layout(location = 0) in vec3 a_pos;

out vec3 v_viewVec;
out vec2 v_uv;

uniform mat4 u_pvInvLocal;
uniform vec3 u_camPos;

void main() {
  vec2 uv = a_pos.xy * 0.5f + 0.5f;
  v_viewVec = calcViewVec(uv, u_pvInvLocal);
  v_uv = uv;

	gl_Position = vec4(a_pos.xy, 1.f, 1.f);
}

