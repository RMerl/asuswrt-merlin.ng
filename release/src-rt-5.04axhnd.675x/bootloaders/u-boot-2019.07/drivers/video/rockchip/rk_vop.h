/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 */

#ifndef __RK_VOP_H__
#define __RK_VOP_H__

#include <asm/arch-rockchip/vop_rk3288.h>

struct rk_vop_priv {
	void *grf;
	void *regs;
};

enum vop_features {
	VOP_FEATURE_OUTPUT_10BIT = (1 << 0),
};

struct rkvop_driverdata {
	/* configuration */
	u32 features;
	/* block-specific setters/getters */
	void (*set_pin_polarity)(struct udevice *, enum vop_modes, u32);
};

/**
 * rk_vop_probe() - common probe implementation
 *
 * Performs the rk_display_init on each port-subnode until finding a
 * working port (or returning an error if none of the ports could be
 * successfully initialised).
 *
 * @dev:	device
 * @return 0 if OK, -ve if something went wrong
 */
int rk_vop_probe(struct udevice *dev);

/**
 * rk_vop_bind() - common bind implementation
 *
 * Sets the plat->size field to the amount of memory to be reserved for
 * the framebuffer: this is always
 *     (32 BPP) x VIDEO_ROCKCHIP_MAX_XRES x VIDEO_ROCKCHIP_MAX_YRES
 *
 * @dev:	device
 * @return 0 (always OK)
 */
int rk_vop_bind(struct udevice *dev);

/**
 * rk_vop_probe_regulators() - probe (autoset + enable) regulators
 *
 * Probes a list of regulators by performing autoset and enable
 * operations on them.  The list of regulators is an array of string
 * pointers and any individual regulator-probe may fail without
 * counting as an error.
 *
 * @dev:	device
 * @names:	array of string-pointers to regulator names to probe
 * @cnt:	number of elements in the 'names' array
 */
void rk_vop_probe_regulators(struct udevice *dev,
			     const char * const *names, int cnt);

#endif
