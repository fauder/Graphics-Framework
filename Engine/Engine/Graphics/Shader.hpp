#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Color.hpp"
#include "Lighting.h"
#include "Uniform.h"
#include "UniformBufferTag.h"
#include "Math/Concepts.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine
{
	enum class ShaderType
	{
		VERTEX,
		FRAGMENT,
		_COUNT_
	};

	constexpr const char* ShaderTypeString( const ShaderType shader_type )
	{
		constexpr std::array< const char*, ( int )ShaderType::_COUNT_ > shader_type_identifiers
		{
			"VERTEX",
			"FRAGMENT"
		};

		return shader_type_identifiers[ ( int )shader_type ];
	}

	constexpr int ShaderTypeID( const ShaderType shader_type )
	{
		constexpr std::array< int, ( int )ShaderType::_COUNT_ > shader_type_identifiers
		{
			GL_VERTEX_SHADER,
			GL_FRAGMENT_SHADER
		};

		return shader_type_identifiers[ ( int )shader_type ];
	}

	class Shader
	{
	public:
		using ID = unsigned int;

	public:
		/* Will be initialized later with FromFile(). */
		Shader( const char* name );
		Shader( const char* name, const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path );
		~Shader();

		bool FromFile( const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path );

		void Bind() const;

		inline const std::string&	Name()	const { return name;		}
		inline		 ID				Id()	const { return program_id;	}

	/* Uniform APIs: */

		inline const std::unordered_map< std::string, Uniform::Information			>& GetUniformInfoMap()			const { return uniform_info_map;		}
		inline const std::unordered_map< std::string, Uniform::BufferInformation	>& GetUniformBufferInfoMap()	const { return uniform_buffer_info_map; }
		inline std::size_t GetTotalUniformSize_DefaultBlockOnly() const { return uniform_book_keeping_info.total_size_default_block; }
		inline std::size_t GetTotalUniformSize_UniformBlocksOnly() const { return uniform_book_keeping_info.total_size_uniform_blocks; }
		inline std::size_t GetTotalUniformSize() const { return uniform_book_keeping_info.total_size; }

		inline bool HasIntrinsicUniformBlocks()	const { return uniform_book_keeping_info.intrinsic_block_count;	}
		inline bool HasGlobalUniformBlocks()	const { return uniform_book_keeping_info.global_block_count;	}
		inline bool HasInstanceUniformBlocks()	const { return uniform_book_keeping_info.instance_block_count;	}
		inline bool HasRegularUniformBlocks()	const { return uniform_book_keeping_info.regular_block_count;	}

#pragma region Uniform Set< Type > Functions
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
		void SetUniform< unsigned int >( const int location, const unsigned int& value )
		{
			GLCALL( glUniform1ui( location, value ) );
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

		template<>
		void SetUniform< Color3 >( const int location, const Engine::Color3& value )
		{
			SetUniform( location, reinterpret_cast< const Vector3& >( value ) );
		}

		template<>
		void SetUniform< Color4 >( const int location, const Engine::Color4& value )
		{
			SetUniform( location, reinterpret_cast< const Vector4& >( value ) );
		}

		template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize >
		requires Concepts::NonZero< RowSize > && Concepts::NonZero< ColumnSize >
		void SetUniform( const int location, const Math:: Matrix< Type, RowSize, ColumnSize >& value )
		{
			if constexpr( RowSize == ColumnSize )
			{
				if constexpr( RowSize == 2U )
				{
					GLCALL( glUniformMatrix2fv( location, 1, GL_TRUE, value.Data() ) );
				}
				if constexpr( RowSize == 3U )
				{
					GLCALL( glUniformMatrix3fv( location, 1, GL_TRUE, value.Data() ) );
				}
				if constexpr( RowSize == 4U )
				{
					GLCALL( glUniformMatrix4fv( location, 1, GL_TRUE, value.Data() ) );
				}
			}
			else
			{
				if constexpr( RowSize == 2U && ColumnSize == 3U )
				{
					GLCALL( glUniformMatrix2x3fv( location, 1, GL_TRUE, value.Data() ) );
				}
				if constexpr( RowSize == 2U && ColumnSize == 4U )
				{
					GLCALL( glUniformMatrix2x4fv( location, 1, GL_TRUE, value.Data() ) );
				}
				if constexpr( RowSize == 3U && ColumnSize == 2U )
				{
					GLCALL( glUniformMatrix3x2fv( location, 1, GL_TRUE, value.Data() ) );
				}
				if constexpr( RowSize == 3U && ColumnSize == 4U )
				{
					GLCALL( glUniformMatrix3x4fv( location, 1, GL_TRUE, value.Data() ) );
				}
				if constexpr( RowSize == 4U && ColumnSize == 2U )
				{
					GLCALL( glUniformMatrix4x2fv( location, 1, GL_TRUE, value.Data() ) );
				}
				if constexpr( RowSize == 4U && ColumnSize == 3U )
				{
					GLCALL( glUniformMatrix4x3fv( location, 1, GL_TRUE, value.Data() ) );
				}
			}
		}
#pragma endregion

	/*
	 * "Setting" Uniform Buffers is not the Shader's responsibility (nor can it do it); It's just an OpenGL Buffer Object -> UBO can Update() itself.
	 * Whoever holds the UBO will call Update() on it.
	 * Global   & Intrinsic Uniform Buffers are kept & updated by the Renderer.
	 * Instance & Regular   Uniform Buffers are kept & updated by the Material.
	 */

	/* Uniform setters; By name & value: */
		template< typename UniformType >
			/* Prohibit Uniform Buffers: */ requires( not std::is_base_of_v< UniformBufferTag, UniformType > )
		void SetUniform( const char* uniform_name, const UniformType& value )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			SetUniform( uniform_info.location_or_block_index, value );
		}

	/* Uniform setters; By info. & pointer: */
		void SetUniform( const Uniform::Information& uniform_info, const void* value_pointer );

	private:
	/* Compilation & Linkage: */
		std::optional< std::string > ParseShaderFromFile( const char* file_path, const ShaderType shader_type );
		bool CompileShader( const char* source, unsigned int& shader_id, const ShaderType shader_type );
		bool LinkProgram( const unsigned int vertex_shader_id, const unsigned int fragment_shader_id );

		/*std::string ShaderSource_CommentsStripped( const std::string& shader_source );*/

	/* Shader Introspection: */
		void GetUniformBookKeepingInfo();
		
		/* Expects empty input vectors. */
		bool GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( const int active_uniform_count,
																		 std::vector< unsigned int >& block_indices, std::vector< unsigned int >& corresponding_uniform_indices ) const;
		void QueryUniformData_In_DefaultBlock( std::unordered_map< std::string, Uniform::Information >& uniform_information_map );
		void QueryUniformData_In_UniformBlocks( std::unordered_map< std::string, Uniform::Information >& uniform_information_map );
		void QueryUniformBufferData( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_information_map );
		void CalculateTotalUniformSizes();
		void EnumerateUniformBufferCategories();

		const Uniform::Information& GetUniformInformation( const std::string& uniform_name );
		const Uniform::BufferInformation& GetUniformBufferInformation( const std::string& uniform_name );

	/* Error Checking/Reporting: */
		void LogErrors_Compilation( const int shader_id, const ShaderType shader_type ) const;
		void LogErrors_Linking() const;
		std::string FormatErrorLog( const char* log ) const;

	/* Misc: */
		std::string UniformEditorName( const std::string& original_name );

	private:
		ID program_id;
		std::string name;

		std::unordered_map< std::string, Uniform::Information		> uniform_info_map;
		std::unordered_map< std::string, Uniform::BufferInformation	> uniform_buffer_info_map;

		Uniform::ActiveUniformBookKeepingInformation uniform_book_keeping_info;
	};
}