/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Tony Dinh <mibodhi@gmail.com>
 * (C) Copyright 2009-2012
 * Wojciech Dubowik <wojciech.dubowik@neratec.com>
 * Luka Perkov <luka@openwrt.org>
 */

#ifndef _CONFIG_ICONNECT_H
#define _CONFIG_ICONNECT_H

#include "mv-common.h"

#define CFG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0"	\
	"kernel=/boot/uImage\0"			\
	"bootargs_root=noinitrd ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs\0"

#endif /* _CONFIG_ICONNECT_H */
