/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
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

#include "ipsec_types.h"

ENUM(ipsec_mode_names, MODE_TRANSPORT, MODE_DROP,
	"TRANSPORT",
	"TUNNEL",
	"BEET",
	"PASS",
	"DROP"
);

ENUM(policy_dir_names, POLICY_IN, POLICY_FWD,
	"in",
	"out",
	"fwd"
);

ENUM(ipcomp_transform_names, IPCOMP_NONE, IPCOMP_LZJH,
	"IPCOMP_NONE",
	"IPCOMP_OUI",
	"IPCOMP_DEFLATE",
	"IPCOMP_LZS",
	"IPCOMP_LZJH"
);

/*
 * See header
 */
bool mark_from_string(const char *value, mark_t *mark)
{
	char *endptr;

	if (!value)
	{
		return FALSE;
	}
	mark->value = strtoul(value, &endptr, 0);
	if (*endptr)
	{
		if (*endptr != '/')
		{
			DBG1(DBG_APP, "invalid mark value: %s", value);
			return FALSE;
		}
		mark->mask = strtoul(endptr+1, &endptr, 0);
		if (*endptr)
		{
			DBG1(DBG_LIB, "invalid mark mask: %s", endptr);
			return FALSE;
		}
	}
	else
	{
		mark->mask = 0xffffffff;
	}
	/* apply the mask to ensure the value is in range */
	mark->value &= mark->mask;
	return TRUE;
}
