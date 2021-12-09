/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef __PHY_DRV_10GAE_H__
#define __PHY_DRV_10GAE_H__

#include "bcm_map_part.h"
#include "bcmnet.h"
#include "board.h"
#include "bcm_otp.h"
#include "bcm_OS_Deps.h"
#include "pmc_drv.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"
#include "pmc_drv.h"

#define eSUB_RATE        0
#define eFULL_RATE       1
/***************************************************************************
// Serdes registers in full chip address map
 ***************************************************************************/
#define eSPEED_1_1       0
#define eSPEED_10_10     1
#define eSPEED_2_1       2
#define eSPEED_10_1      3
#define eSPEED_2_2       4
#define eSPEED_100m_100m 5

#define SERDES_0                     0
#define SERDES_1                     1

#define DEVID_0                      0x00000000
#define DEVID_1                      0x08000000
#define PLL_0                        0x00000000
#define PLL_1                        0x01000000
#define LANE_0                       0x00000000
#define LANE_1                       0x00010000
#define LANE_2                       0x00020000
#define LANE_3                       0x00030000
#define LANE_BRDCST                  0x00FF0000
#define LANE_01                      0x02000000
#define LANE_23                      0x02010000

#define DSC_A_cdr_control_0 0x0000D001
#define DSC_A_cdr_control_1 0x0000D002
#define DSC_A_cdr_control_2 0x0000D003
#define DSC_A_rx_pi_control 0x0000D004
#define DSC_A_cdr_status_integ_reg 0x0000D005
#define DSC_A_cdr_status_phase_error 0x0000D006
#define DSC_A_rx_pi_cnt_bin_d 0x0000D007
#define DSC_A_rx_pi_cnt_bin_p 0x0000D008
#define DSC_A_rx_pi_cnt_bin_m 0x0000D009
#define DSC_A_rx_pi_diff_bin 0x0000D00A
#define DSC_A_trnsum_cntl_5 0x0000D00B
#define DSC_A_dsc_uc_ctrl 0x0000D00D
#define DSC_A_dsc_scratch 0x0000D00E
#define DSC_B_dsc_sm_ctrl_0 0x0000D010
#define DSC_B_dsc_sm_ctrl_1 0x0000D011
#define DSC_B_dsc_sm_ctrl_2 0x0000D012
#define DSC_B_dsc_sm_ctrl_3 0x0000D013
#define DSC_B_dsc_sm_ctrl_4 0x0000D014
#define DSC_B_dsc_sm_ctrl_5 0x0000D015
#define DSC_B_dsc_sm_ctrl_6 0x0000D016
#define DSC_B_dsc_sm_ctrl_7 0x0000D017
#define DSC_B_dsc_sm_ctrl_8 0x0000D018
#define DSC_B_dsc_sm_ctrl_9 0x0000D019
#define DSC_B_dsc_sm_status_dsc_lock 0x0000D01A
#define DSC_B_dsc_sm_status_dsc_state_one_hot 0x0000D01B
#define DSC_B_dsc_sm_status_dsc_state_eee_one_hot 0x0000D01C
#define DSC_B_dsc_sm_status_restart 0x0000D01D
#define DSC_B_dsc_sm_status_dsc_state 0x0000D01E
#define DSC_C_dfe_common_ctl 0x0000D020
#define DSC_C_dfe_1_ctl 0x0000D021
#define DSC_C_dfe_1_pat_ctl 0x0000D022
#define DSC_C_dfe_2_ctl 0x0000D023
#define DSC_C_dfe_2_pat_ctl 0x0000D024
#define DSC_C_dfe_3_ctl 0x0000D025
#define DSC_C_dfe_3_pat_ctl 0x0000D026
#define DSC_C_dfe_4_ctl 0x0000D027
#define DSC_C_dfe_4_pat_ctl 0x0000D028
#define DSC_C_dfe_5_ctl 0x0000D029
#define DSC_C_dfe_5_pat_ctl 0x0000D02A
#define DSC_C_dfe_vga_override 0x0000D02B
#define DSC_C_vga_ctl 0x0000D02C
#define DSC_C_vga_pat_eyediag_ctl 0x0000D02D
#define DSC_C_p1_frac_offs_ctl 0x0000D02E
#define DSC_D_trnsum_ctl_1 0x0000D030
#define DSC_D_trnsum_ctl_2 0x0000D031
#define DSC_D_trnsum_ctl_3 0x0000D032
#define DSC_D_trnsum_ctl_4 0x0000D033
#define DSC_D_trnsum_sts_1 0x0000D034
#define DSC_D_trnsum_sts_2 0x0000D035
#define DSC_D_trnsum_sts_3 0x0000D036
#define DSC_D_trnsum_sts_4 0x0000D037
#define DSC_D_trnsum_sts_5 0x0000D038
#define DSC_D_trnsum_sts_6 0x0000D039
#define DSC_D_vga_p1eyediag_sts 0x0000D03A
#define DSC_D_dfe_1_sts 0x0000D03B
#define DSC_D_dfe_2_sts 0x0000D03C
#define DSC_D_dfe_3_4_5_sts 0x0000D03D
#define DSC_D_vga_tap_bin 0x0000D03E
#define DSC_E_dsc_e_ctrl 0x0000D040
#define DSC_E_dsc_e_pf_ctrl 0x0000D041
#define DSC_E_dsc_e_pf2_lowp_ctrl 0x0000D042
#define DSC_E_dsc_e_offset_adj_data_odd 0x0000D043
#define DSC_E_dsc_e_offset_adj_data_even 0x0000D044
#define DSC_E_dsc_e_offset_adj_p1_odd 0x0000D045
#define DSC_E_dsc_e_offset_adj_p1_even 0x0000D046
#define DSC_E_dsc_e_offset_adj_m1_odd 0x0000D047
#define DSC_E_dsc_e_offset_adj_m1_even 0x0000D048
#define DSC_E_dsc_e_dc_offset 0x0000D049
#define DSC_F_ONU10G_looptiming_ctrl_0 0x0000D050
#define DSC_F_ONU10G_rx_signal_loss_ctrl_2 0x0000D053
#define TX_PI_LBE_tx_pi_control_0 0x0000D070
#define TX_PI_LBE_tx_pi_control_1 0x0000D071
#define TX_PI_LBE_tx_pi_control_2 0x0000D072
#define TX_PI_LBE_tx_pi_control_3 0x0000D073
#define TX_PI_LBE_tx_pi_control_4 0x0000D074
#define TX_PI_LBE_tx_pi_control_6 0x0000D076
#define TX_PI_LBE_tx_pi_status_0 0x0000D078
#define TX_PI_LBE_tx_pi_status_1 0x0000D079
#define TX_PI_LBE_tx_pi_status_2 0x0000D07A
#define TX_PI_LBE_tx_pi_status_3 0x0000D07B
#define TX_PI_LBE_tx_lbe_control_0 0x0000D07C
#define CKRST_CTRL_OSR_MODE_CONTROL 0x0000D080
#define CKRST_CTRL_LANE_CLK_RESET_N_POWERDOWN_CONTROL 0x0000D081
#define CKRST_CTRL_LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL 0x0000D082
#define CKRST_CTRL_LANE_RESET_N_PWRDN_PIN_KILL_CONTROL 0x0000D083
#define CKRST_CTRL_LANE_DEBUG_RESET_CONTROL 0x0000D084
#define CKRST_CTRL_UC_ACK_LANE_CONTROL 0x0000D085
#define CKRST_CTRL_LANE_REG_RESET_OCCURRED_CONTROL 0x0000D086
#define CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL 0x0000D087
#define CKRST_CTRL_PMD_LANE_MODE_STATUS 0x0000D088
#define CKRST_CTRL_LANE_DP_RESET_STATE_STATUS 0x0000D089
#define CKRST_CTRL_LN_MASK 0x0000D08A
#define CKRST_CTRL_OSR_MODE_STATUS 0x0000D08B
#define CKRST_CTRL_AFE_RESET_PWRDN_OSR_MODE_PIN_STATUS 0x0000D08C
#define CKRST_CTRL_PLL_SELECT_CONTROL 0x0000D08D
#define CKRST_CTRL_LN_S_RSTB_CONTROL 0x0000D08E
#define AMS_RX_RX_CONTROL_0 0x0000D090
#define AMS_RX_RX_CONTROL_1 0x0000D091
#define AMS_RX_RX_CONTROL_2 0x0000D092
#define AMS_RX_RX_CONTROL_3 0x0000D093
#define AMS_RX_RX_CONTROL_4 0x0000D094
#define AMS_RX_RX_INTCTRL 0x0000D098
#define AMS_RX_RX_STATUS 0x0000D099
#define AMS_TX_TX_CONTROL_0 0x0000D0A0
#define AMS_TX_TX_CONTROL_1 0x0000D0A1
#define AMS_TX_TX_CONTROL_2 0x0000D0A2
#define AMS_TX_TX_INTCTRL 0x0000D0A8
#define AMS_TX_TX_STATUS 0x0000D0A9
#define AMS_COM_PLL_CONTROL_0 0x0000D0B0
#define AMS_COM_PLL_CONTROL_1 0x0000D0B1
#define AMS_COM_PLL_CONTROL_2 0x0000D0B2
#define AMS_COM_PLL_CONTROL_3 0x0000D0B3
#define AMS_COM_PLL_CONTROL_4 0x0000D0B4
#define AMS_COM_PLL_CONTROL_5 0x0000D0B5
#define AMS_COM_PLL_CONTROL_6 0x0000D0B6
#define AMS_COM_PLL_CONTROL_7 0x0000D0B7
#define AMS_COM_PLL_CONTROL_8 0x0000D0B8
#define AMS_COM_PLL_INTCTRL 0x0000D0B9
#define AMS_COM_PLL_STATUS 0x0000D0BA
#define SIGDET_SIGDET_CTRL_0 0x0000D0C0
#define SIGDET_SIGDET_CTRL_1 0x0000D0C1
#define SIGDET_SIGDET_CTRL_2 0x0000D0C2
#define SIGDET_SIGDET_CTRL   0x0000D0C3
#define SIGDET_SIGDET_STATUS_0 0x0000D0C8
    #define signal_detect_raw   (1<<4)
#define TLB_RX_prbs_chk_cnt_config 0x0000D0D0
#define TLB_RX_prbs_chk_config 0x0000D0D1
#define TLB_RX_dig_lpbk_config 0x0000D0D2
    #define TLB_RX_dig_lpbk_ena (1<<0)
#define TLB_RX_tlb_rx_misc_config 0x0000D0D3
#define TLB_RX_prbs_chk_en_timer_control 0x0000D0D4
#define TLB_RX_dig_lpbk_pd_status 0x0000D0D8
#define TLB_RX_prbs_chk_lock_status 0x0000D0D9
#define TLB_RX_prbs_chk_err_cnt_msb_status 0x0000D0DA
#define TLB_RX_prbs_chk_err_cnt_lsb_status 0x0000D0DB
#define TLB_RX_pmd_rx_lock_status 0x0000D0DC
#define TLB_TX_patt_gen_config 0x0000D0E0
#define TLB_TX_prbs_gen_config 0x0000D0E1
#define TLB_TX_rmt_lpbk_config 0x0000D0E2
#define TLB_TX_tlb_tx_misc_config 0x0000D0E3
#define TLB_TX_tx_pi_loop_timing_config 0x0000D0E4
#define TLB_TX_rmt_lpbk_pd_status 0x0000D0E8
#define DIG_COM_REVID0 0x0000D0F0
#define DIG_COM_RESET_CONTROL_PMD 0x0000D0F1
#define DIG_COM_RESET_CONTROL_CORE_DP 0x0000D0F2
#define DIG_COM_DEBUG_CONTROL 0x0000D0F3
#define DIG_COM_TOP_USER_CONTROL_0 0x0000D0F4
#define DIG_COM_CORE_REG_RESET_OCCURRED_CONTROL 0x0000D0F6
#define DIG_COM_RST_SEQ_TIMER_CONTROL 0x0000D0F7
#define DIG_COM_CORE_DP_RESET_STATE_STATUS 0x0000D0F8
#define DIG_COM_REVID1 0x0000D0FA
#define DIG_COM_REVID2 0x0000D0FE
#define PATT_GEN_patt_gen_seq_0 0x0000D100
#define PATT_GEN_patt_gen_seq_1 0x0000D101
#define PATT_GEN_patt_gen_seq_2 0x0000D102
#define PATT_GEN_patt_gen_seq_3 0x0000D103
#define PATT_GEN_patt_gen_seq_4 0x0000D104
#define PATT_GEN_patt_gen_seq_5 0x0000D105
#define PATT_GEN_patt_gen_seq_6 0x0000D106
#define PATT_GEN_patt_gen_seq_7 0x0000D107
#define PATT_GEN_patt_gen_seq_8 0x0000D108
#define PATT_GEN_patt_gen_seq_9 0x0000D109
#define PATT_GEN_patt_gen_seq_10 0x0000D10A
#define PATT_GEN_patt_gen_seq_11 0x0000D10B
#define PATT_GEN_patt_gen_seq_12 0x0000D10C
#define PATT_GEN_patt_gen_seq_13 0x0000D10D
#define PATT_GEN_patt_gen_seq_14 0x0000D10E
#define TX_FED_txfir_control1 0x0000D110
#define TX_FED_txfir_control2 0x0000D111
#define TX_FED_txfir_control3 0x0000D112
#define TX_FED_txfir_status1 0x0000D113
#define TX_FED_txfir_status2 0x0000D114
#define TX_FED_txfir_status3 0x0000D115
#define TX_FED_txfir_status4 0x0000D116
#define TX_FED_micro_control 0x0000D117
#define TX_FED_misc_control1 0x0000D118
#define TX_FED_txfir_control4 0x0000D119
#define TX_FED_misc_status0 0x0000D11B
#define PLL_CAL_COM_CTL_0 0x0000D120
#define PLL_CAL_COM_CTL_1 0x0000D121
#define PLL_CAL_COM_CTL_2 0x0000D122
#define PLL_CAL_COM_CTL_3 0x0000D123
#define PLL_CAL_COM_CTL_4 0x0000D124
#define PLL_CAL_COM_CTL_5 0x0000D125
#define PLL_CAL_COM_CTL_6 0x0000D126
#define PLL_CAL_COM_CTL_7 0x0000D127
#define PLL_CAL_COM_CTL_STATUS_0 0x0000D128
    #define PLL_CAL_COM_CTL_STATUS_pll_low (1<<9)
#define PLL_CAL_COM_CTL_STATUS_1 0x0000D129
#define TXCOM_CL72_tap_preset_control 0x0000D132
#define TXCOM_CL72_debug_1_register 0x0000D133
#define CORE_PLL_COM_PMD_CORE_MODE_STATUS 0x0000D150
#define CORE_PLL_COM_RESET_CONTROL_PLL_DP 0x0000D152
#define CORE_PLL_COM_TOP_USER_CONTROL 0x0000D154
#define CORE_PLL_COM_UC_ACK_CORE_CONTROL 0x0000D155
#define CORE_PLL_COM_PLL_DP_RESET_STATE_STATUS 0x0000D158
#define CORE_PLL_COM_CORE_PLL_COM_STATUS_2 0x0000D159
#define MICRO_A_ramword 0x0000D200
#define MICRO_A_address 0x0000D201
#define MICRO_A_command 0x0000D202
#define MICRO_A_ram_wrdata 0x0000D203
#define MICRO_A_ram_rddata 0x0000D204
#define MICRO_A_download_status 0x0000D205
#define MICRO_A_sfr_status 0x0000D206
#define MICRO_A_mdio_uc_mailbox_msw 0x0000D207
#define MICRO_A_mdio_uc_mailbox_lsw 0x0000D208
#define MICRO_A_uc_mdio_mailbox_lsw 0x0000D209
#define MICRO_A_command2 0x0000D20A
#define MICRO_A_uc_mdio_mailbox_msw 0x0000D20B
#define MICRO_A_command3 0x0000D20C
#define MICRO_A_command4 0x0000D20D
#define MICRO_A_temperature_status 0x0000D20E
#define MICRO_B_program_ram_control1 0x0000D210
#define MICRO_B_dataram_control1 0x0000D214
#define MICRO_B_iram_control1 0x0000D218
#define MDIO_MMDSEL_AER_COM_mdio_maskdata 0x0000FFDB
#define MDIO_MMDSEL_AER_COM_mdio_brcst_port_addr 0x0000FFDC
#define MDIO_MMDSEL_AER_COM_mdio_mmd_select 0x0000FFDD
#define MDIO_MMDSEL_AER_COM_mdio_aer 0x0000FFDE
#define MDIO_BLK_ADDR_BLK_ADDR 0x0000FFDF

#define XGXSBLK0_XGXSCTRL 0x00008000
#define XGXSBLK1_LANECTRL0 0x00008015
#define XGXSBLK1_LANECTRL1 0x00008016
#define SerdesDigital_misc1 0x00008308
#define Digital4_rp_nextPage_0   0x0008337
    #define Digital4_RemotePhyEnable        (1<<11)
#define Digital4_rp_nextPage_1   0x0008338
    #define Digital4_RemotePhyDecodeEnable  (1<<11)
#define Digital4_Misc3      0x000833c
#define Digital4_Misc4 0x0000833d
#define Digital5_parDetINDControl1 0x00008347
#define Digital5_parDetINDControl2 0x00008348
#define Digital5_Misc7 0x00008349
#define Digital5_Misc6 0x00008345
#define CL49_UserB0_Control1  0x8360
    #define CL49_UserB0_scramblerControl_M  (0x3<<2)
    #define CL49_UserB0_bypassScrambler     (0x0<<2)
    #define CL49_UserB0_payloadScramble     (0x2<<2)
#define CL49_UserB0_rxsm_status  0x8366
    #define r_type_coded_M  (0xf<<12)
        #define R_TYPE_E        (0<<12)
        #define R_TYPE_C        (4<<12)
        #define R_TYPE_BAD      (0xf<<12)
    #define lrxsm_state_RX_C    (1<<4)
    #define RXSM_STATE_M        (0x7<<0)
        #define RXSM_STATE_C    (1<<0)
        #define RXSM_STATE_D    (2<<0)
        #define RXSM_STATE_E    (4<<0)
#define CL49_UserB0_Control     0x8368
    #define CL49_fast_lock_cya  (1<<5)
#define tx66_Control 0x000083b0
    #define tfifo_ptr_sw_rst    (1<<6)
#define FX100_Control3 0x00008402
#define ieee0Blk_MIICntl 0x00000000
#define ieee5Blk_AnLinkParterAbi 5
    #define ieee5_SgmiiMode (1<<0)
    #define ieee5_SgmiiLink (1<<15)
    #define ieee5_SgmiiSpeedMask    (3<<10)
    #define ieee5_SgmiiSpeed1G      (2<<10)
    #define ieee5_SgmiiSpeed100M    (1<<10)
    #define ieee5_SgmiiSpeed10M     (0<<10)
    #define ieee5_SgmiiDuplex       (1<<12)
    
#define FX100_Control1 0x00008400
#define rx66b1_rx66b1_Control1 0x8441
    #define rfifo_ptr_sw_rst    (1<<0)
#define XGXSBLK4_xgxsStatus1 0x00008122
#define XgxsStatus1_LinkStat    (1<<9)
#define XgxsStatus1_Speed_Mask  (0xf)
#define XgxsStatus1_Speed_10G   (0x6)

#define SerdesDigital_Control1000X1 0x8300
    #define SerdesDigital_SgmiiMasterMode (1<<5)
    #define SerdesDigital_SgmiiAutoMode (1<<4)
    #define SerdesDigital_FibreSgmiiModeFibre (1<<0)
#define SerdesDigital_Control1000X2 0x8301
    #define SerdesDigital_AutoNegoFastTimer (1<<6)
    #define SerdesDigital_EnableParallelDetection (1<<0)
#define SerdesDigital_Control1000X3 0x8302

#define set_mask_read_data(d) udelay(1000)
#define clr_mask_read_data() udelay(1000)
#define LOCK_POLL_ENABLE

#define serdes_wait_nsec(nsec) {if(nsec<1000) ndelay(nsec); \
        else {if(nsec<1000000) udelay(nsec/1000); \
        else mdelay(nsec/1000000);} }

#define XgaeSerdesOpWait(pcs, v) do { \
    int d = 1000000; \
    for (; d; d--) { \
        REG_READ_32(pcs?&WAN_TOP->WAN_TOP_PMI_LP_4:&WAN_TOP->WAN_TOP_PMI_LP_3, (v)); \
        if ((v) & PMI_LP3or4_PCS_PMI_LP_ACK) break; \
    } \
    if (!((v) & PMI_LP3or4_PCS_PMI_LP_ACK)) {return -1; } \
    if ((v) & PMI_LP3or4_PCS_PMI_LP_ERR) {return -2;} \
    (v) = (v) & PMI_LP3or4_PCS_PMI_LP_DATA_MASK; \
} while(0)

#if defined(PHY_SERDES_10G_CAPABLE)
static inline uint32_t _writeXgaeSerdesReg(uint32_t pcs, uint32_t addr, uint32_t value, uint32_t mask)
{
    uint32_t ret;
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_1, addr);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_2, ((value)<<16)|((mask) & 0xffff));
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_0, pcs? PMI_LP0_PCS_WR:PMI_LP0_SERDES_WR);
    XgaeSerdesOpWait(pcs, ret);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_0, 0x0);
    udelay(5);
    return 0;
}

#define writeXgaeSerdesReg(addr, val) _writeXgaeSerdesReg(0, addr, ((val)>>16)&0xffff, (val)&0xffff)
#define writeXgaePcsReg(addr, val) _writeXgaeSerdesReg(1, addr, ((val)>>16)&0xffff, (val)&0xffff)

static inline int _readXgaeSerdesReg(uint32_t pcs, uint32_t addr, uint32_t *val)
{
    /* Work around for hardware bug on PHY ID on PCS register space, they have no plan to fix this */
    if (pcs && ((addr & 0xffff) == MII_DEVID1 || (addr & 0xffff) == MII_DEVID2)) {
        REG_READ_32(&WAN_TOP->WAN_AEPCS_IEEE_REGID, (*val));
        *val = addr==MII_DEVID2? (((*val)>>16)&0xffff): ((*val)&0xffff); 
        return 0;
    }

    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_1, addr);
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_0, pcs?PMI_LP0_PCS_RD:PMI_LP0_SERDES_RD);     
    XgaeSerdesOpWait(pcs, (*val));
    REG_WRITE_32(&WAN_TOP->WAN_TOP_PMI_LP_0, 0x0);
    udelay(5);
    return 0;
}

static inline uint32_t rd_xgae_serdes_reg(int serdes_inst, uint32_t addr) 
{uint32_t val; _readXgaeSerdesReg(serdes_inst==SERDES_1, addr, &val); return val;}

static inline int rd_ver_xgae_serdes_reg (uint32_t serdes_inst, uint32_t addr, uint16_t val, uint16_t mask)
{
    uint16_t rdval = rd_xgae_serdes_reg (serdes_inst, addr);
    mask = (~mask) & 0xffff;

    if ((rdval & mask) != (val & mask)) {
        printk("*** Read Compare Error: Serdes: %d, addr: 0x%08x, rdval: 0x%04x, expVal: 0x%04x, mask: 0x%08x\n",
                serdes_inst, addr, rdval, val, mask);
        return 0;
    }
    return 1;
}
#endif

#define readXgaeSerdesReg(addr, ret) _readXgaeSerdesReg(0, addr, ret)
#define readXgaePcsReg(addr, ret) _readXgaeSerdesReg(1, addr, ret)

#define wr_xgae_serdes_reg(serdes_inst, addr, wr_data, mask_data) \
    _writeXgaeSerdesReg(serdes_inst==SERDES_1, addr, wr_data, mask_data)
void ethsw_xgae_serdes_link_stats(phy_dev_t *phy_dev);
int sf2_xgae_serdes_init(phy_dev_t *phy_dev);
int ethsw_xgae_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex);
int ethsw_xgae_enable_an(phy_dev_t *phy_dev);

#endif 
