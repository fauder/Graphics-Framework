#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

#pragma feature INSTANCING_ENABLED

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;
#ifdef INSTANCING_ENABLED
layout (location = 3) in mat4 world_transform;
#endif

out vec4 varying_position_view_space;
out vec4 varying_normal_view_space;
out vec2 varying_tex_coords;

#ifndef INSTANCING_ENABLED
uniform mat4x4 uniform_transform_world;
#endif
uniform vec4 uniform_texture_scale_and_offset;

void main()
{
#ifdef INSTANCING_ENABLED
    mat4x4 world_view_transform = world_transform * _INTRINSIC_TRANSFORM_VIEW;
#else
    mat4x4 world_view_transform = uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW;
#endif

    mat3x3 world_view_transform_for_normals = mat3x3( transpose( inverse( world_view_transform ) ) );

    varying_position_view_space = vec4( position, 1.0 ) * world_view_transform;
    varying_normal_view_space   = vec4( normalize( normal * world_view_transform_for_normals ), 0.0 );
    varying_tex_coords          = tex_coords * uniform_texture_scale_and_offset.xy + uniform_texture_scale_and_offset.zw;
    
    gl_Position = varying_position_view_space * _INTRINSIC_TRANSFORM_PROJECTION;
}