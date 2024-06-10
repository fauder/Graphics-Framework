// Engince Includes.
#include "Core/Utility.h"
#include "Shader.hpp"

// std Includes.
#include <fstream>

namespace Engine
{
	/* Will be initialized later with FromFile(). */
	Shader::Shader()
		:
		program_id( 0 )
	{
	}

	Shader::Shader( const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path )
	{
		FromFile( vertex_shader_source_file_path, fragment_shader_source_file_path );
	}

	Shader::~Shader()
	{
		GLCALL( glDeleteProgram( program_id ) );
	}

	bool Shader::FromFile( const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path )
	{
		unsigned int vertex_shader_id = 0, fragment_shader_id = 0;

		if( const auto vertex_shader_source = ParseShaderFromFile( vertex_shader_source_file_path, ShaderType::VERTEX );
			vertex_shader_source )
		{
			if( !CompileShader( vertex_shader_source->c_str(), vertex_shader_id, ShaderType::VERTEX ) )
				return false;
		}
		else
			return false;

		if( const auto fragment_shader_source = ParseShaderFromFile( fragment_shader_source_file_path, ShaderType::FRAGMENT );
			fragment_shader_source )
		{
			if( !CompileShader( fragment_shader_source->c_str(), fragment_shader_id, ShaderType::FRAGMENT ) )
				return false;
		}
		else
		{
			GLCALL( glDeleteShader( vertex_shader_id ) );
			return false;
		}

		const bool link_result = LinkProgram( vertex_shader_id, fragment_shader_id );

		GLCALL( glDeleteShader( vertex_shader_id ) );
		GLCALL( glDeleteShader( fragment_shader_id ) );

		return link_result;
	}

	void Shader::Bind() const
	{
		GLCALL( glUseProgram( program_id ) );
	}

	std::optional< std::string > Shader::ParseShaderFromFile( const char* file_path, const ShaderType shader_type )
	{
		std::string error_prompt( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) + "::FILE_NOT_SUCCESSFULLY_READ" );

		if( const auto source = Engine::Utility::ReadFileIntoString( file_path, error_prompt.c_str() );
			source )
			return *source;

		return std::nullopt;
	}

	bool Shader::CompileShader( const char* source, unsigned int& shader_id, const ShaderType shader_type )
	{
		GLCALL( shader_id = glCreateShader( ShaderTypeID( shader_type ) ) );
		GLCALL( glShaderSource( shader_id, /* how many strings: */ 1, &source, NULL ) );
		GLCALL( glCompileShader( shader_id ) );

		int success = false;
		GLCALL( glGetShaderiv( shader_id, GL_COMPILE_STATUS, &success ) );
		if( !success )
		{
			char info_log[ 512 ];
			glGetShaderInfoLog( shader_id, 512, NULL, info_log );
			std::cerr << "ERROR::SHADER::" << ShaderTypeString( shader_type ) << "::COMPILE:" << "\n    " << info_log << "\n";
			throw std::logic_error( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) + "::COMPILE:\n    " + info_log );
			return false;
		}

		return true;
	}

	bool Shader::LinkProgram( const unsigned int vertex_shader_id, const unsigned int fragment_shader_id )
	{
		program_id = glCreateProgram();
		GLCALL( glAttachShader( program_id, vertex_shader_id ) );
		GLCALL( glAttachShader( program_id, fragment_shader_id ) );
		GLCALL( glLinkProgram( program_id ) );

		int success;
		GLCALL( glGetProgramiv( program_id, GL_LINK_STATUS, &success ) );
		if( !success )
		{
			char info_log[ 512 ];
			glGetProgramInfoLog( program_id, 512, NULL, info_log );
			std::cerr << "ERROR::SHADER::PROGRAM::LINK:" << info_log << "\n";
			throw std::logic_error( std::string( "ERROR::SHADER::PROGRAM::LINK:" ) + info_log );
			return false;;
		}

		return true;
	}
}
