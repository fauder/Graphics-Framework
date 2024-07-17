// Engine Includes.
#include "Material.hpp"
#include "UniformBufferManager.h"

namespace Engine
{
	Material::Material()
		:
		name( "<unnamed>" ),
		shader( nullptr ),
		uniform_info_map( nullptr ),
		uniform_buffer_info_map( nullptr ),
		uniform_blob_offset_of_uniform_buffers( 0 )
	{
	}

	Material::Material( const std::string& name )
		:
		name( name ),
		shader( nullptr ),
		uniform_info_map( nullptr ),
		uniform_buffer_info_map( nullptr ),
		uniform_blob_offset_of_uniform_buffers( 0 )
	{
	}

	Material::Material( const std::string& name, Shader* const shader )
		:
		name( name ),
		shader( shader ),
		uniform_blob( shader->GetTotalUniformSize(), std::byte{ 0 } ),
		uniform_blob_offset_of_uniform_buffers( shader->GetTotalUniformSize_DefaultBlockOnly() ),
		uniform_info_map( &shader->GetUniformInfoMap() ),
		uniform_buffer_info_map( &shader->GetUniformBufferInfoMap() )
	{
		ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Parameter 'shader' passed to Material::Material( const std::string& name, Shader* const shader ) is nullptr!" );

		PopulateAndSetupUniformBufferMap();
		PopulateTextureMap();
	}

	Material::~Material()
	{
	}

	const Shader* Material::Bind() const
	{
		shader->Bind(); 
		return shader;
	}

	void Material::SetShader( Shader* const shader )
	{
		this->shader = shader;

		uniform_blob                   = std::vector< std::byte >( shader->GetTotalUniformSize() );
		uniform_blob_offset_of_uniform_buffers = shader->GetTotalUniformSize_DefaultBlockOnly();

		uniform_info_map        = ( &shader->GetUniformInfoMap() );
		uniform_buffer_info_map = ( &shader->GetUniformBufferInfoMap() );

		PopulateAndSetupUniformBufferMap();
		PopulateTextureMap();
	}

/*
 *
 *	PRIVATE API:
 *
 */

	void Material::SetTexture( const char* sampler_name_of_new_texture, Texture* texture_to_be_set )
	{
		texture_map[ sampler_name_of_new_texture ] = texture_to_be_set;
	}

	const Texture* Material::GetTexture( const char* sampler_name_of_new_texture ) const
	{
		if( const auto iterator = texture_map.find( sampler_name_of_new_texture ); 
			iterator != texture_map.cend() )
		{
			return iterator->second;
		}

		return nullptr;
	}

	void Material::PopulateAndSetupUniformBufferMap()
	{
		for( auto& [ uniform_buffer_name, uniform_buffer_info ] : *uniform_buffer_info_map )
		{
			/* .first gets the resulting iterator. Calling ->second on it gets the underlying Uniform Buffer object. */

			if( uniform_buffer_info.category == Uniform::BufferCategory::Regular )
			{
				auto& buffer = uniform_buffer_map_regular.emplace( uniform_buffer_name, uniform_buffer_info.size ).first->second;
				UniformBufferManager::ConnectBufferToBlock( buffer, uniform_buffer_name, Uniform::BufferCategory::Regular );
			}
			else if( uniform_buffer_info.category == Uniform::BufferCategory::Instance )
			{
				auto& created_buffer = uniform_buffer_map_instance.emplace( uniform_buffer_name, uniform_buffer_info.size ).first->second;
				UniformBufferManager::ConnectBufferToBlock( created_buffer, uniform_buffer_name, Uniform::BufferCategory::Instance );
			}
		}
	}

	const Uniform::BufferInformation& Material::GetUniformBufferInformation( const std::string& uniform_buffer_name ) const
	{
		try
		{
			return uniform_buffer_info_map->at( uniform_buffer_name );
		}
		catch( const std::exception& )
		{
			throw std::runtime_error( R"(ERROR::MATERIAL::GetUniformBufferInformation(): uniform buffer ")" + std::string( uniform_buffer_name ) + R"(" does not exist!)" );
		}
	}

	const Uniform::Information& Material::GetUniformInformation( const std::string& uniform_name ) const
	{
		try
		{
			return uniform_info_map->at( uniform_name );
		}
		catch( const std::exception& )
		{
			throw std::runtime_error( R"(ERROR::MATERIAL::GetUniformInformation(): uniform ")" + std::string( uniform_name ) + R"(" does not exist!)" );
		}
	}

	void* Material::GetUniformPointer( std::size_t offset )
	{
		return GetValuePointerFromBlob( offset );
	}

	const void* Material::GetUniformPointer( std::size_t offset ) const
	{
		return GetValuePointerFromBlob( offset );
	}

	void* Material::GetUniformBufferPointer( std::size_t offset )
	{
		return GetValuePointerFromBlob( uniform_blob_offset_of_uniform_buffers + offset );
	}

	const void* Material::GetUniformBufferPointer( std::size_t offset ) const
	{
		return GetValuePointerFromBlob( uniform_blob_offset_of_uniform_buffers + offset );
	}

	void Material::CopyValueToBlob( const std::byte* value, const Uniform::Information& uniform_info )
	{
		CopyValueToBlob( value, uniform_info.offset, uniform_info.size );
	}

	void Material::CopyValueToBlob( const std::byte* value, const Uniform::BufferInformation& uniform_buffer_info )
	{
		CopyValueToBlob( value, uniform_blob_offset_of_uniform_buffers + uniform_buffer_info.offset, uniform_buffer_info.size );
	}

	void Material::CopyValueToBlob( const std::byte* value, const std::size_t offset, const std::size_t size )
	{
		std::memcpy( uniform_blob.data() + offset, value, size );
	}

	void* Material::GetValuePointerFromBlob( std::size_t offset )
	{
		return uniform_blob.data() + offset;
	}

	const void* Material::GetValuePointerFromBlob( std::size_t offset ) const
	{
		return uniform_blob.data() + offset;
	}

	void Material::UploadUniform( const Uniform::Information& uniform_info )
	{
		shader->SetUniform( uniform_info, GetUniformPointer( uniform_info.offset ) );
	}

	void Material::UploadUniformBuffer( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update )
	{
		uniform_buffer_to_update.Update( GetUniformBufferPointer( uniform_buffer_info.offset ) );
	}

	void Material::UploadUniformBuffer_Partial( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update,
												const std::size_t offset, const std::size_t size )
	{
		uniform_buffer_to_update.Update_Partial( std::span( ( std::byte* )GetUniformBufferPointer( uniform_buffer_info.offset ), size ), offset );
	}

	void Material::UploadUniforms()
	{
		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( not uniform_info.is_buffer_member )
				UploadUniform( uniform_info );

		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : *uniform_buffer_info_map )
		{
			if( uniform_buffer_info.category == Uniform::BufferCategory::Regular )
				UploadUniformBuffer( uniform_buffer_info, uniform_buffer_map_regular[ uniform_buffer_name ] );
			else if( uniform_buffer_info.category == Uniform::BufferCategory::Instance )
				UploadUniformBuffer( uniform_buffer_info, uniform_buffer_map_instance[ uniform_buffer_name ] );
		}

		// TODO: Implement partial updates for Regular & Instance uniforms.
		// TODO: This means there needs to be SetPartial()< Type > (hopefully with a better name) function.
		/* Idea: 
		 * We somehow have to keep track of which sub-blob areas are dirty.
		 * Material may check if there are any dirty flags set for a given uniform buffer's sub-blob inside the main blob.
		 * If there are any dirty flags set, we do a partial update only for the sub-blobs with dirty flags set and skip the "clean" sub-blobs.
		 * If there aren't any dirty flags set, we do a full update on the whole buffer blob. */
	}

	void Material::PopulateTextureMap()
	{
		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( uniform_info.type == GL_SAMPLER_2D )
				texture_map.emplace( uniform_name, nullptr );
	}
}