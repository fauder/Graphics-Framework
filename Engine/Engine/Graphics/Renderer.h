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
#include <map>
#include <unordered_map>
#include <unordered_set>
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

			All = DepthBuffer | StencilBuffer | ColorBuffer
		};

		enum class PolygonMode
		{
			Point = GL_POINT,
			Line  = GL_LINE, 
			Fill  = GL_FILL,
			
			Wireframe = Line
		};

		enum class ComparisonFunction
		{
			Always	       = GL_ALWAYS,
			Never 	       = GL_NEVER,
			Equal 	       = GL_EQUAL,
			NotEqual       = GL_NOTEQUAL,
			Less 	       = GL_LESS,
			LessOrEqual    = GL_LEQUAL,
			Greater        = GL_GREATER,
			GreaterOrEqual = GL_GEQUAL
		};

		enum class StencilTestResponse
		{
			Keep          = GL_KEEP,
			Zero          = GL_ZERO,
			Replace       = GL_REPLACE,
			Increment     = GL_INCR,
			IncrementWrap = GL_INCR_WRAP,
			Decrement     = GL_DECR,
			DecrementWrap = GL_DECR_WRAP,
			Invert        = GL_INVERT
		};

		struct RenderState
		{
			bool depth_test_enable                       = true;
			bool depth_write_enable                      = true;
			ComparisonFunction depth_comparison_function = ComparisonFunction::Less;

			bool stencil_test_enable                       = false;
			unsigned int stencil_write_mask                = true;
			ComparisonFunction stencil_comparison_function = ComparisonFunction::Always;
			unsigned int stencil_ref                       = 0;
			unsigned int stencil_mask                      = 0xFF;

			StencilTestResponse stencil_test_response_stencil_fail            = StencilTestResponse::Keep;
			StencilTestResponse stencil_test_response_stencil_pass_depth_fail = StencilTestResponse::Keep;
			StencilTestResponse stencil_test_response_both_pass               = StencilTestResponse::Keep;
		};

		using RenderGroupID = unsigned int;

	private:
		struct RenderGroup
		{
			using ReferenceCount = unsigned int;

			bool is_enabled = true;

			std::string name = "<unnamed>";

			RenderState render_state;

			std::vector< Drawable* > drawable_list;

			std::unordered_map< Shader*, ReferenceCount > shaders_in_flight;
			std::unordered_map< std::string, Material* > materials_in_flight; // TODO: Generate an ID for Materials (who will generate it?) and use that ID as the key here.
		};

	public:
		Renderer();
		~Renderer();

		/* 
		 * Main:
		 */

		void Update( Camera& camera );
		void Render( Camera& camera );
		void RenderImGui();
		void OnProjectionParametersChange( Camera& camera );

		/* 
		 * RenderGroup & Drawable:
		 */

		RenderState& GetRenderState( const RenderGroupID group_id_to_fetch );
		void SetRenderGroupName( const RenderGroupID group_id_to_rename, const std::string_view new_name );
		void ToggleRenderGroup( const RenderGroupID group_id_to_toggle, const bool enable );

		void AddDrawable( Drawable* drawable_to_add, const RenderGroupID render_group_id = 0 );
		// TODO: Switch to unsigned map of "Component" UUIDs when Component class is implemented.
		void RemoveDrawable( Drawable* drawable_to_remove );
		void RemoveAllDrawables();

		/* 
		 * Lights:
		 */

		void AddDirectionalLight( DirectionalLight* light_to_add );
		void RemoveDirectionalLight();
		/* Only 1 Directional Light can be active at a time! */

		void AddPointLight( PointLight* light_to_add );
		void RemovePointLight( PointLight* light_to_remove );
		void RemoveAllPointLights();

		void AddSpotLight( SpotLight* light_to_add );
		void RemoveSpotLight( SpotLight* light_to_remove );
		void RemoveAllSpotLights();

		/*
		 * Shaders:
		 */

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

		/*
		 * Clearing:
		 */

		void SetClearColor( const Color3& new_clear_color );
		void SetClearColor( const Color4& new_clear_color );
		void SetClearTargets( const BitFlags< ClearTarget > targets );

		/*
		 * Stencil Test:
		 */

		void EnableStencilTest();
		void DisableStencilTest();
		void SetStencilWriteMask( const unsigned int mask );
		void SetStencilTestResponses( const StencilTestResponse stencil_fail, const StencilTestResponse stencil_pass_depth_fail, const StencilTestResponse both_pass );
		void SetStencilComparisonFunction( const ComparisonFunction comparison_function, const int reference_value, const unsigned int mask );

		/*
		 * Depth Test:
		 */

		void EnableDepthTest();
		void DisableDepthTest();
		void ToggleDepthWrite( const bool enable );
		void SetDepthComparisonFunction( const ComparisonFunction comparison_function );

		/*
		 * Other:
		 */

		void SetPolygonMode( const PolygonMode mode );

	private:

		/*
		 * Main:
		 */

		void Render( const Mesh& mesh );
		void Render_Indexed( const Mesh& mesh );
		void Render_NonIndexed( const Mesh& mesh );
	
		void UploadIntrinsics();
		void UploadGlobals();

		/*
		 * RenderGroup & Drawable:
		 */

		RenderGroup* GetRenderGroup( const Drawable* drawable_of_interest );
		void SetRenderState( const RenderState& render_state_to_set );

		/*
		 * Shaders:
		 */

		void RegisterShader( const Shader& shader );
		void UnregisterShader( const Shader& shader );

		/*
		 * Clearing:
		 */

		void SetClearColor();
		void Clear() const;

	private:

		/*
		 * Clearing:
		 */

		Color4 clear_color;
		BitFlags< ClearTarget > clear_targets;

		/*
		 * Lighting:
		 */

		DirectionalLight*			light_directional;
		std::vector< PointLight* >	lights_point;
		std::vector< SpotLight*	 >	lights_spot;
		int lights_point_active_count;
		int lights_spot_active_count;

		/*
		 * Rendering:
		 */

		std::map< RenderGroupID, RenderGroup > render_group_map;

		std::unordered_set< const Shader* > shaders_registered; // TODO: Revise; This is not utilized at all. Also it may pose problems when dynamic shader re-compile is implemented.

		/*
		 * Uniform Management:
		 */

		std::unordered_set< const Shader* > shaders_using_intrinsics_lighting;
		std::unordered_set< const Shader* > shaders_using_intrinsics_other;

		UniformBufferManagement< DirtyBlob > uniform_buffer_management_global;
		UniformBufferManagement< DirtyBlob > uniform_buffer_management_intrinsic;

		bool update_uniform_buffer_lighting;
		bool update_uniform_buffer_other;
	};
}
