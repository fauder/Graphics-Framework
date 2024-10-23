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
		width( 0 ),
		height( 0 ),
		name( "<unnamed FB>" )
	{
	}

	Framebuffer::Framebuffer( const std::string_view name, const int width, const int height,
							  const std::variant< const Texture*, const Renderbuffer* > color_attachment,
							  const std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_and_stencil_attachment,
							  const std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_attachment,
							  const std::variant< std::monostate, const Texture*, const Renderbuffer* > stencil_attachment,
							  const Usage usage )
		:
		id( {} ),
		width( width ),
		height( height ),
		name( name ),
		usage( usage ),
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
		width( std::exchange( donor.width, 0 ) ),
		height( std::exchange( donor.height, 0 ) ),
		name( std::exchange( donor.name, {} ) ),
		usage( std::exchange( donor.usage, Usage::Invalid ) ),
		color_attachment( std::exchange( donor.color_attachment, {} ) ),
		depth_and_stencil_attachment( std::exchange( donor.depth_and_stencil_attachment, std::monostate{} ) ),
		depth_attachment( std::exchange( donor.depth_attachment, std::monostate{} ) ),
		stencil_attachment( std::exchange( donor.stencil_attachment, std::monostate{} ) )
	{
	}

	Framebuffer& Framebuffer::operator =( Framebuffer&& donor )
	{
		id                           = std::exchange( donor.id,								{} );
		width                        = std::exchange( donor.width,							0 );
		height                       = std::exchange( donor.height,							0 );
		name                         = std::exchange( donor.name,							{} );
		usage                        = std::exchange( donor.usage,							Usage::Invalid );
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

	void Framebuffer::Create()
	{
		glGenFramebuffers( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_FRAMEBUFFER, id.Get(), this->name );
		
		ASSERT( not( HasCombinedDepthStencilAttachment() && ( HasSeparateDepthAttachment() || HasSeparateStencilAttachment() ) ) &&
				"A Framebuffer can not use separate depth/stencil attachment together a combined Depth Stencil attachment!" );
#endif // _DEBUG

		std::visit( [ & ]( auto&& attachment )
        {
            using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )usage, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )usage, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, attachment->Id().Get() );
			else
				static_assert( false, "non-exhaustive visitor!" );
        }, color_attachment );

		std::visit( [ & ]( auto&& attachment )
        {
            using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )usage, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )usage, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, attachment->Id().Get() );
			// else -> std::monostate.
        }, depth_attachment );

		std::visit( [ & ]( auto&& attachment )
        {
            using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )usage, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )usage, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, attachment->Id().Get() );
			// else -> std::monostate.
        }, stencil_attachment );

		std::visit( [ & ]( auto&& attachment )
        {
            using T = std::decay_t< decltype( attachment ) >;

			if constexpr( std::is_same_v< T, const Texture* > )
				glFramebufferTexture2D( ( GLenum )usage, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment->Id().Get(), 0 );
			else if constexpr( std::is_same_v< T, const Renderbuffer* > )
				glFramebufferRenderbuffer( ( GLenum )usage, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, attachment->Id().Get() );
			// else -> std::monostate.
        }, depth_and_stencil_attachment );

		if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
			std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!\n";

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	void Framebuffer::Bind() const
	{
		glBindFramebuffer( ( GLenum )usage, id.Get() );
	}
}