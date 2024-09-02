#pragma once

// std Includes.
#include <cstddef> // std::byte.

#define BUFFER_OFFSET( idx ) ( static_cast< std::byte* >( 0 ) + ( idx ) )

#define ASSET_LOADER_CLASS_DECLARATION( class_name ) \
class Loader\
{\
public:\
	Loader( const Loader& ) = delete;\
	Loader& operator =( const Loader& ) = delete;\
	static std::optional< class_name > FromFile( const std::string_view name, const std::string& file_path, const ImportSettings& import_settings );\
private:\
	Loader()\
	{}\
	static Loader& Instance()\
	{\
		static Loader instance;\
		return instance;\
	}\
};