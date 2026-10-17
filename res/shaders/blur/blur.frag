#version 460 core

out vec4 FragColor;

in vec2 v_uv;

uniform vec2 u_uvOffset;

layout(binding = 0) uniform sampler2D u_image;

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
  vec3 result = texture(u_image, v_uv).rgb * weight[0];

  for (int i = 1; i < 5; i++) {
    result += texture(u_image, v_uv + u_uvOffset).rgb * weight[i];
    result += texture(u_image, v_uv - u_uvOffset).rgb * weight[i];
  }

  FragColor = vec4(result, 1.f);
}

