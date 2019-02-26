/*
 * Copyright (C) 2012-2014 Andreas Steffen
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

#include "os_info.h"

ENUM(os_type_names, OS_TYPE_UNKNOWN, OS_TYPE_WINDOWS,
	"Unknown",
	"Debian",
	"Ubuntu",
	"Fedora",
	"Red Hat",
	"CentOS",
	"SUSE",
	"Gentoo",
	"Android",
	"Windows",
);

ENUM(os_fwd_status_names, OS_FWD_DISABLED, OS_FWD_UNKNOWN,
	"disabled",
	"enabled",
	"unknown"
);

ENUM(os_package_state_names, OS_PACKAGE_STATE_UPDATE, OS_PACKAGE_STATE_BLACKLIST,
	"",
	" [s]",
	" [b]"
);

/**
 * See header
 */
os_type_t os_type_from_name(chunk_t name)
{
	os_type_t type;
	char *name_str;

	for (type = OS_TYPE_DEBIAN; type < OS_TYPE_ROOF; type++)
	{
		/* name_str is a substring of name.ptr */
		name_str = enum_to_name(os_type_names, type);
		if (memeq(name.ptr, name_str, min(name.len, strlen(name_str))))
		{
			return type;
		}
	}
	return OS_TYPE_UNKNOWN;
}
