#pragma once

// Engine Includes.
#include "Graphics.h"

namespace Engine
{
	enum class GLLogType
	{
		ERROR,               
		DEPRECATED_BEHAVIOR,
		UNDEFINED_BEHAVIOR,
		PORTABILITY,
		PERFORMANCE,
		MARKER,
		PUSH_GROUP,
		POP_GROUP,
		OTHER,          

		COUNT,

		INVALID
	};

	inline GLLogType GLenumToGLLogType( GLenum value )
	{
		switch( value )
		{
			case GL_DEBUG_TYPE_ERROR               : return GLLogType::ERROR;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR : return GLLogType::DEPRECATED_BEHAVIOR;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  : return GLLogType::UNDEFINED_BEHAVIOR;
			case GL_DEBUG_TYPE_PORTABILITY         : return GLLogType::PORTABILITY;
			case GL_DEBUG_TYPE_PERFORMANCE         : return GLLogType::PERFORMANCE;

			case GL_DEBUG_TYPE_MARKER              : return GLLogType::MARKER;
			case GL_DEBUG_TYPE_PUSH_GROUP          : return GLLogType::PUSH_GROUP;
			case GL_DEBUG_TYPE_POP_GROUP           : return GLLogType::POP_GROUP;

			case GL_DEBUG_TYPE_OTHER               : return GLLogType::OTHER;

			default:
				std::cerr << "Invalid GLenum passed to GLenumToGLLogType( GLenum value )!\n";
				return GLLogType::INVALID;
		}
	}
}
