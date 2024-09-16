// Engine Includes.
#include "Drawable.h"

namespace Engine
{
	Drawable::Drawable()
		:
		transform( nullptr ),
		mesh( nullptr ),
		material( nullptr )
	{
	}

	Drawable::Drawable( const Mesh* mesh, Material* material, Transform* transform )
		:
		transform( transform ),
		mesh( mesh ),
		material( material )
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
}
