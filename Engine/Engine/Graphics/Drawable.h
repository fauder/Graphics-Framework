#pragma once

// Engine Includes.
#include "Material.hpp"
#include "Mesh.h"
#include "Scene/Transform.h"

namespace Engine
{
	class Renderer;

	class Drawable
	{
		friend class Renderer;

	public:
		Drawable();
		Drawable( const Mesh* mesh, Material* material, Transform* transform );

		Drawable( const Drawable& other )				= default;
		Drawable& operator = ( const Drawable& other )	= default;

		Drawable( Drawable&& donor )					= default;
		Drawable& operator = ( Drawable&& donor )		= default;

		~Drawable();

		void SetMesh( const Mesh* mesh );
		void SetMaterial( Material* material );

	private:
		Transform* transform;
		const Mesh* mesh;
		Material* material;
	};
}
