// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engince Includes.
#include "Core/Utility.hpp"
#include "Shader.hpp"
#include "ShaderTypeInformation.h"

// std Includes.
#include <fstream>
#include <sstream>
#include <string_view>
#include <vector>

namespace Engine
{
	/* Will be initialized later with FromFile(). */
	Shader::Shader( const char* name )
		:
		program_id( 0 ),
		name( name ),
		uniform_size_total( 0 )
	{
	}

	Shader::Shader( const char* name, const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path )
		:
		name( name ),
		uniform_size_total( 0 )
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

		const auto vertex_shader_source = ParseShaderFromFile( vertex_shader_source_file_path, ShaderType::VERTEX );
		if( vertex_shader_source )
		{
			if( !CompileShader( vertex_shader_source->c_str(), vertex_shader_id, ShaderType::VERTEX ) )
				return false;
		}
		else
			return false;

		const auto fragment_shader_source = ParseShaderFromFile( fragment_shader_source_file_path, ShaderType::FRAGMENT );
		if( fragment_shader_source )
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
		{
			QueryUniformData( uniform_info_map );
			ParseUniformData_StructMemberCPUOrders( ShaderSource_CommentsStripped( *vertex_shader_source	) );
			ParseUniformData_StructMemberCPUOrders( ShaderSource_CommentsStripped( *fragment_shader_source	) );
			uniform_size_total = CalculateTotalUniformSize();
		}

		return link_result;
	}

	void Shader::Bind() const
	{
		GLCALL( glUseProgram( program_id ) );
	}

	void Shader::SetUniform( const Uniform::Information& uniform_info, const void* value_pointer )
	{
		switch( uniform_info.type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				: SetUniform( uniform_info.location, *reinterpret_cast< const float*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC2			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector2*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC3			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector3*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC4			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector4*		>( value_pointer ) ); return;
			//case GL_DOUBLE				: SetUniform( uniform_info.location, *reinterpret_cast< const double*		>( value_pointer ) ); return;
			case GL_INT					: SetUniform( uniform_info.location, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_INT_VEC2			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector2I*		>( value_pointer ) ); return;
			case GL_INT_VEC3			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector3I*		>( value_pointer ) ); return;
			case GL_INT_VEC4			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector4I*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT		: SetUniform( uniform_info.location, *reinterpret_cast< const unsigned int*	>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC2	: SetUniform( uniform_info.location, *reinterpret_cast< const Vector2U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC3	: SetUniform( uniform_info.location, *reinterpret_cast< const Vector3U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC4	: SetUniform( uniform_info.location, *reinterpret_cast< const Vector4U*		>( value_pointer ) ); return;
			case GL_BOOL				: SetUniform( uniform_info.location, *reinterpret_cast< const bool*			>( value_pointer ) ); return;
			case GL_BOOL_VEC2			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector2B*		>( value_pointer ) ); return;
			case GL_BOOL_VEC3			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector3B*		>( value_pointer ) ); return;
			case GL_BOOL_VEC4			: SetUniform( uniform_info.location, *reinterpret_cast< const Vector4B*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2 			: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix2x2*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT3 			: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix3x3*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT4 			: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix4x4*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x3 		: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix2x3*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x4 		: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix2x4*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x2 		: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix3x2*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x4 		: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix3x4*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x2 		: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix4x2*	>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x3 		: SetUniform( uniform_info.location, *reinterpret_cast< const Matrix4x3*	>( value_pointer ) ); return;
			case GL_SAMPLER_1D 			: SetUniform( uniform_info.location, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_2D 			: SetUniform( uniform_info.location, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_3D 			: SetUniform( uniform_info.location, *reinterpret_cast< const int*			>( value_pointer ) ); return;
		}

		throw std::runtime_error( "ERROR::SHADER::SetUniform( uniform_info, value_pointer ) called for an unknown GL type!" );
	}

	std::optional< std::string > Shader::ParseShaderFromFile( const char* file_path, const ShaderType shader_type )
	{
		std::string error_prompt( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) + "::FILE_NOT_SUCCESSFULLY_READ\nShader name: " + name + "\n" );

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
			LogErrors_Compilation( shader_id, shader_type );
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
			LogErrors_Linking( program_id );
			return false;
		}

		return true;
	}

	void Shader::QueryUniformData( std::map< std::string, Uniform::Information >& uniform_information_map )
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

#ifdef _DEBUG
			if( array_size_dontCare > 1 )
			{
				// TODO: Implement parsing of arrays of non-struct uniforms.
				throw std::logic_error( "Uniform arrays are not implemented yet." );
			}
#endif // _DEBUG


			GLClearError();
			const auto location = glGetUniformLocation( program_id, name.c_str() );
			ASSERT( GLLogCall( "glGetUniformLocation", __FILE__, __LINE__ ) );

			uniform_information_map[ name.c_str() ] = { location, 
														/* Original order in struct, will be parsed later from source: */ -1,
														size, 
														offset, 
														/* Original memory offset in the struct, will be parsed later from source: */ -1,
														type };

			if( const auto dot_pos = name.find( '.' );
				dot_pos != std::string::npos )
			{
				// This uniform is part of a structure. Add it the the "members" list of the parent structure uniform info.
				const std::string structure_name( name.substr( 0, dot_pos ) );
				auto& structure_uniform_info = uniform_info_map[ structure_name ]; // DELIBERATE USE of [] operator; Create an entry if not present.
				// "location" member variable is initialized to -1 by default. Use this to check whether this is the first uniform in the structure.
				if( structure_uniform_info.location == -1 )
				{
					/* First uniform's location & offset are also the parent structure's location & offset. */
					structure_uniform_info.location = location;
					structure_uniform_info.offset   = offset;
				}
				structure_uniform_info.size += size;
				structure_uniform_info.members.emplace( name.c_str(), &uniform_information_map[ name.c_str() ] );

				// "type" of the structure is unimportant and therefore not assigned.
			}

			offset += size;
		}
	}

	/* Expects: To be called after the shader whose source is passed is compiled & linked successfully. */
	std::string Shader::ShaderSource_CommentsStripped( const std::string& shader_source )
	{
		/* This function is called AFTER the shader is compiled & linked. So it is known for a fact that the block comments have matching pairs of begin/end symbols. */

		auto Strip = [ &shader_source ]( const std::string& source_string, const std::string& comment_begin_token, const std::string& comment_end_token,
										 const bool do_not_erase_new_line = false ) -> std::string
		{
			std::string stripped_shader_source;
			std::size_t current_pos = source_string.find( comment_begin_token, 0 ), last_begin_pos = 0;

			while( current_pos != std::string::npos )
			{
				const std::size_t comment_start_pos = current_pos;
				const std::size_t comment_end_pos   = source_string.find( comment_end_token, current_pos + comment_begin_token.size() );

				stripped_shader_source += source_string.substr( last_begin_pos, comment_start_pos - last_begin_pos );
				last_begin_pos = comment_end_pos + comment_end_token.size() - ( int )do_not_erase_new_line;

				current_pos = source_string.find( comment_begin_token, last_begin_pos );
			}

			/* Add the remaining part of the source string.*/
			stripped_shader_source += source_string.substr( last_begin_pos );

			return stripped_shader_source;
		};

		return Strip( Strip( shader_source, "/*", "*/" ), "//", "\n", true );
	}

	/*
	 * Since OpenGL arbitrarily relocates uniform members inside structs, 
	 * we have to know the original order in order to address the correct memory inside cpu-side struct's memory blob when sending uniforms to shaders.
	 * This makes it possible to write a generic function that can handle ALL uniform structs, instead of having to create a SetUniform< CustomStructType >() for each new struct.
	 *
	 * Expects: To be called after the shader whose source is passed is compiled & linked successfully. */
	void Shader::ParseUniformData_StructMemberCPUOrders( const std::string& shader_source )
	{
		for( auto& [ uniform_name, uniform_info ] : uniform_info_map )
		{
			if( uniform_info.IsUserDefinedStruct() && !uniform_info.HasOriginalOrdersDetermined() )
			{
				std::string uniform_name_without_square_brackets;
				const auto square_bracket_pos = uniform_name.find( '[' );

				if( const auto uniform_struct_name_pos = shader_source.find( uniform_name.substr( 0, square_bracket_pos ) );
					uniform_struct_name_pos != std::string::npos )
				{
					const auto uniform_struct_type_name( Utility::String::FindPreviousWord( shader_source, uniform_struct_name_pos ) );

					const auto struct_type_name_pos = shader_source.find( uniform_struct_type_name );

					if( const auto struct_keyword_pos = shader_source.rfind( "struct", struct_type_name_pos );
						struct_keyword_pos != std::string::npos )
					{
						if( shader_source.find_first_not_of( " \t\n", struct_keyword_pos + 6 ) == struct_type_name_pos )
						{
							if( const auto open_curly_brace_pos = shader_source.find( '{', struct_type_name_pos ), close_curly_brace_pos = shader_source.find( '}', struct_type_name_pos );
								open_curly_brace_pos != std::string::npos && close_curly_brace_pos != std::string::npos && close_curly_brace_pos > open_curly_brace_pos )
							{
								const std::string_view struct_definition_view( shader_source.begin() + open_curly_brace_pos + 1, shader_source.begin() + close_curly_brace_pos );

								int definition_order = uniform_info.location;

								std::size_t line_begin = 0;
								std::size_t semi_colon_pos = struct_definition_view.find( ';' );

								while( semi_colon_pos != std::string_view::npos ) // This loops over lines basically.
								{
									/* DO NOT include the ';' AND/OR any whitespace before it in the line view. */
									const std::string_view line_view( 
										Utility::String::RemoveTrailingWhitespace( std::string_view( struct_definition_view.begin() + line_begin,
																									 struct_definition_view.begin() + semi_colon_pos ) ) );

									/* Case 1: There are NO commas, i.e., the line only contains a single uniform member definition. */
									if( std::size_t comma_pos = line_view.find( ',' );
										comma_pos == std::string_view::npos )
									{
										/* No need to check if find() was successfull;
										 * This shader source compiles successfully, so there HAS TO BE at least one whitespace (between type and variable) in this line. */
										const auto last_preceding_whitespace_position = line_view.find_last_of( " \t" );
											
										const std::string uniform_member_name( line_view.begin() + last_preceding_whitespace_position + 1, line_view.end() );

										uniform_info.members[ uniform_name + "." + uniform_member_name ]->original_order_in_struct = definition_order++;
									}
									/* Case 2: There ARE commas, i.e., the line contains multiple uniform member definitions. */
									else
									{
										std::vector< Uniform::Information* > uniform_infos_found_in_reverse_order;
										/* No need to check if find() was successfull;
										 * This shader source compiles successfully, so there HAS TO BE at least one whitespace (between type and variable) in this line. */
										const auto last_preceding_whitespace_position = line_view.find_last_of( " \t" );

										const std::string uniform_member_name( line_view.begin() + last_preceding_whitespace_position + 1, line_view.end() );

										uniform_infos_found_in_reverse_order.push_back( uniform_info.members[ uniform_name + "." + uniform_member_name ] );

										comma_pos = line_view.rfind( ',', last_preceding_whitespace_position - 1 );
											
										while( comma_pos != std::string_view::npos )
										{
											const std::string_view remaining_line_view( Utility::String::RemoveTrailingWhitespace( std::string_view( line_view.begin(),
																																					 line_view.begin() + comma_pos ) ) );
											const auto uniform_name_begin_position = remaining_line_view.find_last_of( " \t," ) + 1;

											const std::string uniform_member_name( remaining_line_view.begin() + uniform_name_begin_position, remaining_line_view.end() );

											uniform_infos_found_in_reverse_order.push_back( uniform_info.members[ uniform_name + "." + uniform_member_name ] );
											
											comma_pos = remaining_line_view.rfind( ',', uniform_name_begin_position - 1 );
										}

										std::reverse( uniform_infos_found_in_reverse_order.begin(), uniform_infos_found_in_reverse_order.end() );

										for( auto& uniform_info_found : uniform_infos_found_in_reverse_order /*** Now in correct order ***/ )
											uniform_info_found->original_order_in_struct = definition_order++;
									}

									/* Check whether there is only 1 non-processed uniform member left. */
									if( ( definition_order - uniform_info.location ) == uniform_info.members.size() - 1 )
									{
										// There is no need to continue parsing the source code; We can simply process the last remaining member uniform and assign it the next order.
										for( auto& [ member_uniform_name, member_uniform_info ] : uniform_info.members )
											if( member_uniform_info->original_order_in_struct == -1 )
												member_uniform_info->original_order_in_struct = definition_order; // No need to increment either; This is the last member of the struct.
										break;
									}

									line_begin     = semi_colon_pos + 1;
									semi_colon_pos = struct_definition_view.find_first_of( ";", semi_colon_pos + 1 );
								}
							}
						}
					}
				}

				/* Now that the original orders are determined, the offsets based on these original orders can also be calculated. */
				std::map< int /* order */, Uniform::Information* > members_sorted_by_original_order;
				
				for( auto& [ dont_care_about_name, uniform_member_info ] : uniform_info.members )
					members_sorted_by_original_order[ uniform_member_info->original_order_in_struct ] = uniform_member_info;

				int offset = uniform_info.offset;
				for( auto& [ original_order, uniform_member_info ] : members_sorted_by_original_order )
				{
					uniform_member_info->original_offset = offset;

					offset += uniform_member_info->size;
				}
			}
		}
	}

	std::size_t Shader::CalculateTotalUniformSize() const
	{
		std::size_t total = 0;
		for( const auto& [ uniform_name, uniform_info ] : uniform_info_map )
			total += uniform_info.size;

		return total;
	}

	const Uniform::Information& Shader::GetUniformInformation( const std::string& uniform_name )
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

	void Shader::LogErrors_Compilation( const int shader_id, const ShaderType shader_type ) const
	{
		char info_log[ 512 ];
		glGetShaderInfoLog( shader_id, 512, NULL, info_log );

		const std::string complete_error_string( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) + "::COMPILE:\nShader name: " + name + FormatErrorLog( info_log ) );
		std::cerr << complete_error_string;
#if defined( _WIN32 ) && defined( _DEBUG )
		if( IsDebuggerPresent() )
			OutputDebugStringA( ( "\n" + complete_error_string + "\n" ).c_str() );
#endif // _WIN32 && _DEBUG
		throw std::logic_error( complete_error_string );
	}

	void Shader::LogErrors_Linking( const int program_id ) const
	{
		char info_log[ 512 ];
		glGetProgramInfoLog( program_id, 512, NULL, info_log );

		const std::string complete_error_string( "ERROR::SHADER::PROGRAM::LINK:\nShader name: " + name + FormatErrorLog( info_log ) );
		std::cerr << complete_error_string;
#if defined( _WIN32 ) && defined( _DEBUG )
		if( IsDebuggerPresent() )
			OutputDebugStringA( ( "\n" + complete_error_string + "\n" ).c_str() );
#endif // _WIN32 && _DEBUG
		throw std::logic_error( complete_error_string );
	}

	std::string Shader::FormatErrorLog( const char* log ) const
	{
		std::string error_log_string( log );
		Utility::String::Replace( error_log_string, "\n", "\n    " );
		return "\n    " + error_log_string;
	}
}
