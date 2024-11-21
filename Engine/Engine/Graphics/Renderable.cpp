// Engine Includes.
#include "Renderable.h"

namespace Engine
{
	Renderable::Renderable()
		:
		transform( nullptr ),
		mesh( nullptr ),
		material( nullptr ),
		is_enabled( false ),
		is_receiving_shadows( false )
	{
	}

	Renderable::Renderable( const Mesh* mesh, Material* material, Transform* transform, const bool receive_shadows )
		:
		transform( transform ),
		mesh( mesh ),
		material( material ),
		is_enabled( true ),
		is_receiving_shadows( receive_shadows )
	{
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::SetMesh( const Mesh* mesh )
	{
		this->mesh = mesh;
	}

	void Renderable::SetMaterial( Material* material )
	{
		this->material = material;
	}

	void Renderable::ToggleOn()
	{
		is_enabled = true;
	}

	void Renderable::ToggleOff()
	{
		is_enabled = false;
	}

	void Renderable::ToggleOnOrOff()
	{
		is_enabled = !is_enabled;
	}

	void Renderable::ToggleOnOrOff( const bool enable )
	{
		is_enabled = enable;
	}
}
