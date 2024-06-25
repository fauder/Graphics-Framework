// Engine Includes.
#include "ImGuiDrawer.hpp"
#include "ImGuiUtility.h"
#include "Graphics/ShaderTypeInformation.h"

namespace Engine::ImGuiDrawer
{
	bool Draw( Color3& color, const char* name )
	{
		return ImGui::ColorEdit3( name, color.Data() );
	}

	void Draw( const Color3& color, const char* name )
	{
		/* Since the no inputs & no picker flags are passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::ColorEdit3( name, const_cast< float* >( color.Data() ), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker );
	}

	bool Draw( Color4& color, const char* name )
	{
		return ImGui::ColorEdit4( name, color.Data() );
	}

	void Draw( const Color4& color, const char* name )
	{
		/* Since the no inputs & no picker flags are passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::ColorEdit4( name, const_cast< float* >( color.Data() ), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker );
	}

	void Draw( const Shader& shader, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Shaders", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			const auto& uniform_map = shader.GetUniformInformations();

			if( ImGui::TreeNodeEx( shader.GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
			{
				ImGui::SeparatorText( "Uniforms" );
				if( ImGui::BeginTable( "Uniforms", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
				{
					ImGui::TableSetupColumn( "Name" );
					ImGui::TableSetupColumn( "Location" );
					ImGui::TableSetupColumn( "Size" );
					ImGui::TableSetupColumn( "Offset" );
					ImGui::TableSetupColumn( "Type" );

					ImGui::TableNextRow( ImGuiTableRowFlags_Headers ); // Indicates that the header row will be modified.
					ImGuiUtility::Table_Header_ManuallySubmit( std::array< int, 3 >{ 0, 2, 4 } );
					ImGuiUtility::Table_Header_ManuallySubmit_AppendHelpMarker( 1, "For uniform struct members, value in parenthesis shows the original location defined in the struct." );
					ImGuiUtility::Table_Header_ManuallySubmit_AppendHelpMarker( 3, "For uniform struct members, value in parenthesis shows the original offset of the member in the struct." );
					ImGui::TableNextRow(); // Done with the header row, skip to normal rows.

					for( auto& [ uniform_name, uniform_info ] : uniform_map )
					{
						/* Skip uniform struct members; They will be drawn under their parent struct name instead. */
						if( uniform_info.original_order_in_struct != -1 )
							continue;

						if( uniform_info.IsUserDefinedStruct() )
						{
							ImGui::TableNextColumn();
							if( ImGui::TreeNodeEx( uniform_name.c_str()/*, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed*/ ) )
							{
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.location );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.size );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.offset );
								ImGui::TableNextColumn(); ImGui::TextUnformatted( "struct" );

								for( const auto& [ uniform_member_name, uniform_member_info ] : uniform_info.members )
								{
									ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_member_name.c_str() );
									ImGui::TableNextColumn(); ImGui::Text( "%d (%d)", uniform_member_info->location, uniform_member_info->original_order_in_struct );
									ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_member_info->size );
									ImGui::TableNextColumn(); ImGui::Text( "%d (%d)", uniform_member_info->offset, uniform_member_info->original_offset );
									ImGui::TableNextColumn(); ImGui::TextUnformatted( GetNameOfType( uniform_member_info->type ) );
								}

								ImGui::TreePop();
							}
							else
							{
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.location );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.size );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.offset );
								ImGui::TableNextColumn(); ImGui::TextUnformatted( "struct" );
							}
						}
						else
						{
							ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_name.c_str() );
							ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.location );
							ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.size );
							ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.offset );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( GetNameOfType( uniform_info.type ) );
						}
					}

					ImGui::EndTable();
				}

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	bool Draw( Lighting::PointLightData& point_light_data, const char* light_name, const bool hide_position, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			is_modified |= Draw( point_light_data.ambient,  "Ambient"  );
			is_modified |= Draw( point_light_data.diffuse,  "Diffuse"  );
			is_modified |= Draw( point_light_data.specular, "Specular" );
			if( !hide_position )
				is_modified |= Draw( point_light_data.position_world_space, "Position" );
			is_modified |= ImGui::SliderFloat( "Attenuation: Constant",		&point_light_data.attenuation_constant,		0.0f, 5.0f, "%.5g" );
			is_modified |= ImGui::SliderFloat( "Attenuation: Linear",		&point_light_data.attenuation_linear,		0.0f, 1.0f, "%.5g" );
			is_modified |= ImGui::SliderFloat( "Attenuation: Quadratic",	&point_light_data.attenuation_quadratic,	0.0f, 1.0f, "%.5g" );

			ImGui::PopID();
		}

		ImGui::End();

		return is_modified;
	}

	void Draw( const Lighting::PointLightData& point_light_data, const char* light_name, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			Draw( point_light_data.ambient,  "Ambient"  );
			Draw( point_light_data.diffuse,  "Diffuse"  );
			Draw( point_light_data.specular, "Specular" );
			Draw( point_light_data.position_world_space, "Position" );
			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputFloat( "Attenuation: Constant",		const_cast< float* >( &point_light_data.attenuation_constant ),  0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
			ImGui::InputFloat( "Attenuation: Linear",		const_cast< float* >( &point_light_data.attenuation_linear ),    0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
			ImGui::InputFloat( "Attenuation: Quadratic",	const_cast< float* >( &point_light_data.attenuation_quadratic ), 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );

			ImGui::PopID();
		}

		ImGui::End();
	}

	bool Draw( Lighting::SpotLightData& spot_light_data, const char* light_name, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			using namespace Engine::Math::Literals;

			is_modified |= Draw( spot_light_data.ambient,				"Ambient"				);
			is_modified |= Draw( spot_light_data.diffuse,				"Diffuse"				);
			is_modified |= Draw( spot_light_data.specular,				"Specular"				);
			is_modified |= Draw( spot_light_data.position_world_space,	"Position"				);
			is_modified |= Draw( spot_light_data.direction_world_space, "Direction"				);
			is_modified |= Draw( spot_light_data.cutoff_angle_inner,	"Cutoff Angle: Inner", 0.0_deg, spot_light_data.cutoff_angle_outer );
			is_modified |= Draw( spot_light_data.cutoff_angle_outer,	"Cutoff Angle: Outer", spot_light_data.cutoff_angle_inner, 180.0_deg );

			ImGui::PopID();
		}

		ImGui::End();

		return is_modified;
	}

	void Draw( const Lighting::SpotLightData& spot_light_data, const char* light_name, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			Draw( spot_light_data.ambient,					"Ambient"				);
			Draw( spot_light_data.diffuse,					"Diffuse"				);
			Draw( spot_light_data.specular,					"Specular"				);
			Draw( spot_light_data.position_world_space,		"Position"				);
			Draw( spot_light_data.direction_world_space,	"Direction"				);
			Draw( spot_light_data.cutoff_angle_inner,		"Cutoff Angle: Inner"   );
			Draw( spot_light_data.cutoff_angle_outer,		"Cutoff Angle: Outer"	);

			ImGui::PopID();
		}

		ImGui::End();
	}

	bool Draw( Lighting::DirectionalLightData& directional_light_data, const char* light_name, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			is_modified |= Draw( directional_light_data.ambient,				"Ambient" );
			is_modified |= Draw( directional_light_data.diffuse,				"Diffuse" );
			is_modified |= Draw( directional_light_data.specular,				"Specular" );
			is_modified |= Draw( directional_light_data.direction_world_space,	"Direction" );

			ImGui::PopID();
		}

		ImGui::End();

		return is_modified;
	}

	void Draw( const Lighting::DirectionalLightData& directional_light_data, const char* light_name, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Light Data", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( light_name );

			ImGui::PushID( light_name );

			Draw( directional_light_data.ambient,				"Ambient" );
			Draw( directional_light_data.diffuse,				"Diffuse" );
			Draw( directional_light_data.specular,				"Specular" );
			Draw( directional_light_data.direction_world_space, "Position" );
		
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

			is_modified |= ImGui::InputInt( "Diffuse Map ID",  &surface_data.diffuse_map_slot  );
			is_modified |= ImGui::InputInt( "Specular Map ID", &surface_data.specular_map_slot );
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

			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputInt( "Diffuse Map ID",  const_cast< int* >( &surface_data.diffuse_map_slot  ), 0, 0, ImGuiInputTextFlags_ReadOnly );
			ImGui::InputInt( "Specular Map ID", const_cast< int* >( &surface_data.specular_map_slot ), 0, 0, ImGuiInputTextFlags_ReadOnly );

			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputFloat( "Shininess", const_cast< float* >( &surface_data.shininess ), 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly );

			ImGui::PopID();
		}

		ImGui::End();
	}
}
