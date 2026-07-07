#version 460 core

#include "common.glsl"

out vec4 FragColor;

in vec3 v_sphereDir;
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
  vec4 terrainData = texture(u_texArray, vec3(v_uv, v_layerIdx));
  float height = terrainData.r / u_heightScale;
  vec3 terrainNormal = normalize(terrainData.gba);

  // Calculate slope (steepness)
  // We check how much the surface normal aligns with the outward radial direction of the planet.
  // dot = 1.0 means perfectly flat ground. dot = 0.0 means a vertical 90-degree cliff wall.
  vec3 radialDir = normalize(v_sphereDir);
  float flatness = max(0.f, dot(terrainNormal, radialDir));
  float slope = 1.f - flatness;

  vec3 surfaceColor = COLOR_GRASS;

  // Sea level thresholds
  if (height < u_seaThreshold) {
    float waterDeepness = smoothstep(0.f, u_seaThreshold, height);
    surfaceColor = mix(COLOR_DEEP_OCEAN, COLOR_SHALLOW, waterDeepness);
  // Beaches / Sand
  } else if (height < u_sandThreshold) {
    float sandToGrass = smoothstep(u_seaThreshold, u_sandThreshold, height);
    surfaceColor = mix(COLOR_SAND, COLOR_GRASS, sandToGrass);
  } else {
    float slopeWeight = smoothstep(0.25f, 0.6f, slope);
    surfaceColor = mix(COLOR_GRASS, COLOR_ROCK, slopeWeight);

    // Mountains
    if (height > u_mountainThreshold) {
      float snowBlend = smoothstep(u_mountainThreshold, 0.90f, height);
      float cliffWeight = smoothstep(0.35f, 0.15f, slope);
      vec3 highAltitudeColor = mix(COLOR_ROCK, COLOR_SNOW, cliffWeight);
      surfaceColor = mix(surfaceColor, highAltitudeColor, snowBlend);
    }
  }

  float diffuse = max(0.f, dot(terrainNormal, u_lightDir));
  float ambient = 0.1f;

  vec3 color = surfaceColor * (diffuse + ambient);

  FragColor = vec4(color, 1.f);
}

