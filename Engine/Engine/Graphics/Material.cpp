// Engine Includes.
#include "Material.hpp"

namespace Engine
{
	Material::Material()
		:
		name( "<unnamed>" ),
		shader( nullptr ),
		uniform_info_map( nullptr )
	{
	}

	Material::Material( const std::string& name )
		:
		name( name ),
		shader( nullptr ),
		uniform_info_map( nullptr )
	{
	}

	Material::Material( const std::string& name, Shader* const shader )
		:
		name( name ),
		shader( shader ),
		uniform_blob_default_block( shader->GetTotalUniformSize_DefaultBlockOnly() ),
		uniform_info_map( &shader->GetUniformInfoMap() )
	{
		ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Parameter 'shader' passed to Material::Material( const std::string& name, Shader* const shader ) is nullptr!" );
		
		const auto& uniform_buffer_info_map = shader->GetUniformBufferInfoMap_Regular();

		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
			uniform_buffer_management_regular.RegisterBuffer( uniform_buffer_name, &uniform_buffer_info );

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

		uniform_blob_default_block = Blob( shader->GetTotalUniformSize_DefaultBlockOnly() );

		uniform_info_map = ( &shader->GetUniformInfoMap() );

		const auto& uniform_buffer_info_map = shader->GetUniformBufferInfoMap_Regular();

		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
			uniform_buffer_management_regular.RegisterBuffer( uniform_buffer_name, &uniform_buffer_info );

		PopulateTextureMap();
	}

	const void* Material::Get( const Uniform::Information& uniform_info ) const
	{
		ASSERT_DEBUG_ONLY( not uniform_info.is_buffer_member &&
						   "Material::Get( const Uniform::Information& ) called to obtain value of a UBO member.\n"
						   "Call Material::Get( const Uniform::BufferInformation& ) version instead." );

		return uniform_blob_default_block.Get( uniform_info.offset );
	}

	void* Material::Get( const Uniform::Information& uniform_info )
	{
		ASSERT_DEBUG_ONLY( not uniform_info.is_buffer_member &&
						   "Material::Get( const Uniform::Information& ) called to obtain value of a UBO member.\n"
						   "Call Material::Get( const Uniform::BufferInformation& ) version instead." );

		return uniform_blob_default_block.Get( uniform_info.offset );
	}

	const void* Material::Get( const std::string& uniform_buffer_name ) const
	{
		return uniform_buffer_management_regular.Get( uniform_buffer_name );
	}

	void* Material::Get( const std::string& uniform_buffer_name )
	{
		return uniform_buffer_management_regular.Get( uniform_buffer_name );
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

	void Material::UploadUniform( const Uniform::Information& uniform_info )
	{
		shader->SetUniform( uniform_info, uniform_blob_default_block.Get( uniform_info.offset ) );
	}

	void Material::UploadUniforms()
	{
		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( not uniform_info.is_buffer_member )
				UploadUniform( uniform_info );

		uniform_buffer_management_regular.UploadAll();

		unsigned int texture_unit_slots_in_use = 0; // This can be controlled via a central manager class if more complex use-cases arise. For now every Material will act as if it is the only one using Texture Unit slots.

		/* Copy texture slot to blob, activate the slot & upload the slot uniform to GPU. */
		for( auto& [ sampler_name, texture ] : texture_map )
		{
			if( texture )
			{
				const auto& sampler_uniform_info = uniform_info_map->at( sampler_name );
				const unsigned int texture_unit_slot = texture_unit_slots_in_use++;

				uniform_blob_default_block.Set( ( const std::byte* )&texture_unit_slot, sampler_uniform_info.offset, sampler_uniform_info.size );

				texture->Activate( texture_unit_slot );

				UploadUniform( sampler_uniform_info );
			}
		}
	}

	void Material::PopulateTextureMap()
	{
		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( uniform_info.type == GL_SAMPLER_2D )
				texture_map.emplace( uniform_name, nullptr );
	}
}