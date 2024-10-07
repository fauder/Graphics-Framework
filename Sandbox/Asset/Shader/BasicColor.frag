#version 460 core

out vec4 out_color;

uniform vec4 uniform_color; /* _hint_color4 */

void main()
{
	out_color = uniform_color;
}