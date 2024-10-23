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
#include "KernelData.h"
#include "ModelInstance.h"

// std Includes.
#include <numeric> // std::accumulate().

class SandboxApplication : public Engine::Application
{
	DEFINE_MATH_TYPES()

	enum class CameraView
	{
		FRONT,
		BACK,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,

		CUSTOM_1,
	};

	struct ModelInfo
	{
		ModelInstance model_instance;
		Engine::Shader* shader;
		std::string file_path;
	};

public:
	SandboxApplication( const Engine::BitFlags< Engine::CreationFlags > );
	virtual ~SandboxApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	//virtual void Run() override;

	virtual void Update() override;

	virtual void Render() override;

	virtual void RenderImGui() override;
	virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;
	void OnFramebufferResizeEvent( const Vector2I new_size_pixels ); // Convenience overload.

private:
	void RenderImGui_Viewport();

	void ResetLightingData();
	void ResetMaterialData();
	void ResetInstanceData();
	void ResetCamera();
	void ResetProjection();
	void SwitchCameraView( const CameraView view );
	Radians CalculateVerticalFieldOfView( const Radians horizontal_field_of_view ) const;

	bool ReloadModel( ModelInfo& model_info_to_be_loaded, const std::string& file_path, const char* name );
	void UnloadModel( ModelInfo& model_info_to_be_unloaded );

	void ReplaceMeteoriteAndCubeDrawables( bool use_meteorites );

	void InitializeFramebufferTextures( const int width_new_pixels, const int height_new_pixels );
	void InitializeRenderbuffers( const int width_new_pixels, const int height_new_pixels );
	void InitializeFramebuffers( const int width_new_pixels, const int height_new_pixels );

	void RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels );
	void RecalculateProjectionParameters( const Vector2I new_size_pixels ); // Convenience overload.
	void RecalculateProjectionParameters(); // Utilizes current framebuffer size.

private:
/* Renderer: */
	Engine::Renderer renderer;

	Engine::Drawable light_sources_drawable;

	const static constexpr int CUBE_COUNT           = 200'000;
	const static constexpr int CUBE_REFLECTED_COUNT = 10;
	Engine::Drawable cube_drawable;
	Engine::Drawable cube_drawable_outline;

	Engine::Drawable cube_reflected_drawable;

	Engine::Drawable* meteorite_drawable;

	Engine::Drawable skybox_drawable;

	Engine::Drawable ground_drawable;
	Engine::Drawable wall_front_drawable;
	Engine::Drawable wall_left_drawable;
	Engine::Drawable wall_right_drawable;
	Engine::Drawable wall_back_drawable;

	const static constexpr int WINDOW_COUNT = 5;
	std::array< Engine::Drawable, WINDOW_COUNT > window_drawable_array;

	Engine::Drawable offscreen_quad_drawable;

	Engine::Drawable mirror_quad_drawable;

	const Engine::Renderer::RenderGroupID render_group_id_skybox;
	const Engine::Renderer::RenderGroupID render_group_id_regular;
	const Engine::Renderer::RenderGroupID render_group_id_outlined_mesh;
	const Engine::Renderer::RenderGroupID render_group_id_outline;
	const Engine::Renderer::RenderGroupID render_group_id_transparent;
	const Engine::Renderer::RenderGroupID render_group_id_screen_size_quad;

/* Textures: */
	Engine::Texture* skybox_texture;

	std::array< Engine::Texture*, 2 > offscreen_framebuffer_color_attachment_array;
	Engine::Texture* editor_framebuffer_color_attachment;

	Engine::Texture* container_texture_diffuse_map;
	Engine::Texture* container_texture_specular_map;
	Engine::Texture* checker_pattern_texture;
	Engine::Texture* transparent_window_texture;

/* Renderbuffers: */
	std::array< Engine::Renderbuffer, 2 > offscreen_framebuffer_depth_and_stencil_attachment_array;
	Engine::Renderbuffer editor_framebuffer_depth_and_stencil_attachment;

/* Framebuffers: */
	std::array< Engine::Framebuffer, 2 > offscreen_framebuffer_array;
	Engine::Framebuffer editor_framebuffer; // Not to be confused with the default frame-buffer.

/* Vertex Info.: */
	Engine::Mesh cube_mesh, cube_mesh_fullscreen, quad_mesh, quad_mesh_uvs_only, quad_mesh_fullscreen, quad_mesh_mirror;
	Engine::Mesh cube_mesh_instanced;
	Engine::Mesh cube_reflected_mesh_instanced;
	Engine::Mesh cube_mesh_instanced_with_color; // For light sources.

/* Shaders: */
	Engine::Shader skybox_shader;
	Engine::Shader phong_shader;
	Engine::Shader phong_shader_instanced;
	Engine::Shader phong_skybox_reflection_shader;
	Engine::Shader phong_skybox_reflection_shader_instanced;
	Engine::Shader basic_color_shader;
	Engine::Shader basic_textured_shader;
	Engine::Shader basic_textured_transparent_discard_shader;
	Engine::Shader outline_shader;

	Engine::Shader fullscreen_blit_shader;

	Engine::Shader postprocess_grayscale_shader;

	Engine::Shader postprocess_generic_shader;

	Engine::Shader normal_visualization_shader;

/* Models: */
	ModelInfo test_model_info;
	ModelInfo meteorite_model_info;

/* Materials: */
	Engine::Material skybox_material;

	Engine::Material light_source_material;

	Engine::Material cube_material;
	Engine::Material cube_reflected_material;

	Engine::Material ground_material;
	Engine::Material wall_material;

	Engine::Material window_material;

	Engine::Material outline_material;

	Engine::Material offscreen_quad_material;
	Engine::Material mirror_quad_material;

/* Instancing Data: */

	struct LightInstanceData
	{
		Matrix4x4 transform;
		Engine::Color4 color;
	};

	std::vector< Matrix4x4 > cube_instance_data_array;
	std::vector< Matrix4x4 > cube_reflected_instance_data_array;
	std::vector< LightInstanceData > light_source_instance_data_array;

/* Scene: */
	Engine::Transform camera_transform;

	/* Lights: */
	Engine::Transform light_directional_transform;
	std::vector< Engine::Transform > light_point_transform_array;
	Engine::Transform light_spot_transform;

	/* GameObjects: */
	std::vector< Engine::Transform > cube_transform_array;
	std::vector< Engine::Transform > cube_reflected_transform_array;

	Engine::Transform ground_transform;
	Engine::Transform wall_front_transform;
	Engine::Transform wall_left_transform;
	Engine::Transform wall_right_transform;
	Engine::Transform wall_back_transform;

	std::array< Engine::Transform, WINDOW_COUNT > window_transform_array;

/* Camera: */
	Engine::Camera camera;
	float camera_rotation_speed;
	float camera_move_speed;
	Engine::CameraController_Flight camera_controller;

	bool camera_animation_is_enabled;
	float camera_animation_orbit_radius;

	Matrix4x4 view_transformation;

/* Lighting: */
	const static constexpr int LIGHT_POINT_COUNT = 15;

	Engine::MaterialData::PhongMaterialData cube_surface_data;
	Engine::MaterialData::PhongMaterialData ground_quad_surface_data;
	Engine::MaterialData::PhongMaterialData wall_surface_data;
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
	const static constexpr std::array< Vector3, 10 > CUBE_REFLECTED_POSITIONS =
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

	const static constexpr Vector3 CUBES_ORIGIN = std::accumulate( CUBE_REFLECTED_POSITIONS.cbegin(), CUBE_REFLECTED_POSITIONS.cend(), Vector3::Zero() ) / CUBE_COUNT;
	const static constexpr Vector3 CAMERA_ROTATION_ORIGIN = CUBES_ORIGIN;

	bool ui_interaction_enabled;
	bool show_imgui_demo_window;

	bool draw_rear_view_cam_to_imgui;
};