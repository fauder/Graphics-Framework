#pragma once

// Engine Includes.
#include "Math/Angle.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

namespace Engine::Matrix
{
	/* In row-major form. Coordinates are in a left-handed coordinate system (both before & after multiplication). */
	constexpr Matrix4x4 OrthographicProjection( const float left_plane_offset, const float right_plane_offset, const float bottom_plane_offset, const float top_plane_offset, 
												const float near_plane_offset, const float far_plane_offset )
	{
		const float f_plus_n  = far_plane_offset + near_plane_offset;
		const float f_minus_n = far_plane_offset - near_plane_offset;

		return Matrix4x4
		(
			{
				1.0f / right_plane_offset,			0.0f,								0.0f,						0.0f,
				0.0f,								1.0f / top_plane_offset,			0.0f,						0.0f,
				0.0f,								0.0f,								2.0f / f_minus_n,			0.0f,
				0.0f,								0.0f,								-f_plus_n / f_minus_n,		1.0f
			}
		);
	}

	/* In row-major form. Coordinates are in a left-handed coordinate system (both before & after multiplication). */
	Matrix4x4 PerspectiveProjection( const float near_plane_offset, const float far_plane_offset,
									 const float aspect_ratio, const Radians vertical_field_of_view );

	/* In row-major form. */ constexpr Matrix4x4 Scaling( const float new_scale )
	{
		return Matrix4x4
		(
			{
				new_scale,	0.0f,		0.0f,		0.0f,
				0.0f,		new_scale,	0.0f,		0.0f,
				0.0f,		0.0f,		new_scale,	0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 Scaling( const float new_scale_x, const float new_scale_y, const float new_scale_z )
	{
		return Matrix4x4
		(
			{
				new_scale_x,	0.0f,			0.0f,			0.0f,
				0.0f,			new_scale_y,	0.0f,			0.0f,
				0.0f,			0.0f,			new_scale_z,	0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 ScalingOnX( const float new_scale )
	{
		return Matrix4x4
		(
			{
				new_scale,	0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 ScalingOnY( const float new_scale )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		new_scale,	0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 ScalingOnZ( const float new_scale )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		new_scale,	0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundX( Radians pitch );
	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundY( Radians heading );
	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundZ( Radians bank );
	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundAxis( Radians angle, Vector3 vector );
	/* In row-major form. Counter-clockwise rotation. */ void RotationAroundAxis( Matrix4x4& matrix, Radians angle, Vector3 vector );

	/* In row-major form. */ constexpr Matrix4x4 TranslationOnX( const float delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				delta,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 Translation( const float delta_x, const float delta_y, const float delta_z )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				delta_x,	delta_y,	delta_z,	1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 Translation( const Vector3 delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				delta.X(),	delta.Y(),	delta.Z(),	1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 TranslationOnY( const float delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		delta,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 TranslationOnZ( const float delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		0.0f,		delta,		1.0f
			}
		);
	}
};
