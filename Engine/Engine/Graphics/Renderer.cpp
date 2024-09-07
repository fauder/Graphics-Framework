// Engine Includes.
#include "Renderer.h"
#include "UniformBufferManager.h"
#include "Core/ImGuiDrawer.hpp"

namespace Engine
{
	Renderer::Renderer()
		:
		lights_point_active_count( 0 ),
		lights_spot_active_count( 0 ),
		clear_color( Color4::Gray( 0.1f ) ),
		clear_targets( ClearTarget::ColorBuffer, ClearTarget::DepthBuffer )
	{
		SetClearColor();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Update( Camera& camera )
	{
		const auto& view_matrix     = camera.GetViewMatrix();
		const auto& view_matrix_3x3 = view_matrix.SubMatrix< 3 >();

		uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW", view_matrix );

		uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_DIRECTIONAL_LIGHT_IS_ACTIVE", light_directional && light_directional->is_enabled ? 1u : 0u );
		if( light_directional && light_directional->is_enabled )
		{
			light_directional->data.direction_view_space = light_directional->transform->Forward() * view_matrix_3x3;
			uniform_buffer_management_intrinsic.SetPartial_Struct( "_Intrinsic_Lighting", "_INTRINSIC_DIRECTIONAL_LIGHT", light_directional->data );
		}

		lights_point_active_count = 0;
		for( auto index = 0; index < lights_point.size(); index++ )
		{
			auto& point_light = lights_point[ index ];

			if( point_light->is_enabled )
			{
				/* Shaders expect the lights' position & direction in view space. */
				point_light->data.position_view_space  = Vector4( point_light->transform->GetTranslation() ).SetW( 1.0f ) * view_matrix;
				uniform_buffer_management_intrinsic.SetPartial_Array( "_Intrinsic_Lighting", "_INTRINSIC_POINT_LIGHTS", lights_point_active_count++, point_light->data );
			}
		}
		uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_POINT_LIGHT_ACTIVE_COUNT", lights_point_active_count );

		lights_spot_active_count = 0;
		for( auto index = 0; index < lights_spot.size(); index++ )
		{
			auto& spot_light = lights_spot[ index ];

			if( spot_light->is_enabled )
			{
				/* Shaders expect the lights' position & direction in view space. */

				spot_light->data.position_view_space_and_cos_cutoff_angle_inner.vector = ( Vector4( spot_light->transform->GetTranslation() ).SetW( 1.0f ) * view_matrix ).XYZ();
				spot_light->data.position_view_space_and_cos_cutoff_angle_inner.scalar = Math::Cos( Radians( spot_light->data.cutoff_angle_inner ) );

				spot_light->data.direction_view_space_and_cos_cutoff_angle_outer.vector = spot_light->transform->Forward() * view_matrix_3x3;
				spot_light->data.direction_view_space_and_cos_cutoff_angle_outer.scalar = Math::Cos( Radians( spot_light->data.cutoff_angle_outer ) );

				uniform_buffer_management_intrinsic.SetPartial_Array( "_Intrinsic_Lighting", "_INTRINSIC_SPOT_LIGHTS", lights_spot_active_count++, spot_light->data );
			}
		}
		uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_SPOT_LIGHT_ACTIVE_COUNT", lights_spot_active_count );
	}

	void Renderer::Render( Camera& camera )
	{
		Clear();

		UploadIntrinsics();
		UploadGlobals();
		
		for( const auto& [ shader_id, shader ] : shaders_in_flight )
		{
			shader->Bind();

			for( auto& [ material_name, material ] : materials_in_flight )
			{
				if( material->shader->Id() == shader->Id() )
				{
					material->UploadUniforms();

					for( auto& drawable : drawable_list )
					{
						if( drawable->material->Name() == material_name )
						{
							drawable->mesh->Bind();

							material->Set( "uniform_transform_world", drawable->transform->GetFinalMatrix() );

							Render( *drawable->mesh );
						}
					}
				}
			}
		}
	}

	void Renderer::RenderImGui()
	{
		ImGuiDrawer::Draw( uniform_buffer_management_intrinsic, "Shader Intrinsics" );
		ImGuiDrawer::Draw( uniform_buffer_management_global,	"Shader Globals" );
	}

	void Renderer::OnProjectionParametersChange( Camera& camera )
	{
		uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_PROJECTION", camera.GetProjectionMatrix() );
	}

	void Renderer::AddDrawable( Drawable* drawable_to_add )
	{
		drawable_list.push_back( drawable_to_add );

		const auto& shader = drawable_to_add->material->shader;

		shaders_in_flight.try_emplace( shader->Id(), shader );
		materials_in_flight.try_emplace( drawable_to_add->material->Name(), drawable_to_add->material );

		if( not shaders_registered.contains( shader->Id() ) )
			RegisterShader( *shader );
	}

	void Renderer::RemoveDrawable( const Drawable* drawable_to_remove )
	{
		std::erase( drawable_list, drawable_to_remove );

		const auto& shader = *drawable_to_remove->material->shader;

		shaders_in_flight.erase( shader.Id() );
		materials_in_flight.erase( drawable_to_remove->material->Name() );

		if( const auto& iterator = shaders_registered.find( shader.Id() );
			iterator != shaders_registered.cend() )
			shaders_registered.erase( iterator );
	}

	void Renderer::RemoveAllDrawables()
	{
		drawable_list.clear();
		shaders_in_flight.clear();
		materials_in_flight.clear();
	}

	void Renderer::AddDirectionalLight( DirectionalLight* light_to_add )
	{
		if( light_directional )
			throw std::runtime_error( "Only 1 Directional Light can be active at a time!" );

		light_directional = light_to_add;
	}

	void Renderer::RemoveDirectionalLight()
	{
		if( !light_directional )
			throw std::runtime_error( "No Directional Light set yet!" );

		light_directional = nullptr;
	}

	void Renderer::AddPointLight( PointLight* light_to_add )
	{
		lights_point.push_back( light_to_add );
	}

	void Renderer::RemovePointLight( PointLight* light_to_remove )
	{
		std::erase( lights_point, light_to_remove );
	}

	void Renderer::RemoveAllPointLights()
	{
		lights_point.clear();
	}

	void Renderer::AddSpotLight( SpotLight* light_to_add )
	{
		lights_spot.push_back( light_to_add );
	}

	void Renderer::RemoveSpotLight( SpotLight* light_to_remove )
	{
		std::erase( lights_spot, light_to_remove );
	}

	void Renderer::RemoveAllSpotLights()
	{
		lights_spot.clear();
	}

	const void* Renderer::GetShaderGlobal( const std::string& buffer_name ) const
	{
		return uniform_buffer_management_global.Get( buffer_name );
	}

	void* Renderer::GetShaderGlobal( const std::string& buffer_name )
	{
		return uniform_buffer_management_global.Get( buffer_name );
	}

	void Renderer::SetClearColor( const Color3& new_clear_color )
	{
		clear_color = new_clear_color;
		SetClearColor();
	}

	void Renderer::SetClearColor( const Color4& new_clear_color )
	{
		clear_color = new_clear_color;
		SetClearColor();
	}

/*
 * 
 *	PRIVATE API:
 * 
 */

	void Renderer::EnableDepthTest()
	{
		glEnable( GL_DEPTH_TEST );
	}

	void Renderer::DisableDepthTest()
	{
		glDisable( GL_DEPTH_TEST );
	}

	void Renderer::SetPolygonMode( const PolygonMode mode )
	{
		glPolygonMode( GL_FRONT_AND_BACK, ( GLenum )mode );
	}

	void Renderer::Render( const Mesh& mesh )
	{
		mesh.IsIndexed()
			? Render_Indexed( mesh )
			: Render_NonIndexed( mesh );
	}

	void Renderer::Render_Indexed( const Mesh& mesh )
	{
		glDrawElements( ( GLint )mesh.Primitive(), mesh.IndexCount(), GL_UNSIGNED_INT, mesh.Indices_Raw() );
	}

	void Renderer::Render_NonIndexed( const Mesh& mesh )
	{
		glDrawArrays( ( GLint )mesh.Primitive(), 0, mesh.VertexCount() );
	}

	void Renderer::UploadIntrinsics()
	{
		uniform_buffer_management_intrinsic.UploadAll();
	}

	void Renderer::UploadGlobals()
	{
		uniform_buffer_management_global.UploadAll();
	}

	void Renderer::RegisterShader( const Shader& shader )
	{
		if( shader.HasUniformBlocks() )
		{
			/* Regular Uniform Buffers are handled by the Material class.
			 * Globals & Intrinsics are registered here. */

			if( shader.HasGlobalUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Global();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_global.RegisterBuffer( uniform_buffer_name, &uniform_buffer_info );
			}

			if( shader.HasIntrinsicUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Intrinsic();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_intrinsic.RegisterBuffer( uniform_buffer_name, &uniform_buffer_info );
			}
		}

		shaders_registered[ shader.Id() ] = &shader;
	}

	void Renderer::UnregisterShader( const Shader& shader )
	{
	}

	void Renderer::SetClearColor()
	{
		glClearColor( clear_color.R(), clear_color.G(), clear_color.B(), clear_color.A() );
	}

	void Renderer::Clear() const
	{
		glClear( ( GLbitfield )clear_targets.ToBits() );
	}
}
