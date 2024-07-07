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

	void* Material::GetUniformPointer( std::size_t offset )
	{
		return GetValuePointerFromBlob( offset );
	}

	const void* Material::GetUniformPointer( std::size_t offset ) const
	{
		return GetValuePointerFromBlob( offset );
	}

	/* Uses the value in the internal memory blob. */
	void Material::Set( const char* uniform_name )
	{
		const auto& uniform_info = GetUniformInformation( uniform_name );

		if( uniform_info.IsUserDefinedStruct() )
		{
			for( const auto& [ member_uniform_name, member_uniform_info ] : uniform_info.members )
				shader->SetUniform( *member_uniform_info, GetValuePointerFromBlob( member_uniform_info->original_offset ) );
		}
		else
			shader->SetUniform( uniform_info, GetValuePointerFromBlob( uniform_info.offset ) );
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
}