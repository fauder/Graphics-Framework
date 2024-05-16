#pragma once

// Engine Includes.
#include "Graphics.h"

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

		void Use();

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

		template<>
		void SetUniform< std::array< float, 3 > >( const int location, const std::array< float, 3 >& value )
		{
			GLCALL( glUniform3fv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.data() ) );
		}

		template<>
		void SetUniform< std::array< float, 4 > >( const int location, const std::array< float, 4 >& value )
		{
			GLCALL( glUniform4fv( location, /* This is the number of ARRAY elements, not the number of vector components in this case. */ 1, value.data() ) );
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

			std::cerr << "ERROR::SHADER::UNIFORM::SET:\n    " << "There is no uniform named \"" << uniform_name << "\".\n";
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
