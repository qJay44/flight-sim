#define PI 3.14159265358979f
#define TAU (2.f * PI)

#define sq(x) (x*x)
#define saturate(x) clamp(x, 0.f, 1.f)
#define smoothstep_inv(e0, e1, x) (1.f - smoothstep(e1, e0, x))

struct Ray {
  vec3 origin;
  vec3 dir;
};

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

