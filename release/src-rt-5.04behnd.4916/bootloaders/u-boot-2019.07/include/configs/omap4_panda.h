/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated.
 * Steve Sakoman  <steve@sakoman.com>
 *
 * Configuration settings for the TI OMAP4 Panda board.
 * See ti_omap4_common.h for OMAP4 common part
 */

#ifndef __CONFIG_PANDA_H
#define __CONFIG_PANDA_H

/*
 * High Level Configuration Options
 */

/* USB UHH support options */
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO 1
#define CONFIG_OMAP_EHCI_PHY2_RESET_GPIO 62

/* USB Networking options */

#define CONFIG_UBOOT_ENABLE_PADS_ALL

#include <configs/ti_omap4_common.h>

/* GPIO */

/* ENV related config options */

#define CONFIG_ENV_OVERWRITE

#endif /* __CONFIG_PANDA_H */
