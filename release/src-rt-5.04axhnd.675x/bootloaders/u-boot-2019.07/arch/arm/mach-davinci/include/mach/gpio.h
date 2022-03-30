/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 */
#ifndef _GPIO_DEFS_H_
#define _GPIO_DEFS_H_

#define DAVINCI_GPIO_BINTEN	0x01E26008
#define DAVINCI_GPIO_BANK01	0x01E26010
#define DAVINCI_GPIO_BANK23	0x01E26038
#define DAVINCI_GPIO_BANK45	0x01E26060
#define DAVINCI_GPIO_BANK67	0x01E26088
#define DAVINCI_GPIO_BANK8	0x01E260B0

#define davinci_gpio_bank01 ((struct davinci_gpio *)DAVINCI_GPIO_BANK01)
#define davinci_gpio_bank23 ((struct davinci_gpio *)DAVINCI_GPIO_BANK23)
#define davinci_gpio_bank45 ((struct davinci_gpio *)DAVINCI_GPIO_BANK45)
#define davinci_gpio_bank67 ((struct davinci_gpio *)DAVINCI_GPIO_BANK67)
#define davinci_gpio_bank8 ((struct davinci_gpio *)DAVINCI_GPIO_BANK8)

#ifndef CONFIG_DM_GPIO
#define gpio_status()		gpio_info()
#endif
#define GPIO_NAME_SIZE		20
#if !defined(CONFIG_SOC_DA850)
#define MAX_NUM_GPIOS		128
#else
#define MAX_NUM_GPIOS		144
#endif
#define GPIO_BANK(gp)		(davinci_gpio_bank01 + ((gp) >> 5))

void gpio_info(void);

#endif
