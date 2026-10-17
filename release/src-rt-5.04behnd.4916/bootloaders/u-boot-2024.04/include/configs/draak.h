/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/draak.h
 *     This file is Draak board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 */

#ifndef __DRAAK_H
#define __DRAAK_H

#include "rcar-gen3-common.h"

/* Environment in eMMC, at the end of 2nd "boot sector" */

#define CFG_SYS_FLASH_BANKS_LIST	{ 0x08000000 }
#define CFG_SYS_WRITE_SWAPPED_DATA

#endif /* __DRAAK_H */
