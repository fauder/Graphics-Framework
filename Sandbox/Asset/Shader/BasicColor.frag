#version 460 core

out vec4 out_color;

#pragma feature INSTANCING_ENABLED

#ifdef INSTANCING_ENABLED
in vec4 varying_color;
#else
uniform vec4 uniform_color; /* _hint_color4 */
#endif

void main()
{
#ifdef INSTANCING_ENABLED
	out_color = varying_color;
#else
	out_color = uniform_color;
#endif
}