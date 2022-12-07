/*
 * Copyright (C) 2011 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#include "tcg_comp_func_name.h"

char pts_tcg_qualifier_flag_names[] = { 'K', 'S' };

ENUM_BEGIN(pts_tcg_qualifier_type_names, PTS_TCG_QUALIFIER_TYPE_UNKNOWN,
										 PTS_TCG_QUALIFIER_TYPE_TNC,
	"Unknown",
	"Trusted Platform",
	"Operating System",
	"Graphical User Interface",
	"Application",
	"Networking",
	"Library",
	"TNC Defined Component"
);
ENUM_NEXT(pts_tcg_qualifier_type_names, PTS_TCG_QUALIFIER_TYPE_ALL,
										PTS_TCG_QUALIFIER_TYPE_ALL,
										PTS_TCG_QUALIFIER_TYPE_TNC,
	"All Matching Components"
);
ENUM_END(pts_tcg_qualifier_type_names, PTS_TCG_QUALIFIER_TYPE_ALL);

ENUM(pts_tcg_comp_func_names, PTS_TCG_COMP_FUNC_NAME_IGNORE,
							  PTS_TCG_COMP_FUNC_NAME_OPT_ROMS,
	"Ignore",
	"CRTM",
	"BIOS",
	"Platform Extensions",
	"Motherboard Firmware",
	"Initial Program Loader",
	"Option ROMs"
);

