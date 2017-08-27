/*
 * Copyright (C) 2006 Martin Willi
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

#include "eap_method.h"

#include <daemon.h>

ENUM(eap_role_names, EAP_SERVER, EAP_PEER,
	"EAP_SERVER",
	"EAP_PEER",
);

/**
 * See header
 */
bool eap_method_register(plugin_t *plugin, plugin_feature_t *feature,
						 bool reg, void *data)
{
	if (reg)
	{
		charon->eap->add_method(charon->eap, feature->arg.eap, 0,
					feature->type == FEATURE_EAP_SERVER ? EAP_SERVER : EAP_PEER,
					(eap_constructor_t)data);
	}
	else
	{
		charon->eap->remove_method(charon->eap, (eap_constructor_t)data);
	}
	return TRUE;
}
