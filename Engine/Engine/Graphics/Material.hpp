#pragma once

// Engine Includes.
#include "Shader.hpp"

// std Includes.
#include <vector>

namespace Engine
{
	class Material
	{
	public:
		Material();
		Material( const std::string& name );
		Material( const std::string& name, Shader* const shader );

		~Material();

		inline const std::string& GetName() const { return name; }
		inline bool HasShaderAssigned() const { return shader; }
		inline const std::string& GetShaderName() const
		{
			ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Material::GetShaderName() called with nullptr shader!" );
			return shader->GetName();
		}
		inline const std::map< std::string, Uniform::Information >& GetUniformInformations() const { return *uniform_info_map; }

			  void* GetUniformPointer( std::size_t offset );
		const void* GetUniformPointer( std::size_t offset ) const;

		template< typename UniformType >
		void Set( const char* uniform_name, const UniformType& value )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			/* Update the value in the internal memory blob: */
			CopyValueToBlob( reinterpret_cast< const char* >( &value ), uniform_info.offset, uniform_info.size );

			/* Have to call the template version in case of user-defined structs (there is no "upload the whole struct directly to gpu" method anyway, at least not for regular uniform structs). */
			if constexpr( std::is_base_of_v< UniformStruct, UniformType > )
				shader->SetUniform( uniform_name, value );
			else
				shader->SetUniform( uniform_info.location, value );
		}

		/* Uses the value in the internal memory blob. */
		void Set( const char* uniform_name );

		void SetShader( Shader* const shader );

	private:
		const Uniform::Information& GetUniformInformation( const std::string& uniform_name )
		{
			try
			{
				return uniform_info_map->at( uniform_name );
			}
			catch( const std::exception& )
			{
				throw std::runtime_error( R"(ERROR::MATERIAL::UNIFORM::")" + std::string( uniform_name ) + R"("::DOES_NOT_EXIST)" );
			}
		}

		void CopyValueToBlob( const char* value, const std::size_t offset, const std::size_t size );
			  void* GetValuePointerFromBlob( std::size_t offset );
		const void* GetValuePointerFromBlob( std::size_t offset ) const;

	private:
		std::string name;

		Shader* shader;

		std::vector< char > uniform_blob; // Resized only when a shader is assigned and the total uniform size is (re)calculated.
		const std::map< std::string, Uniform::Information >* uniform_info_map; // Obtain only throuh Shader::GetUniformInformations(), only when a shader is assigned.
	};
}
