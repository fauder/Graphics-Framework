#version 330 core

in vec4 varying_color;
out vec4 out_color;

uniform vec4 uniform_light_color;

void main()
{
	out_color = varying_color * uniform_light_color;
};