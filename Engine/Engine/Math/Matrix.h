#pragma once

// Engine Includes.
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

namespace Engine::Matrix
{
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
