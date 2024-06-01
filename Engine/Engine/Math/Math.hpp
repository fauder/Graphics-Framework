#pragma once

// std Includes.
#include <cmath>

namespace Engine::Math
{
/* Trigonometry. */
	template< std::floating_point Value >
	Value Hypothenuse( const Value x, const Value y ) { return std::sqrt( x * x + y * y ); }

	template< std::floating_point Value >
	Value NonHyptothenuseEdge( const Value edge, const Value hyptothenuse ) { return std::sqrt( hyptothenuse * hyptothenuse - edge * edge ); }

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
	Value Clamp( const Value value, const Value minimum, const Value maximum ){ return value < minimum ? minimum : value > maximum ? maximum : value; }
}