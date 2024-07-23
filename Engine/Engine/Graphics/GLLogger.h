#pragma once

// Engine Includes.
#include "GLLogType.h"
#include "Core/ImGuiLog.hpp"

// std Includes.
#include <functional>
#include <stack>

namespace Engine
{
	class GLLogger
	{
	private:
		struct GLLogGroup
		{
			GLLogGroup( GLLogger* logger, const char* group_name, const unsigned int id = 0 )
				:
				logger( logger )
			{
				logger->PushGroup( group_name, id );
			}

			GLLogGroup( const GLLogGroup& other )            = delete;
			GLLogGroup& operator=( const GLLogGroup& other ) = delete;

			GLLogGroup( GLLogGroup&& donor )
				:
				logger( std::exchange( donor.logger, nullptr ) )
			{}

			GLLogGroup& operator=( GLLogGroup&& donor )
			{
				logger = std::exchange( donor.logger, nullptr );
			}

			~GLLogGroup()
			{
				if( logger )
					logger->PopGroup();
			}

		private:
			GLLogger* logger;
		};

	private:
		using CallbackType = std::function< void( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters ) >;

	public:
		GLLogger();
		~GLLogger();

	/* Custom messages: */
		void Insert( const char* message, const unsigned int id = 0 );

	/* Grouping: */
		void PushGroup( const char* group_name, const unsigned int id = 0 );
		void PopGroup();

		GLLogGroup TemporaryLogGroup( const char* group_name, const unsigned int id = 0 );

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
