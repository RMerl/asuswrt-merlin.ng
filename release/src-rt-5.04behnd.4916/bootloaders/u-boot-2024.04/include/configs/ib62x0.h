/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011-2012
 * Gerald Kerma <dreagle@doukki.net>
 * Luka Perkov <luka@openwrt.org>
 */

#ifndef _CONFIG_IB62x0_H
#define _CONFIG_IB62x0_H

#include "mv-common.h"

/*
 * Environment variables configuration
 */

/*
 * Default environment variables
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0"				\
	"kernel=/boot/zImage\0"						\
	"fdt=/boot/ib62x0.dtb\0"					\
	"bootargs_root=ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw\0"

/*
 * SATA driver configuration
 */
#ifdef CONFIG_IDE
#define __io
#endif /* CONFIG_IDE */

#endif /* _CONFIG_IB62x0_H */
