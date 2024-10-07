#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Macros.h"
#include "Math/Concepts.h"

// std Includes.
#include <vector>

namespace Engine
{
	struct VertexAttributeCountAndType
	{
		int count;
		GLenum type;

		inline bool Empty() const { return count == 0; }
	}; 
	
	struct VertexAttribute : VertexAttributeCountAndType
	{
		unsigned int location;
		GLint normalize;

		inline unsigned int Size() const { return count * sizeof( type ); }
	};

	class VertexLayout
	{
	public:
		VertexLayout()
		{}

		/* Prevent copying but allow moving: */
		VertexLayout( const VertexLayout& )				= delete;
		VertexLayout& operator =( const VertexLayout& ) = delete;
		VertexLayout( VertexLayout&& )					= default;
		VertexLayout& operator =( VertexLayout&& )		= default;

		template< typename Collection > 
		VertexLayout( Collection&& attribute_counts_and_types )
		{
			for( const auto& attribute : attribute_counts_and_types )
				if( !attribute.Empty() )
					attributes.push_back( { attribute.count, attribute.type, ( unsigned int )attributes.size(), GL_FALSE } );
		}

		~VertexLayout()
		{}

		template< typename Type >
		void Push( const int count );

		template <>
		void Push< int >( const int count )
		{
			attributes.push_back( { count, GL_INT, ( unsigned int )attributes.size(), GL_FALSE } );
		}

		template <>
		void Push< bool >( const int count )
		{
			attributes.push_back( { count, GL_INT, ( unsigned int )attributes.size(), GL_FALSE } );
		}

		template <>
		void Push< float >( const int count )
		{
			attributes.push_back( { count, GL_FLOAT, ( unsigned int )attributes.size(), GL_FALSE } );
		}

		inline void SetAndEnableAttributes() const
		{
			const unsigned int stride = Stride();
			unsigned int offset = 0;

			for( auto index = 0; index < attributes.size(); index++ )
			{
				const auto& attribute = attributes[ index ];
				glVertexAttribPointer( attribute.location, attribute.count, attribute.type, attribute.normalize, stride, BUFFER_OFFSET( offset ) );
				glEnableVertexAttribArray( attribute.location );
				offset += attribute.Size();
			}
		}

		inline unsigned int Stride() const
		{
			unsigned int stride = 0;
			for( auto index = 0; index < attributes.size(); index++ )
				stride += attributes[ index ].Size();

			return stride;
		}

	private:
		std::vector< VertexAttribute > attributes;
	};
}
