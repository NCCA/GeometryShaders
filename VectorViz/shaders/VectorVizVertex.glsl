#version 410 core

layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inDir;

uniform mat4 MVP;
out vec3 dir;
void main()
{
  gl_Position=MVP*vec4(inPos,1);
  
  //dir=inDir;
  dir=(MVP*vec4(inDir,0)).xyz;

}
