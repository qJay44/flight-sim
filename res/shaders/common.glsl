#define FLOAT_MAX 3.4028235e38f

#define PI 3.14159265358979f
#define PI_2 (PI / 2.0)
#define PI_3 (PI / 3.0)
#define PI_4 (PI / 4.0)
#define PI_6 (PI / 6.0)
#define TAU (2.f * PI)

#define sq(x) ((x)*(x))
#define dot0(x, y) max(0.0, dot(x, y))
#define clamp01(x) clamp(x, 0.0, 1.0)
#define max0(x) max(0.0, x)
#define smoothstep_inv(e0, e1, x) (1.f - smoothstep(e1, e0, x))

struct Ray {
  vec3 origin;
  vec3 dir;
};

struct RayHitInfo {
  float distToSphere;
  float distThroughSphere;
};

struct Material {
  vec3 albedo;
  vec3 normal;
  float ao;
  float roughness;
  float metallic;
  float height;
};

RayHitInfo raySphere(vec3 center, float radius, Ray ray) {
  vec3 offset = ray.origin - center;
  float a = 1.f;
  float b = 2.f * dot(offset, ray.dir);
  float c = dot(offset, offset) - radius * radius;
  float discriminant = b * b - 4.f * a * c;

  if (discriminant > 0.f) {
    float s = sqrt(discriminant);
    float dstToSphereNear = max(0.f, (-b - s) / (2.f * a));
    float dstToSphereFar = (-b + s) / (2.f * a);

    if (dstToSphereFar >= 0.f)
      return RayHitInfo(dstToSphereNear, dstToSphereFar - dstToSphereNear);
  }

  return RayHitInfo(FLOAT_MAX, 0.f);
}

vec3 calcViewVec(vec2 uv, mat4 pvInv) {
  vec2 ndc = uv * 2.f - 1.f;
  vec4 clipPos = vec4(ndc, -1.f, 1.f);
  vec4 worldPos = pvInv * clipPos;
  worldPos /= worldPos.w;

  return worldPos.xyz;
}

vec2 complexMultiply(vec2 n1, vec2 n2) {
  // (a + bi) * (c + di) = ac + adi + bci + bdi^2
  // Since i^2 = -1:
  // Real part:      ac - bd
  // Imaginary part: ad + bc
  return vec2(
    n1.x * n2.x - n1.y * n2.y,
    n1.x * n2.y + n1.y * n2.x
  );
}

vec2 complexExp(vec2 a) {
  return vec2(cos(a.y), sin(a.y)) * exp(a.x);
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

float linearizeDepth(float depth, float near, float far) {
  float z = depth * 2.f - 1.f;
  return (2.f * near * far) / (far + near - z * (far - near));
}

//------------------------------------------------------------------------------
// Atmosphere
//------------------------------------------------------------------------------

#define C_RAYLEIGH (vec3(5.802, 13.558, 33.100) * 1e-6)
#define C_MIE (vec3(3.996) * 1e-6)

float PhaseRayleigh(float costh) {
  return 3.0 * (1.0 + costh * costh) / (16.0 * PI);
}

float PhaseMie(float costh, float g) {
  g = min(g, 0.9381);
  float k = 1.55 * g - 0.55 * g * g * g;
  float kcosth = k * costh;
  return (1.0 - k * k) / ((4.0 * PI) * (1.0 - kcosth) * (1.0 - kcosth));
}

