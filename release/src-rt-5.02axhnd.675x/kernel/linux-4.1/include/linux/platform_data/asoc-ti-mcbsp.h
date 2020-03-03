/*
 * Defines for Multi-Channel Buffered Serial Port
 *
 * Copyright (C) 2002 RidgeRun, Inc.
 * Author: Steve Johnson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#ifndef __ASOC_TI_MCBSP_H
#define __ASOC_TI_MCBSP_H

#include <linux/spinlock.h>
#include <linux/clk.h>

#define MCBSP_CONFIG_TYPE2	0x2
#define MCBSP_CONFIG_TYPE3	0x3
#define MCBSP_CONFIG_TYPE4	0x4

/* Platform specific configuration */
struct omap_mcbsp_ops {
	void (*request)(unsigned int);
	void (*free)(unsigned int);
};

struct omap_mcbsp_platform_data {
	struct omap_mcbsp_ops *ops;
	u16 buffer_size;
	u8 reg_size;
	u8 reg_step;

	/* McBSP platform and instance specific features */
	bool has_wakeup; /* Wakeup capability */
	bool has_ccr; /* Transceiver has configuration control registers */
	int (*enable_st_clock)(unsigned int, bool);
};

/**
 * omap_mcbsp_dev_attr - OMAP McBSP device attributes for omap_hwmod
 * @sidetone: name of the sidetone device
 */
struct omap_mcbsp_dev_attr {
	const char *sidetone;
};

#endif
