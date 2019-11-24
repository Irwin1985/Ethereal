/*
	Copyright (c) 2019, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include "Base.hpp"

const char * VarTypeStrs[ _VT_LAST ] = {
	"none",
	"nil",
	"int",
	"str",
	"flt",
	"bool",

	"enum",
	"vec",
	"map",
	"tuple",

	"struct",

	"custom",
};

int & float_precision()
{
	static int precision = DEFAULT_FLOAT_PRECISION;
	return precision;
}

var_info_t::var_info_t( const int rc, const int si ,const int pc, const bool ia ) :
	ref_ctr( rc ), src_idx( si ), parse_ctr( pc ), implements_assign( ia ) {}

var_base_t::var_base_t( const VarType type, const bool implements_assign, const int src_idx, const int parse_ctr )
	: m_type( type ), m_info( 1, src_idx, parse_ctr, implements_assign ) {}

var_base_t::~var_base_t() {}

std::string var_base_t::type_str() const { return VarTypeStrs[ m_type ]; }
void var_base_t::assn( var_base_t * b ) {}