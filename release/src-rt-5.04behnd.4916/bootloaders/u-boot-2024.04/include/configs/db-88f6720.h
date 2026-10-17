/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_DB_88F6720_H
#define _CONFIG_DB_88F6720_H

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

/* I2C */
#define CFG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE

/* USB/EHCI configuration */

/* Environment in SPI NOR flash */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* SPL */
/* Defines for SPL */

#endif /* _CONFIG_DB_88F6720_H */
