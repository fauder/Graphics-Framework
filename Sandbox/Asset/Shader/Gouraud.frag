#version 330 core

in vec3 varying_ambient_from_light;
in vec3 varying_diffuse_from_light;
in vec3 varying_specular_from_light;

in vec2 varying_tex_coords;

out vec4 out_color;

uniform sampler2D uniform_surface_data_diffuse_map_slot, uniform_surface_data_specular_map_slot;

void main()
{
	vec3 diffuse_sample  = vec3( texture( uniform_surface_data_diffuse_map_slot,  varying_tex_coords ) );
	vec3 specular_sample = vec3( texture( uniform_surface_data_specular_map_slot, varying_tex_coords ) );

	vec3 ambient  =  diffuse_sample * varying_ambient_from_light;
	vec3 diffuse  =  diffuse_sample * varying_diffuse_from_light;
	vec3 specular = specular_sample * varying_specular_from_light;

	out_color = vec4( ambient + diffuse + specular, 1.0 );
}