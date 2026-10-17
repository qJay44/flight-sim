#version 460 core

out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_texScreenHDR;

void main() {
  vec3 col = texture(u_texScreenHDR, v_uv).rgb;
  vec3 brightPart = max(col - vec3(1.f), vec3(0.f));

  FragColor = vec4(brightPart, 1.f);
}

