/*
 * Copyright (C) 2011 Andreas Steffen
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

#include "pts_file_type.h"

ENUM(pts_file_type_names, PTS_FILE_OTHER, PTS_FILE_SOCKET,
	"Other",
	"FIFO",
	"Character-Special",
	"Reserved-3",
	"Directory",
	"Reserved-5",
	"Block-Special",
	"Reserved-7",
	"Regular",
	"Reserved-9",
	"Symbolic-Link",
	"Reserved-11",
	"Socket"
);

