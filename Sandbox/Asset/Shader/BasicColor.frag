#version 460 core

out vec4 out_color;

uniform vec3 uniform_color;

void main()
{
	out_color = vec4( uniform_color, 1.0 );
}