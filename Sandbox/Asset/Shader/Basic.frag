#version 330 core
in vec3 vertex_color;
out vec4 color;

uniform vec4 final_color;

void main()
{
//	color = vec4( vertex_color, 1.0f );
	color = vec4( final_color );
};