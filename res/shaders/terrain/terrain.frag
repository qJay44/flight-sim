#version 460 core

#include "../common.glsl"

out vec4 FragColor;

in vec3 v_worldPos;
in vec3 v_normal;
in vec2 v_uv;
in flat int v_layerIdx;

layout(binding = 0) uniform sampler2DArray u_texArray;

uniform vec3 u_lightDir;
uniform float u_planetRadius;
uniform float u_seaThreshold;
uniform float u_sandThreshold;
uniform float u_mountainThreshold;
uniform float u_heightScale;

void main() {
  vec2 texSize = textureSize(u_texArray, 0).xy;
  vec2 texelSize = 1.f / texSize;
  // vec2 uv = clamp(v_uv, texelSize, texSize - texelSize);
  vec2 uv = v_uv;

  vec4 terrainData = texture(u_texArray, vec3(uv, v_layerIdx));
  float height = terrainData.a / u_heightScale;
  vec3 terrainNormal = terrainData.rgb;

  // Calculate slope (steepness)
  // We check how much the surface normal aligns with the outward radial direction of the planet.
  // dot = 1.0 means perfectly flat ground. dot = 0.0 means a vertical 90-degree cliff wall.
  vec3 radialDir = normalize(v_normal);
  float flatness = max(0.f, dot(terrainNormal, radialDir));
  float slope = 1.f - flatness;

  vec3 surfaceColor = COLOR_GRASS;

  // Beaches / Sand
  if (height < u_sandThreshold) {
    float sandToGrass = smoothstep(u_seaThreshold, u_sandThreshold, height);
    surfaceColor = mix(COLOR_SAND, COLOR_GRASS, sandToGrass);
  } else {
    float slopeWeight = smoothstep(0.25f, 0.6f, slope);
    surfaceColor = mix(COLOR_GRASS, COLOR_ROCK, slopeWeight);

    // Mountains
    if (height > u_mountainThreshold) {
      float snowBlend = smoothstep(u_mountainThreshold, 0.98f, height);
      float cliffWeight = smoothstep(0.35f, 0.15f, slope);
      vec3 highAltitudeColor = mix(COLOR_ROCK, COLOR_SNOW, cliffWeight);
      surfaceColor = mix(surfaceColor, highAltitudeColor, snowBlend);
    }
  }

  float diffuse = max(0.f, dot(terrainNormal, u_lightDir));
  float ambient = 0.1f;

  vec3 color = surfaceColor * (diffuse + ambient);

  FragColor = vec4(color, 1.f);
  // FragColor = vec4(uv, 0.f, 1.f);
}

