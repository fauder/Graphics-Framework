// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Shader.h"
#include "Engine/Graphics/VertexArray.h"
#include "Engine/Graphics/Texture.h"

// std Includes.
#include <array>

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
	Engine::VertexArray vertex_array;

	Engine::Texture container_texture;
	Engine::Texture awesomeface_texture;

	Engine::Shader shader;
};