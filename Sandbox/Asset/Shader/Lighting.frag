#version 330 core

in vec4 varying_position_world;
in vec3 varying_normal;

out vec4 out_color;

uniform vec4 uniform_color;
uniform vec4 uniform_light_color;

uniform float uniform_ambient_strength;
uniform float uniform_diffuse_strength;

uniform vec3 uniform_light_position;

void main()
{
	vec4 ambient = uniform_ambient_strength * uniform_light_color;

	vec4 normal   = vec4( normalize( varying_normal ), 0.0f );
	vec4 to_light = normalize( vec4( uniform_light_position, 1.0 ) - varying_position_world );

	float diffuse_contribution = max( dot( to_light, normal ), 0.0 ) * uniform_diffuse_strength;

	vec4 diffuse = diffuse_contribution * uniform_light_color;

	out_color = uniform_color * ( ambient + diffuse );
};