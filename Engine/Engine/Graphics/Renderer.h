#pragma once

// Engine Includes.
#include "Drawable.h"
#include "Scene/Camera.h"

// std Includes.
#include <vector>

namespace Engine
{
	class Renderer
	{
	public:
		enum class ClearTarget
		{
			DepthBuffer   = GL_DEPTH_BUFFER_BIT,
			StencilBuffer = GL_STENCIL_BUFFER_BIT,
			ColorBuffer   = GL_COLOR_BUFFER_BIT,
		};

	public:
		Renderer();
		~Renderer();

	/* Main: */
		void Render( Camera& camera );

	/* Drawables: */
		void AddDrawable( Drawable* drawable_to_add );
		// TODO: Switch to unsigned map of "Component" UUIDs when Component class is implemented.
		// For now, stick to removing elements from a vector, which is sub-par performance but should be OK for the time being.
		void RemoveDrawable( const Drawable* drawable_to_remove );

	/* Clearing: */
		void SetClearColor( const Color3& new_clear_color );
		void SetClearColor( const Color4& new_clear_color );

		void SetClearTarget( const ClearTarget& target );
		void SetClearTargets( const ClearTarget& target, std::same_as< ClearTarget > auto&& ... other_targets )
		{
			clear_target = ClearTarget( ( int )target | ( ( int )other_targets | ... ) );
		}

	private:
		void SetClearColor();
		void Clear() const;

		void Render( const Mesh& mesh );
		void Render_Indexed( const Mesh& mesh );
		void Render_NonIndexed( const Mesh& mesh );

	private:
		std::vector< Drawable* > drawable_list;

		Color4 clear_color;
		ClearTarget clear_target;
	};
}