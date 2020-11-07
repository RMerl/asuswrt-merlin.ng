/*
  <:copyright-BRCM:2015:proprietary:standard
  
     Copyright (c) 2015 Broadcom 
     All Rights Reserved
  
   This program is the proprietary software of Broadcom and/or its
   licensors, and may only be used, duplicated, modified or distributed pursuant
   to the terms and conditions of a separate, written license agreement executed
   between you and Broadcom (an "Authorized License").  Except as set forth in
   an Authorized License, Broadcom grants no license (express or implied), right
   to use, or waiver of any kind with respect to the Software, and Broadcom
   expressly reserves all rights in and to the Software and all intellectual
   property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
   NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
   BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
   Except as expressly set forth in the Authorized License,
  
   1. This program, including its structure, sequence and organization,
      constitutes the valuable trade secrets of Broadcom, and you shall use
      all reasonable efforts to protect the confidentiality thereof, and to
      use this information only in connection with your use of Broadcom
      integrated circuit products.
  
   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
      AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
      WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
      RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
      ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
      FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
      COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
      TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
      PERFORMANCE OF THE SOFTWARE.
  
   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
      ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
      INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
      WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
      IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
      OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
      SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
      SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
      LIMITED REMEDY.
  :>
*/

/****************************************************************************
 *
 * pon_drv_serdes_util.h -- common head file for all wan type serdes init sequence
 *
 * Description:
 *      provide WAN_TOP and Serdes register access directly
 *  constant definitions
 *  declaration of 'pon_drv_serdes_util.c' functions 
 *
 * Authors: Akiva Sadovski, Vitaly Zborovski
 *
 * $Revision: 1.1 $
 *
 * 2017.July: updated by VZ
 *****************************************************************************/

#include "access_macros.h"
#include "ru_types.h"
#include "bcm_map_part.h"

extern const ru_block_rec *WAN_TOP_BLOCKS[];

// Updated by Akiva for 63158 support
#define WAN_PHYS_BASE_OFFSET (WAN_PHYS_BASE & 0x0FFF)
#define WAN_TOP_VIRTUAL_BASE ((long unsigned int) WAN_TOP_BLOCKS[0]->addr[0])
#define WAN_TOP_WRITE_32(off, r) { uint32_t val = r; unsigned long _a = (WAN_TOP_VIRTUAL_BASE + (off - WAN_PHYS_BASE_OFFSET)); WRITE_32(_a, val); }
#define WAN_TOP_READ_32(off, r) READ_32(WAN_TOP_VIRTUAL_BASE + (off - WAN_PHYS_BASE_OFFSET), r)


/* Constant Definitions */ 
#define    SERDES_DEBUG        0

#define    PPM_TARGET     10       //
#define    PPM_AVG_NUM    10000    //
#define    PPM_UNSTABLE   1        // 
#define    PPM_FAULT      0x7FFF   //  
#define    PPM_FACTOR     (670/10) // rx_ppm = cdr_integ/67.1
#define    PPM_GOOD_LMT   5        // ppm-limit to verify rx-input-signal is stable
#define    PPM_MAX_LMT    100      // maximum input-ppm for correction

#define    FRACN_GOOD_LMT 320      // change of +/-320[dec] in FracN is equal to ~ 6ppm. To avoid unnececary correction  
#define    G_FRACN_HIGH   0x05800  // ITU.T 10G PLL: max allowed FracN value 0x5800 -> is +100ppm 
#define    G_FRACN_LOW    0x02F00  // ITU.T 10G PLL: min allowed FracN value 0x2500 -> is -100ppm 

#define    E_FRACN_HIGH   0x11500  // IEEE 10G PLL: max allowed FracN value 0x11500 -> is +100ppm 
#define    E_FRACN_LOW    0x0EB00  // IEEE 10G PLL: min allowed FracN value 0x0EB00 -> is -100ppm 

#define    CDR_INTEG_AVRG_TIME 10

// standards : G = ITU.T or E = IEEE 
#define    STD_E    0x1
#define    STD_G    0x2
#define    STD_G__SubRate    0x3

// PLL modes
#define    PADREF   0x1
#define    LCREF    0x2

#define    ONE_PLL  0x1
#define    TWO_PLLS 0x2

/***************************************************************************
// Serdes registers in full chip address map
 ***************************************************************************/
#define    PCS_ID       0x0000

#define    DEVID_0      0x0000
#define    DEVID_1      0x0800

#define    serdes_PLL_0    0x0000
#define    serdes_PLL_1    0x0100

#define    LANE_0       0x0000
#define    LANE_1       0x0001
#define    LANE_2       0x0002
#define    LANE_3       0x0003
#define    LANE_BRDCST  0x00FF
#define    LANE_01      0x0200
#define    LANE_23      0x0201

#define DSC_A_cdr_control_0 0xD001
#define DSC_A_cdr_control_1 0xD002
#define DSC_A_cdr_control_2 0xD003
#define DSC_A_rx_pi_control 0xD004
#define DSC_A_cdr_status_integ_reg 0xD005
#define DSC_A_cdr_status_phase_error 0xD006
#define DSC_A_rx_pi_cnt_bin_d 0xD007
#define DSC_A_rx_pi_cnt_bin_p 0xD008
#define DSC_A_rx_pi_cnt_bin_m 0xD009
#define DSC_A_rx_pi_diff_bin 0xD00A
#define DSC_A_trnsum_cntl_5 0xD00B
#define DSC_A_dsc_uc_ctrl 0xD00D
#define DSC_A_dsc_scratch 0xD00E
#define DSC_B_dsc_sm_ctrl_0 0xD010
#define DSC_B_dsc_sm_ctrl_1 0xD011
#define DSC_B_dsc_sm_ctrl_2 0xD012
#define DSC_B_dsc_sm_ctrl_3 0xD013
#define DSC_B_dsc_sm_ctrl_4 0xD014
#define DSC_B_dsc_sm_ctrl_5 0xD015
#define DSC_B_dsc_sm_ctrl_6 0xD016
#define DSC_B_dsc_sm_ctrl_7 0xD017
#define DSC_B_dsc_sm_ctrl_8 0xD018
#define DSC_B_dsc_sm_ctrl_9 0xD019
#define DSC_B_dsc_sm_status_dsc_lock 0xD01A
#define DSC_B_dsc_sm_status_dsc_state_one_hot 0xD01B
#define DSC_B_dsc_sm_status_dsc_state_eee_one_hot 0xD01C
#define DSC_B_dsc_sm_status_restart 0xD01D
#define DSC_B_dsc_sm_status_dsc_state 0xD01E
#define DSC_C_dfe_common_ctl 0xD020
#define DSC_C_dfe_1_ctl 0xD021
#define DSC_C_dfe_1_pat_ctl 0xD022
#define DSC_C_dfe_2_ctl 0xD023
#define DSC_C_dfe_2_pat_ctl 0xD024
#define DSC_C_dfe_3_ctl 0xD025
#define DSC_C_dfe_3_pat_ctl 0xD026
#define DSC_C_dfe_4_ctl 0xD027
#define DSC_C_dfe_4_pat_ctl 0xD028
#define DSC_C_dfe_5_ctl 0xD029
#define DSC_C_dfe_5_pat_ctl 0xD02A
#define DSC_C_dfe_vga_override 0xD02B
#define DSC_C_vga_ctl 0xD02C
#define DSC_C_vga_pat_eyediag_ctl 0xD02D
#define DSC_C_p1_frac_offs_ctl 0xD02E
#define DSC_D_trnsum_ctl_1 0xD030
#define DSC_D_trnsum_ctl_2 0xD031
#define DSC_D_trnsum_ctl_3 0xD032
#define DSC_D_trnsum_ctl_4 0xD033
#define DSC_D_trnsum_sts_1 0xD034
#define DSC_D_trnsum_sts_2 0xD035
#define DSC_D_trnsum_sts_3 0xD036
#define DSC_D_trnsum_sts_4 0xD037
#define DSC_D_trnsum_sts_5 0xD038
#define DSC_D_trnsum_sts_6 0xD039
#define DSC_D_vga_p1eyediag_sts 0xD03A
#define DSC_D_dfe_1_sts 0xD03B
#define DSC_D_dfe_2_sts 0xD03C
#define DSC_D_dfe_3_4_5_sts 0xD03D
#define DSC_D_vga_tap_bin 0xD03E
#define DSC_E_dsc_e_ctrl 0xD040
#define DSC_E_dsc_e_pf_ctrl 0xD041
#define DSC_E_dsc_e_pf2_lowp_ctrl 0xD042
#define DSC_E_dsc_e_offset_adj_data_odd 0xD043
#define DSC_E_dsc_e_offset_adj_data_even 0xD044
#define DSC_E_dsc_e_offset_adj_p1_odd 0xD045
#define DSC_E_dsc_e_offset_adj_p1_even 0xD046
#define DSC_E_dsc_e_offset_adj_m1_odd 0xD047
#define DSC_E_dsc_e_offset_adj_m1_even 0xD048
#define DSC_E_dsc_e_dc_offset 0xD049
#define DSC_F_ONU10G_looptiming_ctrl_0 0xD050
#define TX_PI_LBE_tx_pi_control_0 0xD070
#define TX_PI_LBE_tx_pi_control_1 0xD071
#define TX_PI_LBE_tx_pi_control_2 0xD072
#define TX_PI_LBE_tx_pi_control_3 0xD073
#define TX_PI_LBE_tx_pi_control_4 0xD074
#define TX_PI_LBE_tx_pi_control_6 0xD076
#define TX_PI_LBE_tx_pi_status_0 0xD078
#define TX_PI_LBE_tx_pi_status_1 0xD079
#define TX_PI_LBE_tx_pi_status_2 0xD07A
#define TX_PI_LBE_tx_pi_status_3 0xD07B
#define TX_PI_LBE_tx_lbe_control_0 0xD07C
#define CKRST_CTRL_OSR_MODE_CONTROL 0xD080
#define CKRST_CTRL_LANE_CLK_RESET_N_POWERDOWN_CONTROL 0xD081
#define CKRST_CTRL_LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL 0xD082
#define CKRST_CTRL_LANE_RESET_N_PWRDN_PIN_KILL_CONTROL 0xD083
#define CKRST_CTRL_LANE_DEBUG_RESET_CONTROL 0xD084
#define CKRST_CTRL_UC_ACK_LANE_CONTROL 0xD085
#define CKRST_CTRL_LANE_REG_RESET_OCCURRED_CONTROL 0xD086
#define CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL 0xD087
#define CKRST_CTRL_PMD_LANE_MODE_STATUS 0xD088
#define CKRST_CTRL_LANE_DP_RESET_STATE_STATUS 0xD089
#define CKRST_CTRL_LN_MASK 0xD08A
#define CKRST_CTRL_OSR_MODE_STATUS 0xD08B
#define CKRST_CTRL_AFE_RESET_PWRDN_OSR_MODE_PIN_STATUS 0xD08C
#define CKRST_CTRL_PLL_SELECT_CONTROL 0xD08D
#define CKRST_CTRL_LN_S_RSTB_CONTROL 0xD08E
#define AMS_RX_RX_CONTROL_0 0xD090
#define AMS_RX_RX_CONTROL_1 0xD091
#define AMS_RX_RX_CONTROL_2 0xD092
#define AMS_RX_RX_CONTROL_3 0xD093
#define AMS_RX_RX_CONTROL_4 0xD094
#define AMS_RX_RX_INTCTRL 0xD098
#define AMS_RX_RX_STATUS 0xD099
#define AMS_TX_TX_CONTROL_0 0xD0A0
#define AMS_TX_TX_CONTROL_1 0xD0A1
#define AMS_TX_TX_CONTROL_2 0xD0A2
#define AMS_TX_TX_INTCTRL 0xD0A8
#define AMS_TX_TX_STATUS 0xD0A9
#define AMS_COM_PLL_CONTROL_0 0xD0B0
#define AMS_COM_PLL_CONTROL_1 0xD0B1
#define AMS_COM_PLL_CONTROL_2 0xD0B2
#define AMS_COM_PLL_CONTROL_3 0xD0B3
#define AMS_COM_PLL_CONTROL_4 0xD0B4
#define AMS_COM_PLL_CONTROL_5 0xD0B5
#define AMS_COM_PLL_CONTROL_6 0xD0B6
#define AMS_COM_PLL_CONTROL_7 0xD0B7
#define AMS_COM_PLL_CONTROL_8 0xD0B8
#define AMS_COM_PLL_INTCTRL 0xD0B9
#define AMS_COM_PLL_STATUS 0xD0BA
#define SIGDET_SIGDET_CTRL_0 0xD0C0
#define SIGDET_SIGDET_CTRL_1 0xD0C1
#define SIGDET_SIGDET_CTRL_2 0xD0C2
#define SIGDET_SIGDET_CTRL_3 0xD0C3
#define SIGDET_SIGDET_STATUS_0 0xD0C8
#define TLB_RX_prbs_chk_cnt_config 0xD0D0
#define TLB_RX_prbs_chk_config 0xD0D1
#define TLB_RX_dig_lpbk_config 0xD0D2
#define TLB_RX_tlb_rx_misc_config 0xD0D3
#define TLB_RX_prbs_chk_en_timer_control 0xD0D4
#define TLB_RX_dig_lpbk_pd_status 0xD0D8
#define TLB_RX_prbs_chk_lock_status 0xD0D9
#define TLB_RX_prbs_chk_err_cnt_msb_status 0xD0DA
#define TLB_RX_prbs_chk_err_cnt_lsb_status 0xD0DB
#define TLB_RX_pmd_rx_lock_status 0xD0DC
#define TLB_TX_patt_gen_config 0xD0E0
#define TLB_TX_prbs_gen_config 0xD0E1
#define TLB_TX_rmt_lpbk_config 0xD0E2
#define TLB_TX_tlb_tx_misc_config 0xD0E3
#define TLB_TX_tx_pi_loop_timing_config 0xD0E4
#define TLB_TX_rmt_lpbk_pd_status 0xD0E8
#define DIG_COM_REVID0 0xD0F0
#define DIG_COM_RESET_CONTROL_PMD 0xD0F1
#define DIG_COM_RESET_CONTROL_CORE_DP 0xD0F2
#define DIG_COM_DEBUG_CONTROL 0xD0F3
#define DIG_COM_TOP_USER_CONTROL_0 0xD0F4
#define DIG_COM_CORE_REG_RESET_OCCURRED_CONTROL 0xD0F6
#define DIG_COM_RST_SEQ_TIMER_CONTROL 0xD0F7
#define DIG_COM_CORE_DP_RESET_STATE_STATUS 0xD0F8
#define DIG_COM_REVID1 0xD0FA
#define DIG_COM_REVID2 0xD0FE
#define PATT_GEN_patt_gen_seq_0 0xD100
#define PATT_GEN_patt_gen_seq_1 0xD101
#define PATT_GEN_patt_gen_seq_2 0xD102
#define PATT_GEN_patt_gen_seq_3 0xD103
#define PATT_GEN_patt_gen_seq_4 0xD104
#define PATT_GEN_patt_gen_seq_5 0xD105
#define PATT_GEN_patt_gen_seq_6 0xD106
#define PATT_GEN_patt_gen_seq_7 0xD107
#define PATT_GEN_patt_gen_seq_8 0xD108
#define PATT_GEN_patt_gen_seq_9 0xD109
#define PATT_GEN_patt_gen_seq_10 0xD10A
#define PATT_GEN_patt_gen_seq_11 0xD10B
#define PATT_GEN_patt_gen_seq_12 0xD10C
#define PATT_GEN_patt_gen_seq_13 0xD10D
#define PATT_GEN_patt_gen_seq_14 0xD10E
#define TX_FED_txfir_control1 0xD110
#define TX_FED_txfir_control2 0xD111
#define TX_FED_txfir_control3 0xD112
#define TX_FED_txfir_status1 0xD113
#define TX_FED_txfir_status2 0xD114
#define TX_FED_txfir_status3 0xD115
#define TX_FED_txfir_status4 0xD116
#define TX_FED_micro_control 0xD117
#define TX_FED_misc_control1 0xD118
#define TX_FED_txfir_control4 0xD119
#define TX_FED_misc_status0 0xD11B
#define PLL_CAL_COM_CTL_0 0xD120
#define PLL_CAL_COM_CTL_1 0xD121
#define PLL_CAL_COM_CTL_2 0xD122
#define PLL_CAL_COM_CTL_3 0xD123
#define PLL_CAL_COM_CTL_4 0xD124
#define PLL_CAL_COM_CTL_5 0xD125
#define PLL_CAL_COM_CTL_6 0xD126
#define PLL_CAL_COM_CTL_7 0xD127
#define PLL_CAL_COM_CTL_STATUS_0 0xD128
#define PLL_CAL_COM_CTL_STATUS_1 0xD129
#define TXCOM_CL72_tap_preset_control 0xD132
#define TXCOM_CL72_debug_1_register 0xD133
#define CORE_PLL_COM_PMD_CORE_MODE_STATUS 0xD150
#define CORE_PLL_COM_RESET_CONTROL_PLL_DP 0xD152
#define CORE_PLL_COM_TOP_USER_CONTROL 0xD154
#define CORE_PLL_COM_UC_ACK_CORE_CONTROL 0xD155
#define CORE_PLL_COM_PLL_DP_RESET_STATE_STATUS 0xD158
#define CORE_PLL_COM_CORE_PLL_COM_STATUS_2 0xD159
#define MICRO_A_ramword 0xD200
#define MICRO_A_address 0xD201
#define MICRO_A_command 0xD202
#define MICRO_A_ram_wrdata 0xD203
#define MICRO_A_ram_rddata 0xD204
#define MICRO_A_download_status 0xD205
#define MICRO_A_sfr_status 0xD206
#define MICRO_A_mdio_uc_mailbox_msw 0xD207
#define MICRO_A_mdio_uc_mailbox_lsw 0xD208
#define MICRO_A_uc_mdio_mailbox_lsw 0xD209
#define MICRO_A_command2 0xD20A
#define MICRO_A_uc_mdio_mailbox_msw 0xD20B
#define MICRO_A_command3 0xD20C
#define MICRO_A_command4 0xD20D
#define MICRO_A_temperature_status 0xD20E
#define MICRO_B_program_ram_control1 0xD210
#define MICRO_B_dataram_control1 0xD214
#define MICRO_B_iram_control1 0xD218
#define MDIO_MMDSEL_AER_COM_mdio_maskdata 0xFFDB
#define MDIO_MMDSEL_AER_COM_mdio_brcst_port_addr 0xFFDC
#define MDIO_MMDSEL_AER_COM_mdio_mmd_select 0xFFDD
#define MDIO_MMDSEL_AER_COM_mdio_aer 0xFFDE
#define MDIO_BLK_ADDR_BLK_ADDR 0xFFDF

#define XGXSBLK0_XGXSCTRL 0x8000
#define XGXSBLK1_LANECTRL0 0x8015
#define XGXSBLk1_LANECTRL1 0x8016
#define SerdesDigital_misc1 0x8308
#define Digital4_Misc3      0x33c
#define Digital4_Misc4 0x833d
#define Digital5_parDetINDControl1 0x8347
#define Digital5_parDetINDControl2 0x8348
#define Digital5_Misc7 0x8349
#define Digital5_Misc6 0x8345
#define tx66_Control 0x83b0
#define FX100_Control3 0x8402
#define ieee0Blk_MIICntl 0x0000
#define FX100_Control1 0x8400

/* SerDes/Gearbox utility functions defined in 'pon_drv_serdes_util.c' file */
uint16_t readPONSerdesReg(uint16_t lane, uint16_t address);
void     writePONSerdesReg(uint16_t lane, uint16_t address, uint16_t value, uint16_t mask); 

uint16_t PCSreadSerdes(uint16_t lane, uint16_t address);
void     PCSwriteSerdes(uint16_t lane, uint16_t address, uint16_t value, uint16_t mask);

void     set_clk90_offset(uint8_t desired_m1_d_offset);
void     set_clkp1_offset(uint8_t desired_p_d_offset);
void     rx_pi_spacing(uint8_t desired_m1_d_offset, uint8_t desired_p_d_offset);

void     sgb_rescal_init(void);
void     rx_sigdet_dis(void);
void     set_pll_refclk(uint8_t pll_refclk);

void     serdes_reg_dump(void);
void    gearbox_reg_dump(void);

typedef struct pon_params_s
{
    uint8_t mode;
    uint8_t std;
    uint8_t refclk;
    uint8_t pll_use;
    uint8_t clk90_offset;
    uint8_t p1_offset;
    uint16_t rx_tx_rate_ratio;
} pon_params_t;

#define DATA_FRACN "/data/pll_ppm_adj"
int kerSysFsFileGet(const char *filename, char *buf, int len);
int kerSysFsFileSet(const char *filename, char *buf, int len);
int get_pll_fracn_from_flash(int *div, int *ndiv);
void set_pll_fracn_to_flash(int div, int ndiv);

