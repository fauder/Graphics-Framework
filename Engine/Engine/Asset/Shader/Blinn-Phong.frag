#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Lighting.glsl"

#pragma feature SKYBOX_ENVIRONMENT_MAPPING
#pragma feature SHADOWS_ENABLED

in VS_To_FS
{
	vec4 varying_position_view_space;
    vec4 varying_normal_view_space;
    vec2 varying_tex_coords;
#ifdef SHADOWS_ENABLED
    vec4 varying_position_light_directional_clip_space;
#endif
} fs_in;

out vec4 out_color;

layout ( std140 ) uniform BlinnPhongMaterialData
{
/* These 2 are combined into a vec4 on cpu side. */
	vec3 color_diffuse;
	bool has_texture_diffuse;

	float shininess;

	vec3 padding;
} uniform_blinn_phong_material_data;

uniform sampler2D uniform_diffuse_map_slot, uniform_specular_map_slot;

#ifdef SKYBOX_ENVIRONMENT_MAPPING
uniform samplerCube uniform_texture_skybox_slot;
uniform sampler2D uniform_reflection_map_slot;
uniform float uniform_reflectivity; /* _hint_normalized_percentage */
#endif

#ifdef SHADOWS_ENABLED
uniform sampler2D uniform_shadow_map_slot;

/* Returns either 1 = in-shadow or 0 = not in shadow. */
float CalculateShadowAmount()
{
	vec3 ndc       = fs_in.varying_position_light_directional_clip_space.xyz / fs_in.varying_position_light_directional_clip_space.w;
	vec3 ndc_unorm = ndc * 0.5f + 0.5f;

	float shadow_map_sample_z = texture( uniform_shadow_map_slot, ndc_unorm.xy ).r;
	float current_depth       = ndc_unorm.z;

	return current_depth > shadow_map_sample_z ? 1.0f : 0.0f;
}
#endif

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
	vec4 halfway_angle_view_space = normalize( to_light_view_space + viewing_direction_view_space );

	float specular_contribution = pow( max( dot( halfway_angle_view_space, normal_view_space ), 0.0 ), uniform_blinn_phong_material_data.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_DIRECTIONAL_LIGHT.specular.rgb * specular_contribution;

#ifdef SHADOWS_ENABLED
	float shadow = CalculateShadowAmount();
	return ambient + ( 1.0f - shadow ) * ( diffuse + specular );
#else
	return ambient + diffuse + specular;
#endif
}

float CalculateAttenuation( const int point_light_index, float distance )
{
	return 1.0f / ( _INTRINSIC_POINT_LIGHTS[ point_light_index ].ambient_and_attenuation_constant.w +
					_INTRINSIC_POINT_LIGHTS[ point_light_index ].diffuse_and_attenuation_linear.w	  * distance + 
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
	vec4 to_light_view_space = normalize( vec4( _INTRINSIC_POINT_LIGHTS[ point_light_index ].position_view_space.xyz, 1.0 ) - fs_in.varying_position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0 );
	vec3 diffuse               = diffuse_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].diffuse_and_attenuation_linear.rgb * diffuse_contribution;

/* Specular term: */
	vec4 halfway_angle_view_space = normalize( to_light_view_space + viewing_direction_view_space );

	float specular_contribution = pow( max( dot( halfway_angle_view_space, normal_view_space ), 0.0 ), uniform_blinn_phong_material_data.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].specular_and_attenuation_quadratic.rgb * specular_contribution;

/* Attenuation: */
	float distance_view_space = distance( fs_in.varying_position_view_space.xyz, _INTRINSIC_POINT_LIGHTS[ point_light_index ].position_view_space.xyz );
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

	vec4 to_light_view_space   = normalize( vec4( position_view_space, 1.0 ) - fs_in.varying_position_view_space );
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
	vec4 halfway_angle_view_space = normalize( to_light_view_space + viewing_direction_view_space );

	float specular_contribution = pow( max( dot( halfway_angle_view_space, normal_view_space ), 0.0 ), uniform_blinn_phong_material_data.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].specular.rgb * specular_contribution;

	return ambient + cut_off_intensity * ( diffuse + specular );
}

void main()
{
	vec4 normal_view_space = normalize( fs_in.varying_normal_view_space );
	// No need to subtract from the camera position since the camera is positioned at the origin in view space.
	vec4 viewing_direction_view_space = normalize( -fs_in.varying_position_view_space ); 

	vec3 diffuse_sample  = uniform_blinn_phong_material_data.has_texture_diffuse
							? vec3( texture( uniform_diffuse_map_slot,  fs_in.varying_tex_coords ) )
							: uniform_blinn_phong_material_data.color_diffuse;
	vec3 specular_sample = vec3( texture( uniform_specular_map_slot, fs_in.varying_tex_coords ) );

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

#ifdef SKYBOX_ENVIRONMENT_MAPPING
	vec4 reflected         = reflect( fs_in.varying_position_view_space, normal_view_space );
	vec4 reflection_sample = vec4( texture( uniform_texture_skybox_slot, reflected.xyz ).rgb, 1.0 );

	vec4 reflection_map_sample = vec4( texture( uniform_reflection_map_slot, fs_in.varying_tex_coords ).rgb, 1.0 );

	out_color = mix( out_color, ( 1.0 - reflection_map_sample.r ) * reflection_sample, uniform_reflectivity );
#endif
}