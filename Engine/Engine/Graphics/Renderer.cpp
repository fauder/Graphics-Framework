// Engine Includes.
#include "Renderer.h"

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

		// TODO: 1) Divide uniforms into categories:
		// TODO:	1.a) Lighting related uniforms should be set PER shader, not material. Likewise, there may be more per shader uniforms (camera/projection parameters for example).
		// TODO:	1.b) There may be per instance vs per material uniforms; 
		// TODO:		 1.b cont'd) For example, all instances of the material "Ball" may share the same color while the size needs to be a per instance parameter.

		for( auto& drawable : drawable_list )
		{
			drawable->mesh->Bind();

			/*currently_bound_shader = */drawable->material->Bind();

			drawable->material->Set( "uniform_transform_view",		 camera.GetViewMatrix() );
			drawable->material->Set( "uniform_transform_projection", camera.GetProjectionMatrix() );

			drawable->material->UploadAllUniforms(); 

			Render( *drawable->mesh );
		}
	}

	void Renderer::AddDrawable( Drawable* drawable_to_add )
	{
		drawable_list.push_back( drawable_to_add );
	}

	void Renderer::RemoveDrawable( const Drawable* drawable_to_remove )
	{
		std::ignore = std::remove( drawable_list.begin(), drawable_list.end(), drawable_to_remove );
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

	void Renderer::SetClearColor()
	{
		GLCALL( glClearColor( clear_color.R(), clear_color.G(), clear_color.B(), clear_color.A() ) );
	}

	void Renderer::Clear() const
	{
		GLCALL( glClear( ( GLbitfield )clear_target ) );
	}

	void Renderer::Render( const Mesh& mesh )
	{
		mesh.IsIndexed()
			? Render_Indexed( mesh )
			: Render_NonIndexed( mesh );
	}

	void Renderer::Render_Indexed( const Mesh& mesh )
	{
		GLCALL( glDrawElements( ( GLint )mesh.Primitive(), mesh.IndexCount(), GL_UNSIGNED_INT, mesh.Indices_Raw() ) );
	}

	void Renderer::Render_NonIndexed( const Mesh& mesh )
	{
		GLCALL( glDrawArrays( ( GLint )mesh.Primitive(), 0, mesh.VertexCount() ) );
	}
}