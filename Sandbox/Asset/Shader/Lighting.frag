#version 330 core

in vec4 varying_color;
out vec4 out_color;

uniform vec4 uniform_light_color;
uniform float uniform_ambient_strength;

void main()
{
	vec4 ambient = uniform_ambient_strength * uniform_light_color;

	out_color = varying_color * ambient;
};