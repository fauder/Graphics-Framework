#pragma once

/*
 * Default Constructors:
 */

#define DEFAULT_COPY_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= default;\
	class_name& operator =( const class_name& ) = default;

#define DEFAULT_MOVE_CONSTRUCTORS( class_name )\
	class_name( class_name&& )			   = default;\
	class_name& operator =( class_name&& ) = default;

#define DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= default;\
	class_name& operator =( const class_name& ) = default;\
	class_name( class_name&& )					= default;\
	class_name& operator =( class_name&& )		= default;

#define CONSTEXPR_DEFAULT_COPY_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = default;\
	constexpr class_name& operator =( const class_name& ) = default;

#define CONSTEXPR_DEFAULT_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( class_name&& )			 = default;\
	constexpr class_name& operator =( class_name&& ) = default;

#define CONSTEXPR_DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = default;\
	constexpr class_name& operator =( const class_name& ) = default;\
	constexpr class_name( class_name&& )				  = default;\
	constexpr class_name& operator =( class_name&& )	  = default;

/*
 * Delete Constructors:
 */

#define DELETE_COPY_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= delete;\
	class_name& operator =( const class_name& ) = delete;

#define DELETE_MOVE_CONSTRUCTORS( class_name )\
	class_name( class_name&& )			   = delete;\
	class_name& operator =( class_name&& ) = delete;

#define DELETE_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= delete;\
	class_name& operator =( const class_name& ) = delete;\
	class_name( class_name&& )					= delete;\
	class_name& operator =( class_name&& )		= delete;

#define CONSTEXPR_DELETE_COPY_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = delete;\
	constexpr class_name& operator =( const class_name& ) = delete;

#define CONSTEXPR_DELETE_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( class_name&& )			 = delete;\
	constexpr class_name& operator =( class_name&& ) = delete;

#define CONSTEXPR_DELETE_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = delete;\
	constexpr class_name& operator =( const class_name& ) = delete;\
	constexpr class_name( class_name&& )				  = delete;\
	constexpr class_name& operator =( class_name&& )	  = delete;

/*
 * Comparison Operators:
 */

#define DEFAULT_EQUALITY_OPERATORS( class_name )\
	bool operator ==( const class_name& ) const = default;\
	bool operator !=( const class_name& ) const = default;

#define CONSTEXPR_DEFAULT_EQUALITY_OPERATORS( class_name )\
	constexpr bool operator ==( const class_name& ) const = default;\
	constexpr bool operator !=( const class_name& ) const = default;
