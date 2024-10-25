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
				std::vector< std::uint32_t >&&	indices,
				const PrimitiveType				primitive_type,
				const GLenum					usage,
				/* Below are seldom used so they come after. */
				std::vector< Vector2 >&&		uvs_1,
				std::vector< Vector2 >&&		uvs_2,
				std::vector< Vector2 >&&		uvs_3,
				std::vector< Color4 >&&			colors_rgba )
		:
		name( name ),
		indices( indices ),
		positions( positions ),
		normals( normals ),
		uvs_0( uvs_0 ),
		uvs_1( uvs_1 ),
		uvs_2( uvs_2 ),
		uvs_3( uvs_3 ),
		colors_rgba( colors_rgba ),
		primitive_type( primitive_type ),
		instance_count( 1 )
	{
		unsigned int vertex_count_interleaved;
		const auto interleaved_vertices = MeshUtility::Interleave( vertex_count_interleaved, positions, normals, uvs_0, uvs_1, uvs_2, uvs_3, colors_rgba );

		vertex_buffer = VertexBuffer( vertex_count_interleaved, std::span( interleaved_vertices ), name + " Vertex Buffer", usage );
		vertex_layout = VertexLayout( GatherAttributes( positions, normals, uvs_0, uvs_1, uvs_2, uvs_3, colors_rgba ) );
		index_buffer  = indices.empty() ? std::nullopt : std::optional< IndexBuffer >( std::in_place, std::span( indices ), name + " Index Buffer", usage );
		vertex_array  = VertexArray( vertex_buffer, vertex_layout, index_buffer, name + " VAO");
	}

	Mesh::Mesh( const Mesh& other,
				const std::initializer_list< VertexInstanceAttribute > instanced_attributes,
				const std::vector< float >& instance_data,
				const int instance_count,
				const GLenum instance_buffer_usage )
		:
		name( other.name + " (instanced)" ),
		indices( other.indices ),
		positions( other.positions ),
		normals( other.normals ),
		uvs_0( other.uvs_0 ),
		uvs_1( other.uvs_1 ),
		uvs_2( other.uvs_2 ),
		uvs_3( other.uvs_3 ),
		colors_rgba( other.colors_rgba ),
		primitive_type( other.primitive_type ),
		instance_count( instance_count ),
		vertex_buffer( other.vertex_buffer ),
		vertex_layout( other.vertex_layout ),
		index_buffer( other.index_buffer )
	{
		for( auto instanced_attribute_iterator = instanced_attributes.begin(); instanced_attribute_iterator != instanced_attributes.end(); instanced_attribute_iterator++ )
			vertex_layout.Push( *instanced_attribute_iterator );

		instance_buffer = std::optional< InstanceBuffer >( std::in_place, instance_count, std::span( instance_data ), name + " Instance Buffer", instance_buffer_usage );

		vertex_array = VertexArray( vertex_buffer, vertex_layout, index_buffer, *instance_buffer, name + " VAO (instanced)");
	}

	Mesh::~Mesh()
	{}

	void Mesh::Update( const void* data ) const
	{
		vertex_buffer.Update( data );
	}

	void Mesh::Update_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const
	{
		vertex_buffer.Update_Partial( data_span, offset_from_buffer_start );
	}

	void Mesh::UpdateInstanceData( const void* data ) const
	{
		ASSERT_DEBUG_ONLY( instance_buffer && "UpdateInstanceData() called on non-instanced Mesh!" );

		instance_buffer->Update( data );
	}

	void Mesh::UpdateInstanceData_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const
	{
		ASSERT_DEBUG_ONLY( instance_buffer && "UpdateInstanceData_Partial() called on non-instanced Mesh!" );

		instance_buffer->Update_Partial( data_span, offset_from_buffer_start );
	}

	std::array< VertexAttribute, 7 > Mesh::GatherAttributes( const std::vector< Vector3 >& positions, const std::vector< Vector3 >& normals,
															 const std::vector< Vector2 >& uvs_0, const std::vector< Vector2 >& uvs_1,
															 const std::vector< Vector2 >& uvs_2, const std::vector< Vector2 >& uvs_3,
															 const std::vector< Color4 >& colors_rgba )
	{
		auto CountOf = []( auto&& attribute_container ) { return attribute_container.empty() ? 0 : int( sizeof( attribute_container.front() ) / sizeof( float ) ); };

		constexpr bool is_instanced = false;

		return std::array< VertexAttribute, 7 >
		( {
			VertexAttribute{ CountOf( positions ),		GL_FLOAT,	is_instanced },
			VertexAttribute{ CountOf( normals ),		GL_FLOAT,	is_instanced },
			VertexAttribute{ CountOf( uvs_0 ),			GL_FLOAT,	is_instanced },
			VertexAttribute{ CountOf( uvs_1 ),			GL_FLOAT,	is_instanced },
			VertexAttribute{ CountOf( uvs_2 ),			GL_FLOAT,	is_instanced },
			VertexAttribute{ CountOf( uvs_3 ),			GL_FLOAT,	is_instanced },
			VertexAttribute{ CountOf( colors_rgba ),	GL_FLOAT,	is_instanced }
		} );
	}
}
