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
	Uniform::BindingPoint UniformBufferManager::RegisterUniformBlock( const Shader& shader, const std::string& block_name, const Uniform::BufferCategory category )
	{
		auto& instance = Instance();

		switch( category )
		{
			case Uniform::BufferCategory::Instance:
				return RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_instance );
			case Uniform::BufferCategory::Global:
				return RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_global );
			case Uniform::BufferCategory::Intrinsic:
				return RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_intrinsic );
			// case Uniform::BufferCategory::Regular:
			default:
				return RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_regular );
		}
	}

	void UniformBufferManager::ConnectBufferToBlock( const UniformBuffer& uniform_buffer, const std::string& block_name, const Uniform::BufferCategory category )
	{
		auto& instance = Instance();

		std::optional< Uniform::BindingPoint > binding_point;

		switch( category )
		{
			case Uniform::BufferCategory::Regular:
				binding_point = BindingPoint( block_name, instance.binding_point_book_keeping_regular.binding_point_map );
				break;
			case Uniform::BufferCategory::Instance:
				binding_point = BindingPoint( block_name, instance.binding_point_book_keeping_instance.binding_point_map );
				break;
			case Uniform::BufferCategory::Global:
				binding_point = BindingPoint( block_name, instance.binding_point_book_keeping_global.binding_point_map );
				break;
			case Uniform::BufferCategory::Intrinsic:
				binding_point = BindingPoint( block_name, instance.binding_point_book_keeping_intrinsic.binding_point_map );
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
		binding_point_max_count( 0 )
	{
		GLCALL( glGetIntegerv( GL_MAX_UNIFORM_BUFFER_BINDINGS, ( int* )&binding_point_max_count ) );

		/* Divide max. binding points = max. uniform buffers/blocks allowed into 4 categories and determine their starting offsets from 0. */

		binding_point_book_keeping_intrinsic.maximum_allowed = 4;									//             4 binding points =>             4 buffers/blocks total.
		binding_point_book_keeping_global.maximum_allowed = 4;									//             4 binding points =>             4 buffers/blocks total.
		binding_point_book_keeping_instance.maximum_allowed = ( binding_point_max_count - 8 ) / 2;	// (Max - 8) / 2 binding points => (Max - 8) / 2 buffers/blocks total.
		binding_point_book_keeping_regular.maximum_allowed = ( binding_point_max_count - 8 ) / 2;	// (Max - 8) / 2 binding points => (Max - 8) / 2 buffers/blocks total.

		binding_point_book_keeping_intrinsic.start_offset = 0;
		binding_point_book_keeping_global.start_offset = binding_point_book_keeping_intrinsic.start_offset + binding_point_book_keeping_intrinsic.maximum_allowed;
		binding_point_book_keeping_instance.start_offset = binding_point_book_keeping_global.start_offset + binding_point_book_keeping_global.maximum_allowed;
		binding_point_book_keeping_regular.start_offset = binding_point_book_keeping_instance.start_offset + binding_point_book_keeping_instance.maximum_allowed;
	}

	Uniform::BindingPoint UniformBufferManager::RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BindingPointBookKeeping& binding_point_book_keeping )
	{
		auto& instance = Instance();

		if( const auto found = binding_point_book_keeping.binding_point_map.find( block_name );
			found != binding_point_book_keeping.binding_point_map.cend() )
		{
			const auto binding_point_found = found->second;

			GLCALL( const unsigned int block_index = glGetUniformBlockIndex( shader.Id(), block_name.c_str() ) );
			GLCALL( glUniformBlockBinding( shader.Id(), block_index, binding_point_found ) );

			return binding_point_found;
		}
		else
		{
			if( binding_point_book_keeping.HasRoom() )
			{
				GLCALL( const unsigned int block_index = glGetUniformBlockIndex( shader.Id(), block_name.c_str() ) );
				GLCALL( glUniformBlockBinding( shader.Id(), block_index, binding_point_book_keeping.in_use ) );

				return binding_point_book_keeping.Assign( block_name );
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
		GLCALL( glBindBufferBase( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.Id() ) );
	}

	void UniformBufferManager::BindBufferToBindingPoint_Partial( const UniformBuffer& uniform_buffer, const Uniform::BindingPoint binding_point,
																 const unsigned int offset, const unsigned int size )
	{
		GLCALL( glBindBufferRange( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.Id(), ( GLintptr )offset, ( GLsizeiptr )size ) );
	}

	std::optional< Uniform::BindingPoint > UniformBufferManager::BindingPoint( const std::string& block_name,
																			   const std::map< std::string, Uniform::BindingPoint >& binding_point_map_to_search )
	{
		if( const auto found = binding_point_map_to_search.find( block_name );
			found != binding_point_map_to_search.cend() )
		{
			return found->second;
		}

		return std::nullopt;
	}
}