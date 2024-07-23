// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engince Includes.
#include "Core/ServiceLocator.h"
#include "Core/Utility.hpp"
#include "GLLogger.h"
#include "Shader.hpp"
#include "ShaderTypeInformation.h"
#include "UniformBufferManager.h"

// std Includes.
#include <numeric> // std::iota.

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
		:
		name( name )
	{
		FromFile( vertex_shader_source_file_path, fragment_shader_source_file_path );
	}

	Shader::~Shader()
	{
		glDeleteProgram( program_id );
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
			glDeleteShader( vertex_shader_id );
			return false;
		}

		const bool link_result = LinkProgram( vertex_shader_id, fragment_shader_id );

		glDeleteShader( vertex_shader_id );
		glDeleteShader( fragment_shader_id );

		if( link_result )
		{
#ifdef _DEBUG
			ServiceLocator< GLLogger >::Get().SetLabel( GL_PROGRAM, program_id, name );
#endif // _DEBUG

			GetUniformBookKeepingInfo();
			if( uniform_book_keeping_info.count == 0 )
				return true;

			QueryUniformData_In_DefaultBlock( uniform_info_map );
			QueryUniformData_In_UniformBlocks( uniform_info_map );
			QueryUniformBufferData( uniform_buffer_info_map );

			CalculateTotalUniformSizes();
			EnumerateUniformBufferCategories();

			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
				UniformBufferManager::RegisterUniformBlock( *this, uniform_buffer_name, uniform_buffer_info );
		}

		return link_result;
	}

	void Shader::Bind() const
	{
		glUseProgram( program_id );
	}

	void Shader::SetUniform( const Uniform::Information& uniform_info, const void* value_pointer )
	{
		switch( uniform_info.type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const float*			>( value_pointer ) ); return;
			case GL_FLOAT_VEC2			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC3			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC4			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4*		>( value_pointer ) ); return;
			//case GL_DOUBLE				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const double*		>( value_pointer ) ); return;
			case GL_INT					: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_INT_VEC2			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2I*		>( value_pointer ) ); return;
			case GL_INT_VEC3			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3I*		>( value_pointer ) ); return;
			case GL_INT_VEC4			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4I*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const unsigned int*	>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC2	: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC3	: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC4	: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4U*		>( value_pointer ) ); return;
			case GL_BOOL				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const bool*			>( value_pointer ) ); return;
			case GL_BOOL_VEC2			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2B*		>( value_pointer ) ); return;
			case GL_BOOL_VEC3			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3B*		>( value_pointer ) ); return;
			case GL_BOOL_VEC4			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4B*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2 			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix2x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3 			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix3x3*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4 			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix4x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x3 		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix2x3*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x4 		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix2x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x2 		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix3x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x4 		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix3x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x2 		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix4x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x3 		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix4x3*		>( value_pointer ) ); return;
			case GL_SAMPLER_1D 			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_2D 			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_3D 			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
		}

		throw std::runtime_error( "ERROR::SHADER::SetUniform( uniform_info, value_pointer ) called for an unknown GL type!" );
	}

/*
 *
 *	PRIVATE API:
 *
 */

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
		shader_id = glCreateShader( ShaderTypeID( shader_type ) );
		glShaderSource( shader_id, /* how many strings: */ 1, &source, NULL );
		glCompileShader( shader_id );

		int success = false;
		glGetShaderiv( shader_id, GL_COMPILE_STATUS, &success );
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

		glAttachShader( program_id, vertex_shader_id );
		glAttachShader( program_id, fragment_shader_id );
		glLinkProgram( program_id );

		int success;
		glGetProgramiv( program_id, GL_LINK_STATUS, &success );
		if( !success )
		{
			LogErrors_Linking();
			return false;
		}

		return true;
	}

	void Shader::GetUniformBookKeepingInfo()
	{
		glGetProgramiv( program_id, GL_ACTIVE_UNIFORMS, &uniform_book_keeping_info.count );
		glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_book_keeping_info.name_max_length );
		uniform_book_keeping_info.name_holder = std::string( uniform_book_keeping_info.name_max_length, '?' );
	}

	/* Expects empty input vectors. */
	bool Shader::GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( const int active_uniform_count,
																			 std::vector< unsigned int >& block_indices, std::vector< unsigned int >& corresponding_uniform_indices ) const
	{
		corresponding_uniform_indices.resize( active_uniform_count );
		block_indices.resize( active_uniform_count );
		std::iota( corresponding_uniform_indices.begin(), corresponding_uniform_indices.end(), 0 );

		glGetActiveUniformsiv( program_id, active_uniform_count, corresponding_uniform_indices.data(), GL_UNIFORM_BLOCK_INDEX, reinterpret_cast< int* >( block_indices.data() ) );

		int block_count = 0;
		for( auto uniform_index = 0; uniform_index < active_uniform_count; uniform_index++ )
		{
			if( const auto block_index = block_indices[ uniform_index ];
				block_index != -1 )
			{
				/* Overwrite vector elements starting from the beginning. This should be safe as the uniform_index should be greater than block_count at this point.
				 * So we should be overwriting previous elements as we go through all uniform indices. */
				block_indices[ block_count ]                 = block_index;
				corresponding_uniform_indices[ block_count ] = uniform_index;
				block_count++;
			}
		}

		if( block_count )
		{
			block_indices.resize( block_count );
			corresponding_uniform_indices.resize( block_count );
			return true;
		}

		return false;
	}

	void Shader::QueryUniformData_In_DefaultBlock( std::unordered_map< std::string, Uniform::Information >& uniform_information_map )
	{
		int offset = 0;
		for( auto uniform_index = 0; uniform_index < uniform_book_keeping_info.count; uniform_index++ )
		{
			/*
			 * glGetActiveUniform has a parameter named "size", but its actually the size of the array. So for singular types like int, float, vec2, vec3 etc. the value returned is 1.
			 */
			int array_size_dontCare = 0, length_dontCare = 0;
			GLenum type;
			glGetActiveUniform( program_id, uniform_index, uniform_book_keeping_info.name_max_length,
								&length_dontCare, &array_size_dontCare, &type, 
								uniform_book_keeping_info.name_holder.data() );

			const int size = GetSizeOfType( type );

			const auto location = glGetUniformLocation( program_id, uniform_book_keeping_info.name_holder.c_str() );

			const bool is_buffer_member = location == -1;

			uniform_information_map[ uniform_book_keeping_info.name_holder.c_str() ] =
			{
				.location_or_block_index = location,
				.size                    = size,
				.offset                  = is_buffer_member ? -1 : offset,
				.type                    = type,
				.is_buffer_member		 = is_buffer_member,
				.editor_name			 = UniformEditorName( uniform_book_keeping_info.name_holder )
			};

			offset += !is_buffer_member * size;
		}
	}

	void Shader::QueryUniformData_In_UniformBlocks( std::unordered_map< std::string, Uniform::Information >& uniform_information_map )
	{
		std::vector< unsigned int > block_indices, corresponding_uniform_indices;
		if( not GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( uniform_book_keeping_info.count, block_indices, corresponding_uniform_indices ) )
			return;

		/* Size below is commented out because array uniforms are not implemented yetand size here refers to array size.It is 1 for non - arrays. */

		std::vector< int > /*corresponding_array_sizes( corresponding_uniform_indices.size() ),*/ correspoding_offsets( corresponding_uniform_indices.size() );
		//glGetActiveUniformsiv( program_id, ( int )corresponding_uniform_indices.size(), corresponding_uniform_indices.data(), GL_UNIFORM_SIZE,		corresponding_array_sizes.data()	);
		glGetActiveUniformsiv( program_id, ( int )corresponding_uniform_indices.size(), corresponding_uniform_indices.data(), GL_UNIFORM_OFFSET,	correspoding_offsets.data()			);

		for( int index = 0; index < corresponding_uniform_indices.size(); index++ )
		{
			const auto uniform_index = corresponding_uniform_indices[ index ];
			const auto block_index   = block_indices[ index ];
			const auto offset        = correspoding_offsets[ index ];

			int length = 0;
			glGetActiveUniformName( program_id, uniform_index, uniform_book_keeping_info.name_max_length, &length, uniform_book_keeping_info.name_holder.data() );

			const auto& uniform_name = uniform_book_keeping_info.name_holder.c_str();

			/* 'size', 'type' and 'is_buffer_member' was already initialized during query of default block uniforms. Update the remaining information for block member uniforms: */
			auto& uniform_info = uniform_information_map[ uniform_name ];

			uniform_info.location_or_block_index = ( int )block_index;
			uniform_info.offset                  = offset;
		}
	}

	void Shader::QueryUniformBufferData( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_information_map )
	{
		int active_uniform_block_count = 0;
		glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_BLOCKS, &active_uniform_block_count );

		if( active_uniform_block_count == 0 )
			return;

		int uniform_block_name_max_length = 0;
		glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniform_block_name_max_length );
		std::string name( uniform_block_name_max_length, '?' );

		int offset = 0;
		for( int uniform_block_index = 0; uniform_block_index < active_uniform_block_count; uniform_block_index++ )
		{
			int length = 0;
			glGetActiveUniformBlockName( program_id, uniform_block_index, uniform_block_name_max_length, &length, name.data() );
			
			int size;
			glGetActiveUniformBlockiv( program_id, uniform_block_index, GL_UNIFORM_BLOCK_DATA_SIZE,	&size );

			const auto category = Uniform::DetermineBufferCategory( name );

			auto& uniform_buffer_information = uniform_buffer_information_map[ name.c_str() ] = 
			{
				.binding_point = -1, // This will be filled later via BufferManager::ConnectBufferToBlock().
				.size          = size,
				.offset        = offset,
				.category	   = category
			};

			/* Add members and set their block indices. */
			for( auto& [ uniform_name, uniform_info ] : uniform_info_map )
			{
				if( uniform_info.is_buffer_member && uniform_info.location_or_block_index == uniform_block_index )
					uniform_buffer_information.members_map.emplace( uniform_name.data(), &uniform_info );
			}

			offset += size;
		}
	}

#pragma region Unnecessary Old Stuff
	/* Expects: To be called after the shader whose source is passed is compiled & linked successfully. */
	//std::string Shader::ShaderSource_CommentsStripped( const std::string& shader_source )
	//{
	//	/* This function is called AFTER the shader is compiled & linked. So it is known for a fact that the block comments have matching pairs of begin/end symbols. */

	//	auto Strip = [ &shader_source ]( const std::string& source_string, const std::string& comment_begin_token, const std::string& comment_end_token,
	//									 const bool do_not_erase_new_line = false ) -> std::string
	//	{
	//		std::string stripped_shader_source;
	//		std::size_t current_pos = source_string.find( comment_begin_token, 0 ), last_begin_pos = 0;

	//		while( current_pos != std::string::npos )
	//		{
	//			const std::size_t comment_start_pos = current_pos;
	//			const std::size_t comment_end_pos   = source_string.find( comment_end_token, current_pos + comment_begin_token.size() );

	//			stripped_shader_source += source_string.substr( last_begin_pos, comment_start_pos - last_begin_pos );
	//			last_begin_pos = comment_end_pos + comment_end_token.size() - ( int )do_not_erase_new_line;

	//			current_pos = source_string.find( comment_begin_token, last_begin_pos );
	//		}

	//		/* Add the remaining part of the source string.*/
	//		stripped_shader_source += source_string.substr( last_begin_pos );

	//		return stripped_shader_source;
	//	};

	//	return Strip( Strip( shader_source, "/*", "*/" ), "//", "\n", true );
	//}
#pragma endregion

	void Shader::CalculateTotalUniformSizes()
	{
		uniform_book_keeping_info.total_size_default_block = 0, uniform_book_keeping_info.total_size_uniform_blocks = 0;

		/* Sum of default block (i.e., not in any explicit Uniform Buffer) uniforms: */
		for( const auto& [ uniform_name, uniform_info ] : uniform_info_map )
			if( not uniform_info.is_buffer_member ) // Skip buffer members, as their layout (std140) means their total buffer size is calculated differently.
				uniform_book_keeping_info.total_size_default_block += uniform_info.size;

		/* Now add buffer block sizes (calculated before): */
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
			uniform_book_keeping_info.total_size_uniform_blocks += uniform_buffer_info.size;

		uniform_book_keeping_info.total_size = uniform_book_keeping_info.total_size_default_block + uniform_book_keeping_info.total_size_uniform_blocks;
	}

	void Shader::EnumerateUniformBufferCategories()
	{
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
		{
			switch( uniform_buffer_info.category )
			{
				case Uniform::BufferCategory::Instance:
					uniform_book_keeping_info.instance_block_count++; break;
				case Uniform::BufferCategory::Global:
					uniform_book_keeping_info.global_block_count++; break;
				case Uniform::BufferCategory::Intrinsic:
					uniform_book_keeping_info.intrinsic_block_count++; break;
				// case Uniform::BufferCategory::Regular:
				default:
					uniform_book_keeping_info.regular_block_count++; break;
			}
		}
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
			throw std::runtime_error( R"(ERROR::SHADER::GetUniformInformation(): uniform ")" + std::string( uniform_name ) + R"(" does not exist!)" );
		}
	#else
		return uniform_info_map[ uniform_name ];
	#endif // DEBUG
	}

	const Uniform::BufferInformation& Shader::GetUniformBufferInformation( const std::string& uniform_name )
	{
	#ifdef _DEBUG
		try
		{
			return uniform_buffer_info_map.at( uniform_name );
		}
		catch( const std::exception& )
		{
			throw std::runtime_error( R"(ERROR::SHADER::GetUniformBufferInformation(): uniform ")" + std::string( uniform_name ) + R"(" does not exist!)" );
		}
	#else
		return uniform_buffer_info_map[ uniform_name ];
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

	void Shader::LogErrors_Linking() const
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

	std::string Shader::UniformEditorName( const std::string& original_name )
	{
		std::string editor_name( original_name );

		/* First get rid of parent struct/block names: */
		if( const auto last_dot_pos = editor_name.find_last_of( '.' );
			last_dot_pos != std::string::npos )
			editor_name = editor_name.substr( last_dot_pos + 1 );

		if( editor_name.compare( 0, 7, "_GLOBAL", 7 ) == 0 )
			editor_name.erase( 0, 7 );
		else if( editor_name.compare( 0, 8, "_REGULAR", 8 ) == 0 )
			editor_name.erase( 0, 8 );
		else if( editor_name.compare( 0, 9, "_INSTANCE", 9 ) == 0 )
			editor_name.erase( 0, 9 );
		else if( editor_name.compare( 0, 10, "_INTRINSIC", 10 ) == 0 )
			editor_name.erase( 0, 10 );

		std::replace( editor_name.begin(), editor_name.end(), '_', ' ' );

		if( std::isalpha( editor_name[ 0 ] ) )
			editor_name[ 0 ] = std::toupper( editor_name[ 0 ] );
		else if( editor_name.starts_with( ' ' ) )
			editor_name = editor_name.erase( 0, 1 );

		if( editor_name.compare( 0, 7, "UNIFORM", 7 ) == 0 || editor_name.compare( 0, 7, "Uniform", 7 ) == 0 )
			editor_name.erase( 0, 7 );

		for( auto index = 1; index < editor_name.size(); index++ )
			if( editor_name[ index - 1 ] == ' ' )
				editor_name[ index ] = std::toupper( editor_name[ index ] );

		return editor_name;
	}
}
