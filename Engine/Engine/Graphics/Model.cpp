// Engine Includes.
#include "Model.h"

namespace Engine
{
	Model::Model()
		:
		name( "<unnamed>" )
	{}

	Model::Model( const std::string& name )
		: name( name )
	{
	}

	Model::~Model()
	{
	}
}
