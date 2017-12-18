#version 330 core
layout(lines_adjacency) in;
layout(line_strip, max_vertices = 256) out;
uniform float steps=0.1;
void main()
{
  for(float t=0; t<=1.0; t+=steps)
  {
    vec4 AB=mix(gl_in[0].gl_Position,gl_in[1].gl_Position,t);
    vec4 BC=mix(gl_in[1].gl_Position,gl_in[2].gl_Position,t);
    vec4 CD=mix(gl_in[2].gl_Position,gl_in[3].gl_Position,t);
    vec4 ABBC=mix(AB,BC,t);
    vec4 BCCD=mix(BC,CD,t);
    gl_Position =mix(ABBC,BCCD,t);
    EmitVertex();
  }
  EndPrimitive();

}


