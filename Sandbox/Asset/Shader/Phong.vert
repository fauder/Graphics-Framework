#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec4 varying_position_view_space;
out vec4 varying_normal_view_space;
out vec4 varying_light_position_view_space;

uniform mat4x4 uniform_transform_world;
uniform mat4x4 uniform_transform_view;
uniform mat4x4 uniform_transform_projection;

uniform vec3 uniform_light_position_world_space;

void main()
{
    mat4x4 world_view_transform       = uniform_transform_world * uniform_transform_view;
    varying_position_view_space       = vec4( position, 1.0 ) * world_view_transform;
    varying_normal_view_space         = vec4( normal * mat3x3( transpose( inverse( world_view_transform ) ) ), 0.0 );
    varying_light_position_view_space = vec4( uniform_light_position_world_space, 1.0 ) * uniform_transform_view;
    gl_Position                       = varying_position_view_space * uniform_transform_projection;
}