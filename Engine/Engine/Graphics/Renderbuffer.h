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

	/* Usage: */
		void Bind() const;
		inline const std::string&	Name()				const { return name; }

	private:

	/* Queries: */
		bool IsValid() const { return size.X() > 0 && size.Y() > 0; } // Use the width & height to implicitly define validness state.

	private:
		ID id;
		//int padding;
		Vector2I size;
		std::string name;
	};

}
