#version 330 compatibility
layout(triangles) in;
layout(triangle_strip, max_vertices=1) out;

void main()
{	
  gl_Position = gl_in[i].gl_Position;
  EmitVertex();
  EndPrimitive();
}  