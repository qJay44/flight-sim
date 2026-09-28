#version 460 core

in float v_logW;

out vec4 FragColor;

uniform vec3 u_color;
uniform float u_camFar;

void main() {
  float C = 0.001;
  float distNorm = log(C * v_logW + 1.0) / log(C * u_camFar + 1.0);
  gl_FragDepth = distNorm;

  FragColor = vec4(u_color, 1.f);
}

