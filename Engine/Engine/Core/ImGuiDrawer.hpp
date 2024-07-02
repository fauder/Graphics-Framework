#pragma once

// Engine Includes.
#include "Core/ImGuiSetup.h"
#include "Graphics/Color.hpp"
#include "Graphics/Lighting.h"
#include "Graphics/Shader.hpp"

// Vendor Includes.
#include "ImGui/imgui.h"

namespace Engine::ImGuiDrawer
{
	template< typename Type >
	constexpr ImGuiDataType_ GetImGuiDataType()
	{
		if constexpr( std::is_same_v< Type, int > )
			return ImGuiDataType_S32;
		if constexpr( std::is_same_v< Type, unsigned int > )
			return ImGuiDataType_U32;
		if constexpr( std::is_same_v< Type, float > )
			return ImGuiDataType_Float;
		if constexpr( std::is_same_v< Type, double > )
			return ImGuiDataType_Double;
	}

	template< typename Type >
	constexpr const char* GetFormat()
	{
		if constexpr( std::is_same_v< Type, int > )
			return "%d";
		if constexpr( std::is_same_v< Type, unsigned int > )
			return "%u";
		if constexpr( std::is_same_v< Type, float > )
			return "%.3f";
		if constexpr( std::is_same_v< Type, double > )
			return "%.3lf";
	}

	template< Concepts::Arithmetic Component, std::size_t Size > requires( Size > 1 )
	void Draw( const Math::Vector< Component, Size >& vector, const char* name = "##vector<>" )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

		if constexpr( std::is_same_v< Component, bool > )
		{
			if constexpr( Size >= 2 )
			{
				bool x = vector.X(), y = vector.Y();
				ImGui::Checkbox( "", &x ); ImGui::SameLine(); ImGui::Checkbox( "", &y );
			}
			if constexpr( Size >= 3 )
			{
				bool value = vector.Z();
				ImGui::SameLine(); ImGui::Checkbox( "", &value );
			}
			if constexpr( Size >= 4 )
			{
				bool value = vector.W();
				ImGui::SameLine(); ImGui::Checkbox( "", &value );
			}
		}
		else
			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputScalarN( name, GetImGuiDataType< Component >(), const_cast< Component* >( vector.Data() ), Size, NULL, NULL, GetFormat< Component >(), ImGuiInputTextFlags_ReadOnly );

		ImGui::PopStyleColor();
	}

	template< Concepts::Arithmetic Component, std::size_t Size > requires( Size > 1 )
	bool Draw( Math::Vector< Component, Size >& vector, const char* name = "##vector<>" )
	{
		bool is_modified = false;

		if constexpr( std::is_same_v< Component, bool > )
		{
			if constexpr( Size >= 2 )
			{
				is_modified |= ImGui::Checkbox( "", &vector.X() ); ImGui::SameLine(); is_modified |= ImGui::Checkbox( "", &vector.Y() );
			}
			if constexpr( Size >= 3 )
			{
				ImGui::SameLine(); is_modified |= ImGui::Checkbox( "", &vector.Z() );
			}
			if constexpr( Size >= 4 )
			{
				ImGui::SameLine(); is_modified |= ImGui::Checkbox( "", &vector.W() );
			}
		}
		else
			is_modified |= ImGui::DragScalarN( name, GetImGuiDataType< Component >(), vector.Data(), Size, 1.0f, NULL, NULL, GetFormat< Component >() );

		return is_modified;
	}

	bool Draw(		 Color3& color, const char* name = "##color3" );
	void Draw( const Color3& color, const char* name = "##color3" );
	bool Draw(		 Color4& color, const char* name = "##color4" );
	void Draw( const Color4& color, const char* name = "##color4" );

	template< template< class > class AngleType, typename FloatType >
	bool Draw( AngleType< FloatType >& angle, const char* name,
			   const AngleType< FloatType >& min = AngleType< FloatType >( 0 ), const AngleType< FloatType >& max = Constants< AngleType< FloatType > >::Two_Pi(),
			   const char* format = AngleType< FloatType >::Format( "%.4g" ).data() )
	{
		return ImGui::SliderFloat( name, ( float* )&angle, ( float )min, ( float )max, format );
	}

	template< template< class > class AngleType, typename FloatType >
	void Draw( const AngleType< FloatType >& angle, const char* name = "##angle", const char* format = AngleType< FloatType >::Format( "%.4g" ).data() )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

		/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::InputFloat( name, const_cast< float* >( &angle.Value() ), 0, 0, format, ImGuiInputTextFlags_ReadOnly );

		ImGui::PopStyleColor();
	}

	void Draw( const Shader& shader, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	// TODO: Remove LightData & SurfaceData overloads; Implement a generic uniform struct drawer instead, similar to Shader's implementation.
	bool Draw(		 Lighting::DirectionalLightData&	directional_light_data, const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const Lighting::DirectionalLightData&	directional_light_data, const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	bool Draw(		 Lighting::PointLightData&			point_light_data,		const char* light_name,		const bool hide_position = false,	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const Lighting::PointLightData&			point_light_data,		const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	bool Draw(		 Lighting::SpotLightData&			spot_light_data,		const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const Lighting::SpotLightData&			spot_light_data,		const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	bool Draw(		 Lighting::SurfaceData&				surface_data,			const char* surface_name,										ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const Lighting::SurfaceData&				surface_data,			const char* surface_name,										ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
}
