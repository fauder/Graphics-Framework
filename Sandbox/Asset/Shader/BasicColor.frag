#version 330 core

out vec4 out_color;

uniform vec4 uniform_color;

void main()
{
	out_color = uniform_color;
};