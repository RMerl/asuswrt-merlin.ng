/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

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
// m8up1   : m8up1.html, 16nm_MERLINU_PHY_Core_1.3.pdf (April 21, 2022)

// This M1_merlin.h is for m8up1
// PCS spec used is  16nm_MERLINU_PHY_Core_1.3.pdf
// Link for pcs spec: https://drive.google.com/file/d/11-hSLQowjk19462co_FU7U8YHJn4NRUO/view

/*
Table format:
"register name", "devad", "register address", "bit mask", "value", "BitEndIndex", "BitStartIndex", "FieldName"
*/

#define EMBED_PCS_ENABLE_TABLE
#undef ENABLE_CL72_TRAINING

#define EMPTY_LAST_PROG_SEQUENCE            {"",                                         0x0,  0x0000, 0x0000, 0x0000,      0,        0, ""}

//Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
//{"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0002, 0x0002,    1,    1,    "rstb_tx_lane"},                        //01:01 tx rstb_lane = 1'b1  
//{"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0001, 0x0001,    0,    0,    "enable_tx_lane"},                      //00:00 enable_tx_lane = 1'b1
//{"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0010, 0x0010,    4,    4,    "rstb_tx_port"},                        //04:04 misc rstb tx port = 1'b1
#ifdef EMBED_PCS_ENABLE_TABLE
#define PCS_ENABLE_REGISTERS_SETTING        {"RX_X4_Control0.pma_control_0",                              0x3,  0xC457, 0x0001, 0x0001,    0,    0,    "rstb_lane"},                           \
                                            {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0080, 0x0080,    7,    7,    "mac_creditenable"},                    \
                                            {"RX_X4_Control0.r_usxgmii_dereplicate",                      0x3,  0xC45c, 0x0002, 0x0002,    1,    1,    "rstb_rx_port"},                        \
                                            {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0001, 0x0001,    0,    0,    "enable_tx_lane"},                      \
                                            {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0002, 0x0002,    1,    1,    "rstb_tx_lane"},                        \
                                            {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0010, 0x0010,    4,    4,    "rstb_tx_port"},                        
#else
#define PCS_ENABLE_REGISTERS_SETTING        {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0080, 0x0080,    7,    7,    "mac_creditenable"},                    
#endif

#define USXGMII_AN_TIMER_REGS_SETTING       {"AN_X4_USXGMII.usxgmii_an_link_up_timer",                    0x3, 0xC4B5, 0xffff, 0x0015,   15,    0,    "link_up_timer"},                       \
                                            {"AN_X4_USXGMII.usxgmii_an_complete_ack_timer",               0x3, 0xC4BC, 0xffff, 0x0015,   15,    0,    "complete_ack_timer"},                  \
                                            {"AN_X4_USXGMII.usxgmii_an_break_link_timer",                 0x3, 0xC4BD, 0xffff, 0x0015,   15,    0,    "break_link_timer"},                    \
                                            {"AN_X4_USXGMII.usxgmii_an_break_mac_lf_timer",               0x3, 0xC4BE, 0xffff, 0x0015,   15,    0,    "break_mac_lf_timer"},                  \
                                            {"AN_X4_USXGMII.usxgmii_an_err_timer",                        0x3, 0xC4BB, 0xffff, 0x0060,   15,    0,    "err_timer"},                     

/* -----------------------------------------------------------------
   PMD section  - the PMD design is from maple2 (same as blackfin)
-------------------------------------------------------------------- */

//x Figure 18 Initialize (both blackfin and maple2)
//Figure 27 Initialization for 12.5GHz VCO (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-27 : Initialize_12p5_VCO : pcs_a_ieee.04
prog_seq_tbl Initialize_12p5_VCO[] = {
  {"TX_X1_Control0.os_mode_cl36",                               0x3,  0x9201, 0x000f, 0x0008,    3,    0,    "os_mode_cl36_10m"},                    //os_mode_cl36_10m [03:00]
  {"TX_X1_Control0.os_mode_cl36",                               0x3,  0x9201, 0x00f0, 0x0080,    7,    4,    "os_mode_cl36_100m"},                   //os_mode_cl36_100m [07:04]
  {"TX_X1_Control0.os_mode_cl36",                               0x3,  0x9201, 0x0f00, 0x0800,   11,    8,    "os_mode_cl36_1000m"},                  //os_mode_cl36_1000m [11:08]
  {"TX_X1_Control0.os_mode_cl36",                               0x3,  0x9201, 0xf000, 0x4000,   15,   12,    "os_mode_cl36_2500m"},                  //os_mode_cl36_2500m [15:12]
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x000f, 0x0000,    3,    0,    "osr_mode_cl49"},                       //osr_mode_cl49 [03:00]
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x00f0, 0x0040,    7,    4,    "osr_mode_cl36_2p5"},                   //osr_mode_cl36_2p5[07:04]
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x0f00, 0x0900,   11,    8,    "osr_mode_cl36"},                       //osr_mode_cl36[11:08]
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0xf000, 0x9000,   15,   12,    "osr_mode_cl73"},                       //osr_mode_cl73[15:12]
  {"TX_X1_CREDIT_GEN1.reg1G_credit0",                           0x3,  0x926A, 0x3fff, 0x0019,   13,    0,    "reg1G_ClockCount0"},                   //TX reg1G_ClockCount0 [13:00]
  {"TX_X1_CREDIT_GEN1.reg1G_credit1",                           0x3,  0x926B, 0x1fff, 0x0006,   12,    0,    "reg1G_CGC"},                           //TX reg1G_CGC [12:00]
  {"TX_X1_CREDIT_GEN1.reg1G_credit2",                           0x3,  0x926C, 0x00ff, 0x0000,    7,    0,    "reg1G_ClockCount1"},                   //TX reg1G_ClockCount1 [07:00]
  {"TX_X1_CREDIT_GEN1.reg1G_credit2",                           0x3,  0x926C, 0xff00, 0x0100,   15,    8,    "reg1G_loopcnt0"},                      //TX reg1G_loopcnt0 [15:08]
  {"TX_X1_CREDIT_GEN1.reg1G_credit3",                           0x3,  0x926D, 0x003f, 0x0000,    5,    0,    "reg1G_loopcnt1"},                      //TX reg1G_loopcnt1 [05:00]	//BIT FIELD CHANGED
  {"RX_X1_Control2.reg1G_modulo",                               0x3,  0x9232, 0x01ff, 0x0005,    8,    0,    "reg1G_modulo"},                        //RX reg1G_modulo [08:00]

  {"TX_X1_CREDIT_GEN1.reg100M_credit0",                         0x3,  0x9265, 0x3fff, 0x007D,   13,    0,    "reg100M_ClockCount0"},                 //TX reg100M_ClockCount0 [13:00]
  {"TX_X1_CREDIT_GEN1.reg100M_credit3",                         0x3,  0x9268, 0x1fff, 0x001f,   12,    0,    "reg100M_CGC"},                         //TX reg100M_CGC [12:00]
  {"TX_X1_CREDIT_GEN1.reg100M_credit1",                         0x3,  0x9266, 0x00ff, 0x0000,    7,    0,    "reg100M_ClockCount1"},                 //TX reg100M_ClockCount1 [07:00]
  {"TX_X1_CREDIT_GEN1.reg100M_credit1",                         0x3,  0x9266, 0xff00, 0x0100,   15,    8,    "reg100M_loopcnt0"},                    //TX reg100M_loopcnt0 [15:08]
  {"TX_X1_CREDIT_GEN1.reg100M_credit4",                         0x3,  0x9269, 0xe000, 0x0000,   15,   13,    "reg100M_loopcnt1_Hi"},                 //TX reg100M_loopcnt1_Hi [15:13]
  {"TX_X1_CREDIT_GEN1.reg100M_credit3",                         0x3,  0x9268, 0xe000, 0x0000,   15,   13,    "reg100M_loopcnt1_Lo"},                 //TX reg100M_loopcnt1_Lo [15:13]
  {"TX_X1_CREDIT_GEN1.reg100M_credit2",                         0x3,  0x9267, 0x3fff, 0x0032,   13,    0,    "reg100M_PCS_ClockCount0"},             //TX reg100M_PCS_ClockCount0 [13:00]
  {"TX_X1_CREDIT_GEN1.reg100M_credit4",                         0x3,  0x9269, 0x1fff, 0x0031,   12,    0,    "reg100M_PCS_CGC"},                     //TX reg100M_PCS_CGC [12:00]
  {"RX_X1_Control2.reg100M_modulo",                             0x3,  0x9231, 0x01ff, 0x001F,    8,    0,    "reg100M_modulo"},                      //RX reg100M_modulo [08:00]

  {"TX_X1_CREDIT_GEN1.reg10M_credit0",                          0x3,  0x9260, 0x3fff, 0x0271,   13,    0,    "reg10M_ClockCount0"},                  //TX reg10M_ClockCount0 [13:00]
  {"TX_X1_CREDIT_GEN1.reg10M_credit3",                          0x3,  0x9263, 0x1fff, 0x0138,   12,    0,    "reg10M_CGC"},                          //TX reg10M_CGC [12:00]
  {"TX_X1_CREDIT_GEN1.reg10M_credit1",                          0x3,  0x9261, 0x00ff, 0x0000,    7,    0,    "reg10M_ClockCount1"},                  //TX reg10M_ClockCount1 [07:00]
  {"TX_X1_CREDIT_GEN1.reg10M_credit1",                          0x3,  0x9261, 0xff00, 0x0100,   15,    8,    "reg10M_loopcnt0"},                     //TX reg10M_loopcnt0 [15:08]
  {"TX_X1_CREDIT_GEN1.reg10M_credit4",                          0x3,  0x9264, 0xe000, 0x0000,   15,   13,    "reg10M_loopcnt1_Hi"},                  //TX reg10M_loopcnt1_Hi [15:13]
  {"TX_X1_CREDIT_GEN1.reg10M_credit3",                          0x3,  0x9263, 0xe000, 0x0000,   15,   13,    "reg10M_loopcnt1_Lo"},                  //TX reg10M_loopcnt1_Lo [15:13]
  {"TX_X1_CREDIT_GEN1.reg10M_credit2",                          0x3,  0x9262, 0x3fff, 0x0032,   13,    0,    "reg10M_PCS_ClockCount0"},              //TX reg10M_PCS_ClockCount0 [13:00]
  {"TX_X1_CREDIT_GEN1.reg10M_credit4",                          0x3,  0x9264, 0x1fff, 0x0031,   12,    0,    "reg10M_PCS_CGC"},                      //TX reg10M_PCS_CGC [12:00]
  {"RX_X1_Control2.reg10M_modulo",                              0x3,  0x9230, 0x01ff, 0x0138,    8,    0,    "reg10M_modulo"},                       //RX reg10M_modulo [08:00]
  {"TX_X1_CREDIT_GEN5.pcsCreditClkSpace1_type",                 0x3,  0x92a9, 0x07ff, 0x007d,   10,    0,    "pcsCredit_ClkSpace_100M"},
  {"TX_X1_CREDIT_GEN5.pcsCreditClkSpace2_type",                 0x3,  0x92aa, 0x07ff, 0x04e2,   10,    0,    "pcsCredit_ClkSpace_10M"},
  EMPTY_LAST_PROG_SEQUENCE
}; 

prog_seq_tbl Initialize_10p3125_VCO[] = {
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x0f00, 0x0800,   11,    8,    "osr_mode_cl36"},
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0xf000, 0x8000,   15,   12,    "osr_mode_cl73"},
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-jt : n.a. : PMD_setup_400_10p3125_VCO : n.a.
prog_seq_tbl PMD_setup_400_10p3125_VCO[] = {
  // don't support
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-jt : n.a. : PMD_setup_400_12p5_VCO : n.a.
prog_seq_tbl PMD_setup_400_12p5_VCO[] = {
  // don't support
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 28 PMD Setup 50MHz, 10.3125GHz VCO (m8up1)
// *** PS-Verified-jt : Fig-28 : PMD_setup_50_10p3125_VCO : pcs_a_ieee.42
prog_seq_tbl PMD_setup_50_10p3125_VCO[] = {
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x2000, 0x0000,   13,   13,    "core_dp_s_rstb"},                      //13:13 core_dp_s_rstb = 1'b0
  {"Main0.setup",                                               0x3,  0x9100, 0xf000, 0x6000,   15,   12,    "refclk_sel"},                          //15:12 refclk_sel = 4'h6
  {"AMSCOM8_A.PLL_CTRL_1",                                      0x1,  0xd401, 0x3c00, 0x0000,   13,   10,    "pdiv"},                                //13:10 pdiv = 4'h0
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0x03ff, 0x0067,    9,    0,    "ndiv_int"},                            //09:00 ndiv_int = 10'h67
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0xfc00, 0x0000,   15,   10,    "ndiv_frac_l"},                         //15:10 ndiv_frac_l = 6'h0
  {"AMSCOM8_A.PLL_CTRL_3",                                      0x1,  0xd403, 0x3fff, 0x0800,   13,    0,    "ndiv_frac_h"},                         //13:00 ndiv_frac_h = 14'h800
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 29 PMD Setup 50MHz, 12.5GHz VCO VCO (m8up1)
// *** PS-Verified-jt : Fig-29 : PMD_setup_50_12p5_VCO : pcs_a_ieee.46
prog_seq_tbl PMD_setup_50_12p5_VCO[] = {
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x2000, 0x0000,   13,   13,    "core_dp_s_rstb"},                      //13:13 core_dp_s_rstb = 1'b0
  {"Main0.setup",                                               0x3,  0x9100, 0xf000, 0x6000,   15,   12,    "refclk_sel"},                          //15:12 refclk_sel = 4'h6
  {"AMSCOM8_A.PLL_CTRL_1",                                      0x1,  0xd401, 0x3c00, 0x0000,   13,   10,    "pdiv"},                                //13:10 pdiv = 4'h0
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0x03ff, 0x007d,    9,    0,    "ndiv_int"},                            //09:00 ndiv_int = 10'h7d
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0xfc00, 0x0000,   15,   10,    "ndiv_frac_l"},                         //15:10 ndiv_frac_l = 6'h0
  {"AMSCOM8_A.PLL_CTRL_3",                                      0x1,  0xd403, 0x3fff, 0x0000,   13,    0,    "ndiv_frac_h"},                         //13:00 ndiv_frac_h = 14'h0
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 30 PMD Setup 156.25MHz, 10.3125GHz VCO (m8up1)
// *** PS-Verified-jt : Fig-30 : PMD_setup_156p25_10p3125_VCO : pcs_a_ieee.01
prog_seq_tbl PMD_setup_156p25_10p3125_VCO[] = {
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x2000, 0x0000,   13,   13,    "core_dp_s_rstb"},                      //13:13 core_dp_s_rstb = 1'b0
  {"Main0.setup",                                               0x3,  0x9100, 0xf000, 0x3000,   15,   12,    "refclk_sel"},                          //15:12 refclk_sel = 4'h3  
  {"AMSCOM8_A.PLL_CTRL_1",                                      0x1,  0xd401, 0x3c00, 0x0000,   13,   10,    "pdiv"},                                //13:10 pdiv = 4'h0
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0x03ff, 0x0021,    9,    0,    "ndiv_int"},                            //09:00 ndiv_int = 10'h21
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0xfc00, 0x0000,   15,   10,    "ndiv_frac_l"},                         //15:10 ndiv_frac_l = 6'h0
  {"AMSCOM8_A.PLL_CTRL_3",                                      0x1,  0xd403, 0x3fff, 0x0000,   13,    0,    "ndiv_frac_h"},                         //13:00 ndiv_frac_h = 14'h0
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 31 PMD Setup 156.25MHz, 12.5GHz VCO (m8up1)
// *** PS-Verified-jt : Fig-31 : PMD_setup_156p25_12p5_VCO : pcs_a_ieee.04
prog_seq_tbl PMD_setup_156p25_12p5_VCO[] = {
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x2000, 0x0000,   13,   13,    "core_dp_s_rstb"},                      //13:13 core_dp_s_rstb = 1'b0
  {"Main0.setup",                                               0x3,  0x9100, 0xf000, 0x3000,   15,   12,    "refclk_sel"},                          //15:12 refclk_sel = 4'h3  
  {"AMSCOM8_A.PLL_CTRL_1",                                      0x1,  0xd401, 0x3c00, 0x0000,   13,   10,    "pdiv"},                                //13:10 pdiv = 4'h0
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0x03ff, 0x0028,    9,    0,    "ndiv_int"},                            //09:00 ndiv_int = 10'h28
  {"AMSCOM8_A.PLL_CTRL_2",                                      0x1,  0xd402, 0xfc00, 0x0000,   15,   10,    "ndiv_frac_l"},                         //15:10 ndiv_frac_l = 6'h0
  {"AMSCOM8_A.PLL_CTRL_3",                                      0x1,  0xd403, 0x3fff, 0x0000,   13,    0,    "ndiv_frac_h"},                         //13:00 ndiv_frac_h = 14'h0
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   PCS section  - the PCS (A) design is from merlinU
-------------------------------------------------------------------- */

/* -----------------------------------------------------------------
   CL36
-------------------------------------------------------------------- */

//x Figure 28 Force Speed 5G (maple2)
// (~CL36) 5G
// *** PS-NOTVerified ***
prog_seq_tbl force_speed_5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0010,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h10
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x000f, 0x0001,    3,    0,    "osr_mode_cl49"},                       // pmd_osr_mode [3:0] = 4'h1
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl force_speed_5g_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1		//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0010,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h10
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x000f, 0x0001,    3,    0,    "osr_mode_cl49"},                       // pmd_osr_mode [3:0] = 4'h1
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE

}; 

// *** PS-Verified-Sdg "
//Figure 36 Force Speed 2.5G"  from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_2p5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0003,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h03
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl force_speed_2p5g_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1		//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0003,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h03
  //{"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
  //{"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0400,   10,   10,    "fec_enable"},                          //10:10 TX fec en (optional) = 1'b1  
  //{"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0004,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode (optional) = 3'h4
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Sdg "
//Figure 37 Force Speed 1G"  from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_1g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0002,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h02
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl force_speed_1g_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1		//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0002,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h02
//{"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
//{"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0400,   10,   10,    "fec_enable"},                          //10:10 TX fec en (optional) = 1'b1  
//{"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0004,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode (optional) = 3'h4
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
//x leverage from figure 28 Force Speed 1G (blackfin)
prog_seq_tbl force_speed_1g_kx1[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x000d,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h0d
//{"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
//{"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0400,   10,   10,    "fec_enable"},                          //10:10 TX fec en (optional) = 1'b1  
//{"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0004,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode (optional) = 3'h4
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl force_speed_1g_kx1_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1		//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x000d,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h0d
//{"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
//{"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0400,   10,   10,    "fec_enable"},                          //10:10 TX fec en (optional) = 1'b1  
//{"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0004,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode (optional) = 3'h4
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Sdg "
//Figure 38 Force Speed 100M"  from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_100m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0001,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h01
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Sdg "
//Figure 39 Force Speed 10M"  from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_10m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0000,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h00
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0400, 0x0400,   10,   10,    "replication_cnt"},                     //10:10 replication_cnt = 1'b1
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Sdg "
//Figure 48 Auto Negotiation 1G KX-IEEE CL73" from 16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-48 : auto_neg_1g_kx_ieee_cl73 : pcs_a_ieee.37
prog_seq_tbl auto_neg_1g_kx_ieee_cl73[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x0000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"dev7_ieee1.AN_advertisement1",                              0x7,  0x0011, 0x0020, 0x0020,    5,    5,    "BASE_1000BASE_KX"},                    //05:05 speed advertisement = 1'b1  
  {"dev7_ieee1.AN_advertisement0",                              0x7,  0x0010, 0x001f, 0x0001,    4,    0,    "selector"},                            //04:00 base selector = 5'h01  
  {"dev7_ieee0.AN_ieeeControl1",                                0x7,  0x0000, 0x1000, 0x1000,   12,   12,    "AN_enable"},                           //12:12 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x8000, 0x8000,   15,   15,    "force_ieee_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl auto_neg_1g_kx_ieee_cl73_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1	//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x0000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"dev7_ieee1.AN_advertisement1",                              0x7,  0x0011, 0x0020, 0x0020,    5,    5,    "BASE_1000BASE_KX"},                    //05:05 speed advertisement = 1'b1  
  {"dev7_ieee1.AN_advertisement0",                              0x7,  0x0010, 0x001f, 0x0001,    4,    0,    "selector"},                            //04:00 base selector = 5'h01  
  {"dev7_ieee0.AN_ieeeControl1",                                0x7,  0x0000, 0x1000, 0x1000,   12,   12,    "AN_enable"},                           //12:12 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 49 Auto Negotiation 1G User Space CL73" from 16nm_MERLINU_PHY_Core_1.3.pdf***
// Validated with test 152.1 
prog_seq_tbl auto_neg_1g_user_cl73[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_0",        0x3,  0xC486, 0x0020, 0x0020,    5,    5,    "BASE_1000BASE_KX"},                    //05:05 BASE_1000BASE_KX = 1'b1  
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_1",        0x3,  0xC485, 0x001f, 0x0001,    4,    0,    "base_selector"},                       //04:00 base selector = 5'h01  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0100, 0x0100,    8,    8,    "cl73_enable"},                         //08:08 cl73 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl auto_neg_1g_user_cl73_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1	//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_0",        0x3,  0xC486, 0x0020, 0x0020,    5,    5,    "BASE_1000BASE_KX"},                    //05:05 BASE_1000BASE_KX = 1'b1  
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_1",        0x3,  0xC485, 0x001f, 0x0001,    4,    0,    "base_selector"},                       //04:00 base selector = 5'h01  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0100, 0x0100,    8,    8,    "cl73_enable"},                         //08:08 cl73 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 50 Auto Negotiation 1G IEEE CL37" from 16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-50 : auto_neg_1g_ieee_cl37 : pcs_a_ieee.35
prog_seq_tbl auto_neg_1g_ieee_cl37[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x1000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b01
  {"CL22_B0.AutoNegAdv",                                        0x0,  0x0004, 0x0020, 0x0020,    5,    5,    "full_duplex"},                         //05:05 duplex = 1'b1  
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x1000, 0x1000,   12,   12,    "autoneg_enable"},                      //12:12 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl auto_neg_1g_ieee_cl37_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1	//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x1000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b01
  {"CL22_B0.AutoNegAdv",                                        0x0,  0x0004, 0x0020, 0x0020,    5,    5,    "full_duplex"},                         //05:05 duplex = 1'b1  
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x1000, 0x1000,   12,   12,    "autoneg_enable"},                      //12:12 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 


// *** PS-Verified-Sdg "
//Figure 51 Auto Negotiation 1G User Space CL37" from 16nm_MERLINU_PHY_Core_1.3.pdf***
// pcs_a_test.144, pcs_a_test.145
prog_seq_tbl auto_neg_1g_user_cl37[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0010, 0x0010,    4,    4,    "cl37_full_duplex"},                    //04:04 cl37 duplex = 1'b1
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0040, 0x0040,    6,    6,    "cl37_enable"},                         //06:06 cl37 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl auto_neg_1g_user_cl37_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1	//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0010, 0x0010,    4,    4,    "cl37_full_duplex"},                    //04:04 cl37 duplex = 1'b1
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0040, 0x0040,    6,    6,    "cl37_enable"},                         //06:06 cl37 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   CL49
-------------------------------------------------------------------- */
// *** PS-NOTVerified ***
// *** PS-Verified-jt : Fig-32 : force_speed_10g_R : pcs_a_fault_signal_test.1
prog_seq_tbl force_speed_10g_R[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x000f,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h0f
#ifdef  ENABLE_CL72_TRAINING   
  {"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
#endif
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 TX fec en (optional) = 1'b0
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode (optional) = 3'h1
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 32 Force Speed 10G" from 16nm_MERLINU_PHY_Core_1.3.pdf ***
// *** PS-Verified-jt : Fig-32 : force_speed_10g_R_cl74_fec : fec_error_inj_test.1
prog_seq_tbl force_speed_10g_R_cl74_fec[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x000f,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h0f
#ifdef  ENABLE_CL72_TRAINING
  {"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
#endif  
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0400,   10,   10,    "fec_enable"},                          //10:10 TX fec en (optional) = 1'b1
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0004,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode (optional) = 3'h4
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 45 Auto Negotiation 10G KR - IEEE CL73" from 16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-45 : auto_neg_10g_kr_cl73 : pcs_a_ieee.39
// // *** PS-Verified-sdg: Fig-45 : auto_neg_10g_kr_cl73 : pcs_a_test.154
prog_seq_tbl auto_neg_10g_kr_cl73[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x0000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"dev7_ieee1.AN_advertisement1",                              0x7,  0x0011, 0x0080, 0x0080,    7,    7,    "BASE_10GBASE_KR"},                     //07:07 speed advertisement = 1'b1  
  {"dev7_ieee1.AN_advertisement0",                              0x7,  0x0010, 0x001f, 0x0001,    4,    0,    "selector"},                            //04:00 base selector = 5'h01  
#ifdef  ENABLE_CL72_TRAINING  
  {"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
#endif  
  // {"dev7_ieee1.AN_advertisement2",                              0x7,  0x0012, 0xc000, 0xc000,   15,   14,    "fec_requested"},                       //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"dev7_ieee0.AN_ieeeControl1",                                0x7,  0x0000, 0x1000, 0x1000,   12,   12,    "AN_enable"},                           //12:12 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-jt : Fig-45 : auto_neg_10g_kr_cl73_cl74_fec : pcs_a_ieee.38
// // *** PS-Verified-sg : Fig-45 : auto_neg_10g_kr_cl73_cl74_fec : pcs_a_test.155
prog_seq_tbl auto_neg_10g_kr_cl73_cl74_fec[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x0000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"dev7_ieee1.AN_advertisement1",                              0x7,  0x0011, 0x0080, 0x0080,    7,    7,    "BASE_10GBASE_KR"},                     //07:07 speed advertisement = 1'b1  
  {"dev7_ieee1.AN_advertisement0",                              0x7,  0x0010, 0x001f, 0x0001,    4,    0,    "selector"},                            //04:00 base selector = 5'h01  
#ifdef  ENABLE_CL72_TRAINING  
  {"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
#endif  
  {"dev7_ieee1.AN_advertisement2",                              0x7,  0x0012, 0xc000, 0xc000,   15,   14,    "fec_requested"},                       //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"dev7_ieee0.AN_ieeeControl1",                                0x7,  0x0000, 0x1000, 0x1000,   12,   12,    "AN_enable"},                           //12:12 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 46 Auto Negotiation 10G - User Space CL73" from  16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl auto_neg_10g_user_cl73[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_0",        0x3,  0xC486, 0x0008, 0x0008,    3,    3,    "BASE_10GBASE_KR"},                     //03:03 BASE_10GBASE_KR = 1'b1  
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_1",        0x3,  0xC485, 0x001f, 0x0001,    4,    0,    "base_selector"},                       //04:00 base selector = 5'h01  
#ifdef  ENABLE_CL72_TRAINING
  {"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
#endif  
  {"dev7_ieee1.AN_advertisement2",                              0x7,  0x0012, 0xc000, 0xc000,   15,   14,    "fec_requested"},                       //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  //{"AN_X4_ABILITIES.local_device_cl73_base_abilities_0",        0x3,  0xC486, 0x0300, 0x0300,    9,    8,    "fec"},                                 //09:08 fec = 2'b11  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0100, 0x0100,    8,    8,    "cl73_enable"},                         //08:08 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl auto_neg_10g_user_cl73_cl74_fec[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_0",        0x3,  0xC486, 0x0008, 0x0008,    3,    3,    "BASE_10GBASE_KR"},                     //03:03 speed advertisement = 1'b1  
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_1",        0x3,  0xC485, 0x001f, 0x0001,    4,    0,    "base_selector"},                       //04:00 base selector = 5'h01  
#ifdef  ENABLE_CL72_TRAINING  
  {"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 1'b1
#endif
  //wrong->corrected {"dev7_ieee1.AN_advertisement2",                              0x7,  0x0012, 0xc000, 0xc000,   15,   14,    "fec_requested"},                       //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"AN_X4_ABILITIES.local_device_cl73_base_abilities_0",        0x3,  0xC486, 0x0300, 0x0300,    9,    8,    "fec"},                                 //09:08 fec = 2'b11  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0100, 0x0100,    8,    8,    "cl73_enable"},                         //08:08 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-AddedNew-Sdg "
//Figure 47 Auto Negotiation 5G KR - IEEE CL73" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl auto_neg_5g_kr_cl73[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x0000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"dev7_ieee1.AN_advertisement2",                              0x7,  0x0012, 0x0002, 0x0002,    1,    1,    "BASE_5GBASE_KR"},                      //01:01 speed advertisement 1'b1
  {"dev7_ieee1.AN_advertisement0",                              0x7,  0x0010, 0x001f, 0x0001,    4,    0,    "selector"},                            //04:00 base selector = 5'h01  
  {"dev7_ieee0.AN_ieeeControl1",                                0x7,  0x0000, 0x1000, 0x1000,   12,   12,    "AN_enable"},                           //12:12 AN enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   CL127
-------------------------------------------------------------------- */

// *** PS-Verified-Sdg "
//Figure 40 (Force Speed 5G (CL127-like))"  from  16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_5g_x[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0020,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h20
  // Probable Figure 62, PCS enable per lane
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 41 (Force Speed 2.5G (CL127-like))" from  16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-41 : force_speed_2p5g_x : pcs_a_fault_signal_test.6
prog_seq_tbl force_speed_2p5g_x[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0021,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h21
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Sdg "
//Figure 42 (Force Speed 1G (CL127-like))" from from  16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-42 : force_speed_1g_x : pcs_a_fault_signal_test.5
prog_seq_tbl force_speed_1g_x[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0014,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h14
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// could not validate
// *** PS-NOTVerified ***
prog_seq_tbl cl127_credit_10p3125_VCO[] = {
  {"TX_X1_CREDIT_GEN3.reg1G_CL127_credit0",                     0x3,  0x928C, 0x3fff, 0x0000,   13,    0,    "reg1G_CL127_ClockCount0"},             //reg1G_ClockCount0 [13:00]
  {"TX_X1_CREDIT_GEN3.reg1G_CL127_credit1",                     0x3,  0x928D, 0x1fff, 0x0029,   12,    0,    "reg1G_CL127_CGC"},                     //reg1G_CGC [12:00]
  {"TX_X1_CREDIT_GEN3.reg1G_CL127_credit2",                     0x3,  0x928E, 0x00ff, 0x0000,    7,    0,    "reg1G_CL127_ClockCount1"},             //reg1G_ClockCount1 [07:00]
  {"TX_X1_CREDIT_GEN3.reg1G_CL127_credit2",                     0x3,  0x928E, 0xff00, 0x0100,   15,    8,    "reg1G_CL127_loopcnt0"},                //reg1G_loopcnt0 [15:08]
  {"TX_X1_CREDIT_GEN4.reg1G_CL127_credit3",                     0x3,  0x929C, 0x003f, 0x0000,    5,    0,    "reg5G_KR1_loopcnt1"},                  //reg5G_KR1_loopcnt1 [05:00]	//BIT FIELD CHANGED
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   CL129
-------------------------------------------------------------------- */

// CL129 5GBASE-R
// *** PS-Verified-Sdg "
//Figure 33 (Force Speed 5G (CL129))" from  16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-33 : force_speed_5g_R : pcs_a_fault_signal_test.4
prog_seq_tbl force_speed_5g_R[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0029,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h29
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x000f, 0x0001,    3,    0,    "osr_mode_cl49"},                       //03:00 osr_mode_cl49 = 4'h1
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 34 (Force Speed 2p5G (CL129))" from  16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-34 : force_speed_2p5g_R : pcs_a_fault_signal_test.3
prog_seq_tbl force_speed_2p5g_R[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0028,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h28
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x000f, 0x0004,    3,    0,    "osr_mode_cl49"},                       //03:00 osr_mode_cl49 = 4'h4
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-Verified-Sdg "
//Figure 35 (Force Speed 1G (CL129))" from  16nm_MERLINU_PHY_Core_1.3.pdf***
// *** PS-Verified-jt : Fig-35 : force_speed_1g_R : pcs_a_fault_signal_test.2
prog_seq_tbl force_speed_1g_R[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x003f, 0x0027,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 6'h27
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x000f, 0x0009,    3,    0,    "osr_mode_cl49"},                       //03:00 osr_mode_cl49 = 4'h9
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   Misc. Programming Sequence
-------------------------------------------------------------------- */

// Changed for MERLIN16
//Figure 60 Change Speed (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-60 : change_speed : n.a.
prog_seq_tbl change_speed[] = {
  {"dev3_ieee0.PCS_ieeeControl1",                               0x3,  0x0000, 0x8000, 0x8000,   15,   15,    "rst_sw"},                              //15:15 lane reset = 1'b1   (AL: PCS reset)
  {"RXTXCOM_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",     0x1,  0xD081, 0x0001, 0x0000,    0,    0,    "ln_dp_s_rstb"},                        //00:00 ln_dp_s_rstb = 1'b0
  {"RX_X4_Control0.pma_control_0",                              0x3,  0xC457, 0x0001, 0x0000,    0,    0,    "rstb_lane"},                           //00:00 rx rstb_lane = 1'b0 
  {"TLB_TX.tlb_tx_misc_config",                                 0x1,  0xD0E3, 0x0002, 0x0002,    1,    1,    "tx_pcs_native_ana_frmt_en"},           //01:01 bypass pmd tx oversampling (per lane) = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0001, 0x0000,    0,    0,    "enable_tx_lane"},                      //00:00 enable_tx_lane = 1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0002, 0x0000,    1,    1,    "rstb_tx_lane"},                        //01:01 tx rstb_lane = 1'b0  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0080, 0x0000,    7,    7,    "mac_creditenable"},                    //07:07 mac_creditenable = 1'b0
  EMPTY_LAST_PROG_SEQUENCE
}; 

//core (AL: active low, write 1'b1 to de-assert the core level datapath soft reset)
//Figure 61 Datapath Reset, (core) (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-61 : datapath_reset_core : pcs_a_ieee.01
prog_seq_tbl datapath_reset_core[] = {
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x2000, 0x2000,   13,   13,    "core_dp_s_rstb"},                      //core_dp_s_rstb [13]
  EMPTY_LAST_PROG_SEQUENCE
}; 

//(per lane) (AL: active low, wirte 1'b0 will reset the datapth for a lane if asserted
// Changed for MERLIN16
//Figure 61 Datapath Reset, (per lane) (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-61 : datapath_reset_lane : pcs_a_ieee.01
prog_seq_tbl datapath_reset_lane[] = {
  {"RXTXCOM_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",     0x1,  0xD081, 0x0001, 0x0001,    0,    0,    "ln_dp_s_rstb"},                        //00:00 ln_dp_s_rstb = 1'b1; clear active low reset
  EMPTY_LAST_PROG_SEQUENCE
}; 

// Changed for MERLIN16
// *** PS-NOTVerified ***
prog_seq_tbl en_datapath_reset_lane[] = {
  {"RXTXCOM_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",     0x1,  0xD081, 0x0001, 0x0000,    0,    0,    "ln_dp_s_rstb"},                        //00:00 ln_dp_s_rstb = 1'b0; enable active low reset
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 62 PCS Enable (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-62 : pcs_enable : pcs_a_ieee.01
prog_seq_tbl pcs_enable[] = {
  {"RX_X4_Control0.pma_control_0",                              0x3,  0xc457, 0x0001, 0x0001,    0,    0,    "rstb_lane"},                          //00:00 rstb_lane = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xc433, 0x0001, 0x0001,    0,    0,    "enable_tx_lane"},                     //00:00 enable_tx_lane = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xc433, 0x0002, 0x0002,    1,    1,    "rstb_tx_lane"},                       //01:01 rstb_tx_lane = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xc433, 0x0010, 0x0010,    4,    4,    "rstb_tx_port"},                       //04:04 rstb_tx_port = 1'b1
  {"RX_X4_Control0.r_usxgmii_dereplicate",                      0x3,  0xc45c, 0x0002, 0x0002,    1,    1,    "rstb_rx_port"},                       //01:01 rstb_rx_port = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 63 LPI Enable (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-63 : lpi_enable : mac_filter_check.1
prog_seq_tbl lpi_enable[] = {
  {"RX_X4_Control0.pcs_control_0",                              0x3,  0xC450, 0x0004, 0x0004,    2,    2,    "LPI_ENABLE"},                          //02:02 LPI_ENABLE = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

prog_seq_tbl fec_enable[] = {
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0400,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b1 FEC ensabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0004,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b100  fec mode - enables the func_fec_sync block
  EMPTY_LAST_PROG_SEQUENCE
}; 

// Figure 49 Global PCS Loopback  (AL: this is for per-lane programming)
// *** PS-NOTVerified ***
prog_seq_tbl global_pcs_loopback_lane[] = {
  {"SIGDET.SIGDET_CTRL_1",                                      0x1,  0xD0C1, 0x0080, 0x0080,    7,    7,    "signal_detect_frc"},                   //08:07 singal_detect_frc = 1'b1
  {"SIGDET.SIGDET_CTRL_1",                                      0x1,  0xD0C1, 0x0100, 0x0100,    8,    8,    "signal_detect_frc_val"},               //08:07 singal_detect_frc_value = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 64 Global PCS Loopback (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-64 : global_pcs_loopback : pcs_a_test.035
prog_seq_tbl global_pcs_loopback[] = {
  {"SIGDET.SIGDET_CTRL_1",                                      0x1,  0xD0C1, 0x0080, 0x0080,    7,    7,    "signal_detect_frc"},                   //08:07 singal_detect_frc = 1'b1
  {"SIGDET.SIGDET_CTRL_1",                                      0x1,  0xD0C1, 0x0100, 0x0100,    8,    8,    "signal_detect_frc_val"},               //08:07 singal_detect_frc_value = 1'b1
//{"Main0.loopback_control",                                    0x3,  0x9109, 0x0080, 0x0080,    7,    4,    "local_pcs_loopback_enable"},           //07:07  local_pcs_loopback_enable_ln3 = 1'b1
//{"Main0.loopback_control",                                    0x3,  0x9109, 0x0040, 0x0040,    7,    4,    "local_pcs_loopback_enable"},           //06:06  local_pcs_loopback_enable_ln2 = 1'b1
//{"Main0.loopback_control",                                    0x3,  0x9109, 0x0020, 0x0020,    7,    4,    "local_pcs_loopback_enable"},           //05:05  local_pcs_loopback_enable_ln1 = 1'b1
  {"Main0.loopback_control",                                    0x3,  0x9109, 0x0010, 0x0010,    7,    4,    "local_pcs_loopback_enable"},           //04:04  local_pcs_loopback_enable_ln0 = 1'b1
//{"Main0.loopback_control",                                    0x3,  0x9109, 0x00f0, 0x00f0,    7,    4,    "local_pcs_loopback_enable"},           //merged [07:04]
  EMPTY_LAST_PROG_SEQUENCE
}; 

//(AL: this is for per-lane programming)
//Figure 65 Global PMD Loopback (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-65 : global_pmd_loopback : n.a.
prog_seq_tbl global_pmd_loopback[] = {
  {"SIGDET.SIGDET_CTRL_1",                                      0x1,  0xD0C1, 0x0080, 0x0080,    7,    7,    "signal_detect_frc"},                   //08:07 singal_detect_frc = 1'b1
  {"SIGDET.SIGDET_CTRL_1",                                      0x1,  0xD0C1, 0x0100, 0x0100,    8,    8,    "signal_detect_frc_val"},               //08:07 singal_detect_frc_value = 1'b1
  {"TLB_RX.dig_lpbk_config",                                    0x1,  0xD0D2, 0x0001, 0x0001,    0,    0,    "dig_lpbk_en"},                         //00:00 dig_lpbk_en = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

//(AL: this is for per-core programming)
//Figure 66 Remote PCS Loopback (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-66 : remote_pcs_loopback : pcs_a_test.058
prog_seq_tbl remote_pcs_loopback[] = {
//{"Main0.loopback_control",                                    0x3,  0x9109, 0x8000, 0x8000,   15,   12,    "remote_pcs_loopback_enable"},          //15:15  remote_pcs_loopback_enable_ln3 = 1'b1
//{"Main0.loopback_control",                                    0x3,  0x9109, 0x4000, 0x4000,   15,   12,    "remote_pcs_loopback_enable"},          //14:14  remote_pcs_loopback_enable_ln2 = 1'b1
//{"Main0.loopback_control",                                    0x3,  0x9109, 0x2000, 0x2000,   15,   12,    "remote_pcs_loopback_enable"},          //13:13  remote_pcs_loopback_enable_ln1 = 1'b1
  {"Main0.loopback_control",                                    0x3,  0x9109, 0x1000, 0x1000,   15,   12,    "remote_pcs_loopback_enable"},          //12:12  remote_pcs_loopback_enable_ln0 = 1'b1
//{"Main0.loopback_control",                                    0x3,  0x9109, 0xf000, 0xa000,   15,   12,    "remote_pcs_loopback_enable"},          //14,13  remote_pcs_loopback_enable lane 1 and 3
  EMPTY_LAST_PROG_SEQUENCE
}; 

//(AL: this is for per-lane programming)
//Figure 67 Remote PMD Loopback (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-67 : remote_pmd_loopback : pcs_a_test.066
prog_seq_tbl remote_pmd_loopback[] = {
  {"TX_PI.tx_pi_control_0",                                     0x1,  0xD070, 0x0004, 0x0004,    2,    2,    "tx_pi_ext_ctrl_en"},                   //02:02 tx_pi_ext_ctrl_en = 1'b1
  {"TX_PI.tx_pi_control_0",                                     0x1,  0xD070, 0x0002, 0x0002,    1,    1,    "tx_pi_jitter_filter_en"},              //01:01 tx_pi_jitter_filter_en = 1'b1
  {"TX_PI.tx_pi_control_0",                                     0x1,  0xD070, 0x0001, 0x0001,    0,    0,    "tx_pi_en"},                            //00:00 tx_pi_en = 1'b1
  {"TLB_TX.rmt_lpbk_config",                                    0x1,  0xD0E2, 0x0001, 0x0001,    0,    0,    "rmt_lpbk_en"},                         //00:00 rmt_lpbk_en = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   PRBS
-------------------------------------------------------------------- */

//Figure 68 PRBS 31 Generator (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-Sdg "Figure 68 PRBS 31 Generator" ***
// *** PS-Verified-jt : Fig-68 : prbs31_gen : n.a. 
prog_seq_tbl prbs31_gen[] = {
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x0001, 0x0001,    0,    0,    "prbs_gen_en"},                         //00:00 prbs_gen_en = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 69 PRBS 31 Monitor (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-SDG "Figure 69 PRBS 31 Monitor" ***
//***** check ---- need to wait for x us after setting up the lock to clear the contents for error_cnt_msb/lsb
// *** PS-Verified-jt : Fig-69 : prbs31_mon : n.a.
prog_seq_tbl prbs31_mon[] = {
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x0001, 0x0001,    0,    0,    "prbs_chk_en"},                         //00:00 prbs_chk_en = 1'b1
  {"TLB_RX.prbs_chk_lock_status",                               0x1,  0xD0D9, 0x0001, 0x0001,    0,    0,    "prbs_chk_lock"},                       //00:00 prbs_chk_lock = 1'b1  (READ)
  {"TLB_RX.prbs_chk_err_cnt_msb_status",                        0x1,  0xD0DA, 0x7fff, 0x0000,   14,    0,    "prbs_chk_err_cnt_msb"},                //14:00 err_cnt_msb (to clear contents) = N/A
  {"TLB_RX.prbs_chk_err_cnt_lsb_status",                        0x1,  0xD0DB, 0xffff, 0x0000,   15,    0,    "prbs_chk_err_cnt_lsb"},                //15:00 err_cnt_lsb (to clear contents) = N/A
//{"timeout_100ns",                                             0xa,  0x0000, 0x0000, 0x0000,    0,    0,    "wait 1us"},                 
  {"TLB_RX.prbs_chk_err_cnt_msb_status",                        0x1,  0xD0DA, 0x7fff, 0x0000,   14,    0,    "prbs_chk_err_cnt_msb"},                //14:00 err_cnt_msb = 15'b0 (READ)
  {"TLB_RX.prbs_chk_err_cnt_lsb_status",                        0x1,  0xD0DB, 0xffff, 0x0000,   15,    0,    "prbs_chk_err_cnt_lsb"},                //15:00 err_cnt_lsb = 16'b0 (READ)
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl prbs_gen_31[] = {
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x000e, 0x000a,    3,    1,    "prbs_gen_mode_sel"},                   //03:01 prbs_gen_mode_sel = 1'b5
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x000e, 0x000a,    3,    1,    "prbs_chk_mode_sel"},                   //03:01 prbs_chk_mode_sel = 1'b5
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x0001, 0x0001,    0,    0,    "prbs_gen_en"},                         //00:00 prbs_gen_en = 1'b1
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x0001, 0x0001,    0,    0,    "prbs_chk_en"},                         //00:00 prbs_chk_en = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl prbs_gen_7[] = {
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x000e, 0x0000,    3,    1,    "prbs_gen_mode_sel"},                   //03:01 prbs_gen_mode_sel = 1'b0
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x000e, 0x0000,    3,    1,    "prbs_chk_mode_sel"},                   //03:01 prbs_chk_mode_sel = 1'b0
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x0001, 0x0001,    0,    0,    "prbs_gen_en"},                         //00:00 prbs_gen_en = 1'b1
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x0001, 0x0001,    0,    0,    "prbs_chk_en"},                         //00:00 prbs_chk_en = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl prbs_gen_15[] = {
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x000e, 0x0006,    3,    1,    "prbs_gen_mode_sel"},                   //03:01 prbs_gen_mode_sel = 1'b3
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x000e, 0x0006,    3,    1,    "prbs_chk_mode_sel"},                   //03:01 prbs_chk_mode_sel = 1'b3
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x0001, 0x0001,    0,    0,    "prbs_gen_en"},                         //00:00 prbs_gen_en = 1'b1
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x0001, 0x0001,    0,    0,    "prbs_chk_en"},                         //00:00 prbs_chk_en = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl prbs_gen_58[] = {
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x000e, 0x000c,    3,    1,    "prbs_gen_mode_sel"},                   //03:01 prbs_gen_mode_sel = 1'b6
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x000e, 0x000c,    3,    1,    "prbs_chk_mode_sel"},                   //03:01 prbs_chk_mode_sel = 1'b6
  {"TLB_TX.prbs_gen_config",                                    0x1,  0xD0E1, 0x0001, 0x0001,    0,    0,    "prbs_gen_en"},                         //00:00 prbs_gen_en = 1'b1
  {"TLB_RX.prbs_chk_config",                                    0x1,  0xD0D1, 0x0001, 0x0001,    0,    0,    "prbs_chk_en"},                         //00:00 prbs_chk_en = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   Programming Simulation Speed up (maple2)
-------------------------------------------------------------------- */
//Figure 72 PLL Lock Simulation Speed Up (m8up1)
// *** PS-Verified-jt : Fig-72 : pll_lock_speedup : pcs_a_ieee.01
prog_seq_tbl pll_lock_speedup[] = {
  {"AMSCOM8_B.PLL_CTRL_6",                                      0x1,  0xd416, 0xffff, 0x2422,   15,    0,    "ctrl_misc7_l"},                        //15:00 ctrl_misc7_l = 16'h2422
  {"AMSCOM8_B.PLL_CTRL_7",                                      0x1,  0xd417, 0xffff, 0x6284,   15,    0,    "ctrl_misc7_h"},                        //15:00 ctrl_misc7_h = 16'h6284
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 73 PMD Lock Speed Up (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-Sdg "Figure 73 PMD Lock Speed UP" ***
// *** PS-Verified-jt : Fig-73 : pmd_lock_speedup : pcs_a_ieee.01
prog_seq_tbl pmd_lock_speedup[] = {
  {"DSC_B.dsc_sm_ctrl_4",                                       0x1,  0xD014, 0x7c00, 0x0800,   14,   10,    "hw_tune_timeout"},                     //14:10 hw_tune_timeout = 5'h2
  {"DSC_B.dsc_sm_ctrl_4",                                       0x1,  0xD014, 0x03e0, 0x0020,    9,    5,    "cdr_settle_timeout"},                  //09:05 cdr_settle_timeout = 5'h1
  {"DSC_B.dsc_sm_ctrl_4",                                       0x1,  0xD014, 0x001f, 0x0001,    4,    0,    "acq_cdr_timeout"},                     //04:00 acq_cdr_timeout = 5'h1
  {"TX_FED.micro_control0",                                     0x1,  0xD113, 0x8000, 0x0000,   15,   15,    "tx_eee_alert_en"},                     //tx_eee_alert_en [15] = 0x0
  {"TX_FED.micro_control0",                                     0x1,  0xD113, 0x4000, 0x0000,   14,   14,    "tx_eee_quiet_en"},                     //tx_eee_quiet_en [14] = 0x0
  {"TX_FED.micro_control0",                                     0x1,  0xD113, 0x03f0, 0x0000,    9,    4,    "tx_disable_timer_ctrl"},               //tx_diable_timer_ctrl [9:4] = 0x0
  EMPTY_LAST_PROG_SEQUENCE
}; 

//Figure 74 AN Speed Up (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-sdg "Figure 74 AN Speed Up" ***
// *** PS-Verified-jt : Fig-74 : AN_speedup : pcs_a_ieee.01
prog_seq_tbl AN_speedup[] = {
  {"Main0.tick_control_1",                                      0x3,  0x9107, 0x8000, 0x8000,   15,   15,    "tick_override"},                       //15:15 tick_override = 1'b1
  {"Main0.tick_control_1",                                      0x3,  0x9107, 0x7fff, 0x0001,   14,    0,    "tick_numerator_upper"},                //14:00 tick_numberator_upper = 1'b1
  {"Main0.tick_control_0",                                      0x3,  0x9108, 0xf000, 0x0000,   15,   12,    "tick_numerator_lower"},
  {"Main0.tick_control_0",                                      0x3,  0x9108, 0x0ffc, 0x0004,   11,    2,    "tick_denominator"},                    //11:02 tick_denominator = 10'h1
  {"AN_X1_CONTROL.tx_reset_timer_period",                       0x3,  0x924A, 0xffff, 0x000f,   15,    0,    "tx_reset_timer_period"},               //15:00 tx_reset_timer_period = 16'h000F
  {"AN_X1_TIMERS.cl37_restart",                                 0x3,  0x9250, 0xffff, 0x001f,   15,    0,    "cl37_restart_timer_period"},           //15:00 cl37_restart_timer = 16'h001F
  {"AN_X1_TIMERS.cl37_ack",                                     0x3,  0x9251, 0xffff, 0x001f,   15,    0,    "cl37_ack_timer_period"},               //15:00 cl37_ack_timer = 16'h001F
  {"AN_X1_TIMERS.cl37_error",                                   0x3,  0x9252, 0xffff, 0x0260,   15,    0,    "cl37_error_timer_period"},
  {"AN_X1_TIMERS.cl73_break_link",                              0x3,  0x9253, 0xffff, 0x001a,   15,    0,    "tx_disable_timer_period"},
//for silicon/api value{"AN_X1_TIMERS.cl73_error",                                   5'd3,  0x9254, 0xffff, 0x0260,   15,    0,    "cl73_error_timer_period"},
//for silicon/api value{"AN_X1_TIMERS.cl73_dme_lock",                                5'd3,  0x9255, 0xffff, 0x0000,   15,    0,    "pd_dme_lock_timer_period"},
  {"AN_X1_TIMERS.link_up",                                      0x3,  0x9256, 0xffff, 0x002f,   15,    0,    "cl73_link_up_timer_period"},           //15:00 cl73_link_up = 16'h001F
//for silicon/api value{"AN_X1_TIMERS.link_fail_inhibit_timer_cl72",                 5'd3,  0x9257, 0xffff, 0x0fff,   15,    0,    "link_fail_inhibit_timer_cl72_period"},
//for silicon/api value{"AN_X1_TIMERS.link_fail_inhibit_timer_not_cl72",             5'd3,  0x9258, 0xffff, 0x0fff,   15,    0,    "link_fail_inhibit_timer_ncl72_period"},
  {"AN_X1_TIMERS.pd_sd_timer",                                  0x3,  0x9259, 0xffff, 0x000f,   15,    0,    "pd_sd_timer_period"},
  {"AN_X1_TIMERS.ignore_link_timer",                            0x3,  0x925C, 0xffff, 0x001f,   15,    0,    "ignore_link_timer_period"},            //15:00 ignore_link_timer = 16'h000F
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   ???
-------------------------------------------------------------------- */

// test timeout for invalid address 
// *** PS-NOTVerified ***
prog_seq_tbl timeout_test[] = {
  {"Non existed register 0",                   0x0,  0x0002, 0xffff, 0x0000,        0,        0, "?"},    //IEEE
  {"Non existed register 1",                   0x3,  0xc44f, 0xffff, 0x0000,        0,        0, "?"},    //PCS
  {"Non existed register 2",                   0x1,  0xd0e9, 0xffff, 0x0000,        0,        0, "?"},    //PMD
  {"Non existed register 3",                   0x2,  0x000a, 0xffff, 0x0000,        0,        0, "?"},    //invalid DVICE
  EMPTY_LAST_PROG_SEQUENCE
}; 


// /projects/BCA_Ethernet/BCG_Serdes/serdes_16nm_eval/A107_21/merlin16_shortfin/src/merlin16_shortfin_pwr_mgt.c
// *** PS-NOTVerified ***
prog_seq_tbl powerdn_core[] = {
//merlin16_shasta_core_pwrdn().PWRDN:
  //merlin16_shasta_core_dp_reset(0x1):
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x2000, 0x0000,   13,   13,    "core_dp_s_rstb"},                      //13:13 core_dp_s_rstb = 0
  {"timeout_100ns",                           0xa,  0x0,    0x0,    0x0,          0,       0, "?"},                 //timeout_ns 100ns for above core_dp_s_rstb <== Minimum assertion time is 50 comclk cycles
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x4000, 0x4000,   14,   14,    "afe_s_pll_pwrdn"},                     //14:14 afe_s_pll_pwrdn = 1 
  {"timeout_100ns",                           0xa,  0x0,    0x0,    0x0,          0,       0, "?"},                 //timeout_ns 100ns for above afe_s_pll_pwrdn <== Minimum assertion time is 50 comclk cycles
  {"AMSCOM8_A.PLL_CTRL_0",                                      0x1,  0xd400, 0x0001, 0x0000,    0,    0,    "pwrup"},                               //0:0 pwrup = 1'b0
  {"AMSCOM8_A.PLL_CTRL_0",                                      0x1,  0xd400, 0x0400, 0x0400,   10,   10,    "pwrup_sel"},                           //pwrup_sel = 1'b1
  //merlin16_shasta_core_pwrdn().PWRDN_DEEP:
  //merlin16_shasta_INTERNAL_core_clkgate(): !!! skipped due to the function does nothing
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl powerdn_lane[] = {
  //inserted by Alex: to deal with 10G AN mode issue where RX XGMII locks at Receiver Error after rx_clko stops. 
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0080, 0x0000,    7,    7,    "mac_creditenable"},                    //07:07 mac_creditenable = 0
//merlin16_shasta_lane_pwrdn().PWRDN
  /* do the RX first, since that is what is most users care about */
  {"RX_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",          0x1,  0xd161, 0x0001, 0x0001,    0,    0,    "ln_rx_s_pwrdn"},                       //00 ln_rx_s_pwrdn = 1
  {"TX_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",          0x1,  0xd171, 0x0001, 0x0001,    0,    0,    "ln_tx_s_pwrdn"},                       //00 ln_tx_s_pwrdn = 1;
//merlin16_shasta_lane_pwrdn().PWRDN_DEEP 
  //merlin16_shasta_INTERNAL_lane_clkgate(): !!! 
  //can be skipped due to no effect afe_s_pll_pwrdn or ln_[t|r]_s_pwrdn will also achieve the the same result of pmd_[t|r]x_clk_vld to 0
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0008, 0x0008,    3,    3,    "pmd_rx_clk_vld_frc"},                  //03:03 pmd_rx_clk_vld_frc = 1
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0010, 0x0000,    4,    4,    "pmd_rx_clk_vld_frc_val"},              //04:04 pmd_rx_clk_vld_frc_val = 0
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0008, 0x0008,    3,    3,    "pmd_tx_clk_vld_frc"},                  //03:03 pmd_tx_clk_vld_frc = 1
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0010, 0x0000,    4,    4,    "pmd_tx_clk_vld_frc_val"},              //04:04 pmd_tx_clk_vld_frc_val = 0
  //" It is recommended for user to force pmd_tx_clk_vld to 1'b0 while tx_s_clkgate_frc_on is asserted to 1'b1 by using pmd_tx_clk_vld_frc/frc_val registers. 
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0001, 0x0001,    0,    0,    "ln_rx_s_clkgate_frc_on"},              //00:00 ln_rx_s_clkgate_frc_on = 1
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0001, 0x0001,    0,    0,    "ln_tx_s_clkgate_frc_on"},              //00:00 ln_tx_s_clkgate_frc_on = 1
  //the ln_dp_s_rstb can be skipped when powerdn_core[] is applied prior to this one
  {"RXTXCOM_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",     0x1,  0xd081, 0x0001, 0x0000,    0,    0,    "ln_dp_s_rstb"},                        //00:00 ln_dp_s_rstb = 0
  //{"timeout_100ns",                            0xa,  0x0,    0x0,    0x0,        ?,        ?, "?"},    //timeout_ns 100ns for above afe_s_pll_pwrdn <== Minimum assertion time is 50 comclk cycles
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl powerup_core[] = {
  //merlin16_shasta_core_pwrdn().PWRON:
  //merlin16_shasta_INTERNAL_core_clkgate(): !!! skipped due to the function does nothing
  {"AMSCOM8_A.PLL_CTRL_0",                                      0x1,  0xd400, 0x0001, 0x0001,    0,    0,    "pwrup"},                               // 0: pwrup = 1'b1
  {"AMSCOM8_A.PLL_CTRL_0",                                      0x1,  0xd400, 0x0400, 0x0400,   10,   10,    "pwrup_sel"},                           // 10: pwrup_sel = 1'b1
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x4000, 0x0000,   14,   14,    "afe_s_pll_pwrdn"},                     //14:14 afe_s_pll_pwrdn = 0
  //merlin16_shasta_core_dp_reset(0x0): !!! skipped
  {"CORE_PLL_COM.TOP_USER_CONTROL",                             0x1,  0xd1a4, 0x2000, 0x2000,   13,   13,    "core_dp_s_rstb"},                      //13:13 core_dp_s_rstb = 1
  //** may require delay and poll the pllLock status
//{"timeout_100ns",                           0xa,  0x0,    0x0,    0x0,        ?,        ?, "?"},    //timeout_ns 100ns
  EMPTY_LAST_PROG_SEQUENCE
};  

// *** PS-NOTVerified ***
prog_seq_tbl powerup_lane[] = {
//merlin16_shasta_lane_pwrdn().PWR_ON
  {"TX_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",          0x1,  0xd171, 0x0001, 0x0000,    0,    0,    "ln_tx_s_pwrdn"},                       //00 ln_tx_s_pwrdn = 0;
  {"RX_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",          0x1,  0xd161, 0x0001, 0x0000,    0,    0,    "ln_rx_s_pwrdn"},                       //00 ln_rx_s_pwrdn = 1
  //inserted by Alex: to deal with 10G AN mode issue where RX XGMII locks at Receiver Error after rx_clko stops. 
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0080, 0x0080,    7,    7,    "mac_creditenable"},                    //07:07 mac_creditenable = 1
  //merlin16_shasta_INTERNAL_lane_clkgate(): !!! 
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0008, 0x0000,    3,    3,    "pmd_rx_clk_vld_frc"},                  //03:03 pmd_rx_clk_vld_frc = 0
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0010, 0x0000,    4,    4,    "pmd_rx_clk_vld_frc_val"},              //04:04 pmd_rx_clk_vld_frc_val = 0
  
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0008, 0x0000,    3,    3,    "pmd_tx_clk_vld_frc"},                  //03:03 pmd_tx_clk_vld_frc = 0
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0010, 0x0000,    4,    4,    "pmd_tx_clk_vld_frc_val"},              //04:04 pmd_tx_clk_vld_frc_val = 0
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0001, 0x0000,    0,    0,    "ln_rx_s_clkgate_frc_on"},              //00:00 ln_rx_s_clkgate_frc_on = 0
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0001, 0x0000,    0,    0,    "ln_tx_s_clkgate_frc_on"},              //00:00 ln_tx_s_clkgate_frc_on = 0
  //** ln_dp_s_rstb somehow doesn't get de-asserted in the API ???
  {"RXTXCOM_CKRST_CTRL.LANE_CLK_RESET_N_POWERDOWN_CONTROL",     0x1,  0xd081, 0x0001, 0x0001,    0,    0,    "ln_dp_s_rstb"},                        //00:00 ln_dp_s_rstb = 1
  EMPTY_LAST_PROG_SEQUENCE
}; 

// /projects/BCA_Ethernet/BCG_Serdes/serdes_16nm_eval/A107_21/merlin16_shortfin/src/merlin16_shortfin_pwr_mgt.c
// *** PS-NOTVerified ***
prog_seq_tbl iddq_core[] = {
  {"AMSCOM8_A.PLL_CTRL_0",                                      0x1,  0xd400, 0x0001, 0x0000,    0,    0,    "pwrup"},                               //0: pwrup = 1'b0
  {"AMSCOM8_A.PLL_CTRL_0",                                      0x1,  0xd400, 0x0400, 0x0400,   10,   10,    "pwrup_sel"},                           //10: pwrup_sel = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
}; 

// *** PS-NOTVerified ***
prog_seq_tbl iddq_lane[] = {
  /* Use frc/frc_val to force all RX and TX clk_vld signals to 0 */
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0008, 0x0008,    3,    3,    "pmd_rx_clk_vld_frc"},                  //03:03 pmd_rx_clk_vld_frc = 1
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0010, 0x0000,    4,    4,    "pmd_rx_clk_vld_frc_val"},              //04:04 pmd_rx_clk_vld_frc_val = 0
  
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0008, 0x0008,    3,    3,    "pmd_tx_clk_vld_frc"},                  //03:03 pmd_tx_clk_vld_frc = 1
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0010, 0x0000,    4,    4,    "pmd_tx_clk_vld_frc_val"},              //04:04 pmd_tx_clk_vld_frc_val = 0
  
  /* Use frc/frc_val to force all pmd_rx_lock signals to 0 */
  {"DSC_B.dsc_sm_ctrl_1",                                       0x1,  0xd011, 0x0001, 0x0001,    0,    0,    "rx_dsc_lock_frc"},                     //[0] rx_dsc_lock_frc = 1
  {"DSC_B.dsc_sm_ctrl_1",                                       0x1,  0xd011, 0x0002, 0x0000,    1,    1,    "rx_dsc_lock_frc_val"},                 //[1] rx_dsc_lock_frc_val = 0

  /* Switch all the lane clocks to comclk by writing to RX/TX comclk_sel registers */
//** need to switch the pmd_[t|r]clk20 to comclk before AFE stops wclk20 after applies afe_rx_pwrdn_frc.
  {"RX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd167, 0x0002, 0x0002,    1,    1,    "ln_rx_s_comclk_sel"},                  //01 ln_rx_s_comclk_sel = 1
  {"TX_CKRST_CTRL.CLOCK_N_RESET_DEBUG_CONTROL",                 0x1,  0xd177, 0x0002, 0x0002,    1,    1,    "ln_tx_s_comclk_sel"},                  //00 ln_tx_s_comclk_sel = 1
  /* Assert all the AFE pwrdn/reset pins using frc/frc_val to make sure AFE is in lowest possible power mode */
  {"TX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd172, 0x0001, 0x0001,    0,    0,    "afe_tx_pwrdn_frc"},                    //00:00 afe_tx_pwrdn_frc = 1
  {"TX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd172, 0x0002, 0x0002,    1,    1,    "afe_tx_pwrdn_frc_val"},                //01:01 afe_tx_pwrdn_frc_val = 1
  {"RX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd162, 0x0001, 0x0001,    0,    0,    "afe_rx_pwrdn_frc"},                    //00:00 afe_rx_pwrdn_frc = 1
  {"RX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd162, 0x0002, 0x0002,    1,    1,    "afe_rx_pwrdn_frc_val"},                //01:01 afe_rx_pwrdn_frc_val = 1
  {"TX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd172, 0x0004, 0x0004,    2,    2,    "afe_tx_reset_frc"},                    //02:02 afe_tx_reset_frc = 1
  {"TX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd172, 0x0008, 0x0008,    3,    3,    "afe_tx_reset_frc_val"},                //03:03 afe_tx_reset_frc_val = 1
  {"RX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd162, 0x0004, 0x0004,    2,    2,    "afe_rx_reset_frc"},                    //02:02 afe_rx_reset_frc = 1
  {"RX_CKRST_CTRL.LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",       0x1,  0xd162, 0x0008, 0x0008,    3,    3,    "afe_rx_reset_frc_val"},                //03:03 afe_rx_reset_frc_val = 1
  EMPTY_LAST_PROG_SEQUENCE
}; 

/* -----------------------------------------------------------------
   SGMII (merlinU)
-------------------------------------------------------------------- */
//x Figure 49 Auto Negotiation 1G -SGMII Master (merlinU) + Figure 59 PCS Enable (merlinU)
//Figure 52 Auto Negotiation 1G - SGMII Master (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-52 : sgmii_an_speed_1g_master : pcs_a_test.148 (pcs_a_test.146-151, 528-533, 573-578)
prog_seq_tbl sgmii_an_speed_1g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0004, 0x0004,    2,    2,    "sgmii_full_duplex"},                   //02:02 duplex = 1'b1
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0003, 0x0002,    1,    0,    "sgmii_speed"},                         //01:00 speed = 2'b10
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0200, 0x0200,    9,    9,    "sgmii_master_mode"},                   //09:09 master = 1'b1
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0040, 0x0040,    6,    6,    "cl37_enable"},                         //06:06 cl37_enable = 1'b1  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0080, 0x0080,    7,    7,    "cl37_sgmii_enable"},                   //07:07 cl37_sgmii_enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//x Figure 50 Auto Negotiation 100M -SGMII Master (merlinU) + Figure 59 PCS Enable (merlinU)
//Figure 53 Auto Negotiation 100M - SGMII Master (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-53 : sgmii_an_speed_100m_master : pcs_a_test.147
prog_seq_tbl sgmii_an_speed_100m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0004, 0x0004,    2,    2,    "sgmii_full_duplex"},                   //02:02 duplex = 1'b1
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0003, 0x0001,    1,    0,    "sgmii_speed"},                         //01:00 speed = 2'b01
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0200, 0x0200,    9,    9,    "sgmii_master_mode"},                   //09:09 master = 1'b1
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0040, 0x0040,    6,    6,    "cl37_enable"},                         //06:06 cl37_enable = 1'b1  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0080, 0x0080,    7,    7,    "cl37_sgmii_enable"},                   //07:07 cl37_sgmii_enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//x Figure 51 Auto Negotiation 10M -SGMII Master (merlinU) + Figure 59 PCS Enable (merlinU)
//Figure 54 Auto Negotiation 10M - SGMII Master (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-54 : sgmii_an_speed_10m_master : pcs_a_test.146
prog_seq_tbl sgmii_an_speed_10m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0004, 0x0004,    2,    2,    "sgmii_full_duplex"},                   //02:02 duplex = 1'b1
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0003, 0x0000,    1,    0,    "sgmii_speed"},                         //01:00 speed = 2'b00
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0200, 0x0200,    9,    9,    "sgmii_master_mode"},                   //09:09 master = 1'b1
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0040, 0x0040,    6,    6,    "cl37_enable"},                         //06:06 cl37_enable = 1'b1  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0080, 0x0080,    7,    7,    "cl37_sgmii_enable"},                   //07:07 cl37_sgmii_enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//x Figure 52 Auto Negotiation 10M/100M/1G - SGMII Slave (merlinU) + Figure 59 PCS Enable (merlinU)
//Figure 55 Auto Negotiation 10M/100M/1G - SGMII Slave (16nm_MERLINU_PHY_Core_1.3.pdf)
// *** PS-Verified-jt : Fig-55 : sgmii_an_slave : pcs_a_test.148
prog_seq_tbl sgmii_an_slave[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.local_device_cl37_base_abilities",          0x3,  0xC481, 0x0200, 0x0000,    9,    9,    "sgmii_master_mode"},                   //09:09 master = 1'b0
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0040, 0x0040,    6,    6,    "cl37_enable"},                         //06:06 cl37_enable = 1'b1  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0080, 0x0080,    7,    7,    "cl37_sgmii_enable"},                   //07:07 cl37_sgmii_enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl sgmii_an_slave_xgmii[] = {
  {"Main0.misc_control",                                        0x3,  0x910c, 0x0080, 0x0080,   10,    3,    "reserved0"},                           //07:07 force_cl36_xgmii = 1'b1	//BIT DEFINITION CHANGED 
  {"RX_X1_Control0.reorder",                                    0x3,  0x9224, 0x0020, 0x0020,    5,    5,    "idle_force_link_down"},                //05:05 idle_force_link_down = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x3000, 0x2000,   13,   12,    "use_ieee_reg_ctrl_sel"},               //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0040, 0x0040,    6,    6,    "cl37_enable"},                         //06:06 cl37_enable = 1'b1  
  {"AN_X4_ABILITIES.enables",                                   0x3,  0xC480, 0x0080, 0x0080,    7,    7,    "cl37_sgmii_enable"},                   //07:07 cl37_sgmii_enable = 1'b1  
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},                        //11:11 credit_sw_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

/* -----------------------------------------------------------------
   USXGMII-S (merlinU)
-------------------------------------------------------------------- */
//x Figure 35 Set USXGMII 10G baud (merlinU)
// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_10g_baud[] = {
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:0 ubaud = 0x0: 10g
  EMPTY_LAST_PROG_SEQUENCE
};

//x Figure 36 Set USXGMII 5G baud (merlinU)
// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_5g_baud[] = {
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:0 ubaud = 0x1 : 5g
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 43 Force Speed USXGMII" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_10g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:09 speed, 10G = 3'b011
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 43 Force Speed USXGMII" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_5g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 43 Force Speed USXGMII" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_2p5g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 43 Force Speed USXGMII" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_1g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 43 Force Speed USXGMII" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_100m_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 43 Force Speed USXGMII" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_10m_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 44 Force Speed USXGMII with external pins" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_10g_usxgmii_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 44 Force Speed USXGMII with external pins" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_an_5g_usxgmii_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 44 Force Speed USXGMII with external pins" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl force_speed_an_2p5g_usxgmii_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 43 (Force Speed 10G USXGMII with CL74 FEC and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_10g_usxgmii_cl74_fec[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:09 speed, 10G = 3'b011
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0400,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0004,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 43 (Force AN 5G and Speed 5G USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_5g_speed5g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 43 (Force AN 5G and Speed 2P5G USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_5g_speed2p5g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 43 (Force AN 5G and Speed 1G USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_5g_speed1g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 43 (Force AN 5G and Speed 100M USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_5g_speed100m_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 43 (Force AN 5G and Speed 10M USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_5g_speed10m_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};


//Figure 43 (Force Speed an 2.5G and speed 2.5G USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_2p5g_speed2p5g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2.5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};


//Figure 43 (Force Speed an 2.5G and speed 1G USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_2p5g_speed1g_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};


//Figure 43 (Force Speed an 2.5G and speed 100M USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_2p5g_speed100m_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 43 (Force Speed an 2.5G and speed 10M USXGMII and Figure 62 (pcs enable) from 16nm_MERLINU_PHY_Core_1.3.pdf
// *** PS-Verified-Lin ***
prog_seq_tbl force_speed_an_2p5g_speed10m_usxgmii[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0000,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b0
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0000,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b0 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};


// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_an_enable[] = {
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //01:01 usxgmii_an_enable = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xc4b3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_an_speed_10g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:9: speed 3b'011
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_an_speed_5g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:9: speed 3b'101
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_an_speed_2p5g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:9: speed 3b'100
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_an_speed_1g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:9: speed 3b'010
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_an_speed_100m[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:9: speed 3b'001
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_an_speed_10m[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:9: speed 3b'001
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};


// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10G_Baud[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:09 speed, 10G = 3'b011
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5G_Baud[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2.5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5G_Baud[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:09 speed, 10G = 3'b011
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed2p5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed1g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed,1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed100m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed,100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed10m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed,10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed2p5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed1g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed100m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed10m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed2p5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled  
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed1g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed100m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed10m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:09 speed, 10G = 3'b011
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_master_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed5g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed2p5g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed1g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed100m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed10m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_master_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_master_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed5g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed2p5g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed1g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed100m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed10m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed2p5g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed1g_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed100m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed10m_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,   10,   10,    "usxgmii_master_mode"},                 //10:10 usxgmii_master_mode = 1'b1
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  USXGMII_AN_TIMER_REGS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:09 speed, 10G = 3'b011
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed5g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed2p5g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed1g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed100m_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_speed10m_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 56 USXGMII Auto Negotiation 10G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_10g_chan_info_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0000,    1,    0,    "ubaud"},                               //1:00 baud, 10G =2'b00
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_chan_info_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_chan_info_w_pin[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed5g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:09 speed, 5G = 3'b101
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed2p5g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed1g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed100m_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_5g_speed10m_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //1:00 baud, 5G =2'b01
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed2p5g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:09 speed, 2P5G = 3'b100
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed1g_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:09 speed, 1G = 3'b010
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed100m_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:09 speed, 100M = 3'b001
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 57 USXGMII Auto Negotiation 2P5G Baud + PCS enabled" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_2p5g_speed10m_chan_info[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x1000, 0x0000,   12,   12,    "usxgmii_sw_an_enable"},                //12:12 SW_an_enable=1'b0
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xC4B3, 0x0800, 0x0800,   11,   11,    "usxgmii_an_frc_sel"},                  //11:11 usxgmii_an_frc_sel=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1,
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed, 10M = 3'b000
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //1:00 baud, 2P5G =2'b10
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //0:00 usxgmii_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x2000, 0x2000,   13,   13,    "check_usxgmiiChanInfo"},               //13:13 check_usxgmiiChanInfo = 1'b1      
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0000,    8,    8,    "usxgmii_ext_sel"},                     //8:08 usxgmii_ext_sel =1'b0
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 tx_fec_en, 1'b0 FEC disabled
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //2:00 rx_fec_en 1'b0 FEC disabled
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};


// *** PS-Verified-Lin "
//Figure 58 USXGMII Auto Negotiation Restart (software)" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_restart_config[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 lineside_link=1'b1
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:09 speed=3'b0
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x1000, 0x1000,   12,   12,    "restart_an"},                          //12:12 restart_an = 1'b1 
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x1000, 0x0000,   12,   12,    "restart_an"},                          //12:12 usxgmii_an_restart = 1'b0 
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin "
//Figure 59 USXGMII Auto Negotiation Restart (with pins)" from 16nm_MERLINU_PHY_Core_1.3.pdf***// 
prog_seq_tbl usxgmii_an_restart_config_w_pin[] = {
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //1:1 usxgmii_an_enable = 1'b1 
  // drive usxgmiii_an_restart pin high
  // drive usxgmiii_an_restart pin low
  EMPTY_LAST_PROG_SEQUENCE 
};

//x Figure 53 USXGMII Auto Negotiation Master (merlinU) + Figure 59 PCS Enable (merlinU)
// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_an_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //00:00 usxgmii_an_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0040, 0x0040,    6,    6,    "an_fast_timer"},                       //06:06 an_fast_timer = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,    9,    9,    "usxgmii_master_mode"},                 //09:09 usxgmii_master_mode = 1'b1
//{"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //08:08 usxgmii_ext_sel = 1'b1; vaild only when external usxgmii_basePage_pin is expected to be used
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xc4b3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 TX fec en = 1'b0
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode = 3'h1
//{"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 3'h1 (optional)
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

//x Figure 54 USXGMII Auto Negotiation Slave (merlinU) + Figure 59 PCS Enable (merlinU)
// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_an_slave[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //06:06 SW_actual_speed_force_en = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //00:00 usxgmii_an_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0040, 0x0040,    6,    6,    "an_fast_timer"},                       //06:06 an_fast_timer = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0000,    9,    9,    "usxgmii_master_mode"},                 //09:09 usxgmii_master_mode = 1'b0
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //08:08 usxgmii_ext_sel = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control2",                         0x3,  0xc4b3, 0x2000, 0x2000,   13,   13,    "SW_ubaud_force_en"},                   //13:13 SW_ubaud_force_en
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0400, 0x0000,   10,   10,    "fec_enable"},                          //10:10 TX fec en = 1'b0
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xC454, 0x0007, 0x0001,    2,    0,    "block_sync_mode"},                     //02:00 RX fec block sync mode = 3'h1
//{"CL72_IEEE_TX.cl72it_BASE_R_PMD_control_register_150",       0x1,  0x0096, 0x0002, 0x0002,    1,    1,    "cl72_ieee_training_enable"},           //01:01 cl72_ieee_training_enable = 3'h1 (optional)
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

/* -----------------------------------------------------------------
   USXGMII-M (shortfin)
-------------------------------------------------------------------- */
// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_pcs_reset_and_enable[] = {
//{"TX_FED_txfir_misc_control1",               0x1,  0xD13D, 0x0001, 0x0000,        ?,        ?, "?"},  //00:00 40-bit path = 1'b0
//{"AN_X4_USXGMII_usxgmii_an_control2",        0x3,  0xc6b3, 0x0060, 0x0000,        ?,        ?, "?"},  //06:05 t_rep_wptr_latency = 0x0
  PCS_ENABLE_REGISTERS_SETTING
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_port_reset[] = {                                        //release resets for AN function
  {"RX_X4_Control0.r_usxgmii_dereplicate",                      0x3,  0xC45C, 0x0002, 0x0002,    1,    1,    "rstb_rx_port"},                        //01:01 rstb_rx_port
  {"TX_X4_Control0.misc",                                       0x3,  0xC433, 0x0010, 0x0010,    4,    4,    "rstb_tx_port"},                        //04:04 rstb_tx_port
  EMPTY_LAST_PROG_SEQUENCE
};


//x Figure 77 Switch PCS from A to B (shortfin)
// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_1port_10g[] = {
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //06:06 uport_ubuad_override = 1'b1
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0020, 0x0020,    5,    5,    "usxgmii_pcs_sel"},                     //05:05 usxgmii_pcs_sel = 1'b1
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x00f0, 0x0000,    7,    4,    "osr_mode_cl36_2p5"},                   //07:04 pmd_osr_mode_2 = 'h0 (i.e. osm1)	//BIT DEFINITION CHANGED 
//{"Main0.setup_1",                                             0x3,  0x9101, 0x000c, 0x0000,    4,    0,    "reserved1"},                           //03:02 uport_num = 2'b00 (1port)
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //[6] uport_baud_override_en = 1'b1 (10.3125G)
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //01:00 ubaud_rate = 2'b01;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_2port_10g[] = {
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //06:06 uport_ubuad_override = 1'b1
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0020, 0x0020,    5,    5,    "usxgmii_pcs_sel"},                     //05:05 usxgmii_pcs_sel = 1'b1
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x00f0, 0x0000,    7,    4,    "osr_mode_cl36_2p5"},                   //07:04 pmd_osr_mode_2 = 'h0 (i.e. osm1)	//BIT DEFINITION CHANGED 
//{"Main0.setup_1",                                             0x3,  0x9101, 0x000c, 0x0004,    4,    0,    "reserved1"},                           //03:02 uport_num = 2'b01 (2port)
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //[6] uport_baud_override_en = 1'b1 (10.3125G)
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //01:00 ubaud_rate = 2'b01;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_4port_10g[] = {
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //06:06 uport_ubuad_override = 1'b1
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0020, 0x0020,    5,    5,    "usxgmii_pcs_sel"},                     //05:05 usxgmii_pcs_sel = 1'b1
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x00f0, 0x0000,    7,    4,    "osr_mode_cl36_2p5"},                   //07:04 pmd_osr_mode_2 = 'h0 (i.e. osm1)	//BIT DEFINITION CHANGED 
// {"Main0.setup_1",                                             0x3,  0x9101, 0x000c, 0x0008,    4,    0,    "reserved1"},                           //03:02 uport_num = 2'b10 (4port)
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //[6] uport_baud_override_en = 1'b1 (10.3125G)
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0001,    1,    0,    "ubaud"},                               //01:00 ubaud_rate = 2'b01;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_1port_5g[] = {
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //06:06 uport_ubuad_override = 1'b1
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0020, 0x0020,    5,    5,    "usxgmii_pcs_sel"},                     //05:05 usxgmii_pcs_sel = 1'b1
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x0f00, 0x0100,   11,    8,    "osr_mode_cl36"},                       //11:08 pmd_osr_mode_4 = 'h1 (i.e. osm2)	//BIT DEFINITION CHANGED 
//{"Main0.setup_1",                                             0x3,  0x9101, 0x000c, 0x0000,    4,    0,    "reserved1"},                           //03:02 uport_num = 2'b00 (1port)
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //[6] uport_baud_override_en = 1'b1 (10.3125G)
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //01:00 ubaud_rate = 2'b10;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_2port_5g[] = {
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //06:06 uport_ubuad_override = 1'b1
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0020, 0x0020,    5,    5,    "usxgmii_pcs_sel"},                     //05:05 usxgmii_pcs_sel = 1'b1
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x0f00, 0x0100,   11,    8,    "osr_mode_cl36"},                       //11:08 pmd_osr_mode_4 = 'h1 (i.e. osm2)	//BIT DEFINITION CHANGED 
//{"Main0.setup_1",                                             0x3,  0x9101, 0x000c, 0x0004,    4,    0,    "reserved1"},                           //03:02 uport_num = 2'b01 (2port)
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //[6] uport_baud_override_en = 1'b1 (10.3125G)
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},                               //01:00 ubaud_rate = 2'b10;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_switch_pcs_from_A_to_B_1port_2p5g[] = {
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //06:06 uport_ubuad_override = 1'b1
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0020, 0x0020,    5,    5,    "usxgmii_pcs_sel"},                     //05:05 usxgmii_pcs_sel = 1'b1
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0xf000, 0x4000,   15,   12,    "osr_mode_cl73"},                       //15:12 pmd_osr_mode_8 = 'h4 (i.e. osm4)	//BIT DEFINITION CHANGED 
//{"Main0.setup_1",                                             0x3,  0x9101, 0x000c, 0x0000,    4,    0,    "reserved1"},                           //03:02 uport_num = 2'b00 (1port)
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //[6] uport_baud_override_en = 1'b1 (10.3125G)
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0003,    1,    0,    "ubaud"},                               //01:00 ubaud_rate = 2'b11;
  EMPTY_LAST_PROG_SEQUENCE
};

//x Figure 78 Switch PCS from B to A (shortfin)
// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_switch_pcs_from_B_to_A[] = {
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0080, 0x0000,    7,    7,    "usxgmii_pcs_sel_ckgate_disable"},      //05:05 uxsgmii_pcs_sel = 1'b0; when usxgmii_pcs_sel_pin = 0
  {"Main0.setup_1",                                             0x3,  0x9101, 0x0040, 0x0040,    6,    6,    "Uport_baud_override_en"},              //05:05 uxsgmii_pcs_sel = 1'b0; [6] uport_baud_override_en = 1'b1 ; when usxgmii_pcs_sel_pin = 0
//{"TX_FED_txfir_misc_control1",               0x1,  0xD13D, 0x0001, 0x0001,        ?,        ?, "?"},  //00:00 20-bit path = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_force_speed_10g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x0036,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x36;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_force_speed_5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x0035,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x35;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_force_speed_2p5g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x0030,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x30;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_force_speed_1g[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x002f,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x2f;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_force_speed_100m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x002e,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x2e;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_force_speed_10m[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0040,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b1
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x0037,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x37;
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_mp_an_slave[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x0036,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x36;
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //00:00 usxgmii_an_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0040, 0x0040,    6,    6,    "an_fast_timer"},                       //06:06 an_fast_timer = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0000,    9,    9,    "usxgmii_master_mode"},                 //09:09 usxgmii_master_mode = 1'b0
//{"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //08:08 usxgmii_ext_sel = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-NOTVerified ***
prog_seq_tbl usxgmii_mp_an_master[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},            //[6] SW_actual_speed_force_en = 1'b0
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x003f, 0x0036,    5,    0,    "SW_actual_speed"},                     //05:00 SW_actual_speed = 0x36;
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0001, 0x0001,    0,    0,    "usxgmii_mode"},                        //00:00 usxgmii_an_mode = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0040, 0x0040,    6,    6,    "an_fast_timer"},                       //06:06 an_fast_timer = 1'b1
  {"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0200, 0x0200,    9,    9,    "usxgmii_master_mode"},                 //09:09 usxgmii_master_mode = 1'b1
//{"AN_X4_USXGMII.usxgmii_an_control1",                         0x3,  0xC4B2, 0x0100, 0x0100,    8,    8,    "usxgmii_ext_sel"},                     //08:08 usxgmii_ext_sel = 1'b1; vaild only when external usxgmii_basePage_pin is expected to be used
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_mp_an_enable[] = {
  {"AN_X4_USXGMII.usxgmii_an_control0",                         0x3,  0xC4B1, 0x0002, 0x0002,    1,    1,    "usxgmii_an_enable"},                   //01:01 usxgmii_an_enable = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_mp_an_speed_10g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0600,   11,    9,    "speed"},                               //11:9: speed 3b'011
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_mp_an_speed_5g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0A00,   11,    9,    "speed"},                               //11:9: speed 3b'101
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_mp_an_speed_2p5g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0800,   11,    9,    "speed"},                               //11:9: speed 3b'100
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_mp_an_speed_1g[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0400,   11,    9,    "speed"},                               //11:9: speed 3b'010
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_mp_an_speed_100m[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0200,   11,    9,    "speed"},                               //11:9: speed 3b'001
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// *** PS-Verified-Lin ***
prog_seq_tbl set_usxgmii_mp_an_speed_10m[] = {
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x8000, 0x8000,   15,   15,    "link"},                                //15:15 link; 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x1000, 0x1000,   12,   12,    "duplex"},                              //12:12 duplex 
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0E00, 0x0000,   11,    9,    "speed"},                               //11:9: speed 3b'001
  {"AN_X4_USXGMII.usxgmii_an_config",                           0x3,  0xC4B0, 0x0001, 0x0001,    0,    0,    "sgmii"},                               //sgmii = 1'b1
  EMPTY_LAST_PROG_SEQUENCE
};

// Figure 84 Global PCS Loopback (USXGMII_M_PHY_1_shortfin-002_spec.pdf)
// *** PS-Verified-Lin "
//Figure 64 Global PCS Loopback" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl usxgmii_mp_global_pcs_loopback[] = {
  {"Main0.loopback_control",                                    0x3,  0x9109, 0x00F0, 0x0010,    7,    4,    "local_pcs_loopback_enable"},           //04:04 local_pcs_loopback_enable = 1'b1 per Logical lane
  EMPTY_LAST_PROG_SEQUENCE
}; 

// Figure 85 Remmote PCS Loopback (USXGMII_M_PHY_1_shortfin-002_spec.pdf)
// *** PS-Verified-Lin "
//Figure 66 Remote PCS Loopback" from 16nm_MERLINU_PHY_Core_1.3.pdf***
prog_seq_tbl usxgmii_mp_remote_pcs_loopback[] = {
  {"Main0.loopback_control",                                    0x3,  0x9109, 0xF000, 0x1000,   15,   12,    "remote_pcs_loopback_enable"},          //12:12 remote_pcs_loopback_enable = 1'b1 per Logical lane
  EMPTY_LAST_PROG_SEQUENCE
}; 

prog_seq_tbl force_speed_10g_ieee[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x8000, 0x8000,   15,   15,    "force_ieee_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},
  {"TX_X1_Control0.pmd_osr_mode",                               0x3,  0x9202, 0x000f, 0x0000,    3,    0,    "osr_mode_cl49"},
  {"dev3_ieee0.PCS_ieeeControl1",                               0x3,  0x0000, 0x2000, 0x2000,   13,   13,    "SpeedSelection0"},
  {"dev3_ieee0.PCS_ieeeControl1",                               0x3,  0x0000, 0x0040, 0x0040,    6,    6,    "SpeedSelection1"},
  {"dev3_ieee0.PCS_ieeeControl1",                               0x3,  0x0000, 0x003c, 0x0000,    5,    2,    "SpeedSelection2"},
  {"dev1_ieee0.PMD_ieeeControl2",                               0x1,  0x0007, 0x007f, 0x000b,    6,    0,    "PMA_PMD_typeSel"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0080, 0x0080,    7,    7,    "mac_creditenable"},
  EMPTY_LAST_PROG_SEQUENCE
}; 

prog_seq_tbl force_speed_1g_ieee[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x8000, 0x8000,   15,   15,    "force_ieee_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x4000, 0x4000,   14,   14,    "force_ieee_cl22_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x3000, 0x1000,   13,   12,    "use_ieee_reg_ctrl_sel"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x0100, 0x0100,    8,    8,    "duplex"},
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xc454, 0x0007, 0x0003,    2,    0,    "block_sync_mode"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x0040, 0x0040,    6,    6,    "manual_speed1"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x2000, 0x0000,   13,   13,    "manual_speed0"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0080, 0x0080,    7,    7,    "mac_creditenable"},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl force_speed_100m_ieee[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x8000, 0x8000,   15,   15,    "force_ieee_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x4000, 0x4000,   14,   14,    "force_ieee_cl22_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x3000, 0x1000,   13,   12,    "use_ieee_reg_ctrl_sel"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x0100, 0x0100,    8,    8,    "duplex"},
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xc454, 0x0007, 0x0003,    2,    0,    "block_sync_mode"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x0040, 0x0000,    6,    6,    "manual_speed1"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x2000, 0x2000,   13,   13,    "manual_speed0"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0080, 0x0080,    7,    7,    "mac_creditenable"},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl force_speed_10m_ieee[] = {
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0800, 0x0000,   11,   11,    "credit_sw_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x8000, 0x8000,   15,   15,    "force_ieee_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0040, 0x0000,    6,    6,    "SW_actual_speed_force_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x4000, 0x4000,   14,   14,    "force_ieee_cl22_SpeedSel_en"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x3000, 0x1000,   13,   12,    "use_ieee_reg_ctrl_sel"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x0100, 0x0100,    8,    8,    "duplex"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0400, 0x0400,   10,   10,    "replication_cnt"},
  {"RX_X4_Control0.decode_control_0",                           0x3,  0xc454, 0x0007, 0x0003,    2,    0,    "block_sync_mode"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x0040, 0x0000,    6,    6,    "manual_speed1"},
  {"CL22_B0.MIICntl",                                           0x0,  0x0000, 0x2000, 0x0000,   13,   13,    "manual_speed0"},
  {"Digital.MiscDigControl",                                    0x3,  0xc30b, 0x0080, 0x0080,    7,    7,    "mac_creditenable"},
  EMPTY_LAST_PROG_SEQUENCE
};

//Figure 70 (NOT USED) Load Firmware (16nm_MERLINU_PHY_Core_1.3.pdf)
//Figure 71 (NOT USED) Software Core Reset (16nm_MERLINU_PHY_Core_1.3.pdf)
//Figure 75 (NOT USED) USXGMII AN Speed Up (16nm_MERLINU_PHY_Core_1.3.pdf)

prog_seq_tbl force_speed_common[] = {
  {"rx rstb_lane",                                              0x3,  0xc457, 0x0001, 0x0001,    0,    0,    ""},
  {"mac_creditenable",                                          0x3,  0xc30B, 0x0080, 0x0080,    7,    7,    ""},
  {"tx rstb_lane enable_tx_lane",                              0x3,  0xc433, 0x0003, 0x0003,    1,    0,    ""},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl reset_rxtx_port[] = {
  {"rstb_rx_port",                                              0x3,  0xc45c, 0x0002, 0x0002,    1,    1,    ""},
  {"rstb_tx_port",                                              0x3,  0xc433, 0x0010, 0x0010,    4,    4,    ""},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl set_usxgmii_2p5g_baud[] = {
  {"Digital.Control1000X2",                                     0x3,  0xC301, 0x0003, 0x0002,    1,    0,    "ubaud"},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl cl127_2p5g_credit_10p3125_VCO[] = {
  {"reg2p5G_CL127 credit 0 register",                           0x3,  0x9284, 0x3fff, 0x0021,   13,    0,    "reg2p5G_CL127_ClockCount0"},
  {"reg2p5G_CL127 credit 1 register",                           0x3,  0x9285, 0x1fff, 0x0010,   12,    0,    "reg2p5G_CL127_CGC"},
  {"reg2p5G_CL127 credit 2 register",                           0x3,  0x9286, 0x00ff, 0x0000,    7,    0,    "reg2p5G_CL127_ClockCount1"},
  {"reg2p5G_CL127 credit 2 register",                           0x3,  0x9286, 0xff00, 0x0100,   15,    8,    "reg2p5G_CL127_loopcnt0"},
  {"reg2p5G_CL127 credit 3 register",                           0x3,  0x9287, 0x003f, 0x0000,   12,    0,    "reg2p5G_CL127_loopcnt1"},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl auto_neg_2p5g_kx_cl73[] = {
  {"Miscellaneous digital  control1 register",                  0x3,  0xC30B, 0x3000, 0x0000,   13,   12,    "use_ieee_reg_ctrl_sel"}, 
  {"AN advertisement 2",                                        0x7,  0x0012, 0x0001, 0x0001,    0,    0,    "speed advertisement"},  
  {"AN advertisement 0",                                        0x7,  0x0010, 0x001f, 0x0001,    4,    0,    "base selector"},
  {"AN Control 1",                                              0x7,  0x0000, 0x1000, 0x1000,   12,   12,    "AN enable"}, 
  {"Miscellaneous digital  control1 register",                  0x3,  0xC30B, 0x0800, 0x0000,   11,   11,    ""},
  {"Miscellaneous digital  control1 register",                  0x3,  0xC30B, 0x0040, 0x0000,    6,    6,    ""},
  {"pma_control_0 register",                                    0x3,  0xC457, 0x0001, 0x0001,    0,    0,    "rx rstb_lane"},
  {"Miscellaneous digital  control1 register",                  0x3,  0xC30B, 0x0080, 0x0080,    7,    7,    "mac_creditenable"}, 
  {"Misc register",                                             0x3,  0xC433, 0x0003, 0x0003,    1,    0,    ""},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl pcs_enable_per_lane[] = {
  {"rx rstb_lane",                                              0x3,  0xc457, 0x0001, 0x0001,    0,    0,    ""},
  {"mac_creditenable",                                          0x3,  0xc30b, 0x0080, 0x0080,    7,    7,    ""},
  {"enable_tx_lane",                                            0x3,  0xc433, 0x0001, 0x0001,    0,    0,    ""},
  {"rst_tx_lane",                                               0x3,  0xc433, 0x0002, 0x0002,    1,    1,    ""},
  EMPTY_LAST_PROG_SEQUENCE
};

prog_seq_tbl rst_rx_tx_port_usxgmii[] = {
  {"rst_rx_port(if USXGMII)",                                   0x3,  0xc45c, 0x0002, 0x0002,    1,    1,    ""},
  {"rst_tx_port(if USXGMII)",                                   0x3,  0xc433, 0x0010, 0x0010,    4,    4,    ""},
  EMPTY_LAST_PROG_SEQUENCE
};
