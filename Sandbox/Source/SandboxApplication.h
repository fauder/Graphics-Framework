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
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

private:
	Engine::VertexArray vertex_array_crate;

	Engine::Texture container_texture;
	Engine::Texture awesomeface_texture;

	Engine::Shader shader;

	Engine::Vector3 cube_1_offset, cube_2_offset;
	Engine::Vector3 camera_offset, camera_direction;

	float near_plane, far_plane;
	float aspect_ratio;
	Engine::Radians vertical_field_of_view;
};