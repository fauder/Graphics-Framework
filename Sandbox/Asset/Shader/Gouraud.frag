#version 330 core

in vec3 varying_lighting_color;

out vec4 out_color;

uniform vec3 uniform_color;

void main()
{
	out_color = vec4( uniform_color * varying_lighting_color, 1.0 );
};