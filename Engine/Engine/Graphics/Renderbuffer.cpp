// Engine Includes.
#include "Graphics.h"
#include "Renderbuffer.h"
#include "GLLogger.h"
#include "Core/ServiceLocator.h"

namespace Engine
{
	Renderbuffer::Renderbuffer()
		:
		id( -1 ),
		width( 0 ),
		height( 0 ),
		name( "<unnamed RB>" )
	{
	}

	Renderbuffer::Renderbuffer( const std::string_view name, const int width, const int height )
		:
		id( -1 ),
		width( width ),
		height( height ),
		name( name )
	{
		glGenRenderbuffers( 1, &id );
		Bind();

	#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_RENDERBUFFER, id, this->name );
	#endif // _DEBUG

		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}

	Renderbuffer::Renderbuffer( Renderbuffer&& donor )
		:
		id( std::exchange( donor.id, -1 ) ),
		name( std::exchange( donor.name, {} ) ),
		width( std::exchange( donor.width, 0 ) ),
		height( std::exchange( donor.height, 0 ) )
	{
	}

	Renderbuffer& Renderbuffer::operator =( Renderbuffer&& donor )
	{
		id     = std::exchange( donor.id, -1 );
		name   = std::exchange( donor.name, {} );
		width  = std::exchange( donor.width, 0 );
		height = std::exchange( donor.height, 0 );

		return *this;
	}

	Renderbuffer::~Renderbuffer()
	{
		if( IsValid() )
			glDeleteRenderbuffers( 1, &id );
	}

	void Renderbuffer::SetName( const std::string& new_name )
	{
		name = new_name;
	}

	void Renderbuffer::Bind() const
	{
		glBindRenderbuffer( GL_RENDERBUFFER, id );
	}
}