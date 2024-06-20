#version 330 core

in vec3 varying_lighting_ambient;
in vec3 varying_lighting_diffuse;
in vec3 varying_lighting_specular;

out vec4 out_color;

struct SurfaceData
{
	vec3 ambient, diffuse, specular;
	float shininess, debugging;
};

uniform SurfaceData uniform_surface_data;

uniform vec3 uniform_color;

void main()
{
	vec3 ambient  = uniform_surface_data.ambient  * varying_lighting_ambient;
	vec3 diffuse  = uniform_surface_data.diffuse  * varying_lighting_diffuse;
	vec3 specular = uniform_surface_data.specular * varying_lighting_specular;

	out_color = vec4( ambient + diffuse + specular, 1.0 );
};