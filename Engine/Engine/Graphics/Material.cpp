// Engine Includes.
#include "Material.hpp"

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
		uniform_blob( shader->GetTotalUniformSize_ForMaterial() ),
		uniform_blob_offset_of_uniform_buffers( shader->GetTotalUniformSize_DefaultBlockOnly() ),
		uniform_info_map( &shader->GetUniformInfoMap() ),
		uniform_buffer_info_map( &shader->GetUniformBufferInfoMap_Regular() )
	{
		ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Parameter 'shader' passed to Material::Material( const std::string& name, Shader* const shader ) is nullptr!" );

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

		uniform_blob                           = Blob( shader->GetTotalUniformSize_ForMaterial() );
		uniform_blob_offset_of_uniform_buffers = shader->GetTotalUniformSize_DefaultBlockOnly();

		uniform_info_map        = ( &shader->GetUniformInfoMap() );
		uniform_buffer_info_map = ( &shader->GetUniformBufferInfoMap_Regular() );

		PopulateTextureMap();
	}

	const void* Material::Get( const Uniform::Information& uniform_info ) const
	{
		ASSERT_DEBUG_ONLY( not uniform_info.is_buffer_member &&
						   "Material::Get( const Uniform::Information& ) called to obtain value of a UBO member.\n"
						   "Call Material::Get( const Uniform::BufferInformation& ) version instead." );

		return uniform_blob.Get( uniform_info.offset );
	}

	void* Material::Get( const Uniform::Information& uniform_info )
	{
		ASSERT_DEBUG_ONLY( not uniform_info.is_buffer_member &&
						   "Material::Get( const Uniform::Information& ) called to obtain value of a UBO member.\n"
						   "Call Material::Get( const Uniform::BufferInformation& ) version instead." );

		return ReadFromBlob_Uniform( uniform_info.offset );
	}

	const void* Material::Get( const Uniform::BufferInformation& uniform_buffer_info ) const
	{
		return ReadFromBlob_UniformBuffer( uniform_buffer_info.offset );
	}

	void* Material::Get( const Uniform::BufferInformation& uniform_buffer_info )
	{
		return ReadFromBlob_UniformBuffer( uniform_buffer_info.offset );
	}

/*
 *
 *	PRIVATE API:
 *
 */

	void Material::SetTexture( const char* sampler_name_of_new_texture, Texture* texture_to_be_set )
	{
		if( const auto found = texture_map.find( sampler_name_of_new_texture ); 
			found != texture_map.cend() )
		{
			found->second = texture_to_be_set;
		}
		else
		{
			throw std::runtime_error( R"(ERROR::MATERIAL::SetTexture(): given sampler name ")" + std::string( sampler_name_of_new_texture ) + R"(" does not exist!)" );
		}
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

	void Material::CacheUniformBufferMap( const std::unordered_map< std::string, UniformBuffer >& regular_uniform_buffers_map )
	{
		for( const auto& [ uniform_buffer_name, uniform_buffer ] : regular_uniform_buffers_map )
			uniform_buffer_map_regular.try_emplace( uniform_buffer_name, &uniform_buffer );

		// TODO: Cache Instance Uniform Buffers too, IF Material will take care of them.
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

	void* Material::ReadFromBlob_Uniform( std::size_t offset )
	{
		return uniform_blob.Get( offset );
	}

	const void* Material::ReadFromBlob_Uniform( std::size_t offset ) const
	{
		return uniform_blob.Get( offset );
	}

	void* Material::ReadFromBlob_UniformBuffer( std::size_t offset )
	{
		return uniform_blob.Get( uniform_blob_offset_of_uniform_buffers + offset );
	}

	const void* Material::ReadFromBlob_UniformBuffer( std::size_t offset ) const
	{
		return uniform_blob.Get( uniform_blob_offset_of_uniform_buffers + offset );
	}

	void Material::CopyToBlob_Uniform( const std::byte* value, const Uniform::Information& uniform_info )
	{
		uniform_blob.Set( value, uniform_info.offset, uniform_info.size );
	}

	void Material::CopyToBlob_UniformBuffer( const std::byte* value, const Uniform::BufferInformation& uniform_buffer_info )
	{
		uniform_blob.Set( value, uniform_blob_offset_of_uniform_buffers + uniform_buffer_info.offset, uniform_buffer_info.size );
	}

	void Material::UploadUniform( const Uniform::Information& uniform_info )
	{
		shader->SetUniform( uniform_info, ReadFromBlob_Uniform( uniform_info.offset ) );
	}

	void Material::UploadUniformBuffer( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update )
	{
		uniform_buffer_to_update.Update( ReadFromBlob_UniformBuffer( uniform_buffer_info.offset ) );
	}

	void Material::UploadUniformBuffer_Partial( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update,
												const std::size_t offset, const std::size_t size )
	{
		uniform_buffer_to_update.Update_Partial( std::span( ( std::byte* )ReadFromBlob_UniformBuffer( uniform_buffer_info.offset ), size ), offset );
	}

	void Material::UploadUniforms()
	{
		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( not uniform_info.is_buffer_member )
				UploadUniform( uniform_info );

		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : *uniform_buffer_info_map )
		{
			if( uniform_buffer_info.category == Uniform::BufferCategory::Regular )
				UploadUniformBuffer( uniform_buffer_info, *uniform_buffer_map_regular[ uniform_buffer_name ] );
		}

		// TODO: Upload Instance Uniforms *somewhere*.

		unsigned int texture_unit_slots_in_use = 0; // This can be controlled via a central manager class if more complex use-cases arise. For now every Material will act as if it is the only one using Texture Unit slots.

		/* Copy texture slot to blob, activate the slot & upload the slot uniform to GPU. */
		for( auto& [ sampler_name, texture ] : texture_map )
		{
			const auto& sampler_uniform_info = uniform_info_map->at( sampler_name );
			const unsigned int texture_unit_slot = texture_unit_slots_in_use++;

			CopyToBlob_Uniform( ( const std::byte* )&texture_unit_slot, sampler_uniform_info );

			texture->Activate( texture_unit_slot );

			UploadUniform( sampler_uniform_info );
		}

		// TODO: Implement partial updates for Regular & Instance uniforms.
	}

	void Material::PopulateTextureMap()
	{
		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( uniform_info.type == GL_SAMPLER_2D )
				texture_map.emplace( uniform_name, nullptr );
	}
}