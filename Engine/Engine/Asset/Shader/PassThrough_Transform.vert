#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

layout (location = 0) in vec3 position;

uniform mat4x4 uniform_transform_world;

void main()
{
    gl_Position = vec4( position, 1.0f ) * uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
}