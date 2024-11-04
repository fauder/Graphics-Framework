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
		size( ZERO_INITIALIZATION ),
		sample_count( 0 ),
		target( Target::Invalid ),
		name( "<default constructed FB>" )
	{
	}

	Framebuffer::Framebuffer( const std::string& name, const int width_in_pixels, const int height_in_pixels,
							  const BitFlags< AttachmentType > attachment_bits,
							  const bool is_sRGB,
							  const std::optional< int > multi_sample_count,
							  const Target target )
		:
		id( {} ),
		size( width_in_pixels, height_in_pixels ),
		sample_count( multi_sample_count ),
		target( target ),
		is_sRGB( is_sRGB ),
		name( name )
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

		auto CreateTextureAndAttachToFramebuffer = [ & ]( std::optional< const Texture* >& attachment_texture, 
														  const char* attachment_type_name, 
														  const GLenum attachment_type_enum,
														  const GLenum format,
														  const bool use_sRGB )
		{
			ASSERT_DEBUG_ONLY( ( ( attachment_type_enum >= GL_COLOR_ATTACHMENT0 && attachment_type_enum <= GL_DEPTH_ATTACHMENT ) ||
								 attachment_type_enum == GL_STENCIL_ATTACHMENT ||
								 attachment_type_enum == GL_DEPTH_STENCIL_ATTACHMENT ) &&
							   "Invalid attachment type enum passed to CreateTextureAndAttachToFramebuffer() lambda!" );

			if( multi_sample_count )
			{
				std::string full_name( this->name + attachment_type_name + std::to_string( width_in_pixels ) + "x" + std::to_string( height_in_pixels ) +
									   " (" + std::to_string( *multi_sample_count ) + " samples)" );
				attachment_texture = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateAsset( Engine::Texture( *multi_sample_count,
																												  full_name, format,
																												  width_in_pixels,
																												  height_in_pixels,
																												  use_sRGB ) );
			}
			else
			{
				std::string full_name( this->name + attachment_type_name + std::to_string( width_in_pixels ) + "x" + std::to_string( height_in_pixels ) );
				attachment_texture = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateAsset( Engine::Texture( full_name, format,
																												  width_in_pixels,
																												  height_in_pixels,
																												  use_sRGB ) );
			}

			glFramebufferTexture2D( ( GLenum )target, attachment_type_enum, multi_sample_count ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachment_texture.value()->Id().Get(), 0 );
		};

		if( attachment_bits.IsSet( AttachmentType::Color ) )
			CreateTextureAndAttachToFramebuffer( color_attachment, " Color Tex. ", GL_COLOR_ATTACHMENT0, GL_RGBA, is_sRGB );

		if( attachment_bits.IsSet( AttachmentType::DepthStencilCombined ) )
			CreateTextureAndAttachToFramebuffer( depth_stencil_attachment, " D/S Tex. ", GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL, false /* do not use sRGB. */ );
		else
		{
			if( attachment_bits.IsSet( AttachmentType::Depth ) )
				CreateTextureAndAttachToFramebuffer( depth_attachment, " Depth Tex. ", GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, false /* do not use sRGB. */ );
			if( attachment_bits.IsSet( AttachmentType::Stencil ) )
				CreateTextureAndAttachToFramebuffer( stencil_attachment, " Stencil Tex. ", GL_STENCIL_ATTACHMENT, GL_STENCIL_INDEX, false /* do not use sRGB. */ );
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
		size( std::exchange( donor.size, ZERO_INITIALIZATION ) ),
		sample_count( std::exchange( donor.sample_count, std::nullopt ) ),
		target( std::exchange( donor.target, Target::Invalid ) ),
		is_sRGB( std::exchange( donor.is_sRGB, {} ) ),
		name( std::exchange( donor.name, "<moved-from>" ) ),
		color_attachment( std::exchange( donor.color_attachment, std::nullopt ) ),
		depth_stencil_attachment( std::exchange( donor.depth_stencil_attachment, std::nullopt ) ),
		depth_attachment( std::exchange( donor.depth_attachment, std::nullopt ) ),
		stencil_attachment( std::exchange( donor.stencil_attachment, std::nullopt ) )
	{
	}

	Framebuffer& Framebuffer::operator=( Framebuffer&& donor )
	{
		Destroy();

		id                       = std::exchange( donor.id,							{} );
		size                     = std::exchange( donor.size,						ZERO_INITIALIZATION );
		sample_count             = std::exchange( donor.sample_count,				std::nullopt );
		target                   = std::exchange( donor.target,						Target::Invalid );
		is_sRGB                  = std::exchange( donor.is_sRGB,					{} );
		name                     = std::exchange( donor.name,						"<moved-from>" );
		color_attachment         = std::exchange( donor.color_attachment,			std::nullopt );
		depth_stencil_attachment = std::exchange( donor.depth_stencil_attachment,	std::nullopt );
		depth_attachment         = std::exchange( donor.depth_attachment,			std::nullopt );
		stencil_attachment       = std::exchange( donor.stencil_attachment,			std::nullopt );

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

	void Framebuffer::Destroy()
	{
		if( IsValid() )
		{
			if( HasColorAttachment() )
				Engine::AssetDatabase< Engine::Texture >::RemoveAsset( color_attachment.value()->Name() );

			if( HasCombinedDepthStencilAttachment() )
				Engine::AssetDatabase< Engine::Texture >::RemoveAsset( depth_stencil_attachment.value()->Name() );
			else
			{
				if( HasSeparateDepthAttachment() )
					Engine::AssetDatabase< Engine::Texture >::RemoveAsset( depth_attachment.value()->Name() );
				if( HasSeparateStencilAttachment() )
					Engine::AssetDatabase< Engine::Texture >::RemoveAsset( stencil_attachment.value()->Name() );
			}

			glDeleteFramebuffers( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void Framebuffer::Bind() const
	{
		glBindFramebuffer( ( GLenum )target, id.Get() );
	}

	void Framebuffer::Unbind() const
	{
		glBindFramebuffer( ( GLenum )target, 0 );
	}
}
