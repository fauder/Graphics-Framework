#version 460 core

#include "_Intrinsic_Other.glsl"

layout ( triangles ) in;
layout ( line_strip, max_vertices = 6 ) out;

in vec4 varying_normal_view_space[];

const float MAGNITUDE = 0.4;

void GenerateVertexNormalLine( int vertex_index )
{
    gl_Position = gl_in[ vertex_index ].gl_Position * _INTRINSIC_TRANSFORM_PROJECTION;
    EmitVertex();

    gl_Position += vec4( gl_in[ vertex_index ].gl_Position + varying_normal_view_space[ vertex_index ] * MAGNITUDE ) * _INTRINSIC_TRANSFORM_PROJECTION;
    EmitVertex();

    EndPrimitive();
}

void main()
{
   GenerateVertexNormalLine( 0 );
   GenerateVertexNormalLine( 1 );
   GenerateVertexNormalLine( 2 );
}  