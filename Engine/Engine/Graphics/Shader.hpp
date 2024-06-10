#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Graphics/Color.hpp"
#include "Math/Concepts.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <array>
#include <iostream>
#include <optional>
#include <string>

namespace Engine
{
	enum ShaderType
	{
		VERTEX,
		FRAGMENT,
		_COUNT_
	};

	constexpr const char* ShaderTypeString( const ShaderType shader_type )
	{
		constexpr std::array< const char*, ShaderType::_COUNT_ > shader_type_identifiers
		{
			"VERTEX",
			"FRAGMENT"
		};

		return shader_type_identifiers[ shader_type ];
	}

	constexpr int ShaderTypeID( const ShaderType shader_type )
	{
		constexpr std::array< int, ShaderType::_COUNT_ > shader_type_identifiers
		{
			GL_VERTEX_SHADER,
			GL_FRAGMENT_SHADER
		};

		return shader_type_identifiers[ shader_type ];
	}

	class Shader
	{
	public:
		/* Will be initialized later with FromFile(). */
		Shader();
		Shader( const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path );
		~Shader();

		bool FromFile( const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path );

		void Bind() const;

		template< typename UniformType >
		void SetUniform( const int location, const UniformType& value );

		template<>
		void SetUniform< float >( const int location, const float& value )
		{
			GLCALL( glUniform1f( location, value ) );
		}

		template<>
		void SetUniform< int >( const int location, const int& value )
		{
			GLCALL( glUniform1i( location, value ) );
		}

		template<>
		void SetUniform< bool >( const int location, const bool& value )
		{
			GLCALL( glUniform1i( location, value ) );
		}

		template< typename Component, std::size_t Size > requires( Size >= 2 && Size <= 4 )
		void SetUniform( const int location, const Math::Vector< Component, Size >& value )
		{
			if constexpr( std::is_same_v< Component, bool > )
			{
				/* Since sizeof( bool ) != sizeof( int ), we can not just pass a pointer to data and call the v variant of glUniformX functions. */

				if constexpr( Size == 2 )
				{
					GLCALL( glUniform2i( location, value.X(), value.Y() ) );
				}
				if constexpr( Size == 3 )
				{
					GLCALL( glUniform3i( location, value.X(), value.Y(), value.Z() ) );
				}
				if constexpr( Size == 4 )
				{
					GLCALL( glUniform4i( location, value.X(), value.Y(), value.Z(), value.W() ) );
				}
			}
			else
			{
				if constexpr( std::is_same_v< Component, float > )
				{
					if constexpr( Size == 2 )
					{
						GLCALL( glUniform2fv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
					if constexpr( Size == 3 )
					{
						GLCALL( glUniform3fv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
					if constexpr( Size == 4 )
					{
						GLCALL( glUniform4fv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
				}

				if constexpr( std::is_same_v< Component, int > )
				{
					if constexpr( Size == 2 )
					{
						GLCALL( glUniform2iv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
					if constexpr( Size == 3 )
					{
						GLCALL( glUniform3iv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
					if constexpr( Size == 4 )
					{
						GLCALL( glUniform4iv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
				}

				if constexpr( std::is_same_v< Component, unsigned int > )
				{
					if constexpr( Size == 2 )
					{
						GLCALL( glUniform2uiv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
					if constexpr( Size == 3 )
					{
						GLCALL( glUniform3uiv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
					if constexpr( Size == 4 )
					{
						GLCALL( glUniform4uiv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.Data() ) );
					}
				}
			}
		}

		void SetUniform( const int location, const Engine::Color3& value )
		{
			SetUniform( location, reinterpret_cast< const Vector3& >( value ) );
		}

		void SetUniform( const int location, const Engine::Color4& value )
		{
			SetUniform( location, reinterpret_cast< const Vector4& >( value ) );
		}

		/* Only accepts square matrices. */
		template< Concepts::Arithmetic Type, std::size_t Size >
		requires Concepts::NonZero< Size >
		void SetUniform( const int location, const Math:: Matrix< Type, Size, Size >& value )
		{
			if constexpr( Size == 2U )
			{
				GLCALL( glUniformMatrix2fv( location, 1, GL_TRUE, value.Data() ) );
			}
			if constexpr( Size == 3U )
			{
				GLCALL( glUniformMatrix3fv( location, 1, GL_TRUE, value.Data() ) );
			}
			if constexpr( Size == 4U )
			{
				GLCALL( glUniformMatrix4fv( location, 1, GL_TRUE, value.Data() ) );
			}
		}

		template< typename UniformType >
		bool SetUniform( const char* uniform_name, const UniformType& value )
		{
			GLCALL( const int location_retrieved = glGetUniformLocation( program_id, uniform_name ) );
			if( location_retrieved != -1 )
			{
				SetUniform( location_retrieved, value );
				return true;
			}

			std::cerr << "ERROR::SHADER::UNIFORM::SET:\n" << "    There is no uniform named \"" << uniform_name << "\".\n";
			return false;
		}

	private:
		// Returns shader ID if successfull, -1 if not.
		std::optional< std::string > ParseShaderFromFile( const char* file_path, const ShaderType shader_type );
		bool CompileShader( const char* source, unsigned int& shader_id, const ShaderType shader_type );
		bool LinkProgram( const unsigned int vertex_shader_id, const unsigned int fragment_shader_id );

		int program_id;
	};
}
