/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 Broadcom Corporation.
 */

#ifndef __KONA_SDHCI_H
#define __KONA_SDHCI_H

int kona_sdhci_init(int dev_index, u32 min_clk, u32 quirks);

#endif
