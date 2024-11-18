// Engine Includes.
#include "Graphics.h"
#include "DefaultFramebuffer.h"
#include "Core/Platform.h"

namespace Engine
{
	DefaultFramebuffer::DefaultFramebuffer()
		:
		size( Platform::GetFramebufferSizeInPixels() ),
		clear_targets( ClearTarget::ColorBuffer ),
		clear_color( Color4::Black() ),
		clear_depth_value( 0.0f ),
		clear_stencil_value( 0 )
	{
		/* Query attachments: */
		{
			int attachment;

			glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &attachment );
			if( attachment )
				clear_targets.Set( ClearTarget::DepthBuffer );

			glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &attachment );
			if( attachment )
				clear_targets.Set( ClearTarget::StencilBuffer );
		}

		// Query MSAA info:
		{
			if( glIsEnabled( GL_MULTISAMPLE ) )
			{
				int queried_sample_count;
				glGetIntegerv( GL_SAMPLES, &queried_sample_count );

				if( queried_sample_count > 1 )
					sample_count = queried_sample_count;
			}
		}
	}

	DefaultFramebuffer& DefaultFramebuffer::Instance()
	{
		static DefaultFramebuffer instance;
		return instance;
	}

	void DefaultFramebuffer::Bind( const Framebuffer::BindPoint bind_point )
	{
		glBindFramebuffer( ( GLenum )bind_point, 0 );
	}

	void DefaultFramebuffer::SetClearColor( const Color3& new_clear_color )
	{
		auto& instance = Instance();
		instance.clear_color = new_clear_color;
		SetClearColor();
	}

	void DefaultFramebuffer::SetClearColor( const Color4& new_clear_color )
	{
		auto& instance = Instance();
		instance.clear_color = new_clear_color;
		SetClearColor();
	}

	void DefaultFramebuffer::SetClearDepthValue( const float new_clear_depth_value )
	{
		auto& instance = Instance();
		instance.clear_depth_value = new_clear_depth_value;
		SetClearDepthValue();
	}

	void DefaultFramebuffer::SetClearStencilValue( const int new_clear_stencil_value )
	{
		auto& instance = Instance();
		instance.clear_stencil_value = new_clear_stencil_value;
		SetClearStencilValue();
	}

	void DefaultFramebuffer::SetClearTargets( const BitFlags< ClearTarget > targets )
	{
		auto& instance = Instance();
		instance.clear_targets = targets;
	}

	void DefaultFramebuffer::Clear()
	{
		auto& instance = Instance();
		glClear( ( GLbitfield )instance.clear_targets.ToBits() );
	}

	void DefaultFramebuffer::SetClearColor()
	{
		auto& instance = Instance();
		glClearColor( instance.clear_color.R(), instance.clear_color.G(), instance.clear_color.B(), instance.clear_color.A() );
	}

	void DefaultFramebuffer::SetClearDepthValue()
	{
		auto& instance = Instance();
		glClearDepthf( instance.clear_depth_value );
	}

	void DefaultFramebuffer::SetClearStencilValue()
	{
		auto& instance = Instance();
		glClearStencil( instance.clear_stencil_value );
	}
}
