// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engine Includes.
#include "Shader.hpp"
#include "UniformBufferManager.h"

namespace Engine
{
	Uniform::BindingPoint UniformBufferManager::RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BufferInformation& uniform_buffer_info )
	{
		auto& instance = Instance();

		switch( uniform_buffer_info.category )
		{
			case Uniform::BufferCategory::Instance:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_instance );
			case Uniform::BufferCategory::Global:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_global );
			case Uniform::BufferCategory::Intrinsic:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_intrinsic );
			// case Uniform::BufferCategory::Regular:
			default:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_regular );
		}
	}

	void UniformBufferManager::ConnectBufferToBlock( const UniformBuffer& uniform_buffer, const std::string& block_name, const Uniform::BufferCategory category )
	{
		auto& instance = Instance();

		std::optional< Uniform::BindingPoint > binding_point;

		switch( category )
		{
			case Uniform::BufferCategory::Regular:
				binding_point = instance.binding_point_book_keeping_regular.Find( block_name );
				break;
			case Uniform::BufferCategory::Instance:
				binding_point = instance.binding_point_book_keeping_instance.Find( block_name );
				break;
			case Uniform::BufferCategory::Global:
				binding_point = instance.binding_point_book_keeping_global.Find( block_name );
				break;
			case Uniform::BufferCategory::Intrinsic:
				binding_point = instance.binding_point_book_keeping_intrinsic.Find( block_name );
				break;
		}

		if( binding_point.has_value() )
		{
			BindBufferToBindingPoint( uniform_buffer, *binding_point );
		}
#ifdef _DEBUG
		else
		{
			throw std::runtime_error( "UniformBufferManager::ConnectBufferToBlock(): Block with name \"" + block_name + "\" was not registered." );
		#if defined( _WIN32 )
			if( IsDebuggerPresent() )
				OutputDebugStringA( ( "\nUniformBufferManager::ConnectBufferToBlock(): Block with name \"" + block_name + "\" was not registered.\n" ).c_str() );
		#endif // _WIN32
		}
#endif // _DEBUG
	}

/*
 *
 *	PRIVATE API:
 *
 */

	UniformBufferManager::UniformBufferManager()
		:
		/* Divide max. binding points = max. uniform buffers/blocks allowed into 4 categories and determine their starting offsets from 0. */
		binding_point_max_count( QueryMaximumUniformBufferBindingCount() ),
		binding_point_book_keeping_intrinsic( 0, 4 ),
		binding_point_book_keeping_global( 0 + 4, 4 ),
		binding_point_book_keeping_instance( 0 + 4 + 4, ( binding_point_max_count - 8 ) / 2 ),
		binding_point_book_keeping_regular( 0 + 4 + 4 + ( binding_point_max_count - 8 ) / 2, ( binding_point_max_count - 8 ) / 2 )
	{
	}

	Uniform::BindingPoint UniformBufferManager::RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BindingPointBookKeeping& binding_point_book_keeping )
	{
		auto& instance = Instance();

		if( const auto maybe_binding_point = binding_point_book_keeping.Find( block_name );
			maybe_binding_point.has_value() )
		{
			const auto binding_point_found = *maybe_binding_point;

			const unsigned int block_index = glGetUniformBlockIndex( shader.Id(), block_name.c_str() );
			glUniformBlockBinding( shader.Id(), block_index, binding_point_found );

			return binding_point_found;
		}
		else
		{
			if( binding_point_book_keeping.HasRoom() )
			{
				const auto binding_point_to_assign = binding_point_book_keeping.Assign( block_name );

				const unsigned int block_index = glGetUniformBlockIndex( shader.Id(), block_name.c_str() );
				glUniformBlockBinding( shader.Id(), block_index, binding_point_to_assign );

				return binding_point_to_assign;
			}

#ifdef _DEBUG
			throw std::runtime_error( "UniformBufferManager::RegisterUniformBuffer(): Maximum binding point count has been reached. Can not assign new blocks/buffers." );
#if defined( _WIN32 )
			if( IsDebuggerPresent() )
				OutputDebugStringA( "\nUniformBufferManager::RegisterUniformBuffer(): Maximum binding point count has been reached. Can not assign new blocks/buffers.\n" );
#endif // _WIN32
#endif // _DEBUG
		}
	}

	void UniformBufferManager::BindBufferToBindingPoint( const UniformBuffer& uniform_buffer, const Uniform::BindingPoint binding_point )
	{
		glBindBufferBase( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.Id() );
	}

	void UniformBufferManager::BindBufferToBindingPoint_Partial( const UniformBuffer& uniform_buffer, const Uniform::BindingPoint binding_point,
																 const unsigned int offset, const unsigned int size )
	{
		glBindBufferRange( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.Id(), ( GLintptr )offset, ( GLsizeiptr )size );
	}

	unsigned int UniformBufferManager::QueryMaximumUniformBufferBindingCount()
	{
		unsigned int query_result;
		glGetIntegerv( GL_MAX_UNIFORM_BUFFER_BINDINGS, ( int* )&query_result );
		return query_result;
	}
}
