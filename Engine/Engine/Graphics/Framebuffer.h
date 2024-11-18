#pragma once

// Engine Includes.
#include "Color.hpp"
#include "Enums.h"
#include "Texture.h"
#include "Core/BitFlags.hpp"

// std Includes.
#include <string>

namespace Engine
{
	/* Forward Declarations: */
	class Renderer;

	class Framebuffer
	{
		friend class Renderer;

	public:
		using ID = ID< Framebuffer >;

		enum class BindPoint
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

		struct Description
		{
			int width_in_pixels;
			int height_in_pixels;

			Texture::Filtering minification_filter  = Texture::Filtering::Linear;
			Texture::Filtering magnification_filter = Texture::Filtering::Linear;
			Texture::Wrapping  wrap_u               = Texture::Wrapping::ClampToEdge;
			Texture::Wrapping  wrap_v               = Texture::Wrapping::ClampToEdge;
			std::optional< int > multi_sample_count = std::nullopt;
			BindPoint bind_point                    = BindPoint::Both;
			bool is_sRGB                            = false;
			BitFlags< AttachmentType > attachment_bits;

			// 2 bytes of padding.
		};

	public:
		Framebuffer();
		Framebuffer( const std::string& name, Description&& description );

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

		inline int					SampleCount()		const { return sample_count.value(); }
		inline bool					IsMultiSampled()	const { return sample_count.has_value(); }

		inline bool					Is_sRGB()			const { return is_sRGB; }

		inline const std::string&	Name()				const { return name; }

	/* Attachment Queries: */

		inline bool HasColorAttachment()				const { return color_attachment; }
		inline bool HasSeparateDepthAttachment()		const { return depth_attachment && not stencil_attachment; }
		inline bool HasSeparateStencilAttachment()		const { return stencil_attachment && not depth_attachment; }
		inline bool HasCombinedDepthStencilAttachment()	const { return depth_stencil_attachment; }

		inline const Texture& ColorAttachment()			const { return *color_attachment; }
		inline const Texture& DepthStencilAttachment()	const { return *depth_stencil_attachment; }
		inline const Texture& DepthAttachment()			const { return *depth_attachment; }
		inline const Texture& StencilAttachment()		const { return *stencil_attachment; }

	private:

	/* Usage: */
		void Bind() const;
		void Unbind() const;
		void SetName( const std::string& new_name );
		static void Blit( const Framebuffer& source, const Framebuffer& destination );

	/* Clearing: */

		void SetClearColor( const Color3& new_clear_color );
		void SetClearColor( const Color4& new_clear_color );
		void SetClearDepthValue( const float new_clear_depth_value );
		void SetClearStencilValue( const int new_clear_stencil_value );

		void SetClearTargets( const BitFlags< ClearTarget > targets );
		void Clear() const;

		void Destroy();

	/* Clearing: */

		void SetClearColor();
		void SetClearDepthValue();
		void SetClearStencilValue();

	private:
		ID id;
		BindPoint bind_point;

		Vector2I size;

		std::optional< int > sample_count;

		bool is_sRGB;
		// bool padding[ 3 ];

		BitFlags< ClearTarget > clear_targets;
		Color4 clear_color;
		float clear_depth_value;
		int clear_stencil_value;

		std::string name;

		const Texture* color_attachment;
		const Texture* depth_stencil_attachment;
		const Texture* depth_attachment;
		const Texture* stencil_attachment;
	};
}
