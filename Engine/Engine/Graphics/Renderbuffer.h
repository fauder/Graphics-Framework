#pragma once

// Engine Includes.
#include "ID.hpp"

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
		inline const ID				Id()	const { return id; }
		inline const std::string& Name()	const { return name; }
		void SetName( const std::string& new_name );

	/* Usage: */
		void Bind() const;

	private:

	/* Queries: */
		bool IsValid() const { return width > 0 && height > 0; } // Use the width & height to implicitly define validness state.

	private:
		ID id;
		int width, height;
		std::string name;
	};

}
