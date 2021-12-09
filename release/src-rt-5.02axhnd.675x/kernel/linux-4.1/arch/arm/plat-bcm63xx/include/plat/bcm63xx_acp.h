#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#ifndef __PLAT_BCM63XX_ACP_H
#define __PLAT_BCM63XX_ACP_H
#define BCM_UBUS_CFG_MAX	4

typedef enum {
#ifdef CONFIG_BCM963138
	BCM_UBUS_PID_PCIE0 = 0x0,
	BCM_UBUS_PID_DDR = 0x1,
	BCM_UBUS_PID_ARMAXIACP = 0x2,
	BCM_UBUS_PID_PERIPH = 0x3,
	BCM_UBUS_PID_USBD = 0x4,
	BCM_UBUS_PID_USBH = 0x5,
	BCM_UBUS_PID_SATA = 0x6,
	BCM_UBUS_PID_DECT = 0x7,
	BCM_UBUS_PID_APM = 0x8,
	BCM_UBUS_PID_VDSL = 0x9,
	BCM_UBUS_PID_SAR = 0xa,
	BCM_UBUS_PID_RNR = 0xb,
	BCM_UBUS_PID_RNR_RABR = 0xc,
	BCM_UBUS_PID_SF2 = 0xe,
	BCM_UBUS_PID_PMC = 0xf,
	BCM_UBUS_PID_PCIE1 = 0x10,
	BCM_UBUS_PID_ARMAIPDAP = 0x12,
	BCM_UBUS_PID_SAR2 = 0x1a,
	BCM_UBUS_PID_RNR_RBBR = 0x1c,
	BCM_UBUS_PID_ERROR = 0x1f,
#endif
	BCM_UBUS_PID_MAX,
} bcm_ubus_pid_t;
#define BCM_UBUS_PID_INVALID	0xff

typedef struct {
	uint32_t addr_in;
	uint32_t addr_out;
	uint8_t dst_pid;
	uint8_t size_shift;
	uint8_t en;
} bcm_acp_ubus_cfg_t;

typedef struct {
	/* L2 cache policy for write, recommend value is 0xf for
	 * cacheable WBWA, or 0x0 to disable */
	uint8_t wcache;	
	/* L2 cache policy for read, recommend value is 0xf for
	 * cacheable WBWA, or 0x0 to disable */
	uint8_t rcache;
	/* L1 cache policy for write, recommend value is 0x1 for
	 * cache invalidation, 0x1f for WBWA, or 0x0 to disable */
	uint8_t wuser;
	/* L1 cache policy for read, recommend value is 0x1 for
	 * cache invalidation, 0x1f for WBWA, or 0x0 to disable */
	uint8_t ruser;
} bcm_acp_cache_ctrl_t;

/* enable / disable the ACP feature for a specific block */
int bcm63xx_acp_enable(uint8_t ubus_pid);
int bcm63xx_acp_disable(uint8_t ubus_pid);

/* check if the ACP is enabled */
bool bcm63xx_acp_on(uint8_t ubus_pid);

/* UBUS configuration setting APIs */
int bcm63xx_acp_ubus_cfg_get_entry(uint8_t ubus_pid, uint8_t idx,
		bcm_acp_ubus_cfg_t *acp_ubus_cfg);
int bcm63xx_acp_ubus_cfg_get_all(uint8_t ubus_pid,
		bcm_acp_ubus_cfg_t *acp_ubus_cfg);
int bcm63xx_acp_ubus_cfg_set_entry(uint8_t ubus_pid, uint8_t idx,
		bcm_acp_ubus_cfg_t *acp_ubus_cfg);
int bcm63xx_acp_ubus_cfg_set_all(uint8_t ubus_pid,
		bcm_acp_ubus_cfg_t *acp_ubus_cfg);
void bcm63xx_acp_ubus_cfg_reset(uint8_t ubus_pid);

/* ACP port control */
int bcm63xx_acp_cache_ctrl_get(uint8_t ubus_pid, bcm_acp_cache_ctrl_t *cache_ctrl);
int bcm63xx_acp_cache_ctrl_set(uint8_t ubus_pid, bcm_acp_cache_ctrl_t *cache_ctrl);

#endif /* __PLAT_BCM63XX_ACP_H */
#endif /* defined(CONFIG_BCM_KF_ARM_BCM963XX) */
