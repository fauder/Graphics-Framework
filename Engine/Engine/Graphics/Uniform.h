#pragma once

// Engine Includes.
#include "Graphics.h"

// std Includes.
#include <string>
#include <unordered_map>

namespace Engine
{
	class Uniform // Acts more like a namespace to conceal its inner classes from all but Shader & Material.
	{
		friend class Shader;
		friend class Material;
		friend class UniformBufferManager;
		friend class Renderer;

		using BindingPoint = unsigned int;

		struct Information
		{
			int location_or_block_index; // Changes meaning depending on context; location if this is a stand-alone uniform, or the index of the block if this resides in a uniform buffer block.
			int size;
			int offset;
			GLenum type;
			bool is_buffer_member;

			std::string editor_name;
		};

		enum class BufferCategory
		{
			Regular,	// Per-material data; Same for every shader of a given material; differs per-material.
			Instance,	// Per-model data; Differs per-model.
			Global,		// Global user-defined data. Same for every shader.
			Intrinsic	// Data known & set by the Renderer such as camera details; view & projection matrices. Same for every shader.
		};

		inline static BufferCategory DetermineBufferCategory( std::string_view buffer_name )
		{
			if( buffer_name.find( "_Instance" ) != std::string_view::npos )
				return BufferCategory::Instance;
			else if( buffer_name.find( "_Global" ) != std::string_view::npos )
				return BufferCategory::Global;
			else if( buffer_name.find( "_Intrinsic" ) != std::string_view::npos )
				return BufferCategory::Intrinsic;
			else
				return BufferCategory::Regular;
		}

		struct BufferMemberInformation_Aggregate
		{
			int offset;
			int size;
		};

		struct BufferMemberInformation_Array
		{
			int offset;
			int stride;
			int element_count;
		};

		struct BufferInformation
		{
		public:
			int binding_point;
			int size;
			int offset;

			BufferCategory category;

			std::unordered_map< std::string, Information*						> members_map;
			std::unordered_map< std::string, BufferMemberInformation_Aggregate	> members_aggregate_map;
			std::unordered_map< std::string, BufferMemberInformation_Array		> members_array_map;

			inline bool IsRegular()		const { return category == BufferCategory::Regular;		}
			inline bool IsInstance()	const { return category == BufferCategory::Instance;	}
			inline bool IsGlobal()		const { return category == BufferCategory::Global;		}
			inline bool IsIntrinsic()	const { return category == BufferCategory::Intrinsic;	}

			inline bool IsGlobalOrIntrinsic() const { return IsGlobal() || IsIntrinsic(); }
			
			inline static const char* CategoryString( const BufferCategory category )
			{
				switch( category )
				{
					case BufferCategory::Intrinsic:			return "Intrinsic";
					case BufferCategory::Global:			return "Global";
					case BufferCategory::Instance:			return "Instance";
					default: /* BufferCategory::Regular */	return "Regular";
				}
			}
		};

		struct ActiveUniformBookKeepingInformation
		{
			std::string name_holder;
			int count;
			int name_max_length;
			std::size_t total_size, total_size_default_block, total_size_uniform_blocks;
			int intrinsic_block_count, global_block_count, instance_block_count, regular_block_count;
		};

		class BindingPointBookKeeping
		{
		public:
			BindingPointBookKeeping( const unsigned int start_offset, const unsigned int maximum_allowed )
				:
				start_offset( start_offset ),
				maximum_allowed( maximum_allowed )
			{}

			inline bool HasRoom() const
			{ 
				return ( in_use + 1 ) < maximum_allowed;
			}

			inline BindingPoint Assign( const std::string& block_name )
			{
				return binding_point_map[ block_name ] = start_offset + in_use++;
			}

			inline int InUseCount() const { return in_use; }

			inline bool IsAssigned( const std::string& block_name ) const
			{
				return binding_point_map.contains( block_name );
			}

			/*inline Uniform::BindingPoint BindingPoint( const std::string& block_name ) const
			{
				return binding_point_map.at( block_name );
			}*/

			inline std::optional< const Uniform::BindingPoint > Find( const std::string& block_name ) const
			{
				if( const auto iterator = binding_point_map.find( block_name );
					iterator != binding_point_map.cend() )
					return iterator->second;

				return std::nullopt;
			}

		private:
			std::unordered_map< std::string, Uniform::BindingPoint > binding_point_map;
			unsigned int in_use = 0;
			unsigned int start_offset;
			unsigned int maximum_allowed;
		};
	};
}