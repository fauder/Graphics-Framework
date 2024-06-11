#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <stdexcept>
#include <variant>

namespace Engine
{
	using ShaderTypesVariant = 
		std::variant<
			/* Scalars & vectors: */
			float,			Vector2,	Vector3,	Vector4,
			double,			Vector2D,	Vector3D,	Vector4D,
			int,			Vector2I,	Vector3I,	Vector4I,
			unsigned int,	Vector2U,	Vector3U,	Vector4U,
			bool,			Vector2B,	Vector3B,	Vector4B,
		
			/* Float matrices: */
							Matrix2x2,	Matrix3x3,	Matrix4x4,
							Matrix2x3,	Matrix2x4,
							Matrix3x2,	Matrix3x4,
							Matrix4x2,	Matrix4x3,

			/* Double matrices: */
							Matrix2x2D,	Matrix3x3D,	Matrix4x4D,
							Matrix2x3D,	Matrix2x4D,
							Matrix3x2D,	Matrix3x4D,
							Matrix4x2D,	Matrix4x3D >;

	inline int GetSizeOfType( const GLenum type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT								: return sizeof( float );
			case GL_FLOAT_VEC2							: return sizeof( Vector2 );
			case GL_FLOAT_VEC3							: return sizeof( Vector3 );
			case GL_FLOAT_VEC4							: return sizeof( Vector4 );
			
			case GL_DOUBLE								: return sizeof( double );
			// OpenGL 3.3 does not have vectors of double type.
			
			case GL_INT									: return sizeof( int );
			case GL_INT_VEC2							: return sizeof( Vector2I );
			case GL_INT_VEC3							: return sizeof( Vector3I );
			case GL_INT_VEC4							: return sizeof( Vector4I );
			
			case GL_UNSIGNED_INT						: return sizeof( unsigned int );
			case GL_UNSIGNED_INT_VEC2					: return sizeof( Vector2U );
			case GL_UNSIGNED_INT_VEC3					: return sizeof( Vector3U );
			case GL_UNSIGNED_INT_VEC4					: return sizeof( Vector4U );
			
			case GL_BOOL								: return sizeof( bool );
			case GL_BOOL_VEC2							: return sizeof( Vector2B );
			case GL_BOOL_VEC3							: return sizeof( Vector3B );
			case GL_BOOL_VEC4							: return sizeof( Vector4B );
			
			/* Float matrices: */
			case GL_FLOAT_MAT2 							: return sizeof( Matrix2x2 );
			case GL_FLOAT_MAT3 							: return sizeof( Matrix3x3 );
			case GL_FLOAT_MAT4 							: return sizeof( Matrix4x4 );
			
			case GL_FLOAT_MAT2x3 						: return sizeof( Matrix2x3 );
			case GL_FLOAT_MAT2x4 						: return sizeof( Matrix2x4 );
			case GL_FLOAT_MAT3x2 						: return sizeof( Matrix3x2 );
			case GL_FLOAT_MAT3x4 						: return sizeof( Matrix3x4 );
			case GL_FLOAT_MAT4x2 						: return sizeof( Matrix4x2 );
			case GL_FLOAT_MAT4x3 						: return sizeof( Matrix4x3 );

			// OpenGL 3.3 does not have matrices of double type.

			/* Texture samplers: */
			case GL_SAMPLER_1D 							: return sizeof( int );
			case GL_SAMPLER_2D 							: return sizeof( int );
			case GL_SAMPLER_3D 							: return sizeof( int );
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::GetSizeOfType() called with an unknown GL type!" );
	}

	inline const char* GetNameOfType( const GLenum type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT											: return "float";
			case GL_FLOAT_VEC2										: return "vec2";
			case GL_FLOAT_VEC3										: return "vec3";
			case GL_FLOAT_VEC4										: return "vec4";
			
			case GL_DOUBLE											: return "double";
			// OpenGL 3.3 does not have vectors of double type.
			
			case GL_INT												: return "int";
			case GL_INT_VEC2										: return "ivec2";
			case GL_INT_VEC3										: return "ivec3";
			case GL_INT_VEC4										: return "ivec4";
			
			case GL_UNSIGNED_INT									: return "unsigned int";
			case GL_UNSIGNED_INT_VEC2								: return "uvec2";
			case GL_UNSIGNED_INT_VEC3								: return "uvec3";
			case GL_UNSIGNED_INT_VEC4								: return "uvec4";
			
			case GL_BOOL											: return "bool";
			case GL_BOOL_VEC2										: return "bvec2";
			case GL_BOOL_VEC3										: return "bvec3";
			case GL_BOOL_VEC4										: return "bvec4";
			
			/* Float matrices: */
			case GL_FLOAT_MAT2 										: return "mat2";
			case GL_FLOAT_MAT3 										: return "mat3";
			case GL_FLOAT_MAT4 										: return "mat4";
			
			case GL_FLOAT_MAT2x3 									: return "mat2x3";
			case GL_FLOAT_MAT2x4 									: return "mat2x4";
			case GL_FLOAT_MAT3x2 									: return "mat3x2";
			case GL_FLOAT_MAT3x4 									: return "mat3x4";
			case GL_FLOAT_MAT4x2 									: return "mat4x2";
			case GL_FLOAT_MAT4x3 									: return "mat4x3";

			// OpenGL 3.3 does not have matrices of double type.

			/* Texture samplers: */
			case GL_SAMPLER_1D 										: return "sampler1D";
			case GL_SAMPLER_2D 										: return "sampler2D";
			case GL_SAMPLER_3D 										: return "sampler3D";

			case GL_SAMPLER_CUBE 									: return "samplerCube";
			
			case GL_SAMPLER_1D_SHADOW 								: return "sampler1DShadow";
			case GL_SAMPLER_2D_SHADOW 								: return "sampler2DShadow";
			
			case GL_SAMPLER_1D_ARRAY 								: return "sampler1DArray";
			case GL_SAMPLER_2D_ARRAY 								: return "sampler2DArray";
			
			case GL_SAMPLER_1D_ARRAY_SHADOW 						: return "sampler1DArrayShadow";
			case GL_SAMPLER_2D_ARRAY_SHADOW 						: return "sampler2DArrayShadow";
			
			case GL_SAMPLER_2D_MULTISAMPLE 							: return "sampler2DMS";
			
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY 					: return "sampler2DMSArray";
			
			case GL_SAMPLER_CUBE_SHADOW 							: return "samplerCubeShadow";
			
			case GL_SAMPLER_BUFFER 									: return "samplerBuffer";
			
			case GL_SAMPLER_2D_RECT 								: return "sampler2DRect";
			
			case GL_SAMPLER_2D_RECT_SHADOW 							: return "sampler2DRectShadow";
			
			case GL_INT_SAMPLER_1D 									: return "isampler1D";
			case GL_INT_SAMPLER_2D 									: return "isampler2D";
			case GL_INT_SAMPLER_3D 									: return "isampler3D";
			
			case GL_INT_SAMPLER_CUBE 								: return "isamplerCube";
			
			case GL_INT_SAMPLER_1D_ARRAY 							: return "isampler1DArray";
			case GL_INT_SAMPLER_2D_ARRAY 							: return "isampler2DArray";
			
			case GL_INT_SAMPLER_2D_MULTISAMPLE 						: return "isampler2DMS";
			
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 				: return "isampler2DMSArray";
			
			case GL_INT_SAMPLER_BUFFER 								: return "isamplerBuffer";
			
			case GL_INT_SAMPLER_2D_RECT 							: return "isampler2DRect";
			
			case GL_UNSIGNED_INT_SAMPLER_1D 						: return "usampler1D";
			case GL_UNSIGNED_INT_SAMPLER_2D 						: return "usampler2D";
			case GL_UNSIGNED_INT_SAMPLER_3D 						: return "usampler3D";
			
			case GL_UNSIGNED_INT_SAMPLER_CUBE 						: return "usamplerCube";
			
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 					: return "usampler2DArray";
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 					: return "usampler2DArray";
			
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 			: return "usampler2DMS";
			
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 		: return "usampler2DMSArray";
			
			case GL_UNSIGNED_INT_SAMPLER_BUFFER 					: return "usamplerBuffer";
			
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT 					: return "usampler2DRect";
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::GetNameOfType() called with an unknown GL type!" );
	}
}

// TODO: Add missing types as they are needed.
/* Types not added yet:
 * 
 * GL_DOUBLE_VEC2 	dvec2				 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_VEC3 	dvec3				 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_VEC4 	dvec4				 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * 
 * GL_DOUBLE_MAT2 	dmat2				 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_MAT3 	dmat3				 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_MAT4 	dmat4				 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * 
 * GL_DOUBLE_MAT2x3 	dmat2x3			 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_MAT2x4 	dmat2x4			 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_MAT3x2 	dmat3x2			 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_MAT3x4 	dmat3x4			 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_MAT4x2 	dmat4x2			 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * GL_DOUBLE_MAT4x3						 // Does not exist in OpenGL 3.3 (supported starting from OpenGL 4.0)
 * 
 * GL_SAMPLER_CUBE 	samplerCube
 * 
 * GL_SAMPLER_1D_SHADOW 	sampler1DShadow
 * GL_SAMPLER_2D_SHADOW 	sampler2DShadow
 * 
 * GL_SAMPLER_1D_ARRAY 	sampler1DArray
 * GL_SAMPLER_2D_ARRAY 	sampler2DArray
 * 
 * GL_SAMPLER_1D_ARRAY_SHADOW 	sampler1DArrayShadow
 * GL_SAMPLER_2D_ARRAY_SHADOW 	sampler2DArrayShadow
 * 
 * GL_SAMPLER_2D_MULTISAMPLE 	sampler2DMS
 * 
 * GL_SAMPLER_2D_MULTISAMPLE_ARRAY 	sampler2DMSArray
 * 
 * GL_SAMPLER_CUBE_SHADOW 	samplerCubeShadow
 * 
 * GL_SAMPLER_BUFFER 	samplerBuffer
 * 
 * GL_SAMPLER_2D_RECT 	sampler2DRect
 * 
 * GL_SAMPLER_2D_RECT_SHADOW 	sampler2DRectShadow
 * 
 * GL_INT_SAMPLER_1D 	isampler1D
 * GL_INT_SAMPLER_2D 	isampler2D
 * GL_INT_SAMPLER_3D 	isampler3D
 * 
 * GL_INT_SAMPLER_CUBE 	isamplerCube
 * 
 * GL_INT_SAMPLER_1D_ARRAY 	isampler1DArray
 * GL_INT_SAMPLER_2D_ARRAY 	isampler2DArray
 * 
 * GL_INT_SAMPLER_2D_MULTISAMPLE 	isampler2DMS
 * 
 * GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 	isampler2DMSArray
 * 
 * GL_INT_SAMPLER_BUFFER 	isamplerBuffer
 * 
 * GL_INT_SAMPLER_2D_RECT 	isampler2DRect
 * 
 * GL_UNSIGNED_INT_SAMPLER_1D 	usampler1D
 * GL_UNSIGNED_INT_SAMPLER_2D 	usampler2D
 * GL_UNSIGNED_INT_SAMPLER_3D 	usampler3D
 * 
 * GL_UNSIGNED_INT_SAMPLER_CUBE 	usamplerCube
 * 
 * GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 	usampler2DArray
 * GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 	usampler2DArray
 * 
 * GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 	usampler2DMS
 * 
 * GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 	usampler2DMSArray
 * 
 * GL_UNSIGNED_INT_SAMPLER_BUFFER 	usamplerBuffer
 * 
 * GL_UNSIGNED_INT_SAMPLER_2D_RECT 	usampler2DRect
 * 
 * 
 * 
 * 
 *							// Below do not exist in OpenGL 3.3.
 * 
 * 
 * 
 * 
 * 
 * GL_IMAGE_1D 	image1D
 * GL_IMAGE_2D 	image2D
 * GL_IMAGE_3D 	image3D
 * 
 * GL_IMAGE_2D_RECT 	image2DRect
 * 
 * GL_IMAGE_CUBE 	imageCube
 * 
 * GL_IMAGE_BUFFER 	imageBuffer
 * 
 * GL_IMAGE_1D_ARRAY 	image1DArray
 * GL_IMAGE_2D_ARRAY 	image2DArray
 * 
 * GL_IMAGE_2D_MULTISAMPLE 	image2DMS
 * 
 * GL_IMAGE_2D_MULTISAMPLE_ARRAY 	image2DMSArray
 * 
 * GL_INT_IMAGE_1D 	iimage1D
 * GL_INT_IMAGE_2D 	iimage2D
 * GL_INT_IMAGE_3D 	iimage3D
 * 
 * GL_INT_IMAGE_2D_RECT 	iimage2DRect
 * 
 * GL_INT_IMAGE_CUBE 	iimageCube
 * 
 * GL_INT_IMAGE_BUFFER 	iimageBuffer
 * 
 * GL_INT_IMAGE_1D_ARRAY 	iimage1DArray
 * GL_INT_IMAGE_2D_ARRAY 	iimage2DArray
 * 
 * GL_INT_IMAGE_2D_MULTISAMPLE 	iimage2DMS
 * 
 * GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY 	iimage2DMSArray
 * 
 * GL_UNSIGNED_INT_IMAGE_1D 	uimage1D
 * GL_UNSIGNED_INT_IMAGE_2D 	uimage2D
 * GL_UNSIGNED_INT_IMAGE_3D 	uimage3D
 * 
 * GL_UNSIGNED_INT_IMAGE_2D_RECT 	uimage2DRect
 * 
 * GL_UNSIGNED_INT_IMAGE_CUBE 	uimageCube
 * 
 * GL_UNSIGNED_INT_IMAGE_BUFFER 	uimageBuffer
 * 
 * GL_UNSIGNED_INT_IMAGE_1D_ARRAY 	uimage1DArray
 * GL_UNSIGNED_INT_IMAGE_2D_ARRAY 	uimage2DArray
 * 
 * GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE 	uimage2DMS
 * 
 * GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY 	uimage2DMSArray
 * 
 * GL_UNSIGNED_INT_ATOMIC_COUNTER 	atomic_uint
 */