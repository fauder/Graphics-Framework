#pragma once

// Engine Includes.
#include "Camera.h"
#include "Math/Polar.h"

namespace Engine
{
	class CameraController_Flight
	{
	public:
		CameraController_Flight( Camera* camera, const float move_speed );

		CameraController_Flight( const CameraController_Flight& rhs )              = default;
		CameraController_Flight( CameraController_Flight&& donor )                 = default;
		CameraController_Flight& operator = ( const CameraController_Flight& rhs ) = default;
		CameraController_Flight& operator = ( CameraController_Flight&& donor )    = default;

		CameraController_Flight& Invert();

		inline const Radians GetHeading()	const { return orientation_spherical.Heading(); }
		inline const Radians GetPitch()		const { return orientation_spherical.Pitch(); }

		CameraController_Flight& SetHeading( const Radians new_heading );
		CameraController_Flight& SetHeading( const Radians new_heading, const Radians clamp_minimum, const Radians clamp_maximum );
		CameraController_Flight& OffsetHeading( const Radians delta );
		CameraController_Flight& OffsetHeading( const Radians delta, const Radians clamp_minimum, const Radians clamp_maximum );
		CameraController_Flight& InvertHeading();
		CameraController_Flight& SetPitch( const Radians new_pitch );
		CameraController_Flight& SetPitch( const Radians new_pitch, const Radians clamp_minimum, const Radians clamp_maximum );
		CameraController_Flight& OffsetPitch( const Radians delta );
		CameraController_Flight& OffsetPitch( const Radians delta, const Radians clamp_minimum, const Radians clamp_maximum );
		CameraController_Flight& InvertPitch();

		void ResetToTransform();

	private:
		void RecalculateRotationFromSphericalCoordinates();

	public:
		float move_speed;

	private:
		Math::Polar3_Spherical_Game orientation_spherical; // Spherical coordinates are more natural & convenient to work with for this camera type.
		//int padding;
		Camera* camera;
	};
}

