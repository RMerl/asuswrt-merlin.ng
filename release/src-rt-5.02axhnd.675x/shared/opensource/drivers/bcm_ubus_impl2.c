/*
 * <:copyright-BRCM:2018:DUAL/GPL:standard
 * 
 *    Copyright (c) 2018 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

/* BCM UBUS3 supporting routines */

#include "bcm_map_part.h"
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#define printk  printf
#define udelay  cfe_usleep
#else // Linux
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "bcm_physical_map_part.h"
#include "board.h"
#include <linux/bug.h>
#include "enum_str.h"
#endif

#ifndef _CFE_
int ubus_decode_pcie_wnd_cfg(unsigned int base, unsigned int size, unsigned int core)
{
#if defined(CONFIG_BCM94908)
	uint32 core2pid[3] = {0x00, 0x07, 0x0d};
	uint32 reg_data0, reg_data1;
	int c = -1;
	static int cfg_index = 0;

	if (cfg_index >= 8) {
	    /* Max only 8 registers */
	    printk("%s(0x%x, 0x%x, 0x%x) exceeded max registers\r\n",
	        __FUNCTION__, base, size, core);
	    return -1;
	}

	if (!size) {
	    /* size can not be zero */
	    printk("%s(0x%x, 0x%x, 0x%x) invalid size\r\n",
	        __FUNCTION__, base, size, core);
	    return -1;
	}

	/* Get the size bits (assumed size is always in power of 2) */
	while (size) {
	    c++;
	    size >>= 1;
	}

	/* Is this extension of existing memory region or using a new region ? */
	
	reg_data0 = (base >> 8); //base addr[31:8] @ 23:00
	reg_data0 |= (((base >> 8) & 0xFF) << 24); //remap addr[15:8] @ 31:24
	reg_data1 = (base >> 16); //remap addr[31:16] @ 47:32
	reg_data1 |= (core2pid[core] << 16); //pid[7:0] @ 55:48
	reg_data1 |= (c << 24); //size[4:0] @ 60:56
	reg_data1 |= (0x01 << 30); //enable[1:0] @63:62

	printk("[%d] ubus_cfg_window[0] = 0x%x, ubus_cfg_window[1] = 0x%x\r\n",
		core, reg_data0, reg_data1);

	BIUCTRL->ubus_cfg_window[cfg_index++] = reg_data0;
	BIUCTRL->ubus_cfg_window[cfg_index++] = reg_data1;

	return 0;
#else  /* ! CONFIG_BCM94908 */
	return -1;
#endif /* ! CONFIG_BCM94908 */
}
EXPORT_SYMBOL(ubus_decode_pcie_wnd_cfg);
#endif
