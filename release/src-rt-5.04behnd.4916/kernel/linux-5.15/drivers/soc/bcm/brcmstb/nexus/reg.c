/*
 * Nexus Register(s) API
 *
 * Copyright (C) 2015, Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/brcmstb/brcmstb.h>
#include <linux/brcmstb/reg_api.h>

#include "gpio.h"

int brcmstb_update32(uint32_t addr, uint32_t mask, uint32_t value)
{
	return brcmstb_gpio_update32(addr, mask, value);
}
EXPORT_SYMBOL(brcmstb_update32);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB Nexus register APIs");
