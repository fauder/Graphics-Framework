// Engine Includes.
#include "ImGuiDrawer.hpp"
#include "Graphics/ShaderTypeInformation.h"

namespace Engine::ImGuiDrawer
{
	void Draw( const Color3& color )
	{
		Draw( reinterpret_cast< const Vector3& >( color ) );
	}

	void Draw( const Color4& color )
	{
		Draw( reinterpret_cast< const Vector4& >( color ) );
	}

	void Draw( const Shader& shader, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Shader Info.", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			const auto& uniform_map = shader.GetUniformInformations();

			ImGui::SeparatorText( "General Information" );
			if( ImGui::BeginTable( "General", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
			{
				ImGui::TableNextColumn(); ImGui::TextUnformatted( "Shader Name" );
				ImGui::TableNextColumn(); ImGui::Text( R"("%s")", shader.GetName().c_str() );

				ImGui::EndTable();
			}

			ImGui::SeparatorText( "Uniforms" );
			if( ImGui::BeginTable( "Uniforms", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
			{
				ImGui::TableSetupColumn( "Name" );
				ImGui::TableSetupColumn( "Location" );
				ImGui::TableSetupColumn( "Size" );
				ImGui::TableSetupColumn( "Offset" );
				ImGui::TableSetupColumn( "Type" );

				ImGui::TableHeadersRow();

				for( auto& [ name, uniform_info ] : uniform_map )
				{
					ImGui::TableNextColumn(); ImGui::TextUnformatted( name.c_str() );
					ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.location );
					ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.size );
					ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.offset );
					ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_info.IsUserDefinedStruct() ? "struct" : GetNameOfType(uniform_info.type));
				}

				ImGui::EndTable();
			}
		}

		ImGui::End();
	}

	bool Draw( Lighting::LightData& light_data, const char* light_name, const bool hide_position, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			is_modified |= Draw( light_data.ambient,  "Ambient"  );
			is_modified |= Draw( light_data.diffuse,  "Diffuse"  );
			is_modified |= Draw( light_data.specular, "Specular" );
			if( !hide_position )
				is_modified |= Draw( light_data.position, "Position" );

			ImGui::PopID();
		}

		ImGui::End();

		return is_modified;
	}

	void Draw( const Lighting::LightData& light_data, const char* light_name, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			Draw( light_data.ambient,  "Ambient"  );
			Draw( light_data.diffuse,  "Diffuse"  );
			Draw( light_data.specular, "Specular" );
			Draw( light_data.position, "Position" );
		
			ImGui::PopID();
		}

		ImGui::End();
	}

	bool Draw( Lighting::SurfaceData& surface_data, const char* surface_name, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		if( ImGui::Begin( "Surface Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( surface_name );

			ImGui::PushID( surface_name );

			is_modified |= Draw( surface_data.ambient,  "Ambient"  );
			is_modified |= Draw( surface_data.diffuse,  "Diffuse"  );
			is_modified |= Draw( surface_data.specular, "Specular" );
			is_modified |= ImGui::InputFloat( "Shininess", &surface_data.shininess );

			ImGui::PopID();
		}

		ImGui::End();

		return is_modified;
	}

	void Draw( const Lighting::SurfaceData& surface_data, const char* surface_name, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Surface Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( surface_name );
			
			ImGui::PushID( surface_name );

			Draw( surface_data.ambient,  "Ambient" );
			Draw( surface_data.diffuse,  "Diffuse" );
			Draw( surface_data.specular, "Specular" );

			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputFloat( "Shininess", const_cast< float* >( &surface_data.shininess ), 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly );

			ImGui::PopID();
		}

		ImGui::End();
	}
}
