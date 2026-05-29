#version 460

in vec3 v_rayDir;

out vec4 FragColor;

layout (binding = 0) uniform samplerCube u_skyboxTex;

void main() {
  vec3 col = texture(u_skyboxTex, v_rayDir).rgb;

  FragColor = vec4(col, 1.f);
}

