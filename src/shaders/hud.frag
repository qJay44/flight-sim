#version 460 core

in vec2 v_uv;

out vec4 FragColor;

uniform sampler2D u_tex;
uniform vec3 u_color;

void main() {
  vec3 col = vec3(texture(u_tex, v_uv).r);
  float alpha = col.r;

  FragColor = vec4(col * u_color, alpha);
}

