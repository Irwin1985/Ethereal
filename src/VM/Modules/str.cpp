/*
	Copyright (c) 2019, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include "../Vars/Base.hpp"
#include "../Core.hpp"

std::vector< std::string > str_delimit( const std::string & str, const char ch );

var_base_t * add( std::vector< var_base_t * > & vars )
{
	std::string res;
	std::string & a = AS_STR( vars[ 1 ] )->get();
	std::string & b = AS_STR( vars[ 0 ] )->get();
	res = a + b;
	return new var_str_t( res, vars[ 1 ]->parse_ctr() );
}

var_base_t * add_assn( std::vector< var_base_t * > & vars )
{
	std::string & a = AS_STR( vars[ 0 ] )->get();
	std::string & b = AS_STR( vars[ 1 ] )->get();
	a += b;
	return vars[ 0 ];
}

var_base_t * len( std::vector< var_base_t * > & vars )
{
	return new var_int_t( ( int )AS_STR( vars[ 0 ] )->get().size(), vars[ 0 ]->parse_ctr() );
}

var_base_t * empty( std::vector< var_base_t * > & vars )
{
	return new var_bool_t( AS_STR( vars[ 0 ] )->get().empty(), vars[ 0 ]->parse_ctr() );
}

var_base_t * clear( std::vector< var_base_t * > & vars )
{
	AS_STR( vars[ 0 ] )->get().clear();
	return nullptr;
}

var_base_t * is_int( std::vector< var_base_t * > & vars )
{
	char * p;
	strtol( AS_STR( vars[ 0 ] )->get().c_str(), & p, 10 );
	return new var_bool_t( * p == 0, vars[ 0 ]->parse_ctr() );
}

var_base_t * to_int( std::vector< var_base_t * > & vars )
{
	return new var_int_t( AS_STR( vars[ 0 ] )->get(), vars[ 0 ]->parse_ctr() );
}

var_base_t * set_at( std::vector< var_base_t * > & vars )
{
	std::string & dat = AS_STR( vars[ 0 ] )->get();
	int idx = vars[ 1 ]->to_int().get_si();
	if( idx < 0 || idx >= ( int )dat.size() ) return vars[ 0 ];
	if( vars[ 2 ]->to_str().size() > 0 ) dat[ idx ] = vars[ 2 ]->to_str()[ 0 ];
	return vars[ 0 ];
}

var_base_t * split( std::vector< var_base_t * > & vars )
{
	const std::string & dat = AS_STR( vars[ 0 ] )->get();
	const char delim = vars.size() > 1 && AS_STR( vars[ 1 ] )->get().size() > 0 ? AS_STR( vars[ 1 ] )->get()[ 0 ] : ' ';
	std::vector< std::string > res = str_delimit( dat, delim );
	std::vector< var_base_t * > res_b;
	for( auto & r : res ) {
		res_b.push_back( new var_str_t( r, vars[ 0 ]->parse_ctr() ) );
	}
	return new var_vec_t( res_b, vars[ 0 ]->parse_ctr() );
}

#define DECL_FUNC_ALLOC__STR( name, oper, ret_type )				\
	var_base_t * name( std::vector< var_base_t * > & vars )			\
	{									\
		auto & lhs = AS_STR( vars[ 1 ] )->get();			\
		auto & rhs = AS_STR( vars[ 0 ] )->get();			\
		return new ret_type( lhs oper rhs, vars[ 1 ]->parse_ctr() );	\
	}

DECL_FUNC_ALLOC__STR( eq, ==, var_bool_t )
DECL_FUNC_ALLOC__STR( ne, !=, var_bool_t )
DECL_FUNC_ALLOC__STR( lt, <, var_bool_t )
DECL_FUNC_ALLOC__STR( le, <=, var_bool_t )
DECL_FUNC_ALLOC__STR( gt, >, var_bool_t )
DECL_FUNC_ALLOC__STR( ge, >=, var_bool_t )

REGISTER_MODULE( str )
{
	vm.funcs.add( { "+", 2, 2, { "str", "str" }, FnType::MODULE, { .modfn = add }, true } );
	vm.funcs.add( { "+=", 2, 2, { "str", "str" }, FnType::MODULE, { .modfn = add_assn }, false } );

	// comparisons
	vm.funcs.add( { "==", 2, 2, { "str", "str" }, FnType::MODULE, { .modfn = eq }, true } );
	vm.funcs.add( { "!=", 2, 2, { "str", "str" }, FnType::MODULE, { .modfn = ne }, true } );
	vm.funcs.add( { "<",  2, 2, { "str", "str" }, FnType::MODULE, { .modfn = lt }, true } );
	vm.funcs.add( { "<=", 2, 2, { "str", "str" }, FnType::MODULE, { .modfn = le }, true } );
	vm.funcs.add( { ">",  2, 2, { "str", "str" }, FnType::MODULE, { .modfn = gt }, true } );
	vm.funcs.add( { ">=", 2, 2, { "str", "str" }, FnType::MODULE, { .modfn = ge }, true } );

	functions_t & strfns = vm.typefuncs[ "str" ];

	strfns.add( { "len", 0, 0, {}, FnType::MODULE, { .modfn = len }, true } );
	strfns.add( { "empty", 0, 0, {}, FnType::MODULE, { .modfn = empty }, true } );
	strfns.add( { "clear", 0, 0, {}, FnType::MODULE, { .modfn = clear }, false } );
	strfns.add( { "is_int", 0, 0, {}, FnType::MODULE, { .modfn = is_int }, true } );
	strfns.add( { "to_int", 0, 0, {}, FnType::MODULE, { .modfn = to_int }, true } );
	strfns.add( { "set_at", 2, 2, { "int", "str" }, FnType::MODULE, { .modfn = set_at }, false } );
	strfns.add( { "split", 0, 1, { "str" }, FnType::MODULE, { .modfn = split }, true } );
}

std::vector< std::string > str_delimit( const std::string & str, const char ch )
{
	std::string temp;
	std::vector< std::string > vec;

	for( auto c : str ) {
		if( c == ch ) {
			if( temp.empty() ) continue;
			vec.push_back( temp );
			temp.clear();
			continue;
		}

		temp += c;
	}

	if( !temp.empty() ) vec.push_back( temp );
	return vec;
}