#pragma once

// Engine Includes.
#include "Color.hpp"
#include "Enums.h"
#include "Framebuffer.h"
#include "Core/BitFlags.hpp"

namespace Engine
{
	/* Forward Declarations: */
	class Renderer;

	/* Singleton. */
	class DefaultFramebuffer
	{
		friend class Renderer;

	public:

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( DefaultFramebuffer );

	/* Queries: */
		inline static const Vector2I&		Size()				{ return Instance().size; }
		inline static int					Width()				{ return Instance().size.X(); }
		inline static int					Height()			{ return Instance().size.Y(); }

		inline static int					SampleCount()		{ return Instance().sample_count.value(); }
		inline static bool					IsMultiSampled()	{ return Instance().sample_count.has_value(); }

	/* Attachment Queries: */

		inline static bool HasColorAttachment()					{ return true; }
		inline static bool HasDepthAttachment()					{ return Instance().clear_targets.IsSet( ClearTarget::DepthBuffer   ); }
		inline static bool HasStencilAttachment()				{ return Instance().clear_targets.IsSet( ClearTarget::StencilBuffer ); }

	private:

		DefaultFramebuffer();

		static DefaultFramebuffer& Instance();

	/* Usage: */
		static void Bind( const Framebuffer::BindPoint bind_point = Framebuffer::BindPoint::Both );

	/* Clearing: */

		static void SetClearColor( const Color3& new_clear_color );
		static void SetClearColor( const Color4& new_clear_color );
		static void SetClearDepthValue( const float new_clear_depth_value );
		static void SetClearStencilValue( const int new_clear_stencil_value );

		static void SetClearTargets( const BitFlags< ClearTarget > targets );
		static void Clear();

	/* Clearing: */

		static void SetClearColor();
		static void SetClearDepthValue();
		static void SetClearStencilValue();

	private:
		Vector2I size;

		std::optional< int > sample_count;

		BitFlags< ClearTarget > clear_targets;
		Color4 clear_color;
		float clear_depth_value;
		int clear_stencil_value;
	};
}
