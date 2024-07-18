// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting.h"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.hpp"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/CameraController_Flight.h"

#include "Engine/DefineMathTypes.h"

// std Includes.
#include <numeric> // std::accumulate().

class SandboxApplication : public Engine::Application
{
	DEFINE_MATH_TYPES()

public:
	SandboxApplication();
	virtual ~SandboxApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	//virtual void Run() override;

	virtual void Update() override;

	virtual void Render() override;

	virtual void DrawImGui() override;
	virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;

private:
	void UpdateViewMatrix();

	void ResetLightingData();
	void ResetMaterialData();
	Radians CalculateVerticalFieldOfView( const Radians horizontal_field_of_view ) const;

private:
/* Renderer: */
	Engine::Renderer renderer;

	std::vector< Engine::Drawable > light_source_drawable_array;
	std::vector< Engine::Drawable > cube_drawable_array;
	Engine::Drawable ground_quad_drawable;
	Engine::Drawable front_wall_quad_drawable;

/* Vertex Info.: */
	Engine::Mesh cube_mesh;

/* Materials, Shaders & Textures: */
	Engine::Shader* cube_shader;
	Engine::Shader gouraud_shader, phong_shader;
	Engine::Shader light_source_shader;

	std::vector< Engine::Material > light_source_material_array;
	std::vector< Engine::Material > cube_material_array;
	Engine::Material ground_quad_material;
	Engine::Material front_wall_quad_material;

	Engine::Texture container_texture_diffuse_map, container_texture_specular_map;

/* Scene: */
	std::vector< Engine::Transform > light_source_transform_array;
	std::vector< Engine::Transform > cube_transform_array;
	Engine::Transform ground_quad_transform;
	Engine::Transform front_wall_quad_transform;

/* Camera: */
	Engine::Transform camera_transform;
	Engine::Camera camera;
	float camera_rotation_speed;
	float camera_move_speed;
	Engine::CameraController_Flight camera_controller;

	bool camera_is_animated;

	Matrix4x4 view_transformation;

/* Projection: */
	bool auto_calculate_aspect_ratio;
	bool auto_calculate_vfov_based_on_90_hfov;

/* Lighting: */
	const static constexpr int LIGHT_POINT_COUNT = 1;

	std::vector< Engine::Lighting::SurfaceData > cube_surface_data_array;
	Engine::Lighting::SurfaceData ground_quad_surface_data;
	Engine::Lighting::SurfaceData front_wall_quad_surface_data;

	Engine::Lighting::DirectionalLightData light_directional_data;
	std::vector< Engine::Lighting::PointLightData > light_point_data_array;
	Engine::Lighting::SpotLightData light_spot_data;

	float light_point_orbit_radius;
	bool light_point_is_animated;

/* Frame statistics: */
	Radians current_time_as_angle;

/* Other: */
	const static constexpr int CUBE_COUNT = 10;

	const static constexpr std::array< Vector3, CUBE_COUNT > CUBE_POSITIONS =
	{ {
		{  0.0f,  0.0f,  0.0f	},
		{  2.0f,  5.0f, +15.0f	},
		{ -1.5f, -2.2f, +2.5f	},
		{ -3.8f, -2.0f, +12.3f	},
		{  2.4f, -0.4f, +3.5f	},
		{ -1.7f,  3.0f, +7.5f	},
		{  1.3f, -2.0f, +2.5f	},
		{  1.5f,  2.0f, +2.5f	},
		{  1.5f,  0.2f, +1.5f	},
		{ -1.3f,  1.0f, +1.5f	}
	} };

	const static constexpr Vector3 CUBES_ORIGIN = std::accumulate( CUBE_POSITIONS.cbegin(), CUBE_POSITIONS.cend(), Vector3::Zero() ) / CUBE_COUNT;

	bool ui_interaction_enabled;
};