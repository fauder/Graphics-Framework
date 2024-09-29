#pragma once

// Engine Includes.
#include "Graphics/Std140StructTag.h"

template< unsigned int RowSize, unsigned int ColumnSize > requires( RowSize >= 1 && ColumnSize >= 1 )
struct KernelData : public Engine::Std140StructTag
{
	float values[ RowSize * ColumnSize ];
};
