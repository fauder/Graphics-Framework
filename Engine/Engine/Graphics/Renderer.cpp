// Engine Includes.
#include "Renderer.h"
#include "UniformBufferManager.h"
#include "Core/ImGuiDrawer.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Engine
{
	Renderer::Renderer()
		:
		framebuffer_current( nullptr ),
		lights_point_active_count( 0 ),
		lights_spot_active_count( 0 ),
		clear_color( Color4::Gray( 0.1f ) ),
		clear_targets( ClearTarget::ColorBuffer, ClearTarget::DepthBuffer ),
		update_uniform_buffer_lighting( false ),
		update_uniform_buffer_other( false )
	{
		SetClearColor();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Update( Camera& camera )
	{
		RecompileModifiedShaders();

		const auto& view_matrix               = camera.GetViewMatrix();
		const auto& view_matrix_3x3           = view_matrix.SubMatrix< 3 >();
		const auto& view_matrix_rotation_only = Matrix4x4( view_matrix_3x3 );

		if( update_uniform_buffer_other )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW",				view_matrix );
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_ROTATION_ONLY",	view_matrix_rotation_only );
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_PROJECTION",		view_matrix * camera.GetProjectionMatrix() );
		}

		if( update_uniform_buffer_lighting )
		{
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
					point_light->data.position_view_space = Vector4( point_light->transform->GetTranslation() ).SetW( 1.0f ) * view_matrix;
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
	}

	void Renderer::Render( Camera& camera, std::initializer_list< RenderGroupID > groups_to_render )
	{
		ASSERT_DEBUG_ONLY( groups_to_render.end() == std::find_if( groups_to_render.begin(), groups_to_render.end(), 
																   [ & ]( const RenderGroupID& id ) { return not render_group_map.contains( id ); } ) );

		Clear();

		UploadIntrinsics();
		UploadGlobals();

		for( auto& [ render_group_id, render_group ] : render_group_map )
		{
			if( std::find( groups_to_render.begin(), groups_to_render.end(), render_group_id ) != groups_to_render.end() &&
				render_group.is_enabled )
			{
				SetRenderState( render_group.render_state );

				SortDrawablesInGroup( camera, render_group.drawable_list, render_group.render_state.sorting_mode );

				for( const auto& [ shader, dont_care ] : render_group.shaders_in_flight )
				{
					shader->Bind();

					for( auto& [ material_name, material ] : render_group.materials_in_flight )
					{
						if( material->shader->Id() == shader->Id() )
						{
							material->UploadUniforms();

							for( auto& drawable : render_group.drawable_list )
							{
								if( drawable->is_enabled && drawable->material->Name() == material_name )
								{
									drawable->mesh->Bind();

									if( drawable->transform )
										material->SetAndUploadUniform( "uniform_transform_world", drawable->transform->GetFinalMatrix() );

									Render( *drawable->mesh );
								}
							}
						}
					}
				}
			}
		}

		// TODO: Figure out how to correctly model this behaviour: What if the user wants to explicitly NOT clear a specific buffer?
		/* Reset write masks or clearing will not modify the buffers: */
		ToggleDepthWrite( true );
		SetStencilWriteMask( 0xFF );

	}

	void Renderer::RenderImGui()
	{
		// TODO: Implement drag & drop reordering of RenderGroups.

		if( ImGui::Begin( ICON_FA_DRAW_POLYGON " Drawables", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			for( auto& [ render_group_id, render_group ] : render_group_map )
			{
				ImGui::PushID( render_group_id );
				ImGui::Checkbox( "", &render_group.is_enabled );
				ImGui::PopID();

				ImGui::SameLine();
				if( ImGui::TreeNodeEx( render_group.name.c_str(), 0, "Render Group [%d]: %s", render_group_id, render_group.name.c_str() ) )
				{
					// TODO: Display RenderState info as a collapseable header.
					ImGui::BeginDisabled( not render_group.is_enabled );

					for( const auto& [ shader, dont_care ] : render_group.shaders_in_flight )
					{
						for( const auto& [ material_name, material ] : render_group.materials_in_flight )
						{
							if( material->shader->Id() == shader->Id() )
							{
								for( auto& drawable : render_group.drawable_list )
								{
									if( drawable->material->Name() == material_name )
									{
										ImGui::PushID( drawable );
										ImGui::Checkbox( "", &drawable->is_enabled );
										ImGui::PopID();
										ImGui::BeginDisabled( not drawable->is_enabled );
										ImGui::SameLine(); ImGui::TextUnformatted( drawable->material->name.c_str() );
										ImGui::EndDisabled();
									}
								}
							}
						}
					}

					ImGui::EndDisabled();

					ImGui::TreePop();
				}
			}
		}

		ImGui::End();

		/* Shaders: */
		for( auto& shader : shaders_registered )
			Engine::ImGuiDrawer::Draw( *shader );

		/* Uniforms (Renderer-scope): */
		ImGuiDrawer::Draw( uniform_buffer_management_intrinsic, "Shader Intrinsics" );
		ImGuiDrawer::Draw( uniform_buffer_management_global,	"Shader Globals" );
	}

	void Renderer::OnProjectionParametersChange( Camera& camera )
	{
		if( update_uniform_buffer_other )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_PROJECTION",				camera.GetProjectionMatrix()	);
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_NEAR",					camera.GetNearPlaneOffset()		);
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_FAR",					camera.GetFarPlaneOffset()		);
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_ASPECT_RATIO",			camera.GetAspectRatio()			);
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_VERTICAL_FIELD_OF_VIEW", camera.GetVerticalFieldOfView() );
		}
	}

	void Renderer::OnFramebufferResize( const int new_width_in_pixels, const int new_height_in_pixels )
	{
		glViewport( 0, 0, new_width_in_pixels, new_height_in_pixels );

		if( update_uniform_buffer_other )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_VIEWPORT_SIZE", Vector2( ( float )new_width_in_pixels, ( float )new_height_in_pixels ) );
		}
	}

	void Renderer::AddDrawable( Drawable* drawable_to_add, const RenderGroupID render_group_id )
	{
		auto& render_group = render_group_map[ render_group_id ];

		render_group.drawable_list.push_back( drawable_to_add );

		const auto& shader = drawable_to_add->material->shader;

		if( ++render_group.shaders_in_flight[ shader ] == 1 )
		{
			if( not shaders_registered.contains( shader ) )
				RegisterShader( *shader );
		}

		render_group.materials_in_flight.try_emplace( drawable_to_add->material->Name(), drawable_to_add->material );
	}

	void Renderer::RemoveDrawable( Drawable* drawable_to_remove )
	{
		if( auto render_group_found = GetRenderGroup( drawable_to_remove ) )
		{
			auto& render_group_to_remove_from = *render_group_found;

			// For now, stick to removing elements from a vector, which is sub-par performance but should be OK for the time being.
			std::erase( render_group_to_remove_from.drawable_list, drawable_to_remove );

			const auto& shader = drawable_to_remove->material->shader;

			if( --render_group_to_remove_from.shaders_in_flight[ shader ] == 0 )
			{
				render_group_to_remove_from.shaders_in_flight.erase( shader );
				if( shaders_registered.contains( shader ) )
					UnregisterShader( *shader );
			}

			render_group_to_remove_from.materials_in_flight.erase( drawable_to_remove->material->Name() );
		}
	}

	void Renderer::RemoveAllDrawables()
	{
		for( auto& [ render_group_id, render_group ] : render_group_map )
		{
			render_group.drawable_list.clear();
			render_group.shaders_in_flight.clear();
			render_group.materials_in_flight.clear();
		}
	}

	void Renderer::OnShaderReassign( Shader* previous_shader, const std::string& name_of_material_whose_shader_changed )
	{
		for( auto& [ render_group_id, render_group ] : render_group_map )
		{
			if( auto iterator = render_group.materials_in_flight.find( name_of_material_whose_shader_changed );
				iterator != render_group.materials_in_flight.cend() )
			{
				Material* material = iterator->second;

				if( auto ref_count = render_group.shaders_in_flight[ previous_shader ];
					ref_count == 1 )
					render_group.shaders_in_flight.erase( previous_shader );

				render_group.shaders_in_flight[ material->shader ]++;

				/* Log warning if the new shader is incompatible with mesh(es). */
				for( auto& drawable : render_group.drawable_list )
					if( drawable->material == material &&
						not drawable->mesh->IsCompatibleWith( drawable->material->shader->GetSourceVertexLayout() ) )
						ServiceLocator< GLLogger >::Get().Warning( "Mesh \"" + drawable->mesh->Name() + "\" is not compatible with its current shader \"" + material->shader->Name() + "\"." );
			}
		}
	}

	Renderer::RenderState& Renderer::GetRenderState( const RenderGroupID group_id_to_fetch )
	{
		return render_group_map[ group_id_to_fetch ].render_state;
	}

	void Renderer::SetRenderGroupName( const RenderGroupID group_id_to_rename, const std::string_view new_name )
	{
		render_group_map[ group_id_to_rename ].name = new_name;
	}

	void Renderer::ToggleRenderGroup( const RenderGroupID group_id_to_toggle, const bool enable )
	{
		render_group_map[ group_id_to_toggle ].is_enabled = enable;
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

	void Renderer::RegisterShader( Shader& shader )
	{
		if( shader.HasUniformBlocks() )
		{
			/* Regular Uniform Buffers are handled by the Material class.
			 * Globals & Intrinsics are registered here. */

			if( shader.HasGlobalUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Global();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_global.RegisterBuffer( uniform_buffer_name, uniform_buffer_info );
			}

			if( shader.HasIntrinsicUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Intrinsic();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_intrinsic.RegisterBuffer( uniform_buffer_name, uniform_buffer_info );
			}
		}

		if( shader.GetUniformBufferInfoMap_Intrinsic().contains( "_Intrinsic_Lighting" ) )
		{
			shaders_using_intrinsics_lighting.insert( &shader );
			update_uniform_buffer_lighting = true;
		}

		if( shader.GetUniformBufferInfoMap_Intrinsic().contains( "_Intrinsic_Other" ) )
		{
			shaders_using_intrinsics_other.insert( &shader );
			update_uniform_buffer_other = true;
		}

		shaders_registered.insert( &shader );
	}

	void Renderer::UnregisterShader( Shader& shader )
	{
		if( shader.HasUniformBlocks() )
		{
			/* Regular Uniform Buffers are handled by the Material class.
			 * Globals & Intrinsics are registered here. */

			if( shader.HasGlobalUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Global();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_global.UnregisterBuffer( uniform_buffer_name );
			}

			if( shader.HasIntrinsicUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Intrinsic();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_intrinsic.UnregisterBuffer( uniform_buffer_name );
			}
		}

		if( auto iterator = shaders_using_intrinsics_lighting.find( &shader );
			iterator != shaders_using_intrinsics_lighting.cend() )
		{
			shaders_using_intrinsics_lighting.erase( iterator );
			update_uniform_buffer_lighting = not shaders_using_intrinsics_lighting.empty();
		}

		if( auto iterator = shaders_using_intrinsics_other.find( &shader );
			iterator != shaders_using_intrinsics_other.cend() )
		{
			shaders_using_intrinsics_other.erase( iterator );
			update_uniform_buffer_other = not shaders_using_intrinsics_other.empty();
		}

		shaders_registered.erase( &shader );
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

	void Renderer::SetClearTargets( const BitFlags< ClearTarget > targets )
	{
		clear_targets = targets;
	}

/*
 * 
 *	PRIVATE API:
 * 
 */

	void Renderer::EnableStencilTest()
	{
		glEnable( GL_STENCIL_TEST );
	}

	void Renderer::DisableStencilTest()
	{
		glDisable( GL_STENCIL_TEST );
	}

	void Renderer::SetStencilWriteMask( const unsigned int mask )
	{
		glStencilMask( mask );
	}

	void Renderer::SetStencilTestResponses( const StencilTestResponse stencil_fail, const StencilTestResponse stencil_pass_depth_fail, const StencilTestResponse both_pass )
	{
		glStencilOp( ( GLenum )stencil_fail, ( GLenum )stencil_pass_depth_fail, ( GLenum )both_pass );
	}

	void Renderer::SetStencilComparisonFunction( const ComparisonFunction comparison_function, const int reference_value, const unsigned int mask )
	{
		glStencilFunc( ( GLenum )comparison_function, reference_value, mask );
	}

	void Renderer::EnableDepthTest()
	{
		glEnable( GL_DEPTH_TEST );
	}

	void Renderer::DisableDepthTest()
	{
		glDisable( GL_DEPTH_TEST );
	}

	void Renderer::ToggleDepthWrite( const bool enable )
	{
		glDepthMask( ( GLint )enable );
	}

	void Renderer::SetDepthComparisonFunction( const ComparisonFunction comparison_function )
	{
		glDepthFunc( ( GLenum )comparison_function );
	}

	void Renderer::EnableBlending()
	{
		glEnable( GL_BLEND );
	}

	void Renderer::DisableBlending()
	{
		glDisable( GL_BLEND );
	}

	void Renderer::SetBlendingFactors( const BlendingFactor source_color_factor, const BlendingFactor destination_color_factor,
									   const BlendingFactor source_alpha_factor, const BlendingFactor destination_alpha_factor )
	{
		glBlendFuncSeparate( ( GLenum )source_color_factor, ( GLenum )destination_color_factor, ( GLenum )source_alpha_factor, ( GLenum )destination_alpha_factor );
	}

	void Renderer::SetBlendingFunction( const BlendingFunction function )
	{
		glBlendEquation( ( GLenum )function );
	}

	void Renderer::SetCurrentFramebuffer( const Framebuffer* framebuffer )
	{
		framebuffer_current = framebuffer;
		framebuffer_current->Bind();
	}

	void Renderer::ResetToDefaultFramebuffer( const Framebuffer::Usage usage )
	{
		framebuffer_current = nullptr;
		glBindFramebuffer( ( GLenum )usage, 0 );
	}

	bool Renderer::DefaultFramebufferIsBound() const
	{
		return framebuffer_current == nullptr;
	}

	const Framebuffer* Renderer::CurrentFramebuffer() const
	{
		return framebuffer_current;
	}

	void Renderer::SetPolygonMode( const PolygonMode mode )
	{
		glPolygonMode( GL_FRONT_AND_BACK, ( GLenum )mode );
	}

	void Renderer::Render( const Mesh& mesh )
	{
		mesh.HasIndices()
			? Render_Indexed( mesh )
			: Render_NonIndexed( mesh );
	}

	void Renderer::Render_Indexed( const Mesh& mesh )
	{
		glDrawElements( ( GLint )mesh.Primitive(), mesh.IndexCount(), mesh.IndexType(), 0 );
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

	Renderer::RenderGroup* Renderer::GetRenderGroup( const Drawable* drawable_of_interest )
	{
		for( auto& [ render_group_id, render_group ] : render_group_map )
			if( std::find( render_group.drawable_list.cbegin(), render_group.drawable_list.cend(), drawable_of_interest ) != render_group.drawable_list.cend() )
				return &render_group;

		return nullptr;
	}

	void Renderer::SetRenderState( const RenderState& render_state_to_set )
	{
		if( render_state_to_set.face_culling_enable )
			EnableFaceCulling();
		else
			DisableFaceCulling();

		SetCullFace( render_state_to_set.face_culling_face_to_cull );
		SetFrontFaceConvention( render_state_to_set.face_culling_winding_order );

		if( render_state_to_set.depth_test_enable )
			EnableDepthTest();
		else
			DisableDepthTest();

		ToggleDepthWrite( render_state_to_set.depth_write_enable );
		SetDepthComparisonFunction( render_state_to_set.depth_comparison_function );

		if( render_state_to_set.stencil_test_enable )
			EnableStencilTest();
		else
			DisableStencilTest();

		SetStencilWriteMask( render_state_to_set.stencil_write_mask );
		SetStencilComparisonFunction( render_state_to_set.stencil_comparison_function, render_state_to_set.stencil_ref, render_state_to_set.stencil_mask );
		SetStencilTestResponses( render_state_to_set.stencil_test_response_stencil_fail, 
								 render_state_to_set.stencil_test_response_stencil_pass_depth_fail, 
								 render_state_to_set.stencil_test_response_both_pass );

		if( render_state_to_set.blending_enable )
			EnableBlending();
		else
			DisableBlending();

		SetBlendingFactors( render_state_to_set.blending_source_color_factor, render_state_to_set.blending_destination_color_factor,
							render_state_to_set.blending_source_alpha_factor, render_state_to_set.blending_destination_alpha_factor );
		SetBlendingFunction( render_state_to_set.blending_function );
	}

	void Renderer::SortDrawablesInGroup( Camera& camera, std::vector< Drawable* >& drawable_array_to_sort, const SortingMode sorting_mode )
	{
		switch( sorting_mode )
		{
			case SortingMode::DepthNearestToFarthest:
				std::sort( drawable_array_to_sort.begin(), drawable_array_to_sort.end(),
						   [ & ]( Drawable* drawable_1, Drawable* drawable_2 )
							{
								return Math::Distance( camera.Position(), drawable_1->GetTransform()->GetTranslation() ) <
										Math::Distance( camera.Position(), drawable_2->GetTransform()->GetTranslation() );
							} );
				break;

			case SortingMode::DepthFarthestToNearest:
				std::sort( drawable_array_to_sort.begin(), drawable_array_to_sort.end(),
						   [ & ]( Drawable* drawable_1, Drawable* drawable_2 )
							{
								return Math::Distance( camera.Position(), drawable_1->GetTransform()->GetTranslation() ) >
										Math::Distance( camera.Position(), drawable_2->GetTransform()->GetTranslation() );
							} );
				break;

			default:
			case SortingMode::None:
				break;
		}
	}

	void Renderer::RecompileModifiedShaders()
	{
		/* Shader Recompilation: */
		static std::vector< Shader* > shaders_to_recompile;

		/* Have to do two passes as shaders to be recompiled need to be removed from shaders_registered, which we can not do while traversing the container. */

		shaders_to_recompile.clear();

		for( const auto& shader : shaders_registered )
			if( shader->SourceFilesAreModified() )
				shaders_to_recompile.push_back( shader );

		for( auto& shader : shaders_to_recompile )
		{
			Shader new_shader( shader->name.c_str() );
			if( shader->RecompileFromThis( new_shader ) )
			{
				UnregisterShader( *shader );

				/* Swap: */
				{
					Shader temp( std::move( *shader ) );
					*shader = std::move( new_shader );
					new_shader = std::move( temp );

					new_shader.program_id = 0; // To prevent double-deletion.
				}

				RegisterShader( *shader );

				ServiceLocator< GLLogger >::Get().Info( "\"" + shader->name + "\" shader's source files are modified. It is recompiled." );
			}
			else
				ServiceLocator< GLLogger >::Get().Error( "\"" + shader->name + "\" shader's source files are modified but it could not be recompiled successfully." );
		}
	}

	void Renderer::SetClearColor()
	{
		glClearColor( clear_color.R(), clear_color.G(), clear_color.B(), clear_color.A() );
	}

	void Renderer::Clear() const
	{
		glClear( ( GLbitfield )clear_targets.ToBits() );
	}

	void Renderer::EnableFaceCulling()
	{
		glEnable( GL_CULL_FACE );
	}

	void Renderer::DisableFaceCulling()
	{
		glDisable( GL_CULL_FACE );
	}

	void Renderer::SetCullFace( const Face face )
	{
		glCullFace( ( GLenum )face );
	}

	void Renderer::SetFrontFaceConvention( const WindingOrder winding_order_of_front_faces )
	{
		glFrontFace( ( GLenum )winding_order_of_front_faces );
	}
}
