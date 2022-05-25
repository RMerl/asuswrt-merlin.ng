/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

#include "boardparms.h"
#include "hwapi_mac.h"
#include "rdp_drv_bbh.h"
#include "rdp_ubus.h"
#include "bcm_map_part.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <bcm_map_part.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include "board.h"
#include "bcm_rsvmem.h"
#include "pmc_ssb_access.h"

#if defined(__KERNEL__) && defined(CONFIG_BCM_RDP)
struct device *rdp_dummy_dev = NULL;
EXPORT_SYMBOL(rdp_dummy_dev);
#endif

/* map SWITCH/CROSSBAR port numbers to the order of xMII pas control
   registers in the device
 */

/* this function performs any customization to the voltage regulator setting */
static void bcm_misc_hw_vr_init(void)
{
    unsigned short bmuen;

    /* set vr drive strength for bmu board */
    if ((BpGetBatteryEnable(&bmuen) == BP_SUCCESS) && (bmuen != 0)) {
        write_ssbm_reg(0x00+7,0x12d9,1);
        write_ssbm_reg(0x40+7,0x12d9,1);
        write_ssbm_reg(0x20+7,0x12d9,1);
    }

    /* Set 1.0V, 1.5V and 1.8V  digital voltage switching regulator's gain setting  based on 
       JIRA SWBCACPE-18708 and SWBCACPE-18709 */
#if defined(_BCM963148_) || defined(CONFIG_BCM963148)
    write_ssbm_reg(0x0, 0x840, 1);  /* Set 1.0V rail to the gain of 16*/
    write_ssbm_reg(0x20, 0x830, 1); /* Set 1.8V rail to the gain of 4*/
    write_ssbm_reg(0x40, 0x800, 1); /* Set 1.5V rail to the gain of 8*/
#endif

#if defined(_BCM963138_) || defined(CONFIG_BCM963138)
    write_ssbm_reg(0x0, 0x800, 1);  /* Set 1.0V rail to the gain of 8*/
    write_ssbm_reg(0x20, 0x830, 1); /* Set 1.8V rail to the gain of 4*/
    write_ssbm_reg(0x40, 0x800, 1); /* Set 1.5V rail to the gain of 8*/
#endif

#if defined(CONFIG_BCM963138)
    /* 63138 over current watchdog enabling. It must be done after all switch regulator register is set 
     * This is only needed if we use internal voltage regulator. Read 1.5V rail, if it is zero, the board
     * use external voltage regulator and do NOT need to enable the watchdog */
    if( read_ssbm_reg(0x40) != 0 ) {
        udelay(1000);
        enable_over_current_watchdog();
    }
#endif
    return;
}

#if defined(__KERNEL__) && defined(CONFIG_BCM_RDP)
static void alloc_rdp_dummy_device(void)
{
    if (rdp_dummy_dev == NULL) {
        rdp_dummy_dev = kzalloc(sizeof(struct device), GFP_ATOMIC);
        arch_setup_dma_ops(rdp_dummy_dev, 0, 0, NULL, false);
        dma_coerce_mask_and_coherent(rdp_dummy_dev, DMA_BIT_MASK(32));
    }
}
#endif


void rdp_enable_ubus_masters(void)
{
    UBUS_UBUS_MASTER_BRDG_REG_EN ubus_en;
    UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL ubus_req_ctl;
    UBUS_UBUS_MASTER_BRDG_REG_HP ubus_hp;

    /* Configuration taken from simulation registers. */

    /*first Ubus Master*/
    READ_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);
    ubus_req_ctl.max_pkt_len = 0x90;
    ubus_req_ctl.endian_mode = UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL_ENDIAN_MODE_LB_VALUE;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);

    READ_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);
    ubus_hp.hp_en = 1;
    ubus_hp.hp_cnt_high = 1;
    ubus_hp.hp_cnt_total = 2;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);

    READ_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);
    ubus_en.en = 1;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);

    /*second Ubus Master*/
    READ_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);
    ubus_req_ctl.max_pkt_len = 0x90;
    ubus_req_ctl.endian_mode = UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL_ENDIAN_MODE_LB_VALUE;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);

    READ_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);
    ubus_hp.hp_en = 1;
    ubus_hp.hp_cnt_high = 11;
    ubus_hp.hp_cnt_total = 13;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);

    READ_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);
    ubus_en.en = 1;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);

    /*third Ubus Master*/
    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);
    ubus_req_ctl.max_pkt_len = 0x90;
    ubus_req_ctl.endian_mode = UBUS_UBUS_MASTER_BRDG_REG_REQ_CNTRL_ENDIAN_MODE_LB_VALUE;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_REQ_CNTRL,ubus_req_ctl);

    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);
    ubus_hp.hp_en = 1;
    ubus_hp.hp_comb = 1;
    ubus_hp.hp_cnt_high = 1;
    ubus_hp.hp_cnt_total = 6;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_HP,ubus_hp);

    READ_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);
    ubus_en.en = 1;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN,ubus_en);

    // UBUS arbitration configuration, done through clocks and reset (via PMC)
}

int runner_reserved_memory_get( uint32_t *tm_base_addr, uint32_t *tm_base_addr_phys,
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
            memset((void*)*tm_base_addr, 0x00, *tm_size);
            memset((void*)*mc_base_addr, 0x00, *mc_size);

            printk("tm_base_addr 0x%px, size %u, tm_base_addr_phys 0x%px\n",
                (void*)*tm_base_addr, *tm_size, (void*)*tm_base_addr_phys);

            *mc_base_addr_phys = phy_addr;
            printk("mc_base_addr 0x%px, size %u, mc_base_addr_phys 0x%08x\n",
                (void*)*mc_base_addr, *mc_size, *mc_base_addr_phys);

            *tm_size /= 1024 * 1024;
            *mc_size /= 1024 * 1024;
        }
    }

    return rc;
}

EXPORT_SYMBOL(runner_reserved_memory_get);

int rdp_post_init(void)
{
#if defined(CONFIG_BCM_ENET)
    /* Ethernet WAN */
    mac_hwapi_init_emac(DRV_BBH_EMAC_0);
    mac_hwapi_set_unimac_cfg(DRV_BBH_EMAC_0,1);
    mac_hwapi_set_rxtx_enable(DRV_BBH_EMAC_0,0,0);/* Ethernet WAN EMAC will be enabled when the WAN service gets created */
    mac_hwapi_set_tx_max_frame_len(DRV_BBH_EMAC_0, 2048); /* Why do we set the max frame len here 'hard-coded' ??? FIXME */

    /* SF2 */
    mac_hwapi_init_emac(DRV_BBH_EMAC_1);
    mac_hwapi_set_unimac_cfg(DRV_BBH_EMAC_1,1);
    mac_hwapi_set_rxtx_enable(DRV_BBH_EMAC_1,1,1);
    mac_hwapi_set_tx_max_frame_len(DRV_BBH_EMAC_1, 2048); /* Why do we set the max frame len here 'hard-coded' ??? FIXME */
    mac_hwapi_set_backpressure_ext(DRV_BBH_EMAC_1, 1); /* Enable backpressure towards SF2 */
#endif
    return 0;
}
EXPORT_SYMBOL(rdp_post_init);

int rdp_shut_down(void)
{
    /*put all RDP modules in reset state*/
    // TBD. pmcPutAllRdpModulesInReset();
    return 0;
}
EXPORT_SYMBOL(rdp_shut_down);

int bcm_misc_hw_init(void)
{
    bcm_misc_hw_vr_init();
#if defined(__KERNEL__) && defined(CONFIG_BCM_RDP)
    alloc_rdp_dummy_device();
    rdp_enable_ubus_masters();
#endif

    return 0;
}

arch_initcall(bcm_misc_hw_init);

