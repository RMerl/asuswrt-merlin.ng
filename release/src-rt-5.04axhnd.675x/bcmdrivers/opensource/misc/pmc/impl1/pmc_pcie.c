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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <boardparms.h>

#include "pmc_drv.h"
#include "pmc_pcie.h"
#include "BPCM.h"
#include "bcm_ubus4.h"

/* Power Down bits in SOFT_RESET register */
#define SOFT_RST_PCIE_PD            (1 << 2)   /* Power Down IDDQ only */
#define SOFT_RST_PCIE_PLL_PD        (1 << 29)  /* Power Down PLL */
#define SOFT_RST_PCIE_PLL_LDO_PD    (1 << 30)  /* Power Down PLL LDO */

typedef struct {
    int pmb_addr;    /* PMB Address */
    int pll;         /* TRUE: port uses GEN3 PLL */
} port_info_t;

static const port_info_t pmc_pcie_port_info[]= {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855)
	{ PMB_ADDR_PCIE0, FALSE },
	{ PMB_ADDR_PCIE1, FALSE }
#elif defined(CONFIG_BCM94908) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM963146)
	{ PMB_ADDR_PCIE0, FALSE },
	{ PMB_ADDR_PCIE1, FALSE },
	{ PMB_ADDR_PCIE2, FALSE }
#elif defined(CONFIG_BCM963158)
	{ PMB_ADDR_PCIE0, FALSE },
	{ PMB_ADDR_PCIE1, FALSE },
	{ PMB_ADDR_PCIE2, FALSE },
	{ PMB_ADDR_PCIE3, TRUE }    /* GEN3, PLL */
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
	{ PMB_ADDR_PCIE0, FALSE },
#if (CONFIG_BRCM_CHIP_REV==0x4912A0)
	{ PMB_ADDR_PCIE1, FALSE },
#else
	{ PMB_ADDR_PCIE1, TRUE },    /* GEN3, PLL */
#endif
	{ PMB_ADDR_PCIE2, FALSE },
	{ PMB_ADDR_PCIE3, TRUE }    /* GEN3, PLL */
#elif defined(CONFIG_BCM96858)
	{ PMB_ADDR_PCIE0, FALSE },
	{ PMB_ADDR_PCIE1, FALSE },
	{ PMB_ADDR_PCIE_UBUS, FALSE }
#else /* CONFIG_BCM963178, CONFIG_BCM947622, CONFIG_BCM96878,CONFIG_BCM96756 */
	{ PMB_ADDR_PCIE0, FALSE }
#endif
};

#define MAX_PCIE_UNIT	(sizeof(pmc_pcie_port_info)/sizeof(port_info_t))

#if defined (CONFIG_BCM96858) || defined(CONFIG_BCM96856)
/* registration/deregistration is automatic enable if needed */
//#define UBUS_NODE_REGISTRATION            1
#endif

#define PCIE_INVALID_PORT                 0
#define PCIE_STANDALONE_PORT              1
#define PCIE_BIFURCATED_PORT              2

typedef struct {
    int port_type;
    int slv_node;
    int mst_node;
} ubus_node_info_t;

/* PCIe port UBUS */
static const ubus_node_info_t pmc_pcie_ubus_node[] = {
#if defined (CONFIG_BCM96858)
	{PCIE_BIFURCATED_PORT, UCB_NODE_ID_SLV_PCIE0, UCB_NODE_ID_MST_PCIE0},
	{PCIE_BIFURCATED_PORT, UCB_NODE_ID_SLV_PCIE0, UCB_NODE_ID_MST_PCIE0},
	{PCIE_STANDALONE_PORT, UCB_NODE_ID_SLV_PCIE2, UCB_NODE_ID_MST_PCIE2}
#elif defined(CONFIG_BCM96856)
	{PCIE_BIFURCATED_PORT, UCB_NODE_ID_SLV_PCIE0, UCB_NODE_ID_MST_PCIE0},
	{PCIE_BIFURCATED_PORT, UCB_NODE_ID_SLV_PCIE0, UCB_NODE_ID_MST_PCIE0}
#elif defined(CONFIG_BCM96846)
	{PCIE_BIFURCATED_PORT, UCB_NODE_ID_SLV_PCIE0, UCB_NODE_ID_MST_PCIE0},
	{PCIE_BIFURCATED_PORT, UCB_NODE_ID_SLV_PCIE0, UCB_NODE_ID_MST_PCIE0}
#else
	/* SoC that does not require UBUS registration */
	{PCIE_INVALID_PORT, -1, -1},
	{PCIE_INVALID_PORT, -1, -1},
	{PCIE_INVALID_PORT, -1, -1},
	{PCIE_INVALID_PORT, -1, -1}
#endif
};

void pmc_pcie_register_ubus(int unit)
{
	/* Skip registration for ubus earlier than UBUS4 */
	if (pmc_pcie_ubus_node[unit].port_type == PCIE_INVALID_PORT) {
	    return;
	}

#if defined(UBUS_NODE_REGISTRATION)
	/* Register slave node */
	ubus_register_port(pmc_pcie_ubus_node[unit].slv_node);

	/* Register master node */
	ubus_register_port(pmc_pcie_ubus_node[unit].mst_node);
#endif /* UBUS_NODE_REGISTRATION */

	return;
}

void pmc_pcie_deregister_ubus(int unit)
{

	/* Skip deregistration for ubus earlier than UBUS4 */
	/* skip deregister for bifurcated ports */
	if (pmc_pcie_ubus_node[unit].port_type != PCIE_STANDALONE_PORT) {
	    return;
	}

#if defined(UBUS_NODE_REGISTRATION)
	/* unegister slave node */
	ubus_deregister_port(pmc_pcie_ubus_node[unit].slv_node);

	/* Unegister master node */
	ubus_deregister_port(pmc_pcie_ubus_node[unit].mst_node);
#endif /* UBUS_NODE_REGISTRATION */

	return;
}

/**
 * Re-configure UBUS4 for the given PCI-E core.
 * - master token credits for PCIE -> Runner access
 * - master decode window
 */
static void pmc_pcie_ubus_init(int unit)
{
#if defined(CONFIG_BCM96858)
    int unit2mst_node_tbl[] = {UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE2};
    /* These credits of PCIe to Runner quads are requiered to Wakeup runner
     * in case of DHD Offload RxComplete */
    apply_ubus_credit_each_master(unit2mst_node_tbl[unit]);

#elif defined(CONFIG_BCM96846)
    /*In 6846 both PCIe shares the same UBUS Master*/
    int unit2mst_node_tbl[] = {UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE0};
    ubus_master_set_token_credits(unit2mst_node_tbl[unit], 9, 1);
#elif defined(CONFIG_BCM963158)
    /*In 63158 PCIe#0,1 are bifurcated ports and shares the same UBUS Master */
    int unit2mst_node_tbl[] = {UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE2, UBUS_PORT_ID_PCIE3};
    apply_ubus_credit_each_master(unit2mst_node_tbl[unit]);
#elif defined(CONFIG_BCM96856)
    int unit2mst_node_tbl[] = {UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE2};
#elif defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96756) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)
    int unit2mst_node_tbl[] = {UBUS_PORT_ID_PCIE0};
#elif defined(CONFIG_BCM963146)
    __attribute__((unused)) int unit2mst_node_tbl[] = {UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE1, UBUS_PORT_ID_PCIE2};
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    __attribute__((unused)) int unit2mst_node_tbl[] = {UBUS_PORT_ID_PCIE0, UBUS_PORT_ID_PCIE1, UBUS_PORT_ID_PCIE2, UBUS_PORT_ID_PCIE3};
#endif

#if defined(CONFIG_BCM_UBUS4_DCM)
    ubus_cong_threshold_wr(unit2mst_node_tbl[unit], 0);
#endif

#if defined(CONFIG_BCM_UBUS_DECODE_REMAP)
    ubus_master_remap_port(unit2mst_node_tbl[unit]);
#endif

#if defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)
/* These credits of PCIe to Runner VPB are requiered to wakeup Runner in case of DHD Offload RxComplete */    
    ubus_master_set_token_credits(unit2mst_node_tbl[unit], UBUS_PORT_ID_VPB, 1);
#endif

}

void pmc_pcie_power_up(int unit, int is_dual_lane)
{
	BPCM_SR_CONTROL sr_ctrl = {
		.Bits.sr = 0, // Only iddq
	};
	int addr;
	int dual_lane = is_dual_lane;

	if (unit >= MAX_PCIE_UNIT)
		BUG_ON(1);

	addr = pmc_pcie_port_info[unit].pmb_addr;

	if (PowerOnZone(addr, 0))
		BUG_ON(1);

	mdelay(10);

	pmc_pcie_register_ubus(unit);
#ifndef CONFIG_DT_SUPPORT_ONLY
	/* If not passed from caller, try to get it from Boardparms */
	if (!is_dual_lane)
	    BpGetPciPortDualLane(unit, &dual_lane);
#endif

	if (dual_lane) {
#if defined(CONFIG_BCM963158)
	    int bifur_addr;

	    /* Power up the other bi-furcated port */
	    bifur_addr = pmc_pcie_port_info[unit+1].pmb_addr;
	    if (PowerOnZone(bifur_addr, 0))
		    BUG_ON(1);
	    mdelay(10);
	    if (WriteBPCMRegister(bifur_addr, BPCMRegOffset(sr_control), sr_ctrl.Reg32))
		    BUG_ON(1);
#endif
	    sr_ctrl.Reg32 = 0x80; /* bit7: 1 - Strap override for dual lane support */
	                          /* bit6: Strap value, 0 - dual lane, 1 - single lane */
	    /* reset */
	    if (WriteBPCMRegister(addr, BPCMRegOffset(sr_control), 0xff))
	        BUG_ON(1);

	    mdelay(10);
	}

	if (WriteBPCMRegister(addr, BPCMRegOffset(sr_control), sr_ctrl.Reg32))
		BUG_ON(1);

	pmc_pcie_ubus_init(unit);
}

void pmc_pcie_power_down(int unit, int is_dual_lane)
{
	BPCM_SR_CONTROL sr_ctrl = {
		.Bits.sr = SOFT_RST_PCIE_PD, // Only iddq
	};
	int addr;

	if (unit >= MAX_PCIE_UNIT)
		BUG_ON(1);

	addr = pmc_pcie_port_info[unit].pmb_addr;

#if	 defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96846)
	/*On Bifurcation mode we must not shut down and de-register the ubus port of
	 * bifurcated port (0) since we don't know the future status of core 1 */
	if (pmc_pcie_ubus_node[unit].port_type == PCIE_BIFURCATED_PORT)
		return;
#endif

	if (pmc_pcie_port_info[unit].pll == TRUE) {
		// To power down PLL and PLL-LDO for PCIe gen 3
		sr_ctrl.Reg32 |= (SOFT_RST_PCIE_PLL_PD | SOFT_RST_PCIE_PLL_LDO_PD);
	}

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96855)
	{
		int dual_lane = is_dual_lane;
		int bifur_addr;
#ifndef CONFIG_DT_SUPPORT_ONLY
		/* Power down the other bi-furcated port if this supports dual lane */
		if (!is_dual_lane)
			BpGetPciPortDualLane(unit, &dual_lane);
#endif

		if (dual_lane) {
			/* Identify other bifurcated port address */
			bifur_addr = pmc_pcie_port_info[unit+1].pmb_addr;
			if (WriteBPCMRegister(bifur_addr, BPCMRegOffset(sr_control), sr_ctrl.Reg32))
				BUG_ON(1);
			if (PowerOffZone(bifur_addr, 0))
				BUG_ON(1);
		}
	}
#endif

	if (WriteBPCMRegister(addr, BPCMRegOffset(sr_control), sr_ctrl.Reg32))
		BUG_ON(1);

	mdelay(10);

	pmc_pcie_deregister_ubus(unit);

	if (PowerOffZone(addr, 0))
		BUG_ON(1);
}

EXPORT_SYMBOL(pmc_pcie_power_up);
EXPORT_SYMBOL(pmc_pcie_power_down);
