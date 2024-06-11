#version 330 core

in vec4 varying_position_world;
in vec3 varying_normal_world;

out vec4 out_color;

uniform vec4 uniform_color;
uniform vec4 uniform_light_color;

uniform float uniform_ambient_strength;
uniform float uniform_diffuse_strength;
uniform float uniform_specular_strength;
uniform float uniform_specular_power;

uniform vec3 uniform_light_position_world;
uniform vec3 uniform_camera_position_world;

void main()
{
/* Ambient term: */
	vec4 ambient = uniform_ambient_strength * uniform_light_color;

/* Diffuse term: */
	vec4 normal_world   = vec4( normalize( varying_normal_world ), 0.0f );
	vec4 to_light_world = normalize( vec4( uniform_light_position_world, 1.0 ) - varying_position_world );

	float diffuse_contribution = max( dot( to_light_world, normal_world ), 0.0 ) * uniform_diffuse_strength;
	vec4 diffuse               = diffuse_contribution * uniform_light_color;

/* Specular term: */
	vec4 view_direction_world            = vec4( uniform_camera_position_world, 1.0 ) - varying_position_world;
	vec4 reflected_light_direction_world = reflect( -to_light_world, normal_world ); // reflect() expects the first argument to be the vector FROM the light source to the fragment pos.

	float specular_contribution = pow( max( dot( reflected_light_direction_world, view_direction_world ), 0.0 ), uniform_specular_power ) * uniform_specular_strength;
	vec4 specular               = specular_contribution * uniform_light_color;

	out_color = uniform_color * ( ambient + diffuse + specular );
};