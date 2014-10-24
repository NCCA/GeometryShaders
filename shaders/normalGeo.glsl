#version 330 core
layout(triangles) in;
layout(line_strip, max_vertices = 10) out;

in vec4 normal[];

uniform  float normalSize;
uniform  vec4 vertNormalColour;
uniform  vec4 faceNormalColour;
uniform bool drawFaceNormals;
uniform bool drawVertexNormals;
out vec4 perNormalColour;

void main()
{
  if (drawVertexNormals == true)
  {

  for(int i = 0; i<gl_in.length(); ++i)
  {
    gl_Position = gl_in[i].gl_Position;
    perNormalColour=vec4(1,0,0,1);
    EmitVertex();
    gl_Position = gl_in[i].gl_Position+ normal[i] * abs(normalSize);
    perNormalColour=vec4(1,1,1,1);
    EmitVertex();
    EndPrimitive();
  }
 }
    if (drawFaceNormals == true)
    {
    perNormalColour=faceNormalColour;

    vec4 cent = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;
    vec3 face_normal = normalize(cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz,
                                       gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz));

    gl_Position =  cent;
    EmitVertex();
    gl_Position =  (cent + vec4(face_normal * abs(normalSize), 0.0));
    EmitVertex();
    EndPrimitive();

    perNormalColour=vec4(0,1,0,1);

    gl_Position =  cent;
    face_normal=normalize(normal[0].xyz+normal[1].xyz+normal[2].xyz/3.0);
    EmitVertex();
    gl_Position =  (cent + vec4(face_normal * abs(normalSize), 0.0));
    EmitVertex();
    EndPrimitive();

    }
}


