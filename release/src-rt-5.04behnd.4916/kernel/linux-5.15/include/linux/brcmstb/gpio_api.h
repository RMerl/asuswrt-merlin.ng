/*
 * Copyright © 2015-2016 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#ifndef _BRCMSTB_GPIO_API_H
#define _BRCMSTB_GPIO_API_H

#if IS_ENABLED(CONFIG_GPIO_BRCMSTB) || IS_ENABLED(CONFIG_PINCTRL_BCM2835)
int brcmstb_gpio_update32(uint32_t addr, uint32_t mask, uint32_t value);
#else
static inline int brcmstb_gpio_update32(uint32_t addr, uint32_t mask,
					uint32_t value)
{
	return -ENODEV;
}
#endif
int brcmstb_gpio_irq(uint32_t addr, unsigned int shift);
void brcmstb_gpio_remove(void);

#endif /* _BRCMSTB_GPIO_API_H */
