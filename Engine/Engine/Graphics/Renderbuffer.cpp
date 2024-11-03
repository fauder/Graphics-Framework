// Engine Includes.
#include "Graphics.h"
#include "Renderbuffer.h"
#include "GLLogger.h"
#include "Core/ServiceLocator.h"

namespace Engine
{
	Renderbuffer::Renderbuffer()
		:
		id( {} ),
		size( ZERO_INITIALIZATION ),
		sample_count( 0 ),
		name( "<unnamed RB>" )
	{
	}

	Renderbuffer::Renderbuffer( const std::string_view name, const int width, const int height )
		:
		id( {} ),
		size( width, height ),
		sample_count( 0 ),
		name( name )
	{
		glGenRenderbuffers( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_RENDERBUFFER, id.Get(), this->name );
#endif // _DEBUG

		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height );
		Unbind();
	}

	Renderbuffer::Renderbuffer( const int sample_count, const std::string_view multi_sampled_renderbuffer_name, const int width, const int height )
		:
		id( {} ),
		size( width, height ),
		sample_count( sample_count ),
		name( multi_sampled_renderbuffer_name )

	{
		glGenRenderbuffers( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_RENDERBUFFER, id.Get(), this->name + " (" + std::to_string( sample_count ) + " samples)");
#endif // _DEBUG

		glRenderbufferStorageMultisample( GL_RENDERBUFFER, sample_count, GL_DEPTH24_STENCIL8, width, height );
		Unbind();
	}

	Renderbuffer::Renderbuffer( Renderbuffer&& donor )
		:
		size( std::exchange( donor.size, ZERO_INITIALIZATION ) ),
		sample_count( std::exchange( donor.sample_count, 0 ) ),
		name( std::exchange( donor.name, {} ) )
	{
		Delete();

		id = std::exchange( donor.id, {} );
	}

	Renderbuffer& Renderbuffer::operator =( Renderbuffer&& donor )
	{
		Delete();

		id           = std::exchange( donor.id,				{} );
		size         = std::exchange( donor.size,			ZERO_INITIALIZATION );
		sample_count = std::exchange( donor.sample_count,	0 );
		name         = std::exchange( donor.name,			{} );

		return *this;
	}

	Renderbuffer::~Renderbuffer()
	{
		Delete();
	}

	void Renderbuffer::SetName( const std::string& new_name )
	{
		name = new_name;
	}

	void Renderbuffer::Delete()
	{
		if( id.IsValid() )
		{
			std::cout << "Removing " << name << " (Id: " << id.Get() << ").\n";
			glDeleteRenderbuffers( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void Renderbuffer::Bind() const
	{
		glBindRenderbuffer( GL_RENDERBUFFER, id.Get() );
	}

	void Renderbuffer::Unbind() const
	{
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}
}
