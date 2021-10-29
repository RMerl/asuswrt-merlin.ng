/*
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
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
#ifndef BCM_MISC_HW_INIT_H
#define BCM_MISC_HW_INIT_H

#ifdef _CFE_
#include "lib_types.h"
#else
#include <linux/types.h>
#endif
#include "bcm_ubus4.h"

int bcm_misc_hw_init(void);
int rdp_pre_init(void);
int rdp_post_init(void);
int rdp_shut_down(void);
uint32_t pmu_clk(uint8_t mdiv_shift);

#if defined(CONFIG_BCM960333) && defined(CONFIG_PCI)
void stop_100mhz_afe_clock(void);
#endif

#endif//BCM_MISC_HW_INIT_H
