// Engine Includes.
#include "Renderer.h"

#include "UniformBufferManager.h"

namespace Engine
{
	Renderer::Renderer()
		:
		clear_color( Color4::Gray( 0.1f ) ),
		clear_target( ClearTarget( ( int )ClearTarget::ColorBuffer | ( int )ClearTarget::DepthBuffer ) )
	{
		SetClearColor();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Render( Camera& camera )
	{
		Clear();

		auto& intrinsic_camera_matrices_uniform_buffer = uniform_buffer_map_intrinsic[ "_Intrinsic_" ];

		const auto& view_matrix       = camera.GetViewMatrix();
		const auto& projection_matrix = camera.GetProjectionMatrix();
		// TODO: After blob & intrinsic getter/setters are implemented, replace the hard-coded calls below with the proper ones.
		intrinsic_camera_matrices_uniform_buffer.Update_Partial( std::span( ( std::byte* )view_matrix.Data(),		sizeof( view_matrix ) ),		0 );
		intrinsic_camera_matrices_uniform_buffer.Update_Partial( std::span( ( std::byte* )projection_matrix.Data(), sizeof( projection_matrix ) ), 64 );
		
		for( const auto& [ shader_id, shader ] : shaders_in_flight )
		{
			shader->Bind();

			// TODO: Upload Globals.

			for( auto& [ material_name, material ] : materials_in_flight )
			{
				if( shader->Id() == material->shader->Id() )
				{
					//material->Bind(); // Shader is already bound.

					for( auto& drawable : drawable_list )
					{
						if( drawable->material->Name() == material_name )
						{
							drawable->mesh->Bind();

							material->Set( "uniform_transform_world", drawable->transform->GetFinalMatrix() );

							drawable->material->UploadUniforms();

							// TODO: Implement Instance uniforms.
							// TODO: Upload Instance uniforms.

							Render( *drawable->mesh );
						}
					}
				}
			}
		}
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
		drawable_list.erase( std::remove( drawable_list.begin(), drawable_list.end(), drawable_to_remove ), drawable_list.cend() );

		const auto& shader = *drawable_to_remove->material->shader;

		shaders_in_flight.erase( shader.Id() );
		materials_in_flight.erase( drawable_to_remove->material->Name() );

		if( const auto& iterator = shaders_registered.find( shader.Id() );
			iterator != shaders_registered.cend() )
			shaders_registered.erase( iterator );
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

	void Renderer::SetClearTarget( const ClearTarget& target )
	{
		clear_target = target;
	}

/*
 * 
 *	PRIVATE API:
 * 
 */

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

	void Renderer::RegisterShader( const Shader& shader )
	{
		// TODO: Create a dummy "intrinsics" shader with all the intrinsics the Engine will support defined and use that shader (direct member of Renderer) to gather Intrinsics info.?
		// TODO: If the above plan is implemented, move the first if block below to somewhere else and only register global uniform info from new shaders to be registered.

		if( shader.HasIntrinsicUniformBlocks() )
		{
			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : shader.GetUniformBufferInfoMap() )
			{
				if( uniform_buffer_info.category == Uniform::BufferCategory::Intrinsic )
				{
					auto& buffer = uniform_buffer_map_intrinsic.try_emplace( uniform_buffer_name, uniform_buffer_info.size, uniform_buffer_name ).first->second;
					UniformBufferManager::ConnectBufferToBlock( buffer, uniform_buffer_name, Uniform::BufferCategory::Intrinsic );
				}
			}
		}

		if( shader.HasGlobalUniformBlocks() )
		{
			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : shader.GetUniformBufferInfoMap() )
			{
				if( uniform_buffer_info.category == Uniform::BufferCategory::Global )
				{
					auto& buffer = uniform_buffer_map_global.try_emplace( uniform_buffer_name, uniform_buffer_info.size, uniform_buffer_name ).first->second;
					UniformBufferManager::ConnectBufferToBlock( buffer, uniform_buffer_name, Uniform::BufferCategory::Global );
				}
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
		glClear( ( GLbitfield )clear_target );
	}
}