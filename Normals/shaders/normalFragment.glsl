#version 330 core

/// @brief our output fragment colour
layout (location =0 )out vec4 fragColour;
in vec4 perNormalColour;

void main ()
{
    fragColour = perNormalColour;
}

