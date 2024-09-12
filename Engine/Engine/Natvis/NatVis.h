// Engine Includes.
#include "Math/Math.hpp"

namespace NatVis
{
/* Float: */
	inline float SinFromCos( const float value )
	{
		return Engine::Math::SinFromCos( value );
	}

	inline float Acos( const float value )
	{
		return ( float )Engine::Degrees( Engine::Math::Acos( value ) );
	}

	inline float Asin( const float value )
	{
		return ( float )Engine::Degrees( Engine::Math::Asin( value ) );
	}

	inline float Atan2( const float y, const float x )
	{
		return ( float )Engine::Degrees( Engine::Math::Atan2( y, x ) );
	}

/* Double: */

	inline double SinFromCos( const double value )
	{
		return Engine::Math::SinFromCos( value );
	}

	inline double Acos( const double value )
	{
		return ( double )Engine::DegreesD( Engine::Math::Acos( value ) );
	}

	inline double Asin( const double value )
	{
		return ( double )Engine::DegreesD( Engine::Math::Asin( value ) );
	}

	inline double Atan2( const double y, const double x )
	{
		return ( double )Engine::DegreesD( Engine::Math::Atan2( y, x ) );
	}


	/* Since *actual* code never uses this namespace (only the .natvis does), we need to include this call somewhere. */
	inline void ForceIncludeInBuild()
	{
/* Float: */
		SinFromCos( 1.0f );
		Acos( 1.0f );
		Asin( 1.0f );
		Atan2( 1.0f, 0.0f );

/* Double: */
		SinFromCos( 1.0 );
		Acos( 1.0 );
		Asin( 1.0 );
		Atan2( 1.0, 0.0 );
	}
}