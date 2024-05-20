// Engine Includes.
#include "Engine.h"
#include "Engine/Shader.h"
#include "Engine/Texture.h"

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
	static constexpr const int VERTEX_ATTRIBUTE_ELEMENT_COUNT = 3 + 3 + 2;
	std::array< float, 4 * VERTEX_ATTRIBUTE_ELEMENT_COUNT > vertices;
	std::array< unsigned int, 6 > indices;

	unsigned int vertex_array_object;

	Engine::Texture wooden_container_texture;
	Engine::Shader shader;
};