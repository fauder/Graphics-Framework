#pragma once

// std Includes.
#include <array>

// Vendor includes.
#include "ImGui/imgui.h"

namespace Engine::ImGuiUtility
{
	enum class HorizontalWindowPositioning { LEFT, CENTER,  RIGHT };
	enum class VerticalWindowPositioning   { TOP,  CENTER, BOTTOM };

	void Table_Header_ManuallySubmit( const int column_index );
	template< int array_size >
	void Table_Header_ManuallySubmit( const std::array< int, array_size > column_indices )
	{
		for( std::size_t i = 0; i < array_size; i++ )
			Table_Header_ManuallySubmit( column_indices[ i ] );
	}
	void Table_Header_ManuallySubmit_AppendHelpMarker( const int column_index, const char* help_string );

	/* Helper to display a little (?) mark which shows a tooltip when hovered. */
	void HelpMarker( const char* desc, const int wrap = 35.0f );
	
	void SetNextWindowPos( const HorizontalWindowPositioning horizontal_positioning, const VerticalWindowPositioning vertical_positioning,
						   const ImGuiCond condition = ImGuiCond_Always );
}
