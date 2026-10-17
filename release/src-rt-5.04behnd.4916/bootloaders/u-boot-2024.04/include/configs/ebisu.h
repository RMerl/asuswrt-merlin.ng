/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/ebisu.h
 *     This file is Ebisu board configuration.
 *
 * Copyright (C) 2018 Renesas Electronics Corporation
 */

#ifndef __EBISU_H
#define __EBISU_H

#undef DEBUG

#include "rcar-gen3-common.h"

/* Environment in eMMC, at the end of 2nd "boot sector" */

#define CFG_SYS_FLASH_BANKS_LIST	{ 0x08000000 }
#define CFG_SYS_WRITE_SWAPPED_DATA

#endif /* __EBISU_H */
