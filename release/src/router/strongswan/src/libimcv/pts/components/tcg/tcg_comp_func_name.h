/*
 * Copyright (C) 2011 Sansar Choinyambuu
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup pts_tcg_comp_func_name pts_tcg_comp_func_name
 * @{ @ingroup pts
 */

#ifndef PTS_TCG_COMP_FUNC_NAME_H_
#define PTS_TCG_COMP_FUNC_NAME_H_

typedef enum pts_tcg_qualifier_type_t pts_tcg_qualifier_type_t;
typedef enum pts_tcg_comp_func_name_t pts_tcp_comp_func_name_t;

#include <library.h>

/**
 * PTS Component Functional Name Qualifier Flags for the TCG namespace
 * see section 5.2 of PTS Protocol: Binding to TNC IF-M Specification
 *	
 *	 0 1 2 3 4 5
 *  +-+-+-+-+-+-+
 *  |K|S| Type  |
 *  +-+-+-+-+-+-+
 */
#define PTS_TCG_QUALIFIER_FLAG_KERNEL			(1<<5)
#define PTS_TCG_QUALIFIER_FLAG_SUB				(1<<4)

extern char pts_tcg_qualifier_flag_names[];

/**
 * Size of the PTS Component Functional Name Qualifier Type field
 */
#define PTS_TCG_QUALIFIER_TYPE_SIZE				4

/**
 * PTS Component Functional Name Qualifier Types for the TCG namespace
 * see section 5.2 of PTS Protocol: Binding to TNC IF-M Specification
 */
enum pts_tcg_qualifier_type_t {
	/** Unknown */
	PTS_TCG_QUALIFIER_TYPE_UNKNOWN =			0x0,
	/** Trusted Platform */
	PTS_TCG_QUALIFIER_TYPE_TRUSTED =			0x1,
	/** Operating System */
	PTS_TCG_QUALIFIER_TYPE_OS =					0x2,
	/** Graphical User Interface */
	PTS_TCG_QUALIFIER_TYPE_GUI =				0x3,
	/** Application */
	PTS_TCG_QUALIFIER_TYPE_APP =				0x4,
	/** Networking */
	PTS_TCG_QUALIFIER_TYPE_NET =				0x5,
	/** Library */
	PTS_TCG_QUALIFIER_TYPE_LIB =				0x6,
	/** TNC Defined Component */
	PTS_TCG_QUALIFIER_TYPE_TNC =				0x7,
	/** All matching Components */
	PTS_TCG_QUALIFIER_TYPE_ALL =				0xF,
};

extern enum_name_t *pts_tcg_qualifier_type_names;

/**
 * PTS Component Functional Name Binary Enumeration for the TCG namespace
 * see section 5.3 of PTS Protocol: Binding to TNC IF-M Specification
 */
enum pts_tcg_comp_func_name_t {
	/** Ignore */
	PTS_TCG_COMP_FUNC_NAME_IGNORE =				0x0000,
	/** CRTM */
	PTS_TCG_COMP_FUNC_NAME_CRTM =				0x0001,
	/** BIOS */
	PTS_TCG_COMP_FUNC_NAME_BIOS =				0x0002,
	/** Platform Extensions */
	PTS_TCG_COMP_FUNC_NAME_PLATFORM_EXT =		0x0003,
	/** Motherboard Firmware */
	PTS_TCG_COMP_FUNC_NAME_BOARD =				0x0004,
	/** Initial Program Loader */
	PTS_TCG_COMP_FUNC_NAME_INIT_LOADER =		0x0005,
	/** Option ROMs */
	PTS_TCG_COMP_FUNC_NAME_OPT_ROMS =			0x0006,
};

extern enum_name_t *pts_tcg_comp_func_names;

#endif /** PTS_TCG_COMP_FUNC_NAME_H_ @}*/
