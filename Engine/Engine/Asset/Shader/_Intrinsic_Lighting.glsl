#define POINT_LIGHT_MAX_COUNT 15
#define SPOT_LIGHT_MAX_COUNT 15

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
float padding;
	uint					_INTRINSIC_DIRECTIONAL_LIGHT_IS_ACTIVE;
	uint					_INTRINSIC_POINT_LIGHT_ACTIVE_COUNT;
	uint					_INTRINSIC_SPOT_LIGHT_ACTIVE_COUNT;

	DirectionalLightData	_INTRINSIC_DIRECTIONAL_LIGHT;
	PointLightData			_INTRINSIC_POINT_LIGHTS[ POINT_LIGHT_MAX_COUNT ];
	SpotLightData			_INTRINSIC_SPOT_LIGHTS[ SPOT_LIGHT_MAX_COUNT ];
};