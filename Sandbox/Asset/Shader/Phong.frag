#version 460 core

in vec4 varying_position_view_space;
in vec4 varying_normal_view_space;
in vec2 varying_tex_coords;

out vec4 out_color;

#define POINT_LIGHT_COUNT 15
#define SPOT_LIGHT_COUNT 1

struct DirectionalLightData
{
	vec4 ambient, diffuse, specular;
	vec4 direction_view_space;
};

struct PointLightData
{
	vec4 ambient_and_attenuation_constant, diffuse_and_attenuation_linear, specular_and_attenuation_quadratic;
	vec4 position_view_space;
};

struct SpotLightData
{
	vec4 ambient, diffuse, specular;
	vec4 position_view_space_and_cos_cutoff_angle_inner, direction_view_space_and_cos_cutoff_angle_outer;
};

layout ( std140 ) uniform _Intrinsic_Lighting
{
	DirectionalLightData	_INTRINSIC_DIRECTIONAL_LIGHT;
	PointLightData			_INTRINSIC_POINT_LIGHTS[ POINT_LIGHT_COUNT ];
	SpotLightData			_INTRINSIC_SPOT_LIGHTS[ SPOT_LIGHT_COUNT ];
	uint					_INTRINSIC_DIRECTIONAL_LIGHT_IS_ACTIVE;
	uint					_INTRINSIC_POINT_LIGHT_ACTIVE_COUNT;
	uint					_INTRINSIC_SPOT_LIGHT_ACTIVE_COUNT;
};

layout ( std140 ) uniform SurfaceData
{
	float shininess;
} uniform_surface;

uniform sampler2D uniform_surface_diffuse_map_slot, uniform_surface_specular_map_slot;


vec3 CalculateColorFromDirectionalLight( vec4 normal_view_space, vec4 viewing_direction_view_space,
										 vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * _INTRINSIC_DIRECTIONAL_LIGHT.ambient.rgb;

/* Diffuse term: */
	vec4 to_light_view_space = normalize( -_INTRINSIC_DIRECTIONAL_LIGHT.direction_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * _INTRINSIC_DIRECTIONAL_LIGHT.diffuse.rgb * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_DIRECTIONAL_LIGHT.specular.rgb * specular_contribution;

	return vec3( ambient + diffuse + specular );
}

float CalculateAttenuation( const int point_light_index, float distance )
{
	return 1.0f / ( _INTRINSIC_POINT_LIGHTS[ point_light_index ].ambient_and_attenuation_constant.w +
					_INTRINSIC_POINT_LIGHTS[ point_light_index ].diffuse_and_attenuation_linear.w		* distance + 
					_INTRINSIC_POINT_LIGHTS[ point_light_index ].specular_and_attenuation_quadratic.w * distance * distance );
}

vec3 CalculateColorFromPointLight( const int point_light_index, 
								   vec4 normal_view_space, vec4 viewing_direction_view_space,
								   vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].ambient_and_attenuation_constant.rgb;

/* Diffuse term: */
	// Uniform point light data passed is passed as a vector3 (padded to vector4); w component is not sent by the CPU side, so it can be any value. Set to 1.0 here to make sure.
	vec4 to_light_view_space = normalize( vec4( _INTRINSIC_POINT_LIGHTS[ point_light_index ].position_view_space.xyz, 1.0 ) - varying_position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].diffuse_and_attenuation_linear.rgb * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( -to_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].specular_and_attenuation_quadratic.rgb * specular_contribution;

/* Attenuation: */
	float distance_view_space = distance( varying_position_view_space.xyz, _INTRINSIC_POINT_LIGHTS[ point_light_index ].position_view_space.xyz );
	float attenuation         = CalculateAttenuation( point_light_index, distance_view_space );

	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return vec3( ambient + diffuse + specular );
}

vec3 CalculateColorFromSpotLight( const int spot_light_index, 
								  vec4 normal_view_space, vec4 viewing_direction_view_space,
							      vec3 diffuse_sample, vec3 specular_sample )
{
	vec3 position_view_space  = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].position_view_space_and_cos_cutoff_angle_inner.xyz;
	vec3 direction_view_space = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].direction_view_space_and_cos_cutoff_angle_outer.xyz;

	float cos_cutoff_angle_inner = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].position_view_space_and_cos_cutoff_angle_inner.w;
	float cos_cutoff_angle_outer = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].direction_view_space_and_cos_cutoff_angle_outer.w;

	vec4 to_light_view_space   = normalize( vec4( position_view_space, 1.0 ) - varying_position_view_space );
	vec4 from_light_view_space = -to_light_view_space;

	float cut_off_intensity	= clamp( ( dot( from_light_view_space.xyz, direction_view_space ) - cos_cutoff_angle_outer ) /
									 ( cos_cutoff_angle_inner - cos_cutoff_angle_outer ),
									 0, 1 );

/* Ambient term: */
	vec3 ambient = diffuse_sample * _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].ambient.rgb;

/* Diffuse term: */
	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].diffuse.rgb * diffuse_contribution;

/* Specular term: */
	// reflect() expects the first argument to be the vector FROM the light source to the fragment pos.
	vec4 reflected_light_direction_view_space = reflect( from_light_view_space, normal_view_space );

	float specular_contribution = pow( max( dot( reflected_light_direction_view_space, viewing_direction_view_space ), 0.0 ), uniform_surface.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].specular.rgb * specular_contribution;

	return vec3( ambient + cut_off_intensity * ( diffuse + specular ) );
}

void main()
{
	vec4 normal_view_space = normalize( varying_normal_view_space );
	// No need to subtract from the camera position since the camera is positioned at the origin in view space.
	vec4 viewing_direction_view_space = normalize( -varying_position_view_space ); 

	vec3 diffuse_sample  = vec3( texture( uniform_surface_diffuse_map_slot,  varying_tex_coords ) );
	vec3 specular_sample = vec3( texture( uniform_surface_specular_map_slot, varying_tex_coords ) );

	vec3 from_directional_light = _INTRINSIC_DIRECTIONAL_LIGHT_IS_ACTIVE * 
									CalculateColorFromDirectionalLight( normal_view_space, viewing_direction_view_space,
																		diffuse_sample, specular_sample );

	vec3 from_point_light = vec3( 0 );																		
	for( int i = 0; i < _INTRINSIC_POINT_LIGHT_ACTIVE_COUNT; i++ )
		from_point_light += CalculateColorFromPointLight( i, 
														  normal_view_space, viewing_direction_view_space,
														  diffuse_sample, specular_sample );

	vec3 from_spot_light = vec3( 0 );																		
	for( int i = 0; i < _INTRINSIC_SPOT_LIGHT_ACTIVE_COUNT; i++ )
		from_spot_light += CalculateColorFromSpotLight( i,
														normal_view_space, viewing_direction_view_space,
														diffuse_sample, specular_sample );

	out_color = vec4( from_directional_light + from_point_light + from_spot_light, 1.0 );
}