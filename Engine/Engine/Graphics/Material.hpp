#pragma once

// Engine Includes.
#include "Shader.hpp"

// std Includes.
#include <vector>
#include <cstddef> // std::byte.

namespace Engine
{
	class Renderer;

	class Material
	{
		friend Renderer;

	public:
		Material();
		Material( const std::string& name );
		Material( const std::string& name, Shader* const shader );

		~Material();

	/* Queries: */
		inline const std::string& GetName() const { return name; }

	/* Main: */
		const Shader* Bind() const;

	/* Shader: */
		inline const std::string& GetShaderName() const
		{
			ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Material::GetShaderName() called with nullptr shader!" );
			return shader->GetName();
		}
		inline bool HasShaderAssigned() const { return shader; }

		void SetShader( Shader* const shader );

	/* Uniforms: */
		inline const std::map< std::string, Uniform::Information_Old >& GetUniformInformations() const { return *uniform_info_map; }

			  void* GetUniformPointer( std::size_t offset );
		const void* GetUniformPointer( std::size_t offset ) const;

		template< typename UniformType >
		void Set( const char* uniform_name, const UniformType& value )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			/* Update the value in the internal memory blob: */
			CopyValueToBlob( reinterpret_cast< const std::byte* >( &value ), uniform_info.offset, uniform_info.size );

			///* Have to call the template version in case of user-defined structs (there is no "upload the whole struct directly to gpu" method anyway, at least not for regular uniform structs). */
			//if constexpr( std::is_base_of_v< UniformStruct, UniformType > )
			//	shader->SetUniform( uniform_name, value );
			//else
			//	shader->SetUniform( uniform_info.location, value );
		}

	private:
	/* Shader: */
		inline const Shader* GetShader() const { return shader; };

	/* Uniform: */
		const Uniform::Information_Old& GetUniformInformation( const std::string& uniform_name )
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

		void CopyValueToBlob( const std::byte* value, const std::size_t offset, const std::size_t size );
			  void* GetValuePointerFromBlob( std::size_t offset );
		const void* GetValuePointerFromBlob( std::size_t offset ) const;

		/* Uses the value in the internal memory blob. */
		void UploadUniform( const char* uniform_name );
		void UploadAllUniforms();

	private:
		std::string name;

		Shader* shader;

		std::vector< std::byte > uniform_blob; // Resized only when a shader is assigned and the total uniform size is (re)calculated.
		const std::map< std::string, Uniform::Information_Old >* uniform_info_map; // Obtain only through Shader::GetUniformInformations(), only when a shader is assigned.
	};
}
