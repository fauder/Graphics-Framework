#define IMGUI_DEFINE_MATH_OPERATORS

// Engine Includes.
#include "AssetDatabase.hpp"
#include "ImGuiDrawer.hpp"
#include "ImGuiUtility.h"
#include "Graphics/ShaderTypeInformation.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Engine::ImGuiDrawer
{
	using namespace Engine::Math::Literals;

	void Update()
	{
		IMGUI_STYLE = &ImGui::GetStyle();

		if( ImGui::Begin( ICON_FA_PAINTBRUSH " Materials", nullptr ) )
		{
			ImGui::Checkbox( "Hide padding parameters", &WINDOW_MATERIAL_PADDING_HIDE );
			ImGui::Separator();
		}
		ImGui::End();
	}

	bool Draw( const GLenum type, void* value_pointer, const char* name )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				: return Draw( *reinterpret_cast< float*		>( value_pointer ), name );
			case GL_FLOAT_VEC2			: return Draw( *reinterpret_cast< Vector2*		>( value_pointer ), name );
			case GL_FLOAT_VEC3			: return Draw( *reinterpret_cast< Vector3*		>( value_pointer ), name );
			case GL_FLOAT_VEC4			: return Draw( *reinterpret_cast< Vector4*		>( value_pointer ), name );
			case GL_DOUBLE				: return Draw( *reinterpret_cast< double*		>( value_pointer ), name );
			case GL_INT					: return Draw( *reinterpret_cast< int*			>( value_pointer ), name );
			case GL_INT_VEC2			: return Draw( *reinterpret_cast< Vector2I*		>( value_pointer ), name );
			case GL_INT_VEC3			: return Draw( *reinterpret_cast< Vector3I*		>( value_pointer ), name );
			case GL_INT_VEC4			: return Draw( *reinterpret_cast< Vector4I*		>( value_pointer ), name );
			case GL_UNSIGNED_INT		: return Draw( *reinterpret_cast< unsigned int*	>( value_pointer ), name );
			case GL_UNSIGNED_INT_VEC2	: return Draw( *reinterpret_cast< Vector2U*		>( value_pointer ), name );
			case GL_UNSIGNED_INT_VEC3	: return Draw( *reinterpret_cast< Vector3U*		>( value_pointer ), name );
			case GL_UNSIGNED_INT_VEC4	: return Draw( *reinterpret_cast< Vector4U*		>( value_pointer ), name );
			case GL_BOOL				: return Draw( *reinterpret_cast< bool*			>( value_pointer ), name );
			case GL_BOOL_VEC2			: return Draw( *reinterpret_cast< Vector2B*		>( value_pointer ), name );
			case GL_BOOL_VEC3			: return Draw( *reinterpret_cast< Vector3B*		>( value_pointer ), name );
			case GL_BOOL_VEC4			: return Draw( *reinterpret_cast< Vector4B*		>( value_pointer ), name );
			
			/* Matrices: */
			case GL_FLOAT_MAT2 			: return Draw( *reinterpret_cast< Matrix2x2*	>( value_pointer ), name );
			case GL_FLOAT_MAT3 			: return Draw( *reinterpret_cast< Matrix3x3*	>( value_pointer ), name );
			case GL_FLOAT_MAT4 			: return Draw( *reinterpret_cast< Matrix4x4*	>( value_pointer ), name );
			case GL_FLOAT_MAT2x3 		: return Draw( *reinterpret_cast< Matrix2x3*	>( value_pointer ), name );
			case GL_FLOAT_MAT2x4 		: return Draw( *reinterpret_cast< Matrix2x4*	>( value_pointer ), name );
			case GL_FLOAT_MAT3x2 		: return Draw( *reinterpret_cast< Matrix3x2*	>( value_pointer ), name );
			case GL_FLOAT_MAT3x4 		: return Draw( *reinterpret_cast< Matrix3x4*	>( value_pointer ), name );
			case GL_FLOAT_MAT4x2 		: return Draw( *reinterpret_cast< Matrix4x2*	>( value_pointer ), name );
			case GL_FLOAT_MAT4x3 		: return Draw( *reinterpret_cast< Matrix4x3*	>( value_pointer ), name );

			/* Other: */
			case GL_SAMPLER_1D 			: return Draw( *reinterpret_cast< unsigned int*	>( value_pointer ), name );
			case GL_SAMPLER_2D 			: return Draw( *reinterpret_cast< unsigned int*	>( value_pointer ), name );
			case GL_SAMPLER_3D 			: return Draw( *reinterpret_cast< unsigned int*	>( value_pointer ), name );
			case GL_SAMPLER_CUBE		: return Draw( *reinterpret_cast< unsigned int*	>( value_pointer ), name );
		}

		throw std::runtime_error( "ERROR::IMGUIDRAWER::DRAW( type, void* value_pointer ) called for an undefined GL type!" );
	}

	void Draw( const GLenum type, const void* value_pointer, const char* name )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				: return Draw( *reinterpret_cast< const float*			>( value_pointer ), name );
			case GL_FLOAT_VEC2			: return Draw( *reinterpret_cast< const Vector2*		>( value_pointer ), name );
			case GL_FLOAT_VEC3			: return Draw( *reinterpret_cast< const Vector3*		>( value_pointer ), name );
			case GL_FLOAT_VEC4			: return Draw( *reinterpret_cast< const Vector4*		>( value_pointer ), name );
			case GL_DOUBLE				: return Draw( *reinterpret_cast< const double*			>( value_pointer ), name );
			case GL_INT					: return Draw( *reinterpret_cast< const int*			>( value_pointer ), name );
			case GL_INT_VEC2			: return Draw( *reinterpret_cast< const Vector2I*		>( value_pointer ), name );
			case GL_INT_VEC3			: return Draw( *reinterpret_cast< const Vector3I*		>( value_pointer ), name );
			case GL_INT_VEC4			: return Draw( *reinterpret_cast< const Vector4I*		>( value_pointer ), name );
			case GL_UNSIGNED_INT		: return Draw( *reinterpret_cast< const unsigned int*	>( value_pointer ), name );
			case GL_UNSIGNED_INT_VEC2	: return Draw( *reinterpret_cast< const Vector2U*		>( value_pointer ), name );
			case GL_UNSIGNED_INT_VEC3	: return Draw( *reinterpret_cast< const Vector3U*		>( value_pointer ), name );
			case GL_UNSIGNED_INT_VEC4	: return Draw( *reinterpret_cast< const Vector4U*		>( value_pointer ), name );
			case GL_BOOL				: return Draw( *reinterpret_cast< const bool*			>( value_pointer ), name );
			case GL_BOOL_VEC2			: return Draw( *reinterpret_cast< const Vector2B*		>( value_pointer ), name );
			case GL_BOOL_VEC3			: return Draw( *reinterpret_cast< const Vector3B*		>( value_pointer ), name );
			case GL_BOOL_VEC4			: return Draw( *reinterpret_cast< const Vector4B*		>( value_pointer ), name );

			/* Matrices: */
			case GL_FLOAT_MAT2 			: return Draw( *reinterpret_cast< const Matrix2x2*		>( value_pointer ), name );
			case GL_FLOAT_MAT3 			: return Draw( *reinterpret_cast< const Matrix3x3*		>( value_pointer ), name );
			case GL_FLOAT_MAT4 			: return Draw( *reinterpret_cast< const Matrix4x4*		>( value_pointer ), name );
			case GL_FLOAT_MAT2x3 		: return Draw( *reinterpret_cast< const Matrix2x3*		>( value_pointer ), name );
			case GL_FLOAT_MAT2x4 		: return Draw( *reinterpret_cast< const Matrix2x4*		>( value_pointer ), name );
			case GL_FLOAT_MAT3x2 		: return Draw( *reinterpret_cast< const Matrix3x2*		>( value_pointer ), name );
			case GL_FLOAT_MAT3x4 		: return Draw( *reinterpret_cast< const Matrix3x4*		>( value_pointer ), name );
			case GL_FLOAT_MAT4x2 		: return Draw( *reinterpret_cast< const Matrix4x2*		>( value_pointer ), name );
			case GL_FLOAT_MAT4x3 		: return Draw( *reinterpret_cast< const Matrix4x3*		>( value_pointer ), name );

			/* Other: */
			case GL_SAMPLER_1D 			: return Draw( *reinterpret_cast< const unsigned int*	>( value_pointer ), name );
			case GL_SAMPLER_2D 			: return Draw( *reinterpret_cast< const unsigned int*	>( value_pointer ), name );
			case GL_SAMPLER_3D 			: return Draw( *reinterpret_cast< const unsigned int*	>( value_pointer ), name );
			case GL_SAMPLER_CUBE 		: return Draw( *reinterpret_cast< const unsigned int*	>( value_pointer ), name );
		}

		throw std::runtime_error( "ERROR::IMGUIDRAWER::DRAW( type, const void* value_pointer ) called for an undefined GL type!" );
	}

	bool Draw( int& scalar, const char* name )
	{
		return ImGui::DragInt( name, &scalar );
	}

	bool Draw( int& scalar, const int min, const int max, const char* name )
	{
		return ImGui::SliderInt( name, &scalar, min, max );
	}

	void Draw( const int& scalar, const char* name )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::InputInt( name, const_cast< int* >( &scalar ), 0, 0, ImGuiInputTextFlags_ReadOnly );
		ImGui::PopStyleColor();
	}

	bool Draw( unsigned int& scalar, const char* name )
	{
		return ImGui::DragScalar( name, GetImGuiDataType< unsigned int >(), &scalar, 1.0f, 0, 0, GetFormat< unsigned int >() );
	}

	bool Draw( unsigned int& scalar, const unsigned int min, const unsigned int max, const char* name )
	{
		return ImGui::SliderScalar( name, GetImGuiDataType< unsigned int >(), &scalar, &min, &max, GetFormat< unsigned int >() );
	}

	void Draw( const unsigned int& scalar, const char* name )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::InputScalar( name, GetImGuiDataType< unsigned int >(), const_cast< unsigned int* >( &scalar ), 0, 0, GetFormat< unsigned int >(), ImGuiInputTextFlags_ReadOnly );
		ImGui::PopStyleColor();
	}

	bool Draw( float& scalar, const char* name, const char* format )
	{
		return ImGui::DragFloat( name, &scalar, 1.0f, 0.0f, 0.0f, format );
	}

	void Draw( const float& scalar, const char* name, const char* format )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::InputFloat( name, const_cast< float* >( &scalar ), 0.0f, 0.0f, GetFormat< float >(), ImGuiInputTextFlags_ReadOnly );
		ImGui::PopStyleColor();
	}

	bool Draw( double& scalar, const char* name, const char* format )
	{
		return ImGui::DragScalar( name, GetImGuiDataType< double >(), &scalar, 1.0f, nullptr, nullptr, format );
	}

	void Draw( const double& scalar, const char* name, const char* format )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::InputScalar( name, GetImGuiDataType< double >(), const_cast< double* >( &scalar ), 0, 0, GetFormat< double >(), ImGuiInputTextFlags_ReadOnly );
		ImGui::PopStyleColor();
	}

	bool Draw( bool& value, const char* name )
	{
		return ImGui::Checkbox( name, &value );
	}

	void Draw( const bool& value, const char* name )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::Checkbox( name, const_cast< bool* >( &value ) );
		ImGui::PopStyleColor();
	}

	bool Draw( Color3& color, const char* name )
	{
		ImGui::PushItemWidth( 3.0f * ImGui::CalcTextSize( "R:  255" ).x + 3.0f * IMGUI_STYLE->ItemInnerSpacing.x + 2.0f * IMGUI_STYLE->FramePadding.x );
		return ImGui::ColorEdit3( name, color.Data() );
		ImGui::PopItemWidth();
	}

	void Draw( const Color3& color, const char* name )
	{
		ImGui::PushItemWidth( 3.0f * ImGui::CalcTextSize( "R:  255" ).x + 3.0f * IMGUI_STYLE->ItemInnerSpacing.x + 2.0f * IMGUI_STYLE->FramePadding.x );
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		/* Since the no inputs & no picker flags are passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::ColorEdit3( name, const_cast< float* >( color.Data() ), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker );
		ImGui::PopStyleColor();
		ImGui::PopItemWidth();
	}

	bool Draw( Color4& color, const char* name )
	{
		ImGui::PushItemWidth( 4.0f * ImGui::CalcTextSize( "R:  255" ).x + 4.0f * IMGUI_STYLE->ItemInnerSpacing.x + 2.0f * IMGUI_STYLE->FramePadding.x );
		return ImGui::ColorEdit4( name, color.Data() );
		ImGui::PopItemWidth();
	}

	void Draw( const Color4& color, const char* name )
	{
		ImGui::PushItemWidth( 4.0f * ImGui::CalcTextSize( "R:  255" ).x + 4.0f * IMGUI_STYLE->ItemInnerSpacing.x + 2.0f * IMGUI_STYLE->FramePadding.x );
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		/* Since the no inputs & no picker flags are passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::ColorEdit4( name, const_cast< float* >( color.Data() ), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker );
		ImGui::PopStyleColor();
		ImGui::PopItemWidth();
	}

	bool Draw( Transform& transform, const BitFlags< Transform::Mask > flags, const char* name )
	{
		bool is_modified = false;

		ImGuiUtility::BeginGroupPanel( name );

		ImGui::PushID( name );

		if( flags.IsSet( Transform::Mask::Translation ) )
		{
			Vector3 translation = transform.GetTranslation();
			if( Draw( translation, " " ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT " Position") )
			{
				is_modified = true;
				transform.SetTranslation( translation );
			}
		}

		if( flags.IsSet( Transform::Mask::Rotation ) )
		{
			Quaternion rotation = transform.GetRotation();
			if( Draw( rotation, " " ICON_FA_ARROW_ROTATE_RIGHT " Rotation" ) )
			{
				is_modified = true;
				transform.SetRotation( rotation );
			}
		}

		if( flags.IsSet( Transform::Mask::Scale ) )
		{
			Vector3 scale = transform.GetScaling();
			if( Draw( scale, " " ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER " Scale" ) )
			{
				is_modified = true;
				transform.SetScaling( scale );
			}
		}

		ImGui::PopID();

		ImGuiUtility::EndGroupPanel();

		return is_modified;
	}

	void Draw( const Engine::Transform& transform, const BitFlags< Transform::Mask > flags, const char* name )
	{
		ImGuiUtility::BeginGroupPanel( name );

		ImGui::PushID( name );

		if( flags.IsSet( Transform::Mask::Translation ) )
			Draw( transform.GetTranslation(), " " ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT " Position" );
		if( flags.IsSet( Transform::Mask::Rotation ) )
			Draw( transform.GetRotation(), " " ICON_FA_ARROW_ROTATE_RIGHT " Rotation" );
		if( flags.IsSet( Transform::Mask::Scale ) )
			Draw( transform.GetScaling(), " " ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER " Scale" );

		ImGui::PopID();

		ImGuiUtility::EndGroupPanel();
	}

	bool Draw( Math::Polar2& polar_coords, const bool show_radius, const char* name )
	{
		bool is_modified = false;

		ImGui::PushID( name );

		is_modified |= Draw( polar_coords.Theta(), "Theta" );
		
		if( show_radius )
			is_modified |= Draw( polar_coords.Radius(), "Radius" );

		ImGui::PopID();

		return is_modified;
	}
	
	void Draw( const Math::Polar2& polar_coords, const char* name )
	{
		Draw( polar_coords.Theta(),	 "Theta"  );
		Draw( polar_coords.Radius(), "Radius" );
	}

	bool Draw( Math::Polar3_Spherical_Game& spherical_coords, const bool show_radius, const char* name )
	{
		bool is_modified = false;

		ImGui::PushID( name );

		is_modified |= Draw( spherical_coords.Heading(), "Heading" );
		is_modified |= Draw( spherical_coords.Pitch(),	 "Pitch"   );

		if( show_radius )
			is_modified |= Draw( spherical_coords.Radius(), "Radius" );

		ImGui::PopID();

		return is_modified;
	}
	
	void Draw( const Math::Polar3_Spherical_Game& spherical_coords, const char* name )
	{
		Draw( spherical_coords.Heading(), "Heading" );
		Draw( spherical_coords.Pitch(),	  "Pitch"	);
		Draw( spherical_coords.Radius(),  "Radius"  );
	}

	bool Draw( Texture* texture, const char* name )
	{
		// TODO: Implement texture selection.

		if( texture )
		{
			ImGui::Image( ( void* )( intptr_t )texture->Id(), ImVec2( 24, 24 ), { 0, 1 }, { 1, 0 } );
			ImGui::SameLine();
			ImGui::TextColored( ImVec4( 0.84f, 0.59f, 0.45f, 1.0f ), "%s (ID: %d)", texture->Name().c_str(), texture->Id() );
		}
		else
		{
			ImGui::Dummy( ImVec2( 24, 24 ) + IMGUI_STYLE->ItemInnerSpacing );
			ImGui::SameLine();
			ImGui::TextColored( ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ), "    <unassigned>" );
		}

		return false;
	}

	void Draw( const Texture* texture, const char* name )
	{
		ImGui::BeginDisabled();

		if( texture )
		{
			ImGui::Image( ( void* )( intptr_t )texture->Id(), ImVec2( 24, 24 ), { 0, 1 }, { 1, 0 } );
			ImGui::SameLine();
			ImGui::TextColored( ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ), "%s (ID: %d)", texture->Name().c_str(), texture->Id() );
		}
		else
		{
			ImGui::Dummy( ImVec2( 24, 24 ) + IMGUI_STYLE->ItemInnerSpacing );
			ImGui::SameLine();
			ImGui::TextColored( ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ), "    <unassigned>" );
		}

		ImGui::EndDisabled();
	}

	void Draw( const std::map< std::string, Texture >& texture_map, const Vector2& window_size )
	{
		static const Texture* selected_texture = nullptr;

		ImGui::SetNextWindowSize( reinterpret_cast< const ImVec2& >( window_size ) );
		if( ImGui::Begin( ICON_FA_IMAGE " Textures" ) )
		{
			const ImVec2 region_available( ImGui::GetContentRegionAvail() );
			const float separator_height( IMGUI_STYLE->ItemSpacing.y * 2.0f );
			const float line_height( ImGui::CalcTextSize( "A" ).y );

			const ImVec2 child_size( region_available.x, region_available.y / 2.0f - separator_height - line_height );

			if( ImGui::BeginChild( "Textures", child_size ) )
			{
				for( const auto& [ asset_name, asset ] : texture_map )
				{
					ImGui::PushID( asset_name.c_str() );
					if( ImGui::Selectable( "" ) )
						selected_texture = &asset;
					ImGui::PopID();
					ImGui::SameLine();
					Engine::ImGuiDrawer::Draw( &asset, asset_name.c_str() );
				}
			}

			ImGui::EndChild();

			ImGui::Separator();

			if( selected_texture )
			{
				const ImVec2 preview_area_size( ImGui::GetContentRegionAvail() - ImVec2( 0.0f, line_height ) );

				const float image_width( ( float )selected_texture->Width() );
				const float image_height( ( float )selected_texture->Height() );
				const float image_aspect_ratio = image_width / image_height;

				const float image_width_fit  = preview_area_size.y < preview_area_size.x ? image_aspect_ratio * preview_area_size.y : preview_area_size.x;
				const float image_height_fit = image_width_fit / image_aspect_ratio;

				const float padding_x = ( preview_area_size.x - image_width_fit ) / 2.0f;
				const float padding_y = ( preview_area_size.y - image_height_fit ) / 2.0f;

				ImGui::SetCursorPos( ImGui::GetCursorPos() + ImVec2( padding_x, padding_y ) );
				ImGui::Image( ( void* )( intptr_t )selected_texture->Id(), ImVec2( image_width_fit, image_height_fit ), { 0, 1 }, { 1, 0 } );

				char info_line_buffer[ 255 ];
				sprintf_s( info_line_buffer, 255, "%dx%d", ( int )image_width, ( int )image_height );
				ImGui::Indent( ( preview_area_size.x - ImGui::CalcTextSize( info_line_buffer ).x ) / 2.0f );
				ImGui::TextUnformatted( info_line_buffer );
			}
			else
				ImGui::Dummy( ImGui::GetContentRegionAvail() );
		}

		ImGui::End();
	}

	bool Draw( Camera& camera, const char* name )
	{
		bool is_modified = false;

		if( ImGui::TreeNodeEx( name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
		{
			float near_plane   = camera.GetNearPlaneOffset();
			float far_plane    = camera.GetFarPlaneOffset();
			float aspect_ratio = camera.GetAspectRatio();
			float vertical_fov = ( float )camera.GetVerticalFieldOfView();

			/*																		Min Value:		Max Value:		Format: */
			if( is_modified |= ImGui::SliderFloat( "Near Plane",	&near_plane,	0.0f,			far_plane						) )
				camera.SetNearPlaneOffset( near_plane );
			if( is_modified |= ImGui::SliderFloat( "Far Plane",		&far_plane,		near_plane,		1000.0f							) )
				camera.SetFarPlaneOffset( far_plane );
			if( is_modified |= ImGui::SliderFloat( "Aspect Ratio",	&aspect_ratio,	0.1f,			5.0f							) )
				camera.SetAspectRatio( aspect_ratio );
			if( is_modified |= ImGui::SliderAngle( "Vertical FoV",	&vertical_fov,	1.0f,			180.0f,			"%.3f degrees"	) )
				camera.SetVerticalFieldOfView( Radians( vertical_fov ) );

			ImGui::TreePop();
		}

		return is_modified;
	}

	void Draw( const Camera& camera, const char* name )
	{
		if( ImGui::TreeNodeEx( name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
		{
			float near_plane   = camera.GetNearPlaneOffset();
			float far_plane    = camera.GetFarPlaneOffset();
			float aspect_ratio = camera.GetAspectRatio();
			float vertical_fov = ( float )camera.GetVerticalFieldOfView();

			ImGui::InputFloat( "Near Plane",	&near_plane,	0, 0, "%.3f",			ImGuiInputTextFlags_ReadOnly );
			ImGui::InputFloat( "Far Plane",		&far_plane,		0, 0, "%.3f",			ImGuiInputTextFlags_ReadOnly );
			ImGui::InputFloat( "Aspect Ratio",	&aspect_ratio,	0, 0, "%.3f",			ImGuiInputTextFlags_ReadOnly );
			ImGui::SliderAngle( "Vertical FoV", &vertical_fov,	0, 0, "%.3f degrees",	ImGuiInputTextFlags_ReadOnly );
		
			ImGui::TreePop();
		}
	}

	bool Draw( Material& material, Renderer& renderer, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		Shader* new_shader_to_assign = nullptr; // This is only set upon new shader assignment.

		if( ImGui::Begin( ICON_FA_PAINTBRUSH " Materials", nullptr ) )
		{
			if( ImGui::TreeNodeEx( material.Name().c_str()/*, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed*/ ) )
			{
				ImGuiUtility::BeginGroupPanel();

				ImGui::TextColored( ImGui::GetStyleColorVec4( ImGuiCol_Header ), "Shader:" );
				ImGui::SameLine();

				const auto& current_shader_name( material.HasShaderAssigned() ? material.GetShaderName() : "" );

				const auto& registered_shaders( renderer.RegisteredShaders() );
				const auto& preview( current_shader_name.empty() ? ( *registered_shaders.begin() )->Name() : ICON_FA_CODE " " + current_shader_name );
				if( ImGui::BeginCombo( "##Shader Selection Combobox", preview.c_str(), ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_HeightLarge ) )
				{
					for( const auto& shader : registered_shaders )
					{
						const auto& selectable_shader_name( shader->Name() );

						if( ImGui::Selectable( ( ICON_FA_CODE " " + selectable_shader_name ).c_str() ) && current_shader_name != selectable_shader_name )
							new_shader_to_assign = shader;
					}
					ImGui::EndCombo();
				}

				ImGui::SeparatorText( "Parameters" );

				const auto table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
				if( ImGui::BeginTable( material.Name().c_str(), 2, table_flags ) )
				{
					const auto& uniform_info_map        = material.GetUniformInfoMap();
					const auto& uniform_buffer_info_map = material.GetUniformBufferInfoMap();
					const auto& texture_map             = material.GetTextureMap();

					ImGui::TableNextRow();

					for( const auto& [ uniform_name, uniform_info ] : uniform_info_map )
					{
						/* Skip uniform buffer members; They will be drawn under their parent uniform buffer instead. */
						if( uniform_info.is_buffer_member )
							continue;

						ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_info.editor_name.c_str() );

						ImGui::TableNextColumn();

						/* No need to update the Material when the Draw() call below returns true; Memory from the blob is provided directly to Draw(), so the Material is updated. */
						ImGui::PushID( ( void* )&uniform_info );
						switch( uniform_info.usage_hint )
						{
							case UsageHint::AsColor3:
								is_modified |= Draw( *reinterpret_cast< Color3* >( material.Get( uniform_info ) ) );
								break;
							case UsageHint::AsColor4:
								is_modified |= Draw( *reinterpret_cast< Color4* >( material.Get( uniform_info ) ) );
								break;
							case UsageHint::AsArray:
							{
								// TODO: Handle different array ranks (currently hard-coded to 2).

								void* address = material.Get( uniform_info );

								ImGui::PushItemWidth( uniform_info.usage_hint_array_dimensions[ 1 ] * ImGui::CalcTextSize( " []" ).x + ( uniform_info.usage_hint_array_dimensions[ 1 ] - 1 ) * IMGUI_STYLE->ItemInnerSpacing.x );

								for( int i = 0; i < uniform_info.usage_hint_array_dimensions[ 0 ]; i++ )
								{
									for( int j = 0; j < uniform_info.usage_hint_array_dimensions[ 1 ]; j++ )
									{
										void* element_address = GL::Type::AddressOf( uniform_info.type, address, + i * uniform_info.usage_hint_array_dimensions[ 1 ] + j );
										ImGui::PushID( element_address );
										is_modified |= Draw( uniform_info.type, element_address );
										ImGui::PopID();
										ImGui::SameLine();
									}

									ImGui::NewLine();
								}

								ImGui::PopItemWidth();

								ImGui::NewLine();
							}
								break;
							case UsageHint::AsSlider_Normalized:
								is_modified |= ImGui::SliderFloat( "##scalar_float", reinterpret_cast< float* >( material.Get( uniform_info ) ), 0.0f, 1.0f, GetFormat< float >() );
								break;
							case UsageHint::AsSlider_Normalized_Percentage:
							{
								float& actual_uniform = *reinterpret_cast< float* >( material.Get( uniform_info ) );
								float temp = actual_uniform * 100.0f;
								if( ImGui::SliderFloat( "##scalar_float", &temp, 0.0f, 100.0f, "%.1f%%" ) )
								{
									actual_uniform = temp / 100.0f;
									is_modified = true;
								}
							}
								break;
							default:
								is_modified |= Draw( uniform_info.type, material.Get( uniform_info ) );
								break;
						}
						ImGui::PopID();
					}

					for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					{
						ASSERT_DEBUG_ONLY( not uniform_buffer_info.IsGlobalOrIntrinsic() && "Materials can not have Intrinsic/Global uniforms!" );

						ImGui::TableNextColumn();

						if( ImGui::TreeNodeEx( uniform_buffer_name.c_str(), ImGuiTreeNodeFlags_Framed ) )
						{
							ImGui::TableNextRow();

							std::byte* memory_blob = ( std::byte* )material.Get( uniform_buffer_name );

							for( const auto& [ uniform_buffer_member_struct_name, uniform_buffer_member_struct_info ] : uniform_buffer_info.members_struct_map )
							{
								ImGui::TableNextColumn();

								if( ImGui::TreeNodeEx( uniform_buffer_member_struct_info.editor_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
								{
									ImGui::TableNextRow();

									for( const auto& uniform_buffer_member_info : uniform_buffer_member_struct_info.members_map )
									{
										ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_info->editor_name.c_str() );

										ImGui::TableNextColumn();

										std::byte* effective_offset = memory_blob + uniform_buffer_member_info->offset;

										ImGui::PushID( ( void* )uniform_buffer_member_info );
										is_modified |= Draw( uniform_buffer_member_info->type, effective_offset );
										/* Draw() call above modifies the memory provided but an explicit material.SetPartial_Struct() call is necessary for proper registration of the modification. */
										if( is_modified )
											material.SetPartial_Struct( uniform_buffer_name, uniform_buffer_member_struct_name.c_str(), effective_offset );
										ImGui::PopID();
									}

									ImGui::TreePop();
								}
								else
									ImGui::TableNextRow();
							}

							for( const auto& [ uniform_buffer_member_array_name, uniform_buffer_member_array_info ] : uniform_buffer_info.members_array_map )
							{
								ImGui::TableNextColumn();

								if( ImGui::TreeNodeEx( uniform_buffer_member_array_info.editor_name.c_str(), ImGuiTreeNodeFlags_Framed ) )
								{
									ImGui::TableNextRow();

									for( auto array_index = 0; array_index < uniform_buffer_member_array_info.element_count; array_index++ )
									{
										ImGui::TableNextColumn();

										static char array_member_name_string[ 255 ];
										std::snprintf( array_member_name_string, 255, "%s[%d]", uniform_buffer_member_array_info.editor_name.c_str(), array_index );

										if( ImGui::TreeNodeEx( array_member_name_string, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
										{
											ImGui::TableNextRow();

											for( const auto& uniform_buffer_member_info : uniform_buffer_member_array_info.members_map )
											{
												ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_info->editor_name.c_str() );

												ImGui::TableNextColumn();

												std::byte* effective_offset = memory_blob + uniform_buffer_member_info->offset + array_index * uniform_buffer_member_array_info.stride;

												ImGui::PushID( effective_offset );
												is_modified |= Draw( uniform_buffer_member_info->type, effective_offset );
												/* Draw() call above modifies the memory provided but an explicit material.SetPartial_Array() call is necessary for proper registration of the modification. */
												if( is_modified )
													material.SetPartial_Array( uniform_buffer_name, uniform_buffer_member_array_name.c_str(), array_index, effective_offset );
												ImGui::PopID();
											}

											ImGui::TreePop();
										}
										else
											ImGui::TableNextRow();
									}

									ImGui::TreePop();
								}
								else
									ImGui::TableNextRow();
							}

							for( const auto& [ uniform_buffer_member_name, uniform_buffer_member_info ] : uniform_buffer_info.members_single_map )
							{
								const bool is_padding = uniform_buffer_member_info->editor_name.compare( 0, 8, "Padding", 8 ) == 0;

								if( is_padding && WINDOW_MATERIAL_PADDING_HIDE )
									continue;

								ImGui::BeginDisabled( is_padding );

								ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_info->editor_name.c_str() );
								ImGui::TableNextColumn();

								std::byte* effective_offset = memory_blob + uniform_buffer_member_info->offset;

								ImGui::PushID( ( void* )uniform_buffer_member_info );
								is_modified |= Draw( uniform_buffer_member_info->type, effective_offset );
								ImGui::EndDisabled();
								/* Draw() call above modifies the memory provided but an explicit material.SetPartial() call is necessary for proper registration of the modification. */
								if( is_modified )
									material.SetPartial( uniform_buffer_name, uniform_buffer_member_name.c_str(), effective_offset );
								ImGui::PopID();
							}

							ImGui::TreePop();
						}
						else
							ImGui::TableNextRow();
					}

					ImGui::EndTable();

					if( not texture_map.empty() )
					{
						ImGui::SeparatorText( "Textures" );

						if( ImGui::BeginTable( material.Name().c_str(), 2, table_flags ) )
						{
							for( const auto& [ uniform_sampler_name, texture_pointer ] : texture_map )
							{
								ImGui::TableNextColumn(); ImGui::TextUnformatted( ( "Texture: " + uniform_info_map.at( uniform_sampler_name ).editor_name ).c_str() );

								ImGui::TableNextColumn();

								const auto& current_texture_name( texture_pointer ? ICON_FA_IMAGE " " + texture_pointer->Name() : "<unassigned>");
								if( ImGui::BeginCombo( ( "##Texture Selection Combobox-" + uniform_sampler_name ).c_str(), current_texture_name.c_str(), ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_HeightLarge ) )
								{
									const auto& texture_map( AssetDatabase< Texture >::Assets() );
									for( const auto& [ texture_name, texture ] : texture_map )
									{
										const auto& selectable_texture_name( texture_name );

										if( ImGui::Selectable( ( ICON_FA_IMAGE " " + texture.Name() ).c_str() ) )
											if( texture_name != current_texture_name )
												material.SetTexture( uniform_sampler_name.c_str(), &texture );
									}
									ImGui::EndCombo();
								}
							}

							ImGui::EndTable();
						}
					}
				}

				ImGuiUtility::EndGroupPanel();

				ImGui::TreePop();
			}
		}

		ImGui::End();

		/* Update the shader at the end of all stuff, to prevent invalidating stuff (textures_map of the Material for example). */
		if( new_shader_to_assign )
		{
			const auto& previous_shader = material.GetShader();
			material.SetShader( new_shader_to_assign );

			renderer.OnShaderReassign( previous_shader, material.Name() );

			// TODO: Maybe compare previous vs current shader's vertex layouts and only allow matches?
		}

		return is_modified;
	}

	void Draw( const Shader& shader, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( ICON_FA_CODE " Shaders", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			const auto& uniform_info_map = shader.GetUniformInfoMap();

			if( ImGui::TreeNodeEx( shader.Name().c_str()/*, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed*/ ) )
			{
				ImGuiUtility::BeginGroupPanel();

				ImGui::SeparatorText( "Source Files" );
				auto DrawSourceFiles = [ & ]( const std::string& tree_node_name, const std::string& source_path, const std::vector< std::string >& include_paths )
				{
					if( not source_path.empty() && ImGui::TreeNodeEx( tree_node_name.c_str()/*, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed*/ ) )
					{
						if( ImGui::BeginTable( "Source Files", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
						{
							ImGui::TableSetupColumn( "Type" );
							ImGui::TableSetupColumn( "Path" );

							ImGui::TableHeadersRow();
							ImGui::TableNextRow();

							ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Source" );
							ImGui::TableNextColumn(); ImGui::Text( source_path.c_str() );
							for( auto& include_path : include_paths )
							{
								ImGui::TableNextColumn(); ImGui::TextUnformatted( "Include" );
								ImGui::TableNextColumn(); ImGui::Text( include_path.c_str() );
							}

							ImGui::PopStyleColor();

							ImGui::EndTable();
						}

						ImGui::TreePop();
					}
				};

				DrawSourceFiles( "Vertex Shader##"   + shader.Name(), shader.VertexSourcePath(),   shader.VertexSourceIncludePaths()   );
				DrawSourceFiles( "Geometry Shader##" + shader.Name(), shader.GeometrySourcePath(), shader.GeometrySourceIncludePaths() );
				DrawSourceFiles( "Fragment Shader##" + shader.Name(), shader.FragmentSourcePath(), shader.FragmentSourceIncludePaths() );

				ImGui::NewLine();
				ImGui::SeparatorText( "Uniforms" );

				if( ImGui::BeginTable( "Uniforms", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
				{
					ImGui::TableSetupColumn( "Name"		);
					ImGui::TableSetupColumn( "Location" );
					ImGui::TableSetupColumn( "Size"		);
					ImGui::TableSetupColumn( "Offset"	);
					ImGui::TableSetupColumn( "Type"		);

					ImGui::TableHeadersRow();
					ImGui::TableNextRow();

					ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

					for( const auto& [ uniform_name, uniform_info ] : uniform_info_map )
					{
						/* Skip uniform buffer members; They will be drawn under their parent uniform buffer instead. */
						if( uniform_info.is_buffer_member )
							continue;

						ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_name.c_str() );
						ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.location_or_block_index );
						ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.size );
						ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info.offset );
						ImGui::TableNextColumn(); ImGui::TextUnformatted( GL::Type::NameOf( uniform_info.type ) );
					}

					ImGui::PopStyleColor();

					ImGui::EndTable();
				}

				ImGui::NewLine();
				ImGui::SeparatorText( "Uniform Buffers" );

				if( ImGui::BeginTable( "Uniform Buffers", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
				{
					ImGui::TableSetupColumn( "Name"				);
					ImGui::TableSetupColumn( "Binding/Location" );
					ImGui::TableSetupColumn( "Size"				);
					ImGui::TableSetupColumn( "Offset"			);
					ImGui::TableSetupColumn( "Category/Type"	);

					ImGui::TableHeadersRow();
					ImGui::TableNextRow();

					ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

					auto DrawUniformBufferInfos = []( const auto& uniform_buffer_info_map )
					{
						for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
						{
							ImGui::TableNextColumn();

							if( ImGui::TreeNodeEx( uniform_buffer_name.c_str()/*, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed*/ ) )
							{
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_buffer_info.binding_point );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_buffer_info.size );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_buffer_info.offset );
								ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_info.CategoryString( uniform_buffer_info.category ) );

								for( const auto& [ uniform_name, uniform_info ] : uniform_buffer_info.members_map )
								{
									ImGui::TableNextColumn();

									if( ImGui::TreeNodeEx( uniform_name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen ) )
									{
										ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info->location_or_block_index );
										ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info->size );
										ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_info->offset );
										ImGui::TableNextColumn(); ImGui::TextUnformatted( GL::Type::NameOf( uniform_info->type ) );
									}
								}

								ImGui::TreePop();

								ImGui::TableNextRow();
							}
							else
							{
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_buffer_info.binding_point );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_buffer_info.size );
								ImGui::TableNextColumn(); ImGui::Text( "%d", uniform_buffer_info.offset );
								ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_info.CategoryString( uniform_buffer_info.category ) );
							}
						}
					};

					DrawUniformBufferInfos( shader.GetUniformBufferInfoMap_Regular() );
					DrawUniformBufferInfos( shader.GetUniformBufferInfoMap_Global() );
					DrawUniformBufferInfos( shader.GetUniformBufferInfoMap_Intrinsic() );

					ImGui::PopStyleColor();

					ImGui::EndTable();
				}

				ImGui::NewLine();
				ImGui::SeparatorText( "Features" );
				
				if( const auto& features = shader.Features();
					features.empty() )
				{
					ImGui::TextDisabled( "(None)" );
				}
				else
				{
					if( ImGui::BeginTable( "Feature", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
					{
						ImGui::TableSetupColumn( "Name" );
						ImGui::TableSetupColumn( "Is Set" );
						ImGui::TableSetupColumn( "Value" );

						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						
						ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

						for( const auto& [ feature_name, feature ] : shader.Features() )
						{
							ImGui::TableNextColumn();
							ImGui::TextUnformatted( feature_name.c_str() );
							ImGui::TableNextColumn();
							bool is_set = feature.is_set;
							ImGui::Checkbox( ( "##" + feature_name ).c_str(), &is_set );
							ImGui::TableNextColumn();
							if( feature.value )
								ImGui::TextUnformatted( feature.value->c_str() );
						}

						ImGui::PopStyleColor();

						ImGui::EndTable();
					}
				}

				ImGuiUtility::EndGroupPanel();

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	bool Draw( DirectionalLight& directional_light, const char* light_name, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		ImGuiUtility::BeginGroupPanel( light_name, &directional_light.is_enabled );

		ImGui::PushID( light_name );

		is_modified |= Draw( directional_light.data.ambient,  "Ambient"  );
		is_modified |= Draw( directional_light.data.diffuse,  "Diffuse"  );
		is_modified |= Draw( directional_light.data.specular, "Specular" );

		is_modified |= Draw( *directional_light.transform, Transform::Mask::Rotation, "Transform" );

		ImGui::PopID();

		ImGuiUtility::EndGroupPanel( &directional_light.is_enabled );

		return is_modified;
	}

	void Draw( const DirectionalLight& directional_light, const char* light_name, ImGuiWindowFlags window_flags )
	{
		bool dummy_enabled = directional_light.is_enabled;
		ImGuiUtility::BeginGroupPanel( light_name, &dummy_enabled );

		ImGui::PushID( light_name );

		Draw( directional_light.data.ambient,  "Ambient" );
		Draw( directional_light.data.diffuse,  "Diffuse" );
		Draw( directional_light.data.specular, "Specular" );

		Draw( *directional_light.transform, Transform::Mask::Rotation, "Transform" );

		ImGui::PopID();

		ImGuiUtility::EndGroupPanel( &dummy_enabled );
	}

	bool Draw( PointLight& point_light, const char* light_name, const bool hide_position, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		bool was_enabled = point_light.is_enabled;

		ImGuiUtility::BeginGroupPanel( light_name, &point_light.is_enabled );

		ImGui::PushID( light_name );

		is_modified |= Draw( point_light.data.ambient_and_attenuation_constant.color, "Ambient"  );
		is_modified |= Draw( point_light.data.diffuse_and_attenuation_linear.color,	  "Diffuse"  );
		is_modified |= Draw( point_light.data.specular_attenuation_quadratic.color,	  "Specular" );

		if( !hide_position )
			is_modified |= Draw( *point_light.transform, Transform::Mask::Translation, "Transform" );

		is_modified |= ImGui::SliderFloat( "Attenuation: Constant",	 &point_light.data.ambient_and_attenuation_constant.scalar,	0.0f, 5.0f, "%.5g", ImGuiSliderFlags_Logarithmic );
		is_modified |= ImGui::SliderFloat( "Attenuation: Linear",	 &point_light.data.diffuse_and_attenuation_linear.scalar,	0.0f, 1.0f, "%.5g", ImGuiSliderFlags_Logarithmic );
		is_modified |= ImGui::SliderFloat( "Attenuation: Quadratic", &point_light.data.specular_attenuation_quadratic.scalar,	0.0f, 1.0f, "%.5g", ImGuiSliderFlags_Logarithmic );

		ImGui::PopID();

		ImGuiUtility::EndGroupPanel( &point_light.is_enabled);
		
		return is_modified || was_enabled != point_light.is_enabled;
	}

	void Draw( const PointLight& point_light, const char* light_name, ImGuiWindowFlags window_flags )
	{
		bool dummy_enabled = point_light.is_enabled;
		ImGuiUtility::BeginGroupPanel( light_name, &dummy_enabled );

		ImGui::PushID( light_name );

		Draw( point_light.data.ambient_and_attenuation_constant.color, "Ambient"  );
		Draw( point_light.data.diffuse_and_attenuation_linear.color,   "Diffuse"  );
		Draw( point_light.data.specular_attenuation_quadratic.color,   "Specular" );

		Draw( const_cast< const Transform& >( *point_light.transform ), Transform::Mask::Translation, "Transform" );

		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::InputFloat( "Attenuation: Constant",	 const_cast< float* >( &point_light.data.ambient_and_attenuation_constant.scalar ), 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
		ImGui::InputFloat( "Attenuation: Linear",	 const_cast< float* >( &point_light.data.diffuse_and_attenuation_linear.scalar	 ), 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
		ImGui::InputFloat( "Attenuation: Quadratic", const_cast< float* >( &point_light.data.specular_attenuation_quadratic.scalar	 ), 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
		ImGui::PopStyleColor();

		ImGuiUtility::EndGroupPanel( &dummy_enabled );
		
		ImGui::PopID();
	}

	bool Draw( SpotLight& spot_light, const char* light_name, ImGuiWindowFlags window_flags )
	{
		bool is_modified = false;

		ImGuiUtility::BeginGroupPanel( light_name, &spot_light.is_enabled );

		ImGui::PushID( light_name );

		is_modified |= Draw( spot_light.data.ambient,  "Ambient"  );
		is_modified |= Draw( spot_light.data.diffuse,  "Diffuse"  );
		is_modified |= Draw( spot_light.data.specular, "Specular" );

		is_modified |= Draw( *spot_light.transform, BitFlags< Transform::Mask >( Transform::Mask::Translation, Transform::Mask::Rotation ), "Transform" );

		is_modified |= Draw( spot_light.data.cutoff_angle_inner, "Cutoff Angle: Inner", 0.0_deg,							spot_light.data.cutoff_angle_outer );
		is_modified |= Draw( spot_light.data.cutoff_angle_outer, "Cutoff Angle: Outer", spot_light.data.cutoff_angle_inner, 180.0_deg );

		ImGui::PopID();

		ImGuiUtility::EndGroupPanel( &spot_light.is_enabled );

		return is_modified;
	}

	void Draw( const SpotLight& spot_light, const char* light_name, ImGuiWindowFlags window_flags )
	{
		bool dummy_enabled = spot_light.is_enabled;
		ImGuiUtility::BeginGroupPanel( light_name );

		ImGui::PushID( light_name );

		Draw( spot_light.data.ambient,	"Ambient"  );
		Draw( spot_light.data.diffuse,	"Diffuse"  );
		Draw( spot_light.data.specular,	"Specular" );

		Draw( const_cast< const Transform& >( *spot_light.transform ), BitFlags< Transform::Mask >( Transform::Mask::Translation, Transform::Mask::Rotation ), "Transform" );

		Draw( spot_light.data.cutoff_angle_inner, "Cutoff Angle: Inner" );
		Draw( spot_light.data.cutoff_angle_outer, "Cutoff Angle: Outer"	);

		ImGui::PopID();
	
		ImGuiUtility::EndGroupPanel( &dummy_enabled );
	}
}
