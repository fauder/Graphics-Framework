#version 330 core

in vec2 varying_uv;

out vec4 out_color;

uniform sampler2D uniform_texture_sampler_container;
uniform sampler2D uniform_texture_sampler_awesomeface;

void main()
{
	out_color = mix( texture( uniform_texture_sampler_container, varying_uv ), texture( uniform_texture_sampler_awesomeface, varying_uv ), 0.5 );
}