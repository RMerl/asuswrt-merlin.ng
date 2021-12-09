/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

#include "access_macros.h"
#include "boardparms.h"
#include "bcm_misc_hw_init.h"
#include "rdp_map.h"
#include "rdpa_types.h"
#include "phy_drv.h"
#include "pmc_drv.h"
#include "BPCM.h"
#ifndef _CFE_
#include "board.h"
#include "bcm_rsvmem.h"
#include "bl_os_wraper.h"
#include "wan_drv.h"
#include <bcmsfp_i2c.h>
#endif

#define SHIFTL(_a) ( 1 << _a)

#define BPCM_SRESET_CNTL_REG            BPCMRegOffset(sr_control)

#define RDP_S_RST_UBUS_DBR              0 // DMA ubus bridge
#define RDP_S_RST_UBUS_RABR             1 // Runner A ubus bridge
#define RDP_S_RST_UBUS_RBBR             2 // Runner B ubus bridge
#define RDP_S_RST_UBUS_VPBR             3 // VPB ubus bridge
#define RDP_S_RST_RNR_0                 4
#define RDP_S_RST_RNR_1                 5
#define RDP_S_RST_IPSEC_RNR             6
#define RDP_S_RST_IPSEC_RNG             7
#define RDP_S_RST_IPSEC_MAIN            8
#define RDP_S_RST_RNR_SUB               9
#define RDP_S_RST_IH_RNR               10
#define RDP_S_RST_BB_RNR               11
#define RDP_S_RST_GEN_MAIN             12 // (main tm)
#define RDP_S_RST_VDSL                 13
#define RDP_S_RST_E0_MAIN              14 // (emac0)
#define RDP_S_RST_E0_RST_L             15 // (emac0)
#define RDP_S_RST_E1_MAIN              16 // (emac1)
#define RDP_S_RST_E1_RST_L             17 // (emac1)
#define RDP_S_RST_E2_MAIN              18 // (emac2)
#define RDP_S_RST_E2_RST_L             19 // (emac2)
#define RDP_S_RST_E3_MAIN              20 // (emac3)
#define RDP_S_RST_E3_RST_L             21 // (emac3)
#define RDP_S_RST_E4_MAIN              22 // (emac4)
#define RDP_S_RST_E4_RST_L             23 // (emac4)

static const ETHERNET_MAC_INFO *emac_info;

static void pmc_rdp_module_unreset(uint32_t rdp_module)
{
    uint32_t reg;
    uint32_t ret;

    ret = ReadBPCMRegister(PMB_ADDR_RDP, BPCM_SRESET_CNTL_REG, (uint32 *)&reg);
    if (ret)
        printk("Failed to ReadBPCMRegister RDP block BPCM_SRESET_CNTL_REG. Error=%d\n", ret);

    reg &= ~(SHIFTL(rdp_module));

    ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCM_SRESET_CNTL_REG, reg);
    if (ret)
        printk("Failed to WriteBPCMRegister RDP block BPCM_SRESET_CNTL_REG. Error=%d\n", ret);
}

static void pmc_rdp_reset(void)
{
    uint32_t ret;

    ret = WriteBPCMRegister(PMB_ADDR_RDP, BPCM_SRESET_CNTL_REG, ~0);
    if (ret)
        printk("Failed to WriteBPCMRegister RDP block BPCM_SRESET_CNTL_REG. Error=%d\n", ret);
}

static void ubus_masters_enable(void)
{
    uint32_t reg;

    READ_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN, reg);
    reg |= 1;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN, reg);

    READ_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN, reg);
    reg |= 1;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN, reg);

    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN, reg);
    reg |= 1;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN, reg);

    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP, reg);
    reg |= 0xf0e01; // allow forwarding of Urgent to High priority on UBUS
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP,reg);
}

int rdp_pre_init(void)
{
    uint32_t iter;

    if ((emac_info = BpGetEthernetMacInfoArrayPtr()) == NULL)
    {
        printk("Error reading Ethernet MAC info from board params\n");
        return -1;
    }

    pmc_rdp_reset();

    pmc_rdp_module_unreset(RDP_S_RST_UBUS_DBR);
    pmc_rdp_module_unreset(RDP_S_RST_UBUS_RABR);
    pmc_rdp_module_unreset(RDP_S_RST_UBUS_RBBR);
    pmc_rdp_module_unreset(RDP_S_RST_UBUS_VPBR);

    ubus_masters_enable();

    pmc_rdp_module_unreset(RDP_S_RST_BB_RNR);
    udelay(100);

    pmc_rdp_module_unreset(RDP_S_RST_RNR_SUB);
    pmc_rdp_module_unreset(RDP_S_RST_RNR_1);
    pmc_rdp_module_unreset(RDP_S_RST_RNR_0);
    pmc_rdp_module_unreset(RDP_S_RST_IPSEC_RNR);
    pmc_rdp_module_unreset(RDP_S_RST_IPSEC_RNG);
    pmc_rdp_module_unreset(RDP_S_RST_IPSEC_MAIN);
    pmc_rdp_module_unreset(RDP_S_RST_GEN_MAIN);
    pmc_rdp_module_unreset(RDP_S_RST_IH_RNR);
    pmc_rdp_module_unreset(RDP_S_RST_BB_RNR);

    for (iter = 0; iter < 4; iter++)
    {
        if (emac_info->sw.port_map & (1 << iter))
        {
            pmc_rdp_module_unreset(RDP_S_RST_E0_MAIN + iter * 2);
            pmc_rdp_module_unreset(RDP_S_RST_E0_RST_L + iter * 2);
        }
    }

    return 0;
}
EXPORT_SYMBOL(rdp_pre_init);

#ifndef _CFE_
int bcm_misc_g9991_debug_port_get(void)
{
    int iter;

    for (iter = 0; iter < BP_MAX_SWITCH_PORTS; iter++)
    {
        if (emac_info[0].sw.port_map & (1 << iter) &&
            emac_info[0].sw.port_flags[iter] & PORT_FLAG_MGMT)
        {
            return (rdpa_emac)(rdpa_emac0 + iter);
        }
    }

    return -1;
}
EXPORT_SYMBOL(bcm_misc_g9991_debug_port_get);
    
#endif

int rdp_post_init(void)
{
    return 0;
}
EXPORT_SYMBOL(rdp_post_init);

int rdp_shut_down(void)
{
    pmc_rdp_reset();

    return 0;
}
EXPORT_SYMBOL(rdp_shut_down);

int bcm_misc_hw_init(void)
{
    rdp_pre_init();

    return 0;
}

#ifndef _CFE_
int runner_reserved_memory_get(uint32_t *tm_base_addr, uint32_t *tm_base_addr_phys,
    uint32_t *mc_base_addr, uint32_t *mc_base_addr_phys, uint32_t *tm_size, uint32_t *mc_size)
{
    int rc;
    phys_addr_t phy_addr;

    rc = BcmMemReserveGetByName(TM_BASE_ADDR_STR, (void**)tm_base_addr, &phy_addr, tm_size);
    if (rc)
    {
        printk("%s %s Failed to get TM_BASE_ADDR_STR rc(%d)\n", __FILE__, __FUNCTION__, rc);
    }
    else
    {
         *tm_base_addr_phys = phy_addr;

        rc = BcmMemReserveGetByName(TM_MC_BASE_ADDR_STR, (void**)mc_base_addr, &phy_addr, mc_size);
        if (rc)
        {
            printk("Failed to get valid DDR Multicast address, rc = %d\n", rc);
        }
        else
        {
            *mc_base_addr_phys = phy_addr;

            *tm_size /= 1024 * 1024;
            *mc_size /= 1024 * 1024;
        }
    }

    return rc;
}
EXPORT_SYMBOL(runner_reserved_memory_get);

int proc_show_rdp_mem(char *buf, char **start, off_t off, int cnt, int *eof, void *data)
{
    int rc;
    void *tm_base_addr;
    void *mc_base_addr;
    int size_dummy;
    int n = 0;

    rc = BcmMemReserveGetByName(TM_BASE_ADDR_STR, (void**)&tm_base_addr, NULL, &size_dummy);
    if (rc)
    {
        printk("%s %s Failed to get TM_BASE_ADDR_STR rc(%d)\n", __FILE__, __FUNCTION__, rc);
        return -1;
    }

    rc = BcmMemReserveGetByName(TM_MC_BASE_ADDR_STR, (void**)&mc_base_addr, NULL, &size_dummy);
    if (rc)
    {
        printk("%s %s Failed to get TM_BASE_ADDR_STR rc(%d)\n", __FILE__, __FUNCTION__, rc);
        return -1;
    }

    n = sprintf(buf, "RDP MEM tm_base=%pK mc_base=%pK\n",tm_base_addr, mc_base_addr);
    return n;

}

arch_initcall(bcm_misc_hw_init);
#endif
