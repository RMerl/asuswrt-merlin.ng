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

#ifdef _CFE_
#include "lib_types.h"
#else
#include <linux/types.h>
#endif
int bcm_misc_hw_init(void);
int rdp_pre_init(void);
int rdp_post_init(void);
int rdp_shut_down(void);
uint32_t pmu_clk(uint8_t mdiv_shift);

#if defined(CONFIG_BCM960333) && defined(CONFIG_PCI)
void stop_100mhz_afe_clock(void);
#endif

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
typedef enum
{
    MST_PORT_NODE_PCIE0,
    MST_PORT_NODE_PCIE1,
    MST_PORT_NODE_B53,
    MST_PORT_NODE_PCIE2,
    MST_PORT_NODE_SATA,
    MST_PORT_NODE_USB,
    MST_PORT_NODE_PMC,
    MST_PORT_NODE_APM,
    MST_PORT_NODE_PER,
    MST_PORT_NODE_DMA0,
    MST_PORT_NODE_DMA1,
    MST_PORT_NODE_RQ0,
    MST_PORT_NODE_RQ1,
    MST_PORT_NODE_RQ2,
    MST_PORT_NODE_RQ3,
    MST_PORT_NODE_NATC,
    MST_PORT_NODE_DQM,
    MST_PORT_NODE_QM
}MST_PORT_NODE;

int ubus_master_decode_wnd_cfg(MST_PORT_NODE node, int win, unsigned int phys_addr, unsigned int size_power_of_2, int port_id);
int ubus_master_set_token_credits(MST_PORT_NODE node, int token, int credits);
#endif
