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

	void Renderer::Update( Camera& camera )
	{
		SetIntrinsic( "_Intrinsic_", "_INTRINSIC_TRANSFORM_VIEW", camera.GetViewMatrix() );
	}

	void Renderer::Render( Camera& camera )
	{
		Clear();

		UploadIntrinsics();

		// TODO: Upload Globals.
		
		for( const auto& [ shader_id, shader ] : shaders_in_flight )
		{
			shader->Bind();

			for( auto& [ material_name, material ] : materials_in_flight )
			{
				if( shader->Id() == material->shader->Id() )
				{
					for( auto& drawable : drawable_list )
					{
						if( drawable->material->Name() == material_name )
						{
							drawable->mesh->Bind();

							material->Set( "uniform_transform_world", drawable->transform->GetFinalMatrix() );

							material->UploadUniforms();

							// TODO: Upload Instance uniforms.

							Render( *drawable->mesh );
						}
					}
				}
			}
		}
	}

	void Renderer::OnProjectionParametersChange( Camera& camera )
	{
		SetIntrinsic( "_Intrinsic_", "_INTRINSIC_TRANSFORM_PROJECTION", camera.GetProjectionMatrix() );
	}

	void Renderer::AddDrawable( Drawable* drawable_to_add )
	{
		drawable_list.push_back( drawable_to_add );

		const auto& shader = drawable_to_add->material->shader;

		shaders_in_flight.try_emplace( shader->Id(), shader );
		materials_in_flight.try_emplace( drawable_to_add->material->Name(), drawable_to_add->material );

		if( not shaders_registered.contains( shader->Id() ) )
			RegisterShader( *shader );

		/* Renderer creates & keeps the Uniform Buffer Objects. So it can "lend" them to Material for it to cache them. */
		drawable_to_add->material->CacheUniformBufferMap( uniform_buffer_map_regular );
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

	void Renderer::RemoveAllDrawables()
	{
		drawable_list.clear();
		shaders_in_flight.clear();
		materials_in_flight.clear();
		shaders_registered.clear();
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

	const void* Renderer::GetIntrinsic( const char* uniform_buffer_name, const Uniform::BufferInformation& uniform_buffer_info ) const
	{
		return uniform_blob_map_intrinsic.at( uniform_buffer_name ).Get( uniform_buffer_info.offset );
	}

	void* Renderer::GetIntrinsic( const char* uniform_buffer_name, const Uniform::BufferInformation& uniform_buffer_info )
	{
		return uniform_blob_map_intrinsic[ uniform_buffer_name ].Get( uniform_buffer_info.offset );
	}

	void Renderer::UploadIntrinsics()
	{
		for( auto& [ uniform_buffer_name_intrinsic, uniform_blob_intrinsic ] : uniform_blob_map_intrinsic )
		{
			const auto& uniform_buffer      = uniform_buffer_map_intrinsic[ uniform_buffer_name_intrinsic ];
			const auto& uniform_buffer_info = uniform_buffer_info_map_intrinsic[ uniform_buffer_name_intrinsic ];

			if( uniform_blob_intrinsic.IsDirty() )
			{
				uniform_blob_intrinsic.MergeConsecutiveDirtySections();
				const auto& dirty_sections = uniform_blob_intrinsic.DirtySections();
				for( auto& dirty_section : dirty_sections )
					uniform_buffer.Update_Partial( uniform_blob_intrinsic.SpanFromSection( dirty_section ), dirty_section.offset );
			}
		}
	}

	void Renderer::RegisterShader( const Shader& shader )
	{
		if( shader.HasRegularUniformBlocks() )
		{
			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : shader.GetUniformBufferInfoMap() )
			{
				if( uniform_buffer_info.category == Uniform::BufferCategory::Regular )
				{
					auto& buffer = uniform_buffer_map_regular.try_emplace( uniform_buffer_name, uniform_buffer_info.size, uniform_buffer_name ).first->second;
					UniformBufferManager::ConnectBufferToBlock( buffer, uniform_buffer_name, Uniform::BufferCategory::Regular );
				}
			}
		}

		if( shader.HasInstanceUniformBlocks() )
		{
			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : shader.GetUniformBufferInfoMap() )
			{
				if( uniform_buffer_info.category == Uniform::BufferCategory::Instance )
				{
					auto& buffer = uniform_buffer_map_instance.try_emplace( uniform_buffer_name, uniform_buffer_info.size, uniform_buffer_name ).first->second;
					UniformBufferManager::ConnectBufferToBlock( buffer, uniform_buffer_name, Uniform::BufferCategory::Instance );
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

					//uniform_blob_map_global.try_emplace( uniform_buffer_name, uniform_buffer_info.size );
				}
			}

			//uniform_buffer_info_map_global.try_emplace( uniform_buffer_name, &uniform_buffer_info );
		}

		if( shader.HasIntrinsicUniformBlocks() )
		{
			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : shader.GetUniformBufferInfoMap() )
			{
				if( uniform_buffer_info.category == Uniform::BufferCategory::Intrinsic )
				{
					auto& buffer = uniform_buffer_map_intrinsic.try_emplace( uniform_buffer_name, uniform_buffer_info.size, uniform_buffer_name ).first->second;
					UniformBufferManager::ConnectBufferToBlock( buffer, uniform_buffer_name, Uniform::BufferCategory::Intrinsic );

					uniform_blob_map_intrinsic.try_emplace( uniform_buffer_name, uniform_buffer_info.size );
				}

				uniform_buffer_info_map_intrinsic.try_emplace( uniform_buffer_name, &uniform_buffer_info );
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
