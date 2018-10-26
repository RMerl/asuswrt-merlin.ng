/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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

/*
 *  Created on: Sep 2017
 *      Author: li.xu@broadcom.com
 */

/*
 * Phy drivers for 10G Active Ethernet Serdes
 */
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include "bus_drv.h"
#include "phy_drv_sf2.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
//#include "crossbar_drv_sf2.h"
#include "bcm_map_part.h"
#include "bcmnet.h"
#include "board.h"
#include "bcm_otp.h"
#include "bcm_OS_Deps.h"
#include "pmc_drv.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"
#include "bcmsfp_i2c.h"
#include "opticaldet.h"
#include "bcm_map_part.h"
#include "bcmnet.h"
#include "board.h"
#include "bcm_otp.h"
#include "linux/kernel.h"
#include "linux/spinlock.h"
#include "bcm_OS_Deps.h"
#include "BPCM.h"
#include "pmc_wan.h"
#include "phy_drv_xgae.h"
#include "bcm_physical_map_part.h"

#define error_log printk

#define wr_xgae_serdes_reg(serdes_inst, addr, wr_data, mask_data) \
    _writeXgaeSerdesReg(serdes_inst==SERDES_1, addr, wr_data, mask_data)

#define set_mask_read_data(d) udelay(1000)
#define clr_mask_read_data() udelay(1000)
static int xgae_wan_top_init(phy_serdes_t *phy_serdes);

static inline void xgae_pll_powerdown_reset(phy_dev_t *phy_dev, int rx_pll, int tx_pll)
{
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1|tx_pll|CORE_PLL_COM_TOP_USER_CONTROL), 0x4000, ~(0x6000)); // afe_s_pll_pwrdn = 1, core_dp_s_rstb = 0
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1|rx_pll|CORE_PLL_COM_TOP_USER_CONTROL), 0x4000, ~(0x6000)); // afe_s_pll_pwrdn = 1, core_dp_s_rstb = 0
    serdes_wait_nsec(1000);
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1|tx_pll|CORE_PLL_COM_TOP_USER_CONTROL), 0x2000, ~(0x6000)); // afe_s_pll_pwrdn = 0, core_dp_s_rstb = 1
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1|rx_pll|CORE_PLL_COM_TOP_USER_CONTROL), 0x2000, ~(0x6000)); // afe_s_pll_pwrdn = 0, core_dp_s_rstb = 1
}

#define SerdesPrint(phy_serdes, x, y...) if(phy_serdes->serdes_status == SERDES_NOT_INITED) printk(x, ##y)
static int poll_pll_lock(phy_serdes_t *phy_serdes, int pll_inst, int poll_cnt)
{
    int pll_lock;
    int pll_lock_cnt;
    int pll_error;
    uint32_t rd_data = 0x00000000;

    SerdesPrint(phy_serdes, "pa_comment: Polling for PLL%0d Lock on Serdes\n", pll_inst);

    pll_lock = 0;
    pll_lock_cnt = 0;
    pll_error = -1;
    set_mask_read_data(0x00000000);
    while ((pll_lock == 0) && (pll_lock_cnt < poll_cnt)) {
        if (pll_inst == 1) {
            rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | PLL_1 | PLL_CAL_COM_CTL_STATUS_0));
        } else {
            rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | PLL_0 | PLL_CAL_COM_CTL_STATUS_0));
        }

        pll_lock = ((rd_data & 0x00000100) == 0x00000100);

        //SerdesPrint(phy_serdes, "pa_comment: serdes_pll_lock pll_inst=%0d poll_cnt=%0d pll_lock=%0d\n", pll_inst, pll_lock_cnt, pll_lock);
        pll_lock_cnt = pll_lock_cnt + 1;
    }

  if (pll_lock == 0) {
      printk("pa_comment: ********** PLL LOCK ERROR pll_inst=%0d poll_cnt=%0d PLL_CAL_COM_CTL_STATUS_0=%0d\n",
              pll_inst, pll_lock_cnt, rd_data);
  } else {
      SerdesPrint(phy_serdes, "pa_comment: serdes_pll_locked pll_inst=%0d poll_cnt=%0d pll_lock=%0d\n", pll_inst, pll_lock_cnt, pll_lock);
      pll_error = 0;
  }
  clr_mask_read_data();

  return (pll_error);
}

int xgae_los_asserted(phy_dev_t *phy_dev)
{
    u32 v32;

    v32 = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | SIGDET_SIGDET_STATUS_0 ));
    return !(v32 & signal_detect_raw);
}

static int poll_pll_lock_wan(phy_serdes_t *phy_serdes, int pll_inst, int poll_cnt)
{

    int error_cnt = 0;
    uint32_t rd_data = 0x00000000;
    int itr = 0;

    // Check PLL lock
#define LOCK_POLL_ENABLE
#ifdef LOCK_POLL_ENABLE

    //SerdesPrint(phy_serdes, "pa_comment: Polling for PLL%0d Lock on Serdes\n", pll_inst);

    serdes_wait_nsec(5000);
    rd_data = 0;
    itr = 0;
    while(1) {
        //SerdesPrint(phy_serdes, "pa_comment: poll_pll_lock_wan: SGB_SERDES_INST: Polling PLL lock .. %0d/10\n", itr);
        REG_READ_32(&WAN_TOP->WAN_TOP_SERDES_STATUS, rd_data);
        if (((pll_inst == 0) & ((rd_data & 0x00000001) != 0)) ||
                ((pll_inst == 1) & ((rd_data & 0x00000400) != 0)) ) {
            SerdesPrint(phy_serdes, "pa_comment: poll_pll_lock_wan: SGB_SERDES_INST: PLL lock achieved!!\n");
            break;
        } else {
            if (itr++ == 40) {
                printk("pa_comment: poll_pll_lock_wan: SGB_SERDES_INST: PLL lock timeout!!\n");
                error_cnt++;
                break;
            }
            serdes_wait_nsec(100);
            continue;
        }
    }

#else // LOCK_POLL_ENABLE

    SerdesPrint(phy_serdes, "pa_comment: poll_pll_lock_wan LOCK_FIXED_WAIT\n");

    sim_wait_time_nsec = 20000;
    //printk("pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    printk("pa_comment: SGB_SERDES_INST: Checking PLL lock.\n");
    REG_READ_32(&WAN_TOP->WAN_TOP_SERDES_STATUS, rd_data);
    if (((pll_inst == 0) & ((rd_data & 0x00000001) != 0)) ||
            ((pll_inst == 1) & ((rd_data & 0x00000400) != 0)) ) {
        printk("pa_comment: poll_pll_lock_wan: SGB_SERDES_INST: PLL lock achieved!!\n");
    } else {
        error_log("pa_comment: SGB_SERDES_INST: PLL lock timeout!!\n");
        error_cnt++;
    }

#endif // LOCK_POLL_ENABLE

    return (error_cnt);
}

static int serdes_enable (phy_serdes_t *phy_serdes, int serdes_speed_mode, int serdes_rate_mode)
{
    int sim_wait_time_nsec;
    //int prbs_mode = 100;
    uint32_t serdes_addr = LANE_BRDCST;
    int error_cnt = 0;
    uint32_t wr_data = 0x00000000;
    uint32_t rd_data = 0x00000000;
    // int itr = 0;

    serdes_addr = LANE_0;

    SerdesPrint(phy_serdes, "pa_comment: serdes_enable serdes_speed_mode:%d serdes_rate_modee:%d\n",
            serdes_speed_mode, serdes_rate_mode);

    sim_wait_time_nsec = 10000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    // SERDES Registers:
    //
    // #---------------------------
    // # Startup Sequence
    // #---------------------------
    //
    // # 1. Assert POR reset by forcing pmd_por_h_rstb pin to 1'b0.
    //      Optionally if out of POR then core_s_rstb can be asserted by writing to 1'b0 to reset the whole core.
    //
    //reg_model.wan_top_reg_blk.wan_top_misc_0_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 16)); // cr_xgwan_top_wan_misc_pmd_lane_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000001f) << 06)); // cr_xgwan_top_wan_misc_onu2g_phya.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 05)); // cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 04)); // cr_xgwan_top_wan_misc_mdio_fast_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 03)); // cr_xgwan_top_wan_misc_mdio_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 02)); // cr_xgwan_top_wan_misc_refout_en.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) << 01)); // cr_xgwan_top_wan_misc_refin_en.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 00)); // cr_xgwan_top_wan_misc_onu2g_pmd_status_sel.set(0);

    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_0, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_0, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 0 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 0 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_1_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 16)); // cr_xgwan_top_wan_misc_pmd_core_1_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 00)); // cr_xgwan_top_wan_misc_pmd_core_0_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_1, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_1, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 1 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 1 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_2_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfgNgponRxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfgNgponTxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 20)); // cr_xgwan_top_wan_misc_pmd_rx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_pmd_tx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000003) << 16)); // cr_xgwan_top_wan_misc_pmd_tx_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cr_xgwan_top_wan_misc_pmd_tx_disable.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  8)); // cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  6)); // cr_xgwan_top_wan_misc_pmd_ext_los.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  5)); // cr_xgwan_top_wan_misc_pmd_por_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  4)); // cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  3)); // cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  2)); // cr_xgwan_top_wan_misc_pmd_ln_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_pmd_rx_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_2, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 2 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 2 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_3_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) << 16)); // cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) <<  8)); // cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_wan_cfg_laser_oe.set(1);

    if (serdes_speed_mode == eSPEED_1_1) {
        wr_data = (wr_data | ((0x00000001 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(1);
    } else if (serdes_speed_mode == eSPEED_10_10) {
        wr_data = (wr_data | ((0x00000003 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(3);
    } else if (serdes_speed_mode == eSPEED_2_2) {
        wr_data = (wr_data | ((0x00000002 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(2);
    } else if (serdes_speed_mode == eSPEED_100m_100m) {
        wr_data = (wr_data | ((0x00000000 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(0);
    }
    wr_data = (wr_data | ((0x00000000 & 0x00000003) <<  2)); // cr_xgwan_top_wan_misc_wan_cfg_laser_mode.set(0);
    SerdesPrint(phy_serdes, "pa_comment: CONFIG_BCM963158 0, MISC, 3 written: wr_data=0x%08x\n", wr_data);

    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_wan_cfg_laser_invert.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_wan_cfg_mem_reb.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_3, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_3, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 3 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 3 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_serdes_pll_ctl_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 10)); // cfg_pll1_lcref_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cfg_pll1_refout_en.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  8)); // cfg_pll1_refin_en.set(1);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  2)); // cfg_pll0_lcref_sel.set(1);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cfg_pll0_refout_en.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  0)); // cfg_pll0_refin_en.set(1);
    REG_WRITE_32(&WAN_TOP->WAN_SERDES_PLL_CTL, wr_data);
    REG_READ_32(&WAN_TOP->WAN_SERDES_PLL_CTL, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        // SerdesPrint(phy_serdes, "pa_comment: 0, WAN_SERDES, PLL_CTL written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, WAN_SERDES, PLL_CTL ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_serdes_pram_ctl_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 31)); // cfg_pram_go.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfg_pram_we.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfg_pram_cs.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 24)); // cfg_pram_ability.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) << 16)); // cfg_pram_datain.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) <<  0)); // cfg_pram_addr.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_SERDES_PRAM_CTL, wr_data);
    REG_READ_32(&WAN_TOP->WAN_SERDES_PRAM_CTL, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        // SerdesPrint(phy_serdes, "pa_comment: 0, WAN_SERDES, PRAM_CTL written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, WAN_SERDES, PRAM_CTL ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    // # 2. Wait for stable refclk.
    sim_wait_time_nsec = 10000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);


    // # 3. De-assert pmd_por_h_rstb pin OR core_s_rstb register (only required if this register
    // was manually written to 1'b0. Out of POR de-assertion, core_s_rstb is 1'b1) by making them 1'b1.

    //reg_model.wan_top_reg_blk.wan_top_misc_2_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfgNgponRxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfgNgponTxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 20)); // cr_xgwan_top_wan_misc_pmd_rx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_pmd_tx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000003) << 16)); // cr_xgwan_top_wan_misc_pmd_tx_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cr_xgwan_top_wan_misc_pmd_tx_disable.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  8)); // cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  6)); // cr_xgwan_top_wan_misc_pmd_ext_los.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  5)); // cr_xgwan_top_wan_misc_pmd_por_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  4)); // cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  3)); // cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  2)); // cr_xgwan_top_wan_misc_pmd_ln_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_pmd_rx_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_2, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 2 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 2 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }


    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txlbe_ser_order.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000007) <<  4)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txlbe_ser_init_val.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  3)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txlbe_ser_en.set(0);
    wr_data = (wr_data | (((serdes_rate_mode == eSUB_RATE) & 0x00000001) <<  2)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txfifo_rd_legacy_mode.set(serdes_rate_mode == eSUB_RATE;
    wr_data = (wr_data | ((0x00000000 & 0x00000003) <<  0)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_cfg_gpon_rx_clk.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_OSR_CONTROL, wr_data);

    sim_wait_time_nsec = 10000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    // # Test SerDes register access
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        SerdesPrint(phy_serdes, "pa_comment: Test SerDes register access\n");
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID0), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID0), 0x42e5, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID1), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID1), 0x1034, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID2), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID2), 0x0000, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x5555, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x5555, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xaaaa, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xaaaa, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xffff, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xffff, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x1234, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x1234, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, 0x0000);
    }

    return error_cnt;
}

int ethsw_xgae_enable_an(phy_dev_t *phy_dev)
{
    uint32_t rd_data;

    rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | MII_CONTROL));
    rd_data |= MII_CONTROL_AN_ENABLE|MII_CONTROL_RESTART_AUTONEG;
    wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | MII_CONTROL), rd_data, ~(0xffff));
    msleep(50);
    return 0;
}

int _ethsw_xgae_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex, int force)
{
    phy_serdes_t *phy_serdes = phy_dev->priv;
    phy_dev_t *cascade_next = phy_dev->cascade_next;
    int rc = -1;
    int serdes_speed_mode;
    int serdes_rate_mode = eFULL_RATE;
    int prbs_type = -1;

    int sim_wait_time_nsec;
    int prbs_mode = 100;
    uint32_t serdes_addr = LANE_BRDCST;
    int error_cnt = 0;
    uint32_t wr_data = 0x00000000;
    uint32_t rd_data = 0x00000000;
    int itr = 0;
    int tx_pll_sel = 0;
    int rx_pll_sel = 1;
    uint32_t tx_pll_id = PLL_0;
    uint32_t rx_pll_id = PLL_1;
    int wait_cnt_max = 12;
    int wait_cnt = 0;
    //int rx_dsc_lock = 0;
    //int pmd_rx_lock = 0;

    uint16_t lp_wr_data = 0;
    uint16_t lp_wr_mask = 0;
    //uint16_t lp_rd_data = 0;

    uint16_t tx_pll_vco_div2;
    uint16_t rx_pll_vco_div2;
    uint16_t tx_pll_vco_div4;
    uint16_t rx_pll_vco_div4;
    uint16_t tx_pll_force_kvh_bw;
    uint16_t rx_pll_force_kvh_bw;
    uint16_t tx_pll_2rx_bw;
    uint16_t rx_pll_2rx_bw;
    uint16_t tx_pll_fracn_sel;
    uint16_t rx_pll_fracn_sel;
    uint16_t tx_pll_ditheren;
    uint16_t rx_pll_ditheren;
    uint32_t tx_pll_fracn_div;
    uint32_t rx_pll_fracn_div;
    uint16_t tx_pll_fracn_ndiv;
    uint16_t rx_pll_fracn_ndiv;
    uint16_t tx_pll_mode;
    uint16_t rx_pll_mode;
    uint16_t tx_pon_mac_ctrl;
    uint16_t tx_sync_e_ctrl;
    uint16_t rx_pon_mac_ctrl;
    uint16_t rx_tx_rate_ratio;
    uint16_t rx_osr_mode;
    uint16_t tx_osr_mode;

    if (phy_serdes->serdes_status == SERDES_NOT_INITED) 
        force = 1;

    if (!force) {
        if (phy_serdes->serdes_status > SERDES_NOT_INITED) {
            if (xgae_los_asserted(phy_dev))
                goto ret;

            if (phy_serdes->serdes_status == SERDES_PCS_CONFIGURED &&
                    phy_serdes->current_speed == speed) {
                rc = 0;
                goto ret;
            }
        }
    }

    if (phy_serdes->sfp_module_type == SFP_COPPER) {
            serdes_speed_mode = eSPEED_1_1;
    } else {
        switch (speed) {
            case PHY_SPEED_10000:
                serdes_speed_mode = eSPEED_10_10;
                break;
            case PHY_SPEED_2500:
                serdes_speed_mode = eSPEED_2_2;
                break;
            case PHY_SPEED_1000:
                serdes_speed_mode = eSPEED_1_1;
                break;
            case PHY_SPEED_100:
                serdes_speed_mode = eSPEED_100m_100m;
                serdes_rate_mode = eSUB_RATE;
                break;
            default:
                goto ret;
        }
    }

    phy_serdes->current_speed = speed;

    xgae_wan_top_init(phy_serdes);
    serdes_enable(phy_serdes, serdes_speed_mode, serdes_rate_mode);

    serdes_addr = LANE_0;

    sim_wait_time_nsec = 10000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);


    if (serdes_speed_mode == eSPEED_10_1) {
        rx_pll_sel = 1;
        rx_pll_id = PLL_1;
    }

    // rx_pon_mac_ctrl<2:0> | Div Ratio
    // 111 82.5 (DFS)
    // 110 64
    // 101 40
    // 100 32
    // 011 20
    // 010 16
    // 001 10
    // 000 invalid

    // tx_pon_mac_ctrl<2:0> | Div Ratio
    // 111 82.5 (DFS)
    // 110 64
    // 101 40
    // 100 32
    // 011 20
    // 010 16
    // 001 10
    // 000 invalid state

    // tx_sync_e_ctrl<2:0> | Div Ratio
    // 111 150 (DFS)
    // 110 64
    // 101 25
    // 100 20
    // 011 16
    // 010 invalid
    // 001 invalid
    // 000 invalid

    // PLL frequency programming
    // The PLL output frequency is given by, Fvco = Fref x N
    // pll_mode[3:0],  N division ratio, KVH[1:0], PAD_PTEST / Refclk (freq ratio), Notes
    // 0000 46 00 2
    // 0001 72 01 2
    // 0010 40 00 2
    // 0011 42 00 1
    // 0100 48      20 00 1 use for 1T XAUI
    // 0101 50 00 1
    // 0110 52 01 1
    // 0111 54      32 00 2 use for 1T 5Gb/s
    // 1000 60 10 2
    // 1001 64 00 4
    // 1010 66 00 1
    // 1011 68 1
    // 1100 70 01 4
    // 1101 80 00 4
    // 1110 92 00 4
    // 1111 100 00 8

    tx_pll_vco_div2     = 0x0;
    rx_pll_vco_div2     = 0x0;
    tx_pll_vco_div4     = 0x0;
    rx_pll_vco_div4     = 0x0;
    tx_pll_force_kvh_bw = 0x0;
    rx_pll_force_kvh_bw = 0x0;
    tx_pll_2rx_bw       = 0x0;
    rx_pll_2rx_bw       = 0x0;
    tx_pll_fracn_sel    = 0x0;
    rx_pll_fracn_sel    = 0x0;
    tx_pll_ditheren     = 0x0;
    rx_pll_ditheren     = 0x0;
    tx_pll_fracn_div    = 0x00000;
    rx_pll_fracn_div    = 0x00000;
    tx_pll_fracn_ndiv   = 0x000;
    rx_pll_fracn_ndiv   = 0x000;
    tx_pll_mode         = 0x5;
    rx_pll_mode         = 0x5;
    tx_pon_mac_ctrl     = 0x0;
    tx_sync_e_ctrl      = 0x0;
    rx_pon_mac_ctrl     = 0x0;
    rx_tx_rate_ratio    = 0x0;
    rx_osr_mode         = 0x0;
    tx_osr_mode         = 0x0;

    if (serdes_speed_mode == eSPEED_10_10) {
        tx_pll_vco_div2     = 0x0;
        rx_pll_vco_div2     = 0x0;
        tx_pll_vco_div4     = 0x0;
        rx_pll_vco_div4     = 0x0;
        tx_pll_force_kvh_bw = 0x1;
        rx_pll_force_kvh_bw = 0x1;
        tx_pll_2rx_bw       = 0x0;
        rx_pll_2rx_bw       = 0x0;
        tx_pll_fracn_sel    = 0x1;
        rx_pll_fracn_sel    = 0x1;
        tx_pll_ditheren     = 0x1;
        rx_pll_ditheren     = 0x1;
        tx_pll_fracn_div    = 0x10000;
        rx_pll_fracn_div    = 0x10000;
        tx_pll_fracn_ndiv   = 0x0ce;
        rx_pll_fracn_ndiv   = 0x0ce;
        tx_pll_mode         = 0x2;
        rx_pll_mode         = 0x2;
        tx_pon_mac_ctrl     = 0x3;
        tx_sync_e_ctrl      = 0x7;
        rx_pon_mac_ctrl     = 0x3;
        rx_tx_rate_ratio    = 0x0;
        rx_osr_mode         = 0x0;
        tx_osr_mode         = 0x0;
    } else if ((serdes_speed_mode == eSPEED_2_2)) {
        if (serdes_rate_mode == eFULL_RATE) {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x0;
            rx_pll_vco_div4     = 0x0;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_2rx_bw       = 0x0;
            rx_pll_2rx_bw       = 0x0;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x00000;
            tx_pll_fracn_ndiv   = 0x0fa;
            rx_pll_fracn_ndiv   = 0x0fa;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x5;
            tx_sync_e_ctrl      = 0x1;
            rx_pon_mac_ctrl     = 0x5;
            rx_tx_rate_ratio    = 0x0;
            rx_osr_mode         = 0x4;
            tx_osr_mode         = 0x4;
        } else {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x1;
            rx_pll_vco_div4     = 0x1;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_2rx_bw       = 0x3;
            rx_pll_2rx_bw       = 0x3;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x00000;
            tx_pll_fracn_ndiv   = 0x0fa;
            rx_pll_fracn_ndiv   = 0x0fa;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x1;
            tx_sync_e_ctrl      = 0x5;
            rx_pon_mac_ctrl     = 0x1;
            rx_tx_rate_ratio    = 0x0;
            rx_osr_mode         = 0x0;
            tx_osr_mode         = 0x0;
        }
    } else if (serdes_speed_mode == eSPEED_1_1) {
        if (serdes_rate_mode == eFULL_RATE) {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x0;
            rx_pll_vco_div4     = 0x0;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_2rx_bw       = 0x0;
            rx_pll_2rx_bw       = 0x0;
            tx_pll_fracn_sel    = 0x1;
            rx_pll_fracn_sel    = 0x1;
            tx_pll_ditheren     = 0x1;
            rx_pll_ditheren     = 0x1;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x00000;
            tx_pll_fracn_ndiv   = 0x0c8;
            rx_pll_fracn_ndiv   = 0x0c8;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x0;
            tx_sync_e_ctrl      = 0x0;
            rx_pon_mac_ctrl     = 0x0;
            rx_tx_rate_ratio    = 0x0;
            rx_osr_mode         = 0x7;
            tx_osr_mode         = 0x7;
        } else {
            tx_pll_vco_div2     = 0x0;
            rx_pll_vco_div2     = 0x0;
            tx_pll_vco_div4     = 0x1;
            rx_pll_vco_div4     = 0x1;
            tx_pll_force_kvh_bw = 0x1;
            rx_pll_force_kvh_bw = 0x1;
            tx_pll_2rx_bw       = 0x3;
            rx_pll_2rx_bw       = 0x3;
            tx_pll_fracn_sel    = 0x0;
            rx_pll_fracn_sel    = 0x0;
            tx_pll_ditheren     = 0x0;
            rx_pll_ditheren     = 0x0;
            tx_pll_fracn_div    = 0x00000;
            rx_pll_fracn_div    = 0x00000;
            tx_pll_fracn_ndiv   = 0x000;
            rx_pll_fracn_ndiv   = 0x000;
            tx_pll_mode         = 0x5;
            rx_pll_mode         = 0x5;
            tx_pon_mac_ctrl     = 0x3;
            tx_sync_e_ctrl      = 0x4;
            rx_pon_mac_ctrl     = 0x3;
            rx_tx_rate_ratio    = 0x0;
            rx_osr_mode         = 0x1;
            tx_osr_mode         = 0x1;
        }
    } else if (serdes_speed_mode == eSPEED_100m_100m) {
        tx_pll_vco_div2     = 0x0;
        rx_pll_vco_div2     = 0x0;
        tx_pll_vco_div4     = 0x1;
        rx_pll_vco_div4     = 0x1;
        tx_pll_force_kvh_bw = 0x1;
        rx_pll_force_kvh_bw = 0x1;
        tx_pll_2rx_bw       = 0x3;
        rx_pll_2rx_bw       = 0x3;
        tx_pll_fracn_sel    = 0x0;
        rx_pll_fracn_sel    = 0x0;
        tx_pll_ditheren     = 0x0;
        rx_pll_ditheren     = 0x0;
        tx_pll_fracn_div    = 0x00000;
        rx_pll_fracn_div    = 0x00000;
        tx_pll_fracn_ndiv   = 0x000;
        rx_pll_fracn_ndiv   = 0x000;
        tx_pll_mode         = 0x2;
        rx_pll_mode         = 0x2;
        tx_pon_mac_ctrl     = 0x0;
        tx_sync_e_ctrl      = 0x4;
        rx_pon_mac_ctrl     = 0x0;
        rx_tx_rate_ratio    = 0x0;
        rx_osr_mode         = 0xa;
        tx_osr_mode         = 0xa;
    }

    // SERDES Registers:
    //
    // #---------------------------
    // # Startup Sequence
    // #---------------------------
    //
    // # 1. Assert POR reset by forcing pmd_por_h_rstb pin to 1'b0.
    // Optionally if out of POR then core_s_rstb can be asserted by writing to 1'b0
    // to reset the whole core.
    //
    //reg_model.wan_top_reg_blk.wan_top_misc_0_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 16)); // cr_xgwan_top_wan_misc_pmd_lane_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000001f) << 06)); // cr_xgwan_top_wan_misc_onu2g_phya.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 05)); // cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 04)); // cr_xgwan_top_wan_misc_mdio_fast_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 03)); // cr_xgwan_top_wan_misc_mdio_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 02)); // cr_xgwan_top_wan_misc_refout_en.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) << 01)); // cr_xgwan_top_wan_misc_refin_en.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 00)); // cr_xgwan_top_wan_misc_onu2g_pmd_status_sel.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_0, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_MISC_0, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 0 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, MISC, 0 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_1_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 16)); // cr_xgwan_top_wan_misc_pmd_core_1_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 00)); // cr_xgwan_top_wan_misc_pmd_core_0_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_1, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_MISC_1, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 1 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, MISC, 1 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_2_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfgNgponRxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfgNgponTxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 20)); // cr_xgwan_top_wan_misc_pmd_rx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_pmd_tx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000003) << 16)); // cr_xgwan_top_wan_misc_pmd_tx_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cr_xgwan_top_wan_misc_pmd_tx_disable.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  8)); // cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  6)); // cr_xgwan_top_wan_misc_pmd_ext_los.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  5)); // cr_xgwan_top_wan_misc_pmd_por_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  4)); // cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  3)); // cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  2)); // cr_xgwan_top_wan_misc_pmd_ln_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_pmd_rx_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_MISC_2, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 2 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, MISC, 2 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_3_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) << 16)); // cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) <<  8)); // cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_wan_cfg_laser_oe.set(0);

    if (serdes_speed_mode == eSPEED_1_1) {
        wr_data = (wr_data | ((0x00000001 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(1);
    } else if (serdes_speed_mode == eSPEED_10_10) {
        wr_data = (wr_data | ((0x00000003 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(3);
    } else if (serdes_speed_mode == eSPEED_2_2) {
        wr_data = (wr_data | ((0x00000002 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(2);
    } else if (serdes_speed_mode == eSPEED_100m_100m) {
        wr_data = (wr_data | ((0x00000000 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(0);
    }
    wr_data = (wr_data | ((0x00000000 & 0x00000003) <<  2)); // cr_xgwan_top_wan_misc_wan_cfg_laser_mode.set(0);
    //SerdesPrint(phy_serdes, "pa_comment: CONFIG_BCM963158 0, MISC, 3 written: wr_data=0x%08x\n", wr_data);

    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_wan_cfg_laser_invert.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_wan_cfg_mem_reb.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_3, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_MISC_3, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 3 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, MISC, 3 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    //reg_model.wan_top_reg_blk.wan_serdes_pll_ctl_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 10)); // cfg_pll1_lcref_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cfg_pll1_refout_en.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  8)); // cfg_pll1_refin_en.set(1);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  2)); // cfg_pll0_lcref_sel.set(1);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cfg_pll0_refout_en.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  0)); // cfg_pll0_refin_en.set(1);
    REG_WRITE_32(&WAN_TOP->WAN_SERDES_PLL_CTL, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_SERDES_PLL_CTL, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            // SerdesPrint(phy_serdes, "pa_comment: 0, WAN_SERDES, PLL_CTL written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, WAN_SERDES, PLL_CTL ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    //reg_model.wan_top_reg_blk.wan_serdes_pram_ctl_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 31)); // cfg_pram_go.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfg_pram_we.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfg_pram_cs.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 24)); // cfg_pram_ability.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) << 16)); // cfg_pram_datain.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) <<  0)); // cfg_pram_addr.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_SERDES_PRAM_CTL, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_SERDES_PRAM_CTL, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            // SerdesPrint(phy_serdes, "pa_comment: 0, WAN_SERDES, PRAM_CTL written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, WAN_SERDES, PRAM_CTL ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    // # 2. Wait for stable refclk.
    //
    sim_wait_time_nsec = 10000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);


    // # 3. De-assert pmd_por_h_rstb pin OR core_s_rstb register (only required if this
    // register was manually written to 1'b0. Out of POR de-assertion, core_s_rstb is 1'b1) by making them 1'b1.
    //
    //reg_model.wan_top_reg_blk.wan_top_misc_2_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfgNgponRxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfgNgponTxClk.set(0);
    wr_data = (wr_data | ((rx_osr_mode & 0x0000000f) << 20)); // cr_xgwan_top_wan_misc_pmd_rx_osr_mode.set(rx_osr_mode);
    wr_data = (wr_data | ((tx_osr_mode & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_pmd_tx_osr_mode.set(tx_osr_mode);
    wr_data = (wr_data | ((0x00000000 & 0x00000003) << 16)); // cr_xgwan_top_wan_misc_pmd_tx_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cr_xgwan_top_wan_misc_pmd_tx_disable.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  8)); // cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  6)); // cr_xgwan_top_wan_misc_pmd_ext_los.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  5)); // cr_xgwan_top_wan_misc_pmd_por_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  4)); // cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  3)); // cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  2)); // cr_xgwan_top_wan_misc_pmd_ln_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_pmd_rx_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_MISC_2, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            // SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 2 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, MISC, 2 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    sim_wait_time_nsec = 10000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);



    // # Test SerDes register access
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID0), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID0), 0x42e5, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID1), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID1), 0x1034, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID2), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_REVID2), 0x0000, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x5555, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x5555, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xaaaa, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xaaaa, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xffff, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0xffff, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x1234, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x1234, 0x0000);
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, ~(0xffff));
        rd_ver_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MDIO_MMDSEL_AER_COM_mdio_maskdata), 0x0000, 0x0000);
    }


    // # 4. Setup 2 PLL and 2 PLL calibration (common registers) :
    // //If common or one PLL is used for TX and RX, we need to use PLL0
    //
    // // To enable PLL0, PMI addr[25:24] = 2b00
    // Write (ams_pll_pwrdn , 1b0);  // always power up PLL0
    //
    // // To enable PLL1, PMI addr[25:24] = 2b01
    // Write (ams_pll_pwrdn , 1b0);  // not power down PLL1
    //
    SerdesPrint(phy_serdes, "pa_comment: Configure AMS_COM_PLL_INTCTRL\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | tx_pll_id | AMS_COM_PLL_INTCTRL), 0x0000, ~(0x0004)); // ams_pll_pwrdn = 0
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | rx_pll_id | AMS_COM_PLL_INTCTRL), 0x0000, ~(0x0004)); // ams_pll_pwrdn = 0


    // //  to select  source of TX clock
    // Write (tx_pll_select, val);
    // // val=0 tx_clk from PLL0
    // // val=1 tx_clk from PLL1
    //
    // //  to select  source of RX clock
    // Write (rx_pll_select, val);
    // // val=0 rx_clk from PLL0
    // // val=1 rx_clk from PLL1
    //
    //
    SerdesPrint(phy_serdes, "pa_comment: Configure CKRST_CTRL_PLL_SELECT_CONTROL\n");
    if (tx_pll_id == 0) {
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | CKRST_CTRL_PLL_SELECT_CONTROL), 0x0000, ~(0x0001)); // tx_pll_select = 0
    } else {
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | CKRST_CTRL_PLL_SELECT_CONTROL), 0x0001, ~(0x0001)); // tx_pll_select = 1
    }
    if (rx_pll_id == 0) {
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | CKRST_CTRL_PLL_SELECT_CONTROL), 0x0000, ~(0x0002)); // rx_pll_select = 0
    } else {
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | CKRST_CTRL_PLL_SELECT_CONTROL), 0x0002, ~(0x0002)); // rx_pll_select = 1
    }

    // Please see this link to determine how to program PLL depending the data rate configuration.
    // http://ingbu.broadcom.com/hsip/serdes/XPON/Design%20Documentation/ONU10G/Digital%20Design/ONU10G%20PLL%20programing%20for%20data%20rate.docx
    //

    // //------------------------------------------------------------------------------------------------------
    // //------------------------------------------------------------------------------------------------------
    // // program vcodiv4 =1
    // pll_ctrl[78]=1                     (Force mode)    // ams_pll_force_kvh_bw
    // pll_ctrl[23:22]=10             (Force Mode div 4)  //ams_pll_vco_div4
    //
    // // program vcodiv4 =0
    // pll_ctrl[78]=0                     (Force mode)    // ams_pll_force_kvh_bw
    //
    // // program PLL integer mode
    // pll_ctrl[143]=0                   (Fracn_sel)      //ams_pll_fracn_sel
    // pll_mode = pll_mode_val
    //
    // // Program PLL FracN mode
    // pll_ctrl[143]=1                   (Fracn_sel)      //ams_pll_fracn_sel
    // pll_ctrl[141:132] = fracn_ndiv_int        //ams_pll_fracn_ndiv_int
    // pll_ctrl[129:112] = fracn_ndiv                                    //ams_pll_fracn_div_h,ams_pll_fracn_div_l
    // pll_ctrl[142]=1                   (Fracn_ditheren) //ams_pll_ditheren
    // pll_ctrl[131]=0                   (Fracn_bypass)      //ams_pll_fracn_bypass
    // pll_ctrl[130]=0                   (Fracn_divrange aka MMD range 8/9) //ams_pll_fracn_divrange
    //
    // //------------------------------------------------------------------------------------------------------
    // //------------------------------------------------------------------------------------------------------
    //
    // //****************//
    // //** refclk 50Mhz **//
    // //****************//
    // // 10G EPON Symmetric, TX rate = 10.3125G, RX rate = 10.3125G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 206.25
    // fracn_ndiv_int[9:0] = 0xCE
    // fracn_div[17:0] = 0x10000
    // vcodiv4 = 0
    // // RX PLL, fracN mode
    // N div = 206.25
    // fracn_ndiv_int[9:0] = 0xCE
    // fracn_div[17:0] = 0x10000
    // vcodiv4 = 0
    //
    // // 10G EPON Assymmetric, TX rate = 1.25G, RX rate = 10.3125G
    // // 2 PLL
    // // TX PLL, Integer mode
    // N div = 50
    // Pll_mode = 0x5
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 206.25
    // fracn_ndiv_int[9:0] = 0xCE
    // fracn_div[17:0] = 0x10000
    // vcodiv4 = 0
    //
    // // 2G EPON, TX rate = 1.25G, RX rate = 2.5G
    // // common PLL
    // // TX PLL, Integer mode
    // N div = 50
    // Pll_mode = 0x5
    // vcodiv4 = 1
    // // RX PLL, Integer mode
    // N div = 50
    // Pll_mode = 0x5
    // vcodiv4 = 1
    //
    // // 1G EPON, TX rate = 1.25G, RX rate = 1.25G
    // // common PLL
    // // TX PLL, Integer mode
    // N div = 50
    // Pll_mode = 0x5
    // vcodiv4 = 1
    // // RX PLL, Integer mode
    // N div = 50
    // Pll_mode = 0x5
    // vcodiv4 = 1
    //
    // // GPON, TX rate = 1.244G, RX rate = 2.488G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 1
    //
    // // XGPON/NGPON2(10/2.5), TX rate = 2.488G, RX rate = 9.95328G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 0
    //
    // // 10G active Ethernet, TX rate = 10.3125G, RX rate = 10.3125G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 206.25
    // fracn_ndiv_int[9:0] = 0xCE
    // fracn_div[17:0] = 0x10000
    // vcodiv4 = 0
    // // RX PLL, fracN mode
    // N div = 206.25
    // fracn_ndiv_int[9:0] = 0xCE
    // fracn_div[17:0] = 0x10000
    // vcodiv4 = 0
    //
    // // 2G active Ethernet, TX rate = 3.125G, RX rate = 3.125G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 250
    // fracn_ndiv_int[9:0] = 0xFA
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 250
    // fracn_ndiv_int[9:0] = 0xFA
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 1
    //
    // // 1G active Ethernet, TX rate = 1.25G, RX rate = 1.25G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 50
    // Pll_mode = 0x5
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 50
    // Pll_mode = 0x5
    // vcodiv4 = 1
    //
    // // NGPON2 (10/10) , TX rate = 9.9532G, RX rate = 9.9532G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 0
    // // RX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 0
    //
    // // NGPON2 (10/10, 8b,10b) , TX rate = 9.9532G, RX rate = 12.44G
    // // 2 PLL
    // // TX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 0
    // // RX PLL, fracN mode
    // N div = 248.832
    // fracn_ndiv_int[9:0] = 0xF8
    // fracn_div[17:0] = 0x353F7
    // vcodiv4 = 0
    //
    // // NGPON2 (10/2.5,8b,10b), TX rate = 2.488G, RX rate = 12.44G
    // // 2 PLL
    // // TX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 248.832
    // fracn_ndiv_int[9:0] = 0xF8
    // fracn_div[17:0] = 0x353F7
    // vcodiv4 = 0
    //
    // // NGPON2 (2.5/2.5), TX rate = 2.488G, RX rate = 3.1104G
    // // 2 PLL
    // // TX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 248.832
    // fracn_ndiv_int[9:0] = 0xF8
    // fracn_div[17:0] = 0x353F7
    // vcodiv4 = 1
    //
    // // NGPON2 (2.5/2.5, 8b,10b), TX rate = 2.488G, RX rate = 2.488G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 199.066
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 248.832
    // fracn_ndiv_int[9:0] = 0xC7
    // fracn_div[17:0] = 0x432C
    // vcodiv4 = 1
    //
    // //******************//
    // //** refclk 155.52Mhz **//
    // //******************//
    // // NGPON2 (10/10) , TX rate = 9.9532G, RX rate = 9.9532G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 64
    // fracn_ndiv_int[9:0] = 0x40
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    //
    // // RX PLL, fracN mode
    // N div = 64
    // fracn_ndiv_int[9:0] = 0x40
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    //
    // // NGPON2 (10/10, 8b,10b) , TX rate = 9.9532G, RX rate = 12.44G
    // // 2 PLL
    // // TX PLL, fracN mode
    // N div = 64
    // fracn_ndiv_int[9:0] = 0x40
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    // // RX PLL, fracN mode
    // N div = 80
    // fracn_ndiv_int[9:0] = 0x50
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    //
    // // NGPON2 (10/2.5,8b,10b), TX rate = 2.488G, RX rate = 12.44G
    // // 2 PLL
    // // TX PLL, fracN mode
    // N div = 64
    // fracn_ndiv_int[9:0] = 0x40
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 80
    // fracn_ndiv_int[9:0] = 0x50
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    //
    // // NGPON2 (2.5/2.5), TX rate = 2.488G, RX rate = 3.1104G
    // // 2 PLL
    // // TX PLL, fracN mode
    // N div = 64
    // fracn_ndiv_int[9:0] = 0x40
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 80
    // fracn_ndiv_int[9:0] = 0x50
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 1
    //
    // // NGPON2 (2.5/2.5, 8b,10b), TX rate = 2.488G, RX rate = 2.488G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 64
    // fracn_ndiv_int[9:0] = 0x40
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 64
    // fracn_ndiv_int[9:0] = 0x40
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 1
    //
    // //*******************//
    // //** refclk 78.125Mhz **//
    // //*******************//
    // // 10G EPON Symmetric, TX rate = 10.3125G, RX rate = 10.3125G
    // // common PLL
    // // TX PLL, fracN mode
    // N div = 132
    // fracn_ndiv_int[9:0] = 0x84
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    // // RX PLL, fracN mode
    // N div = 132
    // fracn_ndiv_int[9:0] = 0x84
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    //
    // // 10G EPON Assymmetric, TX rate = 1.25G, RX rate = 10.3125G
    // // 2 PLL
    // // TX PLL, Integer mode
    // N div = 32
    // Pll_mode = 0x7
    // vcodiv4 = 1
    // // RX PLL, fracN mode
    // N div = 132
    // fracn_ndiv_int[9:0] = 0x84
    // fracn_div[17:0] = 0x0
    // vcodiv4 = 0
    //
    //
    //

    SerdesPrint(phy_serdes, "pa_comment: Configure AMS_COM_PLL_CONTROL_1/AMS_COM_PLL_CONTROL_4\n");

    // TX AMS_COM_PLL_CONTROL_1
    lp_wr_data = (((tx_pll_vco_div4 << 7) & (0x0001 << 7)) | ((tx_pll_vco_div2 << 6) & (0x0001 << 6)));
    lp_wr_mask = ((                         (0x0001 << 7)) | (                         (0x0001 << 6)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | tx_pll_id | AMS_COM_PLL_CONTROL_1), (lp_wr_data), ~(lp_wr_mask));

    // TX AMS_COM_PLL_CONTROL_4
    lp_wr_data = (((tx_pll_force_kvh_bw << 14) & (0x0001 << 14)) | ((tx_pll_2rx_bw << 8) & (0x0003 << 8)));
    lp_wr_mask = ((                              (0x0001 << 14)) | (                       (0x0003 << 8)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | tx_pll_id | AMS_COM_PLL_CONTROL_4), (lp_wr_data), ~(lp_wr_mask));

    // RX AMS_COM_PLL_CONTROL_1
    lp_wr_data = (((rx_pll_vco_div4 << 7) & (0x0001 << 7)) | ((rx_pll_vco_div2 << 6) & (0x0001 << 6)));
    lp_wr_mask = ((                         (0x0001 << 7)) | (                         (0x0001 << 6)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | rx_pll_id | AMS_COM_PLL_CONTROL_1), (lp_wr_data), ~(lp_wr_mask));

    // RX AMS_COM_PLL_CONTROL_4
    lp_wr_data = (((rx_pll_force_kvh_bw << 14) & (0x0001 << 14)) | ((rx_pll_2rx_bw << 8) & (0x0003 << 8)));
    lp_wr_mask = ((                              (0x0001 << 14)) | (                       (0x0003 << 8)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | rx_pll_id | AMS_COM_PLL_CONTROL_4), (lp_wr_data), ~(lp_wr_mask));


    // TX AMS_COM_PLL_CONTROL_7
    lp_wr_data = (((tx_pll_fracn_div & 0xffff) << 0) & (0xffff << 0));
    lp_wr_mask = (                                     (0xffff << 0));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | tx_pll_id | AMS_COM_PLL_CONTROL_7), (lp_wr_data), ~(lp_wr_mask));

    // TX AMS_COM_PLL_CONTROL_8
    lp_wr_data = (((tx_pll_fracn_sel << 15) & (0x0001 << 15)) | ((tx_pll_ditheren << 14) & (0x0001 << 14)) | ((tx_pll_fracn_ndiv << 4) & (0x03ff << 4)) | ((((tx_pll_fracn_div & 0x30000)>> 16)  << 0) & (0x0003 << 0)));
    lp_wr_mask = ((                           (0x0001 << 15)) | (                          (0x0001 << 14)) | (                           (0x03ff << 4)) | (                                              (0x0003 << 0)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | tx_pll_id | AMS_COM_PLL_CONTROL_8), (lp_wr_data), ~(lp_wr_mask));

    // RX AMS_COM_PLL_CONTROL_7
    lp_wr_data = (((rx_pll_fracn_div & 0xffff) << 0) & (0xffff << 0));
    lp_wr_mask = (                                     (0xffff << 0));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | rx_pll_id | AMS_COM_PLL_CONTROL_7), (lp_wr_data), ~(lp_wr_mask));

    // RX AMS_COM_PLL_CONTROL_8
    lp_wr_data = (((rx_pll_fracn_sel << 15) & (0x0001 << 15)) | ((rx_pll_ditheren << 14) & (0x0001 << 14)) | ((rx_pll_fracn_ndiv << 4) & (0x03ff << 4)) | ((((rx_pll_fracn_div & 0x30000) >> 16) << 0) & (0x0003 << 0)));
    lp_wr_mask = ((                           (0x0001 << 15)) | (                          (0x0001 << 14)) | (                           (0x03ff << 4)) | (                                              (0x0003 << 0)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | rx_pll_id | AMS_COM_PLL_CONTROL_8), (lp_wr_data), ~(lp_wr_mask));

    // TX PLL_CAL_COM_CTL_7
    lp_wr_data = ((tx_pll_mode << 0) & (0x000f << 0));
    lp_wr_mask = (                     (0x000f << 0));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | tx_pll_id | PLL_CAL_COM_CTL_7), (lp_wr_data), ~(lp_wr_mask));

    // RX PLL_CAL_COM_CTL_7
    lp_wr_data = ((rx_pll_mode << 0) & (0x000f << 0));
    lp_wr_mask = (                     (0x000f << 0));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | rx_pll_id | PLL_CAL_COM_CTL_7), (lp_wr_data), ~(lp_wr_mask));

    // Enable PLL's
    SerdesPrint(phy_serdes, "pa_comment: Configure CORE_PLL_COM_TOP_USER_CONTROL\n");
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | tx_pll_id | CORE_PLL_COM_TOP_USER_CONTROL), 0x2000, ~(0x6000)); // afe_s_pll_pwrdn = 0, core_dp_s_rstb = 1
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | rx_pll_id | CORE_PLL_COM_TOP_USER_CONTROL), 0x2000, ~(0x6000)); // afe_s_pll_pwrdn = 0, core_dp_s_rstb = 1
    xgae_pll_powerdown_reset(phy_dev, rx_pll_id, tx_pll_id);
    sim_wait_time_nsec = 1000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);


    //
    // # 6. For RX OS2 mode, the RX PI spacing needs to be adjusted as follows:
    // # For RX OS2 mode, move the RX data slicer PI to be in phase to RX M1 slicer PI (default for OS1 is 7h20 apart).  It should only be run once at the beginning of OS mode switch.
    //
    // write ( "rx_pi_manual_mode", 1   );    //  1b1
    // write ( "rx_pi_phase_step_cnt", 7h20   );    //  7h20
    // write ( "rx_pi_slicers_en", 4   );  //  3b100
    // write ( "rx_pi_manual_strobe", 1   );  //  1b1
    // write ( "rx_pi_manual_mode", 0   );    //  1b0
    //
    //
    // # To verify, cnt_d_minus_m1 should be 0
    // # or cnt_bin_d_mreg and cnt_bin_m1_mreg should have the same value.
    //
    // # To switch from RX OS2 mode back to RX OS1 mode, RX PI spacing needs to be adjusted accordingly
    //
    // write ( "rx_pi_phase_step_cnt", 7h20   );    //  7h20
    // write ( "rx_pi_phase_step_dir", 1   );    //  1b1
    // write ( "rx_pi_slicers_en", 4   );  //  3b100
    // write ( "rx_pi_manual_strobe", 1   );  //  1b1


    //
    // #6A. CDR programming
    // Please use this link for CDR programming.
    // http://ingbu.broadcom.com/hsip/serdes/XPON/Design%20Documentation/ONU10G/System%20Design/ONU10G_defaults_progseq.xlsx
    //
    // # 7. Set the over-sample mode ( per lane ) : Optional step and is needed when OSR mode is needed to be controlled from the registers instead of the pins
    //
    // write ( "rx_osr_mode_frc_val",                          0);  // OSR Mode Value: 0 -> OSx1, 1 -> OSx2, All other values are invalid
    // write ( "rx_osr_mode_frc",                              1);  // 1'b1 will make the OSR mode to be used as indicated in register "rx_osr_mode_frc_val"
    // write ( "tx_osr_mode_frc_val",                          0);  // OSR Mode Value: 0 -> OSx1, 1 -> OSx2, All other values are invalid
    // write ( "tx_osr_mode_frc",                              1);  // 1'b1 will make the OSR mode to be used as indicated in register "tx_osr_mode_frc_val"
    //
    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_A_cdr_control_0\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_A_cdr_control_0), 0x0005, ~(0x7ff7)); // cdr_freq_en=1, cdr_integ_sat_sel=0, cdr_freq_override_en=0, cdr_phase_sat_ctrl=1

    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_A_cdr_control_2\n");
    if (serdes_speed_mode == eSPEED_10_10 || serdes_speed_mode == eSPEED_10_1) {
        //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_A_cdr_control_2), 0x0030, ~(0x1ff3)); // osx2p_pherr_gain=0, phase_err_offset_mult_2=0, pattern_sel=3
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_A_cdr_control_2), 0x00c0, ~(0x1ff3)); // osx2p_pherr_gain=0, phase_err_offset_mult_2=0, pattern_sel=3
    } else {
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_A_cdr_control_2), 0x00f0, ~(0x1ff3)); // osx2p_pherr_gain=0, phase_err_offset_mult_2=0, pattern_sel=15
    }

    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_B_dsc_sm_ctrl_7\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_B_dsc_sm_ctrl_7), 0x0000, ~(0xffff)); // cdr_bwsel_integ_acqcdr=0, cdr_bwsel_integ_norm=0, cdr_bwsel_prop_acqcdr=0, cdr_bwsel_prop_norm=0

    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_A_cdr_control_1\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_A_cdr_control_1), 0x0690, ~(0xffff)); // cdr_freq_override_val=0x690

    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_B_dsc_sm_ctrl_8\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_B_dsc_sm_ctrl_8), 0x0010, ~(0xcfff)); // phase_err_offset=0, phase_err_offset_en=0


    // # 8. Set the lane polarity ( per lane )   // optional
    //
    // write ( "rx_pmd_dp_invert",                             1);  // Optional : needed only if RX PMD datapath is inverted
    // write ( "tx_pmd_dp_invert",                             1);  // Optional : needed only if TX PMD datapath is inverted
    //
    //
    //
    //
    //
    //
    //
    // # 9. Speed up values for pmd_rx_lock (only applicable for simulation to speed up PMD_LOCK time)
    //
    // if (SIM_SPEEDUP) {
    // // this is for simulation only. Dont program this in silicon or ATE test
    // // Speedup values - pmd_rx_lock lock time about 130us.
    // write ( "hw_tune_timeout",                              1);
    // write ( "cdr_settle_timeout",                           1);
    // write ( "acq_cdr_timeout",                              2);
    // }
    //
#if 0
    SerdesPrint(phy_serdes, "pa_comment: start skip pa trans\n");
    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_B_dsc_sm_ctrl_4 SIM_SPEEDUP start:\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_B_dsc_sm_ctrl_4), 0x0422, ~(0x7fff)); // PMD RX LOCK speedup
    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_B_dsc_sm_ctrl_4 SIM_SPEEDUP done.\n");
    SerdesPrint(phy_serdes, "pa_comment: end skip pa trans\n");
#endif

    // # 10. LBE set up. ( 4bit or 1bit LBE).
    // // 4 bit LBE
    // Write (tx_lbe4_0, 1b1);    // select LBE 4bit mode
    // Write (ams_tx_txclk4_en, 1b1);   // enable clock4
    //
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_lbe_control_0), 0x0080, ~(0x0080)); // tx_lbe4_0=1
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_TX_TX_CONTROL_1), 0x0001, ~(0x0001)); // ams_tx_txclk4_en=1

    // // 1 bit LBE
    // Write (tx_lbe4_0, 1b0);    // select LBE 1 bit mode
    // Write (ams_tx_txclk4_en, 1b0);   // disable clock4
    //
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_lbe_control_0), 0x0000, ~(0x0080)); // tx_lbe4_0=0
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_TX_TX_CONTROL_1), 0x0000, ~(0x0001)); // ams_tx_txclk4_en=0
    //
    //
    //
    // MODE  TXCLK4   TXCLK16  LBE bit  TXDATA Tune resolution  LBE fine tune resolution   LBE coarse tune resolution
    // 10G EPON  Symmetric  2.57G(NA)   644.5M   NA 196.9ps  1.55ns   1.55ns
    // 10G EPON  Asymmetric 625M  156.25M  NA 400ps 1.6ns 1.6ns
    // 2G EPON        625M  156.25M  NA 400ps 1.6ns 1.6ns
    // 1G EPON  625M  156.25M  NA 400ps 1.6ns 1.6ns
    // GPON  625M  156.25M  4  400ps 1.6ns 1.6ns
    // XGPON1 /NGPON2(10/2.5)  625M  156.25M  4  400ps 1.6ns 1.6ns
    // 10G Active Ethernet        NA
    // 1G Active Ethernet         NA
    // 2G Active Ethernet         NA
    // NGPON2 (10/10)    2.488G (NA) 622M  1  100.4ps  1.607ns  1.607ns
    // NGPON2 (10/10, 8b/10b)  2.488G (NA) 622M  1  100.4ps  1.607ns  1.607ns
    // NGPON2 (10/2.5, 8b/10b) 622M  155.5M   4  402ps 1.607ns  6.43ns
    // NGPON2 (2.5/2.5)     622M     155.5M   4  402ps 1.607ns  6.43ns
    // NGPON2 (2.5/2.5, 8b/10b)   622M     155.5M   4  402ps 1.607ns  6.43ns
    //
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_lbe_control_0), 0x0060, ~(0x0090)); // move laser delay 3 * UI

    //
    //
    //
    // # 11 pon_mac_clk and sync_e_clk
    // Write (ams_tx_tx_sync_e_ctrl, sync_e_val);
    // Write (ams_tx_tx_pon_mac_ctrl, pon_mac_val);
    //
    // Mode  TX VCO   Mac_div  Pon_mac_clk Pon_mac_val Sync_e_div  Sync_e_clk  Sync_e_val
    // 10G/10G EPON   10.3125G       82.5  125M  3b111 150   68.75M   3b111
    // 10G/1G  EPON   2.5G  20 125M  101   20 125M  100
    // 1G/1G EPON  2.5G  20 125M  101   20 125M  100
    // 2G/1G EPON  2.5G  20 125M  101   20 125M  100
    // 10G AE   10.3125G 82.5  125M  111   150   68.75M   111
    // 1G AE 2.5G  20 125M  101   20 125M  100
    // 2.5G AE  3.125G   10 312.5M   011   25 125M  101
    // 2.488G XGPON   2.488G   8  311.04M  010   16 155.52M  011
    // 9.953G XGPON   9.953G   32 311.04M  110   64 155.52M  110
    // 1.244G GPON 2.488G   16 155.52M  100   16 155.52M  011
    //
    //
    //////////////////////////
    //****************************//
    // mode            VCO         mac_div           mac_ctrl  sync_e_div           syn_e_ctrl
    // 10G/10G EPON    10.3125G    82.5    (125M)    111       150     (68.75M)     111
    // 10G/1G  EPON    2.5G        20      (125M)    101       20      (125M)       100
    // 1G/1G   EPON    2.5G        20      (125M)    101       20      (125M)       100
    // 2G/1G   EPON    2.5G        20      (125M)    101       20      (125M)       100
    // 10G     AE      10.3125G    82.5    (125M)    111       150     (68.75M)     111
    // 1G      AE      2.5G        20      (125M)    101       20      (125M)       100
    // 2.5G    AE      3.125G      10      (312.5M)  011       25      (125M)       101
    // 2.488G  XGPON   2.488G      8       (311.04M) 010       16      (155.52M)    011
    // 9.953G  XGPON   9.953G      32      (311.04M) 110       64      (155.52M)    110
    // 1.244G  GPON    2.488G      16      (155.52M) 100       16      (155.52M)    011
    SerdesPrint(phy_serdes, "pa_comment: Configure AMS_TX_TX_CONTROL_1\n");

    // TX AMS_TX_TX_CONTROL_1
    lp_wr_data = (((tx_pon_mac_ctrl << 4) & (0x0007 << 4)) | ((tx_sync_e_ctrl << 1) & (0x0007 << 1)));
    lp_wr_mask = ((                         (0x0007 << 4)) | (                        (0x0007 << 1)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_TX_TX_CONTROL_1), (lp_wr_data), ~(lp_wr_mask));

    // TX AMS_RX_RX_CONTROL_2
    lp_wr_data = (((rx_pon_mac_ctrl << 0) & (0x0007 << 0)));
    lp_wr_mask = ((                         (0x0007 << 0)));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_RX_RX_CONTROL_2), (lp_wr_data), ~(lp_wr_mask));


    // # 12. Set Tx FIR and amplitude settings ( per lane) (Optional)
    //
    // # 13. De-assert core_dp_s_rstb bit which will start the PLL calibration.
    // #     Once PLL calibration completes then "pll_lock" will be asserted. (Common Registers)
    //
    // write ( "core_dp_s_rstb",  1);  // This will start the PLL calibration
    //
    // wait_for( "pll_lock", 1);  // check pll lock state and wait till it is locked
    //
    SerdesPrint(phy_serdes, "pa_comment: Polling for Tx PLL lock ..\n");
    rc = poll_pll_lock(phy_serdes, tx_pll_sel, 10000);
    if (rc && !force)
        goto ret;

    SerdesPrint(phy_serdes, "pa_comment: Polling for Rx PLL lock ..\n");
    rc = poll_pll_lock(phy_serdes, rx_pll_sel, 10000);
    if (rc && !force)
        goto ret;

    SerdesPrint(phy_serdes, "pa_comment: Polling for Tx PLL lock from WAN ..\n");
    rc = poll_pll_lock_wan(phy_serdes, tx_pll_sel, 10000);
    if (rc && !force)
        goto ret;

    SerdesPrint(phy_serdes, "pa_comment: Polling for Rx PLL lock from WAN ..\n");
    rc = poll_pll_lock_wan(phy_serdes, rx_pll_sel, 10000);
    if (rc && !force)
        goto ret;

    // # 14. De-assert ln_dp_s_rstb bit (per lane)
    //
    // write ( "ln_dp_s_rstb",                                  1);
    //
    // SerdesPrint(phy_serdes, "pa_comment: Configure DIG_COM_TOP_USER_CONTROL_0\n");  // Wrong register bug
    // wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DIG_COM_TOP_USER_CONTROL_0), 0x2000, ~(0x2000)); // core_dp_s_rstb = 1
    sim_wait_time_nsec = 20000000;
    //SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    SerdesPrint(phy_serdes, "pa_comment: Configure CKRST_CTRL_LANE_CLK_RESET_N_POWERDOWN_CONTROL\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | CKRST_CTRL_LANE_CLK_RESET_N_POWERDOWN_CONTROL), 0x0002, ~(0x0002));

    SerdesPrint(phy_serdes, "pa_comment: Configure AMS_TX_TX_CONTROL_0\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_TX_TX_CONTROL_0), 0x0000, ~(0x00c0));
    sim_wait_time_nsec = 1000;
    // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    // # 15. Check for pmd_rx_lock state
    //
    // wait_for( "dsc_state",                                 DONE);  // wait/poll till it DSC state transitions to the DONE state
    // wait_for( "pmd_rx_lock",                                  1);  // check pmd_rx_lock and wait till it is locked. Once locked then data-integrity should be good
    //
    wait_cnt_max = 12;
    wait_cnt = 0;
    SerdesPrint(phy_serdes, "pa_comment: Fixed wait, waiting %dus", wait_cnt_max);
    while (wait_cnt < wait_cnt_max) {
        // SerdesPrint(phy_serdes, "waiting 1us, wait_cnt:%d", wait_cnt);
        sim_wait_time_nsec = 1000;
        // SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
        serdes_wait_nsec(sim_wait_time_nsec);
        wait_cnt = wait_cnt + 1;
    }

    //
    //
    // #---------------------------
    // # Digital Loopbacks
    // #---------------------------
    //
    //   write ( "rx_pi_manual_mode,         0);          // Optional write: if it was set to 1 before then for dig_lpbk, it should be set to 1'b0 irrespective of the OSR modes
    //   write ( "dig_lpbk_en,               1);          // Digital Loopback enable : 0 = disabled, 1 = enabled
    //
    // wait  (>= 50us                       );             // Wait for rclk and tclk phase lock before expecing good data from digital loopback FIFO.
    //
    // #---------------------------
    // # Remote Loopbacks
    // #---------------------------
    //
    // if (SIM_SPEEDUP) {
    //      write ( "tx_pi_rmt_lpbk_bypass_flt",      1);  // PD filter bypass in remote loopback
    //   } else {
    //      write ( "tx_pi_ext_phase_bwsel_integ", 3'd5);  // 0:5 : PD path bandwidth control : Higher value is faster lock.
    //                                                     // Only applicable when any PD paths and tx_pi_ext_ctrl_en is enabled
    // }
    //
    // write ( "tx_pi_loop_timing_src_sel", 1);            // RX phase_sum_val logic enable
    //
    // write ( "tx_pi_en",                  1);            // TX_PI enable :                       0 = disabled, 1 = enabled
    // write ( "tx_pi_jitter_filter_en",    1);            // Jitter filter enable to lock freq:   0 = disabled, 1 = enabled
    //
    // wait  (>= 25us                    );                // Wait for tclk to lock to CDR,
    //                                                     // # TODO: confirm with Magesh if tx_pi_second_order_loop_en is needed for rmt_lpbk
    //
    // write ( "tx_pi_ext_ctrl_en",         1);            // PD path enable :         0 = disabled, 1 = enabled
    //
    // write ( "rmt_lpbk_en",               1);            // Remote Loopback enable : 0 = disabled, 1 = enabled
    //
    // wait  (>= 50us                     );               // Wait for rclk and tclk phase lock before expecing good data from rmt loopback FIFO.
    //                                                     // Might require longer wait time for smaller values of tx_pi_ext_phase_bwsel_integ
    //
    //if ($test$plusargs("cfg_serdes_digloopback")) {
    //   wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TLB_RX_dig_lpbk_config), 0x0001, 0xfffe);
    //   #50us;
    //}

    //
    // #---------------------------
    // # PRBS
    // #---------------------------
    //
    // # PRBS Checker
    //
    // write ( "prbs_chk_mode_sel",   0   );            // = 0:6 PRBS Polynomial          : 0 = PRBS 7, 1 -> PRBS 9, 2 -> PRBS 11, 3 -> PRBS 15, 4 -> PRBS 23, 5 -> PRBS 31, 6 -> PRBS 58
    // write ( "prbs_chk_mode",       0   );            // = 0:2 PRBS checker mode        : 0 = self-sync mode w/ hysteresis, 1 = initial seed mode w/ hysteresis, 2 = initial seed mode w/o hysteresis
    // write ( "prbs_chk_inv",        0   );            //       PRBS checker data invert : 0 = normal, 1 = invert PRBS data
    // write ( "prbs_chk_en",         1   );            //       PRBS checker enable      : 0 = disabled, 1 = enabled
    //
    // verify( "prbs_chk_lock",       1   );            //       check for PRBS lock to be 1
    // read  ( "prbs_chk_err_cnt_msb"     );            //       check for PRBS error counter, MSB part of counter should be read first which will also latch the LSB portion for reading. Cleared upon read.
    // read  ( "prbs_chk_err_cnt_lsb"     );
    //
    // # PRBS Generator
    //
    // write ( "prbs_gen_mode_sel",   0   );            // = 0:6 PRBS Polynomial      : 0 = PRBS 7, 1 -> PRBS 9, 2 -> PRBS 11, 3 -> PRBS 15, 4 -> PRBS 23, 5 -> PRBS 31, 6 -> PRBS 58
    // write ( "prbs_gen_inv",        0   );            //       PRBS gen data invert : 0 = normal, 1 = invert PRBS data
    // write ( "prbs_gen_en",         1   );            //       PRBS gen enable      : 0 = disabled, 1 = enabled
    //
    // write ( "prbs_gen_err_ins",   0->1 );            //       PRBS gen error insert : write 0->1 on this register to insert 1 bit error for every rising edge on this register
    //
    // #---------------------------
    // # Fixed Pattern Generator
    // #---------------------------
    //
    // write ( "patt_gen_seq_14:0[15:0]",  'h5555);     // = 0:'hffff Fixed pattern sequence : 240 bit sequence is {patt_gen_seq_14, .. patt_gen_seq_0} where MSB bit is transmitted first
    // write ( "patt_gen_start_pos",       14    );     // = 0:14 Patt gen start index : defines the start position index of the pattern in 16 bit chunks, 14 means start is bit 239, 0 means start is bit 15
    // write ( "patt_gen_stop_pos",        0     );     // = 0:14 Patt gen stop  index : defines the stop  position index of the pattern in 16 bit chunks, 14 means stop  is bit 224, 0 means stop  is bit  0
    // write ( "patt_gen_en",              1     );     //        Patt gen enable      : 0 = disabled, 1 = enabled
    //
    //
    //
    //
    // #---------------------------
    // # TLB TX misc
    // #---------------------------
    //
    // # TX PCS Interface Native Analog Format:
    //
    // write ( "tx_pcs_native_ana_frmt_en",    0:1);    // TX PCS Interface Native Analog Format Enable: 0 = disabled, 1 = enabled
    //
    // # TX Data MUX Select Priority Order:
    //
    // write ( "tx_mux_sel_order",             0:1);    // TX Data MUX Select Priority Order.
    //                                                  // 0 => TX Data Mux select order from higher to lower priority is {rmt_lpbk, patt_gen, cl72_tx, prbs_gen, tx_pcs}.
    //                                                  // 1 => TX Data Mux select order from higher to lower priority is {rmt_lpbk, prbs_gen, cl72_tx, patt_gen, tx_pcs}.
    //
    // #---------------------------
    // # PMD Datapath Invert
    // #---------------------------
    //
    // # RX PMD data invert
    //
    // write ( "rx_pmd_dp_invert",             1);      // RX PMD Datapath Invert Control : 0 = normal, 1 = inverted
    //
    // # TX PMD data invert
    //
    // write ( "tx_pmd_dp_invert",             1);      // TX PMD Datapath Invert Control : 0 = normal, 1 = inverted
    //
    //
    //
    //
    // #---------------------------
    // # DC Offset Write Operation:
    // #---------------------------
    //
    //
    // write ( "dc_offs",  dc_offset_val[6:0]);   // dc_offset written value to AFE
    //
    //
    // #---------------------------
    // # Laser Burst Delay Programming
    // #---------------------------
    // // LBE delay fine tuning
    //           write (  "tx_lbe_delay_ctrl_0", lbe_dly_val);  // lbe_dly_val = 2b00 to 2b11
    // // Data delay fine tuning
    //           write (  "tx_data_delay_ctrl_0", data_dly_val);   // data_dly_val = 5b00000 to 5b10000
    //
    //
    //
    //
    //
    //
    //
    //
    // #--------------------------------------
    // # Loop Timing/ Host tracking Programming
    // #--------------------------------------
    // // select rx/tx ratio
    // Write (rx_tx_rate_ratio, val);
    // Val: RX to TX rate ratio
    // 000: 1 to 1 001: 1.25 to 1 010: 2 to 1
    // 011: 4 to 1 100: 5 to 1 101: 8.25 to 1
    //
    // // Select rx_phase_sum_val from the CDR as the loop timing source for the TX PI
    //         wr_field( i, "tx_pi_loop_timing_src_sel", 1'b1);
    // // Enables the IIR filter for the phase_sum_val logic from CDR
    //         wr_field(i, "tx_pi_jitter_filter_en", 1'b1);
    // // Enable Transmit phase interpolator
    //         wr_field(i, "tx_pi_en", 1'b1);
    // // Wait
    //        wait  (>= 25us                    );                // Wait for tclk to lock to CDR,
    //
    //
    //
    //if ($test$plusargs("non_looptimed")) {
    //   wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_pi_control_0), 0x2000, ~(0x7fff));
    //} else {
    /*
    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_F_ONU10G_looptiming_ctrl_0\n");
    lp_wr_data = ((rx_tx_rate_ratio << 0) & (0x0007 << 0));
    lp_wr_mask = (                          (0x0007 << 0));
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_F_ONU10G_looptiming_ctrl_0), (lp_wr_data), ~(lp_wr_mask));

    SerdesPrint(phy_serdes, "pa_comment: Configure TLB_TX_tx_pi_loop_timing_config\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TLB_TX_tx_pi_loop_timing_config), 0x0001, ~(0x0001)); // tx_pi_loop_timing_src_sel
    */

    SerdesPrint(phy_serdes, "pa_comment: Configure TX_PI_LBE_tx_pi_control_0\n");
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_pi_control_0), 0x2003, ~(0x7fff)); // tx_pi_jitter_filter_en=1, tx_pi_en=1
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_pi_control_0), 0x2000, ~(0x7fff)); // tx_pi_jitter_filter_en=1, tx_pi_en=1
    //}
    sim_wait_time_nsec = 25000;
    //SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    //SerdesPrint(phy_serdes, "pa_comment: Read SERDES MICRO_A_temperature_status\n");
    set_mask_read_data(0x00000000);
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MICRO_A_temperature_status));
    SerdesPrint(phy_serdes, "pa_comment: Read SERDES MICRO_A_temperature_status rd_data:0x%04x\n", rd_data);
    clr_mask_read_data();

    //SerdesPrint(phy_serdes, "pa_comment: Read SERDES PLL_CAL_COM_CTL_STATUS_0 - Rescal\n");
    set_mask_read_data(0x00000000);
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | PLL_CAL_COM_CTL_STATUS_0));
    SerdesPrint(phy_serdes, "pa_comment: Read SERDES PLL_CAL_COM_CTL_STATUS_0 - Rescal rd_data:0x%04x\n", rd_data);
    clr_mask_read_data();


    SerdesPrint(phy_serdes, "pa_comment: Done with Serdes Init\n");
    SerdesPrint(phy_serdes, "pa_comment: END wan_top_ral_sequence wan_top_misc INIT_SERDES\n");




    // Configure AMS PLL Control to Disable PTEST output
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Disable PTEST output.\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_COM_PLL_CONTROL_3), 0x0000, 0x0fff);
    sim_wait_time_nsec = 10000;
    //SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);


    // Read AMS PLL status for ATE test
    set_mask_read_data(0x00000000);
    //SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Reading AMS PLL status.\n");
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_COM_PLL_STATUS));
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: AMS PLL status = 0x%04x\n", rd_data);
    clr_mask_read_data();


    // Read PLL CAL COM status for ATE test
    set_mask_read_data(0x00000000);
    //SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Reading PLL CAL COM status.\n");
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | PLL_CAL_COM_CTL_STATUS_0));
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: PLL CAL COM status = 0x%04x\n", rd_data);
    clr_mask_read_data();

    // Read AMS_TX_TX_STATUS status for ATE test
    set_mask_read_data(0x00000000);
    //SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Reading AMS_TX_TX_STATUS status.\n");
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | AMS_TX_TX_STATUS));
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: AMS_TX_TX_STATUS status = 0x%04x\n", rd_data);
    clr_mask_read_data();


    // Enable MICRO for ATE test
    SerdesPrint(phy_serdes, "pa_comment: De-assert MICRO reset.\n");
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MICRO_A_command2), 0x0800, 0xf7ff); // Use inverted clock - DEFAULT
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MICRO_A_command2), 0x0000, 0xf7ff); // Use non-inverted clock
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MICRO_A_command4), 0x0001, 0xfffc);
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MICRO_A_command4), 0x0003, 0xfffc);
    sim_wait_time_nsec = 10000;
    //SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    // Read MICRO_A_temperature_status for ATE test
    set_mask_read_data(0x00000000);
    //SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Reading MICRO_A_temperature_status.\n");
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | MICRO_A_temperature_status));
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: MICRO_A_temperature_status = 0x%04x\n", rd_data);
    clr_mask_read_data();

    // Read RESCAL for ATE test
    //SerdesPrint(phy_serdes, "pa_comment: Read SERDES PLL_CAL_COM_CTL_STATUS_0 - Rescal\n");
    set_mask_read_data(0x00000000);
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | PLL_CAL_COM_CTL_STATUS_0));
    SerdesPrint(phy_serdes, "pa_comment: Read SERDES PLL_CAL_COM_CTL_STATUS_0 - Rescal rd_data:0x%04x\n", rd_data);
    clr_mask_read_data();

    set_mask_read_data(0x00000000);
    REG_READ_32(&WAN_TOP->WAN_TOP_RESCAL_CFG, rd_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_RESCAL_STATUS_0, rd_data);
    clr_mask_read_data();



    // 1) Speed up values for pmd_rx_lock
    // 2) Speed up simulation for TX disable. This needs to be set after dp resets are de-asserted
    //    otherwise command is not committed and transmit PINS are held in HiZ
    // 3) Ignore signal detect indication in DSC sm and assume it to be 1
#if 0
    SerdesPrint(phy_serdes, "pa_comment: start skip pa trans\n");

    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Speed up values for pmd_rx_lock only for simulation. Please remove for silicon test.\n");
    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_B_dsc_sm_ctrl_4 SIM_SPEEDUP start:\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | DSC_B_dsc_sm_ctrl_4), 0x0422, 0x8000);
    SerdesPrint(phy_serdes, "pa_comment: Configure DSC_B_dsc_sm_ctrl_4 SIM_SPEEDUP done.\n");

    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Speed up simulation for TX disable.\n");
    SerdesPrint(phy_serdes, "pa_comment: Configure TX_FED_misc_control1 SIM_SPEEDUP start:\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | TX_FED_misc_control1),  0x0000, 0xff03);
    SerdesPrint(phy_serdes, "pa_comment: Configure TX_FED_misc_control1 SIM_SPEEDUP done.\n");

    SerdesPrint(phy_serdes, "pa_comment: end skip pa trans\n");
#endif

    //SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Ignore signal detect indication in DSC SM.\n");
    //wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | DSC_B_dsc_sm_ctrl_0), 0x0200, 0xfdff);
    //SerdesPrint(phy_serdes, "pa_comment: Configure DSC_B_dsc_sm_ctrl_0\n");
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Configure signal detect indication in DSC SM.\n");
    wr_xgae_serdes_reg(SERDES_0, DEVID_1 | serdes_addr | DSC_B_dsc_sm_ctrl_0, 0x0008, 0);
    wr_xgae_serdes_reg(SERDES_0, DEVID_1 | serdes_addr | SIGDET_SIGDET_CTRL, 0x0023, 0);
    wr_xgae_serdes_reg(SERDES_0, DEVID_1 | serdes_addr | SIGDET_SIGDET_CTRL_1, 0xa00a, 0);

#ifdef SERDES_MICROCODE
    // Load SerDes Microcode and run for tuning
    SerdesPrint(phy_serdes, "pa_comment: SERDES_MICROCODE\n");
    error_cnt = error_cnt + serdes_microcode (serdes_speed_mode, serdes_rate_mode);
#endif // SERDES_MICROCODE

#ifdef SERDES_DIG_LOOPBACK
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Enable DIGITAL LOOPBACK.\n");
    SerdesPrint(phy_serdes, "pa_comment: Configure TLB_RX_DIG_LPBK_CONFIG\n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | TLB_RX_DIG_LPBK_CONFIG), 0x0001, 0xfffe);
#endif // SERDES_DIG_LOOPBACK

    // Enable PRBS generator and checker. PRBS checking will start after DSC lock
    if (prbs_type == 7)       {prbs_mode = 0; }
    else if (prbs_type == 9)  {prbs_mode = 1; }
    else if (prbs_type == 11) {prbs_mode = 2; }
    else if (prbs_type == 15) {prbs_mode = 3; }
    else if (prbs_type == 23) {prbs_mode = 4; }
    else if (prbs_type == 31) {prbs_mode = 5; }
    else if (prbs_type == 58) {prbs_mode = 6; }
    else                      {prbs_mode = 100; }
    if (prbs_mode < 7) {
        SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Enable PRBS generator, PRBS %d (mode %d).\n", prbs_type, prbs_mode);
        //SerdesPrint(phy_serdes, "pa_comment: Configure TLB_TX_prbs_gen_config\n");
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | TLB_TX_prbs_gen_config), (0x0001 | (prbs_mode << 1)), 0xfe00);

        SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Enable PRBS checker clock.\n");
        //SerdesPrint(phy_serdes, "pa_comment: Configure TLB_RX_prbs_chk_config\n");
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | TLB_RX_prbs_chk_config), 0x0800, 0xf7ff);

        SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Configure PRBS checker, PRBS %d (mode %d).\n", prbs_type, prbs_mode);
        //SerdesPrint(phy_serdes, "pa_comment: Configure TLB_RX_prbs_chk_config\n");
        wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | TLB_RX_prbs_chk_config), (0x00a1 | (prbs_mode << 1)), 0xfe00);
    }


    // Wait for Rx DSC Lock
#ifdef LOCK_POLL_ENABLE

    //SerdesPrint(phy_serdes, "pa_comment: serdes_init LOCK_POLL_ENABLE\n");
    serdes_wait_nsec(10000);

    rd_data = 0;
    itr = 0;
    set_mask_read_data(0x00000000);

    if (phy_serdes->serdes_status > SERDES_NOT_INITED) {
        while (1) {
            //SerdesPrint(phy_serdes, "pa_comment: serdes_init: SGB_SERDES_INST: Polling for Rx DSC Lock .. %0d/10\n", itr);
            //SerdesPrint(phy_serdes, "pa_comment: Read DSC_B_dsc_sm_status_dsc_lock\n");
            if (!force && xgae_los_asserted(phy_dev))
                goto ret;

            rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | DSC_B_dsc_sm_status_dsc_lock));
            if ((rd_data & 0x00000001) != 0) {
                SerdesPrint(phy_serdes, "pa_comment: serdes_init: SGB_SERDES_INST: Rx DSC Lock achieved!!\n");
                break;
            } else {
                if (itr++ == 40) {
                    if (!force)
                        error_log("pa_comment: serdes_init: SGB_SERDES_INST: DSC Lock timeout!!\n");
                    error_cnt++;
                    break;
                }
                serdes_wait_nsec(1000);
                continue;
            }
        }
    }
    clr_mask_read_data();

#else // LOCK_POLL_ENABLE

    SerdesPrint(phy_serdes, "pa_comment: serdes_init LOCK_FIXED_WAIT\n");

    sim_wait_time_nsec = 20000;
    SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Checking for Rx DSC Lock .. \n");
    SerdesPrint(phy_serdes, "pa_comment: Read DSC_B_dsc_sm_status_dsc_lock\n");
    set_mask_read_data(0x00000001);
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | DSC_B_dsc_sm_status_dsc_lock));
    clr_mask_read_data();
    if ((rd_data & 0x00000001) != 0) {
        SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Rx DSC Lock achieved!!\n");
    } else {
        if (!force)
            error_log("pa_comment: SGB_SERDES_INST: DSC Lock timeout!!\n");
        error_cnt++;
    }

#endif // LOCK_POLL_ENABLE


    /*
    // Force burst_valid
    if (serdes_speed_mode) {
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Forcing burst_valid 1G .. \n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | DSC_G_BURST_SM_CTL_0), 0x1c14, 0x0000);
    } else {
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES: Forcing burst_valid 10G .. \n");
    wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | DSC_G_BURST_SM_CTL_0), 0x1814, 0x0000);
    }
     */


    // Check for Rx PMD Lock
#ifdef LOCK_POLL_ENABLE

    //SerdesPrint(phy_serdes, "pa_comment: serdes_init LOCK_POLL_ENABLE\n");
    serdes_wait_nsec(1000);

    rd_data = 0;
    itr = 0;
    set_mask_read_data(0x00000000);
    if (phy_serdes->serdes_status > SERDES_NOT_INITED) {
        while (1) {
            //SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Polling for Rx PMD lock .. %0d/10\n", itr);
            //SerdesPrint(phy_serdes, "pa_comment: Read TLB_RX_pmd_rx_lock_status\n");
            if (!force && xgae_los_asserted(phy_dev))
                goto ret;

            rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | TLB_RX_pmd_rx_lock_status));
            if ((rd_data & 0x00000001) != 0) {
                SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Rx PMD Lock achieved!!\n");
                //SerdesPrint(phy_serdes, "\n");
                break;
            } else {
                if (itr++ == 40) {
                    if (!force)
                        error_log("serdes_init: SGB_SERDES_INST: Rx PMD Lock timeout!!\n");
                    break;
                }
                serdes_wait_nsec(1000);
                continue;
            }
        }
    }
    clr_mask_read_data();

#else // LOCK_POLL_ENABLE

    SerdesPrint(phy_serdes, "pa_comment: serdes_init LOCK_FIXED_WAIT\n");

    sim_wait_time_nsec = 10000;
    SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);

    set_mask_read_data(0x00000001);
    //SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Checking for Rx PMD lock.\n");
    S//erdesPrint("pa_comment: Read TLB_RX_pmd_rx_lock_status\n");
    set_mask_read_data(0x00000001);
    rd_data = rd_xgae_serdes_reg(SERDES_0, (DEVID_1 | serdes_addr | TLB_RX_pmd_rx_lock_status));
    clr_mask_read_data();
    if ((rd_data & 0x00000001) != 0) {
        SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Rx PMD Lock achieved!!\n");
        SerdesPrint(phy_serdes, "\n");
    } else {
        if (!force)
            error_log("pa_comment: SGB_SERDES_INST: Rx PMD Lock timeout!!\n");
    }
    clr_mask_read_data();

#endif // LOCK_POLL_ENABLE

    //reg_model.wan_top_reg_blk.wan_top_ae_gearbox_control_0_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000003f) <<  4)); // cr_wan_top_ae_gearbox_tx_fifo_offset.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  2)); // cr_wan_top_ae_gearbox_tx_fifo_offset_ld.set(0);
    wr_data = (wr_data | (((serdes_rate_mode == eFULL_RATE) & 0x00000001) <<  1)); // cr_wan_top_ae_gearbox_full_rate_serdes_mode.set(0);
    if (serdes_speed_mode == eSPEED_10_10) {
        SerdesPrint(phy_serdes, "pa_comment: 0, AE_GEARBOX_CONTROL_0, _0, first time written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  0)); // cr_wan_top_ae_gearbox_width_mode.set(0);
        SerdesPrint(phy_serdes, "pa_comment: 0, AE_GEARBOX_CONTROL_0, _0, first time written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_wan_top_ae_gearbox_width_mode.set(0);
    }
    REG_WRITE_32(&WAN_TOP->WAN_TOP_AE_GEARBOX_CONTROL_0, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_AE_GEARBOX_CONTROL_0, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            //SerdesPrint(phy_serdes, "pa_comment: 0, AE_GEARBOX_CONTROL_0, _0 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, AE_GEARBOX_CONTROL_0, _0 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    /* Reset Gearbox before we bring out WAN_TOP */
    SerdesPrint(phy_serdes, "pa_comment: Reset Gearbox\n");
    pmc_wan_ae_reset();

    //reg_model.wan_top_reg_blk.wan_top_reset_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cfg_pcs_reset_n.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_RESET, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_RESET, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            SerdesPrint(phy_serdes, "pa_comment: 0, TOP_RESET, T written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, TOP_RESET, T ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    //reg_model.wan_top_reg_blk.wan_top_reset_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  0)); // cfg_pcs_reset_n.set(1);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_RESET, wr_data);
    if(phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_RESET, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            SerdesPrint(phy_serdes, "pa_comment: 0, TOP_RESET, T written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, TOP_RESET, T ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }
    msleep(10);

    rd_data = rd_xgae_serdes_reg(SERDES_1, DEVID_0 | LANE_BRDCST | CL49_UserB0_Control);
    rd_data &= ~CL49_fast_lock_cya;
    wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | CL49_UserB0_Control), rd_data, 0);

    //reg_model.wan_top_reg_blk.wan_top_reset_reg
    #if 0
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cfg_pcs_reset_n.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_RESET, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_RESET, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, TOP_RESET, T written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, TOP_RESET, T ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_reset_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  0)); // cfg_pcs_reset_n.set(1);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_RESET, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_RESET, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, TOP_RESET, T written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, TOP_RESET, T ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }
    #endif

    if (phy_serdes->sfp_module_type == SFP_COPPER)
        phy_serdes->sgmii_mode = XGAE_SGMII_AUTO;
    else if(phy_dev->cascade_next && !(phy_dev->cascade_next->flag & PHY_FLAG_DYNAMIC) && speed == PHY_SPEED_100)
        phy_serdes->sgmii_mode = XGAE_SGMII_FORCE;

    if (phy_serdes->sgmii_mode != XGAE_NONSGMII_MODE) {
        // Set SGMII mode for Copper SFP module no greater than 1G speed
        rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_Control1000X1));
        rd_data &= ~(SerdesDigital_FibreSgmiiModeFibre|SerdesDigital_SgmiiMasterMode);
        rd_data |= SerdesDigital_SgmiiAutoMode;
        wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_Control1000X1), rd_data, ~(0xffff));

        if (phy_serdes->sgmii_mode == XGAE_SGMII_AUTO) {
            // Enable Remote PHY Next Page AN
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_0));
            rd_data |= Digital4_RemotePhyEnable;
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_0), rd_data, 0);

            // Enable Remove PHY Next Page Decoding
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_1));
            rd_data |= Digital4_RemotePhyDecodeEnable;
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_1), rd_data, 0);

            /* Always enable Auto Negotiation for SGMII mode */
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | MII_CONTROL));
            rd_data |= MII_CONTROL_AN_ENABLE|MII_CONTROL_RESTART_AUTONEG;
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | MII_CONTROL), rd_data, ~(0xffff));
        }
        else { /* Not working in 158A0 chip */
            // Enable Remote PHY Next Page AN
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_0));
            rd_data &= ~Digital4_RemotePhyEnable;
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_0), rd_data, 0);

            // Enable Remove PHY Next Page Decoding
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_1));
            rd_data &= ~Digital4_RemotePhyDecodeEnable;
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_rp_nextPage_1), rd_data, 0);

            if (speed == PHY_SPEED_100)
                rd_data = MII_CONTROL_SPEED_100M | MII_CONTROL_DUPLEX_MODE;
            else
                rd_data = MII_CONTROL_SPEED_1000M | MII_CONTROL_DUPLEX_MODE;
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | MII_CONTROL), rd_data, ~(0xffff));
        }
            
            

            //rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_Control1000X2));
            /* Note: This is a hardware bug in 63158A0. The SerdesDigital_Control1000X2(0x8301) value will
               change once we set PHY AN-Restart. Internally, ASIC people says reading this register
               will cause writing garbage to it!!! Very careful!! This is an important register */
        rd_data = 0x82;
        wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_Control1000X2), rd_data, ~(0xffff));
        cascade_next->phy_drv->speed_set(cascade_next, phy_serdes->config_speed, PHY_DUPLEX_FULL);
    }
    else {
        /* Set Fibre Mode */
        rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_Control1000X1));
        rd_data |= SerdesDigital_FibreSgmiiModeFibre;
        wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_Control1000X1), rd_data, ~(0xffff));

        /* Disable Auto Negotiation */
        rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | MII_CONTROL));
        rd_data &= ~MII_CONTROL_AN_ENABLE;
        wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | MII_CONTROL), rd_data, ~(0xffff));
        if (serdes_speed_mode == eSPEED_10_10) {
            // PCS Programing for 10G
            //   $display("%t %m BEGIN programing PCS registers through LP for 10G INIT_PCS", $time);
            SerdesPrint(phy_serdes, "pa_comment: BEGIN Programing PCS registers for 10G \n");
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | XGXSBLK0_XGXSCTRL), 0x260f, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | XGXSBLK0_XGXSCTRL), 0x260f, 0x0000);

            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | XGXSBLK1_LANECTRL0), 0x1011, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | XGXSBLK1_LANECTRL0), 0x1011, 0x0000);

            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_misc1), 0x6015, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | SerdesDigital_misc1), 0x6015, 0x0000);

            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_parDetINDControl1), 0x5015, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_parDetINDControl1 ), 0x5015, 0x0000);

            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_parDetINDControl2), 0x0008, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_parDetINDControl2), 0x0008, 0x0000);

            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_Misc7 ), 0x0008, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_Misc7), 0x0008, 0x0000);

            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_Misc6), 0x2a00, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital5_Misc6), 0x2a00, 0x0000);


            //If  Dn scrambler is enabled
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_Misc3), 0x8188, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_Misc3), 0x8188, 0x0000);

            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_Misc4), 0x6000, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | Digital4_Misc4), 0x6000, 0x0000);

            //upstream scrambler enable
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | tx66_Control), 0x4041, ~(0xffff));
            rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | tx66_Control), 0x4041, 0x0000);
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | tx66_Control), 0x4001, ~(0xffff));

            SerdesPrint(phy_serdes, "pa_comment: END Programing PCS registers for 10G \n");

            // Reset Rx Fifo pointer
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | rx66b1_rx66b1_Control1 ), rfifo_ptr_sw_rst, ~(0xffff));
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | rx66b1_rx66b1_Control1 ), 0, ~(0xffff));

        } else if (serdes_speed_mode == eSPEED_1_1) {
            // Configure fixed speed in 1G
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | ieee0Blk_MIICntl));
            wr_data = rd_data & ~MII_CONTROL_SPEED_MASK;
            wr_data |= MII_CONTROL_SPEED_1000M;
            SerdesPrint(phy_serdes, "pa_comment: Programing PCS register ieee0Blk_MIICntrl register for 1G mode \n");
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | ieee0Blk_MIICntl), wr_data, ~(0xffff));
        }else if (serdes_speed_mode == eSPEED_100m_100m) {
            // Configure fixed speed in 100M
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | ieee0Blk_MIICntl));
            wr_data = rd_data & ~MII_CONTROL_SPEED_MASK;
            wr_data |= MII_CONTROL_SPEED_100M;
            SerdesPrint(phy_serdes, "pa_comment: Programing PCS register ieee0Blk_MIICntrl register for 100m mode \n");
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | ieee0Blk_MIICntl), wr_data, ~(0xffff));

            if(phy_serdes->serdes_status == SERDES_NOT_INITED)
                rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | ieee0Blk_MIICntl), wr_data, 0x0000);
            //Adding the programing or 100FX register for 100-FX mode

            #if 0 // 63158A0 Bug, FX_Control1 read will corrupt register
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | FX100_Control1));
            wr_data = rd_data;
            wr_data = wr_data | 0x0020; // bit[5] disable_rx_qual --> 1
            wr_data = wr_data | 0x0001; // bit[0] enable --> 1
            wr_data = wr_data & 0xffff; // manual_speed0 = 0
            SerdesPrint(phy_serdes, "pa_comment: Programing PCS register FX100_Control1 register for 100m mode \n");
            #else
            wr_data = 0x143;
            #endif
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | FX100_Control1), wr_data, ~(0xffff));
            /*
            if(phy_serdes->serdes_status == SERDES_NOT_INITED)
                rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | FX100_Control1), wr_data, 0x0000);
            */

            //Adding the programing or 100FX register for 100-FX mode

/* This is for Simulation, no needed for real code
            rd_data = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | FX100_Control3));
            wr_data = rd_data;
            wr_data = wr_data | 0x0080; // bit[7] correlator_disable --> 1
            wr_data = wr_data | 0x0001; // bit[0] fast_timers --> 1
            wr_data = wr_data & 0xffff;
            SerdesPrint(phy_serdes, "pa_comment: Programing PCS register FX100_Control3 register for 100m mode \n");
            wr_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | FX100_Control3), wr_data, ~(0xffff));
            if(phy_serdes->serdes_status == SERDES_NOT_INITED)
                rd_ver_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | FX100_Control3), wr_data, 0x0000);

            SerdesPrint(phy_serdes, "pa_comment: END Programing PCS registers for 100M mode \n");
            wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_pi_control_0), 0x2000, ~(0x7fff)); // tx_pi_jitter_filter_en=1, tx_pi_en=1
*/
            wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | DSC_A_rx_pi_control), 0x0080, ~(0xffff)); // rx_pi_step_size=1
            wr_xgae_serdes_reg(SERDES_0, (DEVID_1 | LANE_BRDCST | TX_PI_LBE_tx_pi_control_1), 0x8000, ~(0xffff)); // tx_pi_jitter_filter_en=1, tx_pi_en=1
        };
    }

    SerdesPrint(phy_serdes, "pa_comment: Done with SGB_SERDES_INST Serdes Config!!\n");
    rc = 0;

ret:
    if (phy_serdes->serdes_status == SERDES_NOT_INITED) {
        /* This a initialization pass, call again for reguarlar configuration pass */
        phy_serdes->serdes_status = SERDES_INITED;
    }

    if (rc < 0) {
        phy_serdes->serdes_status = SERDES_INITED;
    }
    else {
        phy_serdes->serdes_status = SERDES_PCS_CONFIGURED;
        msleep(50);
    }
    return rc;
}

int ethsw_xgae_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    return _ethsw_xgae_speed_set(phy_dev, speed, duplex, 1);
}

#define error_log printk

static int xgae_wan_top_init(phy_serdes_t *phy_serdes)
{
    int sim_wait_time_nsec;
    int error_cnt = 0;
    uint32_t wr_data = 0x00000000;
    uint32_t rd_data = 0x00000000;
    int serdes_speed_mode = eSPEED_10_10;

    // De-assert sgb_serdes reset
    // Serdes PMD reset SGB_PBI_SERDES0_RST_PWR_CTL [17] 	cfg_sgb_serdes0_pmd_core_dp_h_rstb
    SerdesPrint(phy_serdes, "pa_comment: SGB_SERDES_INST: Serdes PMD reset.\n");
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, 0x0000003e);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, 0x0000003e);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, 0x00000000);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, 0x0000003e);

    sim_wait_time_nsec = 10000;
    //SerdesPrint(phy_serdes, "pa_comment: SIM_WAIT %d ns\n", sim_wait_time_nsec);
    serdes_wait_nsec(sim_wait_time_nsec);


    //reg_model.wan_top_reg_blk.wan_top_misc_0_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 16)); // cr_xgwan_top_wan_misc_pmd_lane_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000001f) << 06)); // cr_xgwan_top_wan_misc_onu2g_phya.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 05)); // cr_xgwan_top_wan_misc_epon_tx_fifo_off_ld.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 04)); // cr_xgwan_top_wan_misc_mdio_fast_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 03)); // cr_xgwan_top_wan_misc_mdio_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 02)); // cr_xgwan_top_wan_misc_refout_en.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 01)); // cr_xgwan_top_wan_misc_refin_en.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 00)); // cr_xgwan_top_wan_misc_onu2g_pmd_status_sel.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_0, wr_data);
    if (phy_serdes->serdes_status == SERDES_NOT_INITED) {
        REG_READ_32(&WAN_TOP->WAN_TOP_MISC_0, rd_data);
        if ((rd_data & 0xffffffff) == wr_data) {
            SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 0 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        } else {
            error_log("pa_comment: 0, MISC, 0 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
            error_cnt++;
        }
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_1_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 16)); // cr_xgwan_top_wan_misc_pmd_core_1_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 00)); // cr_xgwan_top_wan_misc_pmd_core_0_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_1, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_1, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 1 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 1 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_2_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfgNgponRxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfgNgponTxClk.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 20)); // cr_xgwan_top_wan_misc_pmd_rx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000003) << 16)); // cr_xgwan_top_wan_misc_pmd_tx_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_pmd_tx_osr_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cr_xgwan_top_wan_misc_pmd_tx_disable.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  8)); // cr_xgwan_top_wan_misc_pmd_ln_rx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_pmd_ln_tx_h_pwrdn.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  6)); // cr_xgwan_top_wan_misc_pmd_ext_los.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  5)); // cr_xgwan_top_wan_misc_pmd_por_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  4)); // cr_xgwan_top_wan_misc_pmd_core_0_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  3)); // cr_xgwan_top_wan_misc_pmd_core_1_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  2)); // cr_xgwan_top_wan_misc_pmd_ln_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_pmd_ln_dp_h_rstb.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_pmd_rx_mode.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_2, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_2, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 2 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 2 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_3_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) << 16)); // cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) <<  8)); // cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_wan_cfg_laser_oe.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(1);
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) <<  24)); //  making ntr_pulse_width[31:24]-->0     // cfg_ntr_pulse_width.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000003) <<  8)); //  making cfg_ntr_src[9:8]-->0     // cfg_ntr_pulse_width.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000007) <<  2)); // cr_xgwan_top_wan_misc_wan_cfg_laser_mode.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_wan_cfg_laser_invert.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_wan_cfg_mem_reb.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_3, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_3, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 3 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 3 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_misc_3_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) << 16)); // cr_xgwan_top_wan_misc_wan_cfg_epon_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) << 12)); // cr_xgwan_top_wan_misc_wan_cfg_wan_debug_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000000f) <<  8)); // cr_xgwan_top_wan_misc_wan_cfg_ntr_sync_period_sel.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  7)); // cr_xgwan_top_wan_misc_wan_cfg_laser_oe.set(1);

    if (serdes_speed_mode == eSPEED_10_10) {
        wr_data = (wr_data | ((0x00000003 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(3);
    } else if (serdes_speed_mode == eSPEED_10_1) {
        wr_data = (wr_data | ((0x00000003 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(3);
    } else if (serdes_speed_mode == eSPEED_2_2) {
        wr_data = (wr_data | ((0x00000002 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(2);
    } else if (serdes_speed_mode == eSPEED_1_1) {
        wr_data = (wr_data | ((0x00000001 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(1);
    } else if (serdes_speed_mode == eSPEED_100m_100m) {
        wr_data = (wr_data | ((0x00000000 & 0x00000007) <<  4)); // cr_xgwan_top_wan_misc_wan_cfg_wan_interface_select.set(0);
    }
    wr_data = (wr_data | ((0x00000000 & 0x00000003) <<  2)); // cr_xgwan_top_wan_misc_wan_cfg_laser_mode.set(0);
    SerdesPrint(phy_serdes, "pa_comment: CONFIG_BCM963158 0, MISC, 3 written: wr_data=0x%08x\n", wr_data);

    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_wan_cfg_laser_invert.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_wan_cfg_mem_reb.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_MISC_3, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_MISC_3, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, MISC, 3 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, MISC, 3 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_serdes_pll_ctl_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 10)); // cfg_pll1_lcref_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  9)); // cfg_pll1_refout_en.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  8)); // cfg_pll1_refin_en.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  2)); // cfg_pll0_lcref_sel.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cfg_pll0_refout_en.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cfg_pll0_refin_en.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_SERDES_PLL_CTL, wr_data);
    REG_READ_32(&WAN_TOP->WAN_SERDES_PLL_CTL, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, WAN_SERDES, PLL_CTL written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, WAN_SERDES, PLL_CTL ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_serdes_pram_ctl_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 31)); // cfg_pram_go.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 26)); // cfg_pram_we.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 25)); // cfg_pram_cs.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) << 24)); // cfg_pram_ability.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x000000ff) << 16)); // cfg_pram_datain.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) <<  0)); // cfg_pram_addr.set(0);
    REG_WRITE_32(&WAN_TOP->WAN_SERDES_PRAM_CTL, wr_data);
    REG_READ_32(&WAN_TOP->WAN_SERDES_PRAM_CTL, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, WAN_SERDES, PRAM_CTL written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, WAN_SERDES, PRAM_CTL ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_pmi_lp_0_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  1)); // cr_xgwan_top_wan_misc_pmi_lp_en.set(1'b0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  0)); // cr_xgwan_top_wan_misc_pmi_lp_write.set(1'b0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_0, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_PMI_LP_0, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, PMI, LP_0 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, PMI, LP_0 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_pmi_lp_1_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0xffffffff) <<  0)); // cr_xgwan_top_wan_misc_pmi_lp_addr.set(32'd0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_1, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_PMI_LP_1, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, PMI, LP_1 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, PMI, LP_1 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_pmi_lp_2_reg
    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) << 16)); // cr_xgwan_top_wan_misc_pmi_lp_wrdata.set(16'd0);
    wr_data = (wr_data | ((0x00000000 & 0x0000ffff) <<  0)); // cr_xgwan_top_wan_misc_pmi_lp_maskdata.set(16'd0);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_2, wr_data);
    REG_READ_32(&WAN_TOP->WAN_TOP_PMI_LP_2, rd_data);
    if ((rd_data & 0xffffffff) == wr_data) {
        SerdesPrint(phy_serdes, "pa_comment: 0, PMI, LP_2 written: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
    } else {
        error_log("pa_comment: 0, PMI, LP_2 ERROR: wr_data=0x%08x rd_data=0x%08x\n", wr_data, rd_data);
        error_cnt++;
    }

    //reg_model.wan_top_reg_blk.wan_top_pmi_lp_3_reg
    REG_READ_32(&WAN_TOP->WAN_TOP_PMI_LP_3, rd_data);
    SerdesPrint(phy_serdes, "pa_comment: 0, PMI, LP_3 read: rd_data=0x%08x\n", rd_data);

    wr_data = 0x00000000;
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  7)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txlbe_ser_order.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000007) <<  4)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txlbe_ser_init_val.set(0);
    wr_data = (wr_data | ((0x00000000 & 0x00000001) <<  3)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txlbe_ser_en.set(0);
    wr_data = (wr_data | ((0x00000001 & 0x00000001) <<  2)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_txfifo_rd_legacy_mode.set(1);
    wr_data = (wr_data | ((0x00000002 & 0x00000003) <<  0)); // reg_model.wan_top_reg_blk.wan_top_osr_control_reg.cr_wan_top_wan_misc_serdes_oversample_ctrl_cfg_gpon_rx_clk.set(2);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_OSR_CONTROL, wr_data);
    {
        uint32_t val;
        REG_READ_32(&WAN_TOP->WAN_TOP_OSR_CONTROL, val);
    }

    // De-assert RESCAL reset
    REG_WRITE_32(&WAN_TOP->WAN_TOP_RESCAL_CFG, 0x00002000);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_RESCAL_CFG, 0x00008000);
    {
        uint32_t val;
        REG_READ_32(&WAN_TOP->WAN_TOP_RESCAL_CFG, val);
    }

    return error_cnt;
}

static int xgae_gpio_init(phy_dev_t *phy_dev)
{
    unsigned short tx_gpio;

    /* Configure PON GPIO */
    if (BpGetPonTxEnGpio(&tx_gpio) == BP_SUCCESS)
    {
        kerSysSetGpioDir(tx_gpio & BP_GPIO_NUM_MASK);
        kerSysSetGpioState((tx_gpio & BP_GPIO_NUM_MASK), (tx_gpio & BP_ACTIVE_LOW) ? kGpioInactive : kGpioActive);
    }

    return 0;
}

int sf2_xgae_serdes_init(phy_dev_t *phy_dev)
{
    printk("\n\n10G Active Ethernet Initialization Started\n");
    xgae_gpio_init(phy_dev);
    ethsw_xgae_speed_set(phy_dev, PHY_SPEED_10000, PHY_DUPLEX_FULL);
    printk("\n\n10G Active Ethernet Initialization Done\n");
    return 0;
}

void ethsw_xgae_serdes_link_stats(phy_dev_t *phy_dev)
{
    int val;
    int r_type_code;
    phy_serdes_t *phy_serdes = phy_dev->priv;
    int old_link = phy_dev->link;

    if (phy_serdes->serdes_status < SERDES_PCS_CONFIGURED || xgae_los_asserted(phy_dev)) {
        phy_dev->link = 0;
        goto end;
    }

    val = rd_xgae_serdes_reg(SERDES_1, (DEVID_0 | LANE_BRDCST | ieee5Blk_AnLinkParterAbi));
    if (phy_serdes->sgmii_mode != XGAE_NONSGMII_MODE && (val & ieee5_SgmiiMode)) {
        phy_dev->link = (val & ieee5_SgmiiLink) > 0;
        switch(val & ieee5_SgmiiSpeedMask) {
            case ieee5_SgmiiSpeed1G:
                phy_dev->speed = PHY_SPEED_1000;
                break;
            case ieee5_SgmiiSpeed100M:
                phy_dev->speed = PHY_SPEED_100;
                break;
            case ieee5_SgmiiSpeed10M:
                phy_dev->speed = PHY_SPEED_10;
                break;
        }
        phy_dev->duplex = (val & ieee5_SgmiiDuplex) ? PHY_DUPLEX_FULL: PHY_DUPLEX_HALF;
        goto end;
    }

    readXgaePcsReg(DEVID_0 | LANE_0 | XGXSBLK4_xgxsStatus1, &val);
    phy_dev->link = (val & XgxsStatus1_LinkStat) > 0;
    phy_dev->speed = phy_serdes->current_speed;
    phy_dev->duplex = PHY_DUPLEX_FULL;
    if (phy_serdes->current_speed == PHY_SPEED_10000 && phy_dev->link) {
        /* Wait 1MTU time then check rxsm, 9000B at 10G = 7.2us */
        serdes_wait_nsec(10000);
        readXgaePcsReg(DEVID_0 | LANE_0 | CL49_UserB0_rxsm_status, &val);
        r_type_code = val & r_type_coded_M;
        if ((val & lrxsm_state_RX_C) == 0 || r_type_code == R_TYPE_E || r_type_code == R_TYPE_BAD)
            phy_dev->link = 0;
        /* 10G speed tends to mis-link with 2.5 when time passed, re-configure here to refresh status */
        if (!old_link && phy_dev->link) {
            phy_serdes->serdes_status = SERDES_INITED;
            _ethsw_xgae_speed_set(phy_dev, phy_serdes->current_speed, phy_dev->duplex, 0);
            ethsw_xgae_serdes_link_stats(phy_dev);
        }
    }
end:
    return;
}

phy_drv_t phy_drv_sf2_xgae =
{
    .init = sf2_serdes_init,
    .phy_type = PHY_TYPE_XGAE,
    .read_status = sf2_serdes_phy_read_status,
    //.apd_get = ethsw_xgae_power_mode_get,
    //.apd_set = ethsw_xgae_power_mode_set,
    .apd_get = ethsw_serdes_power_mode_get,
    .speed_set = ethsw_serdes_cfg_speed_set,
    .caps_get = ethsw_serdes_caps_get,
    //.isolate_phy = phy_xgae_isolate_phy,
    .config_speed_get = ethsw_serdes_speed_get,

    .name = "10GAE",
};

