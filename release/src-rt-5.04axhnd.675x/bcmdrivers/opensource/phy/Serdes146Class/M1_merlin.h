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

// Blackfin: 16nm_Merlin_PHY_Core.pdf Version 1.7 (June 9, 2017)
// Maple2  : 16nm_Merlin_PHY_Core.pdf Version 1.1 (March 2, 2018)
// MerlinU : 16nm_MERLINU_PHY_4_v0.10.pdf Version 0.10 (May 18, 2018)

/*
Table format:
"register name", "devad", "register address", "bit mask", "value"
*/

/* -----------------------------------------------------------------
   PMD section  - the PMD design is from maple2 (same as blackfin)
-------------------------------------------------------------------- */

enum {SEQ_TYPE_REG, SEQ_TYPE_FUN, SEQ_TYPE_NEST_SEQ, SEQ_TYPE_MASK=0x3};
enum {SEQ_TYPE_FUN_ARG_NONE=0, SEQ_FUN_ARG_CORE};
void merlin_chk_pll_lock (uint32 CoreNum);
#define SEQ_TYPE_FUNC_SET(x) (char *)((uint64_t)(x) + SEQ_TYPE_FUN)
#define SEQ_TYPE_FUNC_GET(x, t) (t)((uint64_t)(x) & (~((uint64_t)SEQ_TYPE_MASK)))
#define SEQ_TYPE_NSEQ_SET(x) (char *)((uint64_t)(x) + SEQ_TYPE_NEST_SEQ)
#define SEQ_TYPE_NSEQ_GET(x) (prog_seq_tbl *)((uint64_t)(x) & (~((uint64_t)SEQ_TYPE_MASK)))
#define SEQ_TYPE_GET(x) ((uint64_t)(x) & SEQ_TYPE_MASK)

// Figure 47 Datapath Reset, (per lane) (AL: active low, wirte 1'b0 will reset the datapth for a lane if asserted
// Changed for MERLIN16
prog_seq_tbl datapath_reset_lane[] = {
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xD081, 0x0001, 0x0001},  //00:00 ln_dp_s_rstb = 1'b1; clear active low reset
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 18 Initialize (both blackfin and maple2)
prog_seq_tbl Initialize_12p5_VCO[] = {
  {"main control register", 0x3, 0x9201, 0xffff, 0x4888},  //os_mode_cl36 [15:00]
  {"main control register", 0x3, 0x9202, 0xffff, 0x9940},  //pmd_osr_mode [15:00]
  {"main control register", 0x3, 0x926A, 0x3fff, 0x0019},  //reg1G_ClockCount0 [13:00]
  {"main control register", 0x3, 0x926B, 0x1fff, 0x0006},  //reg1G_CGC [12:00]
  {"main control register", 0x3, 0x926C, 0x00ff, 0x0000},  //reg1G_ClockCount1 [07:00]
  {"main control register", 0x3, 0x926C, 0xff00, 0x0100},  //reg1G_loopcnt0 [15:08]
  {"main control register", 0x3, 0x926D, 0x1fff, 0x0000},  //reg1G_loopcnt1 [12:00]
  {"main control register", 0x3, 0x9232, 0x01ff, 0x0005},  //reg1G_modulo [08:00]

  {"main control register", 0x3, 0x9265, 0x3fff, 0x007D},  //reg100M_ClockCount0 [13:00]
  {"main control register", 0x3, 0x9268, 0x1fff, 0x001f},  //reg100M_CGC [12:00]
  {"main control register", 0x3, 0x9266, 0x00ff, 0x0000},  //reg100M_ClockCount1 [07:00]
  {"main control register", 0x3, 0x9266, 0xff00, 0x0100},  //reg100M_loopcnt0 [15:08]
  {"main control register", 0x3, 0x9269, 0xe000, 0x0000},  //reg100M_loopcnt1_Hi [15:13]
  {"main control register", 0x3, 0x9268, 0xe000, 0x0000},  //reg100M_loopcnt1_Lo [15:13]
  {"main control register", 0x3, 0x9267, 0x3fff, 0x0032},  //reg100M_PCS_ClockCount1 [13:00]
  {"main control register", 0x3, 0x9269, 0x1fff, 0x0031},  //reg100M_PCS_CGC [12:00]
  {"main control register", 0x3, 0x9231, 0x01ff, 0x001F},  //reg100M_modulo [08:00]

  {"main control register", 0x3, 0x9260, 0x3fff, 0x0271},  //reg10M_ClockCount0 [13:00]
  {"main control register", 0x3, 0x9263, 0x1fff, 0x0138},  //reg10M_CGC [12:00]
  {"main control register", 0x3, 0x9261, 0x00ff, 0x0000},  //reg10M_ClockCount1 [07:00]
  {"main control register", 0x3, 0x9261, 0xff00, 0x0100},  //reg10M_loopcnt0 [15:08]
  {"main control register", 0x3, 0x9264, 0xe000, 0x0000},  //reg10M_loopcnt1_Hi [15:13]
  {"main control register", 0x3, 0x9263, 0xe000, 0x0000},  //reg10M_loopcnt1_Lo [15:13]
  {"main control register", 0x3, 0x9262, 0x3fff, 0x0032},  //reg10M_PCS_ClockCount1 [13:00]
  {"main control register", 0x3, 0x9264, 0x1fff, 0x0031},  //reg10M_PCS_CGC [12:00]
  {"main control register", 0x3, 0x9230, 0x01ff, 0x0138},  //reg10M_modulo [08:00]
  {"",                      0x0, 0x0000, 0x0000, 0x0000}
}; 

//x Figure 19 PMD Setup 400MHz (div2 = 0) (maple2; blackfin is outdated)
prog_seq_tbl PMD_setup_400_10p3125_VCO[] = {
  {"main control register",      0x3,  0x9100, 0xf000, 0x5000}, //refclk_sel [15:12]
  {"digcom_top_user_control_0",  0x1,  0xD0F4, 0x2000, 0x0000}, //core_dp_s_rstb [13]
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x03FF, 0x0019}, // ndiv_int [9:0] = 0x19
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x8000, 0x8000}, // dither_en [15] = 0x1
  {"amscom_pll_ctrl_6",          0x1,  0xd0b6, 0xF000, 0x0000}, // ndiv_frac_l [15:12] = 0x0
  {"amscom_pll_ctrl_7",          0x1,  0xd0b7, 0x3FFF, 0x3200}, // ndiv_frac_h [13:0] = 0x3200
  {"amscom_pll_ctrl_9",          0x1,  0xd0b9, 0x0078, 0x0078}, // f3cap [6:3] = 0xf
  {"amscom_pll_ctrl_1",          0x1,  0xd0b1, 0x000F, 0x0003}, // lcap [3:0] = 0x3
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0x0e00, 0x0000}, // rpar [11:9] = 0x0
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0xc000, 0xc000}, // cpar [15:14] = 0x3  //??? TODO ??? where does this come from 
   /* Toggling PLL mmd reset */
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0000}, // mmd_rstb [0] = 1'b0
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0001}, // mmd_rstb [0] = 1'b1
  //{"digcom_top_user_control_0",  0x1,  0xd0f4, 0x2000, 0x2000}, // core_dp_s_rstb [13]
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 21 PMD Setup 400MHz (div2 = 0) (maple2; blackfin is outdated)
prog_seq_tbl PMD_setup_400_12p5_VCO[] = {
  {"main control register",      0x3,  0x9100, 0xf000, 0x5000}, //refclk_sel [15:12]
  {"digcom_top_user_control_0",  0x1,  0xD0F4, 0x2000, 0x0000}, //core_dp_s_rstb [13]
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x03FF, 0x001f}, // ndiv_int [9:0] = 0x1f
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x8000, 0x8000}, // dither_en [15] = 0x1
  {"amscom_pll_ctrl_6",          0x1,  0xd0b6, 0xF000, 0x0000}, // ndiv_frac_l [15:12] = 0x0
  {"amscom_pll_ctrl_7",          0x1,  0xd0b7, 0x3FFF, 0x1000}, // ndiv_frac_h [13:0] = 0x1000
  {"amscom_pll_ctrl_9",          0x1,  0xd0b9, 0x0078, 0x0078}, // f3cap [6:3] = 0xf
  {"amscom_pll_ctrl_1",          0x1,  0xd0b1, 0x000F, 0x0003}, // lcap [3:0] = 0x3
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0x0e00, 0x0000}, // rpar [11:9] = 0x0
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0xc000, 0xc000}, // cpar [15:14] = 0x3  //??? TODO ??? where does this come from 
   /* Toggling PLL mmd reset */
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0000}, // mmd_rstb [0] = 1'b0
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0001}, // mmd_rstb [0] = 1'b1
  //{"digcom_top_user_control_0",  0x1,  0xd0f4, 0x2000, 0x2000}, // core_dp_s_rstb [13]
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 

//x leverage from the Figure 21 PMD Setup 400MHz (div2 = 0); also refer to ANA_SFI1X_TS16FFC_S6_spec.pdf
prog_seq_tbl PMD_setup_50_10p3125_VCO[] = {
  {"main control register",      0x3,  0x9100, 0xf000, 0x6000}, //refclk_sel [15:12]
  {"digcom_top_user_control_0",  0x1,  0xD0F4, 0x2000, 0x0000}, //core_dp_s_rstb [13]
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x03FF, 0x00ce}, // ndiv_int [9:0] = 0xce
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x8000, 0x0000}, // dither_en [15]  = 0x0
  {"amscom_pll_ctrl_6",          0x1,  0xd0b6, 0x0c00, 0x0800}, // pfd setup [11:10] = 0x2
  {"amscom_pll_ctrl_6",          0x1,  0xd0b6, 0xF000, 0x0000}, // ndiv_frac_l [15:12] = 0x0
  {"amscom_pll_ctrl_7",          0x1,  0xd0b7, 0x3FFF, 0x1000}, // ndiv_frac_h [13:0] = 0x1000
  {"amscom_pll_ctrl_9",          0x1,  0xd0b9, 0x0078, 0x0078}, // f3cap [6:3] = 0xF
  {"amscom_pll_ctrl_1",          0x1,  0xd0b1, 0x000F, 0x0003}, // lcap [3:0] = 0x3
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0x0e00, 0x0200}, // rpar [11:9] = 0x1
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0xc000, 0xc000}, // cpar [15:14] = 0x3
   /* Toggling PLL mmd reset */
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0000}, // mmd_rstb [0] = 1'b0
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0001}, // mmd_rstb [0] = 1'b1
  //{"digcom_top_user_control_0",  0x1,  0xd0f4, 0x2000, 0x2000}, // core_dp_s_rstb [13]
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl PMD_setup_50_12p5_VCO[] = {
  {"main control register",      0x3,  0x9100, 0xf000, 0x6000}, //refclk_sel [15:12]
  {"digcom_top_user_control_0",  0x1,  0xD0F4, 0x2000, 0x0000}, //core_dp_s_rstb [13]
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x03FF, 0x00fa}, // ndiv_int [9:0] = 0xfa
  {"amscom_pll_ctrl_8",          0x1,  0xd0b8, 0x8000, 0x0000}, // dither_en [15] = 0x0
  {"amscom_pll_ctrl_6",          0x1,  0xd0b6, 0x0c00, 0x0000}, // pfd setup [11:10] = 0x0
  {"amscom_pll_ctrl_6",          0x1,  0xd0b6, 0xF000, 0x0000}, // ndiv_frac_l [15:12] = 0x0
  {"amscom_pll_ctrl_7",          0x1,  0xd0b7, 0x3FFF, 0x0000}, // ndiv_frac_h [13:0] = 0x0
  {"amscom_pll_ctrl_9",          0x1,  0xd0b9, 0x0078, 0x0000}, // f3cap [6:3] = 0x0
  {"amscom_pll_ctrl_1",          0x1,  0xd0b1, 0x000F, 0x000d}, // lcap [3:0] = 0xd
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0x0e00, 0x0600}, // rpar [11:9] = 0x3
  {"amscom_pll_ctrl_0",          0x1,  0xd0b0, 0xc000, 0x0000}, // cpar [15:14] = 0x0
   /* Toggling PLL mmd reset */
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0000}, // mmd_rstb [0] = 1'b0
  {"amscom_pll_ctrl_9",          0x1,  0xD0B9, 0x0001, 0x0001}, // mmd_rstb [0] = 1'b1
  //{"digcom_top_user_control_0",  0x1,  0xd0f4, 0x2000, 0x2000}, // core_dp_s_rstb [13]
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 

//x leverage from the Figure 21 PMD Setup 400MHz (div2 = 0); also refer to ANA_SFI1X_TS16FFC_S6_spec.pdf
prog_seq_tbl PMD_setup_156p25_10p3125_VCO[] = {
  {"main control register",       0x3,  0x9100, 0xf000, 0x3000}, //15:12 refclk_sel = 4'h3  
  {"digcom_top_user_control_0",   0x1,  0xD0F4, 0x2000, 0x0000}, //core_dp_s_rstb [13]
  {"TLB TX tlb tx Misc. Control", 0x1,  0xD0E3, 0x0002, 0x0002}, //01:01 bypass pmd tx oversampling = 1'b1 //??? TODO ??? necessary ?
  {"amscom_pll_ctrl_9",           0x1,  0xD0B9, 0x0007, 0x0004}, //00:00 mmd_rstb = 1'h0
  //{"amscom_pll_ctrl_5",           0x1,  0xd0b5, 0x0080, 0x0000}, //07:07 pll_pwrdwn = 1'b0  //default is 0x0
  //{"amscom_pll_ctrl_9",           0x1,  0xD0B9, 0x0006, 0x0004}, //02:01 frac_mode = 2'h2   //default is 0x2
  {"amscom_pll_ctrl_8",           0x1,  0xD0B8, 0x03ff, 0x0042}, // ndiv_int [9:0] = 0x42   //default is 0x42
  {"amscom_pll_ctrl_8",           0x1,  0xd0b8, 0x8000, 0x0000}, // dither_en [15] = 0x0    //default is 0x0
  {"amscom_pll_ctrl_6",           0x1,  0xd0b6, 0x0c00, 0x0000}, // pfd setup [11:10] = 0x0 //default is 0x0
  {"amscom_pll_ctrl_6",           0x1,  0xD0B6, 0xf000, 0x0000}, // ndiv_frac_l [15:12] = 0x0 //default is 0x0
  {"amscom_pll_ctrl_7",           0x1,  0xD0B7, 0x3fff, 0x0000}, // ndiv_frac_h [13:0] = 0x0  //default is 0x0
  {"amscom_pll_ctrl_9",           0x1,  0xd0b9, 0x0078, 0x0000}, // f3cap [6:3] = 0x0  //default is 0x0
  {"amscom_pll_ctrl_1",           0x1,  0xd0b1, 0x000F, 0x000d}, // lcap [3:0] = 0xd   //default is 0x7
  {"amscom_pll_ctrl_0",           0x1,  0xd0b0, 0x0e00, 0x0a00}, // rpar [11:9] = 0x5  //default is 0x2
  {"amscom_pll_ctrl_0",           0x1,  0xd0b0, 0xc000, 0x0000}, // cpar [15:14] = 0x0 //default is 0x0
  //{"timeout_100ns",               0xa,  0x0000, 0x0000, 0x0000}, // timeout_ns 100ns
  {"amscom_pll_ctrl_9",           0x1,  0xD0B9, 0x0007, 0x0005}, //00:00 mmd_rstb = 1'h1
  //{"digcom_top_user_control_0",   0x1,  0xd0f4, 0x2000, 0x2000}, // core_dp_s_rstb [13]
  {"",                            0x0,  0x0000, 0x0000, 0x0000}
}; 
//x leverage from the Figure 21 PMD Setup 400MHz (div2 = 0); also refer to ANA_SFI1X_TS16FFC_S6_spec.pdf
prog_seq_tbl PMD_setup_156p25_12p5_VCO[] = {
  {"main control register",       0x3,  0x9100, 0xf000, 0x3000}, //15:12 refclk_sel = 4'h3  
  {"digcom_top_user_control_0",   0x1,  0xD0F4, 0x2000, 0x0000}, //core_dp_s_rstb [13]
  {"TLB TX tlb tx Misc. Control", 0x1,  0xD0E3, 0x0002, 0x0002}, //01:01 bypass pmd tx oversampling = 1'b1 //??? TODO ??? necessary ?
  {"amscom_pll_ctrl_9",           0x1,  0xD0B9, 0x0007, 0x0004}, //00:00 mmd_rstb = 1'h0
  //{"amscom_pll_ctrl_5",           0x1,  0xd0b5, 0x0080, 0x0000}, //07:07 pll_pwrdwn = 1'b0  //default is 0x0
  //{"amscom_pll_ctrl_9",           0x1,  0xD0B9, 0x0006, 0x0004}, //02:01 frac_mode = 2'h2   //default is 0x2
  {"amscom_pll_ctrl_8",           0x1,  0xD0B8, 0x03ff, 0x0050}, // ndiv_int [9:0] = 0x50   //default is 0x42
  {"amscom_pll_ctrl_8",           0x1,  0xd0b8, 0x8000, 0x0000}, // dither_en [15] = 0x0    //default is 0x0
  {"amscom_pll_ctrl_6",           0x1,  0xd0b6, 0x0c00, 0x0000}, // pfd setup [11:10] = 0x0 //default is 0x0
  {"amscom_pll_ctrl_6",           0x1,  0xD0B6, 0xf000, 0x0000}, // ndiv_frac_l [15:12] = 0x0 //default is 0x0
  {"amscom_pll_ctrl_7",           0x1,  0xD0B7, 0x3fff, 0x0000}, // ndiv_frac_h [13:0] = 0x0  //default is 0x0
  {"amscom_pll_ctrl_9",           0x1,  0xd0b9, 0x0078, 0x0000}, // f3cap [6:3] = 0x0  //default is 0x0
  {"amscom_pll_ctrl_1",           0x1,  0xd0b1, 0x000F, 0x000d}, // lcap [3:0] = 0xd   //default is 0x7
  {"amscom_pll_ctrl_0",           0x1,  0xd0b0, 0x0e00, 0x0a00}, // rpar [11:9] = 0x5  //default is 0x2
  {"amscom_pll_ctrl_0",           0x1,  0xd0b0, 0xc000, 0x0000}, // cpar [15:14] = 0x0 //default is 0x0
  //{"amscom_pll_ctrl_9",           0x1,  0xD0B9, 0x0007, 0x0004}, //00:00 mmd_rstb = 1'h0
  //{"timeout_100ns",               0xa,  0x0000, 0x0000, 0x0000}, // timeout_ns 100ns
  {"amscom_pll_ctrl_9",           0x1,  0xD0B9, 0x0007, 0x0005}, //00:00 mmd_rstb = 1'h1
  //{"digcom_top_user_control_0",   0x1,  0xd0f4, 0x2000, 0x2000}, // core_dp_s_rstb [13]
  {"",                            0x0,  0x0000, 0x0000, 0x0000}
}; 

/* original; obsolete
//Figure 23 PMD Setup 156.25MHz, 10.3125GHz VCO (MerlinU)
// Changed for MERLIN16
prog_seq_tbl PMD_setup_156p25_10p3125_VCO_per_lane[] = {
  {"TLB TX Misc. Control",                     0x1,  0xD0E3, 0x0002, 0x0002},  //01:01 bypass pmd tx oversampling (per lane) = 1'b1
  {"pll pwrdwn",                               0x1,  0xD0B5, 0x0080, 0x0000},  //07:07 pll_pwrdwn (per lane) = 1'b0
  {"frac_mode",                                0x1,  0xD0B9, 0x0007, 0x0005},  //02:01 frac_mode (per lane) = 2'h2
  {"i_ndiv_int",                               0x1,  0xD0B8, 0x03ff, 0x0042},  //09:00 i_ndiv_int (per lane) = 10'h042
  {"i_ndiv_frac_h",                            0x1,  0xD0B7, 0x3fff, 0x0000},  //13:00 i_ndiv_frac_h (per lane) = 14'h0000
  {"i_ndiv_frac_l",                            0x1,  0xD0B6, 0xf000, 0x0000},  //15:12 i_ndiv_frac_l (per lane) = 4'h0
  {"i_ndiv_dither_en",                         0x1,  0xD0B8, 0x8000, 0x0000},  //15:15 i_ndiv_dither_en (per lane) = 1'h0
  {"mmd_rstb",                                 0x1,  0xD0B9, 0x0007, 0x0004},  //00:00 mmd_rstb (per lane) = 1'h0
  {"timeout_100ns",                            0xa,  0x0,  0x0, 0x0},  // timeout_ns 100ns
  {"mmd_rstb",                                 0x1,  0xD0B9, 0x0007, 0x0005},  //00:00 mmd_rstb (per lane) = 1'h1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
*/

/* -----------------------------------------------------------------
   PCS section  - the PCS (A) design is from merlinU
-------------------------------------------------------------------- */

/* -----------------------------------------------------------------
   CL36
-------------------------------------------------------------------- */

//x Figure 28 Force Speed 5G (maple2)
// (~CL36) 5G
prog_seq_tbl force_speed_5g[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0010},  //05:00 SW_actual_speed = 6'h10
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0050},  //merged [11] and [06:00]
  {"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0001},  // pmd_osr_mode [3:0] = 4'h1

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl force_speed_5g_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0010},  //05:00 SW_actual_speed = 6'h10
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0050},  //merged [11] and [06:00]
  {"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0001},  // pmd_osr_mode [3:0] = 4'h1

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 24 Force Speed 2p5G (maple2)
prog_seq_tbl force_speed_2p5g[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0003},  //05:00 SW_actual_speed = 6'h03
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0043},  //merged [11] and [06:00] 
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"C456"                  ,                   0x3,  0xC456, 0x0010, 0x0010},  //04:04  = 1'b1; for 12.5GHz VCO
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl force_speed_2p5g_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0003},  //05:00 SW_actual_speed = 6'h03
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0043},  //merged [11] and [06:00] 
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"C456"                  ,                   0x3,  0xC456, 0x0010, 0x0010},  //04:04  = 1'b1; for 12.5GHz VCO
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 28 Force Speed 1G (blackfin)
prog_seq_tbl force_speed_1g[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0002},  //05:00 SW_actual_speed = 6'h02
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0042},  //merged [11] and [06:00] 
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl force_speed_1g_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0002},  //05:00 SW_actual_speed = 6'h02
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0042},  //merged [11] and [06:00] 
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x leverage from figure 28 Force Speed 1G (blackfin)
prog_seq_tbl force_speed_1g_kx1[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x000d},  //05:00 SW_actual_speed = 6'h0d
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x004d},  //merged [11] and [06:00] 
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl force_speed_1g_kx1_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x000d},  //05:00 SW_actual_speed = 6'h0d
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x004d},  //merged [11] and [06:00] 
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 26 Force Speed 100M (maple2)
prog_seq_tbl force_speed_100m[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0001},  //05:00 SW_actual_speed = 6'h01
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0041},  //merged [11] and [06:00] 

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 27 Force Speed10M  (maple2)
prog_seq_tbl force_speed_10m[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0000},  //05:00 SW_actual_speed = 6'h00
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0040},  //merged [11] and [06:00] 

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 31 Auto Negotiation 1G KX - IEEE CL73 (maple2)
prog_seq_tbl auto_neg_1g_kx_ieee_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0020},  //15:05 speed advertisement = 11'h001  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_1g_kx_ieee_cl73_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0020},  //15:05 speed advertisement = 11'h001  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 38 Auto Negotiation 1G - User Space CL73 (blackfin)
prog_seq_tbl auto_neg_1g_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0020, 0x0020},  //05:05 speed advertisement = 1'b1  
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN ENABLES",                               0x3,  0xC480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_1g_user_cl73_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0020, 0x0020},  //05:05 speed advertisement = 1'b1  
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN ENABLES",                               0x3,  0xC480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//x Figure 39 Auto Negotiation 1G - IEEE CL37 (blackfin)
prog_seq_tbl auto_neg_1g_ieee_cl37[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x1000},  //13:12 use_ieee_reg_ctrl_sel = 2'b01
  {"IEEE AN advertised abilities register",    0x0,  0x0004, 0x0020, 0x0020},  //05:05 duplex = 1'b1  
  {"MII control register",                     0x0,  0x0000, 0x1000, 0x1000},  //12:12 AN = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_1g_ieee_cl37_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x1000},  //13:12 use_ieee_reg_ctrl_sel = 2'b01
  {"IEEE AN advertised abilities register",    0x0,  0x0004, 0x0020, 0x0020},  //05:05 duplex = 1'b1  
  {"MII control register",                     0x0,  0x0000, 0x1000, 0x1000},  //12:12 AN = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 


//x Figure 40 Auto Negotiation 1G - User Space CL37 (blackfin)
prog_seq_tbl auto_neg_1g_user_cl37[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0010, 0x0010},  //04:04 duplex = 1'b1
  {"AN ENABLES",                               0x3,  0xC480, 0x0040, 0x0040},  //06:06 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_1g_user_cl37_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0010, 0x0010},  //04:04 duplex = 1'b1
  {"AN ENABLES",                               0x3,  0xC480, 0x0040, 0x0040},  //06:06 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   CL49
-------------------------------------------------------------------- */
//x Figure 29 Force Speed 10G (merlinU)
prog_seq_tbl force_speed_10g_R[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x000f},  //05:00 SW_actual_speed = 6'h0f
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x004f},  //merged [11] and [06:00]
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0400, 0x0000},  //10:10 TX fec en (optional) = 1'b0
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode (optional) = 3'h1

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 30 Force Speed 10G with CL74 FEC (merlinU)
prog_seq_tbl force_speed_10g_R_cl74_fec[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x000f},  //05:00 SW_actual_speed = 6'h0f
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x004f},  //merged [11] and [06:00]
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4
  // * Delayed in BBS with PLL lock inserted; LX

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  // 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  // * Delayed in BBS with PLL lock inserted; LX
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 43 Auto Negotiation 10G KR - IEEE CL73 (merlinU)
prog_seq_tbl auto_neg_10g_kr_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0080},  //15:05 speed advertisement = 11'h004  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0xc000},  //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_10g_kr_cl73_cl74_fec[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0080},  //15:05 speed advertisement = 11'h004  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  {"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0xc000},  //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//* Figure 44 Auto Negotiation 10G - User Space CL73 (merlinU)
prog_seq_tbl auto_neg_10g_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0008, 0x0008},  //03:03 speed advertisement = 1'b1  
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //wrong->corrected {"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0xc000},  //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  //{"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0300, 0x0300},  //09:08 fec = 2'b11  
  {"AN ENABLES",                               0x3,  0xC480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_10g_user_cl73_cl74_fec[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0008, 0x0008},  //03:03 speed advertisement = 1'b1  
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //wrong->corrected {"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0xc000},  //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0300, 0x0300},  //09:08 fec = 2'b11  
  {"AN ENABLES",                               0x3,  0xC480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1 
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   CL127
-------------------------------------------------------------------- */

//x Figure 32 Force Speed 5G CL127 (blackfin)
prog_seq_tbl force_speed_5g_x[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0020},  //05:00 SW_actual_speed = 6'h20
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0060},  //merged [11] and [06:00]

  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1;
    /* Line above is important for CL127 to guarreentee the credit is set before link up in setting below. Or will hang!! */
    /* Also, don't merge two lines for the same 3.C30B above, timing is different !!! */

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 33 Force Speed 2.5G CL127 (blackfin)
prog_seq_tbl force_speed_2p5g_x[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0021},  //05:00 SW_actual_speed = 6'h21
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0061},  //merged [11] and [06:00]

  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1;
    /* Line above is important for CL127 to guarreentee the credit is set before link up in setting below. Or will hang!! */
    /* Also, don't merge two lines for the same 3.C30B above, timing is different !!! */

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 34 Force Speed 1G CL127 (blackfin)
prog_seq_tbl force_speed_1g_x[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0014},  //05:00 SW_actual_speed = 6'h14
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0054},  //merged [11] and [06:00]

  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1;
    /* Line above is important for CL127 to guarreentee the credit is set before link up in setting below. Or will hang!! */
    /* Also, don't merge two lines for the same 3.C30B above, timing is different !!! */

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl auto_neg_2p5g_kx_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 2",                       0x7,  0x0012, 0x0001, 0x0001},  //00:00 speed advertisement = 1'b1  //added by ALEX  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl auto_neg_2p5g_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x2000, 0x2000},  //13:13 speed advertisement = 1'b1 //added by ALEX
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN ENABLES",                               0x3,  0xC480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl cl127_1g_credit_10p3125_VCO[] = {
  {"reg1G_CL127 credit 0 register",            0x3,  0x928C, 0x3fff, 0x00a5},  //reg1G_ClockCount0 [13:00]
  {"reg1G_CL127 credit 1 register",            0x3,  0x928D, 0x1fff, 0x0029},  //reg1G_CGC [12:00]
  {"reg1G_CL127 credit 2 register",            0x3,  0x928E, 0x00ff, 0x0000},  //reg1G_ClockCount1 [07:00]
  {"reg1G_CL127 credit 2 register",            0x3,  0x928E, 0xff00, 0x0100},  //reg1G_loopcnt0 [15:08]
  {"reg1G_CL127 credit 0 register",            0x3,  0x929C, 0x1fff, 0x0000},  //reg1G_loopcnt1 [12:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl cl127_2p5g_credit_10p3125_VCO[] = {
  {"reg2p5G_CL127 credit 0 register",          0x3,  0x9284, 0x3fff, 0x0021},  //reg2p5G_CL127_ClockCount0 [13:00] = 0x21
  {"reg2p5G_CL127 credit 1 register",          0x3,  0x9285, 0x1fff, 0x0010},  //reg2p5G_CL127_CGC [12:00] = 0x10
  {"reg2p5G_CL127 credit 2 register",          0x3,  0x9286, 0x00ff, 0x0000},  //reg2p5G_CL127_ClockCount1 [07:00] = 0x0
  {"reg2p5G_CL127 credit 2 register",          0x3,  0x9286, 0xff00, 0x0100},  //reg2p5G_CL127_loopcnt0 [15:08] = 0x1
  {"reg2p5G_CL127 credit 3 register",          0x3,  0x9287, 0x003f, 0x0000},  //reg2p5G_CL127_loopcnt1 [12:00] = 0x0
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl cl127_credit_10p3125_VCO[] = {
  {"reg1G_CL127 credit 0 register",            0x3,  0x928C, 0x3fff, 0x0000},  //reg1G_ClockCount0 [13:00]
  {"reg1G_CL127 credit 1 register",            0x3,  0x928D, 0x1fff, 0x0029},  //reg1G_CGC [12:00]
  {"reg1G_CL127 credit 2 register",            0x3,  0x928E, 0x00ff, 0x0000},  //reg1G_ClockCount1 [07:00]
  {"reg1G_CL127 credit 2 register",            0x3,  0x928E, 0xff00, 0x0100},  //reg1G_loopcnt0 [15:08]
  {"reg1G_CL127 credit 0 register",            0x3,  0x929C, 0x1fff, 0x0000},  //reg1G_loopcnt1 [12:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   CL129
-------------------------------------------------------------------- */

//x Figure 24 Force Speed 5G (R-mode) (blackfin)
// CL129 5GBASE-R
prog_seq_tbl force_speed_5g_R[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0029},  //05:00 SW_actual_speed = 6'h29
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0069},  //merged [11] and [06:00]
  {"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0001},  // pmd_osr_mode [3:0] = 4'h1
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 25 Force Speed 2p5G (R-mode) (blackfin)
prog_seq_tbl force_speed_2p5g_R[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0028},  //05:00 SW_actual_speed = 6'h28
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0068},  //merged [11] and [06:00]
  {"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0004},  // pmd_osr_mode [3:0] = 4'h1
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 26 Force Speed 1G (R-mode) (blackfin)
prog_seq_tbl force_speed_1g_R[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0027},  //05:00 SW_actual_speed = 6'h27
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0067},  //merged [11] and [06:00]
  {"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0009},  // pmd_osr_mode [3:0] = 4'h1
  //{"cl72_ieee_training_enable",                0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1  
  //{"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  //{"Misc register",                            0x3,  0xC433, 0x0002, 0x0002},  //01:01 tx rstb_lane = 1'b1  
  //{"Misc register",                            0x3,  0xC433, 0x0001, 0x0001},  //00:00 enable_tx_lane = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //merged [01:00]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl auto_neg_5g_kr_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 2",                       0x7,  0x0012, 0x0002, 0x0002},  //01:01 speed advertisement = 1'b1  //added by ALEX  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl auto_neg_5g_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x1000, 0x1000},  //12:12 speed advertisement = 1'b1 //added by ALEX
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN ENABLES",                               0x3,  0xC480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0840, 0x0000},  //merged [11] & p06]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   Misc. Programming Sequence
-------------------------------------------------------------------- */

// Figure 45 Change Speed 
// Changed for MERLIN16
prog_seq_tbl change_speed[] = {
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xD081, 0x0001, 0x0000},  //00:00 ln_dp_s_rstb = 1'b0; (AL: move here to get rid of junk on GMII RXD after lane_reset)
  {"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0000},  //03:00 pmd_osr_mode [3:0] = 4'b0 (AL: added to clear previous osr_mode for 1G/2.5G/5G/10G Base-R)
  //{"timeout_100ns",                            0x1,  0x0,    0x0,    0x0   },  //timeout_ns 100ns (AL: for debug purpose)
  {"AN ENABLES",                               0x3,  0xC480, 0x01c0, 0x0000},  //08:06 AN enable = 3'b000
  {"AN ENABLES",                               0x3,  0xC480, 0x0003, 0x0000},  //01:00 AN restarts = 2'b0 
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0fff, 0x0000},  //11:00 AN base abilities = 12'b0  
  {"1000X control 2 register",                 0x3,  0xC301, 0x0001, 0x0000},  //00:00 ubaud = 1'b0
  {"Misc register",                            0x3,  0xC433, 0x0400, 0x0000},  //10:10 fec enable = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x007f, 0x0000},  //06:00 actual speed = 7'b0
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Figure 46 Datapath Reset, (core) (AL: active low, write 1'b1 to de-assert the core level datapath soft reset)
prog_seq_tbl datapath_reset_core[] = {
  //{"RESET_CONTROL_CORE_DP",                    0x1,  0xD0F2, 0x0001, 0x0001},  //00:00 core_dp_s_rstb = 1'b1; reset value = 1'b0
  {"digcom_top_user_control_0",                0x1,  0xd0f4, 0x2000, 0x2000}, // core_dp_s_rstb [13]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl usxgmii_mac_crdit_enable[] = {
  {"PLL powerdown",                            0x1,  0xd0f4, 0x4000, 0x0000},
  {"U_PCS_Main0_MiscDigControl",               0x3,  0x9302, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"U_TX_X1_Control1_misc",                    0x3,  0x9413, 0x0001, 0x0001},  //00:00 tx_enable = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Changed for MERLIN16
prog_seq_tbl en_datapath_reset_lane[] = {
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xD081, 0x0001, 0x0000},  //00:00 ln_dp_s_rstb = 1'b0; enable active low reset
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Figure 48 LPI Enable 
prog_seq_tbl lpi_enable[] = {
  {"pcs control 0 register",                   0x3,  0xC450, 0x0004, 0x0004},  //02:02 LPI_ENABLE = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Figure 49 Global PCS Loopback  (AL: this is for per-lane programming)
prog_seq_tbl global_pcs_loopback_lane[] = {
  {"SIGDET_CTRL_1",                            0x1,  0xD0C1, 0x0180, 0x0180},   //08:07 singal_detect_frc = 2'b11
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl global_pcs_loopback[] = {
  //{"SIGDET_CTRL_1",                            0x1,  0xD0C1, 0x0180, 0x0180},   //08:07 singal_detect_frc = 2'b11
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0080, 0x0080},   //07:07  local_pcs_loopback_enable_ln3 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0040, 0x0040},   //06:06  local_pcs_loopback_enable_ln2 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0020, 0x0020},   //05:05  local_pcs_loopback_enable_ln1 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0010, 0x0010},   //04:04  local_pcs_loopback_enable_ln0 = 1'b1
  {"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x00f0, 0x00f0},   //merged [07:04]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Figure 50 Global PMD Loopback  (AL: this is for per-lane programming)
prog_seq_tbl global_pmd_loopback[] = {
  {"SIGDET_CTRL_1",                            0x1,  0xD0C1, 0x0180, 0x0180},   //08:07 singal_detect_frc = 2'b11
  {"Remote Loopback Control",                  0x1,  0xD0D2, 0x0001, 0x0001},   //00:00 dig_lpbk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Figure 51 Remote PCS Loopback (AL: this is for per-core programming)
prog_seq_tbl remote_pcs_loopback[] = {
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x8000, 0x8000},   //15:15  remote_pcs_loopback_enable_ln3 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x4000, 0x4000},   //14:14  remote_pcs_loopback_enable_ln2 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x2000, 0x2000},   //13:13  remote_pcs_loopback_enable_ln1 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x1000, 0x1000},   //12:12  remote_pcs_loopback_enable_ln0 = 1'b1
  {"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0xf000, 0xa000},   //14,13  remote_pcs_loopback_enable lane 1 and 3
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Figure 52 Remote PMD Loopback (AL: this is for per-lane programming)
prog_seq_tbl remote_pmd_loopback[] = {
  {"Remote Loopback Control",                  0x1,  0xD0E2, 0x0001, 0x0001},   //00:00 rmt_lpbk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   PRBS
-------------------------------------------------------------------- */

// Figure 53 PRBS 31 Generator 
prog_seq_tbl prbs31_gen[] = {
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x0001, 0x0001},   //00:00 prbs_gen_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// Figure 53 PRBS 31 Monitor 
prog_seq_tbl prbs31_mon[] = {
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x0001, 0x0001},    //00:00 prbs_chk_en = 1'b1
//  {"PRBS Checker LOCK Status",                 0x1,  0xD0D9, 0x0001, 0x0001},    //00:00 prbs_chk_lock = 1'b1  (READ)
  {"PRBS Checker Error Counter MSB Status",    0x1,  0xD0DA, 0xffff, 0x0000},    //15:00 err_cnt_msb (to clear contents) = N/A
  {"PRBS Checker Error Counter LSB Status",    0x1,  0xD0DB, 0xffff, 0x0000},    //15:00 err_cnt_lsb (to clear contents) = N/A
//  {"PRBS Checker Error Counter MSB Status",    0x1,  0xD0DA, 0xffff, 0x0000},    //15:00 err_cnt_msb = 15'b0 (READ)
//  {"PRBS Checker Error Counter LSB Status",    0x1,  0xD0DB, 0xffff, 0x0000},   //15:00 err_cnt_lsb = 15'b0 (READ)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl prbs_gen_31[] = {
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x000e, 0x000a},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x000e, 0x000a},    //00:00 prbs_chk_en = 1'b1
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x0001, 0x0001},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x0001, 0x0001},    //00:00 prbs_chk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl prbs_gen_7[] = {
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x000e, 0x0000},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x000e, 0x0000},    //00:00 prbs_chk_en = 1'b1
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x0001, 0x0001},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x0001, 0x0001},    //00:00 prbs_chk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl prbs_gen_15[] = {
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x000e, 0x0006},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x000e, 0x0006},    //00:00 prbs_chk_en = 1'b1
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x0001, 0x0001},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x0001, 0x0001},    //00:00 prbs_chk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl prbs_gen_58[] = {
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x000e, 0x000c},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x000e, 0x000c},    //00:00 prbs_chk_en = 1'b1
  {"Pattern Generator Control",                0x1,  0xD0E1, 0x0001, 0x0001},   //00:00 prbs_gen_en = 1'b1
  {"PRBS Generator Control",                   0x1,  0xD0D1, 0x0001, 0x0001},    //00:00 prbs_chk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   Programming Simulation Speed up (maple2)
-------------------------------------------------------------------- */
//x Figure 50 PLL Lock Speed Up  (maple2)
prog_seq_tbl pll_lock_speedup[] = {
  {"PLL_CALCTL_0",                             0x1,  0xD120, 0x1fff, 0x0052},   //12:00 calib_setup_time = 13'h0052
  {"PLL_CALCTL_1",                             0x1,  0xD121, 0xffff, 0x028f},   //15:00 fredet_time = 16'h028F
  {"PLL_CALCTL_2",                             0x1,  0xD122, 0x0fff, 0x0028},   //11:00 calib_cap_charge_time = 12'h028
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 51 PMD Lock Speed Up (maple2)
prog_seq_tbl pmd_lock_speedup[] = {
  //{"DSC STATE MACHINE CONTROL 4",              0x1,  0xD014, 0x7c00, 0x0800},   //14:10 hw_tune_timeout = 5'h2
  //{"DSC STATE MACHINE CONTROL 4",              0x1,  0xD014, 0x03e0, 0x0020},   //09:05 cdr_settle_timeout = 5'h1
  //{"DSC STATE MACHINE CONTROL 4",              0x1,  0xD014, 0x001f, 0x0001},   //04:00 acq_cdr_timeout = 5'h1
  {"DSC STATE MACHINE CONTROL 4",              0x1,  0xD014, 0x7fff, 0x0821},    //merged [14:0]
  //{"TX_EEE idle  ",                            0x1,  0xD113, 0x8000, 0x0000},    //tx_eee_alert_en [15] = 0x0
  //{"TX_EEE idle  ",                            0x1,  0xD113, 0x4000, 0x0000},    //tx_eee_quiet_en [14] = 0x0
  //{"TX_EEE idle  ",                            0x1,  0xD113, 0x03f0, 0x0000},    //tx_diable_timer_ctrl [9:4] = 0x0
  {"TX_EEE idle  ",                            0x1,  0xD113, 0xc3f0, 0x0000},    //merged [15:0]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//x Figure 52 AN Speed Up (maple2)
prog_seq_tbl AN_speedup[] = {
  //{"TICK GENERATION CONTROL REGISTER 1",       0x3,  0x9107, 0x8000, 0x8000},    //15:15 tick_override = 1'b1
  //{"TICK GENERATION CONTROL REGISTER 1",       0x3,  0x9107, 0x7fff, 0x0001},    //14:00 tick_numberator_upper = 1'b1
  {"TICK GENERATION CONTROL REGISTER 1",       0x3,  0x9107, 0xffff, 0x8001},    //merged [15:0]
  {"TICK GENERATION CONTROL REGISTER 0",       0x3,  0x9108, 0x0ffc, 0x0004},    //11:02 tick_denominator = 10'h1
  {"CL37 AUTO-NEG RESTART TIMER",              0x3,  0x9250, 0xffff, 0x001f},    //15:00 cl37_restart_timer = 16'h001F
  {"CL37 AUTO-NEG COMPLETE-ACKNOWLEDGE TIMER", 0x3,  0x9251, 0xffff, 0x001f},    //15:00 cl37_ack_timer = 16'h001F
  {"CL37 AUTO-NEG TIMEOUT-ERROR TIMER",        0x3,  0x9252, 0xffff, 0x0000},    //15:00 cl37_error_timer = 16'h0000
  {"CL73 AUTO-NEG BREAK-LINK TIMER",           0x3,  0x9253, 0xffff, 0x0005},    //15:00 cl73_break_link_timer = 16'h0005
  {"CL73 LINK-UP TIMER",                       0x3,  0x9256, 0xffff, 0x001f},    //15:00 cl73_link_up = 16'h001F
  //{"PERIOD TO IGNORE THE LINK",                0x3,  0x925C, 0xffff, 0x000f},    //15:00 ignore_link_timer = 16'h000F
  {"PERIOD TO IGNORE THE LINK",                0x3,  0x925C, 0xffff, 0x001e},    //15:00 ignore_link_timer = 16'h001e
  {"TX RESET TIMER PERIOD",                    0x3,  0x924A, 0xffff, 0x000f},    //15:00 tx_reset_timer_period = 16'h000F
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   ???
-------------------------------------------------------------------- */

// test timeout for invalid address 
prog_seq_tbl timeout_test[] = {
  {"Non existed register 0",                   0x0,  0x0002, 0xffff, 0x0000},    //IEEE
  {"Non existed register 1",                   0x3,  0xc44f, 0xffff, 0x0000},    //PCS
  {"Non existed register 2",                   0x1,  0xd0e9, 0xffff, 0x0000},    //PMD
  {"Non existed register 3",                   0x2,  0x000a, 0xffff, 0x0000},    //invalid DVICE
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   ???
-------------------------------------------------------------------- */

// test power down for lane 
/* applicable for 6858/65550 only
prog_seq_tbl powerdn_lane[] = {
  //PCS digital power down
  {"RX_X4_Control0_pma_control_0",             0x3,  0xc457, 0x0001, 0x0000},    //00:00 rx rstb_lane = 0

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"Digital_MiscDigControl",                   0x3,  0xc30b, 0x0080, 0x0000},    //07:07 mac_creditenable = 0
  {"TX_X4_Control0_misc",                      0x3,  0xc433, 0x0003, 0x0000},    //01:00 tx rstb_lane = 0 ; enable_tx_lane = 0
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0000},    //01:01 ln_dp_s_rstb = 0
  //PMD analog power down
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd082, 0x0033, 0x0033},    //05:04 afe_tx_pwrdn_frc_val = 1 ; afe_tx_pwrdn_frc = 1; 01:00: afe_rx_pwrdn_frc_val = 1; afe_rx_pwrdn_frc=1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// test power up for lane 
prog_seq_tbl powerup_lane[] = {
  //PMD analog power up
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd082, 0x0033, 0x0000},    //05:04 afe_tx_pwrdn_frc_val = 0 ; afe_tx_pwrdn_frc = 0; 01:00: afe_rx_pwrdn_frc_val = 0; afe_rx_pwrdn_frc=0
  //PCS digital power up
  {"RX_X4_Control0_pma_control_0",             0x3,  0xc457, 0x0001, 0x0001},    //00:00 rx rstb_lane = 1
  {"Digital_MiscDigControl",                   0x3,  0xc30b, 0x0080, 0x0080},    //07:07 mac_creditenable = 1
  {"TX_X4_Control0_misc",                      0x3,  0xc433, 0x0003, 0x0003},    //01:00 tx rstb_lane = 1 ; enable_tx_lane = 1
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0002},    //01:01 ln_dp_s_rstb = 1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
*/

// /projects/BCA_Ethernet/BCG_Serdes/serdes_16nm_eval/A107_21/merlin16_shortfin/src/merlin16_shortfin_pwr_mgt.c
prog_seq_tbl powerdn_core[] = {
//merlin16_shasta_core_pwrdn().PWRDN:
  //merlin16_shasta_core_dp_reset(0x1):
  {"DIGCOM_TOP_USER_CONTROL_0",                0x1,  0xd0f4, 0x2000, 0x0000},    //13:13 core_dp_s_rstb = 0
  {"timeout_100ns",                            0xa,  0x0,    0x0,    0x0   },    //timeout_ns 100ns for above core_dp_s_rstb <== Minimum assertion time is 50 comclk cycles
  {"DIGCOM_TOP_USER_CONTROL_0",                0x1,  0xd0f4, 0x4000, 0x4000},    //14:14 afe_s_pll_pwrdn = 1 
  {"timeout_100ns",                            0xa,  0x0,    0x0,    0x0   },    //timeout_ns 100ns for above afe_s_pll_pwrdn <== Minimum assertion time is 50 comclk cycles
  {"AMSCOM_PLL_CTRL_5",                        0x1,  0xd0b5, 0x0080, 0x0080},    //07:07 pwrdn = 1 (pll_pwrdn: pll_ctrl_int[86])
//merlin16_shasta_core_pwrdn().PWRDN_DEEP:
  //merlin16_shasta_INTERNAL_core_clkgate(): !!! skipped due to the function does nothing
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl powerdn_lane[] = {
  //inserted by Alex: to deal with 10G AN mode issue where RX XGMII locks at Receiver Error after rx_clko stops. 
  {"Digital_MiscDigControl",                   0x3,  0xc30b, 0x0080, 0x0000},    //07:07 mac_creditenable = 0
//merlin16_shasta_lane_pwrdn().PWRDN
  /* do the RX first, since that is what is most users care about */
  {"RX_LANE_CLK_RESET_N_POWERDOWN_CONTROL",    0x1,  0xd161, 0x0001, 0x0001},    //00 ln_rx_s_pwrdn = 1
  {"TX_LANE_CLK_RESET_N_POWERDOWN_CONTROL",    0x1,  0xd171, 0x0001, 0x0001},    //00 ln_tx_s_pwrdn = 1;
//merlin16_shasta_lane_pwrdn().PWRDN_DEEP 
  //merlin16_shasta_INTERNAL_lane_clkgate(): !!! 
  //can be skipped due to no effect afe_s_pll_pwrdn or ln_[t|r]_s_pwrdn will also achieve the the same result of pmd_[t|r]x_clk_vld to 0
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd167, 0x0018, 0x0008},    //04:03 pmd_rx_clk_vld_frc_val = 0 ; pmd_rx_clk_vld_frc = 1
  {"TX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd177, 0x0018, 0x0008},    //04:03 pmd_tx_clk_vld_frc_val = 0 ; pmd_tx_clk_vld_frc = 1
  //" It is recommended for user to force pmd_tx_clk_vld to 1'b0 while tx_s_clkgate_frc_on is asserted to 1'b1 by using pmd_tx_clk_vld_frc/frc_val registers. 
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd167, 0x0001, 0x0001},    //00:00 ln_rx_s_clkgate_frc_on = 1
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd177, 0x0001, 0x0001},    //00:00 ln_tx_s_clkgate_frc_on = 1
  //the ln_dp_s_rstb can be skipped when powerdn_core[] is applied prior to this one
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0001, 0x0000},    //00:00 ln_dp_s_rstb = 0
  //{"timeout_100ns",                            0xa,  0x0,    0x0,    0x0   },    //timeout_ns 100ns for above afe_s_pll_pwrdn <== Minimum assertion time is 50 comclk cycles
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl powerup_core[] = {
//merlin16_shasta_core_pwrdn().PWRON:
  //merlin16_shasta_INTERNAL_core_clkgate(): !!! skipped due to the function does nothing
  {"AMSCOM_PLL_CTRL_5",                        0x1,  0xd0b5, 0x0080, 0x0000},    //07:07 pwrdn = 1
  {"DIGCOM_TOP_USER_CONTROL_0",                0x1,  0xd0f4, 0x4000, 0x0000},    //02:02 afe_s_pll_pwrdn = 0
  //merlin16_shasta_core_dp_reset(0x0): !!! skipped
  {"DIGCOM_TOP_USER_CONTROL_0",                0x1,  0xd0f4, 0x2000, 0x2000},    //13:13 core_dp_s_rstb = 1
//** may require delay and poll the pllLock status
  //{"timeout_100ns",                            0xa,  0x0,    0x0,    0x0   },    //timeout_ns 100ns
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};  
prog_seq_tbl powerup_lane[] = {
//merlin16_shasta_lane_pwrdn().PWR_ON
  {"TX_LANE_CLK_RESET_N_POWERDOWN_CONTROL",    0x1,  0xd171, 0x0001, 0x0000},    //00 ln_tx_s_pwrdn = 0;
  {"RX_LANE_CLK_RESET_N_POWERDOWN_CONTROL",    0x1,  0xd161, 0x0001, 0x0000},    //00 ln_rx_s_pwrdn = 1
  //inserted by Alex: to deal with 10G AN mode issue where RX XGMII locks at Receiver Error after rx_clko stops. 
  {"Digital_MiscDigControl",                   0x3,  0xc30b, 0x0080, 0x0080},    //07:07 mac_creditenable = 1
  //merlin16_shasta_INTERNAL_lane_clkgate(): !!! 
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd167, 0x0018, 0x0000},    //04:03 pmd_rx_clk_vld_frc_val = 0 ; pmd_rx_clk_vld_frc = 0
  {"TX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd177, 0x0018, 0x0000},    //04:03 pmd_tx_clk_vld_frc_val = 0 ; pmd_tx_clk_vld_frc = 0
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd167, 0x0001, 0x0000},    //00:00 ln_rx_s_clkgate_frc_on = 0
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd177, 0x0001, 0x0000},    //00:00 ln_tx_s_clkgate_frc_on = 0
  //** ln_dp_s_rstb somehow doesn't get de-asserted in the API ???
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0001, 0x0001},    //00:00 ln_dp_s_rstb = 1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

// /projects/BCA_Ethernet/BCG_Serdes/serdes_16nm_eval/A107_21/merlin16_shortfin/src/merlin16_shortfin_pwr_mgt.c
prog_seq_tbl iddq_core[] = {
  {"AMSCOM_PLL_CTRL_5",                        0x1,  0xd0b5, 0x0080, 0x0080},    //07:07 pwrdn = 1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl iddq_lane[] = {
  /* Use frc/frc_val to force all RX and TX clk_vld signals to 0 */
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd167, 0x0018, 0x0008},    //04:03 pmd_rx_clk_vld_frc_val = 0 ; pmd_rx_clk_vld_frc = 1
  {"TX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd177, 0x0018, 0x0008},    //04:03 pmd_tx_clk_vld_frc_val = 0 ; pmd_tx_clk_vld_frc = 1
  /* Use frc/frc_val to force all pmd_rx_lock signals to 0 */
  {"DSC_B_dsc_sm_ctrl_1",                      0x1,  0xd011, 0x0003, 0x0001},    //01:00 rx_dsc_lock_frc_val = 0 ; rx_dsc_lock_frc = 1
  /* Switch all the lane clocks to comclk by writing to RX/TX comclk_sel registers */
//** need to switch the pmd_[t|r]clk20 to comclk before AFE stops wclk20 after applies afe_rx_pwrdn_frc.
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd167, 0x0002, 0x0002},    //01 ln_rx_s_comclk_sel = 1
  {"RX_CKRST_CTRL_CLOCK_N_RESET_DEBUG_CONTROL",0x1,  0xd177, 0x0002, 0x0002},    //00 ln_tx_s_comclk_sel = 1
  /* Assert all the AFE pwrdn/reset pins using frc/frc_val to make sure AFE is in lowest possible power mode */
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd172, 0x0003, 0x0003},    //01:00 afe_tx_pwrdn_frc_val = 1 ; afe_tx_pwrdn_frc = 1
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd162, 0x0003, 0x0003},    //01:00 afe_rx_pwrdn_frc_val = 1 ; afe_rx_pwrdn_frc = 1
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd172, 0x000c, 0x000c},    //01:00 afe_tx_reset_frc_val = 1 ; afe_tx_reset_frc = 1
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd162, 0x000c, 0x000c},    //01:00 afe_rx_reset_frc_val = 1 ; afe_rx_reset_frc = 1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   SGMII (merlinU)
-------------------------------------------------------------------- */
//x Figure 49 Auto Negotiation 1G -SGMII Master (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl sgmii_an_speed_1g_master[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0004, 0x0004},  //02:02 duplex = 1'b1
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0003, 0x0002},  //01:00 speed = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0200, 0x0200},  //09:09 master = 1'b1
  {"AN ENABLES",                               0x3,  0xC480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 50 Auto Negotiation 100M -SGMII Master (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl sgmii_an_speed_100m_master[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0004, 0x0004},  //02:02 duplex = 1'b1
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0003, 0x0001},  //01:00 speed = 2'b01
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0200, 0x0200},  //09:09 master = 1'b1
  {"AN ENABLES",                               0x3,  0xC480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 51 Auto Negotiation 10M -SGMII Master (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl sgmii_an_speed_10m_master[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0004, 0x0004},  //02:02 duplex = 1'b1
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0003, 0x0000},  //01:00 speed = 2'b00
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0200, 0x0200},  //09:09 master = 1'b1
  {"AN ENABLES",                               0x3,  0xC480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 52 Auto Negotiation 10M/100M/1G - SGMII Slave (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl sgmii_an_slave[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN ENABLES",                               0x3,  0xC480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl sgmii_an_slave_xgmii[] = {
  {"Main0 misc control1 register",             0x3,  0x910c, 0x0080, 0x0080},  //07:07 force_cl36_xgmii = 1'b1
  {"RX_X1_Control0_reorder",                   0x3,  0x9224, 0x0020, 0x0020},  //05:05 idle_force_link_down = 1'b1
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN ENABLES",                               0x3,  0xC480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

/* -----------------------------------------------------------------
   USXGMII-S (merlinU)
-------------------------------------------------------------------- */
//x Figure 35 Set USXGMII 10G baud (merlinU)
prog_seq_tbl set_usxgmii_10g_baud[] = {
  {"1000X control 2 register",                 0x3,  0xC301, 0x0001, 0x0000},  //0:0 ubaud = 0x0: 10g
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 36 Set USXGMII 5G baud (merlinU)
prog_seq_tbl set_usxgmii_5g_baud[] = {
  {"1000X control 2 register",                 0x3,  0xC301, 0x0001, 0x0001},  //0:0 ubaud = 0x1 : 5g
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 37 Force Speed 10G USXGMII (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl force_speed_10g_usxgmii[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0036},  //05:00 SW_actual_speed = 6'h36
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0076},  //merged [11] and [06:00] 
  {"Misc register",                            0x3,  0xC433, 0x0400, 0x0000},  // Tx_fec_en = 0
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode = 3'h1
  //{"Decoder control 0 register",               0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 3'h1 (optional)

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac-creditenable = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 38 Force Speed 10G USXGMII with CL74 FEC (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl force_speed_10g_usxgmii_cl74_fec[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0036},  //05:00 SW_actual_speed = 6'h36
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0076},  //merged [11] and [06:00] 
  {"Misc register",                            0x3,  0xC433, 0x0400, 0x0400},  // Tx_fec_en = 1
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode = 3'h4
  //{"Decoder control 0 register",               0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 3'h1 (optional)

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac-creditenable = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 39 Force Speed 5G USXGMII (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl force_speed_5g_usxgmii[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0035},  //05:00 SW_actual_speed = 6'h35
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0075},  //merged [11] and [06:00] 
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode = 3'h1

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac-creditenable = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 40 Force Speed 2.5G USXGMII (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl force_speed_2p5g_usxgmii[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x0030},  //05:00 SW_actual_speed = 6'h30
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x0070},  //merged [11] and [06:00] 
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode = 3'h1

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07:07 mac-creditenable = 1'b1
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 41 Force Speed 1G USXGMII (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl force_speed_1g_usxgmii[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x002f},  //05:00 SW_actual_speed = 6'h2f
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x006f},  //merged [11] and [06:00] 
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode = 3'h1

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 42 Force Speed 100M USXGMII (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl force_speed_100m_usxgmii[] = {
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0040},  //06:06 SW_actual_speed_force_en = 1'b1
  //{"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x003f, 0x002e},  //05:00 SW_actual_speed = 6'h2e
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x087f, 0x006e},  //merged [11] and [06:00] 
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode = 3'h1

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_an_enable[] = {
/*
if speed_type = "MLN_SPD_AN_USXGMII_SLAVE" then 
		'Bcm94912.Bcm4912.XRDP.XUMAC_RDP.XUMAC_7.XUMAC_XIB_XUMAC_7.EXTENDED_CONTROL.TX_START_THRESHOLD=5		
		'Bcm94912.Bcm4912.XRDP.XUMAC_RDP.XUMAC_6.XUMAC_XIB_XUMAC_6.EXTENDED_CONTROL.TX_START_THRESHOLD=5
		dim lane_status 
	    sleep 1000
        if core_num=0 then 
        	lane_status = Bcm94912.Bcm4912.ETH.SERDES_0_MISC.SERDES_STATUS.rx_sigdet
        	if lane_status = 1 then print "core 0 rx_sigdet is ok "else print "ERROR RX_SIG is not ok " end if 
        	lane_status = Bcm94912.Bcm4912.ETH.SERDES_0_MISC.SERDES_STATUS.cdr_lock
        	if lane_status = 1 then print "core 0 CDR is ok " else print "ERROR CDR NO LOCK "end if 
        end if
	    print "enable core" & hex(core_num) & " USXGMII AN "
		call merlin_pmi_write16 (core_num, &h0&, &h3&, &hc4b1&, &h0002& , &hfffd& )	 'usxgmii_an_enalbe
end if 
		*/
  {"usxgmii autoneg control0 register",        0x3,  0xC4B1, 0x0002, 0x0002},  //01:01 usxgmii_an_enable = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_an_speed_10g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC4B0, 0xdf81, 0x9601},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_an_speed_5g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC4B0, 0xdf81, 0x9a01},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_an_speed_2p5g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC4B0, 0xdf81, 0x9801},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_an_speed_1g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC4B0, 0xdf81, 0x9401},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_an_speed_100m[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC4B0, 0xdf81, 0x9201},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_an_speed_10m[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC4B0, 0xdf81, 0x9001},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 53 USXGMII Auto Negotiation Master (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl usxgmii_an_master[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"usxgmii autoneg control0 register",        0x3,  0xC4B1, 0x0001, 0x0001},  //00:00 usxgmii_an_mode = 1'b1
  //{"usxgmii autoneg control0 register",        0x3,  0xC4B1, 0x0040, 0x0040},  //06:06 an_fast_timer = 1'b1; For Smltn only
  {"usxgmii autoneg control1 register",        0x3,  0xC4B2, 0x0200, 0x0200},  //09:09 usxgmii_master_mode = 1'b1
  //{"usxgmii autoneg control1 register",        0x3,  0xC4B2, 0x0100, 0x0100},  //08:08 usxgmii_ext_sel = 1'b1; vaild only when external usxgmii_basePage_pin is expected to be used
  {"Misc register",                            0x3,  0xC433, 0x0400, 0x0000},  //10:10 TX fec en = 1'b0
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode = 3'h1
  //{"Decoder control 0 register",               0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 3'h1 (optional)

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 54 USXGMII Auto Negotiation Slave (merlinU) + Figure 59 PCS Enable (merlinU)
prog_seq_tbl usxgmii_an_slave[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"usxgmii autoneg control0 register",        0x3,  0xC4B1, 0x0001, 0x0001},  //00:00 usxgmii_an_mode = 1'b1
  // {"usxgmii autoneg control0 register",        0x3,  0xC4B1, 0x0040, 0x0040},  //06:06 an_fast_timer = 1'b1 For Simulation only
  {"usxgmii autoneg control1 register",        0x3,  0xC4B2, 0x0200, 0x0000},  //09:09 usxgmii_master_mode = 1'b0
  //{"usxgmii autoneg control1 register",        0x3,  0xC4B2, 0x0100, 0x0100},  //08:08 usxgmii_ext_sel = 1'b1; 0:for using 3.c4b0h
  {"Misc register",                            0x3,  0xC433, 0x0400, 0x0000},  //10:10 TX fec en = 1'b0
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0001},  //02:00 RX fec block sync mode = 3'h1
  //{"Decoder control 0 register",               0x1,  0x0096, 0x0002, 0x0002},  //01:01 cl72_ieee_training_enable = 3'h1 (optional)

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"pma_control_0 register",                   0x3,  0xC457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xC30B, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xC433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"r_usxgmii_dereplicate control register",   0x3,  0xC45C, 0x0002, 0x0002},  //01:01 rstb_rx_port (if USXGMII)
  {"Misc register",                            0x3,  0xC433, 0x0010, 0x0010},  //04:04 rstb_tx_port (if USXGMII)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

/* -----------------------------------------------------------------
   USXGMII-M (shortfin)
-------------------------------------------------------------------- */

prog_seq_tbl usxgmii_pcs_reset_and_enable[] = {
  //{"TX_FED_txfir_misc_control1",               0x1,  0xD13D, 0x0001, 0x0000},  //00:00 40-bit path = 1'b0
  {"U_PCS_Main0_MiscDigControl",               0x3,  0x9302, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"U_RX_X1_Control1_pma_control_0",           0x3,  0x9437, 0x0001, 0x0001},  //00:00 rx_rstb = 1'b1
  {"U_TX_X1_Control1_misc",                    0x3,  0x9413, 0x0002, 0x0002},  //01:01 tx_rstb = 1'b1
  {"U_TX_X1_Control1_misc",                    0x3,  0x9413, 0x0001, 0x0001},  //00:00 tx_enable = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_port_reset[] = {                                        //release resets for AN function
  {"RX_X4_Control0_r_usxgmii_dereplicate",     0x3,  0xC69C, 0x0002, 0x0002},  //01:01 rstb_rx_port
  {"U_TX_X8_Control0_tx_x8_control",           0x3,  0xC681, 0x0010, 0x0010},  //04:04 rstb_tx_port
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 77 Switch PCS from A to B (shortfin)
prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_1port_10g[] = {
  {"Main0_setup_1",                            0x3,  0x9101, 0x0040, 0x0040},  //06:06 uport_ubuad_override = 1'b1
  {"Main0_setup_1",                            0x3,  0x9101, 0x0020, 0x0020},  //05:05 usxgmii_pcs_sel = 1'b1
  {"U_TX_X1_Control0_pmd_osr_mode",            0x3,  0x9402, 0x00f0, 0x0000},  //07:04 pmd_osr_mode_2 = 'h0 (i.e. osm1)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x000c, 0x0000},  //03:02 uport_num = 2'b00 (1port)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x0043, 0x0041},  //01:00 ubaud_rate = 2'b01; [6] uport_baud_override_en = 1'b1 (10.3125G)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_2port_10g[] = {
  {"Main0_setup_1",                            0x3,  0x9101, 0x0040, 0x0040},  //06:06 uport_ubuad_override = 1'b1
  {"Main0_setup_1",                            0x3,  0x9101, 0x0020, 0x0020},  //05:05 usxgmii_pcs_sel = 1'b1
  {"U_TX_X1_Control0_pmd_osr_mode",            0x3,  0x9402, 0x00f0, 0x0000},  //07:04 pmd_osr_mode_2 = 'h0 (i.e. osm1)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x000c, 0x0004},  //03:02 uport_num = 2'b01 (2port)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x0043, 0x0041},  //01:00 ubaud_rate = 2'b01; [6] uport_baud_override_en = 1'b1 (10.3125G)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_4port_10g[] = {
  {"Main0_setup_1",                            0x3,  0x9101, 0x0040, 0x0040},  //06:06 uport_ubuad_override = 1'b1
  {"Main0_setup_1",                            0x3,  0x9101, 0x0020, 0x0020},  //05:05 usxgmii_pcs_sel = 1'b1
  {"U_TX_X1_Control0_pmd_osr_mode",            0x3,  0x9402, 0x00f0, 0x0000},  //07:04 pmd_osr_mode_2 = 'h0 (i.e. osm1)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x000c, 0x0008},  //03:02 uport_num = 2'b10 (4port)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x0043, 0x0041},  //01:00 ubaud_rate = 2'b01; [6] uport_baud_override_en = 1'b1 (10.3125G)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_1port_5g[] = {
  {"Main0_setup_1",                            0x3,  0x9101, 0x0040, 0x0040},  //06:06 uport_ubuad_override = 1'b1
  {"Main0_setup_1",                            0x3,  0x9101, 0x0020, 0x0020},  //05:05 usxgmii_pcs_sel = 1'b1
  {"U_TX_X1_Control0_pmd_osr_mode",            0x3,  0x9402, 0x0f00, 0x0100},  //11:08 pmd_osr_mode_4 = 'h1 (i.e. osm2)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x000c, 0x0000},  //03:02 uport_num = 2'b00 (1port)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x0043, 0x0042},  //01:00 ubaud_rate = 2'b10; [6] uport_baud_override_en = 1'b1 (5.15625G)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_2port_5g[] = {
  {"Main0_setup_1",                            0x3,  0x9101, 0x0040, 0x0040},  //06:06 uport_ubuad_override = 1'b1
  {"Main0_setup_1",                            0x3,  0x9101, 0x0020, 0x0020},  //05:05 usxgmii_pcs_sel = 1'b1
  {"U_TX_X1_Control0_pmd_osr_mode",            0x3,  0x9402, 0x0f00, 0x0100},  //11:08 pmd_osr_mode_4 = 'h1 (i.e. osm2)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x000c, 0x0004},  //03:02 uport_num = 2'b01 (2port)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x0043, 0x0042},  //01:00 ubaud_rate = 2'b10; [6] uport_baud_override_en = 1'b1 (10.15625G)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_1port_2p5g[] = {
  {"Main0_setup_1",                            0x3,  0x9101, 0x0040, 0x0040},  //06:06 uport_ubuad_override = 1'b1
  {"Main0_setup_1",                            0x3,  0x9101, 0x0020, 0x0020},  //05:05 usxgmii_pcs_sel = 1'b1
  {"U_TX_X1_Control0_pmd_osr_mode",            0x3,  0x9402, 0xf000, 0x4000},  //15:12 pmd_osr_mode_8 = 'h4 (i.e. osm4)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x000c, 0x0000},  //03:02 uport_num = 2'b00 (1port)
  {"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x0043, 0x0043},  //01:00 ubaud_rate = 2'b11; [6] uport_baud_override_en = 1'b1 (2.578125G)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

//x Figure 78 Switch PCS from B to A (shortfin)
prog_seq_tbl usxgmii_switch_pcs_from_B_to_A[] = {
  //{"U_PCS_Main0_setup_1",                      0x3,  0x9301, 0x0060, 0x0040},  //05:05 uxsgmii_pcs_sel = 1'b0; [6] uport_baud_override_en = 1'b1 ; when usxgmii_pcs_sel_pin = 1
  {"U_PCS_Main0_setup_1",                      0x3,  0x9101, 0x0060, 0x0020},  //05:05 uxsgmii_pcs_sel = 1'b0; [6] uport_baud_override_en = 1'b1 ; when usxgmii_pcs_sel_pin = 0
  //{"TX_FED_txfir_misc_control1",               0x1,  0xD13D, 0x0001, 0x0001},  //00:00 20-bit path = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_force_speed_10g[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x0076},  //05:00 SW_actual_speed = 0x36; [6] SW_actual_speed_force_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl usxgmii_force_speed_5g[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x0075},  //05:00 SW_actual_speed = 0x35; [6] SW_actual_speed_force_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl usxgmii_force_speed_2p5g[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x0070},  //05:00 SW_actual_speed = 0x30; [6] SW_actual_speed_force_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl usxgmii_force_speed_1g[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x006f},  //05:00 SW_actual_speed = 0x2f; [6] SW_actual_speed_force_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl usxgmii_force_speed_100m[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x006e},  //05:00 SW_actual_speed = 0x2e; [6] SW_actual_speed_force_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl usxgmii_force_speed_10m[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x0077},  //05:00 SW_actual_speed = 0x37; [6] SW_actual_speed_force_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl usxgmii_mp_an_slave[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x0036},  //05:00 SW_actual_speed = 0x36; [6] SW_actual_speed_force_en = 1'b0
  {"usxgmii autoneg control0 register",        0x3,  0xC6B1, 0x0001, 0x0001},  //00:00 usxgmii_an_mode = 1'b1
  {"usxgmii autoneg control0 register",        0x3,  0xC6B1, 0x0040, 0x0040},  //06:06 an_fast_timer = 1'b1
  {"usxgmii autoneg control1 register",        0x3,  0xC6B2, 0x0200, 0x0000},  //09:09 usxgmii_master_mode = 1'b0
  //{"usxgmii autoneg control1 register",        0x3,  0xC6B2, 0x0100, 0x0100},  //08:08 usxgmii_ext_sel = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
prog_seq_tbl usxgmii_mp_an_master[] = {
  {"Digital_MiscDigControl",                   0x3,  0xc60b, 0x007f, 0x0036},  //05:00 SW_actual_speed = 0x36; [6] SW_actual_speed_force_en = 1'b0
  {"usxgmii autoneg control0 register",        0x3,  0xC6B1, 0x0001, 0x0001},  //00:00 usxgmii_an_mode = 1'b1
  {"usxgmii autoneg control0 register",        0x3,  0xC6B1, 0x0040, 0x0040},  //06:06 an_fast_timer = 1'b1
  {"usxgmii autoneg control1 register",        0x3,  0xC6B2, 0x0200, 0x0200},  //09:09 usxgmii_master_mode = 1'b1
  //{"usxgmii autoneg control1 register",        0x3,  0xC6B2, 0x0100, 0x0100},  //08:08 usxgmii_ext_sel = 1'b1; vaild only when external usxgmii_basePage_pin is expected to be used
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_mp_an_enable[] = {
  {"usxgmii autoneg control0 register",        0x3,  0xC6B1, 0x0002, 0x0002},  //01:01 usxgmii_an_enable = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_mp_an_speed_10g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC6B0, 0xdf81, 0x9601},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_mp_an_speed_5g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC6B0, 0xdf81, 0x9a01},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_mp_an_speed_2p5g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC6B0, 0xdf81, 0x9801},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_mp_an_speed_1g[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC6B0, 0xdf81, 0x9401},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_mp_an_speed_100m[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC6B0, 0xdf81, 0x9201},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl set_usxgmii_mp_an_speed_10m[] = {
  {" usxgmii autoneg tx_config word register", 0x3,  0xC6B0, 0xdf81, 0x9001},  //15:15 link; 12:12 duplex 11:9: speed 0:0 sgmii = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

// Figure 84 Global PCS Loopback (USXGMII_M_PHY_1_shortfin-002_spec.pdf)
prog_seq_tbl usxgmii_mp_global_pcs_loopback[] = {
  {"LOOPBACK CONTROL REGISTER",                0x3,  0x9309, 0x0010, 0x0010},   //04:04 local_pcs_loopback_enable = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl auto_neg_ieee_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  //{"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x00a0},  //15:05 speed advertisement = 11'h005 
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0020},  //15:05 speed advertisement = 11'h005 
  //{"AN advertisement 2",                       0x7,  0x0012, 0x0003, 0x0003},  //01:00 speed advertisement = 2'b11  //added by ALEX 
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01 
  {"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0x4000},  //15:14 FEC support (optional) = 2'h1
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1 

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl auto_neg_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x3028, 0x3028},  //13, 12,05,03 speed advertisement = 1'b1
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01 
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0100, 0x0100},  //09:08 fec = 2'b01; supported but not requested
  {"AN ENABLES",                               0x3,  0xc480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1 

  {SEQ_TYPE_NSEQ_SET(&datapath_reset_lane)}, // Cause PLL lock; LX d081
  {SEQ_TYPE_FUNC_SET(merlin_chk_pll_lock), SEQ_FUN_ARG_CORE},

  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

 
