#pragma once

// Engine Includes.
#include "Shader.hpp"
#include "UniformBuffer.h"
#include "Texture.h"
#include "Core/Blob.hpp"

// std Includes.
#include <cstddef> // std::byte.
#include <unordered_map>
#include <vector>

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
		inline const std::unordered_map< std::string, Uniform::Information			>& GetUniformInfoMap()			const { return *uniform_info_map;			}
		inline const std::unordered_map< std::string, Uniform::BufferInformation	>& GetUniformBufferInfoMap()	const { return *uniform_buffer_info_map;	}
		inline const std::unordered_map< std::string, Texture*						>& GetTextureMap()				const { return texture_map;					}

		const void* Get( const Uniform::Information& uniform_info ) const;
			  void* Get( const Uniform::Information& uniform_info );
		const void* Get( const Uniform::BufferInformation& uniform_buffer_info ) const;
			  void* Get( const Uniform::BufferInformation& uniform_buffer_info );
		
		template< typename UniformType >
		void Set( const char* uniform_name, const UniformType& value )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			CopyToBlob_Uniform( reinterpret_cast< const std::byte* >( &value ), uniform_info );
		}

		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const char* uniform_buffer_name, const StructType& value )
		{
			const auto& uniform_buffer_info = GetUniformBufferInformation( uniform_buffer_name );

			CopyToBlob_UniformBuffer( reinterpret_cast< const std::byte* >( &value ), uniform_buffer_info );
		}

		/* For PARTIAL setting ARRAY uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const char* uniform_buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const StructType& value )
		{
			const auto& uniform_buffer_info              = GetUniformBufferInformation( uniform_buffer_name );
			const auto& uniform_buffer_member_array_info = uniform_buffer_info.members_array_map.at( uniform_member_array_instance_name );

			const auto effective_offset = uniform_blob_offset_of_uniform_buffers + uniform_buffer_info.offset + uniform_buffer_member_array_info.offset + 
										  array_index * uniform_buffer_member_array_info.stride;

			/* Update the value in the internal memory blob: */
			uniform_blob.Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, uniform_buffer_member_array_info.stride );
		}

		/* For PARTIAL setting STRUCT uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const char* uniform_buffer_name, const char* uniform_member_struct_instance_name, const StructType& value )
		{
			const auto& uniform_buffer_info               = GetUniformBufferInformation( uniform_buffer_name );
			const auto& uniform_buffer_member_struct_info = uniform_buffer_info.members_struct_map.at( uniform_member_struct_instance_name );

			const auto effective_offset = uniform_blob_offset_of_uniform_buffers + uniform_buffer_info.offset + uniform_buffer_member_struct_info.offset;

			/* Update the value in the internal memory blob: */
			uniform_blob.Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, uniform_buffer_member_struct_info.size );
		}

		/* For PARTIAL setting NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		template< typename UniformType >
		void Set( const char* uniform_buffer_name, const char* uniform_member_name, const UniformType& value )
		{
			const auto& uniform_buffer_info = GetUniformBufferInformation( uniform_buffer_name );
			const auto& uniform_info        = uniform_buffer_info.members_single_map.at( uniform_member_name );

			const auto effective_offset = uniform_blob_offset_of_uniform_buffers + uniform_buffer_info.offset + uniform_info->offset;

			/* Update the value in the internal memory blob: */
			uniform_blob.Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, uniform_info->size );
		}

	/* Textures: */
		void SetTexture( const char* sampler_name_of_new_texture, Texture* texture_to_be_set );
		const Texture* GetTexture( const char* sampler_name_of_new_texture ) const;

	private:
	/* Shader: */
		inline const Shader* GetShader() const { return shader; };

	/* Uniform: */
		void CacheUniformBufferMap( const std::unordered_map< std::string, UniformBuffer >& regular_uniform_buffers_map );

		const Uniform::BufferInformation& GetUniformBufferInformation( const std::string& uniform_buffer_name ) const;
		const Uniform::Information& GetUniformInformation( const std::string& uniform_name ) const;

			  void* ReadFromBlob_Uniform( std::size_t offset );
		const void* ReadFromBlob_Uniform( std::size_t offset ) const;
			  void* ReadFromBlob_UniformBuffer( std::size_t offset );
		const void* ReadFromBlob_UniformBuffer( std::size_t offset ) const;

		void CopyToBlob_Uniform( const std::byte* value, const Uniform::Information& uniform_info );
		void CopyToBlob_UniformBuffer( const std::byte* value, const Uniform::BufferInformation& uniform_buffer_info );

		void UploadUniform( const Uniform::Information& uniform_info );
		void UploadUniformBuffer( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update );
		void UploadUniformBuffer_Partial( const Uniform::BufferInformation& uniform_buffer_info, const UniformBuffer& uniform_buffer_to_update,
										  const std::size_t offset, const std::size_t size );
		void UploadUniforms();

	/* Texture: */
		void PopulateTextureMap();

	private:
		std::string name;

		Shader* shader;

		/* Allocated/Resized only when a shader is assigned.
		 * First M bytes are reserved for default block uniforms & the following N bytes are reserved for Uniform Buffers, 
		 * where M & N are queried from the shader. 
		 * For both kinds, the offset parameters point to the relevant part of memory inside the blob. */
		Blob uniform_blob;

		std::size_t uniform_blob_offset_of_uniform_buffers;

		/* Map pointers below are assigned only when the Shader itself is assigne to the Material, through Shader::GetUniformInfoMap() & Shader::GetUniformBufferInfoMap() respectively. */
		const std::unordered_map< std::string, Uniform::Information			>* uniform_info_map;
		const std::unordered_map< std::string, Uniform::BufferInformation	>* uniform_buffer_info_map;

		std::unordered_map< std::string, const UniformBuffer* > uniform_buffer_map_regular;
		// std::unordered_map< std::string, const UniformBuffer* > uniform_buffer_map_instance;

		std::unordered_map< std::string, Texture* > texture_map;
	};
}
