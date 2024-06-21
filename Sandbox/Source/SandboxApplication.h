// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting.h"
#include "Engine/Graphics/Shader.hpp"
#include "Engine/Graphics/VertexArray.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Math/Angle.hpp"
#include "Engine/Math/Vector.hpp"

class SandboxApplication : public Engine::Application
{
public:
	SandboxApplication();
	virtual ~SandboxApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	//virtual void Run() override;

	virtual void Render() override;

	virtual void DrawImGui() override;
	//virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;

private:
	void UpdateViewMatrix( Engine::Shader& shader );
	void UpdateProjectionMatrix( Engine::Shader& shader );

private:
	Engine::VertexArray vertex_array_crate;

	Engine::Texture container_texture_diffuse_map, container_texture_specular_map;

	Engine::Shader* cube_shader;
	Engine::Shader gouraud_shader, phong_shader;
	Engine::Shader light_source_shader;

	Engine::Vector3 camera_offset, camera_direction;
	bool camera_is_animated;

	const static constexpr int CUBE_COUNT = 10;

	std::vector< Engine::Lighting::SurfaceData > cube_surface_data;
	//Engine::Lighting::SurfaceData ground_quad_surface_data;
	Engine::Lighting::DirectionalLightData light_data_directional;

	float near_plane, far_plane;
	float aspect_ratio;
	Engine::Radians vertical_field_of_view;
};