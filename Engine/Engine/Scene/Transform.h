#pragma once

// Engine Includes.
#include "Math/Angle.hpp"
#include "Math/Matrix.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vector.hpp"

namespace Engine
{
	class Transform
	{
	public:
		enum class Mask : std::uint8_t
		{
			Scale       = 1,
			Translation = 2,
			Rotation    = 4,

			NoRotation    = Scale		| Translation,
			NoTranslation = Scale		| Rotation,
			NoScale       = Translation | Rotation,

			All = Scale | Translation | Rotation
		};

	public:
		Transform();
		Transform( const Vector3& scale );
		Transform( const Vector3& scale, const Vector3& translation );
		Transform( const Vector3& scale, const Quaternion& rotation, const Vector3& translation );
		~Transform();

	/* Modification: */
		Transform& SetScaling( const Vector3& new_scale );
		Transform& SetScaling( const float new_x_scale, const float new_y_scale, const float new_z_scale );
		Transform& OffsetScaling( const Vector3& delta );
		Transform& OffsetScaling( const float delta_x_scale, const float delta_y_scale, const float delta_z_scale );
		Transform& MultiplyScaling( const Vector3& multiplier );
		Transform& MultiplyScaling( const float multiplier );

		Transform& SetRotation( const Quaternion& new_rotation );

		Transform& SetTranslation( const Vector3& new_translation );
		Transform& SetTranslation( const float new_x, const float new_y, const float new_z );
		Transform& OffsetTranslation( const Vector3& delta );
		Transform& OffsetTranslation( const float delta_x, const float delta_y, const float delta_z );
		Transform& MultiplyTranslation( const Vector3& multiplier );
		Transform& MultiplyTranslation( const float multiplier );

		void LookAt( const Vector3& direction, const Vector3& up = Vector3::Up() );

		/* For cases where a custom matrix needs to be used. */
		inline void SetFinalMatrix( const Matrix4x4& matrix )
		{
			final_matrix = matrix;
			final_matrix_needsUpdate = scaling_needsUpdate = rotation_needsUpdate = translation_needsUpdate = false;
		}

	/* Queries: */
		const Vector3& GetScaling() const;
		const Quaternion& GetRotation() const;
		const Vector3& GetTranslation() const;

		const Matrix4x4& GetFinalMatrix();
		const Matrix4x4 GetInverseOfFinalMatrix();
		/* If the caller knows there's no scaling involved (for example; Transform of a Camera), calling this function is more preferrable. */
		const Matrix4x4 GetInverseOfFinalMatrix_NoScale();

		const Vector3& Right();
		const Vector3& Up();
		const Vector3& Forward();

	/* Dirty Flags: */
		/* This must be reset (via ResetDirtyFlag()) at the beginning of every frame. */
		inline bool IsDirty() const { return is_dirty; }
		inline bool ResetDirtyFlag() { return is_dirty = false; }

	private:
		void UpdateScalingMatrixIfDirty();
		void UpdateRotationPartOfMatrixIfDirty();
		void UpdateTranslationPartOfMatrixIfDirty();
		void UpdateFinalMatrixIfDirty();

		const Matrix4x4& GetScalingMatrix();
		const Matrix4x4& GetRotationAndTranslationMatrix();

	private:
		Vector3 scale;
		Vector3 translation;
		Quaternion rotation;

		Matrix4x4 scaling_matrix;
		Matrix4x4 rotation_and_translation_matrix;

		Matrix4x4 final_matrix;

		/* 4 flags below are for internal (Transform) use. */
		bool scaling_needsUpdate;
		bool rotation_needsUpdate;
		bool translation_needsUpdate;

		bool final_matrix_needsUpdate;

		/* This flag is for external (clients) use. Calls to GetXXXMatrix() APIs may clear the above flags but is_dirty may still be set.
		 * Once set, it remains set until the beginning of the next frame. Therefore, it reliably tells whether this Transform was modified this frame.
		 * It is only reset by the ResetDirtyFlag(), which is ideally called at the beginning of every frame. */
		bool is_dirty;
	};
}
