// Engine Includes.
#include "Material.hpp"
#include "InternalTextures.h"

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
			uniform_buffer_management_regular.RegisterBuffer( uniform_buffer_name, uniform_buffer_info );

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

	Shader* Material::GetShader()
	{
		return shader;
	}

	void Material::SetShader( Shader* shader )
	{
		/* Clean-up of previous data: */
		uniform_buffer_management_regular.UnregisterAllBuffers();

		/* Setting new data: */
		this->shader = shader;

		uniform_blob_default_block = Blob( shader->GetTotalUniformSize_DefaultBlockOnly() );

		uniform_info_map = ( &shader->GetUniformInfoMap() );

		const auto& uniform_buffer_info_map = shader->GetUniformBufferInfoMap_Regular();

		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
			uniform_buffer_management_regular.RegisterBuffer( uniform_buffer_name, uniform_buffer_info );

		RepopulateTextureMap();
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

	void Material::SetTexture( const char* sampler_name_of_new_texture, const Texture* texture_to_be_set )
	{
#ifdef _DEBUG
		if( const auto found = texture_map.find( sampler_name_of_new_texture ); 
			found != texture_map.cend() )
		{
			found->second = texture_to_be_set;
		}
		else
		{
			ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": Texture ")" + std::string( sampler_name_of_new_texture ) + R"(" can not be set; it does not exist!)" );
		}
#else
		texture_map.at( sampler_name_of_new_texture ).second = texture_to_be_set;
		}
#endif // _DEBUG
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
		if( uniform_info.count_array > 1 )
			shader->SetUniformArray( uniform_info, uniform_blob_default_block.Get( uniform_info.offset ) );
		else
			shader->SetUniform( uniform_info, uniform_blob_default_block.Get( uniform_info.offset ) );
	}

	void Material::UploadUniforms()
	{
		unsigned int texture_unit_slots_in_use = 0; // This can be controlled via a central manager class if more complex use-cases arise. For now every Material will act as if it is the only one using Texture Unit slots.

		auto UploadTexture = [ & ]( const std::string& sampler_name, const Texture& texture )
		{
#if _DEBUG
			if( not uniform_info_map->contains( sampler_name ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": Uniform ")" + std::string( sampler_name ) + R"(" can not be uploaded; it does not exist!)" );
				return;
			}
#endif

			const auto& sampler_uniform_info     = uniform_info_map->at( sampler_name );
			const unsigned int texture_unit_slot = texture_unit_slots_in_use++;

			uniform_blob_default_block.Set( ( const std::byte* )&texture_unit_slot, sampler_uniform_info.offset, sampler_uniform_info.size );

			texture.Activate( texture_unit_slot );
		};

		/* Copy texture slot to blob, activate the slot & upload the slot uniform to GPU. */
		for( auto& [ sampler_name, texture ] : texture_map )
		{
			if( texture )
			{
				UploadTexture( sampler_name, *texture );
			}
#ifdef _DEBUG
			else
			{
				if( sampler_name.find( "normal_map" ) != std::string::npos )
				{
					UploadTexture( sampler_name, *InternalTextures::Get( "Normal Map" ) );
				}
				else
				{
					UploadTexture( sampler_name, *InternalTextures::Get( "Missing" ) );
				}
			}
#endif // _DEBUG
		}

		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( not uniform_info.is_buffer_member )
				UploadUniform( uniform_info );

		uniform_buffer_management_regular.UploadAll();
	}

	void Material::PopulateTextureMap()
	{
		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( uniform_info.type == GL_SAMPLER_2D || uniform_info.type == GL_SAMPLER_2D_MULTISAMPLE || uniform_info.type == GL_SAMPLER_CUBE )
				texture_map.emplace( uniform_name, nullptr );
	}

	void Material::RepopulateTextureMap()
	{
		std::erase_if( texture_map, [ & ]( const auto& name_texture_pair ) { return not uniform_info_map->contains( name_texture_pair.first ); } );

		for( const auto& [ uniform_name, uniform_info ] : *uniform_info_map )
			if( ( uniform_info.type == GL_SAMPLER_2D || uniform_info.type == GL_SAMPLER_2D_MULTISAMPLE || uniform_info.type == GL_SAMPLER_CUBE ) &&
				not texture_map.contains( uniform_name ) )
				texture_map.emplace( uniform_name, nullptr );
	}
}