/* strongSwan netkey starter
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <library.h>
#include <hydra.h>
#include <utils/debug.h>

#include "files.h"

bool starter_netkey_init(void)
{
	struct stat stb;

	if (stat(PROC_NETKEY, &stb) != 0)
	{
		/* af_key module makes the netkey proc interface visible */
		if (stat(PROC_MODULES, &stb) == 0)
		{
			ignore_result(system("modprobe -qv af_key"));
		}

		/* now test again */
		if (stat(PROC_NETKEY, &stb) != 0)
		{
			DBG2(DBG_APP, "kernel appears to lack the native netkey IPsec stack");
			return FALSE;
		}
	}

	/* make sure that all required IPsec modules are loaded */
	if (stat(PROC_MODULES, &stb) == 0)
	{
		ignore_result(system("modprobe -qv ah4"));
		ignore_result(system("modprobe -qv esp4"));
		ignore_result(system("modprobe -qv ipcomp"));
		ignore_result(system("modprobe -qv xfrm4_tunnel"));
		ignore_result(system("modprobe -qv xfrm_user"));
	}

	DBG2(DBG_APP, "found netkey IPsec stack");
	return TRUE;
}

void starter_netkey_cleanup(void)
{
	if (!lib->plugins->load(lib->plugins,
			lib->settings->get_str(lib->settings, "starter.load", PLUGINS)))
	{
		DBG1(DBG_APP, "unable to load kernel plugins");
		return;
	}
	hydra->kernel_interface->flush_sas(hydra->kernel_interface);
	hydra->kernel_interface->flush_policies(hydra->kernel_interface);
	lib->plugins->unload(lib->plugins);
}
