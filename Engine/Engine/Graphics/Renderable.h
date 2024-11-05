#pragma once

// Engine Includes.
#include "Material.hpp"
#include "Mesh.h"
#include "Scene/Transform.h"

namespace Engine
{
	class Renderer;

	class Renderable
	{
		friend class Renderer;

	public:
		Renderable();
		Renderable( const Mesh* mesh, Material* material, Transform* transform = nullptr );

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Renderable );

		~Renderable();

		const Transform*	GetTransform()	const { return transform;	}
		const Mesh*			GetMesh()		const { return mesh;		}
		const Material*		GetMaterial()	const { return material;	}

		void SetMesh( const Mesh* mesh );
		void SetMaterial( Material* material );

		void ToggleOn();
		void ToggleOff();
		void ToggleOnOrOff();
		void ToggleOnOrOff( const bool enable );

	private:
		Transform* transform;
		const Mesh* mesh;
		Material* material;

		bool is_enabled;
		//bool padding[ 7 ];
	};
}
