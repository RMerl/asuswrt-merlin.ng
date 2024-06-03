/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Faraday FTSDC010 Secure Digital Memory Card Host Controller
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */
#include <mmc.h>

#ifndef __FTSDC010_MCI_H
#define __FTSDC010_MCI_H

struct ftsdc010_chip {
	void __iomem *regs;
	uint32_t wprot;   /* write protected (locked) */
	uint32_t rate;    /* actual SD clock in Hz */
	uint32_t sclk;    /* FTSDC010 source clock in Hz */
	uint32_t fifo;    /* fifo depth in bytes */
	uint32_t acmd;
	struct mmc_config cfg;	/* mmc configuration */
	const char *name;
	void *ioaddr;
	unsigned int caps;
	unsigned int version;
	unsigned int clock;
	unsigned int bus_hz;
	unsigned int div;
	int dev_index;
	int dev_id;
	int buswidth;
	u32 fifoth_val;
	struct mmc *mmc;
	void *priv;
	bool fifo_mode;
};

#endif /* __FTSDC010_MCI_H */
