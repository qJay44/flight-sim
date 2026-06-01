#version 460 core

#include "common.glsl"

out vec4 FragColor;

in vec3 v_rayDir;

layout(binding = 0) uniform sampler2D u_bufferA;
layout(binding = 1) uniform sampler2D u_bufferB;
layout(binding = 2) uniform samplerCube u_texSkybox;

uniform vec3 u_sunDir;
uniform vec3 u_sunColor;
uniform vec3 u_camPos;
uniform float u_camNear;
uniform float u_camFar;
uniform ivec2 u_bufferSize;
uniform vec2 u_resolution;

const float scale = 0.01;

vec4 getBufferA(vec2 uv) {
  uv = clamp(uv * scale, vec2(0.001f), vec2(0.999f));
  // uv *= u_bufferSize / u_resolution;

  return texture(u_bufferA, uv);
}

vec4 getBufferB(vec2 uv) {
  uv = fract(uv * scale);
  // uv *= u_bufferSize / u_resolution;

  return texture(u_bufferB, uv);
}

vec3 getSkyColor(vec3 normal) {
  return texture(u_texSkybox, normal).rgb;
}

vec4 map(vec3 p, out float erosion) {
  vec2 uv = p.xz;
  const float heightScale = 220.f;

  vec4 tex = getBufferA(uv);
  float height = tex.x * heightScale;
  vec3 normal = tex.yzz;

  normal.xz /= heightScale;
  normal.y = sqrt(1.0 - dot(normal.xz, normal.xz)); // Recover Y

  erosion = tex.w;

  return vec4(height, normal);
}

float march(Ray ray, out vec3 normal, out int material, out float s_t) {
  s_t = 9999.f;

  float tStart = u_camNear;
  float tEnd = u_camFar;

  material = MATERIAL_GROUND;

  if (ray.dir.y > 0.f && ray.origin.y > u_bufferSize.y)
    return -1.f;

  float stepSize = 0.f;
  float stepScale = 1.f;
  float t = tStart;
  float altitude = 0.f;

  for (int i = 0; i < 64; i++) {
    vec3 pos = ray.origin + t * ray.dir;
    float foo;
    vec4 tex = map(pos, foo);
    float h = tex.x;

    normal = tex.yzw;
    altitude = pos.y - h;
    altitude *= 0.02f;
    s_t = max(0.f, min(s_t, altitude / t));

    if (altitude < 0.f) {
      // Maybe gone through, so decrease step size and move one step back
      stepScale *= 0.5f;
      t -= stepSize * stepScale;
    } else {
      // stepSize = abs(altitude) + 0.01f;
      stepSize = 0.1f + t * 0.1f;
      t += stepSize * stepScale;
    }
  }

  if (t > tEnd) {
    s_t = 9999.f;
    return -1.f;
  }

  if (ray.dir.y != 0.f) {
    float tWater = (WATER_HEIGHT - ray.origin.y) / ray.dir.y;

    if (tWater > tStart && tWater < tEnd && (tWater < t || t < 0.f)) {
      t = tWater;
      normal = vec3(0.f, 1.f, 0.f);
      material = MATERIAL_WATER;
    }
  }

  return t;
}

vec3 getReflection(vec3 p, vec3 r, vec3 sun, float smoothness) {
  vec3 refl = getSkyColor(r) * 4.f;
  Ray ray = Ray(p, r);

  vec3 foo;
  float r_t;
  int r_material;
  march(ray, foo, r_material, r_t);
  return refl * (1.0 - exp(-r_t * 10.0 * sq(smoothness)));
}

void main() {
  Ray ray;
  ray.origin = u_camPos;
  ray.dir = normalize(v_rayDir);

  vec4 foo;
  vec3 normal;
  int material;
  float t = march(ray, normal, material, foo.w);

  vec3 sun = u_sunDir;
  vec3 color;

  if (t < 0.f) {
    color = getSkyColor(ray.dir);
  } else {
    // FragColor = vec4(0.f, 1.f, 0.f, 1.f);
    // return;

    vec3 pos = ray.origin + t * ray.dir;
    float erosion;
    float diff = pos.y - map(pos, erosion).x;

    vec4 breakupTex = getBufferB(pos.xz / u_bufferSize);
    vec3 breakupNormal = breakupTex.zyw;
    normal = normalize(normal + breakupNormal.xzy * 0.1f);

    float breakup = breakupTex.x;
    vec3 f0 = vec3(0.04);
    float smoothness = 0.f;
    float reflAmount = 0.f;
    float occlusion = 1.f;
    vec3 r = reflect(ray.dir, normal);
    vec3 diffuseColor = vec3(0.5f);

    switch (material) {
      case MATERIAL_GROUND: {
        occlusion = sq(saturate(erosion + 0.5f));

        // Cliffs / Dirt
        diffuseColor = CLIFF_COLOR * smoothstep(0.4f, 0.52f, pos.y);
        diffuseColor = mix(diffuseColor, DIRT_COLOR, smoothstep(0.3f, 0.f, occlusion + breakup * 1.f));

        // Grass
        vec3 grassMix = mix(GRASS_COLOR1, GRASS_COLOR2, smoothstep(0.4f, 0.6f, pos.y - erosion * 0.05f + breakup * 0.3f));
        diffuseColor = mix(diffuseColor, grassMix,
          smoothstep(WATER_HEIGHT + 0.05f, WATER_HEIGHT + 0.02f, pos.y - breakup * 0.02f) *
          smoothstep(0.8f, 1.f, normal.y + breakup * 0.1f)
        );

        // Snow
        diffuseColor = mix(diffuseColor, vec3(1.f), smoothstep(0.53f, 0.6f, pos.y + breakup * 0.1f));

        // Sand (beach)
        diffuseColor = mix(diffuseColor, SAND_COLOR, smoothstep(WATER_HEIGHT + 0.005f, WATER_HEIGHT, pos.y + breakup * 0.01f));

        diffuseColor *= 1.f + breakup * 0.5f;
        break;
      }
      case MATERIAL_WATER: {
        float shore = normal.y > 0.01f ? exp(-diff * 60.f) : 0.f;
        float foam = normal.y > 0.01f ? smoothstep(0.005f, 0.f, diff + breakup * 0.005f) : 0.f;
        diffuseColor = mix(WATER_COLOR, WATER_SHORE_COLOR, shore);
        diffuseColor = mix(diffuseColor, vec3(1.f), foam);
        smoothness = 0.95f;
        break;
      }
    }

    float shadow = 1.f;
    // float s_t;
    // int s_material;
    // Ray shadowRay;
    // shadowRay.origin = pos + vec3(0.f, 1.f, 0.f);
    // shadowRay.dir = sun;

    // march(shadowRay, foo.xyz, s_material, s_t);
    // float shadow = 1.f - exp(-s_t * 20.f);

    // Ambient
    color = diffuseColor * getSkyColor(normal) * occlusion * Fd_Lambert();

    // Direct
    color += Shade(diffuseColor, f0, smoothness, normal, -ray.dir, sun, u_sunColor * shadow);

    // Bounce
    color += diffuseColor * u_sunColor * (dot(normal, sun * vec3(1.f, -1.f, 1.f)) * 0.5f + 0.5f) * Fd_Lambert() / PI;

    // Reflection
    color += getReflection(pos, r, sun, smoothness) * F_Schlick(f0, dot(-ray.dir, normal));
  }

  // float costh = dot(ray.dir, sun);
  // float phaseR = PhaseRayleigh(costh);
  // float phaseM = PhaseMie(costh, 0.6f);

  // vec2 od = vec2(0.f);
  // vec3 tsm;
  // vec3 sct = vec3(0.f);
  // float rayLength = (t > 0.f ? t : box.y) - box.x;
  // float stepSize = rayLength / 16.f;

  // for (float i = 0.f; i < 16.f; i++) {
  //   vec3 p = ray.origin + (box.x + (i + 0.5f) * stepSize) * ray.dir;
  //   float h = max(0.f, p.y - 0.35f);
  //   float d = 1.f - saturate(h / 0.2f);

  //   if (p.y < 0.35f)
  //     d = 0.f;

  //   float densityR = d * 1e5f;
  //   float densityM = d * 1e5f;

  //   od += stepSize * vec2(densityR, densityM);
  //   tsm = exp(-(od.x * C_RAYLEIGH + od.y * C_MIE));

  //   sct += tsm * C_RAYLEIGH * phaseR * densityR * stepSize;
  //   sct += tsm * C_MIE      * phaseM * densityM * stepSize;
  // }

  // color = color * tsm + sct * 10.f;
  color = Tonemap_ACES(color);
  color = pow(color, vec3(1.f / 2.2f));

  FragColor = vec4(color, 1.f);
}

