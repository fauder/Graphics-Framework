#pragma once

// Engine Includes.
#include "GLLogType.h"
#include "Core/ImGuiLog.hpp"

// std Includes.
#include <functional>
#include <stack>
#include <string>

namespace Engine
{
	class GLLogger
	{
	private:
		struct GLLogGroup
		{
			GLLogGroup( GLLogger* logger, const char* group_name, const bool omit_empty_group = false, const unsigned int id = 0 );

			GLLogGroup( const GLLogGroup& other )            = delete;
			GLLogGroup& operator=( const GLLogGroup& other ) = delete;

			GLLogGroup( GLLogGroup&& donor );
			GLLogGroup& operator=( GLLogGroup&& donor );

			~GLLogGroup();

			void Close();

		private:
			GLLogger* logger;
		};

	private:
		using CallbackType = std::function< void( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters ) >;

	public:
		GLLogger();
		~GLLogger();

	/* Custom messages: */
		void Insert( const char* message, const unsigned int id = 0 ) const;

	/* Grouping: */
		/* omit_empty_group: If true, defers the push operation until an actual log is recorded between this function call & the PopGroup() call. If no calls were made in-between,
		 * the group is not pushed/popped. It is effectively omitted. */
		void PushGroup( const char* group_name, const bool omit_empty_group = false, const unsigned int id = 0 );
		void PopGroup();

		GLLogGroup TemporaryLogGroup( const char* group_name, const bool omit_empty_group = false, const unsigned int id = 0 );

	/* Labeling: */
		void SetLabel( const GLenum object_type, const GLuint object_id, const char* label ) const;
		void SetLabel( const GLenum object_type, const GLuint object_id, const std::string& name ) const;
		void GetLabel( const GLenum object_type, const GLuint object_id, char* label ) const;
		std::string GetLabel( const GLenum object_type, const GLuint object_id ) const;

	/* Filtering IDs: */
		static void IgnoreID( const unsigned int id_to_ignore );
		static void DontIgnoreID( const unsigned int id_to_restore );

	/* Main: */
		void Draw( bool* show = nullptr );

	/* Queries: */
		CallbackType GetCallback();

	private:
		void InternalDebugOutputCallback( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters );

		static const char* GLenumToString_Source( const GLenum source );
		static const char* GLenumToString_Type( const GLenum type );
		static const char* GLenumToString_Severity( const GLenum severity );

	private:
		ImGuiLog< GLLogType, std::size_t( GLLogType::COUNT ) > logger;

		std::stack< const char* > empty_log_groups;
	};
}
