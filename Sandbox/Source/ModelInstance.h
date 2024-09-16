#pragma once

// Engine Includes.
#include <Engine/DefineMathTypes.h>
#include <Engine/Graphics/Drawable.h>
#include <Engine/Graphics/MaterialData/MaterialData.h>
#include <Engine/Graphics/Model.h>
#include <Engine/Math/Matrix.h>

// std Includes.
#include <vector>

class ModelInstance
{
	DEFINE_MATH_TYPES()

public:
	ModelInstance();
	ModelInstance( const Engine::Model* model, Engine::Shader* const shader,
				   const Vector3 scale, const Quaternion& rotation, const Vector3& translation,
				   const Engine::Texture* diffuse_texture = nullptr, const Engine::Texture* specular_texture = nullptr,
				   const Vector4 texture_scale_and_offset = Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	/* Permit moving, but not copying for now. */
	ModelInstance( ModelInstance&& donor )				= default;
	ModelInstance& operator=( ModelInstance && donor )	= default;

	~ModelInstance();

	void SetMaterialData( Engine::Shader* const shader, 
						  const Engine::Texture* diffuse_texture = nullptr,
						  const Engine::Texture* specular_texture = nullptr,
						  const Vector4 texture_scale_and_offset = Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

/* Queries: */
	inline const std::vector< Engine::Drawable	>& Drawables() const { return node_drawable_array; }
	inline const std::vector< Engine::Material	>& Materials() const { return node_material_array; }

private:
	const Engine::Model* model;
	std::vector< Engine::Drawable > node_drawable_array;
	std::vector< Engine::Material > node_material_array;
	std::vector< Engine::Transform > node_transform_array;
	std::vector< Engine::MaterialData::PhongMaterialData > phong_material_data_array;
};
