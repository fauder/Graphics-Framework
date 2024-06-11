#version 330 core

in vec4 varying_position_view_space;
in vec4 varying_normal_view_space;
in vec4 varying_light_position_view_space;

out vec4 out_color;

uniform vec3 uniform_color;
uniform vec3 uniform_light_color;

uniform float uniform_ambient_strength;
uniform float uniform_diffuse_strength;
uniform float uniform_specular_strength;
uniform float uniform_specular_power;

void main()
{
/* Ambient term: */
	vec3 ambient = uniform_ambient_strength * uniform_light_color;

/* Diffuse term: */
	vec4 normal_view_space   = normalize( varying_normal_view_space );
	vec4 to_light_view_space = normalize( varying_light_position_view_space - varying_position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 ) * uniform_diffuse_strength;
	vec3 diffuse               = diffuse_contribution * uniform_light_color;

/* Specular term: */
	vec4 view_direction_view_space = normalize( -varying_position_view_space );
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, view_direction_view_space ), 0.0 ), uniform_specular_power ) * uniform_specular_strength;
	vec3 specular               = specular_contribution * uniform_light_color;

	out_color = vec4( uniform_color * ( ambient + diffuse + specular ), 1.0 );
};