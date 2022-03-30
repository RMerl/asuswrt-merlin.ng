/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board data structure for musb gadget on OMAPs
 *
 * Copyright (C) 2012, Ilya Yanok <ilya.yanok@gmail.com>
 */

#ifndef __ASM_ARM_OMAP_MUSB_H
#define __ASM_ARM_OMAP_MUSB_H
#include <linux/usb/musb.h>

extern struct musb_platform_ops musb_dsps_ops;
extern const struct musb_platform_ops am35x_ops;
extern const struct musb_platform_ops omap2430_ops;

struct omap_musb_board_data {
	u8 interface_type;
	struct udevice *dev;
	void (*set_phy_power)(struct udevice *dev, u8 on);
	void (*clear_irq)(struct udevice *dev);
	void (*reset)(struct udevice *dev);
};

enum musb_interface    {MUSB_INTERFACE_ULPI, MUSB_INTERFACE_UTMI};

struct ti_musb_platdata {
	void *base;
	void *ctrl_mod_base;
	struct musb_hdrc_platform_data plat;
};

#endif /* __ASM_ARM_OMAP_MUSB_H */
