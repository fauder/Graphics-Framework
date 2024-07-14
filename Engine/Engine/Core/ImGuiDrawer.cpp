// Engine Includes.
#include "ImGuiDrawer.hpp"
#include "ImGuiUtility.h"
#include "Graphics/ShaderTypeInformation.h"

namespace Engine::ImGuiDrawer
{
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
			/*case GL_FLOAT_MAT2 			: return Draw( *reinterpret_cast< Matrix2x2*	>( value_pointer ), name );
			case GL_FLOAT_MAT3 			: return Draw( *reinterpret_cast< Matrix3x3*	>( value_pointer ), name );
			case GL_FLOAT_MAT4 			: return Draw( *reinterpret_cast< Matrix4x4*	>( value_pointer ), name );
			case GL_FLOAT_MAT2x3 		: return Draw( *reinterpret_cast< Matrix2x3*	>( value_pointer ), name );
			case GL_FLOAT_MAT2x4 		: return Draw( *reinterpret_cast< Matrix2x4*	>( value_pointer ), name );
			case GL_FLOAT_MAT3x2 		: return Draw( *reinterpret_cast< Matrix3x2*	>( value_pointer ), name );
			case GL_FLOAT_MAT3x4 		: return Draw( *reinterpret_cast< Matrix3x4*	>( value_pointer ), name );
			case GL_FLOAT_MAT4x2 		: return Draw( *reinterpret_cast< Matrix4x2*	>( value_pointer ), name );
			case GL_FLOAT_MAT4x3 		: return Draw( *reinterpret_cast< Matrix4x3*	>( value_pointer ), name );*/
			case GL_FLOAT_MAT2 			: return false;
			case GL_FLOAT_MAT3 			: return false;
			case GL_FLOAT_MAT4 			: return false;
			case GL_FLOAT_MAT2x3 		: return false;
			case GL_FLOAT_MAT2x4 		: return false;
			case GL_FLOAT_MAT3x2 		: return false;
			case GL_FLOAT_MAT3x4 		: return false;
			case GL_FLOAT_MAT4x2 		: return false;
			case GL_FLOAT_MAT4x3 		: return false;

			/* Other: */
			case GL_SAMPLER_1D 			: return Draw( *reinterpret_cast< int*			>( value_pointer ), name );
			case GL_SAMPLER_2D 			: return Draw( *reinterpret_cast< int*			>( value_pointer ), name );
			case GL_SAMPLER_3D 			: return Draw( *reinterpret_cast< int*			>( value_pointer ), name );
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
			/*case GL_FLOAT_MAT2 			: return Draw( *reinterpret_cast< const Matrix2x2*		>( value_pointer ), name ); return;
			case GL_FLOAT_MAT3 			: return Draw( *reinterpret_cast< const Matrix3x3*		>( value_pointer ), name );
			case GL_FLOAT_MAT4 			: return Draw( *reinterpret_cast< const Matrix4x4*		>( value_pointer ), name );
			case GL_FLOAT_MAT2x3 		: return Draw( *reinterpret_cast< const Matrix2x3*		>( value_pointer ), name );
			case GL_FLOAT_MAT2x4 		: return Draw( *reinterpret_cast< const Matrix2x4*		>( value_pointer ), name );
			case GL_FLOAT_MAT3x2 		: return Draw( *reinterpret_cast< const Matrix3x2*		>( value_pointer ), name );
			case GL_FLOAT_MAT3x4 		: return Draw( *reinterpret_cast< const Matrix3x4*		>( value_pointer ), name );
			case GL_FLOAT_MAT4x2 		: return Draw( *reinterpret_cast< const Matrix4x2*		>( value_pointer ), name );
			case GL_FLOAT_MAT4x3 		: return Draw( *reinterpret_cast< const Matrix4x3*		>( value_pointer ), name );*/
			case GL_FLOAT_MAT2 			: return;
			case GL_FLOAT_MAT3 			: return;
			case GL_FLOAT_MAT4 			: return;
			case GL_FLOAT_MAT2x3 		: return;
			case GL_FLOAT_MAT2x4 		: return;
			case GL_FLOAT_MAT3x2 		: return;
			case GL_FLOAT_MAT3x4 		: return;
			case GL_FLOAT_MAT4x2 		: return;
			case GL_FLOAT_MAT4x3 		: return;

			/* Other: */
			case GL_SAMPLER_1D 			: return Draw( *reinterpret_cast< const int*			>( value_pointer ), name );
			case GL_SAMPLER_2D 			: return Draw( *reinterpret_cast< const int*			>( value_pointer ), name );
			case GL_SAMPLER_3D 			: return Draw( *reinterpret_cast< const int*			>( value_pointer ), name );
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

	bool Draw( float& scalar, const char* name )
	{
		return ImGui::DragFloat( name, &scalar );
	}

	void Draw( const float& scalar, const char* name )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::InputFloat( name, const_cast< float* >( &scalar ), 0.0f, 0.0f, GetFormat< float >(), ImGuiInputTextFlags_ReadOnly );
		ImGui::PopStyleColor();
	}

	bool Draw( double& scalar, const char* name )
	{
		return ImGui::DragScalar( name, GetImGuiDataType< double >(), &scalar );
	}

	void Draw( const double& scalar, const char* name )
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
		return ImGui::ColorEdit3( name, color.Data() );
	}

	void Draw( const Color3& color, const char* name )
	{
		/* Since the no inputs & no picker flags are passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::ColorEdit3( name, const_cast< float* >( color.Data() ), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker );
		ImGui::PopStyleColor();
	}

	bool Draw( Color4& color, const char* name )
	{
		return ImGui::ColorEdit4( name, color.Data() );
	}

	void Draw( const Color4& color, const char* name )
	{
		/* Since the no inputs & no picker flags are passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
		ImGui::ColorEdit4( name, const_cast< float* >( color.Data() ), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker );
		ImGui::PopStyleColor();
	}

	bool Draw( Transform& transform, const char* name )
	{
		bool is_modified = false;

		if( ImGui::Begin( "Transforms", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if( ImGui::TreeNodeEx( name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
			{
				ImGui::PushID( name );

				Vector3 translation = transform.GetTranslation();
				if( Draw( translation, "Position" ) )
				{
					is_modified = true;
					transform.SetTranslation( translation );
				}

				Quaternion rotation = transform.GetRotation();
				if( Draw( rotation, "Rotation" ) )
				{
					is_modified = true;
					transform.SetRotation( rotation );
				}

				Vector3 scale = transform.GetScaling();
				if( Draw( scale, "Scale" ) )
				{
					is_modified = true;
					transform.SetScaling( scale );
				}

				ImGui::PopID();

				ImGui::TreePop();
			}
		}

		ImGui::End();

		return is_modified;
	}

	void Draw( const Engine::Transform& transform, const char* name )
	{
		if( ImGui::Begin( "Transforms", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if( ImGui::TreeNodeEx( name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
			{
				ImGui::PushID( name );

				Draw( transform.GetTranslation(),	"Position"  );
				Draw( transform.GetRotation(),		"Rotation"  );
				Draw( transform.GetScaling(),		"Scale"		);

				ImGui::PopID();

				ImGui::TreePop();
			}
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

	void Draw( Material& material, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Materials", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if( ImGui::TreeNodeEx( material.GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
			{
				// TODO: Implement shader selection.
				if( material.HasShaderAssigned() )
				{
					const auto& shader_name( material.GetShaderName() );
					ImGui::TextColored( ImVec4( 0.38f, 0.12f, 0.68f, 1.0f ), "Shader: %s", material.GetShaderName().c_str() ); // Read-only for now.
				}
				else
					ImGui::TextUnformatted( "Shader: <unassigned>" );

				const auto& uniform_map = material.GetUniformInformations();

				for( auto& [ uniform_name, uniform_info ] : uniform_map )
				{
					/* Skip uniform struct members; They will be drawn under their parent struct name instead. */
					if( uniform_info.original_order_in_struct != -1 )
						continue;


					if( uniform_info.IsUserDefinedStruct() )
					{
						if( ImGui::TreeNodeEx( uniform_name.c_str()/*, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed*/ ) )
						{
							/* No need to update the Material when the Draw() call below returns true; Memory from the blob is provided directly to Draw(), so the Material is updated. */
							for( const auto& [ uniform_member_name, uniform_member_info ] : uniform_info.members )
								Draw( uniform_member_info->type, material.GetUniformPointer( uniform_member_info->original_offset ), uniform_member_name.c_str() );

							ImGui::TreePop();
						}
					}
					else
					{
						/* No need to update the Material when the Draw() call below returns true; Memory from the blob is provided directly to Draw(), so the Material is updated. */
						Draw( uniform_info.type, material.GetUniformPointer( uniform_info.offset ), uniform_name.c_str() );
					}
				}

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	void Draw( const Material& material, ImGuiWindowFlags window_flags )
	{
		if( ImGui::Begin( "Materials", nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if( ImGui::TreeNodeEx( material.GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
			{
				if( material.HasShaderAssigned() )
				{
					const auto& shader_name( material.GetShaderName() );
					ImGui::TextColored( ImVec4( 0.38f, 0.12f, 0.68f, 1.0f ), "Shader: %s", material.GetShaderName().c_str() );
				}
				else
					ImGui::TextUnformatted( "Shader: <unassigned>" );

				const auto& uniform_map = material.GetUniformInformations();

				for( auto& [ uniform_name, uniform_info ] : uniform_map )
				{
					/* Skip uniform struct members; They will be drawn under their parent struct name instead. */
					if( uniform_info.original_order_in_struct != -1 )
						continue;

					if( uniform_info.IsUserDefinedStruct() )
					{
						if( ImGui::TreeNodeEx( uniform_name.c_str()/*, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed*/ ) )
						{
							for( const auto& [ uniform_member_name, uniform_member_info ] : uniform_info.members )
								Draw( uniform_member_info->type, material.GetUniformPointer( uniform_member_info->original_offset ), uniform_member_name.c_str() );

							ImGui::TreePop();
						}
					}
					else
					{
						Draw( uniform_info.type, material.GetUniformPointer( uniform_info.original_offset ), uniform_name.c_str() );
					}
				}

				ImGui::TreePop();
			}
		}

		ImGui::End();
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
					ImGui::TableSetupColumn( "Name"		);
					ImGui::TableSetupColumn( "Location" );
					ImGui::TableSetupColumn( "Size"		);
					ImGui::TableSetupColumn( "Offset"	);
					ImGui::TableSetupColumn( "Type"		);

					ImGui::TableNextRow( ImGuiTableRowFlags_Headers ); // Indicates that the header row will be modified.
					ImGuiUtility::Table_Header_ManuallySubmit( std::array< int, 3 >{ 0, 2, 4 } );
					ImGuiUtility::Table_Header_ManuallySubmit_AppendHelpMarker( 1, "For uniform struct members, value in parenthesis shows the original location defined in the struct." );
					ImGuiUtility::Table_Header_ManuallySubmit_AppendHelpMarker( 3, "For uniform struct members, value in parenthesis shows the original offset of the member in the struct." );
					ImGui::TableNextRow(); // Done with the header row, skip to normal rows.

					ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

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

					ImGui::PopStyleColor();

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

			is_modified |= Draw( point_light_data.ambient_and_attenuation_constant.color,	"Ambient"  );
			is_modified |= Draw( point_light_data.diffuse_and_attenuation_linear.color,		"Diffuse"  );
			is_modified |= Draw( point_light_data.specular_attenuation_quadratic.color,		"Specular" );
			if( !hide_position )
				is_modified |= Draw( point_light_data.position_world_space, "Position" );
			is_modified |= ImGui::SliderFloat( "Attenuation: Constant",		&point_light_data.ambient_and_attenuation_constant.scalar,	0.0f, 5.0f, "%.5g" );
			is_modified |= ImGui::SliderFloat( "Attenuation: Linear",		&point_light_data.diffuse_and_attenuation_linear.scalar,	0.0f, 1.0f, "%.5g" );
			is_modified |= ImGui::SliderFloat( "Attenuation: Quadratic",	&point_light_data.specular_attenuation_quadratic.scalar,	0.0f, 1.0f, "%.5g" );

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

			Draw( point_light_data.ambient_and_attenuation_constant.color,  "Ambient"  );
			Draw( point_light_data.diffuse_and_attenuation_linear.color,	"Diffuse"  );
			Draw( point_light_data.specular_attenuation_quadratic.color,	"Specular" );
			Draw( point_light_data.position_world_space, "Position" );
			ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputFloat( "Attenuation: Constant",		const_cast< float* >( &point_light_data.ambient_and_attenuation_constant.scalar ), 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
			ImGui::InputFloat( "Attenuation: Linear",		const_cast< float* >( &point_light_data.diffuse_and_attenuation_linear.scalar	), 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
			ImGui::InputFloat( "Attenuation: Quadratic",	const_cast< float* >( &point_light_data.specular_attenuation_quadratic.scalar	), 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly );
			ImGui::PopStyleColor();

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

			is_modified |= ImGui::SliderInt( "Diffuse Map ID",	&surface_data.diffuse_map_slot,		0,		16 );
			is_modified |= ImGui::SliderInt( "Specular Map ID", &surface_data.specular_map_slot,	0,		16 );
			is_modified |= ImGui::SliderFloat( "Shininess",		&surface_data.shininess,			0.1f,	64.0f, "%.2f", ImGuiSliderFlags_Logarithmic );

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

			ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputInt( "Diffuse Map ID",  const_cast< int* >( &surface_data.diffuse_map_slot  ), 0, 0, ImGuiInputTextFlags_ReadOnly );
			ImGui::InputInt( "Specular Map ID", const_cast< int* >( &surface_data.specular_map_slot ), 0, 0, ImGuiInputTextFlags_ReadOnly );

			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputFloat( "Shininess", const_cast< float* >( &surface_data.shininess ), 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly );

			ImGui::PopStyleColor();

			ImGui::PopID();
		}

		ImGui::End();
	}
}
