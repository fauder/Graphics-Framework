#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 varying_uv;

uniform mat4x4 uniform_transform_world;

layout ( std140 ) uniform Essential_CameraMatrices
{
    mat4x4 ESSENTIAL_TRANSFORM_VIEW;
    mat4x4 ESSENTIAL_TRANSFORM_PROJECTION;
};

void main()
{
    gl_Position = vec4( position, 1.0 ) * uniform_transform_world * ESSENTIAL_TRANSFORM_VIEW * ESSENTIAL_TRANSFORM_PROJECTION;
    varying_uv  = uv;
}