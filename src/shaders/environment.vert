#version 460 core

const vec2 vertices[] = vec2[](
  vec2(-1, -1),
  vec2(-1,  1),
  vec2( 1,  1),
  vec2( 1,  1),
  vec2( 1, -1),
  vec2(-1, -1)
);

out vec3 v_rayDir;

uniform mat4 u_proj;
uniform mat4 u_viewRot;

void main() {
  vec2 ndc = vertices[gl_VertexID];
  gl_Position = vec4(ndc, 1.f, 1.f);

  mat4 invPV = inverse(u_proj * u_viewRot);
  vec4 unproj = invPV * gl_Position;

  v_rayDir = unproj.xyz / unproj.w;
}

