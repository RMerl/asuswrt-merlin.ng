/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28 GPIO
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#ifndef	__MX28_GPIO_H__
#define	__MX28_GPIO_H__

#ifdef	CONFIG_MXS_GPIO
void mxs_gpio_init(void);
#else
inline void mxs_gpio_init(void) {}
#endif

#endif	/* __MX28_GPIO_H__ */
