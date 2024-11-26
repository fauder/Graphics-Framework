#pragma once

// Engine Includes.
#include "../../Core/Macros.h"
#include "../../Core/Utility.hpp"

namespace Engine
{
    constexpr std::string_view TEXTURE_SOURCE_DIRECTORY( __DIR__ );
    constexpr auto TEXTURE_SOURCE_DIRECTORY_AS_ARRAY( Utility::String::StringViewToArray< TEXTURE_SOURCE_DIRECTORY.size() >( TEXTURE_SOURCE_DIRECTORY ) );
    constexpr std::string_view TEXTURE_SOURCE_DIRECTORY_WITH_SEPARATOR( __DIR_WITH_SEPARATOR__ );
    constexpr auto TEXTURE_SOURCE_DIRECTORY_WITH_SEPARATOR_AS_ARRAY( Utility::String::StringViewToArray< TEXTURE_SOURCE_DIRECTORY_WITH_SEPARATOR.size() >( TEXTURE_SOURCE_DIRECTORY_WITH_SEPARATOR ) );
}
