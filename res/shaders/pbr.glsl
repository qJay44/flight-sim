#include "common.glsl"

struct Material {
  vec3 albedo;
  vec3 normal;
  vec3 emissivity;
  vec3 baseReflectivity;
  float ao;
  float roughness;
  float metallic;
  float height;
};

const Material materialInit = Material(
  vec3(0.f),
  vec3(0.f),
  vec3(0.f),
  vec3(0.f),
  0.f,
  0.f,
  0.f,
  0.f
);


const float epsilon = 1e-6f;

// GGX/Trowbridge-Reitz Normal Distribution Function
float DistributionGGX(float roughness, vec3 N, vec3 H) {
  float a = sq(roughness);
  float a2 = sq(a);
  float NdotH = dot0(N, H);
  float NdotH2 = sq(NdotH);

  float numerator = a2;
  float denominator = PI * sq(NdotH2 * (a2 - 1.f) + 1.f);

  return numerator / max(denominator, epsilon);
}

// Schlick-Beckmann Geometry Shadowing Function
float GeometrySchlickGGX(float NdotX, float k) {
  float numerator = NdotX;
  float denominator = NdotX * (1.f - k) + k;

  return numerator / max(denominator, epsilon);
}

// Smith Model
float GeometrySmith(float roughness, vec3 N, vec3 V, vec3 L, float k) {
  float NdotV = dot0(N, V);
  float NdotL = dot0(N, L);

  return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
}

float GeometrySmith_Direct(float roughness, vec3 N, vec3 V, vec3 L) {
  float k = sq(roughness + 1.f) * 0.125f;
  return GeometrySmith(roughness, N, V, L, k);
}

float GeometrySmith_IBL(float roughness, vec3 N, vec3 V, vec3 L) {
  float k = sq(roughness) * 0.5f;
  return GeometrySmith(roughness, N, V, L, k);
}

// Fresnel-Schlick Function
vec3 FresnelSchlick(vec3 f0, float cosTheta) {
  return f0 + (vec3(1.f) - f0) * pow(clamp01(1.f - cosTheta), 5.f);
}

vec3 FresnelSchlickRoughness(vec3 f0, float cosTheta, float roughness) {
  return f0 + (max(vec3(1.f - roughness), f0) - f0) * pow(clamp01(1.f - cosTheta), 5.f);
}

vec3 BRDF_CookTorrance(Material material, vec3 V, vec3 L, vec3 H, float geomSmith) {
  vec3 N = material.normal;
  vec3 lambert = material.albedo / PI;
  vec3 f0 = mix(material.baseReflectivity, material.albedo, material.metallic);

  float D = DistributionGGX(material.roughness, N, H);
  float G = geomSmith;
  vec3 F = FresnelSchlick(f0, dot0(N, H));

  vec3 numerator = D * F * G;
  float denominator = 4.f * dot0(N, V) * dot0(N, L);
  vec3 Ks = numerator / max(denominator, epsilon);
  vec3 Kd = (vec3(1.f) - Ks) * (1.f - material.metallic);

  return Kd * lambert + Ks;
}

float RadicalInverse_VdC(uint bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

  return float(bits) * 2.3283064365386963e-10; // 0x100000000
}

vec2 Hammersley(uint i, uint N) {
  return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
  float a = roughness * roughness;

  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  // from spherical coordinates to cartesian coordinates
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  // from tangent-space vector to world-space sample vector
  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;

  return normalize(sampleVec);
}

