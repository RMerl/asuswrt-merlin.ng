/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: October 2021
 *      Author: ido.brezel@broadcom.com
 */

/*
 * MPCS driver
 */

#include <linux/delay.h>
#include <linux/io.h>
#include <shared_utils.h>
#include "phy_drv.h" /* For dt_access macros */
#include "memory_access.h"
#include "mpcs.h"

#define mpcs_err(fmt, ...) printk("ERROR: mpcs:" fmt, ##__VA_ARGS__)
#ifdef DEBUG
#define mpcs_dbg(fmt, ...) printk("DBG: mpcs:" fmt, ##__VA_ARGS__)
#else
#define mpcs_dbg(fmt, ...)
#endif

#define DEVID_0                      0x00000000
#define LANE_BRDCST                  0x00FF0000

#define WAN__RX_X4_Control0_pcs_control_0                        0x0000c450
#define WAN__Digital_MiscDigControl                              0x0000c30b
#define WAN__TX_X1_Control0_os_mode_cl49                         0x00009200
#define WAN__TX_X1_Control0_pmd_osr_mode                         0x00009202
#define WAN__TX_X1_Control0_pmd_tx_osr_mode_2                    0x0000920d
#define WAN__RX_X4_Control0_decode_control_0                     0x0000c454
#define WAN__CL72_IEEE_TX_cl72it_BASE_R_PMD_control_register_150 0x00000096
#define WAN__TX_X4_Control0_misc                                 0x0000c433
#define WAN__RX_X4_Control0_pma_control_0                        0x0000c457
#define WAN__Main0_setup_1                                       0x00009101
#define WAN__Main0_misc_control                                  0x0000910c
#define WAN__Digital_Control1000X2                               0x0000c301
#define WAN__AN_X4_USXGMII_usxgmii_an_control0                   0x0000c4b1
#define WAN__AN_X4_USXGMII_usxgmii_an_control1                   0x0000c4b2
#define WAN__TX_X1_Control0_pmd_osr_mode_2                       0x00009206
#define WAN__TX_X1_Control0_pmd_tx_osr_mode                      0x00009207
#define WAN__AN_X4_USXGMII_usxgmii_an_config                     0x0000c4b0
#define WAN__TX_X1_CREDIT_GEN4_reg2p5G_KR1_credit1               0x00009295
#define WAN__TX_X1_CREDIT_GEN4_reg5G_KR1_credit1                 0x00009299
#define WAN__AN_X4_ABILITIES_local_device_cl37_base_abilities    0x0000c481
#define WAN__RX_X4_Control0_cl36_rx_0                            0x0000c456
#define WAN__TX_X1_Control0_os_mode_cl36_2                       0x00009205
#define WAN__CL22_B0_MIICntl                                     0x00000000
#define WAN__TX_X1_Control0_os_mode_cl36                         0x00009201

#define PCS_READ(address, rd_data) pmi_rw(0, 1, address, 0, 0, &rd_data)
#define PCS_WRITE_MASK(address, wr_data, mask) pmi_rw(1, 1, address, wr_data, mask, NULL)
#define PCS_WRITE(address, wr_data) PCS_WRITE_MASK(address, wr_data, 0x0000)

#define SET_BITS(var, shift, val, mask) var = (var & ~(mask << shift)) | (val << shift)

#pragma pack(push,1)
typedef struct
{
    uint32_t pmd_mpcs_rx_lock:1;
    uint32_t md_mpcs_signal_detect:1;
    uint32_t md_mpcs_tx_clk_vld:1;
    uint32_t fg_mpcs_por_rstb:1;
    uint32_t fg_mpcs_clk_en:1;
    uint32_t fg_mpcs_refclk_rstb:1;
    uint32_t fg_mpcs_spare:26;
} mpcs_reg_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t misc_pmi_lp_write:1;
    uint32_t misc_pmi_lp_en:1;
    uint32_t pcs_pmi_lp_en:1;
    uint32_t reserved:29;
} pmi_lp_0_reg_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t misc_pmi_lp_addr;
} pmi_lp_1_reg_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t pmi_lp_maskdata:16;
    uint32_t pmi_lp_wrdata:16;
} pmi_lp_2_reg_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t pmi_lp_rddata:16;
    uint32_t pmi_lp_ack:1;
    uint32_t pmi_lp_err:1;
    uint32_t reserved:14;
} pmi_lp_3_reg_t;
#pragma pack(pop)

static DEFINE_SPINLOCK(pmi_access);
static void __iomem * wantop_base;
#define WANTOP_BASE (volatile void *)wantop_base
#define PMI_LP_0_REG          (WANTOP_BASE + 0x5c)
#define PMI_LP_1_REG          (WANTOP_BASE + 0x60)
#define PMI_LP_2_REG          (WANTOP_BASE + 0x64)
#define PMI_LP_3_REG          (WANTOP_BASE + 0x68)
#define MPCS_REG              (WANTOP_BASE + 0xf8)
#define FORCE_LBE_CONTROL_REG (WANTOP_BASE + 0x9c)
#define WAN_TOP_WAN_TOP_SPARE (WANTOP_BASE + 0xdc)

static void __iomem * epontop_base;
#define EPONTOP_BASE (volatile void *)epontop_base
#define EPON_TOP_CONTROL_REG  (EPONTOP_BASE + 0x10)

static void epon_reset(espeed_t cfg_port_speed)
{
    uint32_t reg = 0;

    if (cfg_port_speed == eSPEED_2_2)
        reg |= 0x10;
    else if (cfg_port_speed == eSPEED_1_1)
        reg |= 0x0;
    else if (cfg_port_speed == eSPEED_10_10 || cfg_port_speed == eSPEED_5_5)
        reg |= 0x3; /* 10G cfgTenGigPonUp | cfgTenGigDns */

    WRITE_32(EPON_TOP_CONTROL_REG, reg);
}

static int is_epon_reset_required(void)
{
#ifdef CONFIG_BCM96888
    return UtilGetChipRev() == 0xa0;
#endif
    return 0;
}

static void spare_set(espeed_t cfg_port_speed)
{
    uint32_t reg = 0;

    if (cfg_port_speed == eSPEED_2_2)
        reg |= 0x2;

    WRITE_32(WAN_TOP_WAN_TOP_SPARE, reg);
}

static void mpcs_reg_post(void)
{
    mpcs_reg_t mpcs_reg;

    mpcs_dbg("INFO: INIT_MPCS POST\n");
    READ_32(MPCS_REG, mpcs_reg);
    mpcs_reg.pmd_mpcs_rx_lock = 0;
    mpcs_reg.md_mpcs_signal_detect = 0;
    mpcs_reg.md_mpcs_tx_clk_vld = 0;
    mpcs_reg.fg_mpcs_por_rstb = 1;
    mpcs_reg.fg_mpcs_clk_en = 1;
    mpcs_reg.fg_mpcs_refclk_rstb = 1;
    WRITE_32(MPCS_REG, mpcs_reg);

    READ_32(MPCS_REG, mpcs_reg);
    mpcs_reg.pmd_mpcs_rx_lock = 1;
    mpcs_reg.md_mpcs_signal_detect = 1;
    mpcs_reg.md_mpcs_tx_clk_vld = 1;
    mpcs_reg.fg_mpcs_por_rstb = 1;
    mpcs_reg.fg_mpcs_clk_en = 1;
    mpcs_reg.fg_mpcs_refclk_rstb = 1;
    WRITE_32(MPCS_REG, mpcs_reg);

    udelay(1000);
}

static void mpcs_reg_init(void)
{
    mpcs_reg_t mpcs_reg;

    mpcs_dbg("INFO: BEGIN wan_top_ral_sequence INIT_MPCS\n");
    READ_32(MPCS_REG, mpcs_reg);
    mpcs_reg.pmd_mpcs_rx_lock = 0;
    mpcs_reg.md_mpcs_signal_detect = 0;
    mpcs_reg.md_mpcs_tx_clk_vld = 0;
    mpcs_reg.fg_mpcs_por_rstb = 0;
    mpcs_reg.fg_mpcs_clk_en = 0;
    mpcs_reg.fg_mpcs_refclk_rstb = 0;
    WRITE_32(MPCS_REG, mpcs_reg);

    READ_32(MPCS_REG, mpcs_reg);
    mpcs_reg.pmd_mpcs_rx_lock = 0;
    mpcs_reg.md_mpcs_signal_detect = 0;
    mpcs_reg.md_mpcs_tx_clk_vld = 0;
    mpcs_reg.fg_mpcs_por_rstb = 0;
    mpcs_reg.fg_mpcs_clk_en = 1;
    mpcs_reg.fg_mpcs_refclk_rstb = 1;
    WRITE_32(MPCS_REG, mpcs_reg);

    READ_32(MPCS_REG, mpcs_reg);
    mpcs_reg.pmd_mpcs_rx_lock = 0;
    mpcs_reg.md_mpcs_signal_detect = 0;
    mpcs_reg.md_mpcs_tx_clk_vld = 0;
    mpcs_reg.fg_mpcs_por_rstb = 1;
    mpcs_reg.fg_mpcs_clk_en = 1;
    mpcs_reg.fg_mpcs_refclk_rstb = 1;
    WRITE_32(MPCS_REG, mpcs_reg);

    udelay(1000);
}

static uint32_t pmi_rw(uint32_t cmd_write, uint32_t pcs, uint32_t addr, uint16_t wr_data, uint16_t mask, uint16_t *rd_data)
{
    pmi_lp_0_reg_t lp0 = {};
    pmi_lp_1_reg_t lp1 = {};
    pmi_lp_2_reg_t lp2 = {};
    pmi_lp_3_reg_t lp3 = {};
    uint32_t zero = 0;
    int max_tries = 10000;
    int ret = -1;

    mpcs_dbg("pmi_rw: %s, addr 0x%x, wr_data 0x%x, mask 0x%x\n", cmd_write ? "write" : "read", addr, wr_data, mask);

    lp0.misc_pmi_lp_write = cmd_write ? 1 : 0;
    lp0.misc_pmi_lp_en = pcs ? 1 : 1;
    lp0.pcs_pmi_lp_en = pcs ? 0 : 0;
    lp0.reserved = 0;
    lp1.misc_pmi_lp_addr = addr;
    lp2.pmi_lp_maskdata = mask;
    lp2.pmi_lp_wrdata = wr_data;

    spin_lock_bh(&pmi_access);

    WRITE_32(PMI_LP_1_REG, lp1);
    WRITE_32(PMI_LP_2_REG, lp2);
    WRITE_32(PMI_LP_0_REG, lp0);

    do
    {
        udelay(5);
        READ_32(PMI_LP_3_REG, lp3);
        if (lp3.pmi_lp_ack == 1)
            break;
    } while (--max_tries);

    if (lp3.pmi_lp_ack == 0 || !max_tries || lp3.pmi_lp_err)
    {
        mpcs_err("pml_lp_ack == 0 | pml_lp_err %d | max_tries %d\n", lp3.pmi_lp_err, max_tries);
        goto exit;
    }

    if (!cmd_write)
    {
        if (pcs)
        {
            READ_32(PMI_LP_3_REG, lp3);
            *rd_data = lp3.pmi_lp_rddata;
            mpcs_dbg("pmi_rw: read result: addr[0x%x] = 0x%x\n", addr, *rd_data);
        }
    }

    WRITE_32(PMI_LP_0_REG, zero);

    ret = 0;
exit:
    spin_unlock_bh(&pmi_access);

    return ret;
}

static void mpcs_cfg(struct mpcs_cfg_s *m)
{
    uint16_t pcs_reg;

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pcs_control_0), pcs_reg);
    SET_BITS(pcs_reg, 2, 0x1, 0x1); // [2] LPI_ENABLE
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pcs_control_0), pcs_reg);

    if ((m->cfg_port_speed == eSPEED_10_10) || (m->cfg_port_speed == eSPEED_5_5))
    {
        mpcs_dbg("BEGIN programing PCS registers through LP for 10G INIT_MPCS");
        if (m->cfg_usxgmii_mode)
        {
            PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Main0_setup_1), pcs_reg);
            SET_BITS(pcs_reg, 5, 0x1, 0x1); // [5] usxgmii_pcs_sel = select CoreB PCS
            PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Main0_setup_1), pcs_reg);

            PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Digital_Control1000X2), pcs_reg);
            SET_BITS(pcs_reg, 0, 0x0, 0x1); // [0] ubaud, 0 = 10G baud rate in single USXGMII mode; 1 = 5G baud rate
            PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Digital_Control1000X2), pcs_reg);

            PCS_READ((DEVID_0 | LANE_BRDCST | WAN__AN_X4_USXGMII_usxgmii_an_config), pcs_reg);
            SET_BITS(pcs_reg, 15, 0x1, 0x1); // [15]  link  link: 1 = link up; 0 = link down
            SET_BITS(pcs_reg, 14, 0x1, 0x1); // [14]  acknowlege  usxgmii autoneg acknowledge
            SET_BITS(pcs_reg, 13, 0x0, 0x1); // [13]  Reserved  Reserved bit must be written with 0.  A read returns an unknown value.
            SET_BITS(pcs_reg, 12, 0x1, 0x1); // [12]  duplex  usxgmii autoneg duplex mode: 1 = full; 0 = half

            if (m->cfg_usxgmii_5g)
            {
                SET_BITS(pcs_reg, 9, 0x5, 0x7); // [11:09] speed usxgmii autoneg network port speed:
            }
            else if (m->cfg_usxgmii_2p5g)
            {
                SET_BITS(pcs_reg, 9, 0x4, 0x7); // [11:09] speed usxgmii autoneg network port speed:
            }
            else if (m->cfg_usxgmii_1g)
            {
                SET_BITS(pcs_reg, 9, 0x2, 0x7); // [11:09] speed usxgmii autoneg network port speed:
            }
            else if (m->cfg_usxgmii_100m)
            {
                SET_BITS(pcs_reg, 9, 0x1, 0x7); // [11:09] speed usxgmii autoneg network port speed:
            }
            else if (m->cfg_usxgmii_10m)
            {
                SET_BITS(pcs_reg, 9, 0x0, 0x7); // [11:09] speed usxgmii autoneg network port speed:
            }
            else
            {
                SET_BITS(pcs_reg, 9, 0x3, 0x7); // [11:09] speed usxgmii autoneg network port speed:
            }
            // 000 = 10Mbps
            // 001 = 100 Mbps
            //  10 = 1000 Mbps
            //  11 = 10 Gbps
            // 100 = 2.5 Gbps
            // 101 = 5 Gbps
            // 110 = Reserved
            // 111 = Reserved
            SET_BITS(pcs_reg, 8, 0x0, 0x1); // [08] EEE_capability  EEE capability: 1 = suported, 0 = not supported
            SET_BITS(pcs_reg, 7, 0x0, 0x1); // [07] EEE_clock_stop_capability EEE clock stop capability: 1 = suported, 0 = not supported
            SET_BITS(pcs_reg, 1, 0x0, 0x3f); // [06:01] Reserved  Reserved bits must be written with 0.  A read returns an unknown value.
            SET_BITS(pcs_reg, 0, 0x1, 0x1); // [00] sgmii
            PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__AN_X4_USXGMII_usxgmii_an_config), pcs_reg);

            PCS_READ((DEVID_0 | LANE_BRDCST | WAN__AN_X4_USXGMII_usxgmii_an_control0), pcs_reg);
            SET_BITS(pcs_reg, 15, 0x0, 0x1); // [15] enable_linkchg_restart  Enable AN restart upon link change
            SET_BITS(pcs_reg, 14, 0x0, 0x0001); // [14] spd_switch_ov_en  spd_switch_ov
            SET_BITS(pcs_reg, 13, 0x0, 0x0001); // [13] spd_switch_ov spd_switch_ov
            SET_BITS(pcs_reg, 12, 0x0, 0x0001); // [12] restart_an  restart_an
            SET_BITS(pcs_reg, 11, 0x0, 0x0001); // [11] disable_rudi_invalid  disable_rudi_invalid
            SET_BITS(pcs_reg, 10, 0x0, 0x0001); // [10] enable_lpi_idle_match enable_lpi_idle_match
            SET_BITS(pcs_reg,  9, 0x0, 0x0001); // [09] enable_linkdown_restart enable_linkdown_restart
            SET_BITS(pcs_reg,  8, 0x0, 0x0001); // [08] check_usxgmii_selector  check_usxgmii_selector
            SET_BITS(pcs_reg,  7, 0x0, 0x0001); // [07] anerr_timer_en  anerr_timer_en
            SET_BITS(pcs_reg,  6, 0x0, 0x0001); // [06] an_fast_timer an_fast_timer
            SET_BITS(pcs_reg,  5, 0x0, 0x0001); // [05] filter_forced_link  filter_forced_link
            SET_BITS(pcs_reg,  4, 0x1, 0x0001); // [04] lane_status_sel_enable  lane_status_sel_enable
            SET_BITS(pcs_reg,  3, 0x1, 0x0001); // [03] port_link_status_delay_en port_link_status_delay_en
            SET_BITS(pcs_reg,  2, 0x0, 0x0001); // [02] force_xmit_data usxgmii an force xmit data
            SET_BITS(pcs_reg,  1, 0x0, 0x0001); // [01] usxgmii_an_enable usxgmii an enable
            SET_BITS(pcs_reg,  0, 0x0, 0x0001); // [00] usxgmii_an_mode usxgmii an mode
            PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__AN_X4_USXGMII_usxgmii_an_control0), pcs_reg);

            PCS_READ((DEVID_0 | LANE_BRDCST | WAN__AN_X4_USXGMII_usxgmii_an_control1), pcs_reg);
            SET_BITS(pcs_reg, 15, 0x0, 0x0001); // [15] enable_an_slave_link_fix  Enable AN slave link fix
            SET_BITS(pcs_reg, 14, 0x0, 0x0001); // [14] disable_an_slave_act_fix  Disable fix of latched link partner's advertised capability.
            SET_BITS(pcs_reg, 13, 0x0, 0x0001); // [13] enable_linkup_restart Enable AN restart upon linkup
            SET_BITS(pcs_reg, 12, 0x0, 0x0001); // [12] usxgmii_an_switch_ov_en usxgmii_an_switch_ov_en
            SET_BITS(pcs_reg, 11, 0x0, 0x0001); // [11] usxgmii_an_idle_ov  usxgmii_an_idle_ov
            SET_BITS(pcs_reg, 10, 0x0, 0x0001); // [10] usxgmii_an_config_ov  usxgmii_an_config_ov
            SET_BITS(pcs_reg,  9, 0x0, 0x0001); // [09] usxgmii_master_mode usxgmii_master_mode
            SET_BITS(pcs_reg,  8, 0x0, 0x0001); // [08] usxgmii_ext_sel usxgmii_ext_sel
            SET_BITS(pcs_reg,  7, 0x0, 0x0001); // [07] force_sequence_tx_en  force_sequence_tx_en
            SET_BITS(pcs_reg,  4, 0x0, 0x0007); // [06:04] force_sequence_tx_sel force_sequence_tx_sel
            SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [03:00] link_timer  link_timer
            PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__AN_X4_USXGMII_usxgmii_an_control1), pcs_reg);
        }

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);
        if (m->cfg_usxgmii_mode)
        {
            if (m->cfg_usxgmii_5g)
            {
                SET_BITS(pcs_reg,  0, 0x35, 0x003f); // [5:0] SW_actual_speed = dr_5G_KR1_USXGMII
            }
            else if (m->cfg_usxgmii_2p5g)
            {
                SET_BITS(pcs_reg,  0, 0x30, 0x003f); // [5:0] SW_actual_speed = dr_2p5G_KR1_USXGMII
            }
            else if (m->cfg_usxgmii_1g)
            {
                SET_BITS(pcs_reg,  0, 0x2f, 0x003f); // [5:0] SW_actual_speed = dr_1G_KR1_USXGMII
            }
            else if (m->cfg_usxgmii_100m)
            {
                SET_BITS(pcs_reg,  0, 0x2e, 0x003f); // [5:0] SW_actual_speed = dr_100M_KR1_USXGMII
            }
            else
            {
                SET_BITS(pcs_reg,  0, 0x36, 0x003f); // [5:0] SW_actual_speed = dr_10G_KR1_USXGMII
            }
        }
        else
        {
            if (m->cfg_5g5g_mode || m->cfg_5g5g_mode_vcoDiv2)
            {
                SET_BITS(pcs_reg,  0, 0x29, 0x003f); // [5:0] SW_actual_speed = dr_5G_KR1_CL129
            }
            else if (m->cfg_2p5g2p5g_mode || m->cfg_2p5g2p5g_mode_vcoDiv4)
            {
                SET_BITS(pcs_reg,  0, 0x28, 0x003f); // [5:0] SW_actual_speed = dr_2p5G_KR1_CL129
            }
            else
            {
                SET_BITS(pcs_reg,  0, 0x0f, 0x003f); // [5:0] SW_actual_speed = dr_10G_KR
            }
        }
        SET_BITS(pcs_reg,  6, 0x1, 0x0001); // [6] SW_actual_speed_force_en = 1
        SET_BITS(pcs_reg,  7, 0x1, 0x0001); // [7] mac_creditenable = 1 Must enable credits to enable Rx rate adaption
        SET_BITS(pcs_reg, 11, 0x0, 0x0001); // [11] credit_sw_en = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl49), pcs_reg);
        if (m->cfg_usxgmii_mode)
        {
            if (m->cfg_usxgmii_5g)
            {
                //lp_wr_data[3:0] = 0x1; // os_mode_cl49 = 1
            } else if (m->cfg_usxgmii_2p5g) {
            } else if (m->cfg_usxgmii_1g) {
            } else if (m->cfg_usxgmii_100m) {
            } else {
            }
        }
        else
        {
            if (m->cfg_5g5g_mode || m->cfg_5g5g_mode_vcoDiv2)
            {
                SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] os_mode_cl49_5g = 0
                SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] os_mode_cl49 = 0
            }
            else if (m->cfg_2p5g2p5g_mode || m->cfg_2p5g2p5g_mode_vcoDiv4)
            {
                SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] os_mode_cl49_2p5g = 0
                SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] os_mode_cl49 = 0
            }
            else
            {
                SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] os_mode_cl49 = 0
            }
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl49), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);
        if (m->cfg_usxgmii_mode)
        {
            if (m->cfg_usxgmii_5g)
            {
                //lp_wr_data[3:0] = 0x1; // osr_mode_cl49 = 1
            } else if (m->cfg_usxgmii_2p5g) {
            } else if (m->cfg_usxgmii_1g) {
            } else if (m->cfg_usxgmii_100m) {
            }
        }
        else
        {
            if (m->cfg_5g5g_mode || m->cfg_5g5g_mode_vcoDiv2)
            {
                SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] osr_mode_cl49 = 0
            }
            else if (m->cfg_2p5g2p5g_mode || m->cfg_2p5g2p5g_mode_vcoDiv4)
            {
                SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] osr_mode_cl36_2p5 = 0
                SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] osr_mode_cl49 = 0
            }
            else
            {
                SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] osr_mode_cl49 = 0
            }
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);
        if (m->cfg_5g5g_mode || m->cfg_5g5g_mode_vcoDiv2)
        {
            SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] osr_mode_cl49_5g = 0
        }
        else if (m->cfg_2p5g2p5g_mode || m->cfg_2p5g2p5g_mode_vcoDiv4)
        {
            SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] osr_mode_cl49_2p5g = 0
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_tx_osr_mode), pcs_reg);
        if (m->cfg_5g5g_mode || m->cfg_5g5g_mode_vcoDiv2)
        {
            SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] tx_osr_mode_cl49 = 0
            SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] pmd_tx_osr_mode = 0
        }
        else if (m->cfg_2p5g2p5g_mode || m->cfg_2p5g2p5g_mode_vcoDiv4)
        {
            SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] tx_osr_mode_cl49 = 0
            SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] pmd_tx_osr_mode = 0
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_tx_osr_mode), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_tx_osr_mode_2), pcs_reg);
        if (m->cfg_5g5g_mode || m->cfg_5g5g_mode_vcoDiv2)
        {
            SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] tx_osr_mode_cl49_5g = 0
            SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] tx_osr_mode_cl36_5 = 0
        }
        else if (m->cfg_2p5g2p5g_mode || m->cfg_2p5g2p5g_mode_vcoDiv4)
        {
            SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] tx_osr_mode_cl49_2p5g = 0
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_tx_osr_mode_2), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_CREDIT_GEN4_reg5G_KR1_credit1), pcs_reg);
        if (m->cfg_5g5g_mode || m->cfg_5g5g_mode_vcoDiv2)
        {
            SET_BITS(pcs_reg,  0, 0x00008, 0x0fff); // [12:0] reg5G_KR1_CGC = 8
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_CREDIT_GEN4_reg5G_KR1_credit1), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_CREDIT_GEN4_reg2p5G_KR1_credit1), pcs_reg);
        if (m->cfg_2p5g2p5g_mode || m->cfg_2p5g2p5g_mode_vcoDiv4)
        {
            SET_BITS(pcs_reg,  0, 0x00010, 0x0fff); // [12:0] reg2p5G_KR1_CGC = 13'h10
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_CREDIT_GEN4_reg2p5G_KR1_credit1), pcs_reg);
    }
    else if (((m->cfg_port_speed == eSPEED_2_2) || ((m->cfg_port_speed == eSPEED_1_1) && (m->clk_mode_312))) && (m->cfg_sgmii_1000m == 0) && (m->cfg_sgmii_100m == 0) && (m->cfg_sgmii_10m == 0))
    {
        mpcs_dbg("BEGIN programing PCS registers through LP for 2.5G INIT_MPCS");

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);
        SET_BITS(pcs_reg,  0, 0x03, 0x003f); // [5:0] SW_actual_speed = dr_2p5G
        SET_BITS(pcs_reg,  6, 0x1, 0x0001); // [6] SW_actual_speed_force_en = 1
        SET_BITS(pcs_reg,  7, 0x1, 0x0001); // [7] mac_creditenable = 1 Must enable credits to enable Rx rate adaption
        SET_BITS(pcs_reg, 11, 0x0, 0x0001); // [11] credit_sw_en = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl36), pcs_reg);
        SET_BITS(pcs_reg, 12, 0x0, 0x000f); // [15:12] os_mode_cl36_2500m = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl36), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);
        SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] osr_mode_cl36_2p5 = 0
        SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] osr_mode_cl49 = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] tx_osr_mode_cl36 = 0
        SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] tx_osr_mode_cl36_2p5 = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);
    }
    else if (((m->cfg_port_speed == eSPEED_1_1)) && (m->cfg_sgmii_1000m == 0) && (m->cfg_sgmii_100m == 0) && (m->cfg_sgmii_10m == 0))
    {
        mpcs_dbg("BEGIN programing PCS registers through LP for 1G INIT_MPCS");

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);
        SET_BITS(pcs_reg,  0, 0x02, 0x003f); // [5:0] SW_actual_speed = dr_1G
        SET_BITS(pcs_reg,  6, 0x1, 0x0001); // [6] SW_actual_speed_force_en = 1
        SET_BITS(pcs_reg,  7, 0x1, 0x0001); // [7] mac_creditenable = 1 Must enable credits to enable Rx rate adaption
        SET_BITS(pcs_reg, 11, 0x0, 0x0001); // [11] credit_sw_en = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl36), pcs_reg);
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] os_mode_cl36_1000m = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl36), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] osr_mode_cl36 = 0
        SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] osr_mode_cl49 = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] tx_osr_mode_cl36 = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);
    }
    else if (((m->cfg_port_speed == eSPEED_1_1)) && ((m->cfg_sgmii_1000m == 1) || (m->cfg_sgmii_100m == 1) || (m->cfg_sgmii_10m == 1)))
    {
        if (m->cfg_sgmii_1000m == 1)
        {
            mpcs_dbg("BEGIN programing PCS registers through LP for 1G INIT_MPCS - sgmii_1000m");
        }
        else if (m->cfg_sgmii_100m == 1)
        {
            mpcs_dbg("BEGIN programing PCS registers through LP for 1G INIT_MPCS - sgmii_100m");
        }
        else if (m->cfg_sgmii_10m == 1)
        {
            mpcs_dbg("BEGIN programing PCS registers through LP for 1G INIT_MPCS - sgmii_10m");
        }

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__CL22_B0_MIICntl), pcs_reg);
        if (m->cfg_sgmii_1000m == 1)
        {
            SET_BITS(pcs_reg,  6, 0x1, 0x0001); // [13] {manual_speed1, manual_speed0}: 1 0 = SGMII 1000 Mb/s
            SET_BITS(pcs_reg, 13, 0x0, 0x0001); // [13] {manual_speed1, manual_speed0}: 1 0 = SGMII 1000 Mb/s
        }
        else if (m->cfg_sgmii_100m == 1)
        {
            SET_BITS(pcs_reg,  6, 0x0, 0x0001); // [13] {manual_speed1, manual_speed0}: 0 1 = SGMII 100 Mb/s
            SET_BITS(pcs_reg, 13, 0x1, 0x0001); // [13] {manual_speed1, manual_speed0}: 0 1 = SGMII 100 Mb/s
        }
        else if (m->cfg_sgmii_10m == 1)
        {
            SET_BITS(pcs_reg,  6, 0x0, 0x0001); // [13] {manual_speed1, manual_speed0}: 0 0 = SGMII 10 Mb/s
            SET_BITS(pcs_reg, 13, 0x0, 0x0001); // [13] {manual_speed1, manual_speed0}: 0 0 = SGMII 10 Mb/s
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__CL22_B0_MIICntl), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);
        SET_BITS(pcs_reg, 15, 0x1, 0x0001); // [15] force_ieee_SpeedSel_en = 0x1
        SET_BITS(pcs_reg, 14, 0x1, 0x0001); // [14] force_ieee_cl22_SpeedSel_en = 0x1
        SET_BITS(pcs_reg, 12, 0x2, 0x0003); // [13:12] use_ieee_reg_ctrl_sel = 2'b10
        SET_BITS(pcs_reg, 11, 0x0, 0x0001); // [11] credit_sw_en = 0
        SET_BITS(pcs_reg,  7, 0x1, 0x0001); // [7] mac_creditenable = 1 Must enable credits to enable Rx rate adaption
        SET_BITS(pcs_reg,  6, 0x0, 0x0001); // [6] SW_actual_speed_force_en = 0
        SET_BITS(pcs_reg,  0, 0x02, 0x0001); // [5:0] SW_actual_speed = dr_1G
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__AN_X4_ABILITIES_local_device_cl37_base_abilities), pcs_reg);
        SET_BITS(pcs_reg,  9, 0x1, 0x0001); // [9] sgmii_master_mode = 1
        SET_BITS(pcs_reg,  2, 0x1, 0x0001); // [2] sgmii_full_duplex = 1
        if (m->cfg_sgmii_1000m == 1)
        {
            SET_BITS(pcs_reg,  0, 0x2, 0x0003); // [1:0] sgmii_speed = 2'b10 -> SGMII 1000 Mb/s
        }
        else if (m->cfg_sgmii_100m == 1)
        {
            SET_BITS(pcs_reg,  0, 0x1, 0x0003); // [1:0] sgmii_speed = 2'b01 -> SGMII 100 Mb/s
        }
        else if (m->cfg_sgmii_10m == 1)
        {
            SET_BITS(pcs_reg,  0, 0x0, 0x0003); // [1:0] sgmii_speed = 2'b00 -> SGMII 10 Mb/s
        }
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__AN_X4_ABILITIES_local_device_cl37_base_abilities), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__AN_X4_ABILITIES_local_device_cl37_base_abilities), pcs_reg);
        SET_BITS(pcs_reg,  7, 0x1, 0x0001); // [7] cl37_sgmii_enable = 1
        SET_BITS(pcs_reg,  6, 0x1, 0x0001); // [6] cl37_enable = 1
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__AN_X4_ABILITIES_local_device_cl37_base_abilities), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl36), pcs_reg);
        SET_BITS(pcs_reg, 12, 0x0, 0x000f); // [15:12] os_mode_cl36_2500m = 0
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] os_mode_cl36_1000m = 0
        SET_BITS(pcs_reg,  4, 0x0, 0x000f); // [7:4] os_mode_cl36_100m = 0
        SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] os_mode_cl36_10m = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_os_mode_cl36), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] osr_mode_cl36 = 0
        SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] osr_mode_cl49 = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] tx_osr_mode_cl36 = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode_2), pcs_reg);
    }
    else if (m->cfg_port_speed == eSPEED_100m_100m)
    {
        mpcs_dbg("BEGIN programing PCS registers through LP for 100M INIT_MPCS");

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);
        SET_BITS(pcs_reg,  0, 0x01, 0x003f); // [5:0] SW_actual_speed = dr_100M
        SET_BITS(pcs_reg,  6, 0x1, 0x0001); // [6] SW_actual_speed_force_en = 1
        SET_BITS(pcs_reg,  7, 0x1, 0x0001); // [7] mac_creditenable = 1 Must enable credits to enable Rx rate adaption
        SET_BITS(pcs_reg, 11, 0x0, 0x0001); // [11] credit_sw_en = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Digital_MiscDigControl), pcs_reg);

        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);
        SET_BITS(pcs_reg,  8, 0x0, 0x000f); // [11:8] osr_mode_cl36 = 0
        SET_BITS(pcs_reg,  0, 0x0, 0x000f); // [3:0] osr_mode_cl49 = 0
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X1_Control0_pmd_osr_mode), pcs_reg);
    }

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__CL72_IEEE_TX_cl72it_BASE_R_PMD_control_register_150), pcs_reg);
    SET_BITS(pcs_reg,  1, 0x1, 0x0001); // [1] cl72_ieee_training_enable = 1
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__CL72_IEEE_TX_cl72it_BASE_R_PMD_control_register_150), pcs_reg);

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X4_Control0_misc), pcs_reg);
    SET_BITS(pcs_reg, 10, 0x0, 0x0001); // [10] fec_enable = 0
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X4_Control0_misc), pcs_reg);

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_decode_control_0), pcs_reg);
    SET_BITS(pcs_reg,  0, 0x0, 0x0007); // [2:0] block_sync_mode = 0 (none)
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_decode_control_0), pcs_reg);

    if ((m->cfg_port_speed == eSPEED_2_2) || (m->cfg_port_speed == eSPEED_1_1))
    {
        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_cl36_rx_0), pcs_reg);
        SET_BITS(pcs_reg,  2, 0x1, 0x0001); // [2] disable_carrier_extend = 1
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_cl36_rx_0), pcs_reg);
    }

    if (((m->cfg_port_speed == eSPEED_2_2) || ((m->cfg_port_speed == eSPEED_1_1) && (m->clk_mode_312))) && (m->cfg_sgmii_1000m == 0) && (m->cfg_sgmii_100m == 0) && (m->cfg_sgmii_10m == 0))
    {
        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_cl36_rx_0), pcs_reg);
        SET_BITS(pcs_reg,  4, 0x1, 0x0001); // [4] Reserved, for 12.5GHz VCO
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_cl36_rx_0), pcs_reg);
    }

    if (m->cfg_port_speed == eSPEED_2_2)
    {
        PCS_READ((DEVID_0 | LANE_BRDCST | WAN__Main0_misc_control), pcs_reg);
        SET_BITS(pcs_reg,  7, !!m->cfg_2p5g_is_xgmii , 0x0001); // cl36_xgmii_mode_en (gmii mode if not set)
        PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__Main0_misc_control), pcs_reg);
    }

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);
    SET_BITS(pcs_reg,  0, 0x1, 0x0001); // [0] rstb_lane = 1
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__TX_X4_Control0_misc), pcs_reg);
    SET_BITS(pcs_reg,  0, 0x1, 0x0001); // [0] enable_tx_lane = 1
    SET_BITS(pcs_reg,  1, 0x1, 0x0001); // [1] rstb_tx_lane = 1
    if (m->cfg_usxgmii_mode)
    {
        SET_BITS(pcs_reg,  4, 0x1, 0x0001); // [4] rstb_tx_port = 1
    }
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__TX_X4_Control0_misc), pcs_reg);

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);
    SET_BITS(pcs_reg,  0, 0x1, 0x0001); // [0] rstb_lane
    SET_BITS(pcs_reg,  1, 0x1, 0x0001); // [1] rx_gbox_afrst_en
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);
}

void mpcs_rx_reset(void)
{
    uint16_t pcs_reg;

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);
    SET_BITS(pcs_reg,  0, 0x1, 0x0001); // [0] rstb_lane = 1
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);
    SET_BITS(pcs_reg,  0, 0x0, 0x0001); // [0] rstb_lane
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);

    PCS_READ((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);
    SET_BITS(pcs_reg,  0, 0x1, 0x0001); // [0] rstb_lane
    PCS_WRITE((DEVID_0 | LANE_BRDCST | WAN__RX_X4_Control0_pma_control_0), pcs_reg);
}

void mpcs_init(struct mpcs_cfg_s *m)
{
    if (!wantop_base)
    {
        mpcs_err("mpcs not initialized! (wantop_base: %p)\n", wantop_base);
        return;
    }

    if (is_epon_reset_required())
    {
        if (!epontop_base)
        {
            mpcs_err("mpcs not initialized! (epontop_base %p)\n", epontop_base);
            return;
        }
        epon_reset(m->cfg_port_speed);
    }

    spare_set(m->cfg_port_speed);

    mpcs_reg_init();

    mpcs_cfg(m);

    mpcs_reg_post();
}

int is_pcs_locked(void)
{
#define MPCS_RXTXLOCK_BITS 0x3
#define MPCS_RXTXLOCK_REG 0xc474
    int ret, reg = MPCS_RXTXLOCK_REG;
    uint16_t _val = 0;

    ret = PCS_READ((DEVID_0 | LANE_BRDCST | reg), _val);

    mpcs_dbg("pcs_lock 0x%x\n", _val);

    return (_val & MPCS_RXTXLOCK_BITS) == MPCS_RXTXLOCK_BITS;
}

int is_pcs_state(void)
{
#define MPCS_STATE_REG 0xc473
    uint16_t _val = 1;

    PCS_READ((DEVID_0 | LANE_BRDCST | MPCS_STATE_REG), _val);

    return _val;
}

int mpcs_read(uint16_t reg, uint16_t *val)
{
    int ret;
    uint16_t _val;

    ret = PCS_READ((DEVID_0 | LANE_BRDCST | reg), _val);
    *val = _val;

    return ret;
}

int mpcs_write(uint16_t reg, uint16_t val)
{
    return PCS_WRITE((DEVID_0 | LANE_BRDCST | reg), val);
}

void mpcs_wantop_force_lbe(void)
{
    int reg = 0xd;

    WRITE_32(FORCE_LBE_CONTROL_REG, reg);
}

static int mpcs_probe(dt_device_t *pdev)
{
    int ret;

    wantop_base = dt_dev_remap(pdev, 0);
    if (IS_ERR(wantop_base))
    {
        ret = PTR_ERR(wantop_base);
        wantop_base = NULL;
        dev_err(&pdev->dev, "Missing wantop_base entry\n");
        goto Exit;
    }

    if (is_epon_reset_required())
    {
        epontop_base = dt_dev_remap(pdev, 1);
        if (IS_ERR(epontop_base))
        {
            ret = PTR_ERR(epontop_base);
            epontop_base = NULL;
            dev_err(&pdev->dev, "Missing epontop_base entry\n");
            goto Exit;
        }
    }

    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,mpcs" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-mpcs",
        .of_match_table = of_platform_table,
    },
    .probe = mpcs_probe,
};
module_platform_driver(of_platform_driver);
