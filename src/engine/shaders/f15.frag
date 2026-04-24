#version 460 core

in vec3 v_worldPos;
in vec3 v_color;
in vec2 v_uv;
in vec3 v_normal;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_debug0Tex;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;
uniform vec3 u_color;
uniform float u_time;

vec3 directionalLight() {
  vec3 lightDistVec = u_lightPos - v_worldPos;
  float lightDist = length(lightDistVec);

  vec3 lightDir = normalize(lightDistVec);
  vec3 viewDir = normalize(u_camPos - v_worldPos);
  vec3 reflectDir = reflect(-lightDir, v_normal);

  float diffuse = max(dot(v_normal, lightDir), 0.f);
  float ambient = 0.2f;
  float specularLight = 0.5f;
  float specAmount = pow(max(dot(viewDir, reflectDir), 0.f), 8);
  float specular = specAmount * specularLight;

  float lightAmount = (diffuse + specular) + ambient;

  return u_lightColor * lightAmount;
}

void main() {
  vec3 col = u_color;
  col *= directionalLight();
	FragColor = vec4(col, 1.f);
}

