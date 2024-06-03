#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv;

out vec3 varying_color;
out vec2 varying_uv;

uniform mat4x4 uniform_transformation;

void main()
{
    gl_Position   = vec4( position, 1.0) * uniform_transformation;
    varying_color = color;
    varying_uv    = uv;
}