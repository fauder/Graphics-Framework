#pragma once

// Engine Includes.
#include "Scene/Transform.h"

namespace Engine
{
	class Camera
	{
	public:
		Camera( Transform* const transform, float aspect_ratio, Radians vertical_field_of_view, const float near_plane = 0.1f, const float far_plane = 100.0f );

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Camera );

	/* Matrix Getters: */

		inline const Matrix4x4& GetViewMatrix();
		inline const Matrix4x4& GetProjectionMatrix();
		const Matrix4x4& GetViewProjectionMatrix();

	/* View related: */

		const Vector3& Position() const { return transform->GetTranslation(); }

		Vector3 Right();
		Vector3 Up();
		Vector3 Forward();

		Camera& SetLookRotation( const Vector3& look_at, const Vector3& up = Vector3::Up() );

	/* Projection Related related: */

		Camera& SetNearPlaneOffset( const float offset );
		Camera& SetFarPlaneOffset( const float offset );
		inline float GetNearPlaneOffset() const { ASSERT_DEBUG_ONLY( not projection_matrix_is_overridden ); return plane_near; }
		inline float GetFarPlaneOffset()  const { ASSERT_DEBUG_ONLY( not projection_matrix_is_overridden ); return plane_far; }

		inline const float GetAspectRatio() const { ASSERT_DEBUG_ONLY( not projection_matrix_is_overridden ); return aspect_ratio; }
		Camera& SetAspectRatio( const float new_aspect_ratio );

		inline const Radians& GetVerticalFieldOfView() const { ASSERT_DEBUG_ONLY( not projection_matrix_is_overridden ); return vertical_field_of_view; }
		Camera& SetVerticalFieldOfView( const Radians new_fov );

	/* Other:*/

		Vector3 ConvertFromScreenSpaceToViewSpace( const Engine::Vector2 screen_space_coordinate, const Engine::Vector2I screen_dimensions );

	private:
		void SetProjectionMatrixDirty();
		void SetViewProjectionMatrixDirty();

	private:
		Matrix4x4 view_matrix;
		Matrix4x4 projection_matrix;
		Matrix4x4 view_projection_matrix;

		Transform* transform;

		float plane_near, plane_far;
		float aspect_ratio;
		Radians vertical_field_of_view;

		/* Do not need the flag below, as Transform's data is the only data needed to update the view matrix and it has IsDirty() query already. */
		//bool view_matrix_needs_update;
		bool projection_matrix_needs_update;
		bool view_projection_matrix_needs_update;
		//bool padding[ 6 ];
	};
}
