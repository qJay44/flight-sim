#version 460 core

in vec3 v_nearPoint;
in vec3 v_farPoint;

out vec4 FragColor;

uniform vec3 u_camPos;
uniform float u_camNear;
uniform float u_camFar;

vec4 grid(vec2 uv, float space) {
  vec2 coord = uv / space;
  vec2 grid = abs(fract(coord - 0.5f) - 0.5f) / fwidth(coord);
  float line = min(grid.x, grid.y);
  float minZ = min(fwidth(coord.x), fwidth(coord.y));
  float alpha = 1.f - min(line, 1.f);

  vec4 color = vec4(vec3(0.4f), alpha);

  float widthFactor = 1.5f;

  if (abs(uv.x) < fwidth(uv.x) * widthFactor)
    color = vec4(0.1f, 0.4f, 0.9f, alpha);
  else if (abs(uv.y) < fwidth(uv.y) * widthFactor)
    color = vec4(0.9f, 0.2f, 0.2f, alpha);

  return color;
}

void main() {
  float t = -v_nearPoint.y / (v_farPoint.y - v_nearPoint.y);

  // If above horizon
  if (t < 0.f)
    discard;

  vec3 localHitPoint = v_nearPoint + t * (v_farPoint - v_nearPoint);
  vec2 worldUV = localHitPoint.xz + u_camPos.xz;

  float distToCam = length(localHitPoint);
  float fadeStart = u_camNear;
  float fadeEnd = u_camFar * 1.1f;
  float fading = 1.f - clamp((distToCam - fadeStart) / (fadeEnd - fadeStart), 0.f, 1.f);

  vec4 gridColor = grid(worldUV, 1.f) + grid(worldUV, 10.f);
  gridColor.a *= fading;

  if (gridColor.a < 0.05f)
    discard;

  FragColor = gridColor;
}

