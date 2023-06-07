// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 Instituto Nokia de Tecnologia - INdT
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "manager.h"

static int sap_init(void)
{
	return sap_manager_init();
}

static void sap_exit(void)
{
	sap_manager_exit();
}

BLUETOOTH_PLUGIN_DEFINE(sap, VERSION,
		BLUETOOTH_PLUGIN_PRIORITY_DEFAULT, sap_init, sap_exit)
