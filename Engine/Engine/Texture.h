#pragma once

namespace Engine
{
	class Texture
	{
	public:
		Texture();
		Texture( const char* file_path );
		~Texture();

		void Use();

		bool FromFile( const char* file_path );

	private:
		unsigned int id;
		int width, height;
	};
};
