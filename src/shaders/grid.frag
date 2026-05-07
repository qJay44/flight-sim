#version 460 core

in vec3 v_worldPos;

out vec4 FragColor;

uniform float u_baseColor;
uniform float u_gridSize;
uniform vec3 u_camPos;

void main() {
  vec2 d = fwidth(v_worldPos.xz);
  vec2 grid = abs(fract(v_worldPos.xz / u_gridSize));
  vec2 line = grid / d;

  float lineMn = min(line.x, line.y);

  float color = u_baseColor - smoothstep(0.f, 1.f, lineMn);

  float distance = length(v_worldPos.xz - u_camPos.xz);
  float alpha = exp(-0.01f * distance) * color;

  FragColor = vec4(vec3(color), alpha);
}

