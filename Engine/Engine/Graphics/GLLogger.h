#pragma once

// Engine Includes.
#include "GLLogType.h"
#include "Core/ImGuiLog.hpp"

// std Includes.
#include <functional>

namespace Engine
{
	class GLLogger
	{
	private:
		using CallbackType = std::function< void( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters ) >;

	public:
		GLLogger();
		~GLLogger();

		void Draw( bool* show = nullptr );

		CallbackType GetCallback();

	private:
		void InternalDebugOutputCallback( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters );

		static const char* GLenumToString_Source( const GLenum source );
		static const char* GLenumToString_Type( const GLenum type );
		static const char* GLenumToString_Severity( const GLenum severity );

	private:
		ImGuiLog< GLLogType, std::size_t( GLLogType::COUNT ) > logger;
	};
}
