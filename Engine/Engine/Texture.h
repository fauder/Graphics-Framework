#pragma once

namespace Engine
{
	class Texture
	{
	public:
		Texture();
		Texture( const char* file_path );
		~Texture();

		void Use() const;

		bool FromFile( const char* file_path );

		static void INITIALIZE();

	private:
		unsigned int id;
		int width, height;
	};
};
