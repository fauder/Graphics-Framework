#version 460 core

#include "_Attributes.glsl"

POSITION vec3 position;

void main()
{
    gl_Position = vec4( position, 1.0 );
}