#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

#pragma feature INSTANCING_ENABLED

layout (location = 0) in vec3 position;

#ifdef INSTANCING_ENABLED
layout (location = 3) in mat4 world_transform;
#endif

#ifndef INSTANCING_ENABLED
uniform mat4x4 uniform_transform_world;
#endif

void main()
{
#ifdef INSTANCING_ENABLED
    gl_Position = vec4( position, 1.0f ) * world_transform * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
#else
    gl_Position = vec4( position, 1.0f ) * uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
#endif
}