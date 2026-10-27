#version 460 core

in float v_fragDepth;

out vec4 FragColor;

uniform vec3 u_color;

void main() {
  gl_FragDepth = v_fragDepth;
  FragColor = vec4(u_color, 1.f);
}

