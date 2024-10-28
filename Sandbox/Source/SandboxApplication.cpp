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
#include "Engine/Graphics/Primitive/Primitive_Cube_FullScreen.h"
#include "Engine/Graphics/Primitive/Primitive_Quad.h"
#include "Engine/Graphics/Primitive/Primitive_Quad_FullScreen.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Random.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.
#include <execution>
#include <fstream>

using namespace Engine::Math::Literals;

Engine::Application* Engine::CreateApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
{
    return new SandboxApplication( flags );
}

SandboxApplication::SandboxApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
	:
	Engine::Application( flags ),
	render_group_id_skybox( Engine::Renderer::RenderGroupID{ 999 } ),
	render_group_id_regular( Engine::Renderer::RenderGroupID{ 0 } ),
	render_group_id_outlined_mesh( Engine::Renderer::RenderGroupID{ 1 } ),
	render_group_id_outline( Engine::Renderer::RenderGroupID{ 2 } ),
	render_group_id_transparent( Engine::Renderer::RenderGroupID{ 3 } ),
	render_group_id_screen_size_quad( Engine::Renderer::RenderGroupID{ 4 } ),
	skybox_shader( "Skybox" ),
	phong_shader( "Phong" ),
	phong_shader_instanced( "Phong (Instanced)" ),
	phong_skybox_reflection_shader( "Phong (w/ Skybox Reflection)" ),
	phong_skybox_reflection_shader_instanced( "Phong (w/ Skybox Reflection, Instanced)" ),
	basic_color_shader( "Basic Color" ),
	basic_textured_shader( "Basic Textured" ),
	basic_textured_transparent_discard_shader( "Basic Textured (Discard Transparents)" ),
	outline_shader( "Outline" ),
	fullscreen_blit_shader( "Fullscreen Blit" ),
	postprocess_grayscale_shader( "Post-process: Grayscale" ),
	postprocess_generic_shader( "Post-process: Generic" ),
	normal_visualization_shader( "Normal Visualization" ),
	test_model_info{ .model_instance = {}, .shader = &phong_shader, .file_path = {} },
	meteorite_model_info{ .model_instance = {}, .shader = &phong_shader_instanced, .file_path = {} },
	light_point_transform_array( LIGHT_POINT_COUNT ),
	cube_transform_array( CUBE_COUNT ),
	cube_reflected_transform_array( CUBE_REFLECTED_COUNT ),
	camera( &camera_transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) ),
	camera_rotation_speed( 5.0f ),
	camera_move_speed( 5.0f ),
	camera_controller( &camera, camera_rotation_speed ),
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

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox GL Init." ) );

/* Textures: */
	Engine::Texture::ImportSettings texture_import_settings;
	texture_import_settings.format          = GL_RGB;
	texture_import_settings.min_filter      = Engine::Texture::Filtering::Linear;
	texture_import_settings.flip_vertically = false;
	skybox_texture = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Skybox", 
																					{
																						R"(Asset/Texture/Skybox/right.jpg)",
																						R"(Asset/Texture/Skybox/left.jpg)",
																						R"(Asset/Texture/Skybox/top.jpg)",
																						R"(Asset/Texture/Skybox/bottom.jpg)",
																						R"(Asset/Texture/Skybox/front.jpg)",
																						R"(Asset/Texture/Skybox/back.jpg)"
																					}, texture_import_settings );

	texture_import_settings.format          = GL_RGBA;
	texture_import_settings.min_filter      = Engine::Texture::Filtering::Linear_MipmapLinear;
	texture_import_settings.flip_vertically = true;

	container_texture_diffuse_map  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Diffuse) Map",	R"(Asset/Texture/container2.png)",					texture_import_settings );
	container_texture_specular_map = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Specular) Map",	R"(Asset/Texture/container2_specular.png)",			texture_import_settings );
	transparent_window_texture     = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Transparent Window",		R"(Asset/Texture/blending_transparent_window.png)",	texture_import_settings );
	
	texture_import_settings.wrap_u = Engine::Texture::Wrapping::Repeat;
	texture_import_settings.wrap_v = Engine::Texture::Wrapping::Repeat;

	checker_pattern_texture = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Checkerboard Pattern (09)", R"(Asset/Texture/kenney_prototype/texture_09.png)", texture_import_settings );

/* Shaders: */
	skybox_shader.FromFile( R"(Asset/Shader/Skybox.vert)", R"(Asset/Shader/Skybox.frag)" );
	phong_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)" );
	phong_shader_instanced.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)", { "INSTANCING_ENABLED" } );
	phong_skybox_reflection_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)", { "SKYBOX_ENVIRONMENT_MAPPING" } );
	phong_skybox_reflection_shader_instanced.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)", { "SKYBOX_ENVIRONMENT_MAPPING", "INSTANCING_ENABLED" } );
	basic_color_shader.FromFile( R"(Asset/Shader/BasicColor.vert)", R"(Asset/Shader/BasicColor.frag)", { "INSTANCING_ENABLED" } );
	basic_textured_shader.FromFile( R"(Asset/Shader/BasicTextured.vert)", R"(Asset/Shader/BasicTextured.frag)" );
	basic_textured_transparent_discard_shader.FromFile( R"(Asset/Shader/BasicTextured.vert)", R"(Asset/Shader/BasicTextured.frag)", { "DISCARD_TRANSPARENT_FRAGMENTS" } );
	outline_shader.FromFile( R"(Asset/Shader/Outline.vert)", R"(Asset/Shader/BasicColor.frag)" );
	fullscreen_blit_shader.FromFile( R"(Asset/Shader/FullScreenBlit.vert)", R"(Asset/Shader/BasicTextured.frag)" );
	postprocess_grayscale_shader.FromFile( R"(Asset/Shader/FullScreenBlit.vert)", R"(Asset/Shader/Grayscale.frag)" );
	postprocess_generic_shader.FromFile( R"(Asset/Shader/FullScreenBlit.vert)", R"(Asset/Shader/GenericPostprocess.frag)" );
	normal_visualization_shader.FromFile( R"(Asset/Shader/VisualizeNormals.vert)", R"(Asset/Shader/BasicColor.frag)", { /* No features defined. */ },
										  R"(Asset/Shader/VisualizeNormals.geom)" );

	/* Register shaders not assigned to materials: */
	renderer.RegisterShader( phong_skybox_reflection_shader );
	renderer.RegisterShader( postprocess_grayscale_shader );
	renderer.RegisterShader( postprocess_generic_shader );
	renderer.RegisterShader( normal_visualization_shader );

/* Instancing Data: */
	ResetInstanceData();

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

	/* Keep the first 10 the same as the ones from LOGL: */
	for( auto cube_index = 0; cube_index < CUBE_REFLECTED_COUNT; cube_index++ )
	{
		Degrees angle( 20.0f * cube_index );
		cube_reflected_transform_array[ cube_index ]
			.SetRotation( Quaternion( angle, Vector3{ 1.0f, 0.3f, 0.5f }.Normalized() ) )
			.SetTranslation( CUBE_REFLECTED_POSITIONS[ cube_index ] + Vector3::Up() * 5.0f );
	}

	for( auto cube_index = 0; cube_index < CUBE_REFLECTED_COUNT; cube_index++ )
		cube_reflected_instance_data_array[ cube_index ] = cube_reflected_transform_array[ cube_index ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.

	/* The rest of the cubes: */
	{
		constexpr Vector3 minimum_offset( -1.0f, -0.4f, -1.0f );
		constexpr Vector3 maximum_offset( +1.0f, +0.4f, +1.0f );

		const auto before = std::chrono::system_clock::now();

		std::for_each_n( std::execution::par, cube_transform_array.begin(), CUBE_COUNT, [ & ]( auto&& cube_transform )
		{
			const int cube_index = ( int )( &cube_transform - cube_transform_array.data() );
			Radians random_xz_angle( Engine::Math::Random::Generate( 0.0_rad, Engine::Constants< Radians >::Two_Pi() ) );
			constexpr Radians inclination_limit = 15.0_deg;
			const Radians inclination_angle( Engine::Math::Random::Generate( 0.0_rad,inclination_limit ) );
			Degrees angle( 20.0f * cube_index + inclination_angle );
			cube_transform_array[ cube_index ]
				.SetScaling( 0.3f )
				.SetRotation( Quaternion( angle, Vector3{ 1.0f, 0.3f, 0.5f }.Normalized() ) )
				.SetTranslation( CUBES_ORIGIN + 
								 Vector3( Engine::Math::Cos( random_xz_angle ), 
										  Engine::Math::Sin( inclination_angle ),
										  Engine::Math::Sin( random_xz_angle ) )
								 * ( float )( ( cube_index % 90 ) + 10 ) );
		} );

		const auto after = std::chrono::system_clock::now();

		Engine::ServiceLocator< Engine::GLLogger >::Get().Info( "Cube instance data gen. took " + std::to_string( std::chrono::duration_cast< std::chrono::milliseconds >( after - before ).count() ) +
																" milliseconds.\n" );
	}

	for( auto cube_index = 0; cube_index < CUBE_COUNT; cube_index++ )
		cube_instance_data_array[ cube_index ] = cube_transform_array[ cube_index ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.

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

	cube_mesh_fullscreen = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Cube_FullScreen::Positions.cbegin(), Engine::Primitive::NonIndexed::Cube_FullScreen::Positions.cend() ),
										 "Cube (Fullscreen)",
										 { /* No normals.	*/ },
										 { /* No uvs.		*/ },
										 { /* No indices.	*/ } );

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

	cube_mesh_instanced = Engine::Mesh( cube_mesh,
										{
											Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4 }	// Transform.
										},
										reinterpret_cast< std::vector< float >& >( cube_instance_data_array ),
										CUBE_COUNT,
										GL_STATIC_DRAW );

	cube_reflected_mesh_instanced = Engine::Mesh( cube_mesh,
												  {
													  Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4 }	// Transform.
												  },
												  reinterpret_cast< std::vector< float >& >( cube_reflected_instance_data_array ),
												  CUBE_REFLECTED_COUNT,
												  GL_STATIC_DRAW );

	cube_mesh_instanced_with_color = Engine::Mesh( cube_mesh,
												   {
													   Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4 }, // Transform.
													   Engine::VertexInstanceAttribute{ 1, GL_FLOAT_VEC4 }	// Color.
												   },
												   reinterpret_cast< std::vector< float >& >( light_source_instance_data_array ),
												   LIGHT_POINT_COUNT,
												   GL_DYNAMIC_DRAW );


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
		renderer.SetRenderGroupName( render_group_id_skybox, "Skybox" );
		auto& render_state_skybox = renderer.GetRenderState( render_group_id_skybox ); // Keep default settings except for back face culling.

		render_state_skybox.depth_comparison_function = Engine::Renderer::ComparisonFunction::LessOrEqual;

		render_state_skybox.face_culling_enable = false;
	}

	{
		renderer.SetRenderGroupName( render_group_id_regular, "Default" );
		auto& render_state_regular_meshes = renderer.GetRenderState( render_group_id_regular ); // Keep default settings.
	}
	
	{
		/* This pass draws the mesh semi-regularly; It also marks the stencil buffer with 1s everywhere the mesh is drawn at. */
		renderer.SetRenderGroupName( render_group_id_outlined_mesh, "Outlined Meshes" );
		auto& render_state_outline_meshes = renderer.GetRenderState( render_group_id_outlined_mesh );

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
		renderer.SetRenderGroupName( render_group_id_outline, "Outlines" );
		auto& render_state_outlines = renderer.GetRenderState( render_group_id_outline );

		/* Omitted settings are left to defaults. */

		render_state_outlines.depth_test_enable = false;

		render_state_outlines.stencil_test_enable         = true;
		render_state_outlines.stencil_write_mask          = 0x00; // Disable writes; This pass only needs to READ the stencil buffer, to figure out where NOT to render.
		render_state_outlines.stencil_comparison_function = Engine::Renderer::ComparisonFunction::NotEqual; // Render everywhere that's not the actual mesh, i.e., the border.
		render_state_outlines.stencil_ref                 = 0x01;
	}

	{
		/* This pass draws the transparent meshes. */
		renderer.SetRenderGroupName( render_group_id_transparent, "Transparent Meshes" );
		auto& render_state_transparent = renderer.GetRenderState( render_group_id_transparent );

		/* Omitted settings are left to defaults. */

		render_state_transparent.blending_enable = true;

		render_state_transparent.blending_source_color_factor      = Engine::Renderer::BlendingFactor::SourceAlpha;
		render_state_transparent.blending_destination_color_factor = Engine::Renderer::BlendingFactor::OneMinusSourceAlpha;
		render_state_transparent.blending_source_alpha_factor      = Engine::Renderer::BlendingFactor::SourceAlpha;
		render_state_transparent.blending_destination_alpha_factor = Engine::Renderer::BlendingFactor::OneMinusSourceAlpha;

		render_state_transparent.sorting_mode = Engine::Renderer::SortingMode::DepthFarthestToNearest;
	}

	{
		renderer.SetRenderGroupName( render_group_id_screen_size_quad, "Screen-size Quad" );
		auto& render_state_screen_size_quad = renderer.GetRenderState( render_group_id_screen_size_quad ); // Keep default settings.
	}

	/* Create Drawables and assign them RenderGroups: */

	skybox_drawable = Engine::Drawable( &cube_mesh_fullscreen, &skybox_material );
	renderer.AddDrawable( &skybox_drawable, render_group_id_skybox );

	light_sources_drawable = Engine::Drawable( &cube_mesh_instanced_with_color, &light_source_material, nullptr /* => No Transform here, as we will provide the Transforms as instance data. */ );
	renderer.AddDrawable( &light_sources_drawable, render_group_id_regular );

	cube_drawable = Engine::Drawable( &cube_mesh_instanced, &cube_material, nullptr /* => No Transform here, as we will provide the Transforms as instance data. */ );
	renderer.AddDrawable( &cube_drawable, render_group_id_outlined_mesh );

	cube_reflected_drawable = Engine::Drawable( &cube_reflected_mesh_instanced, &cube_reflected_material, nullptr /* => No Transform here, as we will provide the Transforms as instance data. */ );
	renderer.AddDrawable( &cube_reflected_drawable, render_group_id_outlined_mesh );

	//cube_drawable_outline = Engine::Drawable( &cube_mesh_instanced, &outline_material, nullptr /* => No Transform here, as we will provide the Transforms as instance data. */ );
	//renderer.AddDrawable( &cube_drawable_outline, render_group_id_outline );

	ground_drawable = Engine::Drawable( &quad_mesh, &ground_material, &ground_transform );
	renderer.AddDrawable( &ground_drawable, render_group_id_regular );

	wall_front_drawable = Engine::Drawable( &quad_mesh, &wall_material, &wall_front_transform );
	wall_left_drawable  = Engine::Drawable( &quad_mesh, &wall_material, &wall_left_transform );
	wall_right_drawable = Engine::Drawable( &quad_mesh, &wall_material, &wall_right_transform );
	wall_back_drawable  = Engine::Drawable( &quad_mesh, &wall_material, &wall_back_transform );
	/*renderer.AddDrawable( &wall_front_drawable, render_group_id_regular );
	renderer.AddDrawable( &wall_left_drawable, render_group_id_regular );
	renderer.AddDrawable( &wall_right_drawable, render_group_id_regular );
	renderer.AddDrawable( &wall_back_drawable, render_group_id_regular );*/

	for( auto i = 0; i < WINDOW_COUNT; i++ )
	{
		window_drawable_array[ i ] = Engine::Drawable( &quad_mesh_uvs_only, &window_material, &window_transform_array[ i ] );
		renderer.AddDrawable( &window_drawable_array[ i ], render_group_id_transparent );
	}

	offscreen_quad_drawable = Engine::Drawable( &quad_mesh_fullscreen, &offscreen_quad_material );
	renderer.AddDrawable( &offscreen_quad_drawable, render_group_id_screen_size_quad );

	mirror_quad_drawable = Engine::Drawable( &quad_mesh_mirror, &mirror_quad_material );
	renderer.AddDrawable( &mirror_quad_drawable, render_group_id_screen_size_quad );

	mirror_quad_drawable.ToggleOnOrOff( not draw_rear_view_cam_to_imgui );

	/* Disable some RenderGroups & Drawables on start-up to decrease clutter. */
	renderer.ToggleRenderGroup( render_group_id_outline, false );
	renderer.ToggleRenderGroup( render_group_id_transparent, false );
	ground_drawable.ToggleOff();

/* Camera: */
	ResetCamera();

	/* This is the earliest place we can MaximizeWindow() at,
	 * because the Renderer will populate its Intrinsic UBO info only upon AddDrawable( <Drawable with a Shader using said UBO> ). */

	/* No need to Initialize Framebuffer related stuff as maximizing the window will cause them to be (re)initialized in OnFramebufferResizeEvent(). */

	Platform::MaximizeWindow();

/* Models: */
	if( auto config_file = std::ifstream( "config.ini" ) )
	{
		std::string token;
		config_file >> token;
		if( token == "test_model_path" )
		{
			config_file >> token /* '=' */ >> token;
			if( not token.empty() && std::filesystem::exists( token ) )
				ReloadModel( test_model_info, token, "Test Model" );
		}

		config_file >> token;
		if( token == "meteorite_model_path" )
		{
			config_file >> token /* '=' */ >> token;
			if( not token.empty() && std::filesystem::exists( token ) )
				if( ReloadModel( meteorite_model_info, token, "Meteorite" ) )
					ReplaceMeteoriteAndCubeDrawables( true );
		}
	}
}

void SandboxApplication::Shutdown()
{
	if( auto config_file = std::ofstream( "config.ini" ) )
	{
		config_file << "test_model_path = "			<< test_model_info.file_path		<< "\n";
		config_file << "meteorite_model_path = "	<< meteorite_model_info.file_path	<< "\n";
	}
}

//void SandboxApplication::Run()
//{
//
//}

void SandboxApplication::Update()
{
	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Update()" ) );

	// TODO: Separate applicationg logs from GL logs.

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
			const auto& old_rotation( light_point_transform_array[ i ].GetRotation() );
			Radians old_heading, old_pitch, bank;
			Engine::Math::QuaternionToEuler( old_rotation, old_heading, old_pitch, bank );

			Radians new_angle( current_time_mod_two_pi + ( float )angle_increment * ( float )i );
			const Radians new_angle_mod_two_pi( std::fmod( new_angle.Value(), Engine::Constants< float >::Two_Pi() ) );

			const bool heading_instead_of_pitching = new_angle_mod_two_pi <= Engine::Constants< Radians >::Pi();

			const auto point_light_rotation( Engine::Math::EulerToMatrix( float(  heading_instead_of_pitching ) * new_angle_mod_two_pi,
																		  float( !heading_instead_of_pitching ) * ( new_angle_mod_two_pi - 3.0f * Engine::Constants< Radians >::Pi_Over_Two() ), bank ) );

			point_light_position_world_space = ( ( ( heading_instead_of_pitching ? Vector4::Forward() : Vector4::Up() ) * light_point_orbit_radius ) *
												 ( point_light_rotation * Engine::Matrix::Translation( CAMERA_ROTATION_ORIGIN ) ) ).XYZ();
			point_light_position_world_space.SetY( point_light_position_world_space.Y() + 2.0f );

			light_point_transform_array[ i ].SetRotation( Engine::Math::MatrixToQuaternion( point_light_rotation ) );
		}

		light_point_transform_array[ i ].SetTranslation( point_light_position_world_space );

		light_source_instance_data_array[ i ].transform = light_point_transform_array[ i ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.
		light_source_instance_data_array[ i ].color     = Engine::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f );
	}

	cube_mesh_instanced_with_color.UpdateInstanceData( light_source_instance_data_array.data() );

	/* Camera transform: */
	if( camera_animation_is_enabled )
	{
		/* Orbit motion: */

		Engine::Math::Vector< Radians, 3 > old_euler_angles;
		Engine::Math::QuaternionToEuler( camera_transform.GetRotation(), old_euler_angles );
		// Don't modify X & Z euler angles; Allow the user to modify them.
		camera_transform.SetRotation( Engine::Math::EulerToQuaternion( -current_time_as_angle * 0.33f, old_euler_angles.X(), old_euler_angles.Z() ) );

		auto new_pos = CAMERA_ROTATION_ORIGIN + -camera_transform.Forward() * camera_animation_orbit_radius;
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
		auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Render(): Render to Offscreen FB 1 (Rear view)" ) );

		camera_controller.Invert();
		renderer.Update( camera );

		renderer.SetCurrentFramebuffer( &offscreen_framebuffer_array[ 0 ] );

		renderer.Render( camera, { render_group_id_regular, render_group_id_outlined_mesh, render_group_id_outline, render_group_id_transparent, render_group_id_skybox } );

		/* Now that the off-screen frame-buffer is filled, we should create mip-maps of it so that it can be mapped onto smaller surfaces: */
		offscreen_framebuffer_color_attachment_array[ 0 ]->GenerateMipmaps();
	}

	/* Pass 2 - Default view: Invert camera direction again (to revert to default view), draw everything to the off-screen frame-buffer 2. */
	{
		auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Render(): Render to Offscreen FB 2 (Default view)" ) );
		
		camera_controller.Invert();
		renderer.Update( camera );

		renderer.SetCurrentFramebuffer( &offscreen_framebuffer_array[ 1 ] );

		renderer.Render( camera, { render_group_id_regular, render_group_id_outlined_mesh, render_group_id_outline, render_group_id_transparent, render_group_id_skybox } );

		/* Now that the off-screen frame-buffer is filled, we should create mip-maps of it so that it can be mapped onto smaller surfaces: */
		offscreen_framebuffer_color_attachment_array[ 1 ]->GenerateMipmaps();
	}

	/* Pass 3: Blit both off-screen frame-buffers to quads on the editor frame-buffer. */
	{
		auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Render(): Blit Offscreen FB(s) to Main FB" ) );

		if( show_imgui )
			renderer.SetCurrentFramebuffer( &editor_framebuffer );
		else
			renderer.ResetToDefaultFramebuffer();

		renderer.Render( camera, { render_group_id_screen_size_quad } );

		/* Now that the editor frame-buffer is filled, we should create mip-maps of it so that it can be mapped onto smaller surfaces: */
		editor_framebuffer_color_attachment->GenerateMipmaps();
	}

	if( show_imgui )
		renderer.ResetToDefaultFramebuffer();
}

void SandboxApplication::RenderImGui()
{
	Application::RenderImGui();

	if( show_imgui_demo_window )
		ImGui::ShowDemoWindow();

	RenderImGui_Viewport();

	const auto& style = ImGui::GetStyle();

	{
		const auto [ width, height ] = Platform::GetFramebufferSizeInPixels();
		ImGui::SetNextWindowSize( ImVec2( width / 4.0f, height / 4.0f ), ImGuiCond_FirstUseEver );
	}
	if( ImGui::Begin( "Rear-view Camera", nullptr, ImGuiWindowFlags_NoScrollbar ) )
	{
		if( ImGui::Checkbox( "Draw to this window instead of default Framebuffer", &draw_rear_view_cam_to_imgui ) )
			mirror_quad_drawable.ToggleOnOrOff( not draw_rear_view_cam_to_imgui );

		if( draw_rear_view_cam_to_imgui )
		{
			const ImVec2 negative_size_offset( style.WindowPadding * 2 );
			ImGui::Image( ( void* )( intptr_t )offscreen_framebuffer_color_attachment_array[ 0 ]->Id().Get(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );
		}
	}

	ImGui::End();

	if( ImGui::Begin( ICON_FA_CUBES " Models", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		enum ModelLoadActionResult
		{
			Loaded, Unloaded, None
		};

		auto DrawModelLine = [ & ]( ModelInfo& model_info, const char* model_name ) -> ModelLoadActionResult
		{
			ModelLoadActionResult action = ModelLoadActionResult::None;

			ImGui::SeparatorText( model_name );

			ImGui::PushID( model_name );

			if( not model_info.file_path.empty() )
			{
				static char buffer[ 260 ];
				strncpy_s( buffer, model_info.file_path.c_str(), model_info.file_path.size() );
				ImGui::BeginDisabled();
				ImGui::InputText( "Loaded Model Path", buffer, ( int )model_info.file_path.size(), ImGuiInputTextFlags_ReadOnly );
				ImGui::EndDisabled();

				if( ImGui::Button( ICON_FA_FOLDER_OPEN " Reload" ) )
				{
					if( auto maybe_file_name = Platform::BrowseFileName( { "glTF (*.gltf;*.glb)",		"*.gltf;*.glb",
																			"Standard glTF (*.gltf)",	"*.gltf",
																			"Binary glTF (*.glb)",		"*.glb" },
																		 "Choose a Model to Load" );
						maybe_file_name.has_value() && *maybe_file_name != model_info.file_path )
					{
						if( ReloadModel( model_info, *maybe_file_name, model_name ) )
							action = ModelLoadActionResult::Loaded;
					}
				}
				ImGui::SameLine();
				if( ImGui::Button( ICON_FA_XMARK " Unload" ) )
				{
					UnloadModel( model_info );
					action = ModelLoadActionResult::Unloaded;
				}
			}
			else
			{
				const auto button_size( ImGui::CalcTextSize( ICON_FA_CUBES " Models   " ) + style.ItemInnerSpacing );
				if( ImGui::Button( ICON_FA_FOLDER_OPEN " Load", button_size ) )
				{
					if( auto maybe_file_name = Platform::BrowseFileName( { "glTF (*.gltf;*.glb)",		"*.gltf;*.glb",
																			"Standard glTF (*.gltf)",	"*.gltf",
																			"Binary glTF (*.glb)",		"*.glb" },
																		 "Choose a Model to Load" );
						maybe_file_name.has_value() && *maybe_file_name != model_info.file_path )
					{
						if( ReloadModel( model_info, *maybe_file_name, model_name ) )
							action = ModelLoadActionResult::Loaded;
					}
				}
			}

			ImGui::PopID();

			return action;
		};

		DrawModelLine( test_model_info, "Test Model" );
		switch( DrawModelLine( meteorite_model_info, "Meteorite" ) )
		{
			case ModelLoadActionResult::Loaded:
				ReplaceMeteoriteAndCubeDrawables( true );
				break;
			case ModelLoadActionResult::Unloaded:
				ReplaceMeteoriteAndCubeDrawables( false );
				break;
			default:
				break;
		}
	}

	ImGui::End();

	Engine::ImGuiDrawer::Draw( skybox_material, renderer );
	Engine::ImGuiDrawer::Draw( light_source_material, renderer );
	Engine::ImGuiDrawer::Draw( ground_material, renderer );
	Engine::ImGuiDrawer::Draw( wall_material, renderer );
	Engine::ImGuiDrawer::Draw( window_material, renderer );
	Engine::ImGuiDrawer::Draw( cube_material, renderer );
	Engine::ImGuiDrawer::Draw( cube_reflected_material, renderer );
	for( auto& test_material : test_model_info.model_instance.Materials() )
		Engine::ImGuiDrawer::Draw( const_cast< Engine::Material& >( test_material ), renderer );
	for( auto& test_material : meteorite_model_info.model_instance.Materials() )
		Engine::ImGuiDrawer::Draw( const_cast< Engine::Material& >( test_material ), renderer );
	Engine::ImGuiDrawer::Draw( outline_material, renderer );
	Engine::ImGuiDrawer::Draw( offscreen_quad_material, renderer );
	Engine::ImGuiDrawer::Draw( mirror_quad_material, renderer );

	if( ImGui::Begin( "Instance Data" ) )
	{
		ImGui::SeparatorText( "Light Sources" );

		if( ImGui::BeginTable( "Light Sources", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
		{
			ImGui::TableSetupColumn( "#" );
			ImGui::TableSetupColumn( "Color" );
			ImGui::TableSetupColumn( "Transform" );

			ImGui::TableHeadersRow();
			ImGui::TableNextRow();

			const auto first_column_width = ImGui::CalcTextSize( " 999" ).x + style.ItemInnerSpacing.x * 2;

			for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
			{
				const auto& instance_data = light_source_instance_data_array[ index ];

				ImGui::PushID( index );

				ImGui::TableNextColumn();
				int no = index + 1;
				ImGui::PushItemWidth( first_column_width );
				ImGui::InputInt( "", &no, 0, 0, ImGuiInputTextFlags_ReadOnly );
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				Engine::ImGuiDrawer::Draw( instance_data.color );
				
				ImGui::TableNextColumn();
				Engine::ImGuiDrawer::Draw( instance_data.transform );


				ImGui::PopID();
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();

	if( ImGui::Begin( ICON_FA_LIGHTBULB " Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Checkbox( "Enabled##AllLights", &light_is_enabled ) )
		{
			light_directional.is_enabled = light_is_enabled;

			for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
			{
				light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable;
				light_source_instance_data_array[ i ].color.SetW( 1.0f - light_source_instance_data_array[ i ].color.W() );
			}

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
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable;
						light_source_instance_data_array[ i ].color.SetW( 1.0f - light_source_instance_data_array[ i ].color.W() );
					}
				}
				ImGui::Checkbox( "Animate (Orbit) Point Lights", &light_point_array_is_animated );
				if( light_point_array_is_animated )
					ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 30.0f );
				Engine::ImGuiUtility::EndGroupPanel();

				if( ImGui::TreeNodeEx( "Point Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						const bool was_enabled = light_point_array[ i ].is_enabled;
						if( Engine::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str(), light_point_array_is_animated /* hide position. */ ) )
						{
							light_source_instance_data_array[ i ].color = Engine::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f );
							if( was_enabled != light_point_array[ i ].is_enabled )
								light_source_instance_data_array[ i ].color.SetW( 1.0f - light_source_instance_data_array[ i ].color.W() );
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
			SwitchCameraView( CameraView::TOP );

	/* Row 2: */
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Left" ) )
			SwitchCameraView( CameraView::LEFT );
		ImGui::SameLine();
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Front" ) )
			SwitchCameraView( CameraView::FRONT );
		ImGui::SameLine();
		ImGui::SetCursorPosX( button_width * 2 );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Right" ) )
			SwitchCameraView( CameraView::RIGHT );

	/* Row 3: */
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Back" ) )
			SwitchCameraView( CameraView::BACK );

	/* Row 4: */
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Bottom" ) )
			SwitchCameraView( CameraView::BOTTOM );


		ImGui::NewLine();

	/* Row 5: */
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Custom (1)" ) )
			SwitchCameraView( CameraView::CUSTOM_1 );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Camera" ) )
			ResetCamera();

		ImGui::Checkbox( "Animate (Rotate) Camera", &camera_animation_is_enabled );
		if( camera_animation_is_enabled )
			ImGui::SliderFloat( "Camera Orbit Radius", &camera_animation_orbit_radius, 0.0f, 50.0f );
		Engine::ImGuiDrawer::Draw( camera_transform, Engine::Transform::Mask::NoScale, "Main Camera" );
	}

	ImGui::End();

	if( ImGui::Begin( "Projection", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		Engine::ImGuiUtility::BeginGroupPanel();
		{
			if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Projection" ) )
				ResetProjection();

			if( Engine::ImGuiDrawer::Draw( camera, "Main Camera", true ) )
				RecalculateProjectionParameters();
		}
		Engine::ImGuiUtility::EndGroupPanel();
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
	if( width_new_pixels == 0 || height_new_pixels == 0 || 
		( editor_framebuffer_color_attachment && editor_framebuffer_color_attachment->Size() == Vector2I{ width_new_pixels, height_new_pixels } ) )
		return;

	renderer.OnFramebufferResize( width_new_pixels, height_new_pixels );

	RecalculateProjectionParameters( width_new_pixels, height_new_pixels );

	// Re-initialize:
	InitializeFramebufferTextures( width_new_pixels, height_new_pixels );
	InitializeRenderbuffers( width_new_pixels, height_new_pixels );
	InitializeFramebuffers( width_new_pixels, height_new_pixels );

	mirror_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 0 ] );
	offscreen_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 1 ] );
}

void SandboxApplication::OnFramebufferResizeEvent( const Vector2I new_size_pixels )
{
	OnFramebufferResizeEvent( new_size_pixels.X(), new_size_pixels.Y() );
}

void SandboxApplication::RenderImGui_Viewport()
{
	{
		const auto [ width, height ] = Platform::GetFramebufferSizeInPixels();
		ImGui::SetNextWindowSize( ImVec2( ( float )width, ( float )height ), ImGuiCond_Appearing );
	}

	if( ImGui::Begin( "Viewport" ) )
	{
		const ImVec2   viewport_size_imvec2( ImGui::GetContentRegionAvail() );
		const Vector2I viewport_size( ( int )viewport_size_imvec2.x, ( int )viewport_size_imvec2.y );

		const auto& imgui_io = ImGui::GetIO();
		if( (     imgui_io.WantCaptureMouse && imgui_io.MouseReleased[ 0 ] ) ||
			( not imgui_io.WantCaptureMouse && Platform::IsMouseButtonReleased( Platform::MouseButton::Left ) ) )
			OnFramebufferResizeEvent( viewport_size.X(), viewport_size.Y() );

		if( ImGui::IsWindowHovered() )
		{
			ImGui::SetNextFrameWantCaptureMouse( false );
			ImGui::SetNextFrameWantCaptureKeyboard( false );
		}

		ImGui::Image( ( void* )( intptr_t )editor_framebuffer_color_attachment->Id().Get(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );
	}

	ImGui::End();
}

void SandboxApplication::ResetLightingData()
{
	light_directional = 
	{
		.is_enabled = true,
		.data =
		{
			.ambient  = Engine::Color3{  0.33f,  0.33f,  0.33f },
			.diffuse  = Engine::Color3{  0.4f,   0.4f,   0.4f  },
			.specular = Engine::Color3{  0.5f,   0.5f,   0.5f  },
		},
		.transform = &light_directional_transform
	};

	light_is_enabled = true;

	light_point_array_disable      = false;
	light_point_array_is_animated  = true;
	light_point_orbit_radius       = 20.0f;

	light_spot_array_disable = false;

	light_point_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		const auto random_color( Engine::Math::Random::Generate< Engine::Color3 >() );
		light_point_array[ i ] =
		{
			.is_enabled = true,
			.data =
			{
				.ambient_and_attenuation_constant = { .color = { 0.05f, 0.05f, 0.05f },	.scalar = 0.06f		},
				.diffuse_and_attenuation_linear   = { .color = random_color,			.scalar = 0.001f	},
				.specular_attenuation_quadratic   = { .color = random_color,			.scalar = 0.0375f	},
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
	skybox_material = Engine::Material( "Skybox", &skybox_shader );
	skybox_material.SetTexture( "uniform_texture_slot", skybox_texture );

	light_source_material = Engine::Material( "Light Source", &basic_color_shader );
	
	/* Set the first cube's material to Phong shader w/ skybox reflection: */
	cube_reflected_material = Engine::Material( "Cube (Reflected)", &phong_skybox_reflection_shader_instanced );
	cube_reflected_material.SetTexture( "uniform_diffuse_map_slot", container_texture_diffuse_map );
	cube_reflected_material.SetTexture( "uniform_specular_map_slot", container_texture_specular_map );
	cube_reflected_material.SetTexture( "uniform_reflection_map_slot", container_texture_specular_map );
	cube_reflected_material.SetTexture( "uniform_texture_skybox_slot", skybox_texture );
	cube_reflected_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );
	cube_reflected_material.Set( "uniform_reflectivity", 1.0f );

	cube_material = Engine::Material( "Cube", &phong_shader_instanced );
	cube_material.SetTexture( "uniform_diffuse_map_slot", container_texture_diffuse_map );
	cube_material.SetTexture( "uniform_specular_map_slot", container_texture_specular_map );
	cube_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

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

	window_material = Engine::Material( "Transparent Window", &basic_textured_shader );
	window_material.SetTexture( "uniform_texture_slot", transparent_window_texture );
	window_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	outline_material = Engine::Material( "Outline", &outline_shader );

	mirror_quad_material = Engine::Material( "Rear-view Mirror", &fullscreen_blit_shader );
	mirror_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 0 ] );

	offscreen_quad_material = Engine::Material( "Offscreen Quad", &fullscreen_blit_shader );
	offscreen_quad_material.SetTexture( "uniform_texture_slot", offscreen_framebuffer_color_attachment_array[ 1 ] );

	ground_quad_surface_data = wall_surface_data = cube_surface_data =
	{
		.color_diffuse       = {},
		.has_texture_diffuse = 1,
		.shininess           = 32.0f
	};

	ground_material.Set( "PhongMaterialData", ground_quad_surface_data );
	wall_material.Set( "PhongMaterialData", wall_surface_data );
	cube_material.Set( "PhongMaterialData", cube_surface_data );
	cube_reflected_material.Set( "PhongMaterialData", cube_surface_data );

	outline_material.Set( "uniform_color", Engine::Color4::Orange() );
	outline_material.Set( "uniform_outline_thickness", 0.1f );
}

void SandboxApplication::ResetInstanceData()
{
	cube_instance_data_array.resize( CUBE_COUNT );
	cube_reflected_instance_data_array.resize( CUBE_REFLECTED_COUNT );
	light_source_instance_data_array.resize( LIGHT_POINT_COUNT );
}

void SandboxApplication::ResetCamera()
{
	camera_animation_orbit_radius = 30.0f;

	ResetProjection();

	SwitchCameraView( CameraView::FRONT );
}

void SandboxApplication::ResetProjection()
{
	camera = Engine::Camera( &camera_transform, camera.GetAspectRatio(), camera.GetVerticalFieldOfView() ); // Keep current aspect ratio & v-fov.
	renderer.OnProjectionParametersChange( camera );
}

void SandboxApplication::SwitchCameraView( const CameraView view )
{
	camera_animation_is_enabled = false;

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

bool SandboxApplication::ReloadModel( ModelInfo& model_info_to_be_loaded, const std::string& file_path, const char* name )
{
	Engine::Model::ImportSettings model_import_settings( GL_STATIC_DRAW );
	auto new_model = Engine::AssetDatabase< Engine::Model >::CreateAssetFromFile( name,
																				  file_path,
																				  model_import_settings );
	
	if( new_model )
	{
		model_info_to_be_loaded.file_path = file_path;

		auto& model_instance_to_load_into = model_info_to_be_loaded.model_instance;

		for( auto& [ render_group_id, drawables_array ] : model_instance_to_load_into.DrawablesMap() )
			for( auto& drawable_to_remove : drawables_array )
				renderer.RemoveDrawable( &drawable_to_remove );

		model_instance_to_load_into = ModelInstance( new_model,
													 model_info_to_be_loaded.shader,
													 Vector3::One(),
													 Quaternion(),
													 Vector3::Up() * 8.0f,
													 /* Diffuse texture: */ nullptr /* -> Use Albedo colors in-model. */,
													 /* Specular texture: */ checker_pattern_texture,
													 Vector4{ 1.0f, 1.0f, 0.0f, 0.0f },
													 {
														{ render_group_id_outlined_mesh,	nullptr },
														{ render_group_id_outline,			&outline_material }
													 } );

		for( const auto& [ render_group_id, drawables_array ] : model_instance_to_load_into.DrawablesMap() )
			for( auto& drawable_to_add : model_instance_to_load_into.Drawables( render_group_id ) )
				renderer.AddDrawable( &drawable_to_add, render_group_id );

		return true;
	}

	return false;
}

void SandboxApplication::UnloadModel( ModelInfo& model_info_to_be_loaded )
{
	model_info_to_be_loaded.file_path = "";

	for( auto& [ id, drawables_array ] : model_info_to_be_loaded.model_instance.DrawablesMap() )
		for( auto& drawable_to_remove : drawables_array )
			renderer.RemoveDrawable( &drawable_to_remove );

	model_info_to_be_loaded.model_instance = {};
}

void SandboxApplication::ReplaceMeteoriteAndCubeDrawables( bool use_meteorites )
{
	if( use_meteorites )
	{
		meteorite_drawable = &meteorite_model_info.model_instance.Drawables( render_group_id_outlined_mesh ).front();
		cube_mesh_instanced = Engine::Mesh( *meteorite_drawable->GetMesh(),
											{
												Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4 }	// Transform.
											},
											reinterpret_cast< std::vector< float >& >( cube_instance_data_array ),
											CUBE_COUNT,
											GL_STATIC_DRAW );
		meteorite_drawable->SetMesh( &cube_mesh_instanced );
		renderer.RemoveDrawable( &cube_drawable );
	}
	else
	{
		renderer.RemoveDrawable( meteorite_drawable );
		meteorite_drawable = nullptr;
		cube_mesh_instanced = Engine::Mesh( cube_mesh,
											{
												Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4 }	// Transform.
											},
											reinterpret_cast< std::vector< float >& >( cube_instance_data_array ),
											CUBE_COUNT,
											GL_STATIC_DRAW );
		renderer.AddDrawable( &cube_drawable );
	}
}

void SandboxApplication::InitializeFramebufferTextures( const int width_new_pixels, const int height_new_pixels )
{
	/* Main: */
	{
		std::string name( "Editor FB Color Tex " + std::to_string( width_new_pixels ) + "x" + std::to_string( height_new_pixels ) );
		editor_framebuffer_color_attachment = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateExistingAsset( Engine::Texture( name, GL_RGBA, width_new_pixels, height_new_pixels ) );
	}

	/* Offscreen: */
	{
		{
			std::string name( "Offscreen FB 1 Color Tex " + std::to_string( width_new_pixels / 4 ) + "x" + std::to_string( height_new_pixels / 4 ) );
			offscreen_framebuffer_color_attachment_array[ 0 ] = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateExistingAsset( Engine::Texture( name, GL_RGBA,
																																					 width_new_pixels / 4, 
																																					 height_new_pixels / 4 ) );
		}
		{
			std::string name( "Offscreen FB 2 Color Tex " + std::to_string( width_new_pixels ) + "x" + std::to_string( height_new_pixels ) );
			offscreen_framebuffer_color_attachment_array[ 1 ] = Engine::AssetDatabase< Engine::Texture >::AddOrUpdateExistingAsset( Engine::Texture( name, GL_RGBA, 
																																					 width_new_pixels,
																																					 height_new_pixels ) );
		}
	}
}

void SandboxApplication::InitializeRenderbuffers( const int width_new_pixels, const int height_new_pixels )
{
	/* Main: */
	{
		std::string name( "Editor FB D/S Tex " + std::to_string( width_new_pixels ) + "x" + std::to_string( height_new_pixels ) );
		editor_framebuffer_depth_and_stencil_attachment = Engine::Renderbuffer( name, width_new_pixels, height_new_pixels );
	}

	/* Offscreen: */
	{
		{
			std::string name( "Offscreen FB 1 D/S Tex " + std::to_string( width_new_pixels / 4 ) + "x" + std::to_string( height_new_pixels / 4 ) );
			offscreen_framebuffer_depth_and_stencil_attachment_array[ 0 ] = Engine::Renderbuffer( name, width_new_pixels / 4, height_new_pixels / 4 );
		}
		{
			std::string name( "Offscreen FB 2 D/S Tex " + std::to_string( width_new_pixels ) + "x" + std::to_string( height_new_pixels ) );
			offscreen_framebuffer_depth_and_stencil_attachment_array[ 1 ] = Engine::Renderbuffer( name, width_new_pixels, height_new_pixels );
		}
	}
}

void SandboxApplication::InitializeFramebuffers( const int width_new_pixels, const int height_new_pixels )
{
	/* Main: */
	{
		std::string name( "Editor FB " + std::to_string( width_new_pixels ) + "x" + std::to_string( height_new_pixels ) );
		editor_framebuffer = Engine::Framebuffer( name, width_new_pixels, height_new_pixels,
												  editor_framebuffer_color_attachment,
												  &editor_framebuffer_depth_and_stencil_attachment );
	}

	/* Offscreen: */
	{
		{
			std::string name( "Offscreen FB 1 " + std::to_string( width_new_pixels / 4 ) + "x" + std::to_string( height_new_pixels / 4 ) );
			offscreen_framebuffer_array[ 0 ] = Engine::Framebuffer( name, width_new_pixels / 4, height_new_pixels / 4,
																	offscreen_framebuffer_color_attachment_array[ 0 ],
																	&offscreen_framebuffer_depth_and_stencil_attachment_array[ 0 ] );
		}
		{
			std::string name( "Offscreen FB 2 " + std::to_string( width_new_pixels ) + "x" + std::to_string( height_new_pixels ) );
			offscreen_framebuffer_array[ 1 ] = Engine::Framebuffer( name, width_new_pixels, height_new_pixels,
																	offscreen_framebuffer_color_attachment_array[ 1 ],
																	&offscreen_framebuffer_depth_and_stencil_attachment_array[ 1 ] );
		}
	}
}

void SandboxApplication::RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels )
{
	camera.SetAspectRatio( float( width_new_pixels ) / height_new_pixels );
	camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );

	renderer.OnProjectionParametersChange( camera );
}

void SandboxApplication::RecalculateProjectionParameters( const Vector2I new_size_pixels )
{
	RecalculateProjectionParameters( new_size_pixels.X(), new_size_pixels.Y() );
}

void SandboxApplication::RecalculateProjectionParameters()
{
	RecalculateProjectionParameters( editor_framebuffer_color_attachment->Size() );
}
