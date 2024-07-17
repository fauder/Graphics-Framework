#pragma once

namespace Engine
{
	class Texture
	{
	public:
		using ID = unsigned int;

	public:
		Texture();
		Texture( const std::string_view& name );
		Texture( const std::string_view& name, const char* file_path, const int format );
		~Texture();

	/* Static API: */
		static void INITIALIZE();

	/* Creation: */
		bool FromFile( const char* file_path, const int format );

	/* Queries: */
		inline const std::string& Name() const { return name; }

	/* Usage: */
		void Activate( const int slot );

	private:
	/* Usage: */
		void Bind() const;

	private:
		ID id;
		int width, height;
		std::string name;
	};
};
