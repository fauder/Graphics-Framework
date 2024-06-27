// Engine Includes.
#include "Math.hpp"
#include "Polar.h"
#include "Vector.hpp"

namespace Engine::Math
{
/* Conversions Between Cartesian <=> Polar/Spherical Coordinates. */

	Polar2 ToPolar2( const Vector2& cartesian )
	{
		if( cartesian.IsZero() )
			return Polar2( ZERO_INITIALIZATION );

		return Polar2( cartesian.Magnitude(), Math::Atan2( cartesian.Y(), cartesian.X() ) );
	}

	Vector2 ToVector2( const Polar2& polar2 )
	{
		const auto r = polar2.R();

		if( IsZero( r ) )
			return Vector2( ZERO_INITIALIZATION );


		return Vector2( Math::Cos( polar2.Theta() ) * r, Math::Sin( polar2.Theta() ) * r );
	}

	Polar3_Spherical_Game ToPolar3_Spherical_Game( const Vector3& cartesian )
	{
		const auto x = cartesian.X(), y = cartesian.Y(), z = cartesian.Z();

		if( cartesian.IsZero() )
			return Polar3_Spherical_Game( ZERO_INITIALIZATION );

		const auto xz_projection_of_r = Math::Hypothenuse( x, z );

		return Polar3_Spherical_Game( cartesian.Magnitude(), Math::Atan2( x, z ), Math::Atan2( -y, xz_projection_of_r ) );
	}

	Vector3 ToVector3( const Polar3_Spherical_Game& polar3 )
	{
		const auto r = polar3.R();

		if( IsZero( r ) )
			return Vector3( ZERO_INITIALIZATION );

		const auto xz_projection_of_r = r * Math::Cos( polar3.Pitch() );

		return Vector3( xz_projection_of_r * Math::Sin( polar3.Heading() ), 
						r * -Math::Sin( polar3.Pitch() ),
						xz_projection_of_r * Math::Cos( polar3.Heading() ) );
	}
}