#version 410 core
layout(lines_adjacency) in;
layout(line_strip, max_vertices = 256) out;
uniform float steps=0.1;
out vec4 colour;
subroutine vec4 curve(in float t,in vec4 p0,in vec4 p1,in vec4 p2,in vec4 p3);

subroutine uniform curve curveSubroutine;

subroutine (curve)
vec4 bezier(in float t,in vec4 p0,in vec4 p1,in vec4 p2,in vec4 p3)
{
    colour=vec4(0,1,0,0);
    float B0 = (1.-t)*(1.-t)*(1.-t);
    float B1 = 3.*t*(1.-t)*(1.-t);
    float B2 = 3.*t*t*(1.-t);
    float B3 = t*t*t;

    vec4 p = B0*p0 + B1*p1 + B2*p2 + B3*p3;
    return p;
}

subroutine (curve)
vec4 lerpCurve(in float t,in vec4 p0,in vec4 p1,in vec4 p2,in vec4 p3)
{
  colour=vec4(1,0,0,0);
  vec4 AB=mix(p0,p1,t);
  vec4 BC=mix(p1,p2,t);
  vec4 CD=mix(p2,p3,t);
  vec4 ABBC=mix(AB,BC,t);
  vec4 BCCD=mix(BC,CD,t);
  return mix(ABBC,BCCD,t);
}

void main()
{
  for(float t=0; t<=1.0; t+=steps)
  {

    gl_Position=curveSubroutine(t,gl_in[0].gl_Position,
        gl_in[1].gl_Position,
        gl_in[2].gl_Position,
        gl_in[3].gl_Position);
    EmitVertex();
  }
  EndPrimitive();

}


