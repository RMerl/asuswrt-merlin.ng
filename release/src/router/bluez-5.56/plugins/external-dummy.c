// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "src/plugin.h"
#include "src/log.h"

static int dummy_init(void)
{
	DBG("");

	return 0;
}

static void dummy_exit(void)
{
	DBG("");
}

BLUETOOTH_PLUGIN_DEFINE(external_dummy, VERSION,
		BLUETOOTH_PLUGIN_PRIORITY_LOW, dummy_init, dummy_exit)
