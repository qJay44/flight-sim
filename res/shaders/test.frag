#version 460 core

in vec3 v_worldPos;
in vec2 v_uv;
in vec3 v_normal;
in vec3 v_color;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_debug0Tex;

uniform vec3 u_lightColor;
uniform vec3 u_lightDir;
uniform vec3 u_camPos;
uniform float u_lightAmbient;
uniform float u_lightSpecular;
uniform float u_time;

vec3 directionalLight() {
  vec3 viewDir = normalize(u_camPos - v_worldPos);
  vec3 reflectDir = reflect(-u_lightDir, v_normal);

  float diffuse = max(dot(v_normal, u_lightDir), 0.f);
  float specAmount = pow(max(dot(viewDir, reflectDir), 0.f), 8);
  float specular = specAmount * u_lightSpecular;
  float lightAmount = (diffuse + specular) + u_lightAmbient;

  return u_lightColor * lightAmount;
}

void main() {
  vec2 offset = vec2(0.f);
  offset.y = -u_time * 0.1f;

  vec3 col = texture(u_debug0Tex, v_uv + offset).rgb;
  col *= directionalLight();

	FragColor = vec4(col, 1.f);
}

