#version 460 core

const vec2 vertices[] = vec2[](
  vec2(-1, -1),
  vec2(-1,  1),
  vec2( 1,  1),
  vec2( 1,  1),
  vec2( 1, -1),
  vec2(-1, -1)
);

out vec3 v_nearPoint;
out vec3 v_farPoint;

uniform mat4 u_camProj;
uniform mat4 u_localView;

vec3 unprojPoint(float x, float y, float z, mat4 invPV) {
  vec4 upp = invPV * vec4(x, y, z, 1.f);
  return upp.xyz / upp.w;
}

void main() {
  mat4 invPV = inverse(u_camProj * u_localView);
  vec2 pos = vertices[gl_VertexID];

  v_nearPoint = unprojPoint(pos.x, pos.y, 0.f, invPV);
  v_farPoint = unprojPoint(pos.x, pos.y, 1.f, invPV);

  gl_Position = vec4(pos, 0.f, 1.f);
}

