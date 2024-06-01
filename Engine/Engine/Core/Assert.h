#pragma once

#ifdef _DEBUG
#define ASSERT( x ) if( !( x ) ) __debugbreak();
#define ASSERT_DEBUG_ONLY( x ) ASSERT( x )
#else
#define ASSERT( x ) x
#define ASSERT_DEBUG_ONLY( x )
#endif
