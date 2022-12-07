// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "hdp_manager.h"

static int hdp_init(void)
{
	return hdp_manager_init();
}

static void hdp_exit(void)
{
	hdp_manager_exit();
}

BLUETOOTH_PLUGIN_DEFINE(health, VERSION,
			BLUETOOTH_PLUGIN_PRIORITY_DEFAULT, hdp_init, hdp_exit)
