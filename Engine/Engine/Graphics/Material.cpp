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
		uniform_blob( shader->GetTotalUniformSize() ),
		uniform_info_map( &shader->GetUniformInformations() )
	{
		ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Parameter 'shader' passed to Material::Material( const std::string& name, Shader* const shader ) is nullptr!" );
	}

	Material::~Material()
	{
	}

	const Shader* Material::Bind() const
	{
		shader->Bind(); 
		return shader;
	}

	void* Material::GetUniformPointer( std::size_t offset )
	{
		return GetValuePointerFromBlob( offset );
	}

	const void* Material::GetUniformPointer( std::size_t offset ) const
	{
		return GetValuePointerFromBlob( offset );
	}

	void Material::SetShader( Shader* const shader )
	{
		this->shader     = shader;
		uniform_info_map = ( &shader->GetUniformInformations() );
		uniform_blob     = std::vector< char >( shader->GetTotalUniformSize() );
	}

	void Material::CopyValueToBlob( const char* value, const std::size_t offset, const std::size_t size )
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

	/* Uploads the value in the internal memory blob to GPU. */
	void Material::UploadUniform( const char* uniform_name )
	{
		const auto& uniform_info = GetUniformInformation( uniform_name );

		/*if( uniform_info.IsUserDefinedStruct() )
		{
			for( const auto& [ member_uniform_name, member_uniform_info ] : uniform_info.members )
				shader->SetUniform( *member_uniform_info, GetValuePointerFromBlob( member_uniform_info->original_offset ) );
		}
		else*/
			shader->SetUniform( uniform_info, GetValuePointerFromBlob( uniform_info.original_offset == -1 ? uniform_info.offset : uniform_info.original_offset ) );
	}

	/* Uploads the values in the internal memory blob to GPU. */
	void Material::UploadAllUniforms()
	{
		for( const auto& [ uniform_info_name, uniform_info ] : *uniform_info_map )
			if( not uniform_info.IsUserDefinedStruct() ) // Skip structs to prevent duplicate processing of members. Can not upload the whole struct at once anyways.
				UploadUniform( uniform_info_name.c_str() );
	}
}