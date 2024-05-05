#pragma once

#ifdef _DEBUG
#define ASSERT( x ) if( !( x ) ) __debugbreak();
#else
#define ASSERT( x ) x
#endif
