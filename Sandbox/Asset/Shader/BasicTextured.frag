#version 460 core

in vec2 varying_uv;

out vec4 out_color;

uniform sampler2D uniform_texture_sampler;

void main()
{
	out_color = texture( uniform_texture_sampler, varying_uv );
}