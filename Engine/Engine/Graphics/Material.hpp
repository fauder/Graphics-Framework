#pragma once

// Engine Includes.
#include "Shader.hpp"
#include "UniformBuffer.h"

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

		/* Permit moving, but not copying for now. */
		Material( Material&& donor )				= default; 
		Material& operator = ( Material&& donor )	= default;

		~Material();

	/* Queries: */
		inline const std::string& Name() const { return name; }

	/* Main: */
		const Shader* Bind() const;

	/* Shader: */
		inline const std::string& GetShaderName() const
		{
			ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Material::GetShaderName() called with nullptr shader!" );
			return shader->Name();
		}
		inline bool HasShaderAssigned() const { return shader; }

		void SetShader( Shader* const shader );

	/* Uniforms: */
		inline const std::map< std::string, Uniform::Information		>& GetUniformInfoMap()			const { return *uniform_info_map;			}
		inline const std::map< std::string, Uniform::BufferInformation	>& GetUniformBufferInfoMap()	const { return *uniform_buffer_info_map;	}

			  void* GetUniformPointer( std::size_t offset );
		const void* GetUniformPointer( std::size_t offset ) const;
			  void* GetUniformBufferPointer( std::size_t offset );
		const void* GetUniformBufferPointer( std::size_t offset ) const;

		template< typename UniformType >
		void Set( const char* uniform_name, const UniformType& value )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			/* Update the value in the internal memory blob: */
			CopyValueToBlob( reinterpret_cast< const std::byte* >( &value ), uniform_info.offset, uniform_info.size );
		}

		template< typename StructType > 
			requires( std::is_base_of_v< UniformBufferTag, StructType > )
		void Set( const char* uniform_buffer_name, const StructType& value )
		{
				// TODO: Implement.
		}

	private:
	/* Shader: */
		inline const Shader* GetShader() const { return shader; };

	/* Uniform: */
		void PopulateAndSetupUniformBuffersMap();

		const Uniform::BufferInformation& GetUniformBufferInformation( const std::string& uniform_buffer_name ) const;
		const Uniform::Information& GetUniformInformation( const std::string& uniform_name ) const;

		void CopyValueToBlob( const std::byte* value, const std::size_t offset, const std::size_t size );
			  void* GetValuePointerFromBlob( std::size_t offset );
		const void* GetValuePointerFromBlob( std::size_t offset ) const;

		void UploadUniform( const Uniform::Information& uniform_info );
		void UploadUniformBuffer( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update );
		void UploadUniformBuffer_Partial( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update,
										  const std::size_t offset, const std::size_t size );
		void UploadUniforms();

	private:
		std::string name;

		Shader* shader;

		/* Allocated/Resized only when a shader is assigned.
		 * First M bytes are reserved for default block uniforms & the following N bytes are reserved for Uniform Buffers, 
		 * where M & N are queried from the shader. 
		 * For both kinds, the offset parameters point to the relevant part of memory inside the blob. */
		std::vector< std::byte > uniform_blob;

		std::size_t uniform_blob_offset_of_uniform_buffers;

		/* Map pointers below are assigned only when the Shader itself is assigne to the Material, through Shader::GetUniformInfoMap() & Shader::GetUniformBufferInfoMap() respectively. */
		const std::map< std::string, Uniform::Information		>* uniform_info_map;
		const std::map< std::string, Uniform::BufferInformation >* uniform_buffer_info_map;

		std::map< std::string, UniformBuffer > uniform_buffer_map_regular;
		std::map< std::string, UniformBuffer > uniform_buffer_map_instance;
	};
}
