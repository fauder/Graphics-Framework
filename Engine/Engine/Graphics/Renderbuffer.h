#pragma once

// Engine Includes.
#include "ID.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <string>

namespace Engine
{
	class Renderbuffer
	{
	public:
		using ID = ID< Renderbuffer >;

	public:
		Renderbuffer();
		Renderbuffer( const std::string_view name, const int width, const int height );
		Renderbuffer( const int sample_count, const std::string_view multi_sampled_renderbuffer_name, const int width, const int height );

		/* Prevent copying for now: */
		Renderbuffer( const Renderbuffer& ) = delete;
		Renderbuffer& operator =( const Renderbuffer& ) = delete;

		/* Allow moving: */
		Renderbuffer( Renderbuffer&& );
		Renderbuffer& operator =( Renderbuffer&& );

		~Renderbuffer();

	/* Queries: */
		inline const ID				Id()				const { return id; }
		inline const Vector2I&		Size()				const { return size; }
		inline int					Width()				const { return size.X(); }
		inline int					Height()			const { return size.Y(); }
		void SetName( const std::string& new_name );
		inline const std::string&	Name()				const { return name; }
		inline bool					IsMultiSampled()	const { return sample_count; }
		inline int					SampleCount()		const { return sample_count; }

	private:

		/* Usage: */
		void Bind() const;
		void Unbind() const;

	/* Queries: */
		bool IsValid() const { return size.X() > 0 && size.Y() > 0; } // Use the width & height to implicitly define validness state.

	private:
		ID id;
		Vector2I size;
		int sample_count;
		std::string name;
	};

}
