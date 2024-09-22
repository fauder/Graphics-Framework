// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting/Lighting.h"
#include "Engine/Graphics/MaterialData/MaterialData.h"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Model.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/CameraController_Flight.h"

#include "Engine/DefineMathTypes.h"

// Project Includes.
#include "ModelInstance.h"

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

	virtual void RenderImGui() override;
	virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;

private:
	void UpdateViewMatrix();

	void ResetLightingData();
	void ResetMaterialData();
	void ResetCamera();
	void ResetCamera_2ndView();
	Radians CalculateVerticalFieldOfView( const Radians horizontal_field_of_view ) const;

	void ReloadModel( const std::string& file_path );
	void UnloadModel();

private:
/* Renderer: */
	Engine::Renderer renderer;

	std::vector< Engine::Drawable > light_source_drawable_array;
	std::vector< Engine::Drawable > cube_drawable_array;
	std::vector< Engine::Drawable > cube_drawable_outline_array;
	Engine::Drawable ground_quad_drawable;
	Engine::Drawable front_wall_quad_drawable;

/* Vertex Info.: */
	Engine::Mesh cube_mesh, quad_mesh;

/* Models: */
	ModelInstance test_model_instance;

/* Materials, Shaders & Textures: */
	Engine::Shader phong_shader;
	Engine::Shader basic_color_shader;
	Engine::Shader outline_shader;

	std::vector< Engine::Material > light_source_material_array;
	std::vector< Engine::Material > cube_material_array;
	Engine::Material ground_quad_material;
	Engine::Material front_wall_quad_material;

	Engine::Material outline_material;

	Engine::Texture* container_texture_diffuse_map;
	Engine::Texture* container_texture_specular_map;
	Engine::Texture* checker_pattern;

/* Scene: */
	Engine::Transform camera_transform;

	/* Lights: */
	Engine::Transform light_directional_transform;
	std::vector< Engine::Transform > light_point_transform_array;
	Engine::Transform light_spot_transform;

	/* GameObjects: */
	std::vector< Engine::Transform > cube_transform_array;
	Engine::Transform ground_quad_transform;
	Engine::Transform front_wall_quad_transform;

/* Camera: */
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
	const static constexpr int LIGHT_POINT_COUNT = 15;

	std::vector< Engine::MaterialData::PhongMaterialData > cube_surface_data_array;
	Engine::MaterialData::PhongMaterialData ground_quad_surface_data;
	Engine::MaterialData::PhongMaterialData front_wall_quad_surface_data;
	std::vector< Engine::MaterialData::PhongMaterialData > test_model_node_surface_data_array;

	Engine::DirectionalLight light_directional;
	std::vector< Engine::PointLight > light_point_array;
	Engine::SpotLight light_spot;

	bool light_is_enabled;
	bool light_point_array_disable;
	bool light_point_array_is_animated;
	float light_point_orbit_radius;

	bool light_spot_array_disable;

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
	bool show_imgui_demo_window;

	std::string test_model_file_path;
};