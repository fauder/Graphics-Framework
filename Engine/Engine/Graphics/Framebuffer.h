#pragma once

// Engine Includes.
#include "Texture.h"
#include "Renderbuffer.h"
#include "Core/BitFlags.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <string>
#include <variant>

namespace Engine
{
	class Framebuffer
	{
	public:
		using ID = ID< Framebuffer >;

		enum class Target
		{
			Invalid = 0,
			Both    = GL_FRAMEBUFFER,
			Draw    = GL_DRAW_FRAMEBUFFER,
			Read    = GL_READ_FRAMEBUFFER
		};

		enum class AttachmentType : std::uint8_t
		{
			Color                = 1,
			Depth                = 2,
			Stencil              = 4,
			DepthStencilCombined = 8,

			Color_Depth_Stencil        = Color | Depth | Stencil,
			Color_DepthStencilCombined = Color | DepthStencilCombined
		};

	public:
		Framebuffer();
		Framebuffer( const std::string& name,
					 const int width_in_pixels, const int height_in_pixels,
					 const BitFlags< AttachmentType > attachment_bits,
					 const std::optional< int > multi_sample_count = std::nullopt,
					 const Target target = Target::Both );

		DELETE_COPY_CONSTRUCTORS( Framebuffer );

		Framebuffer( Framebuffer&& );
		Framebuffer& operator =( Framebuffer&& );

		~Framebuffer();

	/* Queries: */
		bool IsValid() const { return id.IsValid(); }

		inline const ID				Id()				const { return id;	 }
		
		inline const Vector2I&		Size()				const { return size; }
		inline int					Width()				const { return size.X(); }
		inline int					Height()			const { return size.Y(); }

		inline bool					SampleCount()		const { return sample_count.value(); }
		inline bool					IsMultiSampled()	const { return sample_count.has_value(); }

		inline const std::string&	Name()				const { return name; }

	/* Attachment Queries: */

		inline bool HasColorAttachment()				const { return color_attachment.has_value(); }
		inline bool HasSeparateDepthAttachment()		const { return depth_attachment.has_value() && not stencil_attachment.has_value(); }
		inline bool HasSeparateStencilAttachment()		const { return stencil_attachment.has_value() && not depth_attachment.has_value(); }
		inline bool HasCombinedDepthStencilAttachment()	const { return depth_stencil_attachment.has_value(); }

		inline const Texture& ColorAttachment()			const { return *color_attachment.value(); }
		inline const Texture& DepthStencilAttachment()	const { return *depth_stencil_attachment.value(); }
		inline const Texture& DepthAttachment()			const { return *depth_attachment.value(); }
		inline const Texture& StencilAttachment()		const { return *stencil_attachment.value(); }

	/* Usage: */
		void Bind() const;
		void Unbind() const;
		void SetName( const std::string& new_name );
		static void Blit( const Framebuffer& source, const Framebuffer& destination );

	private:

		void Destroy();

	private:
		ID id;
		Vector2I size;
		std::optional< int > sample_count;
		Target target;

		std::string name;

		std::optional< const Texture* > color_attachment;
		std::optional< const Texture* > depth_stencil_attachment;
		std::optional< const Texture* > depth_attachment;
		std::optional< const Texture* > stencil_attachment;
	};
}
