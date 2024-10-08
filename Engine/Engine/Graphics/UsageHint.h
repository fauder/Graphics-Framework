#pragma once

// std Includes.
#include <string>

namespace Engine
{
	enum class UsageHint
	{
		Unassigned, // This is necessary to be able to have a None value & to be able to check whether there is a mismatch between multiple stages of the shader for a given parameter.

		None,
		AsColor3,
		AsColor4,
		AsArray,
		AsSlider_Normalized,
		AsSlider_Normalized_Percentage,
	};

	static UsageHint UsageHint_StringToEnum( const std::string& string )
	{
		if( string == "color3" )
			return UsageHint::AsColor3;
		else if( string == "color4" )
			return UsageHint::AsColor4;
		else if( string.compare( 0, 5, "array", 5 ) == 0 )
			return UsageHint::AsArray;
		else if( string == "normalized" )
			return UsageHint::AsSlider_Normalized;
		else if( string == "normalized_percentage" )
			return UsageHint::AsSlider_Normalized_Percentage;
		else
			return UsageHint::None;
	}
}
