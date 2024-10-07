// Engine Includes.
#include "Mesh.h"

namespace Engine
{
	Mesh::Mesh()
		:
		primitive_type( PrimitiveType::Triangles )
	{}

	Mesh::Mesh( std::vector< Vector3 >&&		positions,
				const std::string&				name,
				std::vector< Vector3 >&&		normals,
				std::vector< Vector2 >&&		uvs_0,
				std::vector< std::uint16_t >&&	indices_u16,
				std::vector< std::uint32_t >&&	indices_u32,
				const PrimitiveType				primitive_type,
				const GLenum					usage,
				/* Below are seldom used so they come after. */
				std::vector< Vector2 >&&		uvs_1,
				std::vector< Vector2 >&&		uvs_2,
				std::vector< Vector2 >&&		uvs_3,
				std::vector< Color4 >&&			colors_rgba )
		:
		primitive_type( primitive_type ),
		indices_u16( indices_u16 ),
		indices_u32( indices_u32 ),
		positions( positions ),
		normals( normals ),
		uvs_0( uvs_0 ),
		uvs_1( uvs_1 ),
		uvs_2( uvs_2 ),
		uvs_3( uvs_3 ),
		colors_rgba( colors_rgba ),
		name( name )
	{
		unsigned int vertex_count_interleaved;
		const auto interleaved_vertices = MeshUtility::Interleave( vertex_count_interleaved, positions, normals, uvs_0, uvs_1, uvs_2, uvs_3, colors_rgba );

		vertex_buffer    = VertexBuffer( vertex_count_interleaved, std::span( interleaved_vertices ), name + " Vertex Buffer", usage );
		vertex_layout    = VertexLayout( AttributeCountsAndTypes( positions, normals, uvs_0, uvs_1, uvs_2, uvs_3, colors_rgba ) );
		index_buffer_u16 = indices_u16.empty() ? std::nullopt : std::optional< IndexBuffer_U16 >( std::in_place, indices_u16, name + " Index Buffer (U16)", usage );
		index_buffer_u32 = indices_u32.empty() ? std::nullopt : std::optional< IndexBuffer_U32 >( std::in_place, indices_u32, name + " Index Buffer (U32)", usage );
		vertex_array     = VertexArray( vertex_buffer, vertex_layout, index_buffer_u16, index_buffer_u32, name + " VAO");
	}

	Mesh::~Mesh()
	{}

	std::array< VertexAttributeCountAndType, 7 > Mesh::AttributeCountsAndTypes( const std::vector< Vector3 >& positions, const std::vector< Vector3 >& normals,
																				const std::vector< Vector2 >& uvs_0, const std::vector< Vector2 >& uvs_1,
																				const std::vector< Vector2 >& uvs_2, const std::vector< Vector2 >& uvs_3,
																				const std::vector< Color4 >& colors_rgba )
	{
		return
		{ {
			positions.empty()	? 0 : int( sizeof( positions.front() )		/ sizeof( float ) ),	GL_FLOAT,
			normals.empty()		? 0 : int( sizeof( normals.front() )		/ sizeof( float ) ),	GL_FLOAT,
			uvs_0.empty()		? 0 : int( sizeof( uvs_0.front() )			/ sizeof( float ) ),	GL_FLOAT,
			uvs_1.empty()		? 0 : int( sizeof( uvs_1.front() )			/ sizeof( float ) ),	GL_FLOAT,
			uvs_2.empty()		? 0 : int( sizeof( uvs_2.front() )			/ sizeof( float ) ),	GL_FLOAT,
			uvs_3.empty()		? 0 : int( sizeof( uvs_3.front() )			/ sizeof( float ) ),	GL_FLOAT,
			colors_rgba.empty() ? 0 : int( sizeof( colors_rgba.front() )	/ sizeof( float ) ),	GL_FLOAT
		} };
	}
}
