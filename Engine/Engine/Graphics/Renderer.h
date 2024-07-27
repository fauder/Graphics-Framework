#pragma once

// Engine Includes.
#include "Drawable.h"
#include "Core/DirtyBlob.h"
#include "Scene/Camera.h"

// std Includes.
#include <unordered_map>
#include <vector>

namespace Engine
{
	class Renderer
	{
	public:
		enum class ClearTarget
		{
			DepthBuffer   = GL_DEPTH_BUFFER_BIT,
			StencilBuffer = GL_STENCIL_BUFFER_BIT,
			ColorBuffer   = GL_COLOR_BUFFER_BIT,
		};

		enum class PolygonMode
		{
			Point = GL_POINT,
			Line  = GL_LINE, 
			Fill  = GL_FILL,
			
			Wireframe = Line
		};

	public:
		Renderer();
		~Renderer();

	/* Main: */
		void Update( Camera& camera );
		void Render( Camera& camera );
		void OnProjectionParametersChange( Camera& camera );

	/* Drawables: */
		void AddDrawable( Drawable* drawable_to_add );
		// TODO: Switch to unsigned map of "Component" UUIDs when Component class is implemented.
		// For now, stick to removing elements from a vector, which is sub-par performance but should be OK for the time being.
		void RemoveDrawable( const Drawable* drawable_to_remove );
		void RemoveAllDrawables();

	/* Shaders: */
		// TODO: Provide Global setting/getting functions.

	/* Clearing: */
		void SetClearColor( const Color3& new_clear_color );
		void SetClearColor( const Color4& new_clear_color );

		void SetClearTarget( const ClearTarget& target );
		void SetClearTargets( const ClearTarget& target, std::same_as< ClearTarget > auto&& ... other_targets )
		{
			clear_target = ClearTarget( ( int )target | ( ( int )other_targets | ... ) );
		}

	/* Depth Test: */
		void EnableDepthTest();
		void DisableDepthTest();

	/* Other: */
		void SetPolygonMode( const PolygonMode mode );

	private:
	/* Main: */
		void Render( const Mesh& mesh );
		void Render_Indexed( const Mesh& mesh );
		void Render_NonIndexed( const Mesh& mesh );

	/* Intrinsic Uniforms: */
		const void* GetIntrinsic( const char* uniform_buffer_name, const Uniform::BufferInformation& uniform_buffer_info ) const;
			  void* GetIntrinsic( const char* uniform_buffer_name, const Uniform::BufferInformation& uniform_buffer_info );

		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetIntrinsic( const char* uniform_buffer_name, const StructType& value )
		{
			const auto& uniform_buffer_info = *uniform_buffer_info_map_intrinsic[ uniform_buffer_name ];

			uniform_blob_map_intrinsic[ uniform_buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), uniform_buffer_info.offset, uniform_buffer_info.size );
		}

		/* For PARTIAL setting ARRAY uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetIntrinsic( const char* uniform_buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const StructType& value )
		{
			const auto& uniform_buffer_info              = *uniform_buffer_info_map_intrinsic[ uniform_buffer_name ];
			const auto& uniform_buffer_member_array_info = uniform_buffer_info.members_array_map.at( uniform_member_array_instance_name );

			const auto effective_offset = uniform_buffer_member_array_info.offset + 
										  array_index * uniform_buffer_member_array_info.stride;

			/* Update the value in the internal memory blob: */
			uniform_blob_map_intrinsic[ uniform_buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, uniform_buffer_member_array_info.stride );
		}

		/* For PARTIAL setting STRUCT uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetIntrinsic( const char* uniform_buffer_name, const char* uniform_member_struct_instance_name, const StructType& value )
		{
			const auto& uniform_buffer_info               = *uniform_buffer_info_map_intrinsic[ uniform_buffer_name ];
			const auto& uniform_buffer_member_struct_info = uniform_buffer_info.members_struct_map.at( uniform_member_struct_instance_name );

			const auto effective_offset = uniform_buffer_member_struct_info.offset;

			/* Update the value in the internal memory blob: */
			uniform_blob_map_intrinsic[ uniform_buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, uniform_buffer_member_struct_info.size );
		}

		/* For PARTIAL setting NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		template< typename UniformType >
		void SetIntrinsic( const char* uniform_buffer_name, const char* uniform_member_name, const UniformType& value )
		{
			const auto& uniform_buffer_info = *uniform_buffer_info_map_intrinsic[ uniform_buffer_name ];
			const auto& uniform_info        = uniform_buffer_info.members_single_map.at( uniform_member_name );

			/* Update the value in the internal memory blob: */
			uniform_blob_map_intrinsic[ uniform_buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), uniform_info->offset, uniform_info->size );
		}

		void UploadIntrinsics();

	/* Shaders: */
		void RegisterShader( const Shader& shader );
		void UnregisterShader( const Shader& shader );

	/* Clearing: */
		void SetClearColor();
		void Clear() const;

	private:
		std::vector< Drawable* > drawable_list;

		std::unordered_map< Shader::ID, const Shader* > shaders_registered;

		std::unordered_map< Shader::ID,  Shader* > shaders_in_flight;
		std::unordered_map< std::string, Material* > materials_in_flight; // TODO: Generate an ID for Materials (who will generate it?) and use that ID as the key here.

		Color4 clear_color;
		ClearTarget clear_target;

		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_regular;
		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_instance;
		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_global;
		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_intrinsic;

		//std::unordered_map< std::string, const Uniform::BufferInformation > uniform_buffer_info_map_global;
		std::unordered_map< std::string, const Uniform::BufferInformation* > uniform_buffer_info_map_intrinsic;

		//std::unordered_map< std::string, DirtyBlob > uniform_blob_map_global;
		std::unordered_map< std::string, DirtyBlob > uniform_blob_map_intrinsic;
	};
}