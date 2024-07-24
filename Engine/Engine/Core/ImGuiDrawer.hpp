#pragma once

// Engine Includes.
#include "Core/ImGuiSetup.h"
#include "Graphics/Color.hpp"
#include "Graphics/Lighting.h"
#include "Graphics/Material.hpp"
#include "Math/Quaternion.hpp"
#include "Scene/Camera.h"

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

	bool Draw( const GLenum type,	    void* value_pointer, const char* name = "##hidden" );
	void Draw( const GLenum type, const void* value_pointer, const char* name = "##hidden" );

	bool Draw( int& scalar,					const char* name = "##scalar_int" 	 );
	bool Draw( int& scalar,					const int min, const int max, const char* name = "##scalar_int" );
	void Draw( const int& scalar,			const char* name = "##scalar_int"    );
	bool Draw( unsigned int& scalar,		const char* name = "##scalar_uint"	 );
	bool Draw( unsigned int& scalar,		const unsigned int min, const unsigned int max, const char* name = "##scalar_uint" );
	void Draw( const unsigned int& scalar,	const char* name = "##scalar_uint"	 );
	bool Draw( float& scalar,				const char* name = "##scalar_float"  );
	void Draw( const float& scalar,			const char* name = "##scalar_float"  );
	bool Draw( double& scalar,				const char* name = "##scalar_double" );
	void Draw( const double& scalar,		const char* name = "##scalar_double" );
	bool Draw( bool& value,					const char* name = "##bool" );
	void Draw( const bool& value,			const char* name = "##bool" );

	template< Concepts::Arithmetic Component, std::size_t Size > requires( Size > 1 )
	void Draw( const Math::Vector< Component, Size >& vector, const char* name = "##vector<>" )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

		if constexpr( std::is_same_v< Component, bool > )
		{
			if constexpr( Size >= 2 )
			{
				bool x = vector.X(), y = vector.Y();
				ImGui::Checkbox( "##x", &x ); ImGui::SameLine(); ImGui::Checkbox( "##y", &y );
			}
			if constexpr( Size >= 3 )
			{
				bool value = vector.Z();
				ImGui::SameLine(); ImGui::Checkbox( "##z", &value );
			}
			if constexpr( Size >= 4 )
			{
				bool value = vector.W();
				ImGui::SameLine(); ImGui::Checkbox( "##w", &value );
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
				is_modified |= ImGui::Checkbox( "##x", &vector[ 0 ] ); ImGui::SameLine(); is_modified |= ImGui::Checkbox( "##y", &vector[ 1 ] );
			}
			if constexpr( Size >= 3 )
			{
				ImGui::SameLine(); is_modified |= ImGui::Checkbox( "##z", &vector[ 2 ] );
			}
			if constexpr( Size >= 4 )
			{
				ImGui::SameLine(); is_modified |= ImGui::Checkbox( "##w", &vector[ 3 ] );
			}
		}
		else
			is_modified |= ImGui::DragScalarN( name, GetImGuiDataType< Component >(), vector.Data(), Size, 1.0f, NULL, NULL, GetFormat< Component >() );

		return is_modified;
	}

	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize > requires Concepts::NonZero< RowSize >&& Concepts::NonZero< ColumnSize >
	void Draw( const Math::Matrix< Type, RowSize, ColumnSize >& matrix, const char* name = "##matrix<>" )
	{
		if( ImGui::TreeNodeEx( name, 0 ) )
		{
			ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

			for( auto row_index = 0; row_index < RowSize; row_index++ )
			{
				const auto& row_vector = matrix.GetRow< ColumnSize >( row_index );
				ImGui::PushID( row_index );
				Draw( row_vector );
				ImGui::PopID();
			}

			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}

	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize > requires Concepts::NonZero< RowSize >&& Concepts::NonZero< ColumnSize >
	bool Draw( Math::Matrix< Type, RowSize, ColumnSize >& matrix, const char* name = "##matrix<>" )
	{
		bool is_modified = false;

		if( ImGui::TreeNodeEx( name, 0 ) )
		{
			auto& first_row_vector = matrix.GetRow< ColumnSize >();
			is_modified |= Draw( first_row_vector );

			if( name[ 0 ] != '#' || name[ 1 ] != '#' )
			{
				ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x );
				ImGui::TextUnformatted( name );
			}

			for( auto row_index = 1; row_index < RowSize; row_index++ )
			{
				ImGui::PushID( row_index );
				auto& row_vector = matrix.GetRow< ColumnSize >( row_index );
				is_modified |= Draw( row_vector );
				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		return is_modified;
	}

	template< Concepts::Arithmetic Component >
	void Draw( const Math::Quaternion< Component >& quaternion, const char* name = "##quaternion<>" )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

		Math::Vector< Math::Degrees< Component >, 3 > euler;
		Math::QuaternionToEuler( quaternion, euler );

		/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::InputScalarN( name, GetImGuiDataType< Component >(), euler.Data(), euler.Dimension(), NULL, NULL, GetFormat< Component >(), ImGuiInputTextFlags_ReadOnly );

		ImGui::PopStyleColor();
	}

	template< Concepts::Arithmetic Component >
	bool Draw( Math::Quaternion< Component >& quaternion, const char* name = "##quaternion<>" )
	{
		Math::Vector< Math::Degrees< Component >, 3 > euler;
		Math::QuaternionToEuler( quaternion, euler );

		if( ImGui::DragScalarN( name, GetImGuiDataType< Component >(), euler.Data(), euler.Dimension(), 1.0f, NULL, NULL, GetFormat< Component >() ) )
		{
			quaternion = Math::EulerToQuaternion( euler );
			return true;
		}

		return false;
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

	bool Draw(		 Texture* texture, const char* name );
	void Draw( const Texture* texture, const char* name );

	bool Draw(		 Camera& camera, const char* name = "##camera" );
	void Draw( const Camera& camera, const char* name = "##camera" );

	bool Draw(		 Transform& transform, const char* name = "##transform", const bool hide_scale = false );
	void Draw( const Transform& transform, const char* name = "##transform", const bool hide_scale = false );

	void Draw(		 Material& material,	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const Material& material,	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );

	void Draw( const Shader& shader,		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );

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
