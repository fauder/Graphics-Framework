#pragma once

// Engince Includes.
#include "Concepts.h"
#include "Vector.hpp"
#include "Graphics/Color.hpp"

// std Includes.
#include <random>

namespace Engine::Math
{
	// Singleton.
	class Random
	{
	public:
		Random( Random const& )				= delete;
		void operator = ( Random const& )	= delete;

		static void Seed( const unsigned int seed )
		{
			Instance( seed );
		}

		static void SeedRandom()
		{
			Instance( std::random_device()( ) );
		}

		template< Concepts::Arithmetic Type >
		static Type Generate( const Type min = Type( 0 ), const Type max = Type( 1 ) )
		{
			auto& instance = Instance();

			if constexpr( std::is_integral_v< Type > )
			{
				std::uniform_int_distribution< Type > uniform_dist( min, max );
				return uniform_dist( instance.engine );
			}
			else
			{
				std::uniform_real_distribution< Type > uniform_dist( min, max );
				return uniform_dist( instance.engine );
			}
		}

		template< typename VectorType > requires( VectorType::Dimension() > 1 )
		static VectorType Generate( const VectorType& min = VectorType::Zero(), 
									const VectorType& max = VectorType::One() )
		{
			/*static_assert( !std::is_same_v< VectorType, Color3 > && !std::is_same_v< VectorType, Color4 >,
						   "Please use the Generate( const Color3/4 min, const Color3/4 max ) overload instead." );*/
			auto& instance = Instance();

			VectorType vector;

			for( auto i = 0; i < VectorType::Dimension(); i++ )
				vector[ i ] = Generate( min[ i ], max[ i ] );

			return vector;
		}

		/*template< typename ColorType > requires( ColorType::Dimension() > 1 && ( std::is_same_v< ColorType, Color3 > || std::is_same_v< ColorType, Color4 > ) )
		static ColorType Generate( const ColorType& min = ColorType::White(), const ColorType& max = ColorType::Black() )
		{
			using UnderlyingVectorType = Vector< ColorType::ComponentType, ColorType::Dimension() >;
			return Generate< UnderlyingVectorType >( reinterpret_cast< const UnderlyingVectorType& >( min ), reinterpret_cast< const UnderlyingVectorType& >( max ) );
		}*/

		//template<>
		//static Color3 Generate< Color3 >( const Color3& min, const Color3& max )
		//{
		//	//return Generate< Vector3 >( reinterpret_cast< const Vector3& >( min ), reinterpret_cast< const Vector3& >( max ) );
		//}

	private:
		Random( const unsigned int seed )
			:
			engine( seed )
		{}

		static Random& Instance( const unsigned int seed = std::default_random_engine::default_seed )
		{
			static Random instance( seed );
			return instance;
		}

	private:
		std::random_device device;
		std::default_random_engine engine;
	};
}
