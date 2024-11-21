#pragma once

// Engine Includes.
#include "Renderable.h"
#include "RenderState.h"

namespace Engine
{
	/* Forward declarations: */
	class Renderer;

	struct RenderQueue
	{
		friend class Renderer;

		using ReferenceCount = unsigned int;

	public:
		enum class ID : std::uint16_t {};

	public:
		inline const std::string&	Name()				const { return name; }
		inline const RenderState&	GetRenderState()	const { return render_state_override; }
		inline bool					IsEnabled()			const { return is_enabled; }

		std::string name = "<unnamed-queue>";

		RenderState render_state_override;

		std::vector< Renderable* > renderable_list;

		std::unordered_map< Shader*, ReferenceCount > shaders_in_flight;
		std::unordered_map< std::string, Material* > materials_in_flight; // TODO: Generate an ID for Materials (who will generate it?) and use that ID as the key here.

		bool is_enabled = true;

		// 7 bytes of padding.
	};
}
