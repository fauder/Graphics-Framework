// Engine Includes.
#include "Camera.h"
#include "Math/Matrix.h"

namespace Engine
{
	Camera::Camera( Transform* const transform, float aspect_ratio, const float near_plane, const float far_plane, Degrees field_of_view )
		:
		transform( transform ),
		plane_near( near_plane ),
		plane_far( far_plane ),
		aspect_ratio( aspect_ratio ),
		field_of_view( field_of_view ),
		projection_matrix_needs_update( true ),
		view_projection_matrix_needs_update( true )
	{}

	const Matrix4x4& Camera::GetViewMatrix()
	{
		if( transform->IsDirty() )
			view_matrix = transform->GetInverseOfFinalMatrix_NoScale();

		return view_matrix;
	}

	const Matrix4x4& Camera::GetProjectionMatrix()
	{
		if( projection_matrix_needs_update )
		{
			projection_matrix = Matrix::PerspectiveProjection( plane_near, plane_far, aspect_ratio, field_of_view );
			projection_matrix_needs_update = false;
		}

		return projection_matrix;
	}
	
	const Matrix4x4& Camera::GetViewProjectionMatrix()
	{
		if( view_projection_matrix_needs_update || transform->IsDirty() )
		{
			view_projection_matrix = GetViewMatrix() * GetProjectionMatrix();
			view_projection_matrix_needs_update = false;
		}

		return view_projection_matrix;
	}
	
	Camera& Camera::SetNearPlaneOffset( const float offset )
	{
		plane_near = offset;
		SetProjectionMatrixDirty();
		return *this;
	}

	Camera& Camera::SetFarPlaneOffset( const float offset )
	{
		plane_far = offset;
		SetProjectionMatrixDirty();
		return *this;
	}

	Vector3 Camera::Right()
	{
		GetViewMatrix();
		return view_matrix.GetColumn< 3 >( 0 );
	}

	Vector3 Camera::Up()
	{
		GetViewMatrix();
		return view_matrix.GetColumn< 3 >( 1 );
	}

	Vector3 Camera::Forward()
	{
		GetViewMatrix();
		return view_matrix.GetColumn< 3 >( 2 ); 
	}

	Camera& Camera::SetAspectRatio( const float new_aspect_ratio )
	{
		SetProjectionMatrixDirty();
		aspect_ratio = new_aspect_ratio;
		return *this;
	}

	Camera& Camera::SetFieldOfView( const Degrees new_fov )
	{
		SetProjectionMatrixDirty();
		field_of_view = new_fov;
		return *this;
	}

	Camera& Camera::SetLookRotation( const Vector3& look_at, const Vector3& up )
	{
		transform->LookAt( look_at, up );
		return *this;
	}

	Vector3 Camera::ConvertFromScreenSpaceToViewSpace( const Engine::Vector2 screen_space_coordinate, const Engine::Vector2I screen_dimensions )
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
				plane_near
		};
	}

	void Camera::SetProjectionMatrixDirty()
	{
		projection_matrix_needs_update = true;
		SetViewProjectionMatrixDirty();
	}

	void Camera::SetViewProjectionMatrixDirty()
	{
		view_projection_matrix_needs_update = true;
	}
}