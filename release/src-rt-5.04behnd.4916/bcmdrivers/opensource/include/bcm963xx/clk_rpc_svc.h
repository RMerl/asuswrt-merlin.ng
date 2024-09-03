/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/****************************************************************************
 * Power RPC Driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#ifndef _CLK_RPC_SVC_H_
#define _CLK_RPC_SVC_H_

#include <linux/types.h>
#include <linux/platform_device.h>

#ifdef __KERNEL__    /* Linux kernel */
#include <itc_rpc.h>
#endif

#define CLK_DOMAIN_NAME_MAX_LEN    (8)
#define RPC_SERVICE_VER_CLK_DOMAIN_ID        (0)
#define RPC_SERVICE_VER_CLK_DOMAIN_NAME      (0)
#define RPC_SERVICE_VER_CLK_GET_DOMAIN_STATE (0)
#define RPC_SERVICE_VER_CLK_SET_DOMAIN_STATE (0)


/* clk svc functions */
int bcm_rpc_clk_get_domain_state(char *name, uint8_t name_size, uint8_t *enabled, uint32_t *rate);
int bcm_rpc_clk_set_domain_state(char *name, uint8_t name_size, uint8_t enabled, uint32_t rate);
int bcm_rpc_clk_get_rate(char *name, uint8_t name_size, uint32_t *rate);
int bcm_rpc_clk_set_rate(char *name, uint8_t name_size, uint32_t rate);
int bcm_rpc_clk_enable_disable(char *name, uint8_t name_size, char enable);
int bcm_rpc_set_clk_mode(uint32_t blk, uint32_t mode);
int bcm_rpc_get_clk_mode(uint32_t block, uint32_t *mode);
#endif

