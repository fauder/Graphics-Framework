#version 330 core

in vec4 varying_position_view_space;
in vec4 varying_normal_view_space;
in vec2 varying_tex_coords;

out vec4 out_color;

#define POINT_LIGHT_COUNT 1

layout ( std140 ) uniform DirectionalLightData
{
	vec3 ambient, diffuse, specular;
	vec3 direction_view_space;
} uniform_directional_light_data;

struct PointLightDataStruct
{
	vec4 ambient_and_attenuation_constant, diffuse_and_attenuation_linear, specular_and_attenuation_quadratic;
	vec3 position_view_space;
};

layout ( std140 ) uniform PointLightData
{
	PointLightDataStruct point_light_data[ POINT_LIGHT_COUNT ];
} uniform_point_light_data;

layout ( std140 ) uniform SpotLightData
{
	vec3 ambient, diffuse, specular;
	vec4 position_view_space_and_cos_cutoff_angle_inner, direction_view_space_and_cos_cutoff_angle_outer;
} uniform_spot_light_data;

layout ( std140 ) uniform SurfaceData
{
	float shininess;
} uniform_surface_data;

uniform sampler2D uniform_surface_data_diffuse_map_slot, uniform_surface_data_specular_map_slot;


vec3 CalculateColorFromDirectionalLight( vec4 normal_view_space, vec4 viewing_direction_view_space,
										 vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * uniform_directional_light_data.ambient;

/* Diffuse term: */
	vec4 to_light_view_space = normalize( vec4( -uniform_directional_light_data.direction_view_space, 0.0 ) );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * uniform_directional_light_data.diffuse * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = specular_sample * uniform_directional_light_data.specular * specular_contribution;

	return vec3( ambient + diffuse + specular );
}

float CalculateAttenuation( const int point_light_index, float distance )
{
	return 1.0f / ( uniform_point_light_data.point_light_data[ point_light_index ].ambient_and_attenuation_constant.w +
					uniform_point_light_data.point_light_data[ point_light_index ].diffuse_and_attenuation_linear.w		* distance + 
					uniform_point_light_data.point_light_data[ point_light_index ].specular_and_attenuation_quadratic.w * distance * distance );
}

vec3 CalculateColorFromPointLight( const int point_light_index, 
								   vec4 normal_view_space, vec4 viewing_direction_view_space,
								   vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * uniform_point_light_data.point_light_data[ point_light_index ].ambient_and_attenuation_constant.rgb;

/* Diffuse term: */
	vec4 to_light_view_space = normalize( vec4( uniform_point_light_data.point_light_data[ point_light_index ].position_view_space, 1.0 ) - varying_position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * uniform_point_light_data.point_light_data[ point_light_index ].diffuse_and_attenuation_linear.rgb * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = specular_sample * uniform_point_light_data.point_light_data[ point_light_index ].specular_and_attenuation_quadratic.rgb * specular_contribution;

/* Attenuation: */
	float distance_view_space = distance( varying_position_view_space.xyz, uniform_point_light_data.point_light_data[ point_light_index ].position_view_space );
	float attenuation         = CalculateAttenuation( point_light_index, distance_view_space );

	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return vec3( ambient + diffuse + specular );
}

vec3 CalculateColorFromSpotLight( vec4 normal_view_space, vec4 viewing_direction_view_space,
							      vec3 diffuse_sample, vec3 specular_sample )
{
	vec3 position_view_space  = uniform_spot_light_data.position_view_space_and_cos_cutoff_angle_inner.xyz;
	vec3 direction_view_space = uniform_spot_light_data.direction_view_space_and_cos_cutoff_angle_outer.xyz;

	float cos_cutoff_angle_inner = uniform_spot_light_data.position_view_space_and_cos_cutoff_angle_inner.w;
	float cos_cutoff_angle_outer = uniform_spot_light_data.direction_view_space_and_cos_cutoff_angle_outer.w;

	vec4 to_light_view_space   = normalize( vec4( position_view_space, 1.0 ) - varying_position_view_space );
	vec4 from_light_view_space = -to_light_view_space;

	float cut_off_intensity	= clamp( ( dot( from_light_view_space.xyz, direction_view_space ) - cos_cutoff_angle_outer ) /
									 ( cos_cutoff_angle_inner - cos_cutoff_angle_outer ),
									 0, 1 );

/* Ambient term: */
	vec3 ambient = diffuse_sample * uniform_spot_light_data.ambient;

/* Diffuse term: */
	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * uniform_spot_light_data.diffuse * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( from_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface_data.shininess );
	vec3 specular               = specular_sample * uniform_spot_light_data.specular * specular_contribution;

	return vec3( ambient + cut_off_intensity * ( diffuse + specular ) );
}

void main()
{
	vec4 normal_view_space = normalize( varying_normal_view_space );
	// No need to subtract from the camera position since the camera is positioned at the origin in view space.
	vec4 viewing_direction_view_space = normalize( -varying_position_view_space ); 

	vec3 diffuse_sample  = vec3( texture( uniform_surface_data_diffuse_map_slot,  varying_tex_coords ) );
	vec3 specular_sample = vec3( texture( uniform_surface_data_specular_map_slot, varying_tex_coords ) );

	vec3 from_directional_light = CalculateColorFromDirectionalLight( normal_view_space, viewing_direction_view_space,
																	  diffuse_sample, specular_sample );

	vec3 from_point_light = vec3( 0 );																		
	for( int i = 0; i < POINT_LIGHT_COUNT; i++ )
	{
		from_point_light += CalculateColorFromPointLight( i, 
														  normal_view_space, viewing_direction_view_space,
														  diffuse_sample, specular_sample );
	}

	vec3 from_spot_light = CalculateColorFromSpotLight( normal_view_space, viewing_direction_view_space,
													    diffuse_sample, specular_sample );

	out_color = vec4( from_directional_light + from_point_light + from_spot_light, 1.0 );
}