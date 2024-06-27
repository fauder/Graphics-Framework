#pragma once

// Engine Includes.
#include "Angle.hpp"
#include "Concepts.h"

// std Includes.
#include <cmath>

/* Forward Declarations. */
namespace Engine::Math
{
	class Polar2;
	class Polar3_Spherical_Game;

	template< typename Component, std::size_t Size > requires( Size > 1 ) 
	class Vector;
}

namespace Engine
{
	using Vector2 = Math::Vector< float, 2 >;
	using Vector3 = Math::Vector< float, 3 >;
}

namespace Engine::Math
{
/* Arithmetic. */
	template< std::floating_point Value >
	Value SquareOf( Value value ) { return std::pow( value, Value{ 2 } ); }

	template< std::floating_point Value >
	Value Abs( const Value value ) { return std::abs( value ); }

	template< std::floating_point Value >
	Value Sqrt( const Value value ) { return std::sqrt( value ); }

	template< typename Value, std::floating_point PercentType >
	Value Lerp( const Value value_a, const Value value_b, const PercentType t ) { return ( PercentType( 1 ) - t ) * value_a + t * value_b; }

	template< std::totally_ordered Value >
	[[ nodiscard( "Clamped value is not assigned back to any variable." ) ]]
	Value Clamp( const Value value, const Value minimum, const Value maximum ) { return value < minimum ? minimum : value > maximum ? maximum : value; }

/* Trigonometry. */
	template< std::floating_point Value >
	Value Hypothenuse( const Value x, const Value y ) { return std::sqrt( x * x + y * y ); }

	template< std::floating_point Value >
	Value NonHyptothenuseEdge( const Value edge, const Value hyptothenuse ) { return std::sqrt( hyptothenuse * hyptothenuse - edge * edge ); }

	template< std::floating_point Value >
	Value Sin( const Radians< Value > angle ) { return std::sin( Value( angle ) ); }

	template< std::floating_point Value >
	Value Cos( const Radians< Value > angle ) { return std::cos( Value( angle ) ); }

	template< std::floating_point Value >
	Value SinFromCos( const Value cos ) { return std::sqrt( Value( 1 ) - cos * cos ); }

	template< std::floating_point Value >
	Value CosFromSin( const Value sin ) { return std::sqrt( Value( 1 ) - sin * sin ); }

	template< std::floating_point Value >
	Value Tan( const Radians< Value > angle ) { return std::tan( Value( angle ) ); }

	template< std::floating_point Value >
	Radians< Value > Acos( const Value cosine ) { return Radians< Value >( std::acos( cosine ) ); }

	template< std::floating_point Value >
	Radians< Value > Asin( const Value sine ) { return Radians< Value >( std::asin( sine ) ); }

	template< std::floating_point Value >
	Radians< Value > Atan( const Value slope ) { return Radians< Value >( std::atan( slope ) ); }

	template< std::floating_point Value >
	Radians< Value > Atan2( const Value y, const Value x ) { return Radians< Value >( std::atan2( y, x ) ); }

	template< Concepts::Arithmetic Value, std::size_t Size >
	Radians< Value > Angle( const Vector< Value, Size >& a, const Vector< Value, Size >& b )
	{
	#ifdef _DEBUG
		ASSERT( a.IsNormalized() && R"(Math::Angle(): The vector "a" is not normalized!)" );
		ASSERT( b.IsNormalized() && R"(Math::Angle(): The vector "b" is not normalized!)" );
	#endif

		return Math::Acos( Math::Clamp( Dot( a, b ), Value( -1 ), Value( +1 ) ) );
	}

/* Conversions Between Cartesian, Polar, Cylindrical & Spherical Coordinates. */
	Polar2 ToPolar2( const Vector2& cartesian );
	Vector2 ToVector2( const Polar2& polar2 );
	Polar3_Spherical_Game ToPolar3_Spherical_Game( const Vector3& cartesian );
	Vector3 ToVector3( const Polar3_Spherical_Game& polar3 );
}