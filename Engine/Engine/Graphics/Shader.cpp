// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engince Includes.
#include "Core/ServiceLocator.h"
#include "Core/Utility.hpp"
#include "GLLogger.h"
#include "Shader.hpp"
#include "ShaderTypeInformation.h"
#include "UniformBlockBindingPointManager.h"

// std Includes.
#include <numeric> // std::iota.
#include <regex>

// Vendor Includes.
#include <stb/stb_include.h>

namespace Engine
{
	/* Will be initialized later with FromFile(). */
	Shader::Shader( const char* name )
		:
		program_id( 0 ),
		name( name )
	{
	}

	Shader::Shader( const char* name, const char* vertex_shader_source_file_path, const char* fragment_shader_source_file_path, 
					const std::vector< std::string >& features_to_set,
					const char* geometry_shader_source_file_path )
		:
		name( name ),
		vertex_source_path( vertex_shader_source_file_path ),
		geometry_source_path( geometry_shader_source_file_path ),
		fragment_source_path( fragment_shader_source_file_path ),
		features_requested( features_to_set )
	{
		FromFile( vertex_shader_source_file_path, fragment_shader_source_file_path, features_to_set, geometry_shader_source_file_path );
	}

	Shader::~Shader()
	{
		if( IsValid() )
		{
			glDeleteProgram( program_id.Get() );
			program_id.Reset();
		}
	}

	bool Shader::FromFile( const char* vertex_shader_source_file_path,
						   const char* fragment_shader_source_file_path,
						   const std::vector< std::string >& features_to_set,
						   const char* geometry_shader_source_file_path )
	{
		this->vertex_source_path   = vertex_shader_source_file_path;
		this->geometry_source_path = geometry_shader_source_file_path ? geometry_shader_source_file_path : "";
		this->fragment_source_path = fragment_shader_source_file_path;

		features_requested = features_to_set;

		unsigned int vertex_shader_id = 0, geometry_shader_id = 0, fragment_shader_id = 0;

		std::optional< std::string > vertex_shader_source;
		std::optional< std::string > geometry_shader_source;
		std::optional< std::string > fragment_shader_source;
		std::unordered_map< std::string, Feature > vertex_shader_features;
		std::unordered_map< std::string, Feature > geometry_shader_features;
		std::unordered_map< std::string, Feature > fragment_shader_features;

		if( vertex_shader_source = ParseShaderFromFile( vertex_shader_source_file_path, ShaderType::VERTEX );
			vertex_shader_source )
		{
			auto& shader_source = *vertex_shader_source;

			vertex_source_include_path_array = PreprocessShaderStage_GetIncludeFilePaths( shader_source );
			PreProcessShaderStage_IncludeDirectives( vertex_shader_source_file_path, shader_source, ShaderType::VERTEX );
			vertex_shader_features = PreProcessShaderStage_ParseFeatures( shader_source );
			PreProcessShaderStage_SetFeatures( shader_source, vertex_shader_features, features_to_set );

			if( !CompileShader( shader_source.c_str(), vertex_shader_id, ShaderType::VERTEX ) )
				return false;
		}
		else
			return false;

		feature_map.insert( vertex_shader_features.begin(), vertex_shader_features.end() );

		if( geometry_shader_source_file_path )
		{
			if( geometry_shader_source = ParseShaderFromFile( geometry_shader_source_file_path, ShaderType::GEOMETRY );
				geometry_shader_source )
			{
				auto& shader_source = *geometry_shader_source;

				geometry_source_include_path_array = PreprocessShaderStage_GetIncludeFilePaths( shader_source );
				PreProcessShaderStage_IncludeDirectives( geometry_shader_source_file_path, shader_source, ShaderType::GEOMETRY );
				geometry_shader_features = PreProcessShaderStage_ParseFeatures( shader_source );
				PreProcessShaderStage_SetFeatures( shader_source, geometry_shader_features, features_to_set );

				if( !CompileShader( shader_source.c_str(), geometry_shader_id, ShaderType::GEOMETRY ) )
					return false;
			}
			else
			{
				glDeleteShader( vertex_shader_id );
				return false;
			}

			feature_map.insert( geometry_shader_features.begin(), geometry_shader_features.end() );
		}

		if( fragment_shader_source = ParseShaderFromFile( fragment_shader_source_file_path, ShaderType::FRAGMENT );
			fragment_shader_source )
		{
			auto& shader_source = *fragment_shader_source;

			fragment_source_include_path_array = PreprocessShaderStage_GetIncludeFilePaths( shader_source );
			PreProcessShaderStage_IncludeDirectives( fragment_shader_source_file_path, shader_source, ShaderType::FRAGMENT );
			fragment_shader_features = PreProcessShaderStage_ParseFeatures( shader_source );
			PreProcessShaderStage_SetFeatures( shader_source, fragment_shader_features, features_to_set );

			if( !CompileShader( shader_source.c_str(), fragment_shader_id, ShaderType::FRAGMENT ) )
				return false;
		}
		else
		{
			glDeleteShader( vertex_shader_id );
			if( geometry_shader_id > 0 )
				glDeleteShader( geometry_shader_id );

			return false;
		}

		feature_map.insert( fragment_shader_features.begin(), fragment_shader_features.end() );

		const bool link_result = LinkProgram( vertex_shader_id, geometry_shader_id, fragment_shader_id );

		glDeleteShader( vertex_shader_id );
		if( geometry_shader_source_file_path )
			glDeleteShader( geometry_shader_id );
		glDeleteShader( fragment_shader_id );

		if( link_result )
		{
		#ifdef _DEBUG
			ServiceLocator< GLLogger >::Get().SetLabel( GL_PROGRAM, program_id.Get(), name );
		#endif // _DEBUG

			QueryVertexAttributes();

			GetUniformBookKeepingInfo();
			if( uniform_book_keeping_info.count == 0 )
				return true;

			QueryUniformData();

			ParseShaderSource_VertexLayout( *vertex_shader_source );
			ParseShaderSource_UniformUsageHints( *vertex_shader_source, ShaderType::VERTEX );
			if( geometry_shader_source )
				ParseShaderSource_UniformUsageHints( *geometry_shader_source, ShaderType::GEOMETRY );
			ParseShaderSource_UniformUsageHints( *fragment_shader_source, ShaderType::FRAGMENT );

			QueryUniformData_BlockIndexAndOffsetForBufferMembers();
			QueryUniformBufferData( uniform_buffer_info_map_regular, Uniform::BufferCategory::Regular );
			QueryUniformBufferData_Aggregates( uniform_buffer_info_map_regular );
			QueryUniformBufferData( uniform_buffer_info_map_global, Uniform::BufferCategory::Global );
			QueryUniformBufferData_Aggregates( uniform_buffer_info_map_global );
			QueryUniformBufferData( uniform_buffer_info_map_intrinsic, Uniform::BufferCategory::Intrinsic );
			QueryUniformBufferData_Aggregates( uniform_buffer_info_map_intrinsic );

			CalculateTotalUniformSizes();
			EnumerateUniformBufferCategories();

			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_regular )
				UniformBlockBindingPointManager::RegisterUniformBlock( *this, uniform_buffer_name, uniform_buffer_info );

			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_global )
				UniformBlockBindingPointManager::RegisterUniformBlock( *this, uniform_buffer_name, uniform_buffer_info );

			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_intrinsic )
				UniformBlockBindingPointManager::RegisterUniformBlock( *this, uniform_buffer_name, uniform_buffer_info );

			last_write_time_map.emplace( vertex_source_path, std::filesystem::last_write_time( vertex_source_path ) );
			if( not geometry_source_path.empty() )
				last_write_time_map.emplace( geometry_source_path, std::filesystem::last_write_time( geometry_source_path ) );
			last_write_time_map.emplace( fragment_source_path, std::filesystem::last_write_time( fragment_source_path ) );
		}

		return link_result;
	}

	void Shader::Bind() const
	{
		glUseProgram( program_id.Get() );
	}

	bool Shader::RecompileFromThis( Shader& new_shader )
	{
		return new_shader.FromFile( vertex_source_path.c_str(), fragment_source_path.c_str(), features_requested, geometry_source_path.empty() ? nullptr : geometry_source_path.c_str() );
	}

	bool Shader::SourceFilesAreModified()
	{
		for( auto& [ source, last_write_time ] : last_write_time_map )
		{
			if( const auto new_last_write_time = std::filesystem::last_write_time( source );
				new_last_write_time != last_write_time )
			{
				last_write_time = new_last_write_time;
				return true;
			}
		}

		return false;
	}

	void Shader::SetUniform( const Uniform::Information& uniform_info, const void* value_pointer )
	{
		switch( uniform_info.type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT					: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const float*			>( value_pointer ) ); return;
			case GL_FLOAT_VEC2				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC3				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC4				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4*		>( value_pointer ) ); return;
			//case GL_DOUBLE					: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const double*		>(  count_array ); return;
			case GL_INT						: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_INT_VEC2				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2I*		>( value_pointer ) ); return;
			case GL_INT_VEC3				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3I*		>( value_pointer ) ); return;
			case GL_INT_VEC4				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4I*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const unsigned int*	>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC2		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC3		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC4		: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4U*		>( value_pointer ) ); return;
			case GL_BOOL					: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const bool*			>( value_pointer ) ); return;
			case GL_BOOL_VEC2				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector2B*		>( value_pointer ) ); return;
			case GL_BOOL_VEC3				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector3B*		>( value_pointer ) ); return;
			case GL_BOOL_VEC4				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Vector4B*		>( value_pointer ) ); return;
			/* Matrices: */
			case GL_FLOAT_MAT2				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix2x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix3x3*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix4x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x3			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix2x3*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x4			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix2x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x2			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix3x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x4			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix3x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x2			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix4x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x3			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const Matrix4x3*		>( value_pointer ) ); return;
			/* Samplers: */
			case GL_SAMPLER_1D				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_2D				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_2D_MULTISAMPLE	: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_3D				: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_CUBE			: SetUniform( uniform_info.location_or_block_index, *reinterpret_cast< const int*			>( value_pointer ) ); return;
		}

		throw std::runtime_error( "ERROR::SHADER::SetUniform( uniform_info, value_pointer ) called for an unknown GL type!" );
	}

	void Shader::SetUniformArray( const Uniform::Information& uniform_info, const void* value_pointer )
	{
		switch( uniform_info.type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT					: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const float*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_VEC2				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_VEC3				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector3*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_VEC4				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector4*		>( value_pointer ), uniform_info.count_array ); return;
			//case GL_DOUBLE					: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const double*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT						: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT_VEC2				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector2I*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT_VEC3				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector3I*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT_VEC4				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector4I*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const unsigned int*	>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT_VEC2		: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector2U*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT_VEC3		: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector3U*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT_VEC4		: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector4U*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_BOOL					: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const bool*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_BOOL_VEC2				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector2B*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_BOOL_VEC3				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector3B*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_BOOL_VEC4				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Vector4B*		>( value_pointer ), uniform_info.count_array ); return;
			/* Matrices: */
			case GL_FLOAT_MAT2				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix2x2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT3				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix3x3*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT4				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix4x4*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT2x3			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix2x3*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT2x4			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix2x4*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT3x2			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix3x2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT3x4			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix3x4*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT4x2			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix4x2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT4x3			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const Matrix4x3*		>( value_pointer ), uniform_info.count_array ); return;
			/* Samplers: */
			case GL_SAMPLER_1D				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_2D				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_2D_MULTISAMPLE	: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_3D				: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_CUBE			: SetUniformArray( uniform_info.location_or_block_index, reinterpret_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
		}

		throw std::runtime_error( "ERROR::SHADER::SetUniformArray( uniform_info, value_pointer ) called for an unknown GL type!" );
	}

/*
 *
 *	PRIVATE API:
 *
 */

	/* Private, for shader recompilation only, called by the Renderer alone. */
	Shader::Shader( Shader&& donor )
		:
		program_id( std::exchange( donor.program_id, {} ) ),
		name( std::exchange( donor.name, "<scheduled-for-deletion>" ) ),

		vertex_source_path( std::move( donor.vertex_source_path ) ),
		geometry_source_path( std::move( donor.geometry_source_path ) ),
		fragment_source_path( std::move( donor.fragment_source_path ) ),

		vertex_source_include_path_array( std::move( donor.vertex_source_include_path_array ) ),
		geometry_source_include_path_array( std::move( donor.geometry_source_include_path_array ) ),
		fragment_source_include_path_array( std::move( donor.fragment_source_include_path_array ) ),

		features_requested( std::move( donor.features_requested ) ),
		feature_map( std::move( donor.feature_map ) ),

		uniform_info_map( std::move( donor.uniform_info_map ) ),

		uniform_buffer_info_map_regular( std::move( donor.uniform_buffer_info_map_regular ) ),
		uniform_buffer_info_map_global( std::move( donor.uniform_buffer_info_map_global ) ),
		uniform_buffer_info_map_intrinsic( std::move( donor.uniform_buffer_info_map_intrinsic ) ),

		last_write_time_map( std::exchange( donor.last_write_time_map, {} ) ),

		uniform_book_keeping_info( std::move( donor.uniform_book_keeping_info ) ),

		vertex_layout_source( std::move( donor.vertex_layout_source ) ),
		vertex_layout_active( std::move( donor.vertex_layout_active ) )
	{
	}

	/* Private, for shader recompilation only, called by the Renderer alone. */
	Shader& Shader::operator=( Shader&& donor )
	{
		program_id = std::exchange( donor.program_id, {} );
		name       = std::exchange( donor.name, "<scheduled-for-deletion>" );

		vertex_source_path   = std::move( donor.vertex_source_path );
		geometry_source_path = std::move( donor.geometry_source_path );
		fragment_source_path = std::move( donor.fragment_source_path );

		vertex_source_include_path_array   = std::move( donor.vertex_source_include_path_array );
		geometry_source_include_path_array = std::move( donor.geometry_source_include_path_array );
		fragment_source_include_path_array = std::move( donor.fragment_source_include_path_array );

		features_requested = std::move( donor.features_requested );
		feature_map        = std::move( donor.feature_map );

		uniform_info_map = std::move( donor.uniform_info_map );

		uniform_buffer_info_map_regular   = std::move( donor.uniform_buffer_info_map_regular );
		uniform_buffer_info_map_global    = std::move( donor.uniform_buffer_info_map_global );
		uniform_buffer_info_map_intrinsic = std::move( donor.uniform_buffer_info_map_intrinsic );

		last_write_time_map = std::exchange( donor.last_write_time_map, {} );

		uniform_book_keeping_info = std::move( donor.uniform_book_keeping_info );

		vertex_layout_source = std::move( donor.vertex_layout_source );
		vertex_layout_active = std::move( donor.vertex_layout_active );

		return *this;
	}

	std::optional< std::string > Shader::ParseShaderFromFile( const char* file_path, const ShaderType shader_type )
	{
		const std::string error_prompt( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) + "::FILE_NOT_SUCCESSFULLY_READ\n\tShader name: " + name + "\n" );

		if( const auto source = Engine::Utility::ReadFileIntoString( file_path, error_prompt.c_str() );
			source )
			return *source;

		ServiceLocator< GLLogger >::Get().Error( error_prompt );

		return std::nullopt;
	}

	std::vector< std::string > Shader::PreprocessShaderStage_GetIncludeFilePaths( std::string shader_source ) const
	{
		std::regex pattern( R"(#include\s+"\s*(\S+)\s*")" );
		std::smatch matches;

		std::vector< std::string > includes;

		if( std::regex_search( shader_source, matches, pattern ) )
		{
			do
			{
				const auto& match = matches[ 1 ]; /* First match is the pattern itself. */
				if( match.length() != 0 && match.matched )
					includes.emplace_back( match );

				shader_source = matches.suffix();
			}
			while( std::regex_search( shader_source, matches, pattern ) );
		}

		return includes;
	}

	void Shader::PreprocessShaderStage_StripDefinesToBeSet( std::string& shader_source_to_modify, const std::vector< std::string >& features_to_set )
	{
		std::regex pattern( R"(#define\s+([_[:alnum:]]+)\s*(\S+)?\s*?\r?\n)" ); // ([_[:alnum:]]+) is to FAIL the Regex for non alnum & non-underscore characters, to exclude macros mostly.
		std::smatch matches;

		std::string shader_source_copy( shader_source_to_modify );

		while( std::regex_search( shader_source_copy, matches, pattern ) )
		{
			if( std::find_if( features_to_set.begin(), features_to_set.end(), [ & ]( const std::string& feature )
				{ return feature.find( matches[ 1 ] ) != std::string::npos; } ) != features_to_set.end() )
				shader_source_to_modify = std::string( matches.prefix() ) + std::string( matches.suffix() );

			shader_source_copy = matches.suffix();
		}
	}

	std::unordered_map< std::string, Shader::Feature > Shader::PreProcessShaderStage_ParseFeatures( std::string shader_source )
	{
		std::unordered_map< std::string, Feature > features;

		/* Parse declarations via "#pragma feature <feature_name>" syntax: */
		{
			std::regex pattern( R"(#pragma\s+feature\s*(\S+))" );
			std::smatch matches;

			if( std::regex_search( shader_source, matches, pattern ) )
			{

				do
				{
					const auto& match_feature_name = matches[ 1 ]; /* First match is the pattern itself. */
					if( match_feature_name.length() != 0 && match_feature_name.matched )
						features.try_emplace( match_feature_name, std::nullopt, false );

					shader_source = matches.suffix();
				}
				while( std::regex_search( shader_source, matches, pattern ) );
			}
		}

		/* Parse definitions via "#define <feature_name> <optional_value>" syntax: */
		{
			std::regex pattern( R"(#define\s+([_[:alnum:]]+)\s*(\S+)?\s*\r?\n)" ); // ([_[:alnum:]]+) is to FAIL the Regex for non alnum & non-underscore characters, to exclude macros mostly.
			std::smatch matches;

			if( std::regex_search( shader_source, matches, pattern ) )
			{
				do
				{
					const auto& match_feature_name = matches[ 1 ]; /* First match is the pattern itself. */
					if( match_feature_name.length() != 0 && match_feature_name.matched )
					{
						if( matches.size() > 2 && matches[ 2 ].length() != 0 && matches[ 2 ].matched )
						{
							const auto& match_feature_value = matches[ 2 ];
							features.try_emplace( match_feature_name, match_feature_value, true );
						}
						else
							features.try_emplace( match_feature_name, std::nullopt, true );
					}

					shader_source = matches.suffix();
				}
				while( std::regex_search( shader_source, matches, pattern ) );
			}
		}

		return features;
	}

	void Shader::PreProcessShaderStage_SetFeatures( std::string& shader_source_to_modify,
													std::unordered_map< std::string, Feature >& defined_features,
													const std::vector< std::string >& features_to_set )
	{
		const auto first_new_line = shader_source_to_modify.find( "\n" );

		/* Remove all #defines THAT ARE SET by the client code from the shader, so that we can add the modified versions all in one go later.
		 * This saves us from the work of finding/replacing lines of #defines individually.
		 * We also do not remove the #define lines of macros & Features that are NOT SET by the client code. */
		PreprocessShaderStage_StripDefinesToBeSet( shader_source_to_modify, features_to_set );

		std::string define_directives_combined;
		for( const auto& feature_definition : features_to_set )
		{
			auto splitted( Utility::String::Split( feature_definition, ' ' ) );
			const std::string feature_name( std::move( splitted.front() ) );

			if( auto iterator = defined_features.find( feature_name );
				iterator != defined_features.cend() )
			{
				auto& defined_feature = iterator->second;

				defined_feature.is_set = true;

				if( splitted.size() > 1 )
				{
					const std::string feature_value( std::move( splitted[ 1 ] ) );
					defined_feature.value  = feature_value;

					define_directives_combined += "#define " + feature_name + " " + feature_value + "\n";
				}
				else
					define_directives_combined += "#define " + feature_name + "\n";
			}
		}

		if( not define_directives_combined.empty() )
			shader_source_to_modify = shader_source_to_modify.substr( 0, first_new_line + 1 ) + define_directives_combined + shader_source_to_modify.substr( first_new_line + 1 );
	}

	bool Shader::PreProcessShaderStage_IncludeDirectives( const std::filesystem::path& shader_source_path, std::string& shader_source_to_modify, const ShaderType shader_type )
	{
		static char error_string[ 256 ];

		const std::string shader_file_name( shader_source_path.filename().string() );
		const std::filesystem::path directory_path( shader_source_path.parent_path() );

		std::unique_ptr< char* > preprocessed_source = std::make_unique< char* >( stb_include_string( shader_source_to_modify.data(),
																									  nullptr,
																									  const_cast< char* >( directory_path.string().c_str() ),
																									  const_cast< char* >( shader_file_name.c_str() ),
																									  error_string ) );
		if( not *preprocessed_source )
		{
			const std::string error_prompt( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) + "::INCLUDE_FILE_NOT_SUCCESSFULLY_READ\n\tShader name: " + name + "\n\t" 
											+ error_string );
			LogErrors( error_prompt );
			return false;
		}

		shader_source_to_modify = *preprocessed_source;
		return true;
	}

	bool Shader::CompileShader( const char* source, unsigned int& shader_id, const ShaderType shader_type )
	{
		shader_id = glCreateShader( ShaderTypeID( shader_type ) );
		glShaderSource( shader_id, /* how many strings: */ 1, &source, NULL );
		glCompileShader( shader_id );

		int success = false;
		glGetShaderiv( shader_id, GL_COMPILE_STATUS, &success );
		if( !success )
		{
			LogErrors_Compilation( shader_id, shader_type );
			return false;
		}

		return true;
	}

	bool Shader::LinkProgram( const unsigned int vertex_shader_id, const unsigned int fragment_shader_id )
	{
		return LinkProgram( vertex_shader_id, 0, fragment_shader_id );
	}


	bool Shader::LinkProgram( const unsigned int vertex_shader_id, const unsigned int geometry_shader_id, const unsigned int fragment_shader_id )
	{
		program_id = ID( glCreateProgram() );

		glAttachShader( program_id.Get(), vertex_shader_id );
		if( geometry_shader_id > 0 )
			glAttachShader( program_id.Get(), geometry_shader_id );
		glAttachShader( program_id.Get(), fragment_shader_id );

		glLinkProgram( program_id.Get() );

		int success;
		glGetProgramiv( program_id.Get(), GL_LINK_STATUS, &success );
		if( !success )
		{
			LogErrors_Linking();
			return false;
		}

		return true;
	}

#pragma region Unnecessary Old Stuff
///* Expects: To be called after the shader whose source is passed is compiled & linked successfully. */
//	std::string Shader::ShaderSource_CommentsStripped( const std::string& shader_source )
//	{
//		/* This function is called AFTER the shader is compiled & linked. So it is known for a fact that the block comments have matching pairs of begin/end symbols. */
//
//		auto Strip = [ &shader_source ]( const std::string& source_string, const std::string& comment_begin_token, const std::string& comment_end_token,
//										 const bool do_not_erase_new_line = false )->std::string
//		{
//			std::string stripped_shader_source;
//			std::size_t current_pos = source_string.find( comment_begin_token, 0 ), last_begin_pos = 0;
//
//			while( current_pos != std::string::npos )
//			{
//				const std::size_t comment_start_pos = current_pos;
//				const std::size_t comment_end_pos = source_string.find( comment_end_token, current_pos + comment_begin_token.size() );
//
//				stripped_shader_source += source_string.substr( last_begin_pos, comment_start_pos - last_begin_pos );
//				last_begin_pos = comment_end_pos + comment_end_token.size() - ( int )do_not_erase_new_line;
//
//				current_pos = source_string.find( comment_begin_token, last_begin_pos );
//			}
//
//			/* Add the remaining part of the source string.*/
//			stripped_shader_source += source_string.substr( last_begin_pos );
//
//			return stripped_shader_source;
//		};
//
//		return Strip( Strip( shader_source, "/*", "*/" ), "//", "\n", true );
//	}
#pragma endregion

	void Shader::ParseShaderSource_UniformUsageHints( const std::string& shader_source, const ShaderType shader_type )
	{
		std::size_t current_pos = 0;
		do
		{
			if( const auto comment_block_start_token_pos = shader_source.find( "/*", current_pos );
				comment_block_start_token_pos != std::string::npos )
			{
				current_pos = comment_block_start_token_pos;
				if( const auto comment_block_end_token_pos = shader_source.find( "*/", current_pos );
					comment_block_end_token_pos != std::string::npos )
				{
					if( const auto hint_token_pos = shader_source.find( "_hint_", current_pos );
						hint_token_pos != std::string::npos && hint_token_pos < comment_block_end_token_pos )
					{
						if( const auto token_end_pos = shader_source.find( ' ', current_pos + 6 /* to get past "_hint_" */ );
							token_end_pos != std::string::npos )
						{
							current_pos = hint_token_pos + 6 /* to get past "_hint_" */;

							// TODO: Support parsing #defines from _hint_array_WIDTH_HEIGHT.

							int array_dimensions[ 3 ] = { 0, 0, 0 };
							auto dimension_current_pos = current_pos;
							for( auto i = 0; i < 3; i++ )
							{
								if( const auto hint_array_dimensions_0_pos = shader_source.find( '_', dimension_current_pos );
									hint_array_dimensions_0_pos != std::string::npos && hint_array_dimensions_0_pos < token_end_pos )
								{
									array_dimensions[ i ] = std::atoi( shader_source.data() + hint_array_dimensions_0_pos + 1 );

									dimension_current_pos = hint_array_dimensions_0_pos + 1;
								}
								else
									break;
							}
							
							const std::string hint_string( shader_source.substr( current_pos, token_end_pos - current_pos ) );
							const auto hint = UsageHint_StringToEnum( hint_string );

							if( const auto semicolon_pos = std::string_view( shader_source ).rfind( ';', comment_block_start_token_pos);
								semicolon_pos != std::string::npos )
							{
								if( const auto delimiter_whitespace_pos = std::string_view( shader_source ).rfind( ' ', semicolon_pos );
									delimiter_whitespace_pos != std::string::npos )
								{
									std::string uniform_name;

									if( hint == UsageHint::AsArray )
									{
										if( const auto first_square_bracket_pos = std::string_view( shader_source ).rfind( '[', delimiter_whitespace_pos );
											first_square_bracket_pos != std::string::npos )
										{
											if( const auto delimiter_whitespace_before_bracket_pos = std::string_view( shader_source ).rfind( ' ', first_square_bracket_pos );
												delimiter_whitespace_before_bracket_pos != std::string::npos )
											{
												uniform_name = shader_source.substr( delimiter_whitespace_before_bracket_pos + 1, first_square_bracket_pos - delimiter_whitespace_before_bracket_pos - 1 ) +
																"[0]"; // GLSL array uniform naming convention.
											}
										}

									}
									else
										uniform_name = shader_source.substr( delimiter_whitespace_pos + 1, semicolon_pos - delimiter_whitespace_pos - 1 );

									if( auto iterator = uniform_info_map.find( uniform_name );
										iterator != uniform_info_map.cend() && iterator->second.usage_hint != UsageHint::Unassigned && hint != iterator->second.usage_hint )
									{
										const std::string complete_error_string( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) +
																				 "::POST-LINK::PARSE_UNIFORM_USAGE_HINTS:\nShader name: " + name +
																				 "\nMismatched uniform usage hints detected." );

										LogErrors( complete_error_string );
									}
									else
									{
										if( uniform_info_map.contains( uniform_name ) )
										{
											uniform_info_map[ uniform_name ].usage_hint = hint;
											std::memcpy( uniform_info_map[ uniform_name ].usage_hint_array_dimensions, array_dimensions, sizeof( int ) * 3 );
										}
									}
								}
							}
						}
					}
					else
						current_pos++; // To prevent endless loop on find() calls.
				}
				else
					current_pos++; // To prevent endless loop on find() calls.
			}
			else if( ( current_pos = shader_source.find( "//", current_pos ) ) != std::string::npos )
			{
				if( const auto hint_token_pos = shader_source.find( "_hint_", current_pos );
					hint_token_pos != std::string::npos )
				{
					throw std::logic_error( "Not implemented yet!" );
				}
				else
					current_pos++; // To prevent endless loop on find() calls.
			}
		}
		while( current_pos != std::string::npos && current_pos != 0 );
	}

	void Shader::ParseShaderSource_VertexLayout( std::string shader_source )
	{
		/* Example:
			...
			layout (location = 0) in vec3 position;
			layout (location = 1) in vec2 tex_coords;
			...
		*/

		/*												  location			   type	   name			*/
		std::regex pattern( R"(layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+(\w+)\s+(\w+)\s*;)" );
		std::smatch matches;

		std::vector< VertexAttribute > attributes;

		while( std::regex_search( shader_source, matches, pattern ) )
		{
			const std::string location_string( matches[ 1 ] );
			const std::string location_type( matches[ 2 ] );
			const std::string location_name( matches[ 3 ] );

			if( location_string.length() != 0 && matches[ 1 ].matched && 
				  location_type.length() != 0 && matches[ 2 ].matched &&
				  location_name.length() != 0 && matches[ 3 ].matched )
			{
				const GLenum type( GL::Type::TypeOf( location_type.c_str() ) );
				/* Source attributes */
				attributes.emplace_back( GL::Type::CountOf( type ), GL::Type::ComponentTypeOf( type ), false /* => instance info does not matter. */,
										( unsigned int )std::stoi( location_string ) );
			}

			shader_source = matches.suffix();
		}

		if( not attributes.empty() )
		{
			std::sort( attributes.begin(), attributes.end(), []( const VertexAttribute& left, const VertexAttribute& right ) { return left.location < right.location; } );
			vertex_layout_source = VertexLayout( attributes );
		}
	}

	void Shader::QueryVertexAttributes()
	{
		int active_attribute_count;
		glGetProgramiv( program_id.Get(), GL_ACTIVE_ATTRIBUTES, &active_attribute_count );

		static char attribute_name[ 255 ];

		std::vector< VertexAttribute > attributes;
		attributes.reserve( active_attribute_count );
		for( auto attribute_index = 0; attribute_index < active_attribute_count; attribute_index++ )
		{
			int attribute_name_length, attribute_size;
			GLenum attribute_vector_type;
			glGetActiveAttrib( program_id.Get(), attribute_index, 255, &attribute_name_length, &attribute_size, &attribute_vector_type, attribute_name );
			const unsigned int attribute_location = glGetAttribLocation( program_id.Get(), attribute_name );

			GLint divisor_data;
			glGetIntegeri_v( GL_VERTEX_BINDING_DIVISOR, attribute_location, &divisor_data );
			const bool is_instanced = divisor_data >= 1;

			attributes.emplace_back( GL::Type::CountOf( attribute_vector_type ), GL::Type::ComponentTypeOf( attribute_vector_type ), is_instanced, attribute_location );
		}

		std::sort( attributes.begin(), attributes.end(), []( const VertexAttribute& left, const VertexAttribute& right ) { return left.location < right.location; } );
		vertex_layout_active = VertexLayout( attributes );
	}

	void Shader::GetUniformBookKeepingInfo()
	{
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORMS, &uniform_book_keeping_info.count );
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_book_keeping_info.name_max_length );
		uniform_book_keeping_info.name_holder = std::string( uniform_book_keeping_info.name_max_length, '?' );
	}

	/* Expects empty input vectors. */
	bool Shader::GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( const int active_uniform_count,
																			 std::vector< unsigned int >& block_indices, std::vector< unsigned int >& corresponding_uniform_indices ) const
	{
		corresponding_uniform_indices.resize( active_uniform_count );
		block_indices.resize( active_uniform_count );
		std::iota( corresponding_uniform_indices.begin(), corresponding_uniform_indices.end(), 0 );

		glGetActiveUniformsiv( program_id.Get(), active_uniform_count, corresponding_uniform_indices.data(), GL_UNIFORM_BLOCK_INDEX, reinterpret_cast< int* >( block_indices.data() ) );

		int block_count = 0;
		for( auto uniform_index = 0; uniform_index < active_uniform_count; uniform_index++ )
		{
			if( const auto block_index = block_indices[ uniform_index ];
				block_index != -1 )
			{
				/* Overwrite vector elements starting from the beginning. This should be safe as the uniform_index should be greater than block_count at this point.
				 * So we should be overwriting previous elements as we go through all uniform indices. */
				block_indices[ block_count ]                 = block_index;
				corresponding_uniform_indices[ block_count ] = uniform_index;
				block_count++;
			}
		}

		if( block_count )
		{
			block_indices.resize( block_count );
			corresponding_uniform_indices.resize( block_count );
			return true;
		}

		return false;
	}

	void Shader::QueryUniformData()
	{
		int offset = 0;
		for( auto uniform_index = 0; uniform_index < uniform_book_keeping_info.count; uniform_index++ )
		{
			/*
			 * glGetActiveUniform has a parameter named "size", but its actually the size of the array. So for singular types like int, float, vec2, vec3 etc. the value returned is 1.
			 */
			int array_element_count = 0, length_dontCare = 0;
			GLenum type;
			glGetActiveUniform( program_id.Get(), uniform_index, uniform_book_keeping_info.name_max_length,
								&length_dontCare, &array_element_count, &type, 
								uniform_book_keeping_info.name_holder.data() );

			const int size = GL::Type::SizeOf( type );

			const auto location = glGetUniformLocation( program_id.Get(), uniform_book_keeping_info.name_holder.c_str() );

			const bool is_buffer_member = location == -1;

			uniform_info_map[ uniform_book_keeping_info.name_holder.c_str() ] = 
			{
				.location_or_block_index = location,
				.size                    = size,
				.offset                  = is_buffer_member ? -1 : offset,
				.count_array             = array_element_count,
				.type                    = type,
				.is_buffer_member		 = is_buffer_member,
				.editor_name			 = UniformEditorName( uniform_book_keeping_info.name_holder ),
				.usage_hint				 = UsageHint::Unassigned
			};

			offset += !is_buffer_member * size * array_element_count;
		}
	}

	void Shader::QueryUniformData_BlockIndexAndOffsetForBufferMembers()
	{
		std::vector< unsigned int > block_indices, corresponding_uniform_indices;
		if( not GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( uniform_book_keeping_info.count, block_indices, corresponding_uniform_indices ) )
			return;

		std::vector< int > corresponding_offsets( corresponding_uniform_indices.size() );
		glGetActiveUniformsiv( program_id.Get(), ( int )corresponding_uniform_indices.size(), corresponding_uniform_indices.data(), GL_UNIFORM_OFFSET, corresponding_offsets.data() );

		for( int index = 0; index < corresponding_uniform_indices.size(); index++ )
		{
			const auto uniform_index = corresponding_uniform_indices[ index ];
			const auto block_index   = block_indices[ index ];
			const auto offset        = corresponding_offsets[ index ];

			int length = 0;
			glGetActiveUniformName( program_id.Get(), uniform_index, uniform_book_keeping_info.name_max_length, &length, uniform_book_keeping_info.name_holder.data() );

			const auto& uniform_name = uniform_book_keeping_info.name_holder.c_str();

			/* 'size', 'type' and 'is_buffer_member' was already initialized during query of default block uniforms. Update the remaining information for block member uniforms: */
			auto& uniform_info = uniform_info_map[ uniform_name ];

			uniform_info.location_or_block_index = ( int )block_index;
			uniform_info.offset                  = offset;
		}
	}

	void Shader::QueryUniformBufferData( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_info_map, const Uniform::BufferCategory category_of_interest )
	{
		int active_uniform_block_count = 0;
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORM_BLOCKS, &active_uniform_block_count );

		if( active_uniform_block_count == 0 )
			return;

		int uniform_block_name_max_length = 0;
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniform_block_name_max_length );
		std::string name( uniform_block_name_max_length, '?' );

		int offset = 0;
		for( int uniform_block_index = 0; uniform_block_index < active_uniform_block_count; uniform_block_index++ )
		{
			int length = 0;
			glGetActiveUniformBlockName( program_id.Get(), uniform_block_index, uniform_block_name_max_length, &length, name.data() );
			
			int size;
			glGetActiveUniformBlockiv( program_id.Get(), uniform_block_index, GL_UNIFORM_BLOCK_DATA_SIZE,	&size );

			const auto category = Uniform::DetermineBufferCategory( name );

			if( category == category_of_interest )
			{
				auto& uniform_buffer_information = uniform_buffer_info_map[ name.c_str() ] =
				{
					.binding_point = -1, // This will be filled later via BufferManager::ConnectBufferToBlock().
					.size          = size,
					.offset        = offset,
					.category	   = category
				};

				/* Add members and set their block indices. */
				for( auto& [ uniform_name, uniform_info ] : uniform_info_map )
				{
					if( uniform_info.is_buffer_member && uniform_info.location_or_block_index == uniform_block_index )
						uniform_buffer_information.members_map.emplace( uniform_name.data(), &uniform_info );
				}

				offset += size;
			}
		}
	}

	void Shader::QueryUniformBufferData_Aggregates( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_info_map )
	{
		std::vector< Uniform::Information* > members_map;

		for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
		{
			using BufferInfoPair = std::pair< const std::string, Uniform::Information* >;
			std::vector< BufferInfoPair* > uniform_buffer_info_sorted_by_offset;
			uniform_buffer_info_sorted_by_offset.reserve( uniform_buffer_info.members_map.size() );
			for( auto& pair : uniform_buffer_info.members_map )
				uniform_buffer_info_sorted_by_offset.push_back( &pair );

			std::stable_sort( uniform_buffer_info_sorted_by_offset.begin(), uniform_buffer_info_sorted_by_offset.end(),
							  []( const BufferInfoPair* left, const BufferInfoPair* right )
			{
				return left->second->offset < right->second->offset;
			} );

			for( int i = 0; i < uniform_buffer_info_sorted_by_offset.size(); i++ ) // -> Outer for loop.
			{
				const auto& buffer_info_pair = uniform_buffer_info_sorted_by_offset[ i ];
				const auto& uniform_name     = buffer_info_pair->first;
				const auto& uniform_info     = buffer_info_pair->second;

				const std::string_view uniform_name_without_buffer_name( uniform_name.cbegin() + ( uniform_name.starts_with( uniform_buffer_name )
																								   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																								   : 0 ),
																		 uniform_name.cend() );

				if( const auto bracket_pos = uniform_name_without_buffer_name.find( '[' );
					bracket_pos != std::string_view::npos )
				{
					int stride = uniform_info->size, member_count = 1;

					members_map.push_back( uniform_info );

					/* Find the other members of the array's CURRENT element: */
					bool done_processing_array_element = false;
					int j = i + 1;
					for( ; j < uniform_buffer_info_sorted_by_offset.size() && not done_processing_array_element; j++ )
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						const std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + ( other_uniform_name.starts_with( uniform_buffer_name )
																													   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																													   : 0 ),
																					   other_uniform_name.cend() );

						// + 2 to include the index, which has to match for a given array element.
						if( other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, bracket_pos + 2 ) ) )
						{
							stride += other_uniform_info->size;
							member_count++;
							members_map.push_back( other_uniform_info );
						}
						else
							done_processing_array_element = true;
					}

					member_count = j - i - ( done_processing_array_element ? 1 : 0 ); // -1 because j had been incremented once more before the for loop ended.
					stride       = Math::RoundToMultiple_PowerOf2( stride, sizeof( Vector4 ) ); // Std140 dictates this.

					if( done_processing_array_element )
						j--;

					done_processing_array_element = false;
					/* Now count the elements. If the array had 1 element, done_processing_array_element will cause the below loop to skip immediately. */
					for( ; j < uniform_buffer_info_sorted_by_offset.size() && not done_processing_array_element; j++ )
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						const std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + ( other_uniform_name.starts_with( uniform_buffer_name )
																													   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																													   : 0 ),
																					   other_uniform_name.cend() );

						// No +2 this time; we're looking for the array name only.
						if( !other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, bracket_pos ) ) )
							done_processing_array_element = true;
					}

					const int element_count = ( j - i - ( done_processing_array_element ? 1 : 0 ) ) / member_count; // -1 because j had been incremented once more before the for loop ended.
					
					const auto aggregate_name( uniform_name_without_buffer_name.substr( 0, bracket_pos ) );
					uniform_buffer_info.members_array_map.emplace( aggregate_name,
																   Uniform::BufferMemberInformation_Array
																   { 
																		.offset        = uniform_info->offset,
																		.stride        = stride,
																		.element_count = element_count,
																		.editor_name   = UniformEditorName_BufferMemberAggregate( aggregate_name ),
																		.members_map   = members_map,
																   } );

					members_map.clear(); // Re-use the existing vector with its grown capacity.

					i += j - i - ( done_processing_array_element ? 1 : 0 ) - 1; // Outer for loop's i++ will also increment i, that's why there is a minus 1. The other -1 is the same as the ones above; for loop increments the J for one last time.
				}
				else if( const auto dot_pos = uniform_name_without_buffer_name.find( '.' );
						 dot_pos != std::string_view::npos )
				{
					int member_count = 1;
					int size = uniform_info->size;

					members_map.push_back( uniform_info );

					// Don't need to check whether the loop ended naturally or not in this case; If the loop ends naturally, then incrementing i is not important any more.

					for( int j = i + 1; j < uniform_buffer_info_sorted_by_offset.size(); j++ ) // -> Inner for loop.
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						const std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + ( other_uniform_name.starts_with( uniform_buffer_name )
																													   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																													   : 0 ),
																					   other_uniform_name.cend() );

						if( other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, dot_pos ) ) )
						{
							size += other_uniform_info->size;

							members_map.push_back( other_uniform_info );
						}
						else
						{
							size = Math::RoundToMultiple_PowerOf2( size, sizeof( Vector4 ) ); // Std140 dictates this.

							member_count = j - i;
							i += member_count - 1; // Outer for loop's i++ will also increment i, that's why there is a minus 1.

							break;
						}
					}

					const auto aggregate_name( uniform_name_without_buffer_name.substr( 0, dot_pos ) );
					uniform_buffer_info.members_struct_map.emplace( aggregate_name,
																	Uniform::BufferMemberInformation_Struct
																	{
																		 .offset      = uniform_info->offset,
																		 .size        = size,
																		 .editor_name = UniformEditorName_BufferMemberAggregate( aggregate_name ),
																		 .members_map = members_map
																	} );

					members_map.clear(); // Re-use the existing vector with its grown capacity.
				}
				else
				{
					uniform_buffer_info.members_single_map.emplace( uniform_name_without_buffer_name, uniform_info );
				}
			}
		}
	}

	void Shader::CalculateTotalUniformSizes()
	{
		uniform_book_keeping_info.default_block_size = 0;

		/* Sum of default block (i.e., not in any explicit Uniform Buffer) uniforms: */
		for( const auto& [ uniform_name, uniform_info ] : uniform_info_map )
			if( not uniform_info.is_buffer_member ) // Skip buffer members, as their layout (std140) means their total buffer size is calculated differently.
				uniform_book_keeping_info.default_block_size += uniform_info.size * uniform_info.count_array;

		/* Now add buffer block sizes (calculated before): */
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_regular )
			uniform_book_keeping_info.regular_total_size = uniform_buffer_info.size;
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_global )
			uniform_book_keeping_info.global_total_size = uniform_buffer_info.size;
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_intrinsic )
			uniform_book_keeping_info.intrinsic_total_size = uniform_buffer_info.size;

		uniform_book_keeping_info.total_size = uniform_book_keeping_info.default_block_size + uniform_book_keeping_info.TotalSize_Blocks();
	}

	void Shader::EnumerateUniformBufferCategories()
	{
		uniform_book_keeping_info.regular_block_count   = ( int )uniform_buffer_info_map_regular.size();
		uniform_book_keeping_info.global_block_count    = ( int )uniform_buffer_info_map_global.size();
		uniform_book_keeping_info.intrinsic_block_count = ( int )uniform_buffer_info_map_intrinsic.size();
	}

	const Uniform::Information& Shader::GetUniformInformation( const std::string& uniform_name )
	{
	#ifdef _DEBUG
		try
		{
			return uniform_info_map.at( uniform_name );
		}
		catch( const std::exception& )
		{
			throw std::runtime_error( R"(ERROR::SHADER::GetUniformInformation(): uniform ")" + std::string( uniform_name ) + R"(" does not exist!)" );
		}
	#else
		return uniform_info_map[ uniform_name ];
	#endif // DEBUG
	}

	void Shader::LogErrors( const std::string& error_string ) const
	{
		std::cerr << error_string;
#if defined( _WIN32 ) && defined( _DEBUG )
		if( IsDebuggerPresent() )
			OutputDebugStringA( ( "\n" + error_string + "\n" ).c_str() );
#endif // _WIN32 && _DEBUG

		ServiceLocator< GLLogger >::Get().Error( error_string );

		throw std::logic_error( error_string );
	}

	void Shader::LogErrors_Compilation( const int shader_id, const ShaderType shader_type ) const
	{
		char info_log[ 512 ];
		glGetShaderInfoLog( shader_id, 512, NULL, info_log );

		const std::string complete_error_string( std::string( "ERROR::SHADER::" ) + ShaderTypeString( shader_type ) + "::COMPILE:\nShader name: " + name + FormatErrorLog( info_log ) );
		LogErrors( complete_error_string );
	}

	void Shader::LogErrors_Linking() const
	{
		char info_log[ 512 ];
		glGetProgramInfoLog( program_id.Get(), 512, NULL, info_log );

		const std::string complete_error_string( "ERROR::SHADER::PROGRAM::LINK:\nShader name: " + name + FormatErrorLog( info_log ) );
		LogErrors( complete_error_string );
	}

	std::string Shader::FormatErrorLog( const char* log ) const
	{
		std::string error_log_string( log );
		Utility::String::Replace( error_log_string, "\n", "\n    " );
		return "\n    " + error_log_string;
	}

	std::string Shader::UniformEditorName( const std::string_view original_name )
	{
		const auto null_terminator_pos = original_name.find( '\0' );
		std::string editor_name( original_name.cbegin(), null_terminator_pos != std::string::npos
																? original_name.cbegin() + null_terminator_pos + 1
																: original_name.cend() );

		/* First get rid of parent struct/block names: */
		if( const auto last_dot_pos = editor_name.find_last_of( '.' );
			last_dot_pos != std::string::npos )
			editor_name = editor_name.substr( last_dot_pos + 1 );

		if( editor_name.compare( 0, 7, "_GLOBAL", 7 ) == 0 )
			editor_name.erase( 0, 7 );
		else if( editor_name.compare( 0, 8, "_REGULAR", 8 ) == 0 )
			editor_name.erase( 0, 8 );
		else if( editor_name.compare( 0, 10, "_INTRINSIC", 10 ) == 0 )
			editor_name.erase( 0, 10 );

		std::replace( editor_name.begin(), editor_name.end(), '_', ' ' );

		if( editor_name.compare( 0, 8, "UNIFORM ", 8 ) == 0 || editor_name.compare( 0, 8, "uniform ", 8 ) == 0 )
			editor_name.erase( 0, 8 );

		if( std::isalpha( editor_name[ 0 ] ) )
			editor_name[ 0 ] = std::toupper( editor_name[ 0 ] );
		else if( editor_name.starts_with( ' ' ) )
			editor_name = editor_name.erase( 0, 1 );

		for( auto index = 1; index < editor_name.size(); index++ )
			if( editor_name[ index - 1 ] == ' ' )
				editor_name[ index ] = std::toupper( editor_name[ index ] );

		/* Get rid of the 0 inside [0] for arrays: */
		if( const auto bracket_zero_pos = editor_name.find( "[0]" );
			bracket_zero_pos != std::string::npos )
			editor_name.erase( bracket_zero_pos + 1, 1 );

		return editor_name;
	}

	std::string Shader::UniformEditorName_BufferMemberAggregate( const std::string_view aggregate_name )
	{
		std::string editor_name( aggregate_name );

		std::replace( editor_name.begin(), editor_name.end(), '_', ' ' );

		if( std::isalpha( editor_name[ 0 ] ) )
			editor_name[ 0 ] = std::toupper( editor_name[ 0 ] );
		else if( editor_name.starts_with( ' ' ) )
			editor_name = editor_name.erase( 0, 1 );

		for( auto index = 1; index < editor_name.size(); index++ )
			if( editor_name[ index - 1 ] == ' ' )
				editor_name[ index ] = std::toupper( editor_name[ index ] );

		return editor_name;
	}
}
