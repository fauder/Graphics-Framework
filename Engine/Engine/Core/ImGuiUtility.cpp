// Engine Includes.
#include "ImGuiUtility.h"

namespace Engine::ImGuiUtility
{
	void Table_Header_ManuallySubmit( const int column_index )
	{
		ImGui::TableSetColumnIndex( column_index );
		const char* column_name = ImGui::TableGetColumnName( column_index ); // Retrieve name passed to TableSetupColumn().
		ImGui::PushID( column_index );
		//ImGui::Indent( ( float )font_width );
		ImGui::TableHeader( column_name );
		//ImGui::Unindent( ( float )font_width );
		ImGui::PopID();
	}

	void Table_Header_ManuallySubmit_AppendHelpMarker( const int column_index, const char* help_string )
	{
		/* Due to a Table API limitation, the help marker can NOT be placed AFTER the header name. Therefore it has to be placed before. */

		ImGui::TableSetColumnIndex( column_index );
		const char* column_name = ImGui::TableGetColumnName( column_index ); // Retrieve name passed to TableSetupColumn().
		ImGui::PushID( column_index );
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0, 0 ) );
		ImGuiUtility::HelpMarker( help_string );
		ImGui::PopStyleVar();
		ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x );
		ImGui::TableHeader( column_name );
		ImGui::PopID();
	}

	void HelpMarker( const char* desc, const int wrap )
	{
		ImGui::TextDisabled( "(?)" );
		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			if( wrap )
				ImGui::PushTextWrapPos( ImGui::GetFontSize() * wrap );
			ImGui::TextUnformatted( desc );
			if( wrap )
				ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void SetNextWindowPos( const HorizontalWindowPositioning horizontal_positioning, const VerticalWindowPositioning vertical_positioning, const ImGuiCond condition )
	{
		const auto& io = ImGui::GetIO();
		const auto horizontal_position = horizontal_positioning == HorizontalWindowPositioning::RIGHT
			? io.DisplaySize.x
			: horizontal_positioning == HorizontalWindowPositioning::CENTER
				? io.DisplaySize.x / 2.0f
				: 0.0f;
		const auto vertical_position = vertical_positioning == VerticalWindowPositioning::BOTTOM
			? io.DisplaySize.y
			: vertical_positioning == VerticalWindowPositioning::CENTER
				? io.DisplaySize.y / 2.0f
				: 0.0f;
		const auto horizontal_pivot = ( float )horizontal_positioning / 2.0f; // Map to [+1, 0] range.
		const auto vertical_pivot   = ( float )vertical_positioning   / 2.0f; // Map to [+1, 0] range.

		ImGui::SetNextWindowPos( { horizontal_position, vertical_position }, condition, { horizontal_pivot, vertical_pivot } );
	}
}