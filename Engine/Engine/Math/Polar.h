#pragma once

// Engine Includes.
#include "Core/Initialization.h"
#include "Math/Angle.hpp"

namespace Engine::Math
{
	/* In classical mathematical notation (r, theta). X is right & Y is forward & Z is up (right-handed space.) */
	class Polar2
	{
	public:
	/* Constructors. */
		Polar2( Initialization::ZeroInitialization )
			:
			r(),
			theta()
		{}

		Polar2( const float r, Engine::Radians theta )
			:
			r( r ),
			theta( theta )
		{}

	/* Getters & Setters. */
		constexpr float R() const { return r; }
		float& R() { return r; }
		constexpr Engine::Radians Theta() const { return theta; }
		Engine::Radians& Theta() { return theta; }

	private:
		float r;
		Engine::Radians theta;
	};

	/* Deviates from classical mathematical notation (r, theta, phi), more in-line with video game conventions/needs.
	 * In this convention, Y is up, X is right & Z is forward (<0,0,+1>) (left-handed).
	 * Uses (r, h, p), h = heading, p = pitch.
	 * Heading = 0 -> facing forward (<0,0,+1>) (assuming Pitch is also zero). Its value increases -> clockwise rotation.
	 * Pitch   = 0 -> horizontal (on XZ plane). It's values increases -> clockwise rotation around X (i.e, facing "more downward", therefore also called the Angle of Declination).
	*/
	class Polar3_Spherical_Game
	{
	public:
	/* Constructors. */
		Polar3_Spherical_Game( Initialization::ZeroInitialization )
			:
			r(),
			heading(),
			pitch()
		{}

		Polar3_Spherical_Game( const float r, const Engine::Radians heading, const Engine::Radians pitch )
			:
			r( r ),
			heading( heading ),
			pitch( pitch )
		{}

	/* Getters & Setters. */
		constexpr float R() const { return r; }
		float& R() { return r; }
		/* Heading = 0 means forward (<0,0,+1>) (assuming Pitch is also zero).
		 * Increasing Heading means clockwise rotation around the Y axis.
		 */
		constexpr Engine::Radians Heading() const { return heading; }
		/* Heading = 0 means forward (<0,0,+1>) (assuming Pitch is also zero).
		 * Increasing Heading means clockwise rotation around the Y axis.
		 */
		Engine::Radians& Heading() { return heading; }
		/* Pitch = 0 means horizontal.
		 * Also called the Angle of Declination, as Z decreases with increasing Pitch.
		 */
		constexpr Engine::Radians Pitch() const { return pitch; }
		/* Pitch = 0 means horizontal.
		 * Also called the Angle of Declination, as Z decreases with increasing Pitch.
		 */
		Engine::Radians& Pitch() { return pitch; }

	private:
		float r;
		Engine::Radians heading;
		Engine::Radians pitch;
	};
}
