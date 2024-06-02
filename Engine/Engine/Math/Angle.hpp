#pragma once

// Engine Includes.
#include "Math/Concepts.h"
#include "Math/Constants.h"
#include "Math/Unit.hpp"

// std Includes.
#include <cmath>

namespace Engine
{
	namespace Math
	{
		template< std::floating_point > class Degrees;
		template< std::floating_point > class Radians;

		template< std::floating_point FloatType >
		class Degrees : public Unit< FloatType, Degrees >
		{
			using Base = Unit< FloatType, Degrees >;

		public:
		/* Constructors. */
			constexpr Degrees() : Base() {}
			constexpr Degrees( const Degrees& other ) : Base( other ) {}
			
			explicit Degrees( Initialization::NoInitialization ) : Base( NO_INITIALIZATION ) {}

			constexpr explicit Degrees( FloatType value ) : Base( value ) {}

			/* Construct (convert) from other underlying type. */
			template< typename OtherType >
			constexpr Degrees( const Unit< OtherType, Degrees >& other ) : Base( other ) {}

			/* Construct from Radians. */
			constexpr Degrees( const Unit< FloatType, Radians >& radians ) : Base( FloatType( radians ) * Constants< FloatType >::Rad_To_Deg() ) {}
			
		/* Trigonometric Functions. */
			/* Wraps a given angle between (-180, +180] range. */
			Degrees Wrapped180( Degrees angle ) const
			{
				return Degrees( angle - FloatType( 360 ) * std::floor( ( angle + FloatType( 180 ) ) / FloatType( 360 ) ) );
			}

			/* Wraps a given angle between (-180, +180] range. Returns self for daisy-chaining. */
			Degrees& Wrap180( Degrees angle )
			{
				Base::value = angle - FloatType( 360 ) * std::floor( ( angle + FloatType( 180 ) ) / FloatType( 360 ) );
				return *this;
			}
		};

		template< std::floating_point FloatType >
		class Radians : public Unit< FloatType, Radians >
		{
			using Base = Unit< FloatType, Radians >;

		public:
		/* Constructors. */
			constexpr Radians() : Base() {}
			constexpr Radians( const Radians& other ) : Base( other ) {}

			explicit Radians( Initialization::NoInitialization ) : Base( NO_INITIALIZATION ) {}

			constexpr explicit Radians( FloatType value ) : Base( value ) {}

			/* Construct (convert) from other underlying type. */
			template< typename OtherType >
			constexpr Radians( const Unit< OtherType, Radians >& other ) : Base( other ) {}

			/* Construct from Degrees. */
			constexpr Radians( const Unit< FloatType, Degrees >& degrees ) : Base( FloatType( degrees ) * Constants< FloatType >::Deg_To_Rad() ) {}
		
		/* Trigonometric Functions. */
			/* Wraps a given angle between (-PI, +PI] range. */
			Radians Wrapped180( Radians angle ) const
			{
				return Radians( angle - FloatType( Constants< FloatType >::Two_Pi() ) * std::floor( ( angle + FloatType( Constants< FloatType >::Pi() ) ) / FloatType( Constants< FloatType >::Two_Pi() ) ) );
			}

			/* Wraps a given angle between (-PI, +PI] range. Returns self for daisy-chaining. */
			Radians& Wrap180( Radians angle )
			{
				Base::value = angle - FloatType( Constants< FloatType >::Two_Pi() ) * std::floor( ( angle + FloatType( Constants< FloatType >::Pi() ) ) / FloatType( Constants< FloatType >::Two_Pi() ) );
				return *this;
			}
		};

		namespace Literals
		{
			constexpr Math::Degrees< float > operator"" _deg( long double value )
			{
				return Math::Degrees< float >( static_cast< float >( value ) );
			}

			constexpr Math::Degrees< float > operator"" _deg( unsigned long long value )
			{
				return Math::Degrees< float >( static_cast< float >( value ) );
			}

			constexpr Math::Degrees< double > operator"" _degd( long double value )
			{
				return Math::Degrees< double >( static_cast< double >( value ) );
			}

			constexpr Math::Degrees< double > operator"" _degd( unsigned long long value )
			{
				return Math::Degrees< double >( static_cast< double >( value ) );
			}

			constexpr Math::Radians< float > operator"" _rad( long double value )
			{
				return Math::Radians< float >( static_cast< float >( value ) );
			}

			constexpr Math::Radians< float > operator"" _rad( unsigned long long value )
			{
				return Math::Radians< float >( static_cast< float >( value ) );
			}

			constexpr Math::Radians< double > operator"" _radd( long double value )
			{
				return Math::Radians< double >( static_cast< double >( value ) );
			}

			constexpr Math::Radians< double > operator"" _radd( unsigned long long value )
			{
				return Math::Radians< double >( static_cast< double >( value ) );
			}
		};
	}

	using Degrees = Math::Degrees< float >;
	using Radians = Math::Radians< float >;

	using DegreesD = Math::Degrees< double >;
	using RadiansD = Math::Radians< double >;

	template<> struct Constants< Degrees >
	{
		Constants() = delete;

		static constexpr Degrees Pi()				{ return Degrees( Constants< float >::Pi() );	}
		static constexpr Degrees Pi_Over_Two()		{ return Pi() / 2.0f;							}
		static constexpr Degrees Pi_Over_Three()	{ return Pi() / 3.0f;							}
		static constexpr Degrees Pi_Over_Four()		{ return Pi() / 4.0f;							}
		static constexpr Degrees Pi_Over_Six()		{ return Pi() / 6.0f;							}
		static constexpr Degrees Two_Pi()			{ return Pi() * 2.0f;							}
	};

	template<> struct Constants< Radians >
	{
		Constants() = delete;

		static constexpr Radians Pi()				{ return Radians( Constants< float >::Pi() );	}
		static constexpr Radians Pi_Over_Two()		{ return Pi() / 2.0f;							}
		static constexpr Radians Pi_Over_Three()	{ return Pi() / 3.0f;							}
		static constexpr Radians Pi_Over_Four()		{ return Pi() / 4.0f;							}
		static constexpr Radians Pi_Over_Six()		{ return Pi() / 6.0f;							}
		static constexpr Radians Two_Pi()			{ return Pi() * 2.0f;							}
	};

	template<> struct Constants< DegreesD >
	{
		Constants() = delete;

		static constexpr DegreesD Pi()				{ return DegreesD( Constants< double >::Pi() );	}
		static constexpr DegreesD Pi_Over_Two()		{ return Pi() / 2.0;							}
		static constexpr DegreesD Pi_Over_Three()	{ return Pi() / 3.0;							}
		static constexpr DegreesD Pi_Over_Four()	{ return Pi() / 4.0;							}
		static constexpr DegreesD Pi_Over_Six()		{ return Pi() / 6.0;							}
		static constexpr DegreesD Two_Pi()			{ return Pi() * 2.0;							}
	};

	template<> struct Constants< RadiansD >
	{
		Constants() = delete;

		static constexpr RadiansD Pi()				{ return RadiansD( Constants< double >::Pi() );	}
		static constexpr RadiansD Pi_Over_Two()		{ return Pi() / 2.0;							}
		static constexpr RadiansD Pi_Over_Three()	{ return Pi() / 3.0;							}
		static constexpr RadiansD Pi_Over_Four()	{ return Pi() / 4.0;							}
		static constexpr RadiansD Pi_Over_Six()		{ return Pi() / 6.0;							}
		static constexpr RadiansD Two_Pi()			{ return Pi() * 2.0;							}
	};
}
