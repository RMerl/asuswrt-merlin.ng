/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * GTBUS initialisation
 *
 * (C) Copyright 2016 Theobroma Systems Design und Consulting GmbH
 *                    Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#ifndef _SUNXI_GTBUS_H
#define _SUNXI_GTBUS_H

#if defined(CONFIG_MACH_SUN9I)
#include <asm/arch/gtbus_sun9i.h>
#endif

#ifndef __ASSEMBLY__
void gtbus_init(void);
#endif

#endif
