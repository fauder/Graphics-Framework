#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec4 varying_position_world;
out vec3 varying_normal;

uniform mat4x4 uniform_transform_world;
uniform mat4x4 uniform_transform_view;
uniform mat4x4 uniform_transform_projection;

void main()
{
    varying_position_world = vec4( position, 1.0 ) * uniform_transform_world;
    varying_normal         = normal;
    gl_Position            = varying_position_world * uniform_transform_view * uniform_transform_projection;
}