// Engine Includes.
#include "Math/Angle.hpp"
#include "Math/Constants.h"
#include "Math/Math.hpp"
#include "Math/Matrix.h"

namespace Engine::Matrix
{
	/* In row-major form. Coordinates are in a left-handed coordinate system (both before & after multiplication). */
	Matrix4x4 PerspectiveProjection( const float near_plane_offset, const float far_plane_offset, const float aspect_ratio, const Radians vertical_field_of_view )
	{
		const Radians half_v_fov( vertical_field_of_view / 2.0f );
		const float half_tangent = Math::Tan( half_v_fov );

		const float f_plus_n    = far_plane_offset + near_plane_offset;
		const float f_minus_n   = far_plane_offset - near_plane_offset;
		const float f_n			= far_plane_offset * near_plane_offset;

		return Matrix4x4
		(
			{
				1.0f / ( aspect_ratio * half_tangent ),		0.0f,					0.0f,							0.0f,
				0.0f,										1.0f / half_tangent,	0.0f,							0.0f,
				0.0f,										0.0f,					f_plus_n / f_minus_n,			1.0f,
				0.0f,										0.0f,					-2.0f * f_n / f_minus_n,		0.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundX( Radians pitch )
	{
		const auto cosine_term = Math::Cos( pitch );
		const auto   sine_term = Math::Sin( pitch );

		return Matrix4x4
		(
			{
				1.0f,			0.0f,			0.0f,			0.0f,
				0.0f,			cosine_term,	sine_term,		0.0f,
				0.0f,			-sine_term,		cosine_term,	0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundY( Radians heading )
	{
		const auto cosine_term = Math::Cos( heading );
		const auto   sine_term = Math::Sin( heading );

		return Matrix4x4
		(
			{
				cosine_term,	0.0f,			-sine_term,		0.0f,
				0.0f,			1.0f,			0.0f,			0.0f,
				sine_term,		0.0f,			cosine_term,	0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundZ( Radians bank )
	{
		const auto cosine_term = Math::Cos( bank );
		const auto   sine_term = Math::Sin( bank );

		return Matrix4x4
		(
			{
				cosine_term,	sine_term,		0.0f,			0.0f,
				-sine_term,		cosine_term,	0.0f,			0.0f,
				0.0f,			0.0f,			1.0f,			0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundAxis( Radians angle, Vector3 vector )
	{
		vector.Normalize();

		const Radians theta( angle );

		const auto nx                           = vector.X();
		const auto ny                           = vector.Y();
		const auto nz                           = vector.Z();
		const auto cos_theta                    = Math::Cos( theta );
		const auto sin_theta                    = Math::Sin( theta );
		const auto one_minus_cos_theta          = 1.0f - cos_theta;
		const auto nx_times_one_minus_cos_theta = nx * one_minus_cos_theta;
		const auto ny_times_one_minus_cos_theta = ny * one_minus_cos_theta;
		const auto nz_times_one_minus_cos_theta = nz * one_minus_cos_theta;
		const auto nx_sin_theta                 = nx * sin_theta;
		const auto ny_sin_theta                 = ny * sin_theta;
		const auto nz_sin_theta                 = nz * sin_theta;

		return Matrix4x4
		(
			{
				nx * nx_times_one_minus_cos_theta + cos_theta,		ny * nx_times_one_minus_cos_theta + nz_sin_theta,		nz * nx_times_one_minus_cos_theta - ny_sin_theta,	0.0f,
				nx * ny_times_one_minus_cos_theta - nz_sin_theta,	ny * ny_times_one_minus_cos_theta + cos_theta,			nz * ny_times_one_minus_cos_theta + nx_sin_theta,	0.0f,
				nx * nz_times_one_minus_cos_theta + ny_sin_theta,	ny * nz_times_one_minus_cos_theta - nx_sin_theta,		nz * nz_times_one_minus_cos_theta + cos_theta,		0.0f,
				0.0f,												0.0f,													0.0f,												1.0f
			}
		);
	}

	/* In-place modification of the upper-left 3x3 portion. */
	/* In row-major form. Counter-clockwise rotation. */
	void Matrix::RotationAroundAxis( Matrix4x4& matrix, Radians angle, Vector3 vector )
	{
		vector.Normalize();

		const Radians theta( angle );

		const auto nx                           = vector.X();
		const auto ny                           = vector.Y();
		const auto nz                           = vector.Z();
		const auto cos_theta                    = Math::Cos( theta );
		const auto sin_theta                    = Math::Sin( theta );
		const auto one_minus_cos_theta          = 1.0f - cos_theta;
		const auto nx_times_one_minus_cos_theta = nx * one_minus_cos_theta;
		const auto ny_times_one_minus_cos_theta = ny * one_minus_cos_theta;
		const auto nz_times_one_minus_cos_theta = nz * one_minus_cos_theta;
		const auto nx_sin_theta                 = nx * sin_theta;
		const auto ny_sin_theta                 = ny * sin_theta;
		const auto nz_sin_theta                 = nz * sin_theta;

		matrix[ 0 ][ 0 ] = nx * nx_times_one_minus_cos_theta + cos_theta;		matrix[ 0 ][ 1 ] = ny * nx_times_one_minus_cos_theta + nz_sin_theta;	matrix[ 0 ][ 2 ] = nz * nx_times_one_minus_cos_theta - ny_sin_theta;
		matrix[ 1 ][ 0 ] = nx * ny_times_one_minus_cos_theta - nz_sin_theta;	matrix[ 1 ][ 1 ] = ny * ny_times_one_minus_cos_theta + cos_theta;		matrix[ 1 ][ 2 ] = nz * ny_times_one_minus_cos_theta + nx_sin_theta;
		matrix[ 2 ][ 0 ] = nx * nz_times_one_minus_cos_theta + ny_sin_theta;	matrix[ 2 ][ 1 ] = ny * nz_times_one_minus_cos_theta - nx_sin_theta;	matrix[ 2 ][ 2 ] = nz * nz_times_one_minus_cos_theta + cos_theta;	
	}
}