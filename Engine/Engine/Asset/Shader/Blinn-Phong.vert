#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Lighting.glsl"
#include "_Intrinsic_Other.glsl"

#pragma feature INSTANCING_ENABLED
#pragma feature SHADOWS_ENABLED

POSITION  vec3 position;
NORMAL    vec3 normal;
TEXCOORDS vec2 tex_coords;
#ifdef INSTANCING_ENABLED
INSTANCE_WORLD_TRANSFORM mat4 world_transform;
#endif

out VS_To_FS
{
    vec4 varying_position_view_space;
    vec4 varying_normal_view_space;
    vec2 varying_tex_coords;
#ifdef SHADOWS_ENABLED
    vec4 varying_position_light_directional_clip_space;
#endif
} vs_out;

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

#ifdef SHADOWS_ENABLED
    #ifdef INSTANCING_ENABLED
        vs_out.varying_position_light_directional_clip_space = vec4( position, 1.0 ) * world_transform * _INTRINSIC_DIRECTIONAL_LIGHT_VIEW_PROJECTION_TRANSFORM;
    #else
        vs_out.varying_position_light_directional_clip_space = vec4( position, 1.0 ) * uniform_transform_world * _INTRINSIC_DIRECTIONAL_LIGHT_VIEW_PROJECTION_TRANSFORM;
    #endif
#endif

    vs_out.varying_position_view_space = vec4( position, 1.0 ) * world_view_transform;
    vs_out.varying_normal_view_space   = vec4( normalize( normal * world_view_transform_for_normals ), 0.0 );
    vs_out.varying_tex_coords          = tex_coords * uniform_texture_scale_and_offset.xy + uniform_texture_scale_and_offset.zw;
    
    gl_Position = vs_out.varying_position_view_space * _INTRINSIC_TRANSFORM_PROJECTION;
}