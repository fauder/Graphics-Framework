#version 330 core

in vec4 varying_position_view_space;
in vec4 varying_normal_view_space;
in vec4 varying_light_position_view_space;

out vec4 out_color;

struct LightData
{
	vec3 ambient, diffuse, specular;
	vec3 position;
};

struct SurfaceData
{
	vec3 ambient, diffuse, specular;
	float shininess;
};

uniform LightData uniform_light_data;
uniform SurfaceData uniform_surface_data;

void main()
{
/* Ambient term: */
	vec3 ambient = uniform_surface_data.ambient * uniform_light_data.ambient;

/* Diffuse term: */
	vec4 normal_view_space   = normalize( varying_normal_view_space );
	vec4 to_light_view_space = normalize( varying_light_position_view_space - varying_position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = uniform_surface_data.diffuse * uniform_light_data.diffuse * diffuse_contribution;

/* Specular term: */
	vec4 view_direction_view_space = normalize( -varying_position_view_space );
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, view_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = uniform_surface_data.specular * uniform_light_data.specular * specular_contribution;

	out_color = vec4( ambient + diffuse + specular, 1.0 );
};