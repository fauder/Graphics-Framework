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
	}

	template< Concepts::Arithmetic Component, std::size_t Size > requires( Size > 1 )
	bool Draw( Math::Vector< Component, Size >& vector, const char* name = "##vector<>", const bool is_read_only = false )
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
			is_modified |= ImGui::InputScalarN( name, GetImGuiDataType< Component >(), vector.Data(), Size, NULL, NULL, GetFormat< Component >() );

		return is_modified;
	}

	void Draw( const Color3& color );
	void Draw( const Color4& color );

	void Draw( const Shader& shader, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	bool Draw(		 Lighting::LightData& light_data, const char* light_name, const bool hide_position = false, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const Lighting::LightData& light_data, const char* light_name, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	bool Draw(		 Lighting::SurfaceData& surface_data, const char* surface_name, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const Lighting::SurfaceData& surface_data, const char* surface_name, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
}
