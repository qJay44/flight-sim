#define PI 3.14159265358979

#define HEIGHT_TILES      2.0
#define HEIGHT_OCTAVES    3
#define HEIGHT_AMP        0.25
#define HEIGHT_GAIN       0.1
#define HEIGHT_LACUNARITY 2.0

#define EROSION_TILES 2.0
#define EROSION_OCTAVES 5
#define EROSION_GAIN 0.5
#define EROSION_LACUNARITY 2.0

// Scale the input slope, leading to more erosion.
// (No effect if EROSION_SLOPE_SENSITIVITY is zero.)
#define EROSION_SLOPE_STRENGTH 3.0

// Set below 1.0 to make the amount of erosion less dependent on the slope.
// This can be used to produce a more consistent and predictable look across
// mountains of different steepness.
// Set to 1.0 for behavior consistent with https://www.shadertoy.com/view/7ljcRW
#define EROSION_SLOPE_SENSITIVITY 0.3

// Continuously modify the noise direction based on the previous fractal sample.
// This is what gives the slopes an interesting "branching" structure.
// A higher value will give you more branches.
#define EROSION_BRANCH_STRENGTH 3.0

// Maximum amount the erosion will modify the base height map
#define EROSION_STRENGTH 0.04

#define MATERIAL_GROUND 0
#define MATERIAL_WATER  1

#define CLIFF_COLOR  vec3(0.22, 0.2, 0.2)
#define DIRT_COLOR   vec3(0.6, 0.5, 0.4)
#define GRASS_COLOR1 vec3(0.15, 0.3, 0.1)
#define GRASS_COLOR2 vec3(0.4, 0.5, 0.2)
#define SAND_COLOR   vec3(0.8, 0.7, 0.6)

#define WATER_HEIGHT 0.45
#define WATER_COLOR vec3(0.0, 0.05, 0.1)
#define WATER_SHORE_COLOR vec3(0.0, 0.25, 0.25)

#define sq(x) (x*x)
#define saturate(x) clamp(x, 0.f, 1.f)

struct Ray {
  vec3 origin;
  vec3 dir;
};

vec2 hash(vec2 x) {
  const vec2 k = vec2(0.3183099, 0.3678794);
  x = x * k + k.yx;
  return -1.0 + 2.0 * fract(16.0 * k * fract(x.x * x.y * (x.x + x.y)));
}

// from https://www.shadertoy.com/view/XdXBRH
vec3 noised(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);

  vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
  vec2 du = 30.0 * f * f * (f * (f - 2.0) + 1.0);

  vec2 ga = hash(i + vec2(0.0, 0.0));
  vec2 gb = hash(i + vec2(1.0, 0.0));
  vec2 gc = hash(i + vec2(0.0, 1.0));
  vec2 gd = hash(i + vec2(1.0, 1.0));

  float va = dot(ga, f - vec2(0.0, 0.0));
  float vb = dot(gb, f - vec2(1.0, 0.0));
  float vc = dot(gc, f - vec2(0.0, 1.0));
  float vd = dot(gd, f - vec2(1.0, 1.0));

  return vec3(
      va + u.x * (vb - va) + u.y * (vc - va) + u.x * u.y * (va - vb - vc + vd),
      ga + u.x * (gb - ga) + u.y * (gc - ga) + u.x * u.y * (ga - gb - gc + gd) +
          du * (u.yx * (va - vb - vc + vd) + vec2(vb, vc) - va));
}

// https://iquilezles.org/articles/intersectors
vec2 boxIntersection(Ray ray, vec3 boxSize, out vec3 outNormal) {
  vec3 m = 1.0 / ray.dir; // can precompute if traversing a set of aligned boxes
  vec3 n = m * ray.origin;   // can precompute if traversing a set of aligned boxes
  vec3 k = abs(m) * boxSize;
  vec3 t1 = -n - k;
  vec3 t2 = -n + k;
  float tN = max(max(t1.x, t1.y), t1.z);
  float tF = min(min(t2.x, t2.y), t2.z);
  if (tN > tF || tF < 0.0)
    return vec2(-1.0); // no intersection
  outNormal = -sign(ray.dir) * step(t1.yzx, t1.xyz) * step(t1.zxy, t1.xyz);

  return vec2(tN, tF);
}

vec3 Tonemap_ACES(vec3 x) {
  // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return (x * (a * x + b)) / (x * (c * x + d) + e);
}

//------------------------------------------------------------------------------
// BRDF
// https://www.shadertoy.com/view/XlKSDR
//------------------------------------------------------------------------------

float pow5(float x) {
  float x2 = x * x;
  return x2 * x2 * x;
}

float D_GGX(float linearRoughness, float NoH, const vec3 h) {
  // Walter et al. 2007, "Microfacet Models for Refraction through Rough
  // Surfaces"
  float oneMinusNoHSquared = 1.0 - NoH * NoH;
  float a = NoH * linearRoughness;
  float k = linearRoughness / (oneMinusNoHSquared + a * a);
  float d = k * k * (1.0 / PI);
  return d;
}

float V_SmithGGXCorrelated(float linearRoughness, float NoV, float NoL) {
  // Heitz 2014, "Understanding the Masking-Shadowing Function in
  // Microfacet-Based BRDFs"
  float a2 = linearRoughness * linearRoughness;
  float GGXV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
  float GGXL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
  return 0.5 / (GGXV + GGXL);
}

vec3 F_Schlick(const vec3 f0, float VoH) {
  // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
  return f0 + (vec3(1.0) - f0) * pow5(1.0 - VoH);
}

float F_Schlick(float f0, float f90, float VoH) {
  return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

float Fd_Burley(float linearRoughness, float NoV, float NoL, float LoH) {
  // Burley 2012, "Physically-Based Shading at Disney"
  float f90 = 0.5 + 2.0 * linearRoughness * LoH * LoH;
  float lightScatter = F_Schlick(1.0, f90, NoL);
  float viewScatter = F_Schlick(1.0, f90, NoV);
  return lightScatter * viewScatter * (1.0 / PI);
}

float Fd_Lambert() {
  return 1. / PI;
}

vec3 Shade(vec3 diffuse, vec3 f0, float smoothness, vec3 n, vec3 v, vec3 l, vec3 lc) {
  vec3 h = normalize(v + l);

  float NoV = abs(dot(n, v)) + 1e-5;
  float NoL = saturate(dot(n, l));
  float NoH = saturate(dot(n, h));
  float LoH = saturate(dot(l, h));

  float roughness = 1.0 - smoothness;
  float linearRoughness = roughness * roughness;
  float D = D_GGX(linearRoughness, NoH, h);
  float V = V_SmithGGXCorrelated(linearRoughness, NoV, NoL);
  vec3 F = F_Schlick(f0, LoH);
  vec3 Fr = (D * V) * F;

  vec3 Fd = diffuse * Fd_Burley(linearRoughness, NoV, NoL, LoH);

  return (Fd + Fr) * lc * NoL;
}

//------------------------------------------------------------------------------
// Atmosphere
//------------------------------------------------------------------------------

#define C_RAYLEIGH (vec3(5.802, 13.558, 33.100) * 1e-6)
#define C_MIE (vec3(3.996, 3.996, 3.996) * 1e-6)

float PhaseRayleigh(float costh) {
  return 3.0 * (1.0 + costh * costh) / (16.0 * PI);
}

float PhaseMie(float costh, float g) {
  g = min(g, 0.9381);
  float k = 1.55 * g - 0.55 * g * g * g;
  float kcosth = k * costh;
  return (1.0 - k * k) / ((4.0 * PI) * (1.0 - kcosth) * (1.0 - kcosth));
}

