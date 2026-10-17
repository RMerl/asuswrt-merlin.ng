/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022  Tony Dinh <mibodhi@gmail.com>
 * Copyright (C) 2010  Eric C. Cooper <ecc@cmu.edu>
 *
 * Based on sheevaplug.h originally written by
 * Prafulla Wadaskar <prafulla@marvell.com>
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 */

#ifndef _CONFIG_DOCKSTAR_H
#define _CONFIG_DOCKSTAR_H

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * Default environment variables
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0" \
	"kernel=/boot/uImage\0" \
	"initrd=/boot/uInitrd\0" \
	"bootargs_root=ubi.mtd=1 root=ubi0:root rootfstype=ubifs ro\0"

#endif /* _CONFIG_DOCKSTAR_H */
