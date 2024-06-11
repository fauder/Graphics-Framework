// Engine Includes.
#include "Engine.h"
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

	Engine::Texture container_texture;
	Engine::Texture awesomeface_texture;

	Engine::Shader* cube_shader;
	Engine::Shader gouraud_shader, phong_shader;
	Engine::Shader light_source_shader;

	Engine::Vector3 cube_1_offset, cube_2_offset;
	Engine::Vector3 light_source_offset;
	Engine::Vector3 camera_offset, camera_direction;

	Engine::Color4 cube_1_color, cube_2_color, light_color;
	float light_ambient_strength, light_diffuse_strength, light_specular_strength;
	float light_specular_power;

	float near_plane, far_plane;
	float aspect_ratio;
	Engine::Radians vertical_field_of_view;
};