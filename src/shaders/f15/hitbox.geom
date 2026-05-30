#version 460 core

layout(triangles) in;
layout(line_strip, max_vertices = 3) out;

uniform mat4 u_modelView;
uniform mat4 u_camProj;

void main() {
  mat4 pv = u_camProj * u_modelView;
  gl_Position = pv * gl_in[0].gl_Position; EmitVertex();
  gl_Position = pv * gl_in[1].gl_Position; EmitVertex();
  gl_Position = pv * gl_in[2].gl_Position; EmitVertex();

  EndPrimitive();
}

