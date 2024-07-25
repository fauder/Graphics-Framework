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

			QueryUniformData();
			QueryUniformData_BlockIndexAndOffsetForBufferMembers();
			QueryUniformBufferData();
			QueryUniformBufferData_Aggregates();

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

	void Shader::QueryUniformData()
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

			uniform_info_map[ uniform_book_keeping_info.name_holder.c_str() ] =
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

	void Shader::QueryUniformData_BlockIndexAndOffsetForBufferMembers()
	{
		std::vector< unsigned int > block_indices, corresponding_uniform_indices;
		if( not GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( uniform_book_keeping_info.count, block_indices, corresponding_uniform_indices ) )
			return;

		/* Size below is commented out because array uniforms are not implemented yetand size here refers to array size.It is 1 for non - arrays. */

		std::vector< int > corresponding_offsets( corresponding_uniform_indices.size() );
		glGetActiveUniformsiv( program_id, ( int )corresponding_uniform_indices.size(), corresponding_uniform_indices.data(), GL_UNIFORM_OFFSET, corresponding_offsets.data() );

		for( int index = 0; index < corresponding_uniform_indices.size(); index++ )
		{
			const auto uniform_index = corresponding_uniform_indices[ index ];
			const auto block_index   = block_indices[ index ];
			const auto offset        = corresponding_offsets[ index ];

			int length = 0;
			glGetActiveUniformName( program_id, uniform_index, uniform_book_keeping_info.name_max_length, &length, uniform_book_keeping_info.name_holder.data() );

			const auto& uniform_name = uniform_book_keeping_info.name_holder.c_str();

			/* 'size', 'type' and 'is_buffer_member' was already initialized during query of default block uniforms. Update the remaining information for block member uniforms: */
			auto& uniform_info = uniform_info_map[ uniform_name ];

			uniform_info.location_or_block_index = ( int )block_index;
			uniform_info.offset                  = offset;
		}
	}

	void Shader::QueryUniformBufferData()
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

			auto& uniform_buffer_information = uniform_buffer_info_map[ name.c_str() ] = 
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

	void Shader::QueryUniformBufferData_Aggregates()
	{
		std::vector< Uniform::Information* > members_map;

		for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
		{
			using BufferInfoPair = std::pair< const std::string, Uniform::Information* >;
			std::vector< BufferInfoPair* > uniform_buffer_info_sorted_by_offset;
			for( auto& pair : uniform_buffer_info.members_map )
				uniform_buffer_info_sorted_by_offset.push_back( &pair );

			std::stable_sort( uniform_buffer_info_sorted_by_offset.begin(), uniform_buffer_info_sorted_by_offset.end(),
							  []( const BufferInfoPair* left, const BufferInfoPair* right )
			{
				return left->second->offset < right->second->offset;
			} );

			for( int i = 0; i < uniform_buffer_info_sorted_by_offset.size(); i++ ) // -> Outer for loop.
			{
				const auto& buffer_info_pair = uniform_buffer_info_sorted_by_offset[ i ];
				const auto& uniform_name     = buffer_info_pair->first;
				const auto& uniform_info     = buffer_info_pair->second;

				std::string_view uniform_name_without_buffer_name( uniform_name.cbegin() + uniform_buffer_name.size() + 1, uniform_name.cend() ); // +1 for the dot.

				if( const auto bracket_pos = uniform_name_without_buffer_name.find( '[' );
					bracket_pos != std::string_view::npos )
				{
					int stride = uniform_info->size, member_count = 1;

					members_map.push_back( uniform_info );

					/* Find the other members of the array's CURRENT element: */
					bool done_processing_array_element = false;
					int j = i + 1;
					for( ; j < uniform_buffer_info_sorted_by_offset.size() && not done_processing_array_element; j++ )
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + uniform_buffer_name.size() + 1, other_uniform_name.cend() ); // +1 for the dot.

						// + 2 to include the index, which has to match for a given array element.
						if( other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, bracket_pos + 2 ) ) )
						{
							stride += other_uniform_info->size;
							member_count++;
							members_map.push_back( other_uniform_info );
						}
						else
							done_processing_array_element = true;
					}

					member_count = j - i - 1; // -1 because j had been incremented once more before the for loop ended.
					stride       = Math::RoundToMultiple_PowerOf2( stride, sizeof( Vector4 ) ); // Std140 dictates this.

					done_processing_array_element = false;
					for( ; j < uniform_buffer_info_sorted_by_offset.size() && not done_processing_array_element; j++ )
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + uniform_buffer_name.size() + 1, other_uniform_name.cend() ); // +1 for the dot.

						// No +2 this time; we're looking for the array name only.
						if( !other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, bracket_pos ) ) )
							done_processing_array_element = true;
					}

					const int element_count = ( j - i - 1 ) / member_count; // -1 because j had been incremented once more before the for loop ended.
					
					const auto aggregate_name( uniform_name_without_buffer_name.substr( 0, bracket_pos ) );
					uniform_buffer_info.members_array_map.emplace( aggregate_name,
																   Uniform::BufferMemberInformation_Array
																   { 
																		.offset        = uniform_info->offset,
																		.stride        = stride,
																		.element_count = element_count,
																		.editor_name   = UniformEditorName_BufferMemberAggregate( aggregate_name ),
																		.members_map   = members_map,
																   } );

					members_map.clear(); // Re-use the existing vector with its grown capacity.

					i += j - i - 1 - 1; // Outer for loop's i++ will also increment i, that's why there is a minus 1. The other -1 is the same as the ones above; for loop increments the J for one last time.
				}
				else if( const auto dot_pos = uniform_name_without_buffer_name.find( '.' );
						 dot_pos != std::string_view::npos )
				{
					int member_count = 1;
					int size = uniform_info->size;

					members_map.push_back( uniform_info );

					// Don't need to check whether the loop ended naturally or not in this case; If the loop ends naturally, then incrementing i is not important any more.

					for( int j = i + 1; j < uniform_buffer_info_sorted_by_offset.size(); j++ ) // -> Inner for loop.
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + uniform_buffer_name.size() + 1, other_uniform_name.cend() ); // +1 for the dot.

						if( other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, dot_pos ) ) )
						{
							size += other_uniform_info->size;

							members_map.push_back( other_uniform_info );
						}
						else
						{
							size = Math::RoundToMultiple_PowerOf2( size, sizeof( Vector4 ) ); // Std140 dictates this.

							member_count = j - i;
							i += member_count - 1; // Outer for loop's i++ will also increment i, that's why there is a minus 1.

							break;
						}
					}

					const auto aggregate_name( uniform_name_without_buffer_name.substr( 0, dot_pos ) );
					uniform_buffer_info.members_struct_map.emplace( aggregate_name,
																	Uniform::BufferMemberInformation_Struct
																	{
																		 .offset = uniform_info->offset,
																		 .size = size,
																		 .editor_name = UniformEditorName_BufferMemberAggregate( aggregate_name ),
																		 .members_map = members_map
																	} );

					members_map.clear(); // Re-use the existing vector with its grown capacity.
				}
				else
				{
					uniform_buffer_info.members_single.push_back( uniform_info );
				}
			}
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

		// TODO: Make a distinction between real total size and total size without intrinsics & globals, as Material uses this total size to allocate its blob (and it shouldn't allocate for Intrinsics/Globals).
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

	std::string Shader::UniformEditorName( const std::string_view original_name )
	{
		const auto null_terminator_pos = original_name.find( '\0' );
		std::string editor_name( original_name.cbegin(), null_terminator_pos != std::string::npos
																? original_name.cbegin() + null_terminator_pos + 1
																: original_name.cend() );

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

	std::string Shader::UniformEditorName_BufferMemberAggregate( const std::string_view aggregate_name )
	{
		std::string editor_name( aggregate_name );

		std::replace( editor_name.begin(), editor_name.end(), '_', ' ' );

		if( std::isalpha( editor_name[ 0 ] ) )
			editor_name[ 0 ] = std::toupper( editor_name[ 0 ] );
		else if( editor_name.starts_with( ' ' ) )
			editor_name = editor_name.erase( 0, 1 );

		for( auto index = 1; index < editor_name.size(); index++ )
			if( editor_name[ index - 1 ] == ' ' )
				editor_name[ index ] = std::toupper( editor_name[ index ] );

		return editor_name;
	}
}
