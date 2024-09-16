// Engine Includes.
#include "Model.h"

namespace Engine
{
	Model::Node::Node( const std::string& name, const Matrix4x4& transform_local, MeshGroup* mesh_group )
		:
		name( name ),
		transform_local( transform_local ),
		mesh_group( mesh_group )
	{
	}

	Model::Node::Node( Node&& donor )
		:
		children( std::move( donor.children ) ),
		name( std::move( donor.name ) ),
		transform_local( std::move( donor.transform_local ) ),
		mesh_group( std::exchange( donor.mesh_group, nullptr ) )
	{
	}

	Model::Node& Model::Node::operator=( Node&& donor )
	{
		children        = std::move( donor.children );
		name            = std::move( donor.name );
		transform_local = std::move( donor.transform_local );
		mesh_group      = std::exchange( donor.mesh_group, nullptr );

		return *this;
	}

	Model::Model()
		:
		name( "<unnamed>" )
	{}

	Model::Model( const std::string& name )
		:
		name( name )
	{
	}

	Model::~Model()
	{
	}
}
