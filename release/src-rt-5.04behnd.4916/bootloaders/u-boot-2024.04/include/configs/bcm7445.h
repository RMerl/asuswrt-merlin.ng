/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 *
 * Configuration settings for the Broadcom BCM7445 SoC family.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_INIT_RAM_ADDR	0x80200000

#include "bcmstb.h"

#define BCMSTB_TIMER_LOW	0xf0412008
#define BCMSTB_TIMER_HIGH	0xf041200c
#define BCMSTB_TIMER_FREQUENCY	0xf0412020
#define BCMSTB_HIF_MSPI_BASE	0xf03e3400
#define BCMSTB_BSPI_BASE	0xf03e3200
#define BCMSTB_HIF_SPI_INTR2	0xf03e1a00
#define BCMSTB_CS_REG		0xf03e0920

/*
 * Environment configuration for SPI flash.
 */

#endif	/* __CONFIG_H */
