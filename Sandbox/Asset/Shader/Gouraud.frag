#version 330 core

in vec4 varying_lighting_color;

out vec4 out_color;

uniform vec4 uniform_color;

void main()
{
	out_color = uniform_color * varying_lighting_color;
};