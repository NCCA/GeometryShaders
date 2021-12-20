#version 410 core
// based on https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;  
in vec3 dir[];
out vec3 colour;

uniform float thickness;
uniform float thickness2;
uniform vec2  viewportSize;//=vec2(1024,720);


void main()
{
  vec4 p1 = gl_in[0].gl_Position;
  vec4 p2;
  p2.xyz = gl_in[0].gl_Position.xyz+dir[0];
  p2.w=p1.w;
  vec2 linedir    = normalize((p2.xy - p1.xy) * viewportSize);
  vec2 offset =  vec2(-linedir.y, linedir.x) * thickness / viewportSize;
  vec2 offset2 = vec2(-linedir.y, linedir.x) * thickness2 / viewportSize;

  gl_Position = p1 + vec4(offset.xy * p1.w, 0.0, 0.0);
  colour=vec3(1,0,0);
  EmitVertex();
  gl_Position = p1 - vec4(offset.xy * p1.w, 0.0, 0.0);
  EmitVertex();
  colour=vec3(1,1,1);

  gl_Position = p2 + vec4(offset2.xy * p2.w, 0.0, 0.0);
  EmitVertex();
  gl_Position = p2 - vec4(offset2.xy * p2.w, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}



