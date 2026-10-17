/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Ilya Yanok, <ilya.yanok@gmail.com>
 */

#ifndef __ASM_ARCH_OMAP3_MUSB_H
#define __ASM_ARCH_OMAP3_MUSB_H
void am35x_musb_reset(struct udevice *dev);
void am35x_musb_phy_power(struct udevice *dev, u8 on);
void am35x_musb_clear_irq(struct udevice *dev);
#endif
