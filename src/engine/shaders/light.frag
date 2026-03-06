#version 460 core

in vec2 v_uv;

out vec4 FragColor;

uniform vec3 u_lightColor;

void main() {
  float dist = length(v_uv - 0.5f);
  float alpha = 1.f - smoothstep(0.f, 0.5f, dist);

  FragColor = vec4(u_lightColor, alpha);
}

