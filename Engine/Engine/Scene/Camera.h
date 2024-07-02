#pragma once

// Engine Includes.
#include "Scene/Transform.h"

namespace Engine
{
	class Camera
	{
	public:
		Camera( Transform* const transform, float aspect_ratio, Degrees vertical_field_of_view, const float near_plane = 0.1f, const float far_plane = 100.0f );

		Camera( const Camera& rhs )              = default;
		Camera( Camera&& donor )                 = default;
		Camera& operator = ( const Camera& rhs ) = default;
		Camera& operator = ( Camera&& donor )    = default;

		inline const Matrix4x4& GetViewMatrix();
		inline const Matrix4x4& GetProjectionMatrix();
		const Matrix4x4& GetViewProjectionMatrix();

		Camera& SetNearPlaneOffset( const float offset );
		Camera& SetFarPlaneOffset( const float offset );
		inline float GetNearPlaneOffset() const { return plane_near; }
		inline float GetFarPlaneOffset()  const { return plane_far; }

		Vector3 Right();
		Vector3 Up();
		Vector3 Forward();

		inline const float GetAspectRatio() const { return aspect_ratio; }
		Camera& SetAspectRatio( const float new_aspect_ratio );

		inline const Degrees& GetFieldOfView() const { return vertical_field_of_view; }
		Camera& SetFieldOfView( const Degrees new_fov );

		Camera& SetLookRotation( const Vector3& look_at, const Vector3& up = Vector3::Up() );

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
		Degrees vertical_field_of_view;

		/* Do not need the flag below, as Transform's data is the only data needed to update the view matrix and it has IsDirty() query already. */
		//bool view_matrix_needs_update;
		bool projection_matrix_needs_update;
		bool view_projection_matrix_needs_update;
	};
}
