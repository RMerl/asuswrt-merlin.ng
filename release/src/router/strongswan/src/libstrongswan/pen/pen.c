/*
 * Copyright (C) 2011-2015 Andreas Steffen
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

#include "pen.h"

ENUM_BEGIN(pen_names, PEN_IETF, PEN_IETF,
	"IETF");
ENUM_NEXT(pen_names, PEN_IBM, PEN_IBM, PEN_IETF,
	"IBM");
ENUM_NEXT(pen_names, PEN_MICROSOFT, PEN_MICROSOFT, PEN_IBM,
	"Microsoft");
ENUM_NEXT(pen_names, PEN_REDHAT, PEN_REDHAT, PEN_MICROSOFT,
	"Redhat");
ENUM_NEXT(pen_names, PEN_PWG, PEN_PWG, PEN_REDHAT,
	"PWG");
ENUM_NEXT(pen_names, PEN_ALTIGA, PEN_ALTIGA, PEN_PWG,
	"Altiga");
ENUM_NEXT(pen_names, PEN_OSC, PEN_OSC, PEN_ALTIGA,
	"OSC");
ENUM_NEXT(pen_names, PEN_DEBIAN, PEN_DEBIAN, PEN_OSC,
	"Debian Project");
ENUM_NEXT(pen_names, PEN_GOOGLE, PEN_GOOGLE, PEN_DEBIAN,
	"Google");
ENUM_NEXT(pen_names, PEN_TCG, PEN_TCG, PEN_GOOGLE,
	"TCG");
ENUM_NEXT(pen_names, PEN_CANONICAL, PEN_CANONICAL, PEN_TCG,
	"Canonical");
ENUM_NEXT(pen_names, PEN_FEDORA, PEN_FEDORA, PEN_CANONICAL,
	"Fedora Project");
ENUM_NEXT(pen_names, PEN_FHH, PEN_FHH, PEN_FEDORA,
	"FHH");
ENUM_NEXT(pen_names, PEN_ITA, PEN_ITA, PEN_FHH,
	"ITA-HSR");
ENUM_NEXT(pen_names, PEN_OPENPTS, PEN_OPENPTS, PEN_ITA,
	"OpenPTS");
ENUM_NEXT(pen_names, PEN_UNASSIGNED, PEN_RESERVED, PEN_OPENPTS,
	"Unassigned",
	"Reserved");
ENUM_END(pen_names, PEN_RESERVED);
