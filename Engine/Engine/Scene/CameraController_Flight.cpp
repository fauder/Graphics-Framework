// Engine Includes.
#include "CameraController_Flight.h"
#include "Math/Math.hpp"

namespace Engine
{
	using namespace Math::Literals;

	CameraController_Flight::CameraController_Flight( Camera* camera, const float move_speed )
		:
		orientation_spherical( 1.0f, 0_rad, 0_rad ), // Unit-sphere used as these spherical coordinates are used as an orientation & the look-at vector is computed from it.
		move_speed( move_speed ),
		camera( camera )
	{
	}

	CameraController_Flight& CameraController_Flight::SetHeading( const Radians new_heading )
	{
		orientation_spherical.Heading() = new_heading;
		RecalculateRotationFromSphericalCoordinates();
		return *this;
	}

	CameraController_Flight& CameraController_Flight::SetPitch( const Radians new_pitch )
	{
		orientation_spherical.Pitch() = new_pitch;
		RecalculateRotationFromSphericalCoordinates();
		return *this;
	}

	void CameraController_Flight::RecalculateRotationFromSphericalCoordinates()
	{
		camera->SetLookRotation( Math::ToVector3( orientation_spherical ) );
	}
}