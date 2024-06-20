// Engince Includes.
#include "Core/Utility.h"
#include "Shader.hpp"
#include "ShaderTypeInformation.h"

// std Includes.
#include <fstream>

namespace Engine
{
	/* Will be initialized later with FromFile(). */
	Shader::Shader( const char* name )
		:
		program_id( 0 ),
		name( name )
	{
	}

	Shader::Shader( const char* name, const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path )
		: name( name )
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

		if( link_result )
			ParseUniformData( uniform_info_map );

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

	void Shader::ParseUniformData( std::unordered_map< std::string, UniformInformation >& uniform_information_map )
	{
		int active_uniform_count = 0;
		GLCALL( glGetProgramiv( program_id, GL_ACTIVE_UNIFORMS, &active_uniform_count ) );

		if( active_uniform_count == 0 )
			return;

		int uniform_name_max_length = 0;
		GLCALL( glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_name_max_length ) );
		std::string name( uniform_name_max_length, '?' );

		int offset = 0;
		for( int uniform_index = 0; uniform_index < active_uniform_count; uniform_index++ )
		{
			/*
			 * glGetActiveUniform has a parameter named "size", but its actually the size of the array. So for singular types like int, float, vec2, vec3 etc. the value returned is 1.
			 */
			int array_size_dontCare = 0, length = 0;
			GLenum type;
			glGetActiveUniform( program_id, uniform_index, uniform_name_max_length, &length, &array_size_dontCare, &type, name.data() );
			GLCALL( glGetActiveUniform( program_id, uniform_index, uniform_name_max_length, &length, &array_size_dontCare, &type, name.data() ) );

			const int size = GetSizeOfType( type );

			GLClearError();
			uniform_information_map[ name.data() ] = { glGetUniformLocation( program_id, name.data() ), size, offset, type };
			ASSERT( GLLogCall( "glGetUniformLocation", __FILE__, __LINE__ ) );

			offset += size;
		}
	}

	const Shader::UniformInformation& Shader::GetUniformInformation( const std::string& uniform_name )
	{
	#ifdef _DEBUG
		try
		{
			return uniform_info_map.at( uniform_name );
		}
		catch( const std::exception& )
		{
			throw std::runtime_error( R"(ERROR::SHADER::UNIFORM::")" + std::string( uniform_name ) + R"("::DOES_NOT_EXIST)" );
		}
	#else
		return uniform_info_map[ uniform_name ];
	#endif // DEBUG
	}
}
