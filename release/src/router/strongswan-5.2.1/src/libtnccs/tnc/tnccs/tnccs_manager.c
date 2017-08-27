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

#include "tnccs_manager.h"

#include "tnc/tnc.h"

#include <utils/debug.h>

/**
 * See header
 */
bool tnccs_method_register(plugin_t *plugin, plugin_feature_t *feature,
						   bool reg, void *data)
{
	if (!tnc || !tnc->tnccs)
	{
		DBG1(DBG_TNC, "TNC TNCCS manager does not exist");
		return FALSE;
	}
	if (reg)
	{
		if (feature->type == FEATURE_CUSTOM)
		{
			tnccs_type_t type = TNCCS_UNKNOWN;

			if (streq(feature->arg.custom, "tnccs-2.0"))
			{
				type = TNCCS_2_0;
			}
			else if (streq(feature->arg.custom, "tnccs-1.1"))
			{
				type = TNCCS_1_1;
			}
			else if (streq(feature->arg.custom, "tnccs-dynamic"))
			{
				type = TNCCS_DYNAMIC;
			}
			else
			{
				return FALSE;
			}
			tnc->tnccs->add_method(tnc->tnccs, type, (tnccs_constructor_t)data);
		}
	}
	else
	{
		tnc->tnccs->remove_method(tnc->tnccs, (tnccs_constructor_t)data);
	}
	return TRUE;
}
