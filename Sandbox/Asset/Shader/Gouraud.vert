#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out vec3 varying_ambient_from_light;
out vec3 varying_diffuse_from_light; // Needs to be multiplied by the surface's diffuse info.
out vec3 varying_specular_from_light;

out vec2 varying_tex_coords;






layout ( std140 ) uniform TestBlock
{
	mat2x2 mat_2x2;
	mat2x3 mat_2x3;
	mat3x2 mat_3x2;
	mat3x3 mat_3x3;
	mat3x4 mat_3x4;
	mat4x2 mat_4x2;
	mat4x3 mat_4x3;
	mat4x4 mat_4x4;
};

layout ( std140 ) uniform DirectionalLightData
{
	vec3 ambient, diffuse, specular;
	vec3 direction;
} uniform_directional_light_data;

layout ( std140 ) uniform SurfaceData
{
	float shininess;
} uniform_surface_data;

uniform mat4x4 uniform_transform_world;

layout ( std140 ) uniform Essential_CameraMatrices
{
    mat4x4 ESSENTIAL_TRANSFORM_VIEW;
    mat4x4 ESSENTIAL_TRANSFORM_PROJECTION;
};

void main()
{
    mat4x4 world_view_transform             = uniform_transform_world * ESSENTIAL_TRANSFORM_VIEW;
	mat3x3 world_view_transform_for_normals = mat3x3( transpose( inverse( world_view_transform ) ) );

    vec4 position_view_space                = vec4( position, 1.0 ) * world_view_transform;
    vec4 normal_view_space                  = vec4( normalize( normal * world_view_transform_for_normals ), 0.0 );
    vec4 light_direction_view_space         = normalize( vec4( uniform_directional_light_data.direction, 0.0 ) * ESSENTIAL_TRANSFORM_VIEW );

    varying_tex_coords                      = tex_coords;
    gl_Position                             = position_view_space * ESSENTIAL_TRANSFORM_PROJECTION;

/*** Lighting Calculations ***/

	/* Sampling the lighting maps here in the vertex shader would be too extreme; for example, for a cube, 6 vertices per cube face means only 6 texels per face.
	 * Therefore, the lighting maps are sampled in the fragment shader. */

	/* Ambient term: */
	vec3 ambient = uniform_directional_light_data.ambient;

	/* Diffuse term: */
	vec4 to_light_view_space = normalize( -light_direction_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = uniform_directional_light_data.diffuse * diffuse_contribution;

	/* Specular term: */
	vec3 view_direction_view_space = normalize( -position_view_space.xyz );
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space.xyz, view_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = uniform_directional_light_data.specular * specular_contribution;

	varying_ambient_from_light  = ambient;
	varying_diffuse_from_light  = diffuse;
	varying_specular_from_light = specular;
}