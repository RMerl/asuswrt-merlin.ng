/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _OMAP_I2C_H
#define _OMAP_I2C_H

#if CONFIG_IS_ENABLED(DM_I2C)

/* Information about a GPIO bank */
struct omap_i2c_plat {
	ulong base;	/* address of registers in physical memory */
	int speed;
	int ip_rev;
};

#endif

enum {
	OMAP_I2C_REV_V1 = 0,
	OMAP_I2C_REV_V2 = 1,
};

#endif /* _OMAP_I2C_H */
