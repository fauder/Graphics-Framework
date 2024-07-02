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

		inline const Radians GetHeading()	const { return orientation_spherical.Heading(); }
		inline const Radians GetPitch()		const { return orientation_spherical.Pitch(); }
		CameraController_Flight& SetHeading( const Radians new_heading );
		CameraController_Flight& SetPitch( const Radians new_pitch );

	private:
		void RecalculateRotationFromSphericalCoordinates();

	public:
		float move_speed;

	private:
		Math::Polar3_Spherical_Game orientation_spherical; // Spherical coordinates are more natural & convenient to work with for this camera type.
		Camera* camera;
	};
}

