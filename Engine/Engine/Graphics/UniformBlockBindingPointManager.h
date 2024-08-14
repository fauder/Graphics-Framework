#pragma once

// Engine Includes.
#include "UniformBuffer.h"
#include "Uniform.h"

// std Includes.
#include <optional>

namespace Engine
{
	/* Forward declaration: */
	class Shader;

	// Singleton.
	class UniformBlockBindingPointManager
	{
	public:
		UniformBlockBindingPointManager( UniformBlockBindingPointManager const& ) = delete;
		void operator = ( UniformBlockBindingPointManager const& ) = delete;

	/* Block registering API: */
		static Uniform::BindingPoint RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BufferInformation& uniform_buffer_info );

	/* Buffer-Block connecting API: */
		static void ConnectBufferToBlock( const UniformBuffer& uniform_buffer, const std::string& block_name, const Uniform::BufferCategory category = Uniform::BufferCategory::Regular );

	private:
	/* Private default constructor. */
		UniformBlockBindingPointManager();

	/* Singleton related: */
		static UniformBlockBindingPointManager& Instance()
		{
			static UniformBlockBindingPointManager instance;
			return instance;
		}

	/* Block registering API: */
		static Uniform::BindingPoint RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BindingPointBookKeeping& binding_point_book_keeping );

	/* Buffer-Block connecting API: */
		static void BindBufferToBindingPoint( const UniformBuffer&, const Uniform::BindingPoint );
		static void BindBufferToBindingPoint_Partial( const UniformBuffer&, const Uniform::BindingPoint,
													  const unsigned int offset, const unsigned int size );

	/* Misc.: */
		static unsigned int QueryMaximumUniformBufferBindingCount();

	private:
		unsigned int binding_point_max_count;

		Uniform::BindingPointBookKeeping binding_point_book_keeping_regular;
		Uniform::BindingPointBookKeeping binding_point_book_keeping_instance;
		Uniform::BindingPointBookKeeping binding_point_book_keeping_global;
		Uniform::BindingPointBookKeeping binding_point_book_keeping_intrinsic;
	};
}