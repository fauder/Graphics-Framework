// Engine Includes.
#include "Camera.h"

namespace Engine
{
	namespace Camera
	{
		Engine::Vector3 ConvertFromScreenSpaceToViewSpace( const Engine::Vector2 screen_space_coordinate, const Engine::Vector2I screen_dimensions,
														   const float aspect_ratio, const float near_plane )
		{
			/* Viewport transform converts the NDC coordinates at the near plane (-1 to + 1 both horizontally & vertically)
			 * to viewport coordinates (for example 0 to 1600 from left to right and 0 to 900 from top to bottom). So it is a scale & offset:
			 * 
			 * Screen_Space X = ( NDC_X + 1) * (Screen_Space_Width  / 2)		->	First term is the offset & the second is the scale.
			 * Screen_Space Y = (-NDC_Y + 1) * (Screen_Space_Height / 2)		->	First term is the offset & the second is the scale.
			 * 
			 * So to convert back from viewport coordinates to NDC (which has the origin in the middle of the near plane both vertically & horizontally):
			 *
			 * NDC_X = ( Screen_Space_X * 2 / Screen_Space_Width ) - 1
			 * NDC_Y = (-Screen_Space_Y * 2 / Screen_Space_Height) + 1
			 * 
			 * Finally, to convert from NDC to View space, we consider that the near plane in view space has an aspect ratio & the height 1, therefore having the width = aspect ratio.
			 * Therefore we simply multiply the x component with the aspect ratio:
			 * 
			 * View_X (@ near plane) = [ ( Screen_Space_X * 2 / Screen_Space_Width ) - 1 ] * aspect_ratio
			 * View_Y (@ near plane) = [ (-Screen_Space_Y * 2 / Screen_Space_Height) + 1 ]					(same as NDC_Y)
			 */

			return
			{ 
				( (  screen_space_coordinate.X() * 2 / screen_dimensions.X() ) - 1 ) * aspect_ratio,
				  ( -screen_space_coordinate.Y() * 2 / screen_dimensions.Y() ) + 1,
					near_plane
			};
		}
	}
}