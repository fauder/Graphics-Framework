#version 330 core

in vec4 varying_position_view_space;
in vec4 varying_normal_view_space;
in vec2 varying_tex_coords;

out vec4 out_color;

struct DirectionalLightData
{
	vec3 ambient, diffuse, specular;
	vec3 direction_view_space;
};

struct PointLightData
{
	vec3 ambient, diffuse, specular;
	vec3 position_view_space;

	float attenuation_constant, attenuation_linear, attenuation_quadratic;
};

struct SpotLightData
{
	vec3 ambient, diffuse, specular;
	vec3 position_view_space, direction_view_space;
	float cos_cutoff_angle_inner, cos_cutoff_angle_outer;
};

struct SurfaceData
{
	sampler2D diffuse_map_slot, specular_map_slot;
	float shininess;
};

uniform DirectionalLightData	uniform_directional_light_data;
uniform PointLightData			uniform_point_light_data;
uniform SpotLightData			uniform_spot_light_data;

uniform SurfaceData				uniform_surface_data;

vec3 CalculateColorFromDirectionalLight( DirectionalLightData directional_light_data, 
										 vec4 normal_view_space, vec4 viewing_direction_view_space,
										 vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * directional_light_data.ambient;

/* Diffuse term: */
	vec4 to_light_view_space = normalize( vec4( -directional_light_data.direction_view_space, 0.0 ) );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * directional_light_data.diffuse * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = specular_sample * directional_light_data.specular * specular_contribution;

	return vec3( ambient + diffuse + specular );
}

float CalculateAttenuation( PointLightData point_light_data, float distance )
{
	return 1.0f / ( point_light_data.attenuation_constant  +
					point_light_data.attenuation_linear    * distance + 
					point_light_data.attenuation_quadratic * distance * distance );
}

vec3 CalculateColorFromPointLight( PointLightData point_light_data, 
								   vec4 normal_view_space, vec4 viewing_direction_view_space,
								   vec4 fragment_position_view_space,
								   vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * point_light_data.ambient;

/* Diffuse term: */
	vec4 to_light_view_space = normalize( vec4( point_light_data.position_view_space, 1.0 ) - varying_position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * point_light_data.diffuse * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = specular_sample * point_light_data.specular * specular_contribution;

/* Attenuation: */
	float distance_view_space = distance( fragment_position_view_space.xyz, point_light_data.position_view_space );
	float attenuation         = CalculateAttenuation( point_light_data, distance_view_space );

	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return vec3( ambient + diffuse + specular );
}

vec3 CalculateColorFromSpotLight( SpotLightData spot_light_data, 
							      vec4 normal_view_space, vec4 viewing_direction_view_space,
							      vec3 diffuse_sample, vec3 specular_sample )
{
	vec4 to_light_view_space   = normalize( vec4( spot_light_data.position_view_space, 1.0 ) - varying_position_view_space );
	vec4 from_light_view_space = -to_light_view_space;

	bool fragment_is_inside_cone = dot( from_light_view_space.xyz, spot_light_data.direction_view_space ) > spot_light_data.cos_cutoff_angle_inner;

/* Ambient term: */
	vec3 ambient = diffuse_sample * spot_light_data.ambient;

/* Diffuse term: */
	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * spot_light_data.diffuse * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( from_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = specular_sample * spot_light_data.specular * specular_contribution;

	return vec3( ambient + int( fragment_is_inside_cone ) * ( diffuse + specular ) );
}

void main()
{
	vec4 normal_view_space = normalize( varying_normal_view_space );
	// No need to subtract from the camera position since the camera is positioned at the origin in view space.
	vec4 viewing_direction_view_space = normalize( -varying_position_view_space ); 

	vec3 diffuse_sample  = vec3( texture( uniform_surface_data.diffuse_map_slot,  varying_tex_coords ) );
	vec3 specular_sample = vec3( texture( uniform_surface_data.specular_map_slot, varying_tex_coords ) );

	vec3 from_directional_light = CalculateColorFromDirectionalLight( uniform_directional_light_data, 
																	  normal_view_space, viewing_direction_view_space,
																	  diffuse_sample, specular_sample );

	vec3 from_point_light = CalculateColorFromPointLight( uniform_point_light_data, 
														  normal_view_space, viewing_direction_view_space,
														  varying_position_view_space,
														  diffuse_sample, specular_sample );

	vec3 from_spot_light = CalculateColorFromSpotLight( uniform_spot_light_data, 
													    normal_view_space, viewing_direction_view_space,
													    diffuse_sample, specular_sample );

	out_color = vec4( from_directional_light + from_point_light + from_spot_light, 1.0 );
};