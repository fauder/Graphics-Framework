#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Lighting.glsl"
#include "_Intrinsic_Other.glsl"

#pragma feature INSTANCING_ENABLED
#pragma feature SHADOWS_ENABLED
#pragma feature PARALLAX_MAPPING_ENABLED

POSITION  vec3 position;
NORMAL    vec3 normal;
TEXCOORDS vec2 tex_coords;
TANGENT   vec3 tangent;
#ifdef INSTANCING_ENABLED
INSTANCE_WORLD_TRANSFORM mat4 world_transform;
#endif

out VS_To_FS
{
    vec4 position_view_space;
    vec4 surface_normal_view_space;
    vec2 tex_coords;

    mat3x3 tangent_to_view_space_transformation;

#ifdef PARALLAX_MAPPING_ENABLED
    vec3 viewing_direction_tangent_space;
#endif

#ifdef SHADOWS_ENABLED
    vec4 position_light_directional_clip_space;
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
        vs_out.position_light_directional_clip_space = vec4( position, 1.0 ) * world_transform * _INTRINSIC_DIRECTIONAL_LIGHT_VIEW_PROJECTION_TRANSFORM;
    #else
        vs_out.position_light_directional_clip_space = vec4( position, 1.0 ) * uniform_transform_world * _INTRINSIC_DIRECTIONAL_LIGHT_VIEW_PROJECTION_TRANSFORM;
    #endif
#endif

    vs_out.position_view_space       = vec4( position, 1.0 ) * world_view_transform;
    vs_out.surface_normal_view_space = vec4( normalize( normal * world_view_transform_for_normals ), 0.0 );
    vs_out.tex_coords                = tex_coords * uniform_texture_scale_and_offset.xy + uniform_texture_scale_and_offset.zw;

/* Tangent Space: */

    /* Tangent   (view-space) => */ vs_out.tangent_to_view_space_transformation[ 0 ] = normalize( tangent * mat3x3( world_view_transform ) );
    /* Normal    (view-space) => */ vs_out.tangent_to_view_space_transformation[ 2 ] = vs_out.surface_normal_view_space.xyz;

    /* Gram-Schmidt re-orthogonalization of the tangent: */
    vs_out.tangent_to_view_space_transformation[ 0 ] = normalize( vs_out.tangent_to_view_space_transformation[ 0 ] - 
                                                                  dot( vs_out.tangent_to_view_space_transformation[ 0 ], vs_out.tangent_to_view_space_transformation[ 2 ] ) *
                                                                  vs_out.tangent_to_view_space_transformation[ 2 ] );

    /* Tangent space is right handed (tangent points in the u direction, bitangent points in the v direction.
     * BUT to construct a tangent-to-view transformation matrix, we need to find tangent space cardinal axes IN view space. 
     * Therefore, the bitangent cardinal axis of this basis is the left handed cross product of cross( tangent, normal ). */
    /* Bitangent (view-space) => */ vs_out.tangent_to_view_space_transformation[ 1 ] = cross( vs_out.tangent_to_view_space_transformation[ 0 ],
                                                                                              vs_out.tangent_to_view_space_transformation[ 2 ] );

#ifdef PARALLAX_MAPPING_ENABLED
    /* Faster way of transposing tangent_to_view_space_transformation: utilize the dot products: */
    vec3 viewing_direction_view_space = normalize( -vs_out.position_view_space.xyz );
    vs_out.viewing_direction_tangent_space = vec3( dot( vs_out.tangent_to_view_space_transformation[ 0 ], viewing_direction_view_space ),
                                                   dot( vs_out.tangent_to_view_space_transformation[ 1 ], viewing_direction_view_space ),
                                                   dot( vs_out.tangent_to_view_space_transformation[ 2 ], viewing_direction_view_space ) );
#endif

    gl_Position = vs_out.position_view_space * _INTRINSIC_TRANSFORM_PROJECTION;
}