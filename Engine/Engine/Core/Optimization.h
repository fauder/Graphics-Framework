#pragma once

#if defined( _MSC_VER )
#define UNREACHABLE() __assume( 0 )
#elif defined( __GNUC__ ) || defined( __clang__ )
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE()
#endif
