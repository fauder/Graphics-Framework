// Engine Includes.
#include "Renderer.h"
#include "DefaultFramebuffer.h"
#include "InternalShaders.h"
#include "UniformBufferManager.h"
#include "Core/ImGuiDrawer.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#ifdef _DEBUG
#define CONSOLE_ERROR( message ) logger.Error( ICON_FA_DRAW_POLYGON " " message )
#else
#define CONSOLE_ERROR( message ) do {} while( false )
#endif

#ifdef _DEBUG
#define CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( function_name, pass_id )\
if( not render_pass_map.contains( pass_id ) )\
{\
	logger.Error( ICON_FA_DRAW_POLYGON " " function_name "() called for non-existing pass." );\
	return;\
}
#else
#define CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( function_name, pass_id ) do {} while( false )
#endif

#ifdef _DEBUG
#define CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( function_name, queue_id )\
if( not render_queue_map.contains( queue_id ) )\
{\
	logger.Error( ICON_FA_DRAW_POLYGON " " function_name "() called for non-existing queue." );\
	return;\
}
#else
#define CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( function_name, queue_id ) do {} while( false )
#endif

namespace Engine
{
	Renderer::Renderer( std::array< std::optional< int >, Renderer::FRAMEBUFFER_OFFSCREEN_COUNT >&& offscreen_framebuffer_msaa_sample_count_values )
		:
		logger( ServiceLocator< GLLogger >::Get() ),
		framebuffer_current( nullptr ),
		offscreen_framebuffer_msaa_sample_count_array( std::move( offscreen_framebuffer_msaa_sample_count_values ) ),
		lights_point_active_count( 0 ),
		lights_spot_active_count( 0 ),
		update_uniform_buffer_lighting( false ),
		update_uniform_buffer_other( false ),
		sRGB_encoding_is_enabled( false )
	{
		DefaultFramebuffer::Instance(); // Initialize.
		
		InternalShaders::Initialize( *this );

		InitializeBuiltinQueues();
		InitializeBuiltinPasses();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Update()
	{
		RecompileModifiedShaders();
	}

	void Renderer::UpdatePerPass( const RenderPass::ID pass_id_to_update, Camera& camera )
	{
#ifdef _DEBUG
		if( not render_pass_map.contains( pass_id_to_update ) )
			CONSOLE_ERROR( " UpdatePerPass() called for non-existing pass." );
#endif // _DEBUG

		auto& pass = render_pass_map[ pass_id_to_update ];

		pass.view_matrix       = camera.GetViewMatrix();
		pass.projection_matrix = camera.GetProjectionMatrix();

		if( camera.UsesPerspectiveProjection() )
		{
			pass.plane_near             = camera.GetNearPlaneOffset();
			pass.plane_far              = camera.GetFarPlaneOffset();
			pass.aspect_ratio           = camera.GetAspectRatio();
			pass.vertical_field_of_view = camera.GetVerticalFieldOfView();
		}
	}

	void Renderer::Render()
	{
		for( auto& [ pass_id, pass ] : render_pass_map )
		{
			if( pass.is_enabled )
			{
				auto log_group( logger.TemporaryLogGroup( ( "[Pass]:" + pass.name ).c_str() ) );

				/* Update view/projection matrix, only if dirty: */
				{
					BitFlags< IntrinsicModifyTarget > intrinsic_modification_targets;

					if( pass.view_matrix && pass.view_matrix != current_camera_info.view_matrix )
					{
						intrinsic_modification_targets.Set( IntrinsicModifyTarget::UniformBuffer_View );
						current_camera_info.view_matrix = *pass.view_matrix;
					}
					
					if( pass.projection_matrix && pass.projection_matrix != current_camera_info.projection_matrix )
					{
						intrinsic_modification_targets.Set( IntrinsicModifyTarget::UniformBuffer_Projection );
						current_camera_info.projection_matrix      = *pass.projection_matrix;
						current_camera_info.plane_near             = pass.plane_near;
						current_camera_info.plane_far              = pass.plane_far;
						current_camera_info.aspect_ratio           = pass.aspect_ratio;
						current_camera_info.vertical_field_of_view = pass.vertical_field_of_view;
					}

					if( intrinsic_modification_targets.IsSet( IntrinsicModifyTarget::UniformBuffer_View ) || 
						intrinsic_modification_targets.IsSet( IntrinsicModifyTarget::UniformBuffer_Projection ) )
					{
						current_camera_info.view_projection_matrix = current_camera_info.view_matrix * current_camera_info.projection_matrix;
					}

					SetIntrinsics( intrinsic_modification_targets );
				}

				UploadIntrinsics();
				UploadGlobals();

				SetRenderState( pass.render_state, pass.target_framebuffer, pass.clear_framebuffer );

				for( auto& queue_id : pass.queue_id_set )
				{
					if( auto& queue = render_queue_map[ queue_id ]; 
						queue.is_enabled )
					{
						auto log_group( logger.TemporaryLogGroup( ( "[Queue]:" + queue.name ).c_str() ) );

						// TODO: Do not set render state for state that is not changing (i.e., dirty check).
						if( pass.render_state_override_is_allowed )
							SetRenderState( queue.render_state_override, pass.target_framebuffer /* No clearing for queues. */ );

						SortRenderablesInQueue( current_camera_info.view_matrix.GetRow< 3 >( 3 ), queue.renderable_list, queue.render_state_override.sorting_mode );

						for( const auto& [ shader, dont_care ] : queue.shaders_in_flight )
						{
							shader->Bind();

							for( auto& [ material_name, material ] : queue.materials_in_flight )
							{
								if( material->shader->Id() == shader->Id() )
								{
									material->UploadUniforms();

									for( auto& renderable : queue.renderable_list )
									{
										if( renderable->is_enabled && renderable->material->Name() == material_name )
										{
											renderable->mesh->Bind();

											if( renderable->transform )
												material->SetAndUploadUniform( "uniform_transform_world", renderable->transform->GetFinalMatrix() );

											Render( *renderable->mesh );
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void Renderer::RenderImGui()
	{
		if( ImGui::Begin( ICON_FA_DRAW_POLYGON " Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			for( auto& [ pass_id, pass ] : render_pass_map )
			{
				ImGui::PushID( ( int )pass_id );
				ImGui::Checkbox( "", &pass.is_enabled );
				ImGui::PopID();

				ImGui::SameLine();
				if( ImGui::TreeNodeEx( pass.name.c_str(), 0, "Pass [%d]: %s", ( int )pass_id, pass.name.c_str() ) )
				{
					// TODO: Display RenderState info as a collapseable header.
					ImGui::BeginDisabled( not pass.is_enabled );

					for( auto& queue_id : pass.queue_id_set )
					{
						auto& queue = render_queue_map[ queue_id ];

						ImGui::PushID( ( int )queue_id );
						ImGui::Checkbox( "", &queue.is_enabled );
						ImGui::PopID();

						ImGui::SameLine();

						if( ImGui::TreeNodeEx( queue.name.c_str(), 0, "Queue [%d]: %s", ( int )queue_id, queue.name.c_str() ) )
						{
							ImGui::BeginDisabled( not queue.is_enabled );

							for( const auto& [ shader, dont_care ] : queue.shaders_in_flight )
							{
								for( const auto& [ material_name, material ] : queue.materials_in_flight )
								{
									if( material->shader->Id() == shader->Id() )
									{
										for( auto& renderable : queue.renderable_list )
										{
											if( renderable->material->Name() == material_name )
											{
												ImGui::PushID( renderable );
												ImGui::Checkbox( "", &renderable->is_enabled );
												ImGui::PopID();
												ImGui::BeginDisabled( not renderable->is_enabled );
												ImGui::SameLine(); ImGui::TextUnformatted( renderable->material->name.c_str() );
												if( renderable->mesh->HasInstancing() )
												{
													int instance_Count = renderable->mesh->InstanceCount();
													ImGui::SameLine(); ImGui::TextColored( ImVec4( 0.84f, 0.59f, 0.45f, 1.0f ), "(Instance Count: %d)", instance_Count );
												}
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

	void Renderer::OnFramebufferResize( const int new_width_in_pixels, const int new_height_in_pixels )
	{
		glViewport( 0, 0, new_width_in_pixels, new_height_in_pixels );

		if( update_uniform_buffer_other )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_VIEWPORT_SIZE", Vector2( ( float )new_width_in_pixels, ( float )new_height_in_pixels ) );
		}

		/* Shadow maps: */
		light_directional_shadow_map_framebuffer = Engine::Framebuffer( "Shadow Map: Dir. Light", Framebuffer::Description
																		{
																			.width_in_pixels  = new_width_in_pixels / 2,
																			.height_in_pixels = new_height_in_pixels / 2,
																			
																			.minification_filter  = Engine::Texture::Filtering::Nearest,
																			.magnification_filter = Engine::Texture::Filtering::Nearest,
																			.wrap_u               = Engine::Texture::Wrapping::Repeat, 
																			.wrap_v               = Engine::Texture::Wrapping::Repeat,

																			.attachment_bits = Engine::Framebuffer::AttachmentType::Depth
																		} );

		/* Main: */
		editor_framebuffer = Engine::Framebuffer( "Editor FB", Framebuffer::Description
												  {
													  .width_in_pixels  = new_width_in_pixels,
													  .height_in_pixels = new_height_in_pixels,

													  .is_sRGB          = true, /* This is the final step, so sRGB encoding should be on. */
													  .attachment_bits  = Engine::Framebuffer::AttachmentType::Color_DepthStencilCombined
												  } );

		/* Offscreen: */
		for( auto index = 0; index < offscreen_framebuffer_msaa_sample_count_array.size(); index++ )
			offscreen_framebuffer_array[ index ] = Engine::Framebuffer( "Offscreen FB " + std::to_string( index ), Framebuffer::Description
																		{
																			.width_in_pixels    = new_width_in_pixels,
																			.height_in_pixels   = new_height_in_pixels,

																			.multi_sample_count = offscreen_framebuffer_msaa_sample_count_array[ index ],
																			.attachment_bits    = Engine::Framebuffer::AttachmentType::Color_DepthStencilCombined
																		} );

		// TODO: Keep descriptions for offscreen framebuffers around, so client code can request modification to them and indirectly modify offscreen framebuffers' properties.
		// This would make offscreen_framebuffer_msaa_sample_count_array etc. redundant.
	}

	void Renderer::AddRenderable( Renderable* renderable_to_add, const RenderQueue::ID queue_id )
	{
		auto& queue = render_queue_map[ queue_id ];

		queue.renderable_list.push_back( renderable_to_add );

		const auto& shader = renderable_to_add->material->shader;

		if( ++queue.shaders_in_flight[ shader ] == 1 )
		{
			if( not shaders_registered.contains( shader ) )
				RegisterShader( *shader );
		}

		queue.materials_in_flight.try_emplace( renderable_to_add->material->Name(), renderable_to_add->material );
	}

	void Renderer::RemoveRenderable( Renderable* renderable_to_remove )
	{
		auto& queues_containing_renderable = RenderQueuesContaining( renderable_to_remove );
		for( auto& queue : queues_containing_renderable )
		{
			// For now, stick to removing elements from a vector, which is sub-par performance but should be OK for the time being.
			std::erase( queue.renderable_list, renderable_to_remove );

			const auto& shader = renderable_to_remove->material->shader;

			if( --queue.shaders_in_flight[ shader ] == 0 )
			{
				queue.shaders_in_flight.erase( shader );
				if( --shaders_registered_reference_count_map[ shader ] == 0 )
					UnregisterShader( *shader );
			}

			queue.materials_in_flight.erase( renderable_to_remove->material->Name() );
		}
	}

	void Renderer::OnShaderReassign( Shader* previous_shader, const std::string& name_of_material_whose_shader_changed )
	{
		for( auto& [ queue_id, queue ] : render_queue_map )
		{
			if( auto iterator = queue.materials_in_flight.find( name_of_material_whose_shader_changed );
				iterator != queue.materials_in_flight.cend() )
			{
				Material* material = iterator->second;

				if( auto ref_count = queue.shaders_in_flight[ previous_shader ];
					ref_count == 1 )
					queue.shaders_in_flight.erase( previous_shader );

				queue.shaders_in_flight[ material->shader ]++;

				/* Log warning if the new shader is incompatible with mesh(es). */
				for( auto& renderable : queue.renderable_list )
					if( renderable->material == material &&
						not renderable->mesh->IsCompatibleWith( renderable->material->shader->GetSourceVertexLayout() ) )
						logger.Warning( "Mesh \"" + renderable->mesh->Name() +
																   "\" is not compatible with its current shader \"" + material->shader->Name() + "\"." );
			}
		}
	}

	RenderState& Renderer::GetRenderState( const RenderPass::ID pass_id_to_fetch )
	{
		return render_pass_map[ pass_id_to_fetch ].render_state;
	}

	void Renderer::AddPass( const RenderPass::ID new_pass_id, RenderPass&& new_pass )
	{
		if( not render_pass_map.try_emplace( new_pass_id, std::move( new_pass ) ).second )
			CONSOLE_ERROR( "Attempting to add a new pass with the ID of an existing queue!" );
	}

	void Renderer::RemovePass( const RenderPass::ID pass_id_to_remove )
	{
		if( pass_id_to_remove == PASS_ID_LIGHTING || pass_id_to_remove == PASS_ID_SHADOW_MAPPING ||
			pass_id_to_remove == PASS_ID_OUTLINE  || pass_id_to_remove == PASS_ID_POSTPROCESSING )
		{
			CONSOLE_ERROR( "Removing a built-in pass is not allowed! Please use TogglePass( id, false ) to disable unwanted passes instead." );
			return;
		}

#ifdef _DEBUG
		if( const auto iterator = render_pass_map.find( pass_id_to_remove );
			iterator != render_pass_map.cend() )
			render_pass_map.erase( pass_id_to_remove );
		else
			throw std::runtime_error( "Attempting to remove a non-existing pass!" );
#else
		render_pass_map.erase( pass_id_to_remove );
#endif // _DEBUG
	}

	void Renderer::TogglePass( const RenderPass::ID pass_id_to_toggle, const bool enable )
	{
#ifdef _DEBUG
		if( auto iterator = render_pass_map.find( pass_id_to_toggle );
			iterator != render_pass_map.cend() )
		{
			iterator->second.is_enabled = enable;
		}
		else
			CONSOLE_ERROR( "Attempting to toggle a non-existing pass!" );
#else
		render_pass_map[ pass_id_to_toggle ].is_enabled = enable;
#endif
	}

	void Renderer::AddQueue( const RenderQueue::ID new_queue_id, RenderQueue&& new_queue )
	{
		if( not render_queue_map.try_emplace( new_queue_id, std::move( new_queue ) ).second )
			CONSOLE_ERROR( "Attempting to add a new queue with the ID of an existing queue!" );
	}

	void Renderer::RemoveQueue( const RenderQueue::ID queue_id_to_remove )
	{
		if( queue_id_to_remove == QUEUE_ID_GEOMETRY    || queue_id_to_remove == QUEUE_ID_GEOMETRY_OUTLINED ||
			queue_id_to_remove == QUEUE_ID_TRANSPARENT || queue_id_to_remove == QUEUE_ID_SKYBOX ||
			queue_id_to_remove == QUEUE_ID_POSTPROCESSING )
		{
			CONSOLE_ERROR( "Removing a built-in queue is not allowed! Please use ToggleQueue( id, false ) to disable unwanted passes instead." );
			return;
		}

#ifdef _DEBUG
		if( const auto iterator = render_queue_map.find( queue_id_to_remove );
			iterator != render_queue_map.cend() )
			render_queue_map.erase( queue_id_to_remove );
		else
			throw std::runtime_error( "Attempting to remove a non-existing queue!" );
	#else
		render_queue_map.erase( queue_id_to_remove );
#endif // _DEBUG
	}

	void Renderer::AddQueueToPass( const RenderQueue::ID queue_id_to_add, const RenderPass::ID pass_to_add_to )
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "AddQueueToPass", pass_to_add_to );
		CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( "AddQueueToPass", queue_id_to_add );

		if( not render_pass_map[ pass_to_add_to ].queue_id_set.emplace( queue_id_to_add ).second )
			CONSOLE_ERROR( "Attempting to add an already existing queue to a pass!" );

	}
	void Renderer::RemoveQueueFromPass( const RenderQueue::ID queue_id_to_remove, const RenderPass::ID pass_to_remove_from )
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "RemoveQueueFromPass", pass_to_remove_from );
		CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( "RemoveQueueFromPass", queue_id_to_remove );

		if( not render_pass_map[ pass_to_remove_from ].queue_id_set.erase( queue_id_to_remove ) )
			CONSOLE_ERROR( "Attempting to add a non-existing queue from a pass!" );
	}

	void Renderer::ToggleQueue( const RenderQueue::ID queue_id_to_toggle, const bool enable )
	{
#ifdef _DEBUG
		if( auto iterator = render_queue_map.find( queue_id_to_toggle );
			iterator != render_queue_map.cend() )
		{
			iterator->second.is_enabled = enable;
		}
		else
			CONSOLE_ERROR( "Attempting to toggle a non-existing queue!" );
#else
		render_queue_map[ queue_id_to_toggle ].is_enabled = enable;
#endif
	}

	void Renderer::SetFinalPassToUseEditorFramebuffer()
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "SetFinalPassToUseEditorFramebuffer", PASS_ID_POSTPROCESSING );

		auto& pass = render_pass_map[ PASS_ID_POSTPROCESSING ];
		pass.target_framebuffer = &editor_framebuffer;
	}

	void Renderer::SetFinalPassToUseDefaultFramebuffer()
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "SetFinalPassToUseDefaultFramebuffer", PASS_ID_POSTPROCESSING );

		auto& pass = render_pass_map[ PASS_ID_POSTPROCESSING ];
		pass.target_framebuffer = nullptr;
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
					uniform_buffer_management_global.RegisterBuffer_ForceUpdateBufferInfoIfBufferExists( uniform_buffer_name, uniform_buffer_info );
			}

			if( shader.HasIntrinsicUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Intrinsic();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_intrinsic.RegisterBuffer_ForceUpdateBufferInfoIfBufferExists( uniform_buffer_name, uniform_buffer_info );
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

		if( ++shaders_registered_reference_count_map[ &shader ] == 1 )
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
				{
					// Check if this buffer is still used by other registered Shaders:
					if( std::find_if( shaders_registered.cbegin(), shaders_registered.cend(),
										[ &uniform_buffer_name ]( const Shader* shader ) { return shader->HasGlobalUniformBlock( uniform_buffer_name ); } ) == shaders_registered.cend() )
						uniform_buffer_management_global.UnregisterBuffer( uniform_buffer_name );
				}
			}

			if( shader.HasIntrinsicUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Intrinsic();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
				{
					// Check if this buffer is still used by other registered Shaders:
					if( std::find_if( shaders_registered.cbegin(), shaders_registered.cend(),
										[ &uniform_buffer_name ]( const Shader* shader ) { return shader->HasIntrinsicUniformBlock( uniform_buffer_name ); } ) == shaders_registered.cend() )
						uniform_buffer_management_intrinsic.UnregisterBuffer( uniform_buffer_name );
				}
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
		shaders_registered_reference_count_map.erase( &shader );
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

	void Renderer::SetCurrentFramebuffer( Framebuffer* framebuffer )
	{
		ASSERT_DEBUG_ONLY( framebuffer && ICON_FA_DRAW_POLYGON " " "SetCurrentFramebuffer() called with nullptr. ResetToDefaultFramebuffer() can be used to bind the default framebuffer." );

		const Vector2I old_framebuffer_size = framebuffer_current ? framebuffer_current->Size() : Vector2I{};
		
		framebuffer_current = framebuffer;
		framebuffer_current->Bind();

		if( framebuffer->Size() != old_framebuffer_size )
			glViewport( 0, 0, framebuffer->Width(), framebuffer->Height() );
	}

	void Renderer::ResetToDefaultFramebuffer( const Framebuffer::BindPoint bind_point )
	{
		framebuffer_current = nullptr;
		DefaultFramebuffer::Bind();
	}

	bool Renderer::DefaultFramebufferIsBound() const
	{
		return framebuffer_current == nullptr;
	}

	Framebuffer* Renderer::CurrentFramebuffer()
	{
		return framebuffer_current;
	}

	Framebuffer& Renderer::EditorFramebuffer()
	{
		return editor_framebuffer;
	}

	Framebuffer& Renderer::OffscreenFramebuffer( const unsigned int framebuffer_index )
	{
		return offscreen_framebuffer_array[ framebuffer_index ];
	}

	void Renderer::Enable_sRGBEncoding()
	{
		glEnable( GL_FRAMEBUFFER_SRGB );
		sRGB_encoding_is_enabled = true;
	}

	void Renderer::Disable_sRGBEncoding()
	{
		glDisable( GL_FRAMEBUFFER_SRGB );
		sRGB_encoding_is_enabled = false;
	}

	void Renderer::SetPolygonMode( const PolygonMode mode )
	{
		glPolygonMode( GL_FRONT_AND_BACK, ( GLenum )mode );
	}

	void Renderer::InitializeBuiltinQueues()
	{
		AddQueue( QUEUE_ID_GEOMETRY,
				  RenderQueue
				  {
					  .name                  = "Geometry",
					  .render_state_override = RenderState
					  {
						  .sorting_mode = Engine::SortingMode::FrontToBack
					  }
				  } );

		AddQueue( QUEUE_ID_GEOMETRY_OUTLINED,
				  RenderQueue
				  {
					  .name                  = "Geometry - Outlined",
					  .render_state_override = RenderState
					  {
						  /* This pass renders the mesh semi-regularly; It also marks the stencil buffer with 1s everywhere the mesh is rendered at. */

						  .stencil_test_enable = true,

						  .sorting_mode = Engine::SortingMode::FrontToBack,

						  .stencil_write_mask                            = 0xFF,
						  .stencil_comparison_function                   = Engine::ComparisonFunction::Always,
						  .stencil_ref                                   = 0x01,
						  .stencil_test_response_stencil_fail            = Engine::StencilTestResponse::Keep,
						  .stencil_test_response_stencil_pass_depth_fail = Engine::StencilTestResponse::Keep,
						  .stencil_test_response_both_pass               = Engine::StencilTestResponse::Replace
					  }
				  } );

		AddQueue( QUEUE_ID_TRANSPARENT,
				  RenderQueue
				  {
					  .name                  = "Transparent",
					  .render_state_override = RenderState
					  {
						  .blending_enable = true,

						  .sorting_mode = Engine::SortingMode::BackToFront,

						  .blending_source_color_factor      = Engine::BlendingFactor::SourceAlpha,
						  .blending_destination_color_factor = Engine::BlendingFactor::OneMinusSourceAlpha,
						  .blending_source_alpha_factor      = Engine::BlendingFactor::SourceAlpha,
						  .blending_destination_alpha_factor = Engine::BlendingFactor::OneMinusSourceAlpha
					  }
				  } );


		AddQueue( QUEUE_ID_SKYBOX,
				  RenderQueue
				  {
					  .name                  = "Skybox",
					  .render_state_override = RenderState
					  {
						 .face_culling_enable = false,

						 .depth_comparison_function = ComparisonFunction::LessOrEqual
					  }
				  } );
		AddQueue( QUEUE_ID_POSTPROCESSING,
				  RenderQueue
				  {
					  .name = "Post-processing",
				  } );
	}

	void Renderer::InitializeBuiltinPasses()
	{
		AddPass( PASS_ID_SHADOW_MAPPING,
				 RenderPass
				 {
					 .name               = "Shadow Mapping",
					 .target_framebuffer = &light_directional_shadow_map_framebuffer,
					 .queue_id_set       = { QUEUE_ID_GEOMETRY, QUEUE_ID_GEOMETRY_OUTLINED },
					 .render_state       = RenderState
					 {
						 .sorting_mode = Engine::SortingMode::FrontToBack,
					 },
					 .render_state_override_is_allowed = false,
					 .clear_framebuffer                = true
				 } );


		AddPass( PASS_ID_LIGHTING,
				 RenderPass
				 {
					 .name               = "Lighting",
					 .target_framebuffer = &offscreen_framebuffer_array[ 0 ],
					 .queue_id_set       = { QUEUE_ID_GEOMETRY, QUEUE_ID_GEOMETRY_OUTLINED, QUEUE_ID_TRANSPARENT, QUEUE_ID_SKYBOX },
					 .clear_framebuffer  = true,
				 } );

		AddPass( PASS_ID_OUTLINE,
				 RenderPass
				 {
					 .name               = "Outline",
					 .target_framebuffer = &offscreen_framebuffer_array[ 0 ],
					 .queue_id_set       = { QUEUE_ID_GEOMETRY },
					 .render_state       = RenderState
					 {
					     .depth_test_enable   = false,
						 .stencil_test_enable = true,


						 .stencil_write_mask          = 0x00, // Disable writes; This pass only needs to READ the stencil buffer, to figure out where NOT to render.
						 .stencil_comparison_function = Engine::ComparisonFunction::NotEqual, // Render everywhere that's not the actual mesh, i.e., the border.
						 .stencil_ref                 = 0x01
					 },
					 .render_state_override_is_allowed = false,
					 .clear_framebuffer                = false, // => because rendering into the same framebuffer as the Lighting pass.
				 } );

		AddPass( PASS_ID_POSTPROCESSING,
				 RenderPass
				 {
					 .name               = "Post-processing",
					 .target_framebuffer = &editor_framebuffer,
					 .queue_id_set       = { QUEUE_ID_POSTPROCESSING }
				 } );
	}

	void Renderer::Render( const Mesh& mesh )
	{
		mesh.HasInstancing()
			? mesh.HasIndices()
				? RenderInstanced_Indexed( mesh )
				: RenderInstanced_NonIndexed( mesh )
			: mesh.HasIndices()
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

	void Renderer::RenderInstanced( const Mesh& mesh )
	{
		mesh.HasIndices()
			? RenderInstanced_Indexed( mesh )
			: RenderInstanced_NonIndexed( mesh );
	}

	void Renderer::RenderInstanced_Indexed( const Mesh& mesh )
	{
		glDrawElementsInstanced( ( GLint )mesh.Primitive(), mesh.IndexCount(), mesh.IndexType(), 0, mesh.InstanceCount() );
	}

	void Renderer::RenderInstanced_NonIndexed( const Mesh& mesh )
	{
		glDrawArraysInstanced( ( GLint )mesh.Primitive(), 0, mesh.VertexCount(), mesh.InstanceCount() );
	}

	void Renderer::SetIntrinsics( const BitFlags< IntrinsicModifyTarget > targets )
	{
		if( targets == IntrinsicModifyTarget::None )
			return;

		if( targets.IsSet( IntrinsicModifyTarget::UniformBuffer_Projection ) && update_uniform_buffer_other )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_PROJECTION", current_camera_info.projection_matrix );
			if( not targets.IsSet( IntrinsicModifyTarget::UniformBuffer_View ) ) // No need to upload twice.
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_PROJECTION", current_camera_info.view_projection_matrix );

			if( Matrix::IsPerspectiveProjection( current_camera_info.projection_matrix ) ) // No need to upload these if they will not mean anything anyway.
			{
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_NEAR",					current_camera_info.plane_near );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_FAR",					current_camera_info.plane_far );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_ASPECT_RATIO",			current_camera_info.aspect_ratio );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_VERTICAL_FIELD_OF_VIEW", current_camera_info.vertical_field_of_view );
			}
		}

		if( targets.IsSet( IntrinsicModifyTarget::UniformBuffer_View ) )
		{
			const auto& view_matrix               = current_camera_info.view_matrix;
			const auto& view_matrix_3x3           = view_matrix.SubMatrix< 3 >();
			const auto& view_matrix_rotation_only = Matrix4x4( view_matrix_3x3 );

			if( update_uniform_buffer_other )
			{
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW",				view_matrix );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_ROTATION_ONLY",	view_matrix_rotation_only );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_PROJECTION",		current_camera_info.view_projection_matrix );
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
	}

	void Renderer::UploadIntrinsics()
	{
		uniform_buffer_management_intrinsic.UploadAll();
	}

	void Renderer::UploadGlobals()
	{
		uniform_buffer_management_global.UploadAll();
	}

	std::vector< RenderQueue >& Renderer::RenderQueuesContaining( const Renderable* renderable_of_interest )
	{
		static std::vector< RenderQueue > queues;
		queues.clear();

		for( const auto& [ queue_id, queue ] : render_queue_map )
			if( std::find( queue.renderable_list.cbegin(), queue.renderable_list.cend(), renderable_of_interest ) != queue.renderable_list.cend() )
				queues.push_back( queue );

		return queues;
	}

	void Renderer::SetRenderState( const RenderState& render_state_to_set, Framebuffer* target_framebuffer, const bool clear_framebuffer )
	{
		if( framebuffer_current != target_framebuffer )
		{
			if( target_framebuffer )
				SetCurrentFramebuffer( target_framebuffer );
			else
				ResetToDefaultFramebuffer();
		}

		if( clear_framebuffer )
			framebuffer_current ? framebuffer_current->Clear() : DefaultFramebuffer::Clear();

		if( auto framebuffer_uses_srgb_encoding = framebuffer_current ? framebuffer_current->Is_sRGB() : true /* Default framebuffer always uses sRGB Encoding. */;
			framebuffer_uses_srgb_encoding != sRGB_encoding_is_enabled )
		{
			if( framebuffer_uses_srgb_encoding )
				Enable_sRGBEncoding();
			else
				Disable_sRGBEncoding();
		}

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

	void Renderer::SortRenderablesInQueue( const Vector3& camera_position, std::vector< Renderable* >& renderable_array_to_sort, const SortingMode sorting_mode )
	{
		switch( sorting_mode )
		{
			case SortingMode::FrontToBack:
				std::sort( renderable_array_to_sort.begin(), renderable_array_to_sort.end(),
						   [ & ]( Renderable* renderable_1, Renderable* renderable_2 )
							{
								if( renderable_1->GetTransform() && renderable_2->GetTransform() )
									return Math::Distance( camera_position, renderable_1->GetTransform()->GetTranslation() ) <
										   Math::Distance( camera_position, renderable_2->GetTransform()->GetTranslation() );

								return renderable_1 < renderable_2; // Does not matter;
							} );
				break;

			case SortingMode::BackToFront:
				std::sort( renderable_array_to_sort.begin(), renderable_array_to_sort.end(),
						   [ & ]( Renderable* renderable_1, Renderable* renderable_2 )
							{
								if( renderable_1->GetTransform() && renderable_2->GetTransform() )
									return Math::Distance( camera_position, renderable_1->GetTransform()->GetTranslation() ) >
										   Math::Distance( camera_position, renderable_2->GetTransform()->GetTranslation() );
							
								return renderable_1 < renderable_2; // Does not matter;
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

				*shader = std::move( new_shader );

				RegisterShader( *shader );

				logger.Info( "\"" + shader->name + "\" shader's source files are modified. It is recompiled." );
			}
			else
				logger.Error( "\"" + shader->name + "\" shader's source files are modified but it could not be recompiled successfully." );
		}
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
