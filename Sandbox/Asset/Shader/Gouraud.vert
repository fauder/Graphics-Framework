#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec4 varying_lighting_color;

uniform mat4x4 uniform_transform_world;
uniform mat4x4 uniform_transform_view;
uniform mat4x4 uniform_transform_projection;

uniform vec4 uniform_light_color;

uniform float uniform_ambient_strength;
uniform float uniform_diffuse_strength;
uniform float uniform_specular_strength;
uniform float uniform_specular_power;

uniform vec3 uniform_light_position_world_space;

void main()
{
    mat4x4 world_view_transform       = uniform_transform_world * uniform_transform_view;
    vec4 position_view_space          = vec4( position, 1.0 ) * world_view_transform;
    vec4 normal_view_space            = vec4( normalize( normal * mat3x3( transpose( inverse( world_view_transform ) ) ) ), 0.0 );
    vec4 light_position_view_space    = vec4( uniform_light_position_world_space, 1.0 ) * uniform_transform_view;
    gl_Position                       = position_view_space * uniform_transform_projection;

/*** Lighting Calculations ***/

	/* Ambient term: */
	vec4 ambient = uniform_ambient_strength * uniform_light_color;

	/* Diffuse term: */
	vec4 to_light_view_space = normalize( light_position_view_space - position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 ) * uniform_diffuse_strength;
	vec4 diffuse               = diffuse_contribution * uniform_light_color;

	/* Specular term: */
	vec4 view_direction_view_space = vec4( 0.0, 0.0, 0.0, 1.0 ) - position_view_space;
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, view_direction_view_space ), 0.0 ), uniform_specular_power ) * uniform_specular_strength;
	vec4 specular               = specular_contribution * uniform_light_color;

	varying_lighting_color = ambient + diffuse + specular;
}