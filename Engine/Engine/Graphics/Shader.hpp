#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Graphics/Color.hpp"
#include "Graphics/Lighting.h"
#include "Graphics/Uniform.h"
#include "UniformBufferTag.h"
#include "Math/Concepts.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <map>
#include <vector>

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
		Shader( const char* name );
		Shader( const char* name, const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path );
		~Shader();

		bool FromFile( const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path );

		void Bind() const;

		inline const std::string& GetName() const { return name; }

		inline const std::map< std::string, Uniform::Information_Old >& GetUniformInformations() const { return uniform_info_map_legacy; }
		inline std::size_t GetTotalUniformSize() const { return uniform_book_keeping_info.total_size; }

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

		template< typename UniformType >
		void SetUniform( const char* uniform_name, const UniformType& value )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			if constexpr( std::is_base_of_v< UniformBufferTag, UniformType > )
			{
				ASSERT_DEBUG_ONLY( uniform_info.IsUserDefinedStruct() && "Non-struct uniform attempted to be set via SetUniform< uniform-struct-type >()." );

				for( const auto& [ dont_care_member_uniform_name, member_uniform_info ] : uniform_info.members )
				{
					const int   parent_relative_offset = member_uniform_info->original_offset - uniform_info.offset;
					const void* member_uniform_pointer = ( const void* )( ( const char* )&value + parent_relative_offset );

					SetUniform( *member_uniform_info, member_uniform_pointer );
				}
			}
			else
			{
				SetUniform( uniform_info.location, value );
			}
		}

		void SetUniform( const Uniform::Information_Old& uniform_info, const void* value_pointer );

	private:
		std::optional< std::string > ParseShaderFromFile( const char* file_path, const ShaderType shader_type );
		bool CompileShader( const char* source, unsigned int& shader_id, const ShaderType shader_type );
		bool LinkProgram( const unsigned int vertex_shader_id, const unsigned int fragment_shader_id );

		std::string ShaderSource_CommentsStripped( const std::string& shader_source );

		void GetUniformBookKeepingInfo();
		
		/* Expects empty input vectors. */
		bool GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( const int active_uniform_count,
																		 std::vector< unsigned int >& block_indices, std::vector< unsigned int >& corresponding_uniform_indices ) const;
		void QueryUniformData_In_DefaultBlock( std::map< std::string, Uniform::Information >& uniform_information_map );
		void QueryUniformData_In_UniformBlocks( std::map< std::string, Uniform::Information >& uniform_information_map );
		void QueryUniformBufferData( std::map< std::string, Uniform::BufferInformation >& uniform_buffer_information_map );
		void ParseUniformData_StructMemberCPUOrders( const std::string& shader_source );
		std::size_t CalculateTotalUniformSize() const;

		const Uniform::Information_Old& GetUniformInformation( const std::string& uniform_name );

		void LogErrors_Compilation( const int shader_id, const ShaderType shader_type ) const;
		void LogErrors_Linking( const int program_id ) const;
		std::string FormatErrorLog( const char* log ) const;

	private:
		int program_id;
		std::string name;
		std::map< std::string, Uniform::Information_Old > uniform_info_map_legacy;
		std::map< std::string, Uniform::Information > uniform_info_map;
		std::map< std::string, Uniform::BufferInformation > uniform_buffer_info_map;
		Uniform::ActiveUniformBookKeepingInformation uniform_book_keeping_info;
	};
}