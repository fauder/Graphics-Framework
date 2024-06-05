#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 varying_uv;

uniform mat4x4 uniform_transform_world;
uniform mat4x4 uniform_transform_view;
uniform mat4x4 uniform_transform_projection;

void main()
{
    gl_Position = vec4( position, 1.0 ) * uniform_transform_world * uniform_transform_view * uniform_transform_projection;
    varying_uv  = uv;
}