#pragma once

// Engine Includes.
#include "Drawable.h"
#include "Core/BitFlags.hpp"
#include "Core/DirtyBlob.h"
#include "Scene/Camera.h"
#include "Lighting/DirectionalLight.h"
#include "Lighting/PointLight.h"
#include "Lighting/SpotLight.h"
#include "UniformBufferManagement.hpp"

// std Includes.
#include <unordered_map>
#include <vector>

namespace Engine
{
	class Renderer
	{
	public:
		enum class ClearTarget : unsigned int
		{
			None = 0,

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
		void RenderImGui();
		void OnProjectionParametersChange( Camera& camera );

	/* Drawables: */
		void AddDrawable( const Drawable* drawable_to_add );
		// TODO: Switch to unsigned map of "Component" UUIDs when Component class is implemented.
		// For now, stick to removing elements from a vector, which is sub-par performance but should be OK for the time being.
		void RemoveDrawable( const Drawable* drawable_to_remove );
		void RemoveAllDrawables();

	/* Lights: */
		void AddDirectionalLight( DirectionalLight* light_to_add );
		void RemoveDirectionalLight();
		/* Only 1 Directional Light can be active at a time! */

		void AddPointLight( PointLight* light_to_add );
		void RemovePointLight( PointLight* light_to_remove );
		void RemoveAllPointLights();

		void AddSpotLight( SpotLight* light_to_add );
		void RemoveSpotLight( SpotLight* light_to_remove );
		void RemoveAllSpotLights();

	/* Shaders: */
		const void* GetShaderGlobal( const std::string& buffer_name ) const;
			  void* GetShaderGlobal( const std::string& buffer_name );

		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetShaderGlobal( const std::string& buffer_name, const StructType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, value );
		}

		/* For PARTIAL setting of ARRAY uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetShaderGlobal( const std::string& buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const StructType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, uniform_member_array_instance_name, array_index, value );
		}

		/* For PARTIAL setting of STRUCT uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetShaderGlobal( const std::string& buffer_name, const char* uniform_member_struct_instance_name, const StructType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, uniform_member_struct_instance_name, value );
		}
		
		/* For PARTIAL setting of NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		template< typename UniformType >
		void SetShaderGlobal( const std::string& buffer_name, const char* uniform_member_name, const UniformType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, uniform_member_name, value );
		}

	/* Clearing: */
		void SetClearColor( const Color3& new_clear_color );
		void SetClearColor( const Color4& new_clear_color );

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
	
		void UploadIntrinsics();
		void UploadGlobals();

	/* Shaders: */
		void RegisterShader( const Shader& shader );
		void UnregisterShader( const Shader& shader );

	/* Clearing: */
		void SetClearColor();
		void Clear() const;

	private:
		std::vector< const Drawable* > drawable_list;

		DirectionalLight*			light_directional;
		std::vector< PointLight* >	lights_point;
		std::vector< SpotLight*	 >	lights_spot;
		int lights_point_active_count;
		int lights_spot_active_count;

		std::unordered_map< Shader::ID, const Shader* > shaders_registered;

		std::unordered_map< Shader::ID,  Shader* > shaders_in_flight;
		std::unordered_map< std::string, Material* > materials_in_flight; // TODO: Generate an ID for Materials (who will generate it?) and use that ID as the key here.

		Color4 clear_color;
		BitFlags< ClearTarget > clear_targets;

		UniformBufferManagement< DirtyBlob > uniform_buffer_management_global;
		UniformBufferManagement< DirtyBlob > uniform_buffer_management_intrinsic;
	};
}
