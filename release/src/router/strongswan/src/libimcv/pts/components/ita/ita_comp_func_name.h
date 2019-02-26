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
 * @defgroup pts_ita_comp_func_name pts_ita_comp_func_name
 * @{ @ingroup pts
 */

#ifndef PTS_ITA_COMP_FUNC_NAME_H_
#define PTS_ITA_COMP_FUNC_NAME_H_

typedef enum pts_ita_qualifier_type_t pts_ita_qualifier_type_t;
typedef enum pts_ita_comp_func_name_t pts_ita_comp_func_name_t;

#include <library.h>

/**
 * PTS Component Functional Name Qualifier Flags for the ITA namespace
 */
#define PTS_ITA_QUALIFIER_FLAG_KERNEL			(1<<5)
#define PTS_ITA_QUALIFIER_FLAG_SUB				(1<<4)

extern char pts_ita_qualifier_flag_names[];

/**
 * Size of the PTS Component Functional Name Qualifier Type field
 */
#define PTS_ITA_QUALIFIER_TYPE_SIZE				4

/**
 * PTS Component Functional Name Qualifier Types for the ITA namespace
 * equal to section 5.2 of PTS Protocol: Binding to TNC IF-M Specification
 */
enum pts_ita_qualifier_type_t {
	/** Unknown */
	PTS_ITA_QUALIFIER_TYPE_UNKNOWN =			0x0,
	/** Trusted Platform */
	PTS_ITA_QUALIFIER_TYPE_TRUSTED =			0x1,
	/** Operating System */
	PTS_ITA_QUALIFIER_TYPE_OS =					0x2,
	/** Graphical User Interface */
	PTS_ITA_QUALIFIER_TYPE_GUI =				0x3,
	/** Application */
	PTS_ITA_QUALIFIER_TYPE_APP =				0x4,
	/** Networking */
	PTS_ITA_QUALIFIER_TYPE_NET =				0x5,
	/** Library */
	PTS_ITA_QUALIFIER_TYPE_LIB =				0x6,
	/** TNC Defined Component */
	PTS_ITA_QUALIFIER_TYPE_TNC =				0x7,
	/** All Matching Components */
	PTS_ITA_QUALIFIER_TYPE_ALL =				0xF,
};

extern enum_name_t *pts_ita_qualifier_type_names;

/**
 * PTS Component Functional Name Binary Enumeration for the ITA namespace
 */
enum pts_ita_comp_func_name_t {
	/** Ignore */
	PTS_ITA_COMP_FUNC_NAME_IGNORE =				0x0000,
	/** Trusted GRUB Boot Loader */
	PTS_ITA_COMP_FUNC_NAME_TGRUB =				0x0001,
	/** Trusted Boot */
	PTS_ITA_COMP_FUNC_NAME_TBOOT =				0x0002,
	/** Linux Integrity Measurement Architecture */
	PTS_ITA_COMP_FUNC_NAME_IMA =				0x0003,
};

extern enum_name_t *pts_ita_comp_func_names;

#endif /** PTS_ITA_COMP_FUNC_NAME_H_ @}*/
