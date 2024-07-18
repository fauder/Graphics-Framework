#pragma once

// Engine Includes.
#include "Drawable.h"
#include "Scene/Camera.h"

// std Includes.
#include <unordered_map>
#include <vector>

namespace Engine
{
	class Renderer
	{
	public:
		enum class ClearTarget
		{
			DepthBuffer   = GL_DEPTH_BUFFER_BIT,
			StencilBuffer = GL_STENCIL_BUFFER_BIT,
			ColorBuffer   = GL_COLOR_BUFFER_BIT,
		};

	public:
		Renderer();
		~Renderer();

	/* Main: */
		void Render( Camera& camera );

	/* Drawables: */
		void AddDrawable( Drawable* drawable_to_add );
		// TODO: Switch to unsigned map of "Component" UUIDs when Component class is implemented.
		// For now, stick to removing elements from a vector, which is sub-par performance but should be OK for the time being.
		void RemoveDrawable( const Drawable* drawable_to_remove );

	/* Shaders: */
		// TODO: Provide Global setting/getting functions.

	/* Clearing: */
		void SetClearColor( const Color3& new_clear_color );
		void SetClearColor( const Color4& new_clear_color );

		void SetClearTarget( const ClearTarget& target );
		void SetClearTargets( const ClearTarget& target, std::same_as< ClearTarget > auto&& ... other_targets )
		{
			clear_target = ClearTarget( ( int )target | ( ( int )other_targets | ... ) );
		}

	private:
	/* Main: */
		void Render( const Mesh& mesh );
		void Render_Indexed( const Mesh& mesh );
		void Render_NonIndexed( const Mesh& mesh );

	/* Shaders: */
		// TODO: Provide Intrinsic setting/getting functions.

		void RegisterShader( const Shader& shader );
		void UnregisterShader( const Shader& shader );

	/* Clearing: */
		void SetClearColor();
		void Clear() const;

	private:
		std::vector< Drawable* > drawable_list;

		std::unordered_map< Shader::ID, const Shader* > shaders_registered;

		std::unordered_map< Shader::ID,  Shader* > shaders_in_flight;
		std::unordered_map< std::string, Material* > materials_in_flight; // TODO: Generate an ID for Materials (who will generate it?) and use that ID as the key here.

		Color4 clear_color;
		ClearTarget clear_target;

		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_intrinsic;
		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_global;

		//std::unordered_map< UniformBuffer::ID, 

		// TODO: Probably going to need to keep a blob similar to Material's here for intrinsic & global uniforms.
	};
}