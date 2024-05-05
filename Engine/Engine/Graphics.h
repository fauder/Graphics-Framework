#pragma once

// GLAD Includes.
#include <glad/glad.h>

// Project Includes.
#include "Assert.h"

// std Includes.
#include <iostream>

#define BUFFER_OFFSET(idx) ( static_cast< char* >( 0 ) + ( idx ) )

#define GLCALL( x ) GLClearError();\
x;\
ASSERT( GLLogCall( #x, __FILE__, __LINE__ ) )

static void GLClearError()
{
	while( auto error = glGetError() );
}

static bool GLLogCall( const char* function, const char* file, const int line )
{
	while( auto error = glGetError() )
	{
		std::cerr << "ERROR::GL::";
		switch( error )
		{
			case GL_INVALID_ENUM:		std::cerr << "GL_INVALID_ENUM";			break;
			case GL_INVALID_VALUE:		std::cerr << "GL_INVALID_VALUE";		break;
			case GL_INVALID_OPERATION:	std::cerr << "GL_INVALID_OPERATION";	break;
			case GL_OUT_OF_MEMORY:		std::cerr << "GL_OUT_OF_MEMORY";		break;

			default:					std::cerr << "UNKNOWN";					break; // This should not be possible, but won't hurt to guard against.
		}
		std::cerr << "\n\t in function " << function;
		std::cerr << "\n\t\tin file [" << file << "::Line " << line << "]";

		return false;
	}

	return true;
}
