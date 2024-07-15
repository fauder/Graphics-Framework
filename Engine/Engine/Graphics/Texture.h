#pragma once

namespace Engine
{
	class Texture
	{
	public:
		using ID = unsigned int;

	public:
		Texture();
		Texture( const char* file_path, const int format );
		~Texture();

		void ActivateAndUse( const int slot );

		bool FromFile( const char* file_path, const int format );

		static void INITIALIZE();

	private:
		void Bind() const;

	private:
		ID id;
		int width, height;
	};
};
