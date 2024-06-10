#version 330 core

layout (location = 0) in vec3 position;

out vec4 varying_color;

uniform mat4x4 uniform_transform_world;
uniform mat4x4 uniform_transform_view;
uniform mat4x4 uniform_transform_projection;

uniform vec4 uniform_color;

void main()
{
    gl_Position = vec4( position, 1.0 ) * uniform_transform_world * uniform_transform_view * uniform_transform_projection;
    varying_color = uniform_color;
}