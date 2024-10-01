#define IMGUI_DEFINE_MATH_OPERATORS

// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Core/AssetDatabase.hpp"
#include "Engine/Core/ImGuiDrawer.hpp"
#include "Engine/Core/ImGuiSetup.h"
#include "Engine/Core/ImGuiUtility.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Graphics/GLLogger.h"
#include "Engine/Graphics/MeshUtility.hpp"
#include "Engine/Graphics/Primitive/Primitive_Cube.h"
#include "Engine/Graphics/Primitive/Primitive_Quad.h"
#include "Engine/Graphics/Primitive/Primitive_Quad_FullScreen.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Random.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.
#include <fstream>

using namespace Engine::Math::Literals;

Engine::Application* Engine::CreateApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
{
    return new SandboxApplication( flags );
}

SandboxApplication::SandboxApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
	:
	Engine::Application( flags ),
	light_source_drawable_array( LIGHT_POINT_COUNT ),
	cube_drawable_array( CUBE_COUNT ),
	cube_drawable_outline_array( CUBE_COUNT ),
	phong_shader( "Phong" ),
	basic_color_shader( "Basic Color" ),
	basic_textured_shader( "Basic Textured" ),
	basic_textured_transparent_discard_shader( "Basic Textured (Discard Transparents)" ),
	outline_shader( "Outline" ),
	fullscreen_blit_shader( "Fullscreen Blit" ),
	postprocess_grayscale_shader( "Post-process: Grayscale" ),
	postprocess_generic_shader( "Post-process: Generic" ),
	light_point_transform_array( LIGHT_POINT_COUNT ),
	cube_transform_array( CUBE_COUNT),
	camera( &camera_transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) ),
	camera_rotation_speed( 5.0f ),
	camera_move_speed( 5.0f ),
	camera_controller( &camera, camera_rotation_speed ),
	auto_calculate_aspect_ratio( true ),
	auto_calculate_vfov_based_on_90_hfov( true ),
	ui_interaction_enabled( false ),
	show_imgui_demo_window( false ),
	draw_rear_view_cam_to_imgui( true )
{
	Initialize();
}

SandboxApplication::~SandboxApplication()	
{
	Shutdown();
}

void SandboxApplication::Initialize()
{
	renderer.SetClearColor( Engine::Color4::Gray() );
	renderer.SetClearTargets( Engine::Renderer::ClearTarget::All );

	Platform::ChangeTitle( "Sandbox (Graphics Framework)" );
	
	Engine::ServiceLocator< Engine::GLLogger >::Register( &gl_logger );

	gl_logger.IgnoreID( 131185 );

	//Engine::Math::Random::SeedRandom();

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox GL Init.", true /* omit if the group is empty */ ) );

/* Textures: */
	Engine::Texture::ImportSettings texture_import_settings;
	container_texture_diffuse_map  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Diffuse) Map",	R"(Asset/Texture/container2.png)",					texture_import_settings );
	container_texture_specular_map = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Specular) Map", R"(Asset/Texture/container2_specular.png)",			texture_import_settings );
	grass_texture                  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Grass",					R"(Asset/Texture/grass.png)",						texture_import_settings );
	transparent_window_texture     = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Transparent Window",		R"(Asset/Texture/blending_transparent_window.png)",	texture_import_settings );
	
	texture_import_settings.wrap_u = Engine::Texture::Wrapping::Repeat;
	texture_import_settings.wrap_v = Engine::Texture::Wrapping::Repeat;

	checker_pattern_texture = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Checkerboard Pattern (09)", R"(Asset/Texture/kenney_prototype/texture_09.png)", texture_import_settings );

	InitializeFramebufferTextures();

/* Renderbuffers: */
	InitializeRenderbuffers();

/* Framebuffers: */
	InitializeFramebuffers();

/* Shaders: */
	phong_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)" );
	basic_color_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/BasicColor.frag)" );
	basic_textured_shader.FromFile( R"(Asset/Shader/BasicTextured.vert)", R"(Asset/Shader/BasicTextured.frag)" );
	basic_textured_transparent_discard_shader.FromFile( R"(Asset/Shader/BasicTextured.vert)", R"(Asset/Shader/BasicTextured.frag)", { "DISCARD_TRANSPARENT_FRAGMENTS" } );
	outline_shader.FromFile( R"(Asset/Shader/Outline.vert)", R"(Asset/Shader/BasicColor.frag)" );
	fullscreen_blit_shader.FromFile( R"(Asset/Shader/FullScreenBlit.vert)", R"(Asset/Shader/BasicTextured.frag)" );
	postprocess_grayscale_shader.FromFile( R"(Asset/Shader/FullScreenBlit.vert)", R"(Asset/Shader/Grayscale.frag)" );
	postprocess_generic_shader.FromFile( R"(Asset/Shader/FullScreenBlit.vert)", R"(Asset/Shader/GenericPostprocess.frag)" );

	/* Register shaders not assigned to materials: */
	renderer.RegisterShader( postprocess_grayscale_shader );
	renderer.RegisterShader( postprocess_generic_shader );

/* Initial transforms: */
	ground_transform
		.SetScaling( 30.0f, 60.0f, 1.0f )
		.SetRotation( 0.0_deg, 90.0_deg, 0.0_deg );

	wall_front_transform
		.SetScaling( 30.0f, 15.0f, 1.0f )
		.SetTranslation( 0.0f, 7.5, 30.0f );
	wall_left_transform
		.SetScaling( 60.0f, 15.0f, 1.0f )
		.SetRotation( -90.0_deg, 0.0_deg, 0.0_deg )
		.SetTranslation( -15.0f, 7.5f, 0.0f );
	wall_right_transform
		.SetScaling( 60.0f, 15.0f, 1.0f )
		.SetRotation( +90.0_deg, 0.0_deg, 0.0_deg )
		.SetTranslation( +15.0f, 7.5f, 0.0f );
	wall_back_transform
		.SetScaling( 30.0f, 15.0f, 1.0f )
		.SetRotation( 180.0_deg, 0.0_deg, 0.0_deg )
		.SetTranslation( 0.0f, 7.5f, -30.0f );

	for( auto cube_index = 0; cube_index < CUBE_COUNT; cube_index++ )
	{
		Degrees angle( 20.0f * cube_index );
		cube_transform_array[ cube_index ]
			.SetRotation( Quaternion( angle, Vector3{ 1.0f, 0.3f, 0.5f }.Normalized() ) )
			.SetTranslation( CUBE_POSITIONS[ cube_index ] + Vector3::Up() * 5.0f );
	}

	grass_quad_transform_array[ 0 ].SetTranslation( Vector3( -1.5f,	5.0f, -0.48f + 0.01f ) ); // + 0.01f to keep the grass behind the window.
	grass_quad_transform_array[ 1 ].SetTranslation( Vector3(  1.5f,	5.0f,  0.51f + 0.01f ) ); // + 0.01f to keep the grass behind the window.
	grass_quad_transform_array[ 2 ].SetTranslation( Vector3(  0.0f,	5.0f,  0.7f  + 0.01f ) ); // + 0.01f to keep the grass behind the window.
	grass_quad_transform_array[ 3 ].SetTranslation( Vector3( -0.3f,	5.0f, -2.3f  + 0.01f ) ); // + 0.01f to keep the grass behind the window.
	grass_quad_transform_array[ 4 ].SetTranslation( Vector3(  0.5f,	5.0f, -0.6f  + 0.01f ) ); // + 0.01f to keep the grass behind the window.

	window_transform_array[ 0 ].SetTranslation( Vector3( -1.5f,	5.0f, -0.48f ) );
	window_transform_array[ 1 ].SetTranslation( Vector3(  1.5f,	5.0f,  0.51f ) );
	window_transform_array[ 2 ].SetTranslation( Vector3(  0.0f,	5.0f,  0.7f  ) );
	window_transform_array[ 3 ].SetTranslation( Vector3( -0.3f,	5.0f, -2.3f  ) );
	window_transform_array[ 4 ].SetTranslation( Vector3(  0.5f,	5.0f, -0.6f  ) );

/* Vertex/Index Data: */
	cube_mesh = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Cube::Positions.cbegin(), Engine::Primitive::NonIndexed::Cube::Positions.cend() ),
							  "Cube",
							  std::vector< Vector3 >( Engine::Primitive::NonIndexed::Cube::Normals.cbegin(), Engine::Primitive::NonIndexed::Cube::Normals.cend() ),
							  std::vector< Vector2 >( Engine::Primitive::NonIndexed::Cube::UVs.cbegin(), Engine::Primitive::NonIndexed::Cube::UVs.cend() ),
							  { /* No indices. */ } );

	quad_mesh_uvs_only = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Quad::Positions.cbegin(), Engine::Primitive::NonIndexed::Quad::Positions.cend() ),
									   "Quad (UVs Only)",
									   { /* No normals. */ },
									   std::vector< Vector2 >( Engine::Primitive::NonIndexed::Quad::UVs.cbegin(), Engine::Primitive::NonIndexed::Quad::UVs.cend() ),
									   { /* No indices. */ } );

	quad_mesh = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Quad::Positions.cbegin(), Engine::Primitive::NonIndexed::Quad::Positions.cend() ),
							  "Quad",
							  std::vector< Vector3 >( Engine::Primitive::NonIndexed::Quad::Normals.cbegin(), Engine::Primitive::NonIndexed::Quad::Normals.cend() ),
							  std::vector< Vector2 >( Engine::Primitive::NonIndexed::Quad::UVs.cbegin(), Engine::Primitive::NonIndexed::Quad::UVs.cend() ),
							  { /* No indices. */ } );

	quad_mesh_fullscreen = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Quad_FullScreen::Positions.cbegin(), Engine::Primitive::NonIndexed::Quad_FullScreen::Positions.cend() ),
										 "Quad (FullScreen)",
										 { /* No normals. */ },
										 std::vector< Vector2 >( Engine::Primitive::NonIndexed::Quad_FullScreen::UVs.cbegin(), Engine::Primitive::NonIndexed::Quad_FullScreen::UVs.cend() ),
										 { /* No indices. */ } );


	constexpr std::array< Vector3, 6 > quad_mesh_positions_ndc
	( {
		{ -0.425f, 0.35f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{  0.425f, 0.35f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{  0.425f, 0.80f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{  0.425f, 0.80f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{ -0.425f, 0.80f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{ -0.425f, 0.35f, -0.01f }  // A little z offset backwards to make sure this is rendered in front & shows.
	} );

	quad_mesh_mirror = Engine::Mesh( std::vector< Vector3 >( quad_mesh_positions_ndc.cbegin(), quad_mesh_positions_ndc.cend() ),
									 "Quad (Rear-view mirror)",
									 { /* No normals. */ },
									 std::vector< Vector2 >( Engine::Primitive::NonIndexed::Quad_FullScreen::UVs.cbegin(), Engine::Primitive::NonIndexed::Quad_FullScreen::UVs.cend() ),
									 { /* No indices. */ } );

/* Lighting: */
	ResetLightingData();

	renderer.AddDirectionalLight( &light_directional );

	for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
		renderer.AddPointLight( &light_point_array[ index ] );

	renderer.AddSpotLight( &light_spot );

/* Materials: */
	ResetMaterialData();

/* Renderer (Drawables, RenderStates etc.): */
	{
		renderer.SetRenderGroupName( Engine::Renderer::RenderGroupID( 0 ), "Default" );
		auto& render_state_regular_meshes = renderer.GetRenderState( Engine::Renderer::RenderGroupID( 0 ) ); // Keep default settings.
	}
	
	{
		/* This pass draws the mesh semi-regularly; It also marks the stencil buffer with 1s everywhere the mesh is drawn at. */
		renderer.SetRenderGroupName( Engine::Renderer::RenderGroupID( 1 ), "Outlined Meshes" );
		auto& render_state_outline_meshes = renderer.GetRenderState( Engine::Renderer::RenderGroupID( 1 ) );

		/* Omitted settings are left to defaults. */

		render_state_outline_meshes.stencil_test_enable                           = true;
		render_state_outline_meshes.stencil_write_mask                            = 0xFF;
		render_state_outline_meshes.stencil_comparison_function                   = Engine::Renderer::ComparisonFunction::Always;
		render_state_outline_meshes.stencil_ref                                   = 0x01;
		render_state_outline_meshes.stencil_test_response_stencil_fail            = Engine::Renderer::StencilTestResponse::Keep;
		render_state_outline_meshes.stencil_test_response_stencil_pass_depth_fail = Engine::Renderer::StencilTestResponse::Keep;
		render_state_outline_meshes.stencil_test_response_both_pass               = Engine::Renderer::StencilTestResponse::Replace;
	}

	{
		/* This pass draws the outlines only; It does this by rendering the mesh at everywhere the stencil buffer is NOT 1. */
		renderer.SetRenderGroupName( Engine::Renderer::RenderGroupID( 2 ), "Outlines" );
		auto& render_state_outlines = renderer.GetRenderState( Engine::Renderer::RenderGroupID( 2 ) );

		/* Omitted settings are left to defaults. */

		render_state_outlines.depth_test_enable = false;

		render_state_outlines.stencil_test_enable         = true;
		render_state_outlines.stencil_write_mask          = 0x00; // Disable writes; This pass only needs to READ the stencil buffer, to figure out where NOT to render.
		render_state_outlines.stencil_comparison_function = Engine::Renderer::ComparisonFunction::NotEqual; // Render everywhere that's not the actual mesh, i.e., the border.
		render_state_outlines.stencil_ref                 = 0x01;
	}

	{
		/* This pass draws the transparent meshes. */
		renderer.SetRenderGroupName( Engine::Renderer::RenderGroupID( 3 ), "Transparent Meshes" );
		auto& render_state_transparent = renderer.GetRenderState( Engine::Renderer::RenderGroupID( 3 ) );

		/* Omitted settings are left to defaults. */

		render_state_transparent.blending_enable = true;

		render_state_transparent.blending_source_color_factor      = Engine::Renderer::BlendingFactor::SourceAlpha;
		render_state_transparent.blending_destination_color_factor = Engine::Renderer::BlendingFactor::OneMinusSourceAlpha;
		render_state_transparent.blending_source_alpha_factor      = Engine::Renderer::BlendingFactor::SourceAlpha;
		render_state_transparent.blending_destination_alpha_factor = Engine::Renderer::BlendingFactor::OneMinusSourceAlpha;

		render_state_transparent.sorting_mode = Engine::Renderer::SortingMode::DepthFarthestToNearest;
	}

	{
		renderer.SetRenderGroupName( Engine::Renderer::RenderGroupID( 4 ), "Screen-size Quad" );
		auto& render_state_screen_size_quad = renderer.GetRenderState( Engine::Renderer::RenderGroupID( 4 ) ); // Keep default settings.
	}

	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_source_drawable_array[ i ] = Engine::Drawable( &cube_mesh, &light_source_material_array[ i ], &light_point_transform_array[ i ] );
		renderer.AddDrawable( &light_source_drawable_array[ i ], Engine::Renderer::RenderGroupID( 0 ) );
	}

	for( auto i = 0; i < CUBE_COUNT; i++ )
	{
		cube_drawable_array[ i ] = Engine::Drawable( &cube_mesh, &cube_material_array[ i ], &cube_transform_array[ i ] );
		renderer.AddDrawable( &cube_drawable_array[ i ], Engine::Renderer::RenderGroupID( 1 ) );

		cube_drawable_outline_array[ i ] = Engine::Drawable( &cube_mesh, &outline_material, &cube_transform_array[ i ] );
		renderer.AddDrawable( &cube_drawable_outline_array[ i ], Engine::Renderer::RenderGroupID( 2 ) );
	}

	ground_drawable = Engine::Drawable( &quad_mesh, &ground_material, &ground_transform );
	renderer.AddDrawable( &ground_drawable, Engine::Renderer::RenderGroupID( 0 ) );

	wall_front_drawable = Engine::Drawable( &quad_mesh, &wall_material, &wall_front_transform );
	wall_left_drawable  = Engine::Drawable( &quad_mesh, &wall_material, &wall_left_transform );
	wall_right_drawable = Engine::Drawable( &quad_mesh, &wall_material, &wall_right_transform );
	wall_back_drawable  = Engine::Drawable( &quad_mesh, &wall_material, &wall_back_transform );
	renderer.AddDrawable( &wall_front_drawable, Engine::Renderer::RenderGroupID( 0 ) );
	renderer.AddDrawable( &wall_left_drawable,  Engine::Renderer::RenderGroupID( 0 ) );
	renderer.AddDrawable( &wall_right_drawable, Engine::Renderer::RenderGroupID( 0 ) );
	renderer.AddDrawable( &wall_back_drawable,  Engine::Renderer::RenderGroupID( 0 ) );

	/*for( auto i = 0; i < GRASS_COUNT; i++ )
	{
		grass_quad_drawable_array[ i ] = Engine::Drawable( &quad_mesh_uvs_only, &grass_quad_material, &grass_quad_transform_array[ i ] );
		renderer.AddDrawable( &grass_quad_drawable_array[ i ], Engine::Renderer::RenderGroupID( 0 ) );
	}*/

	for( auto i = 0; i < WINDOW_COUNT; i++ )
	{
		window_drawable_array[ i ] = Engine::Drawable( &quad_mesh_uvs_only, &window_material, &window_transform_array[ i ] );
		renderer.AddDrawable( &window_drawable_array[ i ], Engine::Renderer::RenderGroupID( 3 ) );
	}

	offscreen_quad_drawable = Engine::Drawable( &quad_mesh_fullscreen, &offscreen_quad_material );
	renderer.AddDrawable( &offscreen_quad_drawable, Engine::Renderer::RenderGroupID( 4 ) );

	mirror_quad_drawable = Engine::Drawable( &quad_mesh_mirror, &mirror_quad_material );
	renderer.AddDrawable( &mirror_quad_drawable, Engine::Renderer::RenderGroupID( 4 ) );

	mirror_quad_drawable.ToggleOnOrOff( not draw_rear_view_cam_to_imgui );

	/* This is the earliest place we can MaximizeWindow() at,
	 * because the Renderer will populate its Intrinsic UBO info only upon AddDrawable( <Drawable with a Shader using said UBO> ). */
	ResetCamera();
	Platform::MaximizeWindow();

/* Models: */
	if( auto config_file = std::ifstream( "config.ini" ) )
	{
		std::string ignore;
		config_file >> ignore;
		if( ignore == "test_model_path" )
		{
			config_file >> ignore /* '=' */ >> test_model_file_path;

			if( !test_model_file_path.empty() )
				ReloadModel( test_model_file_path );
		}
	}

/* Other: */
	renderer.SetFrontFaceConvention( Engine::Renderer::WindingOrder::CounterClockwise );
	renderer.EnableFaceCulling( Engine::Renderer::Face::Back );
}

void SandboxApplication::Shutdown()
{
	if( auto config_file = std::ofstream( "config.ini" ) )
	{
		config_file << "test_model_path = " << test_model_file_path;
	}
}

//void SandboxApplication::Run()
//{
//
//}

void SandboxApplication::Update()
{
	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Update", true /* omit if the group is empty */ ) );

	current_time_as_angle = Radians( time_current );
	const Radians current_time_mod_two_pi( std::fmod( time_current, Engine::Constants< float >::Two_Pi() ) );

	/* Light sources' transform: */
	constexpr Radians angle_increment( Engine::Constants< Radians >::Two_Pi() / LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		/* Light source transform: */
		Vector3 point_light_position_world_space;

		if( light_point_array_is_animated )
		{
			const auto old_rotation( light_point_transform_array[ i ].GetRotation() );
			Radians old_heading, old_pitch, bank;
			Engine::Math::QuaternionToEuler( old_rotation, old_heading, old_pitch, bank );

			Radians new_angle( current_time_mod_two_pi + ( float )angle_increment * ( float )i );
			const Radians new_angle_mod_two_pi( std::fmod( new_angle.Value(), Engine::Constants< float >::Two_Pi() ) );

			const bool heading_instead_of_pitching = new_angle_mod_two_pi <= Engine::Constants< Radians >::Pi();

			const auto point_light_rotation( Engine::Math::EulerToMatrix( float(  heading_instead_of_pitching ) * new_angle_mod_two_pi,
																		  float( !heading_instead_of_pitching ) * ( new_angle_mod_two_pi - 3.0f * Engine::Constants< Radians >::Pi_Over_Two() ), bank ) );

			point_light_position_world_space = ( ( ( heading_instead_of_pitching ? Vector4::Forward() : Vector4::Up() ) * light_point_orbit_radius ) *
												 ( point_light_rotation * Engine::Matrix::Translation( CUBES_ORIGIN ) ) ).XYZ();
			point_light_position_world_space.SetY( point_light_position_world_space.Y() + 2.0f );

			light_point_transform_array[ i ].SetRotation( Engine::Math::MatrixToQuaternion( point_light_rotation ) );
		}

		light_point_transform_array[ i ].SetTranslation( point_light_position_world_space );
	}

	/* Camera transform: */
	if( camera_is_animated )
	{
		/* Orbit motion: */

		Engine::Math::Vector< Radians, 3 > old_euler_angles;
		Engine::Math::QuaternionToEuler( camera_transform.GetRotation(), old_euler_angles );
		// Don't modify X & Z euler angles; Allow the user to modify them.
		camera_transform.SetRotation( Engine::Math::EulerToQuaternion( -current_time_as_angle * 0.33f, old_euler_angles.X(), old_euler_angles.Z() ) );

		auto new_pos = CUBES_ORIGIN + -camera_transform.Forward() * 30.0f;
		new_pos.SetY( camera_transform.GetTranslation().Y() ); // Don't modify Y position; Allow the user to modify it.
		camera_transform.SetTranslation( new_pos );
	}
	else
	{
		if( !ui_interaction_enabled )
		{
			// Control via mouse:
			const auto [ mouse_x_delta_pos, mouse_y_delta_pos ] = Platform::GetMouseCursorDeltas();
			camera_controller
				.OffsetHeading( Radians( +mouse_x_delta_pos ) )
				.OffsetPitch( Radians( +mouse_y_delta_pos ), -Engine::Constants< Radians >::Pi_Over_Two(), Engine::Constants< Radians >::Pi_Over_Two() );
		}
	}

	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_W ) )
		camera_transform.OffsetTranslation( camera_transform.Forward() * +camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_S ) )
		camera_transform.OffsetTranslation( camera_transform.Forward() * -camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_A ) )
		camera_transform.OffsetTranslation( camera_transform.Right()   * -camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_D ) )
		camera_transform.OffsetTranslation( camera_transform.Right()   * +camera_move_speed * time_delta );
}

void SandboxApplication::Render()
{
	Engine::Application::Render();

	/* Pass 1 - Rear-cam view: Invert camera direction, draw everything to the off-screen frame-buffer 1. */
	{
		auto log_group( gl_logger.TemporaryLogGroup( "SandboxApp::Render(): Render to Offscreen FB 1 (Rear view)", true /* omit if the group is empty */ ) );

		camera_controller.Invert();
		renderer.Update( camera );

		renderer.SetCurrentFramebuffer( &offscreen_framebuffer_array[ 0 ] );

		renderer.Render( camera, { 0, 1, 2, 3 } );

		/* Now that the off-screen framebuffer is filled, we should create mip-maps of it so that it can be mapped onto smaller surfaces: */
		offscreen_framebuffer_color_attachment_array[ 0 ]->GenerateMipmaps();
	}

	/* Pass 2 - Default view: Invert camera direction again (to revert to default view), draw everything to the off-screen frame-buffer 2. */
	{
		auto log_group( gl_logger.TemporaryLogGroup( "SandboxApp::Render(): Render to Offscreen FB 2 (Default view)", true /* omit if the group is empty */ ) );
		
		camera_controller.Invert();
		renderer.Update( camera );

		renderer.SetCurrentFramebuffer( &offscreen_framebuffer_array[ 1 ] );

		renderer.Render( camera, { 0, 1, 2, 3 } );

		/* Now that the off-screen framebuffer is filled, we should create mip-maps of it so that it can be mapped onto smaller surfaces: */
		offscreen_framebuffer_color_attachment_array[ 1 ]->GenerateMipmaps();
	}

	/* Pass 3: Blit both off-screen frame-buffers to quads on default frame-buffer. */
	{
		auto log_group( gl_logger.TemporaryLogGroup( "SandboxApp::Render(): Blit Offscreen FB(s) to Main FB", true /* omit if the group is empty */ ) );

		renderer.ResetToDefaultFramebuffer();

		renderer.Render( camera, { 4 } );
	}
}

void SandboxApplication::RenderImGui()
{
	{
		auto log_group( gl_logger.TemporaryLogGroup( "Application ImGui", true /* omit if the group is empty */ ) );

		Application::RenderImGui();
	}

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox ImGui", true /* omit if the group is empty */ ) );

	if( show_imgui_demo_window )
		ImGui::ShowDemoWindow();

	const auto& style = ImGui::GetStyle();

	{
		const auto [ width, height ] = Platform::GetFramebufferSizeInPixels();
		static auto rear_view_window_size = ImVec2( width / 4.0f, height / 4.0f );
		ImGui::SetNextWindowSize( ImVec2( width / 4.0f, height / 4.0f ), ImGuiCond_FirstUseEver );
	}
	if( ImGui::Begin( "Rear-view Camera", nullptr, ImGuiWindowFlags_NoScrollbar ) )
	{
		if( ImGui::Checkbox( "Draw to this window instead of default Framebuffer", &draw_rear_view_cam_to_imgui ) )
			mirror_quad_drawable.ToggleOnOrOff( not draw_rear_view_cam_to_imgui );

		if( draw_rear_view_cam_to_imgui )
		{
			const ImVec2 negative_size_offset( style.WindowPadding * 2 );
			ImGui::Image( ( void* )( intptr_t )offscreen_framebuffer_color_attachment_array[ 0 ]->Id(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );
		}
	}

	ImGui::End();

	if( ImGui::Begin( ICON_FA_CUBES " Models", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		bool load_button_is_clicked = false;

		if( not test_model_file_path.empty() )
		{
			static char buffer[ 260 ];
			strncpy_s( buffer, test_model_file_path.c_str(), test_model_file_path.size());
			ImGui::BeginDisabled();
			ImGui::InputText( "Loaded Model Path", buffer, ( int )test_model_file_path.size(), ImGuiInputTextFlags_ReadOnly);
			ImGui::EndDisabled();

			if( ImGui::Button( ICON_FA_FOLDER_OPEN " Reload" ) )
			{
				if( auto maybe_file_name = Platform::BrowseFileName( {	"glTF (*.gltf;*.glb)",		"*.gltf;*.glb",	
																		"Standard glTF (*.gltf)",	"*.gltf",
																		"Binary glTF (*.glb)",		"*.glb" },
																	 "Choose a Model to Load" );
					maybe_file_name.has_value() && *maybe_file_name != test_model_file_path )
				{
					ReloadModel( *maybe_file_name );
				}
			}
			ImGui::SameLine();
			if( ImGui::Button( ICON_FA_XMARK " Unload" ) )
				UnloadModel();
		}
		else
		{
			const auto button_size( ImGui::CalcTextSize( ICON_FA_CUBES " Models   " ) + style.ItemInnerSpacing );
			if( ImGui::Button( ICON_FA_FOLDER_OPEN " Load", button_size ) )
			{
				if( auto maybe_file_name = Platform::BrowseFileName( {	"glTF (*.gltf;*.glb)",		"*.gltf;*.glb",	
																		"Standard glTF (*.gltf)",	"*.gltf",
																		"Binary glTF (*.glb)",		"*.glb" },
																	 "Choose a Model to Load" );
					maybe_file_name.has_value() && *maybe_file_name != test_model_file_path )
				{
					ReloadModel( *maybe_file_name );
				}
			}
		}
	}

	ImGui::End();

	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		Engine::ImGuiDrawer::Draw( light_source_material_array[ i ], renderer );
	Engine::ImGuiDrawer::Draw( ground_material, renderer );
	Engine::ImGuiDrawer::Draw( wall_material, renderer );
	Engine::ImGuiDrawer::Draw( grass_quad_material, renderer );
	Engine::ImGuiDrawer::Draw( window_material, renderer );
	for( auto& cube_material : cube_material_array )
		Engine::ImGuiDrawer::Draw( cube_material, renderer );
	for( auto& test_material : test_model_instance.Materials() )
		Engine::ImGuiDrawer::Draw( const_cast< Engine::Material& >( test_material ), renderer );
	Engine::ImGuiDrawer::Draw( outline_material, renderer );
	Engine::ImGuiDrawer::Draw( offscreen_quad_material, renderer );
	Engine::ImGuiDrawer::Draw( mirror_quad_material, renderer );

	if( ImGui::Begin( ICON_FA_LIGHTBULB " Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Checkbox( "Enabled##AllLights", &light_is_enabled ) )
		{
			light_directional.is_enabled = light_is_enabled;

			for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
				light_source_drawable_array[ i ].ToggleOnOrOff( light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable );

			light_spot.is_enabled = light_is_enabled;
		}

		ImGui::SameLine( 0.0f, 20.0f );

		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Lights" ) )
			ResetLightingData();

		ImGui::NewLine();

		if( ImGui::BeginTabBar( "Lights Tab Bar", ImGuiTabBarFlags_DrawSelectedOverline | ImGuiTabBarFlags_NoTabListScrollingButtons ) )
		{
			if( ImGui::BeginTabItem( "Directional Light" ) )
			{
				Engine::ImGuiDrawer::Draw( light_directional, "Directional Light" );
				ImGui::EndTabItem();
			}

			if( ImGui::BeginTabItem( "Point Lights" ) )
			{
				Engine::ImGuiUtility::BeginGroupPanel( "Options" );
				if( ImGui::Checkbox( "Disable All", &light_point_array_disable ) )
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
						light_source_drawable_array[ i ].ToggleOnOrOff( light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable );
				ImGui::Checkbox( "Animate (Orbit) Point Lights", &light_point_array_is_animated );
				if( light_point_array_is_animated )
					ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 15.0f );
				Engine::ImGuiUtility::EndGroupPanel();

				if( ImGui::TreeNodeEx( "Point Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						const bool was_enabled = light_point_array[ i ].is_enabled;
						if( Engine::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str(), light_point_array_is_animated /* hide position. */ ) )
						{
							light_source_material_array[ i ].Set( "uniform_color", Engine::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f ) );
							if( was_enabled != light_point_array[ i ].is_enabled )
								light_source_drawable_array[ i ].ToggleOnOrOff();
						}
					}

					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			if( ImGui::BeginTabItem( "Spot Lights" ) )
			{
				if( ImGui::TreeNodeEx( "Spot Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					Engine::ImGuiDrawer::Draw( light_spot, "Spot Light" );

					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();

	if( ImGui::Begin( ICON_FA_VIDEO " Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
	/* Row 1: */
		const float button_width( ImGui::CalcTextSize( ICON_FA_ARROWS_ROTATE " XXXXXX" ).x + style.ItemInnerSpacing.x );
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Top" ) )
			ResetCamera( CameraView::TOP );

	/* Row 2: */
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Left" ) )
			ResetCamera( CameraView::LEFT );
		ImGui::SameLine();
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Front" ) )
			ResetCamera( CameraView::FRONT );
		ImGui::SameLine();
		ImGui::SetCursorPosX( button_width * 2 );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Right" ) )
			ResetCamera( CameraView::RIGHT );

	/* Row 3: */
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Back" ) )
			ResetCamera( CameraView::BACK );

	/* Row 4: */
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Bottom" ) )
			ResetCamera( CameraView::BOTTOM );


		ImGui::NewLine();

	/* Row 5: */
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Custom (1)" ) )
			ResetCamera( CameraView::CUSTOM_1 );

		ImGui::Checkbox( "Animate (Rotate) Camera", &camera_is_animated );
		Engine::ImGuiDrawer::Draw( camera_transform, Engine::Transform::Mask::NoScale, "Main Camera" );

		if( ImGui::CollapsingHeader( "Projection" ) )
		{
			Engine::ImGuiUtility::BeginGroupPanel();
			{
				if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset" ) )
				{
					auto_calculate_aspect_ratio = true;
					auto_calculate_vfov_based_on_90_hfov = true;

					camera.SetAspectRatio( Platform::GetAspectRatio() ); // If we don't do this, CalculateVerticalFieldOfView() below will work with possibly the old aspect ratio and produce incorrect results.
					camera = Engine::Camera( &camera_transform, camera.GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );
				}

				bool modified = false;

				modified |= Engine::ImGuiDrawer::Draw( camera, "Main Camera" );

				float v_fov_radians = ( float )camera.GetVerticalFieldOfView();

				modified |= ImGui::Checkbox( "Auto-calculate Aspect Ratio", &auto_calculate_aspect_ratio );
				modified |= ImGui::Checkbox( "Auto-calculate Vertical FoV to match 90 degrees Horizontal FoV", &auto_calculate_vfov_based_on_90_hfov );

				if( modified )
				{
					if( auto_calculate_aspect_ratio )
						camera.SetAspectRatio( Platform::GetAspectRatio() );

					if( auto_calculate_vfov_based_on_90_hfov )
						camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );
					else
						camera.SetVerticalFieldOfView( Radians( v_fov_radians ) );

					renderer.OnProjectionParametersChange( camera );
				}
			}
			Engine::ImGuiUtility::EndGroupPanel();
		}
	}

	ImGui::End();

	Engine::ImGuiDrawer::Draw( Engine::AssetDatabase< Engine::Texture >::Assets(), { 400.0f, 512.0f } );

	renderer.RenderImGui();
}

void SandboxApplication::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
{
	switch( key_code )
	{
		case Platform::KeyCode::KEY_ESCAPE:
			if( key_action == Platform::KeyAction::PRESS )
				Platform::SetShouldClose( true );
			break;
		/* Use the key below ESC to toggle between game & menu/UI. */
		case Platform::KeyCode::KEY_GRAVE_ACCENT:
			if( key_action == Platform::KeyAction::PRESS )
			{
				ui_interaction_enabled = !ui_interaction_enabled;
				Platform::ResetMouseDeltas();
			}
			break;
		case Platform::KeyCode::KEY_W:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Forward() * +camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_S:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Forward() * -camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_A:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Right() * -camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_D:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Right() * +camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_U:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				light_spot.data.cutoff_angle_inner = Engine::Math::Min( light_spot.data.cutoff_angle_inner + 0.33_deg, light_spot.data.cutoff_angle_outer );
			break;
		case Platform::KeyCode::KEY_Y:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				light_spot.data.cutoff_angle_inner = Engine::Math::Max( light_spot.data.cutoff_angle_inner - 0.33_deg, 0_deg );
			break;
		case Platform::KeyCode::KEY_I:
			if( key_action == Platform::KeyAction::PRESS )
				show_imgui = !show_imgui;
			break;
		case Platform::KeyCode::KEY_O:
			if( key_action == Platform::KeyAction::PRESS )
				show_imgui_demo_window = !show_imgui_demo_window;
			break;
		default:
			break;
	}
}

void SandboxApplication::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
{
	/* Do nothing on minimize: */
	if( width_new_pixels == 0 || height_new_pixels == 0 )
		return;

	renderer.OnFramebufferResize( width_new_pixels, height_new_pixels );

	// Re-calculate the aspect ratio:
	if( auto_calculate_aspect_ratio )
	{
		camera.SetAspectRatio( float( width_new_pixels ) / height_new_pixels );
		if( auto_calculate_vfov_based_on_90_hfov )
			camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );

		renderer.OnProjectionParametersChange( camera );
	}

	// Re-initialize:
	InitializeFramebufferTextures();
	InitializeRenderbuffers();
	InitializeFramebuffers();

	mirror_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 0 ] );
	offscreen_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 1 ] );
}

void SandboxApplication::ResetLightingData()
{
	light_directional = 
	{
		.is_enabled = true,
		.data =
		{
			.ambient  = Engine::Color3{  0.05f,  0.05f,  0.05f },
			.diffuse  = Engine::Color3{  0.4f,   0.4f,   0.4f  },
			.specular = Engine::Color3{  0.5f,   0.5f,   0.5f  },
		},
		.transform = &light_directional_transform
	};

	light_is_enabled = true;

	light_point_array_disable      = false;
	light_point_array_is_animated  = true;
	light_point_orbit_radius       = 13.25f;

	light_spot_array_disable = false;

	light_point_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_point_array[ i ] =
		{
			.is_enabled = true,
			.data =
			{
				.ambient_and_attenuation_constant = {.color = {  0.05f,  0.05f,  0.05f },							.scalar = 0.075f	},
				.diffuse_and_attenuation_linear   = {.color = Engine::Math::Random::Generate< Engine::Color3 >(),	.scalar = 0.0035f	},
				.specular_attenuation_quadratic   = {.color = {  1.0f,   1.0f,   1.0f  },							.scalar = 0.09f },
			},
			.transform = &light_point_transform_array[ i ]
		};
	}

	light_spot =
	{
		.is_enabled = true,
		.data =
		{
			.ambient  = Engine::Color3{  0.05f,  0.05f,  0.03f },
			.diffuse  = Engine::Color3{  0.4f,   0.4f,   0.27f  },
			.specular = Engine::Color3{  0.5f,   0.5f,   0.33f  },

		/* End of GLSL equivalence. */
			.cutoff_angle_inner = 12.5_deg,
			.cutoff_angle_outer = 17.5_deg
		},
		.transform = &light_spot_transform
	};
}

void SandboxApplication::ResetMaterialData()
{
	light_source_material_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ] = Engine::Material( "Light Source #" + std::to_string( i + 1 ), &basic_color_shader );
	
	cube_material_array.resize( CUBE_COUNT );
	for( auto i = 0; i < CUBE_COUNT; i++ )
	{
		cube_material_array[ i ] = Engine::Material( "Cube #" + std::to_string( i + 1 ), &phong_shader );
		cube_material_array[ i ].SetTexture( "uniform_diffuse_map_slot", container_texture_diffuse_map );
		cube_material_array[ i ].SetTexture( "uniform_specular_map_slot", container_texture_specular_map );
		cube_material_array[ i ].Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );
	}

	ground_material = Engine::Material( "Ground", &phong_shader );
	ground_material.SetTexture( "uniform_diffuse_map_slot", checker_pattern_texture );
	ground_material.SetTexture( "uniform_specular_map_slot", checker_pattern_texture );
	const auto& ground_quad_scale( ground_transform.GetScaling() );
	Vector4 ground_texture_scale_and_offset( ground_quad_scale.X(), ground_quad_scale.Y() /* Offset is 0 so no need to set it explicitly. */ );
	ground_material.Set( "uniform_texture_scale_and_offset", ground_texture_scale_and_offset );

	wall_material = Engine::Material( "Front Wall", &phong_shader );
	wall_material.SetTexture( "uniform_diffuse_map_slot", checker_pattern_texture );
	wall_material.SetTexture( "uniform_specular_map_slot", checker_pattern_texture );
	const auto& front_wall_quad_scale( wall_front_transform.GetScaling() );
	Vector4 front_wall_texture_scale_and_offset( front_wall_quad_scale /* Offset is 0 so no need to set it explicitly. */ );
	wall_material.Set( "uniform_texture_scale_and_offset", front_wall_texture_scale_and_offset );

	grass_quad_material = Engine::Material( "Grass Quad", &basic_textured_transparent_discard_shader );
	grass_quad_material.SetTexture( "uniform_texture_slot", grass_texture );
	grass_quad_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	window_material = Engine::Material( "Transparent Window", &basic_textured_shader );
	window_material.SetTexture( "uniform_texture_slot", transparent_window_texture );
	window_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	outline_material = Engine::Material( "Outline", &outline_shader );

	offscreen_quad_material = Engine::Material( "Offscreen Quad", &fullscreen_blit_shader );
	offscreen_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 1 ] );

	mirror_quad_material = Engine::Material( "Rear-view Mirror", &fullscreen_blit_shader );
	mirror_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 0 ] );

	ground_quad_surface_data = wall_surface_data =
	{
		.color_diffuse       = {},
		.has_texture_diffuse = 1,
		.shininess           = 32.0f
	};

	cube_surface_data_array = std::vector< Engine::MaterialData::PhongMaterialData >( CUBE_COUNT, ground_quad_surface_data );

	ground_material.Set( "PhongMaterialData", ground_quad_surface_data );
	wall_material.Set( "PhongMaterialData", wall_surface_data );
	for( auto i = 0; i < CUBE_COUNT; i++ )
		cube_material_array[ i ].Set( "PhongMaterialData", cube_surface_data_array[ i ] );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ].Set( "uniform_color", Engine::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f ) );

	outline_material.Set( "uniform_color", Engine::Color4::Orange() );
	outline_material.Set( "uniform_outline_thickness", 0.1f );
}

void SandboxApplication::ResetCamera( const CameraView view )
{
	camera_is_animated = false;

	switch( view )
	{
		case CameraView::FRONT: 
			camera_transform.SetTranslation( 0.0f, 10.0f, -20.0f );
			camera_transform.LookAt( Vector3::Forward() );
			break;
		case CameraView::BACK:
			camera_transform.SetTranslation( 0.0f, 10.0f, +20.0f );
			camera_transform.LookAt( Vector3::Backward() );
			break;
		case CameraView::LEFT:
			camera_transform.SetTranslation( -10.0f, 10.0f, 0.0f );
			camera_transform.LookAt( Vector3::Right() );
			break;
		case CameraView::RIGHT:
			camera_transform.SetTranslation( +10.0f, 10.0f, 0.0f );
			camera_transform.LookAt( Vector3::Left() );
			break;
		case CameraView::TOP:
			camera_transform.SetTranslation( 0.0f, 60.0f, 0.0f );
			camera_transform.LookAt( Vector3::Down() );
			break;
		case CameraView::BOTTOM:
			camera_transform.SetTranslation( 0.0f, -20.0f, 0.0f );
			camera_transform.LookAt( Vector3::Up() );
			break;



		case CameraView::CUSTOM_1:
			camera_transform.SetTranslation( 12.0f, 10.0f, -14.0f );
			camera_transform.SetRotation( -35_deg, 0_deg, 0_deg );
			break;

		default:
			break;
	}

	camera_controller.ResetToTransform();
}

SandboxApplication::Radians SandboxApplication::CalculateVerticalFieldOfView( const Radians horizontal_field_of_view ) const
{
	return 2.0f * Engine::Math::Atan2( Engine::Math::Tan( horizontal_field_of_view / 2.0f ), camera.GetAspectRatio() );
}

void SandboxApplication::ReloadModel( const std::string& file_path )
{
	Engine::Model::ImportSettings model_import_settings( GL_STATIC_DRAW );
	auto new_model = Engine::AssetDatabase< Engine::Model >::CreateAssetFromFile( "Test Model",
																				  file_path,
																				  model_import_settings );

	if( new_model )
	{
		test_model_file_path = file_path;

		for( auto& [ render_group_id, drawables_array ] : test_model_instance.DrawablesMap() )
			for( auto& drawable_to_remove : drawables_array )
				renderer.RemoveDrawable( &drawable_to_remove );

		test_model_instance = ModelInstance( new_model,
											 &phong_shader,
											 Vector3::One(),
											 Quaternion(),
											 Vector3::Up() * 8.0f,
											 /* Diffuse texture: */ nullptr /* -> Use Albedo colors in-model. */,
											 /* Specular texture: */ checker_pattern_texture,
											 Vector4{ 1.0f, 1.0f, 0.0f, 0.0f },
											 {
												{ Engine::Renderer::RenderGroupID( 1 ), nullptr },
												{ Engine::Renderer::RenderGroupID( 2 ), &outline_material }
											 } );

		for( const auto& [ render_group_id, drawables_array ] : test_model_instance.DrawablesMap() )
			for( auto& drawable_to_add : test_model_instance.Drawables( render_group_id ) )
				renderer.AddDrawable( &drawable_to_add, render_group_id );

		ResetCamera();
	}
}

void SandboxApplication::UnloadModel()
{
	test_model_file_path = "";

	for( auto& [ id, drawables_array ] : test_model_instance.DrawablesMap() )
		for( auto& drawable_to_remove : drawables_array )
			renderer.RemoveDrawable( &drawable_to_remove );

	test_model_instance = {};
}

void SandboxApplication::InitializeFramebufferTextures()
{
	const auto width( Platform::GetFramebufferWidthInPixels() ), height( Platform::GetFramebufferHeightInPixels() );

	std::string name( "Offscreen FB 1 Color Tex" + std::to_string( width ) + "x" + std::to_string( height ) );
	offscreen_framebuffer_color_attachment_array[ 0 ] = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateExistingAsset( Engine::Texture( name, GL_RGBA, width, height ) );
	name[ strlen( "Offscreen FB " ) ] = '2';
	offscreen_framebuffer_color_attachment_array[ 1 ] = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateExistingAsset( Engine::Texture( name, GL_RGBA, width, height ) );
}

void SandboxApplication::InitializeRenderbuffers()
{
	const auto width( Platform::GetFramebufferWidthInPixels() ), height( Platform::GetFramebufferHeightInPixels() );
	std::string name( "Offscreen FB 1 D/S Tex " + std::to_string( width ) + "x" + std::to_string( height ) );

	offscreen_framebuffer_depth_and_stencil_attachment_array[ 0 ] = Engine::Renderbuffer( name, Platform::GetFramebufferWidthInPixels(), Platform::GetFramebufferHeightInPixels() );
	name[ strlen( "Offscreen FB " ) ] = '2';
	offscreen_framebuffer_depth_and_stencil_attachment_array[ 1 ] = Engine::Renderbuffer( name, Platform::GetFramebufferWidthInPixels(), Platform::GetFramebufferHeightInPixels() );
}

void SandboxApplication::InitializeFramebuffers()
{
	const auto width( Platform::GetFramebufferWidthInPixels() ), height( Platform::GetFramebufferHeightInPixels() );
	std::string name( "Offscreen FB 1 " + std::to_string( width ) + "x" + std::to_string( height ) );

	offscreen_framebuffer_array[ 0 ] = Engine::Framebuffer( name, Platform::GetFramebufferWidthInPixels(), Platform::GetFramebufferHeightInPixels(),
															offscreen_framebuffer_color_attachment_array[ 0 ],
															&offscreen_framebuffer_depth_and_stencil_attachment_array[ 0 ] );
	name[ strlen( "Offscreen FB " ) ] = '2';
	offscreen_framebuffer_array[ 1 ] = Engine::Framebuffer( name, Platform::GetFramebufferWidthInPixels(), Platform::GetFramebufferHeightInPixels(),
															offscreen_framebuffer_color_attachment_array[ 1 ],
															&offscreen_framebuffer_depth_and_stencil_attachment_array[ 1 ] );
}
