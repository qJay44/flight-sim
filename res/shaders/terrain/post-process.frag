#version 460 core

out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_texScreenColor;
layout(binding = 1) uniform sampler2D u_texScreenDepth;
layout(binding = 2) uniform sampler2D u_texWaterHDR;
layout(binding = 3) uniform sampler2D u_texScreenBlurredWaterSpecular;

const float exposure = 0.25f;
const float gamma = 2.2f;

void main() {
  vec3 color = texture(u_texScreenColor, v_uv).rgb;
  vec3 colorWaterHDR = texture(u_texWaterHDR, v_uv).rgb;
  vec3 colorWaterBlurredSpecular = texture(u_texScreenBlurredWaterSpecular, v_uv).rgb;
  vec3 bloom = colorWaterHDR + colorWaterBlurredSpecular;
  bloom = vec3(1.f) - exp(-bloom * exposure);

  vec3 finalColor = color + bloom;

  finalColor = pow(finalColor, vec3(1.f / gamma));

  FragColor = vec4(finalColor, 1.f);
}

