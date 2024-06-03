/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2012,2015 Stephen Warren
 */

#ifndef _BCM2835_SDHCI_H_
#define _BCM2835_SDHCI_H_

#ifndef CONFIG_BCM2835
#define BCM2835_SDHCI_BASE 0x3f300000
#else
#define BCM2835_SDHCI_BASE 0x20300000
#endif

int bcm2835_sdhci_init(u32 regbase, u32 emmc_freq);

#endif
