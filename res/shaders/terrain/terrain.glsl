#define TERRAIN_MAX_NODES 512
#define BASE_VERTEX_COUNT (128*128)

#define COLOR_DEEP_OCEAN vec3(0.05, 0.15, 0.3)
#define COLOR_SHALLOW    vec3(0.1, 0.3, 0.5)
#define COLOR_SAND       vec3(0.78, 0.72, 0.55)
#define COLOR_GRASS      vec3(0.22, 0.45, 0.18)
#define COLOR_ROCK       vec3(0.4, 0.38, 0.36)
#define COLOR_SNOW       vec3(0.95, 0.95, 0.95)
#define COLOR_SKY        vec3(0.69, 0.84, 1.0)

struct NodeData {
  vec2 center;
  float extents;
  int faceIdx;
  int texLayerIdx;
};

struct Config {
  float planetRadius;
  float planetRadiusPercent;
  float globalScale;
  float initAmplitude;
  float initFrequency;
  float gain;
  float lacunarity;
  float initAmplitudeDetail;
  float initFrequencyDetail;
  float gainDetail;
  float lacunarityDetail;
  float f1VoronoiFreq;
  float displaceStrength;
  float continentFreq;
  float f2f1VoronoiFreq;
  int octavesDisplace;
  int terraceSteps;
};

vec3 cubeToSphere(vec2 pos, int faceIdx) {
  float u = pos.x;
  float v = pos.y;
  vec3 p;

  switch (faceIdx) {
    case 0: p = vec3( 1,  v,  u); break; // Right
    case 1: p = vec3(-1,  v, -u); break; // Left
    case 2: p = vec3( u,  1,  v); break; // Top
    case 3: p = vec3(-u, -1,  v); break; // Bottom
    case 4: p = vec3(-u,  v,  1); break; // Front
    case 5: p = vec3( u,  v, -1); break; // Back
  }

  return p;
}

