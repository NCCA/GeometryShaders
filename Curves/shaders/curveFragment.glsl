#version 410 core

/// @brief our output fragment colour
layout (location =0 )out vec4 fragColour;
in vec4 colour;
void main ()
{
    fragColour =  colour;
}

