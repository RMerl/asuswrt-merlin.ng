/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2012,2015 Stephen Warren
 */

#ifndef _BCM2835_SDHCI_H_
#define _BCM2835_SDHCI_H_

#include <asm/arch/base.h>

#define BCM2835_SDHCI_PHYSADDR	rpi_sdhci_base

int bcm2835_sdhci_init(u32 regbase, u32 emmc_freq);

#endif
