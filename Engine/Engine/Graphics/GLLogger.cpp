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

	void GLLogger::Insert( const char* message, const unsigned int id )
	{
		glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, id, GL_DEBUG_SEVERITY_NOTIFICATION, -1, message );
	}

	/* omit_empty_group: If true, defers the push operation until an actual log is recorded between this function call & the PopGroup() call. If no calls were made in-between,
	 * the group is not pushed/popped. It is effectively omitted. */
	void GLLogger::PushGroup( const char* group_name, const bool omit_empty_group, const unsigned int id )
	{
		if( omit_empty_group )
		{
			/* Defer actual glPushDebugGroup() to InternalDebugOutputCallback(); This way a group can be checked to see if it is emptyand if so, skipped.Else, it is pushed first. */
			empty_log_groups.push( group_name );
		}
		else
			glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, id, -1, group_name );
	}

	void GLLogger::PopGroup()
	{
		if( not empty_log_groups.empty() )
		{
			empty_log_groups.pop();
			return;
		}

		glPopDebugGroup();
	}

	GLLogger::GLLogGroup GLLogger::TemporaryLogGroup( const char* group_name, const bool omit_empty_group, const unsigned int id )
	{
		GLLogGroup group( this, group_name, omit_empty_group, id );
		return group;
	}

	void GLLogger::SetLabel( const GLenum object_type, const GLuint object_id, const char* label )
	{
		glObjectLabel( object_type, object_id, -1, label );
	}

	void GLLogger::SetLabel( const GLenum object_type, const GLuint object_id, const std::string& name )
	{
		glObjectLabel( object_type, object_id, -1, name.c_str() );
	}

	void GLLogger::GetLabel( const GLenum object_type, const GLuint object_id, char* label )
	{
		int label_length;
		glGetObjectLabel( object_type, object_id, GL_MAX_LABEL_LENGTH, &label_length, label );
	}

	std::string GLLogger::GetLabel( const GLenum object_type, const GLuint object_id )
	{
		static char OBJECT_LABEL_STORAGE[ GL_MAX_LABEL_LENGTH ];
		int label_length;
		glGetObjectLabel( object_type, object_id, GL_MAX_LABEL_LENGTH, &label_length, OBJECT_LABEL_STORAGE );
		return OBJECT_LABEL_STORAGE;
	}

	void GLLogger::IgnoreID( const unsigned int id_to_ignore )
	{
		glDebugMessageControl( GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &id_to_ignore, false );
	}

	void GLLogger::DontIgnoreID( const unsigned int id_to_restore )
	{
		glDebugMessageControl( GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &id_to_restore, true );
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
		/* We pop one group name every time a log is made. This way, we can check if any logs were recorded between push/pop of a specific log group & can skip the group if it is empty. */
		if( not empty_log_groups.empty() )
		{
			const auto& group_name = empty_log_groups.top();

			if( group_name != message )
			{
				glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, id, -1, group_name );

				empty_log_groups.pop();
			}
		}

		const auto severity_string = GLenumToString_Severity( severity );	// Has a max length of 17.
		const auto source_string   = GLenumToString_Source( source );		// Has a max length of 13.
		const auto type_string     = GLenumToString_Type( type );			// Has a max length of 13.

		constexpr std::size_t fixed_portion_length =
			1 + 1 +				// 1 vertical line + 1 space.
			17 + 1 + 1 + 1 +	// Severity ( max length = 17 ) + 1 space + 1 vertical line + 1 space.
			13 + 1 + 1 + 1 +	// Source   ( max length = 13 ) + 1 space + 1 vertical line + 1 space.
			13 + 1 + 1 + 1 +	// Type     ( max length = 13 ) + 1 space + 1 vertical line + 1 space.
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