/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/salvator-x.h
 *     This file is Salvator-X board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 */

#ifndef __SALVATOR_X_H
#define __SALVATOR_X_H

#include "rcar-gen3-common.h"

/* Environment in eMMC, at the end of 2nd "boot sector" */

#define CFG_SYS_FLASH_BANKS_LIST	{ 0x08000000 }
#define CFG_SYS_WRITE_SWAPPED_DATA

#endif /* __SALVATOR_X_H */
