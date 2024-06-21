// Engine Includes.
#include "ImGuiUtility.h"

// Vendor Includes.
#include "ImGui/imgui.h"

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
}