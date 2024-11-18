// Engine Includes.
#include "Graphics.h"
#include "Framebuffer.h"
#include "GLLogger.h"
#include "Core/Assertion.h"
#include "Core/AssetDatabase.hpp"
#include "Core/ServiceLocator.h"

namespace Engine
{
	Framebuffer::Framebuffer()
		:
		id( {} ),
		bind_point( BindPoint::Invalid ),
		size( ZERO_INITIALIZATION ),
		is_sRGB( false ),
		clear_color( Color4::Black() ),
		clear_depth_value( 0.0f ),
		clear_stencil_value( 0 ),
		name( "<default constructed FB>" ),
		color_attachment( nullptr ),
		depth_stencil_attachment( nullptr ),
		depth_attachment( nullptr ),
		stencil_attachment( nullptr )
	{
	}

	Framebuffer::Framebuffer( const std::string& name, Description&& description )
		:
		id( {} ),
		bind_point( description.bind_point ),
		size( description.width_in_pixels, description.height_in_pixels ),
		sample_count( description.multi_sample_count ),
		is_sRGB( description.is_sRGB ),
		clear_color( Color4::Black() ),
		clear_depth_value( 0.0f ),
		clear_stencil_value( 0 ),
		name( name ),
		color_attachment( nullptr ),
		depth_stencil_attachment( nullptr ),
		depth_attachment( nullptr ),
		stencil_attachment( nullptr )
	{
		glGenFramebuffers( 1, id.Address() );
		Bind();

	#ifdef _DEBUG
		if( not name.empty() )
		{
			const std::string full_name( IsMultiSampled() ? this->name + " (" + std::to_string( *sample_count ) + " samples)" : this->name );
			ServiceLocator< GLLogger >::Get().SetLabel( GL_FRAMEBUFFER, id.Get(), full_name );
		}
	#endif // _DEBUG

		auto CreateTextureAndAttachToFramebuffer = [ & ]( const Texture*& attachment_texture, 
														  const char* attachment_type_name, 
														  const GLenum attachment_type_enum,
														  const GLenum format,
														  const bool use_sRGB )
		{
			ASSERT_DEBUG_ONLY( ( ( attachment_type_enum >= GL_COLOR_ATTACHMENT0 && attachment_type_enum <= GL_DEPTH_ATTACHMENT ) ||
								 attachment_type_enum == GL_STENCIL_ATTACHMENT ||
								 attachment_type_enum == GL_DEPTH_STENCIL_ATTACHMENT ) &&
							   "Invalid attachment type enum passed to CreateTextureAndAttachToFramebuffer() lambda!" );

			if( sample_count )
			{
				std::string full_name( this->name + attachment_type_name + std::to_string( size.X() ) + "x" + std::to_string( size.Y() ) +
									   " (" + std::to_string( *sample_count ) + " samples)" );
				attachment_texture = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateAsset( Engine::Texture( *sample_count,
																												  full_name, format,
																												  size.X() ,
																												  size.Y(),
																												  use_sRGB ) );
			}
			else
			{
				std::string full_name( this->name + attachment_type_name + std::to_string( size.X() ) + "x" + std::to_string( size.Y() ) );
				attachment_texture = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateAsset( Engine::Texture( full_name, format,
																												  size.X() ,
																												  size.Y(),
																												  use_sRGB,
																												  description.wrap_u, description.wrap_v,
																												  description.minification_filter, description.magnification_filter ) );
			}

			glFramebufferTexture2D( ( GLenum )bind_point, attachment_type_enum, sample_count ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachment_texture->Id().Get(), 0 );
		};

		if( description.attachment_bits.IsSet( AttachmentType::Color ) )
		{
			CreateTextureAndAttachToFramebuffer( color_attachment, " Color Tex. ", GL_COLOR_ATTACHMENT0, GL_RGBA, is_sRGB );
			clear_targets.Set( ClearTarget::ColorBuffer );
		}

		if( description.attachment_bits.IsSet( AttachmentType::DepthStencilCombined ) )
		{
			CreateTextureAndAttachToFramebuffer( depth_stencil_attachment, " D/S Tex. ", GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL, false /* do not use sRGB. */ );
			clear_targets.Set( ClearTarget::DepthBuffer, ClearTarget::StencilBuffer );
		}
		else
		{
			if( description.attachment_bits.IsSet( AttachmentType::Depth ) )
			{
				CreateTextureAndAttachToFramebuffer( depth_attachment, " Depth Tex. ", GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, false /* do not use sRGB. */ );
				clear_targets.Set( ClearTarget::DepthBuffer );
			}
			if( description.attachment_bits.IsSet( AttachmentType::Stencil ) )
			{
				CreateTextureAndAttachToFramebuffer( stencil_attachment, " Stencil Tex. ", GL_STENCIL_ATTACHMENT, GL_STENCIL_INDEX, false /* do not use sRGB. */ );
				clear_targets.Set( ClearTarget::StencilBuffer );
			}
		}

		if( not HasColorAttachment() )
		{
			/* Disable color read/write since there's no color attachment attached. */
			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );
		}

		ASSERT_DEBUG_ONLY( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
		if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
			std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!\n";

		Unbind();
	}

	Framebuffer::Framebuffer( Framebuffer&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		bind_point( std::exchange( donor.bind_point, BindPoint::Invalid ) ),
		size( std::exchange( donor.size, ZERO_INITIALIZATION ) ),
		sample_count( std::exchange( donor.sample_count, std::nullopt ) ),
		is_sRGB( std::exchange( donor.is_sRGB, {} ) ),
		clear_targets( std::exchange( donor.clear_targets, {} ) ),
		clear_color( std::exchange( donor.clear_color, Color4::Black() ) ),
		clear_depth_value( std::exchange( donor.clear_depth_value, 0.0f ) ),
		clear_stencil_value( std::exchange( donor.clear_stencil_value, 0 ) ),
		name( std::exchange( donor.name, "<moved-from>" ) ),
		color_attachment( std::exchange( donor.color_attachment, nullptr ) ),
		depth_stencil_attachment( std::exchange( donor.depth_stencil_attachment, nullptr ) ),
		depth_attachment( std::exchange( donor.depth_attachment, nullptr ) ),
		stencil_attachment( std::exchange( donor.stencil_attachment, nullptr ) )
	{
	}

	Framebuffer& Framebuffer::operator=( Framebuffer&& donor )
	{
		Destroy();

		id                       = std::exchange( donor.id,							{} );
		bind_point               = std::exchange( donor.bind_point,					BindPoint::Invalid );
		size                     = std::exchange( donor.size,						ZERO_INITIALIZATION );
		sample_count             = std::exchange( donor.sample_count,				std::nullopt );
		is_sRGB                  = std::exchange( donor.is_sRGB,					{} );
		clear_targets            = std::exchange( donor.clear_targets,				{} );
		clear_color              = std::exchange( donor.clear_color,				Color4::Black() );
		clear_depth_value        = std::exchange( donor.clear_depth_value,			0.0f );
		clear_stencil_value      = std::exchange( donor.clear_stencil_value,		0 );
		name                     = std::exchange( donor.name,						"<moved-from>" );
		color_attachment         = std::exchange( donor.color_attachment,			nullptr );
		depth_stencil_attachment = std::exchange( donor.depth_stencil_attachment,	nullptr );
		depth_attachment         = std::exchange( donor.depth_attachment,			nullptr );
		stencil_attachment       = std::exchange( donor.stencil_attachment,			nullptr );

		return *this;
	}

	Framebuffer::~Framebuffer()
	{
		Destroy();
	}

	void Framebuffer::SetName( const std::string& new_name )
	{
		name = new_name;
	}

	void Framebuffer::Blit( const Framebuffer& source, const Framebuffer& destination )
	{
		glBindFramebuffer( GL_READ_FRAMEBUFFER, source.Id().Get() );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, destination.Id().Get() );
		glBlitFramebuffer( 0, 0, source.Width(), source.Height(),
						   0, 0, destination.Width(), destination.Height(),
						   GL_COLOR_BUFFER_BIT, GL_LINEAR );
	}

	void Framebuffer::SetClearColor( const Color3& new_clear_color )
	{
		clear_color = new_clear_color;
		SetClearColor();
	}

	void Framebuffer::SetClearColor( const Color4& new_clear_color )
	{
		clear_color = new_clear_color;
		SetClearColor();
	}

	void Framebuffer::SetClearDepthValue( const float new_clear_depth_value )
	{
		clear_depth_value = new_clear_depth_value;
		SetClearDepthValue();
	}

	void Framebuffer::SetClearStencilValue( const int new_clear_stencil_value )
	{
		clear_stencil_value = new_clear_stencil_value;
		SetClearStencilValue();
	}

	void Framebuffer::SetClearTargets( const BitFlags< ClearTarget > targets )
	{
		clear_targets = targets;
	}

	void Framebuffer::Clear() const
	{
		glClear( ( GLbitfield )clear_targets.ToBits() );
	}

	void Framebuffer::Destroy()
	{
		if( IsValid() )
		{
			if( HasColorAttachment() )
				Engine::AssetDatabase< Engine::Texture >::RemoveAsset( color_attachment->Name() );

			if( HasCombinedDepthStencilAttachment() )
				Engine::AssetDatabase< Engine::Texture >::RemoveAsset( depth_stencil_attachment->Name() );
			else
			{
				if( HasSeparateDepthAttachment() )
					Engine::AssetDatabase< Engine::Texture >::RemoveAsset( depth_attachment->Name() );
				if( HasSeparateStencilAttachment() )
					Engine::AssetDatabase< Engine::Texture >::RemoveAsset( stencil_attachment->Name() );
			}

			glDeleteFramebuffers( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void Framebuffer::SetClearColor()
	{
		glClearColor( clear_color.R(), clear_color.G(), clear_color.B(), clear_color.A() );
	}

	void Framebuffer::SetClearDepthValue()
	{
		glClearDepthf( clear_depth_value );
	}

	void Framebuffer::SetClearStencilValue()
	{
		glClearStencil( clear_stencil_value );
	}

	void Framebuffer::Bind() const
	{
		glBindFramebuffer( ( GLenum )bind_point, id.Get() );
	}

	void Framebuffer::Unbind() const
	{
		glBindFramebuffer( ( GLenum )bind_point, 0 );
	}
}
