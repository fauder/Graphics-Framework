// Engine Includes.
#include "GLLogger.h"

namespace Engine
{
	GLLogger::GLLogger()
		:
		logger(
		{
			/* Red for error logs.		*/ ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ), // <-- ERROR
			/* Yellow for warning logs. */ ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), // <-- DEPRECATED_BEHAVIOR
			/* Yellow for warning logs. */ ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), // <-- UNDEFINED_BEHAVIOR 
			/* Yellow for warning logs. */ ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), // <-- PORTABILITY        
			/* Yellow for warning logs. */ ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), // <-- PERFORMANCE        
			/* White for other logs.	*/ ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ), // <-- MARKER    
			/* White for other logs.	*/ ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ), // <-- PUSH_GROUP		 
			/* White for other logs.	*/ ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ), // <-- POP_GROUP		 
			/* White for other logs.	*/ ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ), // <-- OTHER		
		} )
	{
	}
		
	GLLogger::~GLLogger()
	{
	}

	void GLLogger::Draw( bool* show )
	{
		logger.Draw( "GL Logs", show );
	}

	GLLogger::CallbackType GLLogger::GetCallback()
	{
		return [ = ]( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters )
		{
			this->InternalDebugOutputCallback( source, type, id, severity, length, message, parameters );
		};
	}

/* 
 * Private API:
 */

	void GLLogger::InternalDebugOutputCallback( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters )
	{
		const auto severity_string = GLenumToString_Severity( severity );	// Has a max length of 17.
		const auto source_string   = GLenumToString_Source( source );		// Has a max length of 13.
		const auto type_string     = GLenumToString_Type( type );			// Has a max length of 13.

		constexpr std::size_t fixed_portion_length =
			1 + 1 +						// 1 vertical line + 1 space.
			17 + 1 + 1 + 1 +			// Severity ( max length = 17 ) + 1 space + 1 vertical line + 1 space.
			13 + 1 + 1 + 1 +			// Source   ( max length = 13 ) + 1 space + 1 vertical line + 1 space.
			13 + 1 + 1 + 1 +			// Type     ( max length = 13 ) + 1 space + 1 vertical line + 1 space.
			1 + 10 + 1 + 1 + 4;	// Open parenthesis + id (max 10 digits) + close parenthesis + colon + 4 spaces.

		static std::vector< char > full_message( fixed_portion_length + 255, ' ' ); // Initial max. message length of 255 reserved.

		if( length > 255 )
		{
			full_message.resize( fixed_portion_length + length );
		}

		sprintf_s( full_message.data(), full_message.size(), "| %-17s | %13s | %13s | (%u):    %s", severity_string, source_string, type_string, id, message );

		logger.AddLog( GLenumToGLLogType( type ), full_message.data() );
	}

	const char* GLLogger::GLenumToString_Source( const GLenum source )
	{
		switch( source )
		{
			case GL_DEBUG_SOURCE_API				: return "GL";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM 		: return "WINDOW SYS.";
			case GL_DEBUG_SOURCE_SHADER_COMPILER 	: return "SHADER COMP.";
			case GL_DEBUG_SOURCE_THIRD_PARTY 		: return "3RD PARTY";
			case GL_DEBUG_SOURCE_APPLICATION 		: return "APPLICATION";
			case GL_DEBUG_SOURCE_OTHER				: return "OTHER";
			
			default:
				return "INVALID ENUM";
		}
	}

	const char* GLLogger::GLenumToString_Type( const GLenum type )
	{
		
		switch( type )
		{
			case GL_DEBUG_TYPE_ERROR 				: return "ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 	: return "DEPRECATED";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 	: return "UNDEFINED";
			case GL_DEBUG_TYPE_PORTABILITY 			: return "PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE 			: return "PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER 				: return "MARKER";
			case GL_DEBUG_TYPE_PUSH_GROUP 			: return "PUSH_GROUP";
			case GL_DEBUG_TYPE_POP_GROUP 			: return "POP_GROUP";
			case GL_DEBUG_TYPE_OTHER 				: return "OTHER";

			default:
				return "INVALID ENUM";
		}
	}

	const char* GLLogger::GLenumToString_Severity( const GLenum severity )
	{
		switch( severity )
		{
			case GL_DEBUG_SEVERITY_HIGH 			: return "SEVERITY: HIGH";
			case GL_DEBUG_SEVERITY_MEDIUM 			: return "SEVERITY: MEDIUM";
			case GL_DEBUG_SEVERITY_LOW 				: return "SEVERITY: LOW";
			case GL_DEBUG_SEVERITY_NOTIFICATION		: return "SEVERITY: NOTIF.";

			default:
				return "INVALID ENUM";
		}
	}
}