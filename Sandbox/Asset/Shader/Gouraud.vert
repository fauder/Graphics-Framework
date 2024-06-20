#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 varying_lighting_ambient;
out vec3 varying_lighting_diffuse;
out vec3 varying_lighting_specular;

struct LightData
{
	vec3 ambient, diffuse, specular;
	vec3 position;
};

struct SurfaceData
{
	vec3 ambient, diffuse, specular;
	float shininess, debugging;
};

uniform LightData uniform_light_data;
uniform SurfaceData uniform_surface_data;

uniform mat4x4 uniform_transform_world;
uniform mat4x4 uniform_transform_view;
uniform mat4x4 uniform_transform_projection;

void main()
{
    mat4x4 world_view_transform       = uniform_transform_world * uniform_transform_view;
    vec4 position_view_space          = vec4( position, 1.0 ) * world_view_transform;
    vec4 normal_view_space            = vec4( normalize( normal * mat3x3( transpose( inverse( world_view_transform ) ) ) ), 0.0 );
    vec4 light_position_view_space    = vec4( uniform_light_data.position, 1.0 ) * uniform_transform_view;
    gl_Position                       = position_view_space * uniform_transform_projection;

/*** Lighting Calculations ***/

	/* Ambient term: */
	vec3 ambient = uniform_surface_data.ambient * uniform_light_data.ambient;

	/* Diffuse term: */
	vec4 to_light_view_space = normalize( light_position_view_space - position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = uniform_surface_data.diffuse * uniform_light_data.diffuse * diffuse_contribution;

	/* Specular term: */
	vec3 view_direction_view_space = normalize( -position_view_space.xyz );
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space.xyz, view_direction_view_space ), 0.0 ), uniform_surface_data.shininess * uniform_surface_data.debugging );
	vec3 specular               = uniform_surface_data.specular * uniform_light_data.specular * specular_contribution;

	varying_lighting_ambient  = ambient;
	varying_lighting_diffuse  = diffuse;
	varying_lighting_specular = specular;
}