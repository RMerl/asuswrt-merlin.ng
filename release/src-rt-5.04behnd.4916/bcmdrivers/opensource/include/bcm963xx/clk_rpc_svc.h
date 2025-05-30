/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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

