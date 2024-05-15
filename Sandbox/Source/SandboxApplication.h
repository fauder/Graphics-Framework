// Engine Includes.
#include "Engine.h"

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
};