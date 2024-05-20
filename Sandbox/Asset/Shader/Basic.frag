#version 330 core
in vec3 varying_color;
in vec2 varying_uv;

out vec4 out_color;

//uniform vec4 uniform_color;
uniform sampler2D uniform_texture_sampler_container;

void main()
{
//	out_color = vec4( varying_color, 1.0f );
	out_color = texture( uniform_texture_sampler_container, varying_uv );
//	out_color = mix( vec4( uniform_color ), texture( uniform_texture_sampler_container, varying_uv ), 0.5f );
};