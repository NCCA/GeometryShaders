#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
/// @brief flag to indicate if model has unit normals if not normalize
uniform bool Normalize;
/// @brief the current fragment normal for the vert being processed
out vec3 fragmentNormal;
// the eye position of the camera
uniform vec3 viewerPos;
/// @brief the vertex passed in
layout (location =0) in vec3 inVert;
/// @brief the normal passed in
layout (location =2) in vec3 inNormal;
/// @brief the in uv
layout (location =1) in vec2 inUV;
out vec3 eyeDirection;

struct Lights
{
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float spotCosCutoff;

};

#define numLights 8
// array of lights
uniform Lights light[numLights];
// direction of the lights used for shading
out vec3 lightDir[numLights];
// direction of the lights used for shading
out vec3 halfVector[numLights];

uniform mat4 M;
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix;

void main()
{
 // calculate the fragments surface normal
 fragmentNormal = (normalMatrix*inNormal);

 if (Normalize == true)
 {
	fragmentNormal = normalize(fragmentNormal);
 }

 // calculate the vertex position
 gl_Position = MVP*vec4(inVert,1.0);
 // Transform the vertex to eye co-ordinates for frag shader
 /// @brief the vertex in eye co-ordinates  homogeneous
vec4 eyeCord=MV*vec4(inVert,1);

vec4 worldPosition = M * vec4(inVert, 1.0);
eyeDirection = normalize(viewerPos - worldPosition.xyz);
float dist;

 for(int i=0; i<numLights; ++i)
 {
	 lightDir[i]=vec3(light[i].position.xyz-eyeCord.xyz);
	 dist = length(lightDir[i]);
	 lightDir[i]/= dist;
	 halfVector[i] = normalize(eyeDirection + lightDir[i]);
 }

}
