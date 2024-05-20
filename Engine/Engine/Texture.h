#pragma once

namespace Engine
{
	class Texture
	{
	public:
		Texture();
		Texture( const char* file_path, const int format );
		~Texture();

		void ActivateAndUse( const int slot );

		bool FromFile( const char* file_path, const int format );

		static void INITIALIZE();

	private:
		void Use() const;

	private:
		unsigned int id;
		int width, height;
	};
};
