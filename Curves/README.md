# Drawing Curves

This uses GL_LINES_ADJACENCY to draw a curve with the four input values. The geometry is send using 4 values in the VAO and a restart index is used to start the new primitive.

```
    B ------- C
   /            \
  /              \
A/                \D


for t=0 -> t = 1.0 :
  AB=mix(A,B,t) 
  BC=mix(B,C,t)
  CD=mix(D,D,t)
  ABBC=mix(AB,BC,t);
  BCCD=mix(BC,CD,t);
  gl_Position=mix(ABBC,BCCD,t)
  EmitVertex()
EndPrimitive()
```