/*
	Copyright (c) 2019, Electrux
	All rights reserved.
	Using the BSD 3-Clause license for the project,
	main LICENSE file resides in project's root directory.
	Please read that file and understand the license terms
	before using or altering the project.
*/

#include "Instructions.hpp"

const char * InstrCodeStrs[ _IC_LAST ] = {
	"push",
	"pop",
	"store",
	"store_load",

	"build_enum",

	"load_mod",

	"call_func",
	"call_mem_func",
	"struct_decl",
	"struct_mem_decl",

	"struct_field",

	"build_struct",
};