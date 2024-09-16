// Engine Includes.
#include "Drawable.h"

namespace Engine
{
	Drawable::Drawable()
		:
		transform( nullptr ),
		mesh( nullptr ),
		material( nullptr ),
		is_enabled( false )
	{
	}

	Drawable::Drawable( const Mesh* mesh, Material* material, Transform* transform )
		:
		transform( transform ),
		mesh( mesh ),
		material( material ),
		is_enabled( true )
	{
	}

	Drawable::~Drawable()
	{
	}

	void Drawable::SetMesh( const Mesh* mesh )
	{
		this->mesh = mesh;
	}

	void Drawable::SetMaterial( Material* material )
	{
		this->material = material;
	}

	void Drawable::ToggleOn()
	{
		is_enabled = true;
	}

	void Drawable::ToggleOff()
	{
		is_enabled = false;
	}

	void Drawable::ToggleOnOrOff()
	{
		is_enabled = !is_enabled;
	}

	void Drawable::ToggleOnOrOff( const bool enable )
	{
		is_enabled = enable;
	}
}
