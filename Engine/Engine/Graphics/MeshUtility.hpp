#pragma once

// Engine Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <utility>

namespace Engine
{
	namespace MeshUtility
	{
		template< typename VertexAttributeArray, typename ... VertexAttributeArrays >
		constexpr auto Interleave( const VertexAttributeArray& vertex_attribute_array_first, const VertexAttributeArrays& ... vertex_attribute_array_pack )
		{
			// TODO: ASSERT that the array sizes are the same.
			// TODO: ASSERT that the array element types are the same.

			constexpr std::size_t attribute_count_first = std::remove_reference_t< decltype( vertex_attribute_array_first[ 0 ] ) >::Dimension();
			constexpr std::size_t attribute_count       = attribute_count_first + ( std::remove_reference_t< decltype( vertex_attribute_array_pack[ 0 ] ) >::Dimension() + ... );

			constexpr std::size_t element_count = std::tuple_size< std::remove_reference_t< decltype( vertex_attribute_array_first ) > >::value;

			using ComponentType = std::remove_reference_t< decltype( vertex_attribute_array_first[ 0 ] ) >::ComponentType;

			std::array< Math::Vector< ComponentType, attribute_count >, element_count > interleaved_vertex_attribute_array;

			for( std::size_t i = 0; i < element_count; i++ )
			{
				for( std::size_t j = 0; j < attribute_count; )
				{
					/* Below for loop covers the assignment of the elements of the FIRST attribute array. */
					for( std::size_t element_index = 0; element_index < attribute_count_first; element_index++ )
						interleaved_vertex_attribute_array[ i ][ j++ ] = vertex_attribute_array_first[ i ][ element_index ];

					/* Below fold expression of the lambda over the comma operator covers the assignment of the rest; i.e, array elements in the attribute array PACK. */
					( [&]
					{
						constexpr std::size_t dimension = std::remove_reference_t< decltype( vertex_attribute_array_pack[ 0 ] ) >::Dimension();
						for( std::size_t element_index = 0; element_index < dimension; element_index++ )
							interleaved_vertex_attribute_array[ i ][ j++ ] = vertex_attribute_array_pack[ i ][ element_index ];
					}() , ... ); // Fold expression: Lambda over the comma operator.
				}
			}

			return interleaved_vertex_attribute_array;
		}
	}
}
