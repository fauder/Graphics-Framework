#pragma once

// Engine Includes.
#include "Math/Constants.h"

// std includes.
#include <concepts>

namespace Engine
{
	template< typename T > struct TypeTraits
	{};

	template<> struct TypeTraits< float >
	{
		TypeTraits() = delete;

		static constexpr float Epsilon()			{ return 10e-5f;			}
		static constexpr float TwoEpsilon()			{ return Epsilon() * 2.0f;	}
		static constexpr float OneMinusEpsilon()	{ return 1.0f - Epsilon();	}

	};

	template<> struct TypeTraits< double >
	{
		TypeTraits() = delete;

		static constexpr double Epsilon()			{ return 10e-15;			}
		static constexpr double TwoEpsilon()		{ return Epsilon() * 2.0;	}
		static constexpr double OneMinusEpsilon()	{ return 1.0 - Epsilon();	}
	};

	namespace Math
	{
		/* Based on Magnum's TypeTraits::Equals(). */
		template< std::floating_point Value >
		constexpr bool IsEqual( Value lhs, Value rhs, const float epsilon = TypeTraits< Value >::Epsilon() )
		{
			if( lhs == rhs ) // Check for binary and infinity equalities.
				return true;

			Value abs_lhs = std::abs( lhs );
			Value abs_rhs = std::abs( rhs );
			Value difference = std::abs( lhs - rhs );

			/* Do not use relative error if one of the values is zero or the difference is smaller than the epsilon. */
			if( lhs == Value{} || rhs == Value{} || difference < epsilon )
				return difference < epsilon;

			return difference / std::abs( lhs + rhs ) < epsilon;
		}

		template< std::floating_point Value >
		bool IsEqualSquared( Value lhs, Value rhs, const float epsilon = TypeTraits< Value >::TwoEpsilon() )
		{
			return IsEqual( lhs, rhs, epsilon );
		}

		template< std::floating_point Value >
		constexpr bool IsGreaterThan( Value lhs, Value rhs, const float epsilon = TypeTraits< Value >::Epsilon() ) { return lhs - rhs > epsilon; }

		template< std::floating_point Value >
		bool IsGreaterThanOrEqual( Value lhs, Value rhs, const float epsilon = TypeTraits< Value >::Epsilon() ) {
			return std::abs( lhs - rhs ) < epsilon ||
				lhs - rhs > epsilon;
		}

		template< std::floating_point Value >
		constexpr bool IsLessThan( Value lhs, Value rhs, const float epsilon = TypeTraits< Value >::Epsilon() ) { return rhs - lhs > epsilon; }

		template< std::floating_point Value >
		bool IsLessThanOrEqual( Value lhs, Value rhs, const float epsilon = TypeTraits< Value >::Epsilon() ) {
			return std::abs( rhs - lhs ) < epsilon ||
				rhs - lhs > epsilon;
		}

		template< std::floating_point Value >
		bool IsZero( Value value, const float epsilon = TypeTraits< Value >::Epsilon() ) { return IsEqual( value, Value{ 0 }, epsilon ); }
	}
}
