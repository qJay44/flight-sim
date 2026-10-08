#version 460 core

#include "terrain.glsl"

#define MESH_RESOLUTION 256
#define BASE_VERTEX_COUNT (MESH_RESOLUTION*MESH_RESOLUTION)

layout(location = 0) in vec3 a_pos;

out vec3 v_worldPos;
out vec3 v_normal;
out vec3 v_viewVec;
out vec2 v_uv;
out flat int v_face;

uniform mat4 u_proj;
uniform mat4 u_localView;
uniform mat4 u_localTranslation;
uniform vec3 u_camPos;
uniform float u_camFar;
uniform float u_planetRadius;
uniform float u_heightScale;
uniform float u_heightScaleScale;
uniform int u_quadsPerAxis;

layout(binding = 0) uniform sampler2D u_texDisplacement;

void main() {
  float sign = gl_VertexID >= BASE_VERTEX_COUNT ? -1.f : 1.f;
  float offsetSize = 1.f / u_quadsPerAxis;
  int quadsTotal = u_quadsPerAxis * u_quadsPerAxis;
  int face = gl_InstanceID / quadsTotal;
  int quad = gl_InstanceID % quadsTotal;
  int row = quad / u_quadsPerAxis;
  int col = quad % u_quadsPerAxis;

  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec2 localPos = uv / (u_quadsPerAxis * 0.5f) - 1.f;
  localPos += vec2(col, row) * 0.5f;

  vec3 sphereDir = normalize(cubeToSphere(localPos, face));
  vec3 pos = sphereDir * (u_planetRadius + u_heightScale * u_heightScaleScale * sign);

  vec3 wave = texture(u_texDisplacement, uv).rgb;
  vec4 worldPos = vec4(pos + sphereDir * wave * 30.f, 1.f);

  v_worldPos = worldPos.xyz;
  v_normal = sphereDir;
  v_viewVec = u_camPos - v_worldPos;
  v_uv = uv;
  v_face = face;

	gl_Position = u_proj * (u_localView * (u_localTranslation * worldPos));

  float C = 0.001;
  float distNorm = log(C * gl_Position.w + 1.0) / log(C * u_camFar + 1.0);
  gl_Position.z = (distNorm * 2.0 - 1.0) * gl_Position.w;
}

