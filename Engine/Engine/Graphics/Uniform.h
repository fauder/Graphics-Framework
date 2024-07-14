#pragma once

// Engine Includes.
#include "Graphics.h"

// std Includes.
#include <map>
#include <string>

namespace Engine
{
	/* Forward declaration: */
	class Material;

	class Uniform // Acts more like a namespace to conceal its inner classes from all but Shader & Material.
	{
		friend class Shader;
		friend class Material;

		class Information_Old
		{
		private:
			friend class Shader; // HasOriginalOrdersDetermined() should ideally only be visible to Shader.

		public:
			inline bool IsUserDefinedStruct() const { return !members.empty(); }

		public:
			int location = -1;
			int original_order_in_struct = -1;	// This will be used to address the correct memory inside the memory blob of the CPU-side uniform struct when uploading uniforms to shader.
			int size;
			int offset;							// Material uses this to address uniforms in its blob of memory.
			int original_offset = -1;			// The offset based on the original (CPU-side) in-struct order. Used for the cpu-side memory-addressing inside the struct's blob.
			GLenum type;

			std::map< std::string, Information_Old* > members;

		private:
			inline bool HasOriginalOrdersDetermined() const
			{
				for( auto& [ member_uniform_name, member_uniform_info ] : members )
					if( member_uniform_info->original_order_in_struct == -1 )
						return false;

				return true;
			}
		};

		struct Information
		{
			int location_or_block_index; // Changes meaning depending on context; location if this is a stand-alone uniform, or the index of the block if this resides in a uniform buffer block.
			int size;
			int offset;
			GLenum type;
			bool is_buffer_member;
		};

		struct BufferInformation
		{
		public:
			int binding_point;
			int size;
			int offset;

			std::map< std::string, Information* > members_map;
		};

		struct ActiveUniformBookKeepingInformation
		{
			std::string name_holder;
			int count;
			int name_max_length;
			std::size_t total_size;
		};
	};
}