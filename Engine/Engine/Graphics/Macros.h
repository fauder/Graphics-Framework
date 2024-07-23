#pragma once

// std Includes.
#include <cstddef> // std::byte.

#define BUFFER_OFFSET( idx ) ( static_cast< std::byte* >( 0 ) + ( idx ) )

