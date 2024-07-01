// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting.h"
#include "Engine/Graphics/Shader.hpp"
#include "Engine/Graphics/VertexArray.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/DefineMathTypes.h"

class SandboxApplication : public Engine::Application
{
	DEFINE_MATH_TYPES()

public:
	SandboxApplication();
	virtual ~SandboxApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	//virtual void Run() override;

	virtual void Render() override;

	virtual void DrawImGui() override;
	virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;

private:
	void UpdateViewMatrix( Engine::Shader& shader );
	void UpdateProjectionMatrix( Engine::Shader& shader );

	void ResetLightingData();
	Radians CalculateVerticalFieldOfView( const Radians horizontal_field_of_view ) const;

private:
	Engine::VertexArray vertex_array_crate;

	Engine::Texture container_texture_diffuse_map, container_texture_specular_map;

	Engine::Shader* cube_shader;
	Engine::Shader gouraud_shader, phong_shader;
	Engine::Shader light_source_shader;

	Vector3 camera_offset, camera_direction;
	bool camera_is_animated;

	const static constexpr int CUBE_COUNT = 10;
	const static constexpr int LIGHT_POINT_COUNT = 15;

	std::vector< Engine::Lighting::SurfaceData > cube_surface_data_array;
	Engine::Lighting::SurfaceData ground_quad_surface_data;
	Engine::Lighting::SurfaceData front_wall_quad_surface_data;
	Engine::Lighting::DirectionalLightData light_directional_data;
	std::vector< Engine::Lighting::PointLightData > light_point_data_array;
	Engine::Lighting::SpotLightData light_spot_data;
	float light_point_orbit_radius;
	bool light_point_is_animated;

	float near_plane, far_plane;
	float aspect_ratio;
	Radians vertical_field_of_view;
	bool auto_calculate_aspect_ratio;
	bool auto_calculate_vfov_based_on_90_hfov;
};