#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out vec4 varying_position_view_space;
out vec4 varying_normal_view_space;
out vec2 varying_tex_coords;

uniform mat4x4 uniform_transform_world;

void main()
{
    mat4x4 world_view_transform             = uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW;
    mat3x3 world_view_transform_for_normals = mat3x3( transpose( inverse( world_view_transform ) ) );

    varying_position_view_space        = vec4( position, 1.0 ) * world_view_transform;
    varying_normal_view_space          = vec4( normalize( normal * world_view_transform_for_normals ), 0.0 );
    varying_tex_coords                 = tex_coords;
    
    gl_Position                        = varying_position_view_space * _INTRINSIC_TRANSFORM_PROJECTION;
}