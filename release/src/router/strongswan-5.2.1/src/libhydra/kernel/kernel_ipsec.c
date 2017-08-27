/*
 * Copyright (C) 2008 Tobias Brunner
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

#include "kernel_ipsec.h"

#include <hydra.h>

/**
 * See header
 */
bool kernel_ipsec_register(plugin_t *plugin, plugin_feature_t *feature,
						   bool reg, void *data)
{
	if (reg)
	{
		hydra->kernel_interface->add_ipsec_interface(hydra->kernel_interface,
											(kernel_ipsec_constructor_t)data);
	}
	else
	{
		hydra->kernel_interface->remove_ipsec_interface(hydra->kernel_interface,
											(kernel_ipsec_constructor_t)data);
	}
	return TRUE;
}
