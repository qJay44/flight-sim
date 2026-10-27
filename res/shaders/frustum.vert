#version 460 core

#include "common.glsl"

layout (location = 0) in vec3 a_pos;

out float v_fragDepth;

uniform mat4 u_proj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform float u_camFar;

void main() {
	gl_Position = u_proj * (u_localView * (u_localTranslation * vec4(a_pos, 1.f)));
  v_fragDepth = linearDepthToLogDepth(gl_Position.w, u_camFar) / gl_Position.w * 0.5f + 0.5f;
}

