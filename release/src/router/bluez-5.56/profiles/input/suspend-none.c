// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Nordic Semiconductor Inc.
 *  Copyright (C) 2012  Instituto Nokia de Tecnologia - INdT
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "src/log.h"
#include "suspend.h"

int suspend_init(suspend_event suspend, resume_event resume)
{
	DBG("");

	return 0;
}

void suspend_exit(void)
{
	DBG("");
}
