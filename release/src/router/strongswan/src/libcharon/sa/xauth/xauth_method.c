/*
 * Copyright (C) 2006 Martin Willi
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

#include "xauth_method.h"

#include <daemon.h>

ENUM(xauth_role_names, XAUTH_SERVER, XAUTH_PEER,
	"XAUTH_SERVER",
	"XAUTH_PEER",
);

/**
 * See header
 */
bool xauth_method_register(plugin_t *plugin, plugin_feature_t *feature,
						 bool reg, void *data)
{
	if (reg)
	{
		charon->xauth->add_method(charon->xauth, feature->arg.xauth,
			feature->type == FEATURE_XAUTH_SERVER ? XAUTH_SERVER : XAUTH_PEER,
			(xauth_constructor_t)data);
	}
	else
	{
		charon->xauth->remove_method(charon->xauth, (xauth_constructor_t)data);
	}
	return TRUE;
}
