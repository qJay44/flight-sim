#version 460

layout (location = 0) in vec3 inPos;

out vec3 v_uv;

uniform mat4 u_camProj;
uniform mat4 u_camView;

void main() {
  v_uv = inPos;
  mat4 skyboxView = mat4(mat3(u_camView));
  vec4 clipSpace = u_camProj * skyboxView * vec4(inPos, 1.f);
  gl_Position = clipSpace.xyww;
}

