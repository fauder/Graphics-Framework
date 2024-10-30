#pragma once

// Engine Includes.
#include "Texture.h"
#include "Renderbuffer.h"
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
			Invalid  = 0,
			Both     = GL_FRAMEBUFFER,
			DrawOnly = GL_DRAW_FRAMEBUFFER,
			ReadOnly = GL_READ_FRAMEBUFFER
		};

	public:
		Framebuffer();
		Framebuffer( const std::string_view name,
					 const int width, const int height,
					 const std::variant< const Texture*, const Renderbuffer* > color_attachment,
					 const std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_and_stencil_attachment = std::monostate{},
					 const std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_attachment             = std::monostate{},
					 const std::variant< std::monostate, const Texture*, const Renderbuffer* > stencil_attachment           = std::monostate{},
					 const Target usage = Target::Both );

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

		inline bool					SampleCount()		const { return sample_count; }
		inline bool					IsMultiSampled()	const { return sample_count; }

		inline const std::string&	Name()				const { return name; }

		inline bool HasSeparateDepthAttachment()		const { return not std::holds_alternative< std::monostate >( depth_attachment ); }
		inline bool HasSeparateStencilAttachment()		const { return not std::holds_alternative< std::monostate >( stencil_attachment ); }
		inline bool HasCombinedDepthStencilAttachment()	const { return not std::holds_alternative< std::monostate >( depth_and_stencil_attachment ); }

		inline bool HasColorTexture()					const { return std::holds_alternative< const Texture*		>( color_attachment ); }
		inline bool HasColorRenderbuffer()				const { return std::holds_alternative< const Renderbuffer*  >( color_attachment ); }

		inline bool HasDepthTexture()					const { return std::holds_alternative< const Texture*		>( depth_attachment ); }
		inline bool HasDepthRenderbuffer()				const { return std::holds_alternative< const Renderbuffer*  >( depth_attachment ); }

		inline bool HasStencilTexture()					const { return std::holds_alternative< const Texture*		>( stencil_attachment ); }
		inline bool HasStencilRenderbuffer()			const { return std::holds_alternative< const Renderbuffer*  >( stencil_attachment ); }

		inline bool HasDepthStencilTexture()			const { return std::holds_alternative< const Texture*		>( depth_and_stencil_attachment ); }
		inline bool HasDepthStencilRenderbuffer()		const { return std::holds_alternative< const Renderbuffer*  >( depth_and_stencil_attachment ); }

	/* Usage: */
		void Bind() const;
		void Unbind() const;
		void SetName( const std::string& new_name );
		static void Blit( const Framebuffer& source, const Framebuffer& destination );

	private:

		void Create();

	private:
		ID id;
		Vector2I size;
		int sample_count;
		//int padding;
		Target target;

		std::string name;

		std::variant< const Texture*, const Renderbuffer* > color_attachment;

		std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_and_stencil_attachment;
		std::variant< std::monostate, const Texture*, const Renderbuffer* > depth_attachment;
		std::variant< std::monostate, const Texture*, const Renderbuffer* > stencil_attachment;
	};
}
