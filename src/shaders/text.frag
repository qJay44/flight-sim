#version 460 core

out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_texTextAtlas;

uniform vec3 u_color;
uniform vec3 u_colorOutline;
uniform float u_outlineThreshold;

void main() {
  const float textThreshold = 0.5f; // basically a threshold where letter starts

  float sdf = texture(u_texTextAtlas, v_uv).r;
  float edgeSmoothing = fwidth(sdf);

  float textMask = smoothstep(textThreshold - edgeSmoothing, textThreshold + edgeSmoothing, sdf);
  float outlineMask = smoothstep(u_outlineThreshold - edgeSmoothing, u_outlineThreshold + edgeSmoothing, sdf);

  vec3 col = mix(u_colorOutline, u_color, textMask);

  FragColor = vec4(col, outlineMask);
}

