// Engine Includes.
#include "Graphics.h"
#include "Framebuffer.h"
#include "GLLogger.h"
#include "Core/Assertion.h"
#include "Core/ServiceLocator.h"

namespace Engine
{
	Framebuffer::Framebuffer()
		:
		id( {} ),
		size( ZERO_INITIALIZATION ),
		sample_count( 0 ),
		target( Target::Invalid ),
		name( "<unnamed FB>" )
	{
	}

	Framebuffer::Framebuffer( const std::string_view name, const int width, const int height,
							  const std::variant< const Texture*, const Renderbuffer* > color_attachment,
							  const std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_and_stencil_attachment,
							  const std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_attachment,
							  const std::variant< std::monostate, const Texture*, const Renderbuffer* > stencil_attachment,
							  const Target target )
		:
		id( {} ),
		size( width, height ),
		// sample_count is not initialized here because it will be set in Create() below.
		target( target ),
		name( name ),
		color_attachment( color_attachment ),
		depth_and_stencil_attachment( depth_and_stencil_attachment ),
		depth_attachment( depth_attachment ),
		stencil_attachment( stencil_attachment )
	{
		Create();
	}

	Framebuffer::Framebuffer( Framebuffer&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		size( std::exchange( donor.size, ZERO_INITIALIZATION ) ),
		sample_count( std::exchange( donor.sample_count, 0 ) ),
		target( std::exchange( donor.target, Target::Invalid ) ),
		name( std::exchange( donor.name, {} ) ),
		color_attachment( std::exchange( donor.color_attachment, {} ) ),
		depth_and_stencil_attachment( std::exchange( donor.depth_and_stencil_attachment, std::monostate{} ) ),
		depth_attachment( std::exchange( donor.depth_attachment, std::monostate{} ) ),
		stencil_attachment( std::exchange( donor.stencil_attachment, std::monostate{} ) )
	{
	}

	Framebuffer& Framebuffer::operator =( Framebuffer&& donor )
	{
		id                           = std::exchange( donor.id,								{} );
		size                         = std::exchange( donor.size,							ZERO_INITIALIZATION );
		sample_count                 = std::exchange( donor.sample_count,					0 );
		target                       = std::exchange( donor.target,							Target::Invalid );
		name                         = std::exchange( donor.name,							{} );
		color_attachment             = std::exchange( donor.color_attachment,				{} );
		depth_and_stencil_attachment = std::exchange( donor.depth_and_stencil_attachment,	std::monostate{} );
		depth_attachment             = std::exchange( donor.depth_attachment,				std::monostate{} );
		stencil_attachment           = std::exchange( donor.stencil_attachment,				std::monostate{} );

		return *this;
	}

	Framebuffer::~Framebuffer()
	{
		if( IsValid() )
			glDeleteFramebuffers( 1, id.Address() );
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

	void Framebuffer::Create()
	{
		glGenFramebuffers( 1, id.Address() );
		Bind();

#ifdef _DEBUG		
		ASSERT( not( HasCombinedDepthStencilAttachment() && ( HasSeparateDepthAttachment() || HasSeparateStencilAttachment() ) ) &&
				"A Framebuffer can not use separate depth/stencil attachment together a combined Depth Stencil attachment!" );
#endif // _DEBUG

		std::visit( [ & ]( auto&& attachment )
        {
            using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )target, GL_COLOR_ATTACHMENT0, attachment->IsMultiSampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )target, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, attachment->Id().Get() );
			else
				static_assert( false, "non-exhaustive visitor!" );

			sample_count = attachment->SampleCount();
        }, color_attachment );
		
		std::visit( [ & ]( auto&& attachment )
        {
            using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )target, GL_DEPTH_ATTACHMENT, attachment->IsMultiSampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )target, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, attachment->Id().Get() );
			// else -> std::monostate.
		}, depth_attachment );

		std::visit( [ & ]( auto&& attachment )
		{
			using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )target, GL_STENCIL_ATTACHMENT, attachment->IsMultiSampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )target, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, attachment->Id().Get() );
			// else -> std::monostate.
		}, stencil_attachment );

		std::visit( [ & ]( auto&& attachment )
		{
			using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )target, GL_DEPTH_STENCIL_ATTACHMENT, attachment->IsMultiSampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )target, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, attachment->Id().Get() );
			// else -> std::monostate.
        }, depth_and_stencil_attachment );

		if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
			std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!\n";


#ifdef _DEBUG
		// This block is intentionally placed after attachments, as the sample_count is set there.
		if( not name.empty() )
		{
			const std::string name( IsMultiSampled() ? this->name + " (" + std::to_string( sample_count ) + " samples)" : this->name );
			ServiceLocator< GLLogger >::Get().SetLabel( GL_FRAMEBUFFER, id.Get(), name );
		}
#endif // _DEBUG

		Unbind();
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