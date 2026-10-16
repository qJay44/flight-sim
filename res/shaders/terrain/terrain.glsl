#define TERRAIN_MAX_NODES 512

// Linear space
#define COLOR_SAND  vec3(0.5789, 0.4854, 0.2684)
#define COLOR_GRASS vec3(0.0358, 0.1726, 0.0230)
#define COLOR_ROCK  vec3(0.1332, 0.1190, 0.1056)
#define COLOR_SNOW  vec3(0.8933, 0.8933, 0.8933)
#define COLOR_SKY   vec3(0.4420, 0.6814, 1.0000)

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

