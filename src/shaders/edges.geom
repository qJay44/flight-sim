#version 460 core

layout(triangles) in;
layout(line_strip, max_vertices = 3) out;

uniform mat4 u_camPV;

void main() {
  gl_Position = u_camPV * gl_in[0].gl_Position; EmitVertex();
  gl_Position = u_camPV * gl_in[1].gl_Position; EmitVertex();
  gl_Position = u_camPV * gl_in[2].gl_Position; EmitVertex();

  EndPrimitive();
}

