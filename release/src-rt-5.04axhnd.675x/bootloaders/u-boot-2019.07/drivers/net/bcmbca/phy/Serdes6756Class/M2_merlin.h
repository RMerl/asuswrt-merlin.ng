// MERLIN_PHY_2.NONE.REL_09152016 : 28nm_Merlin_PHY_Core.pdf Version 1.19 (September 14, 2016)

/*
Table format:
"register name", "devad", "register address", "bit mask", "value"
*/

/* -----------------------------------------------------------------
   PMD section
-------------------------------------------------------------------- */
//Figure 18 Initialization for 9.375GHz VCO
prog_seq_tbl Initialize_9p375_VCO[] = {
  {"CL36 OS Mode Register",     0x3, 0x9201, 0xffff, 0x2999},  //os_mode_cl36 [15:00]
  {"PMD OSR Mode Register",     0x3, 0x9202, 0xffff, 0x6620},  //pmd_osr_mode [15:00]
  {"reg2p5G credit 0 register", 0x3, 0x9270, 0x3fff, 0x0003},  //reg2p5G_ClockCount0 [13:00]
  {"reg2p5G credit 1 register", 0x3, 0x9271, 0x1fff, 0x0001},  //reg2p5G_CGC [12:00]
  {"reg2p5G credit 2 register", 0x3, 0x9272, 0xffff, 0x0102},  //reg2p5G_loopcnt0/ClockCount1 [15:00]
  {"reg2p5G credit 3 register", 0x3, 0x9273, 0x1fff, 0x0006},  //reg2p5G_loopcnt1 [12:00]
  {"reg2p5G modulo register",   0x3, 0x9233, 0x01ff, 0x0001},  //reg2p5G_modulo [08:00]
  {"reg1G credit 0 register",   0x3, 0x926A, 0x3fff, 0x0019},  //reg1G_ClockCount0 [13:00]
  {"reg1G credit 1 register",   0x3, 0x926B, 0x1fff, 0x0004},  //reg1G_CGC [12:00]
  {"reg1G credit 2 register",   0x3, 0x926C, 0xffff, 0x0105},  //reg1G_loopcnt0/ClockCount1 [15:00]
  {"reg1G credit 3 register",   0x3, 0x926D, 0x1fff, 0x000a},  //reg1G_loopcnt1 [12:00]
  {"reg1G modulo register",     0x3, 0x9232, 0x01ff, 0x0003},  //reg1G_modulo [08:00]
  {"reg100M credit 0 register", 0x3, 0x9265, 0x3fff, 0x0177},  //reg100M_ClockCount0 [13:00]
  {"reg100M credit 3 register", 0x3, 0x9268, 0x1fff, 0x0017},  //reg100M_CGC [12:00]
  {"reg100M credit 1 register", 0x3, 0x9266, 0xffff, 0x0100},  //reg100M_loopcnt0/ClockCount1 [15:00]
  {"reg100M credit 4 register", 0x3, 0x9269, 0xe000, 0x0000},  //reg100M_loopcnt1_Hi [15:13]
  {"reg100M credit 3 register", 0x3, 0x9268, 0xe000, 0x0000},  //reg100M_loopcnt1_Lo [15:13]
  {"reg100M credit 2 register", 0x3, 0x9267, 0x3fff, 0x004b},  //reg100M_PCS_ClockCount1 [13:00]
  {"reg100M credit 4 register", 0x3, 0x9269, 0x1fff, 0x0025},  //reg100M_PCS_CGC [12:00]
  {"reg100M modulo register",   0x3, 0x9231, 0x01ff, 0x0016},  //reg100M_modulo [08:00]
  {"reg10M credit 0 register",  0x3, 0x9260, 0x3fff, 0x0753},  //reg10M_ClockCount0 [13:00]
  {"reg10M credit 3 register",  0x3, 0x9263, 0x1fff, 0x00ea},  //reg10M_CGC [12:00]
  {"reg10M credit 1 register",  0x3, 0x9261, 0xffff, 0x0100},  //reg10M_loopcnt0/ClockCount1 [07:00]
  {"reg10M credit 4 register",  0x3, 0x9264, 0xe000, 0x0000},  //reg10M_loopcnt1_Hi [15:13]
  {"reg10M credit 3 register",  0x3, 0x9263, 0xe000, 0x0000},  //reg10M_loopcnt1_Lo [15:13]
  {"reg10M credit 2 register",  0x3, 0x9262, 0x3fff, 0x004b},  //reg10M_PCS_ClockCount1 [13:00]
  {"reg10M credit 4 register",  0x3, 0x9264, 0x1fff, 0x0025},  //reg10M_PCS_CGC [12:00]
  {"reg10M modulo register",    0x3, 0x9230, 0x01ff, 0x00e9},  //reg10M_modulo [08:00]
  {"",                          0x0, 0x0000, 0x0000, 0x0000}
}; 
//Figure 19 PMD setup 156.25MHz, 10.3125GHz VCO
prog_seq_tbl PMD_setup_156p25_10p3125_VCO[] = {
  {"PLL_CTRL_3",                 0x1,  0xd0b3, 0xf800, 0xa000},  //div[15:11] = 0x14
  {"main control register",      0x3,  0x9100, 0xe000, 0x6000},  //refclk_sel[15:13] = 3'h3
  //{"TLB TX Misc. Control",       0x1,  0xd0e3, 0x0002, 0x0002},  //tx_pcs_native_ana_frmt_en[1] = 0x1; bypass pmd tx oversampling (per lane)
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 20 PMD Setup 156.25MHz, 9.375GHz VCO
prog_seq_tbl PMD_setup_156p25_9p375_VCO[] = {
  {"PLL_CTRL_3",                 0x1,  0xd0b3, 0xf800, 0xe000},  //div[15:11] = 0x1c
  {"main control register",      0x3,  0x9100, 0xe000, 0x6000},  //refclk_sel[15:13] = 3'h3
  //{"TLB TX Misc. Control",       0x1,  0xd0e3, 0x0002, 0x0002},  //tx_pcs_native_ana_frmt_en[1] = 0x1; bypass pmd tx oversampling (per lane)
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 
/*
    MERLIN_PMD_USER: MERLIN PMD User Blocks (Clause 22, AER DeviceType = 1 OR Clause 45, Device = 1) 
    (4 copies except 1 copy for following: 
    AMSCOM, DIGCOM, PATT_GEN, PLL_CAL, TXCOM, MICRO, MDIO_MMDSEL_AER, MDIO_BLK_ADDR )
*/
//Figure 23 PMD Setup 50MHz, 10.3125GHz VCO
prog_seq_tbl PMD_setup_50_10p3125_VCO[] = {
  {"PLL_CTRL_9",                 0x1,  0xd0b9, 0x0001, 0x0000}, // mmb_resetb[0] = 0x0
  {"PLL_CTRL_3",                 0x1,  0xd0b3, 0xf800, 0x2000}, // div[15:11] = 0x04
  {"PLL_CTRL_6",                 0x1,  0xd0b6, 0xf000, 0x0000}, // i_ndiv_frac_l[15:12] = 0x0
  {"PLL_CTRL_7",                 0x1,  0xd0b7, 0x3FFF, 0x1000}, // i_ndiv_frac_h [13:0] = 0x1000
  //{"PLL_CTRL_8",                 0x1,  0xd0b8, 0x4000, 0x4000}, // i_pll_sdm_pwrdnb[14] = 0x1
  //{"PLL_CTRL_8",                 0x1,  0xd0b8, 0x2000, 0x2000}, // mmd_en[13] = 0x1
  //{"PLL_CTRL_8",                 0x1,  0xd0b8, 0x1000, 0x0000}, // mmd_prsc4or5pwdb[12] = 0x0
  //{"PLL_CTRL_8",                 0x1,  0xd0b8, 0x0800, 0x0800}, // mmd_prsc8or9pwdb[11] = 0x1
  //{"PLL_CTRL_8",                 0x1,  0xd0b8, 0x0400, 0x0400}, // mmd_div_range[10] = 0x1
  //{"PLL_CTRL_8",                 0x1,  0xd0b8, 0x03ff, 0x00ce}, // i_ndiv_int[9:0] = 0x0ce
  {"PLL_CTRL_8",                 0x1,  0xd0b8, 0x7fff, 0x6cce}, // merged [14]=0x1, [13]=0x1, [12]=0x0, [11]=0x1, [10]=0x1; [9:0]=0x0ce
  {"PLL_CTRL_9",                 0x1,  0xd0b9, 0x0007, 0x0003}, // i_pll_frac_mode, mmd_resetb [2:0] = 0x3
  {"main control register",      0x3,  0x9100, 0xe000, 0xc000}, // refclk_sel[15:13] = 0x6
  //{"TLB TX Misc. Control",       0x1,  0xd0e3, 0x0002, 0x0002}, //tx_pcs_native_ana_frmt_en[1] = 0x1; bypass pmd tx oversampling (per lane)
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 24 PMD Setup 50MHz, 9.375GHz VCO
prog_seq_tbl PMD_setup_50_9p375_VCO[] = {
  {"PLL_CTRL_3",                 0x1,  0xd0b3, 0xf800, 0xd000},  //div[15:11] = 0x1a
  {"main control register",      0x3,  0x9100, 0xe000, 0xc000},  //refclk_sel[15:13] = 3'h6
  //{"TLB TX Misc. Control",       0x1,  0xd0e3, 0x0002, 0x0002},  //tx_pcs_native_ana_frmt_en[1] = 0x1; bypass pmd tx oversampling (per lane)
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl PMD_set_bypass_tx_osr_lane[] = {
  {"TLB TX Misc. Control",       0x1,  0xd0e3, 0x0002, 0x0002},  //tx_pcs_native_ana_frmt_en[1] = 0x1; bypass pmd tx oversampling (per lane)
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl PMD_clr_bypass_tx_osr_lane[] = {
  {"TLB TX Misc. Control",       0x1,  0xd0e3, 0x0002, 0x0000},  //tx_pcs_native_ana_frmt_en[1] = 0x0; bypass pmd tx oversampling (per lane)
  {"",                           0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   PCS section  - the PCS (A) design is from merlinU
-------------------------------------------------------------------- */

/* -----------------------------------------------------------------
   CL36
-------------------------------------------------------------------- */
//Figure 28 Force Speed 2.5G
prog_seq_tbl force_speed_2p5g_vco9g[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x0043},  //merged [11]=0x0, [06]=0x1 [00]=0x3
  {"cl36_rx_0 register",                       0x3,  0xC456, 0x0010, 0x0010},  //spd_2p5G_sel[04] = 1'b1 for 2.5G at 9.375G VCO
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl force_speed_2p5g_vco10g[]= {
  {"Credit",                                   0x3,  0x9270, 0x3fff, 0x0021},  // Credit needs to be set on the top
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x0043},  //merged [11]=0x0, [06]=0x1 [00]=0x3
  {"cl36_rx_0 register",                       0x3,  0xC456, 0x0010, 0x0010},  //spd_2p5G_sel[04] = 1'b1 for 2.5G at 9.375G VCO
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//Figure 29 Force Speed 1.0G
prog_seq_tbl force_speed_1g[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x0042},  // merged [11]=0x0, [06]=0x1, [05:00]=0x2 
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  // 00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  // 07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  // merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
//Figure 30 Force Speed 100M
prog_seq_tbl force_speed_100m[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x0041},  // merged [11]=0x0, [06] = 0x1, [00]=0x1
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  // 00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  // 07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  // merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
//Figure 31 Force Speed 10M
prog_seq_tbl force_speed_10m[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x0040},  //merged [11] and [06:00] 
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
//Figure 34 Auto Negotiation 1G KX - IEEE CL73
prog_seq_tbl auto_neg_1g_kx_ieee_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0020},  //15:05 speed advertisement = 11'h001  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 35 Auto Negotiation 1G -User Space CL73
prog_seq_tbl auto_neg_1g_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0020, 0x0020},  //05:05 speed advertisement = 1'b1  
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN ENABLES",                               0x3,  0xc480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 36 Auto Negotiation 1G - IEEE CL37
prog_seq_tbl auto_neg_1g_ieee_cl37[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x1000},  //13:12 use_ieee_reg_ctrl_sel = 2'b01
  {"IEEE AN advertised abilities register",    0x0,  0x0004, 0x0020, 0x0020},  //05:05 duplex = 1'b1 
  {"MII control register",                     0x0,  0x0000, 0x1000, 0x1000},  //12:12 AN = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 37 Auto Negotiation 1G - User Space CL37
prog_seq_tbl auto_neg_1g_user_cl37[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0010, 0x0010},  //04:04 duplex = 1'b1
  {"AN ENABLES",                               0x3,  0xc480, 0x0040, 0x0040},  //06:06 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
/* -----------------------------------------------------------------
   CL49
-------------------------------------------------------------------- */
//Figure 27 Force Speed 10G
prog_seq_tbl force_speed_10g_R[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x004f},  //merged [11]=0x0, [06]=0x1 [00]=0xf
  //{"CL49 OS Mode Register",                    0x3,  0x9200, 0x000f, 0x0001},  // os_mode_cl49 [3:0] = 4'h1  //FIXME 5G experiment
  //{"PMD OSR Mode Register",                    0x3,  0x9202, 0x000f, 0x0001},  // pmd_osr_mode [3:0] = 4'h1  //FIXME 5G experiment
  //{"reg10G credit 1 register",                 0x3,  0x9275, 0x01ff, 0x0008},  // reg10G_CGC [12:0] = 4'h8   //FIXME 5G experiment
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 27 Force Speed 10G
prog_seq_tbl force_speed_10g_R_cl74_fec[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x004f},  //merged [11]=0x0, [06]=0x1 [00]=0xf
  {"Misc register",                            0x3,  0xc433, 0x0400, 0x0400},  //10:10 TX fec en (optional) = 1'b1
  {"Decoder control 0 register",               0x3,  0xC454, 0x0007, 0x0004},  //02:00 RX fec block sync mode (optional) = 3'h4
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 32 Auto Negotiation 10G KR - IEEE CL73
prog_seq_tbl auto_neg_10g_kr_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0080},  //15:05 speed advertisement = 11'h004  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  //{"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0xc000},  //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 32 Auto Negotiation 10G KR - IEEE CL73
prog_seq_tbl auto_neg_10g_kr_cl73_cl74_fec[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x0080},  //15:05 speed advertisement = 11'h004  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0xc000},  //15:14 FEC support (optional) = 2'h1 or 2'h3 (v)
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 33 Auto Negotiation 10G - User Space CL73
prog_seq_tbl auto_neg_10g_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0008, 0x0008},  //03:03 speed advertisement = 1'b1  
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  //{"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0300, 0x0300},  //09:08 fec = 2'b11  
  {"AN ENABLES",                               0x3,  0xc480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 33 Auto Negotiation 10G - User Space CL73
prog_seq_tbl auto_neg_10g_user_cl73_cl74_fec[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0008, 0x0008},  //03:03 speed advertisement = 1'b1  
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0300, 0x0300},  //09:08 fec = 2'b11  
  {"AN ENABLES",                               0x3,  0xc480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
/* -----------------------------------------------------------------
   CL129
-------------------------------------------------------------------- */
//Figure 24 Force Speed 5G (R-mode) (blackfin)
prog_seq_tbl force_speed_5g_R[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x0069},  //merged [11] and [06:00]
  //{"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0001},  //pmd_osr_mode [3:0] = 4'h1  //ALEX: 0x9206[7:4] osr_mode_cl49_5g
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 25 Force Speed 2p5G (R-mode) (blackfin)
prog_seq_tbl force_speed_2p5g_R[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x087f, 0x0068},  //merged [11] and [06:00]
  //{"pmd_osr_mode",                             0x3,  0x9202, 0x000f, 0x0004},  // pmd_osr_mode [3:0] = 4'h4 //ALEX: 0x9206[11:8] osr_mode_cl49_2p5g
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_5g_kr_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 2",                       0x7,  0x0012, 0x0002, 0x0002},  //01:01 speed advertisement = 1'b1  //added by ALEX  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_5g_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x1000, 0x1000},  //12:12 speed advertisement = 1'b1 //added by ALEX
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN ENABLES",                               0x3,  0xc480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   CL73
-------------------------------------------------------------------- */
prog_seq_tbl auto_neg_ieee_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x0000},  //13:12 use_ieee_reg_ctrl_sel = 2'b00
  {"AN advertisement 1",                       0x7,  0x0011, 0xffe0, 0x00a0},  //15:05 speed advertisement = 11'h005  
  {"AN advertisement 2",                       0x7,  0x0012, 0x0002, 0x0002},  //01:01 speed advertisement = 1'b1  //added by ALEX  
  {"AN advertisement 0",                       0x7,  0x0010, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"AN advertisement 2",                       0x7,  0x0012, 0xc000, 0x4000},  //15:14 FEC support (optional) = 2'h1
  {"AN Control 1",                             0x7,  0x0000, 0x1000, 0x1000},  //12:12 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
prog_seq_tbl auto_neg_user_cl73[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x1028, 0x1028},  //12,05,03 speed advertisement = 1'b1
  {"CL73 BASE PAGE ABILITIES REG 1",           0x3,  0xC485, 0x001f, 0x0001},  //04:00 base selector = 5'h01  
  {"CL73 BASE PAGE ABILITIES REG 0",           0x3,  0xC486, 0x0100, 0x0100},  //09:08 fec = 2'b01; supported but not requested 
  {"AN ENABLES",                               0x3,  0xc480, 0x0100, 0x0100},  //08:08 AN enable = 1'b1  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0840, 0x0000},  //merged [11]=0x0, [06]=0x0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07:07 mac_creditenable = 1'b1
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //merged [01] = 1'b1, [00] = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   SGMII 
-------------------------------------------------------------------- */
//Figure 38 Auto Negotiatin 1G - SGMII Master
prog_seq_tbl sgmii_an_speed_1g_master[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
//  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0004, 0x0004},  //02:02 duplex = 1'b1
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0003, 0x0002},  //01:00 speed = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0200, 0x0200},  //09:09 master = 1'b1
  {"AN ENABLES",                               0x3,  0xc480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
//Figure 39 Auto Negotiation 100M - SGMII Master
prog_seq_tbl sgmii_an_speed_100m_master[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0004, 0x0004},  //02:02 duplex = 1'b1
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0003, 0x0001},  //01:00 speed = 2'b01
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0200, 0x0200},  //09:09 master = 1'b1
  {"AN ENABLES",                               0x3,  0xc480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
//Figure 40 Auto Negotiation 10M - SGMII Master
prog_seq_tbl sgmii_an_speed_10m_master[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0004, 0x0004},  //02:02 duplex = 1'b1
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0003, 0x0000},  //01:00 speed = 2'b00
  {"CL37 BASE PAGE ABILITIES",                 0x3,  0xC481, 0x0200, 0x0200},  //09:09 master = 1'b1
  {"AN ENABLES",                               0x3,  0xc480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
//Figure 41 Auto Negotiation 10M/100M/1G - SGMII Slave
prog_seq_tbl sgmii_an_slave[] = {
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x3000, 0x2000},  //13:12 use_ieee_reg_ctrl_sel = 2'b10
  {"AN ENABLES",                               0x3,  0xc480, 0x00c0, 0x00c0},  //07:06 AN enable = 2'b11  
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0800, 0x0000},  //11:11 credit_sw_en = 1'b0
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0040, 0x0000},  //06:06 SW_actual_speed_force_en = 1'b0
  {"pma_control_0 register",                   0x3,  0xc457, 0x0001, 0x0001},  //00:00 rx rstb_lane = 1'b1 
  {"Miscellaneous digital  control1 register", 0x3,  0xc30b, 0x0080, 0x0080},  //07: per-lane ctrl; enables credits to be generated for the MAC
  {"Misc register",                            0x3,  0xc433, 0x0003, 0x0003},  //01:01 rstb_tx_lan; 00:00 enable_tx_lane
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

prog_seq_tbl sgmii_an_1g_credit_vco10p3125g[] = {  //41*3+21*2 = 165 ; Q(41/5)=8, Q(21/5)=4, credit = 8*3+4*2 = 32; avg. freq = 644.53125*(32/165)=125MHz
  {"reg1G credit 0 register",                  0x3,  0x926A, 0x3fff, 0x0029},  //reg1G_ClockCount0 [13:00] = 41
  {"reg1G credit 1 register",                  0x3,  0x926B, 0x1fff, 0x0005},  //reg1G_CGC [12:00] = 5
  {"reg1G credit 2 register",                  0x3,  0x926C, 0xffff, 0x0315},  //reg1G_loopcnt0 [15:08]= 3 ClockCount1 [07:00] = 21
  {"reg1G credit 3 register",                  0x3,  0x926D, 0x1fff, 0x0002},  //reg1G_loopcnt1 [12:00] = 2
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};

/* -----------------------------------------------------------------
   Misc. Programming Sequence
-------------------------------------------------------------------- */
//Figure 42 Change Speed
prog_seq_tbl change_speed[] = {
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0000},  //01:01 ln_dp_s_rstb = 1'b0 (AL: move here to get rid of junk on GMII RXD after lane_reset)
  {"PCS Control 1",                            0x3,  0x0000, 0x8000, 0x8000},  //15:15 lane reset = 1'b1
  //{"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0000},  //01:01 ln_dp_s_rstb = 1'b0
  {"timeout_100ns",                            0x3,  0x0000, 0x0000, 0x0000}, // timeout_ns 300ns; observed ~250us strech in waveform
  {"TLB TX Misc. Control",                     0x1,  0xd0e3, 0x0002, 0x0002},  //01:01 bypass pmd tx oversampling (per lane) = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 43 Datapath Reset, core
prog_seq_tbl datapath_get_core_out_reset[] = {
  {"RESET_CONTROL_CORE_DP",                    0x1,  0xd0f2, 0x0001, 0x0001},  //00:00 core_dp_s_rstb = 1'b1; reset value = 1'b0
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//Figure 43 Datapath Reset, core
prog_seq_tbl datapath_put_core_in_reset[] = {
  {"RESET_CONTROL_CORE_DP",                    0x1,  0xd0f2, 0x0000, 0x0001},  //00:00 core_dp_s_rstb = 1'b1; reset value = 1'b0
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//Figure 44 Datapath Reset, per lane
prog_seq_tbl datapath_reset_lane[] = {
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0002},  //01:01 ln_dp_s_rstb = 1'b1; clear active low reset
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//~Figure 44 Datapath Reset, per lane
prog_seq_tbl en_datapath_reset_lane[] = {
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0000},  //01:01 ln_dp_s_rstb = 1'b0; enable active low reset
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 45 LPI Enable
prog_seq_tbl lpi_enable[] = {
  {"pcs control 0 register",                   0x3,  0xc450, 0x0004, 0x0004},  //02:02 LPI_ENABLE = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//???
prog_seq_tbl global_pcs_loopback_lane[] = {
  {"SIGDET_CTRL_1",                            0x1,  0xd0c1, 0x0180, 0x0180},   //08:07 singal_detect_frc = 2'b11
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
};
//Figure 46 Global PCS Loopback
prog_seq_tbl global_pcs_loopback[] = {
  {"SIGDET_CTRL_1",                            0x1,  0xd0c1, 0x0180, 0x0180},   //08:07 singal_detect_frc = 2'b11
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0080, 0x0080},   //07:07  local_pcs_loopback_enable_ln3 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0040, 0x0040},   //06:06  local_pcs_loopback_enable_ln2 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0020, 0x0020},   //05:05  local_pcs_loopback_enable_ln1 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x0010, 0x0010},   //04:04  local_pcs_loopback_enable_ln0 = 1'b1
  {"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x00f0, 0x00f0},   //merged [07:04]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 47 Global PMD Loopback
prog_seq_tbl global_pmd_loopback[] = {
  {"SIGDET_CTRL_1",                            0x1,  0xd0c1, 0x0180, 0x0180},   //08:07 singal_detect_frc = 2'b11
  {"Digital Loopback Control",                 0x1,  0xd0d2, 0x0001, 0x0001},   //00:00 dig_lpbk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 48 Remote PCS Loopback
prog_seq_tbl remote_pcs_loopback[] = {
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x8000, 0x8000},   //15:15  remote_pcs_loopback_enable_ln3 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x4000, 0x4000},   //14:14  remote_pcs_loopback_enable_ln2 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x2000, 0x2000},   //13:13  remote_pcs_loopback_enable_ln1 = 1'b1
  //{"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0x1000, 0x1000},   //12:12  remote_pcs_loopback_enable_ln0 = 1'b1
  {"LOOPBACK CONTROL REGISTER",                0x3,  0x9109, 0xf000, 0xf000},   //merged [15:12]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 49 Remote PMD Loopback
prog_seq_tbl remote_pmd_loopback[] = {
  {"Remote Loopback Control",                  0x1,  0xd0e2, 0x0001, 0x0001},   //00:00 rmt_lpbk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   PRBS
-------------------------------------------------------------------- */
// Figure 50 PRBS 31 Generator
prog_seq_tbl prbs31_gen[] = {
  {"Pattern Generator Control",                0x1,  0xd0e1, 0x0001, 0x0001},   //00:00 prbs_gen_en = 1'b1
  {"PRBS_Check_en",                            0x1,  0xd0d1, 0x0001, 0x0001},   //00:00 prbs_chk_en = 1'b1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
// Figure 51 PRBS 31 Monitor (per lane)
prog_seq_tbl prbs31_mon[] = {
  {"PRBS Generator Control",                   0x1,  0xd0d1, 0x0001, 0x0001},    //00:00 prbs_chk_en = 1'b1
//  {"PRBS Checker LOCK Status",                 0x1,  0xd0d9, 0x0001, 0x0001},    //00:00 prbs_chk_lock = 1'b1  (READ)
  {"PRBS Checker Error Counter MSB Status",    0x1,  0xd0dA, 0xffff, 0x0000},    //15:00 err_cnt_msb (to clear contents) = N/A
  {"PRBS Checker Error Counter LSB Status",    0x1,  0xd0dB, 0xffff, 0x0000},    //15:00 err_cnt_lsb (to clear contents) = N/A
//  {"PRBS Checker Error Counter MSB Status",    0x1,  0xd0dA, 0xffff, 0x0000},    //15:00 err_cnt_msb = 15'b0 (READ)
//  {"PRBS Checker Error Counter LSB Status",    0x1,  0xd0dB, 0xffff, 0x0000},   //15:00 err_cnt_lsb = 15'b0 (READ)
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   Programming Simulation Speed up (maple2)
-------------------------------------------------------------------- */
//Figure 53 PLL Lock Speed Up
prog_seq_tbl pll_lock_speedup[] = {
  {"PLL_CALCTL_0",                             0x1,  0xd120, 0x1fff, 0x0052},   //12:00 calib_setup_time = 13'h0052
  {"PLL_CALCTL_1",                             0x1,  0xd121, 0xffff, 0x028f},   //15:00 fredet_time = 16'h028F
  {"PLL_CALCTL_2",                             0x1,  0xd122, 0x0fff, 0x0028},   //11:00 calib_cap_charge_time = 12'h028
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 
//Figure 54 PMD Lock Speed Up
prog_seq_tbl pmd_lock_speedup[] = {
  //{"DSC STATE MACHINE CONTROL 4",              0x1,  0xD014, 0x7c00, 0x0800},   //14:10 hw_tune_timeout = 5'h2
  //{"DSC STATE MACHINE CONTROL 4",              0x1,  0xD014, 0x03e0, 0x0020},   //09:05 cdr_settle_timeout = 5'h1
  //{"DSC STATE MACHINE CONTROL 4",              0x1,  0xD014, 0x001f, 0x0001},   //04:00 acq_cdr_timeout = 5'h1
  {"DSC STATE MACHINE CONTROL 4",              0x1,  0xd014, 0x7fff, 0x0821},    //merged [14:0]
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

//Figure 55 AN Speed Up
prog_seq_tbl AN_speedup[] = {
  //{"TICK GENERATION CONTROL REGISTER 1",       0x3,  0x9107, 0x8000, 0x8000},    //15:15 tick_override = 1'b1
  //{"TICK GENERATION CONTROL REGISTER 1",       0x3,  0x9107, 0x7fff, 0x0001},    //14:00 tick_numberator_upper = 1'b1
  {"TICK GENERATION CONTROL REGISTER 1",       0x3,  0x9107, 0xffff, 0x8001},    //merged [15:0]
  {"TICK GENERATION CONTROL REGISTER 0",       0x3,  0x9108, 0x0ffc, 0x0004},    //11:02 tick_denominator = 10'h1
  {"CL37 AUTO-NEG RESTART TIMER",              0x3,  0x9250, 0xffff, 0x001f},    //15:00 cl37_restart_timer = 16'h001F
  {"CL37 AUTO-NEG COMPLETE-ACKNOWLEDGE TIMER", 0x3,  0x9251, 0xffff, 0x001f},    //15:00 cl37_ack_timer = 16'h001F
  {"CL37 AUTO-NEG TIMEOUT-ERROR TIMER",        0x3,  0x9252, 0xffff, 0x0000},    //15:00 cl37_error_timer = 16'h0000
  {"CL73 AUTO-NEG BREAK-LINK TIMER",           0x3,  0x9253, 0xffff, 0x0005},    //15:00 cl73_break_link_timer = 16'h0005
  //{"CL73 AUTO-NEG TIMEOUT-ERROR TIMER",        0x3,  0x9254, 0xffff, 0x0000},    //15:00 cl73_error_timer = default
  //{"CL73 PARALLEL-DETECT DME-CLOCK TIMER",     0x3,  0x9255, 0xffff, 0x0000},    //15:00 cl73_pd_dme_lock_tikmer = default
  {"CL73 LINK-UP TIMER",                       0x3,  0x9256, 0xffff, 0x001f},    //15:00 cl73_link_up = 16'h001F
  //{"TIMER FOR QUALIFYING A LINK_STATUS",       0x3,  0x9257, 0xffff, 0x0000},    //15:00 link_fail_inhibit_timer_cl72 = default
  //{"Timer FOR QUALIFYING A LINK_STATUS",       0x3,  0x9258, 0xffff, 0x0000},    //15:00 link_fail_inhibit_timer_ncl72 = default
  //{"MAXIMUM TRAINING TIME",                    0x3,  0x925a, 0xffff, 0x0000},    //15:00 cl72_max_wait_timer = default
  //{"PERIOD TO IGNORE THE LINK",                0x3,  0x925C, 0xffff, 0x000f},    //15:00 ignore_link_timer = 16'h000F //ALEX
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
   power up/down sequence (applicable for 6858/65550 only)
-------------------------------------------------------------------- */

// test power down for lane 
prog_seq_tbl powerdn_lane[] = {
  //PCS digital power down
  {"RX_X4_Control0_pma_control_0",             0x3,  0xc457, 0x0001, 0x0000},    //00:00 rx rstb_lane = 0
  {"Digital_MiscDigControl",                   0x3,  0xc30b, 0x0080, 0x0000},    //07:07 mac_creditenable = 0
  {"TX_X4_Control0_misc",                      0x3,  0xc433, 0x0003, 0x0000},    //01:00 tx rstb_lane = 0 ; enable_tx_lane = 0
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0000},    //01:01 ln_dp_s_rstb = 0
  //PMD analog power down
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd082, 0x0033, 0x0033},    //05:04 afe_tx_pwrdn_frc_val = 1 ; afe_tx_pwrdn_frc = 1; 01:00: afe_rx_pwrdn_frc_val = 1; afe_rx_pwrdn_frc=1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* 0xd082 only for power up */
// test power up for lane 
prog_seq_tbl powerup_lane[] = {
  //PMD analog power up
  {"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd082, 0x0033, 0x0000},    //05:04 afe_tx_pwrdn_frc_val = 0 ; afe_tx_pwrdn_frc = 0; 01:00: afe_rx_pwrdn_frc_val = 0; afe_rx_pwrdn_frc=0
  //PCS digital power up
  {"RX_X4_Control0_pma_control_0",             0x3,  0xc457, 0x0001, 0x0001},    //00:00 rx rstb_lane = 1
  //{"Digital_MiscDigControl",                   0x3,  0xc30b, 0x0080, 0x0080},    //07:07 mac_creditenable = 1
  {"TX_X4_Control0_misc",                      0x3,  0xc433, 0x0003, 0x0003},    //01:00 tx rstb_lane = 1 ; enable_tx_lane = 1
  {"LANE_CLK_RESET_N_POWERDOWN_CONTROL",       0x1,  0xd081, 0x0002, 0x0002},    //01:01 ln_dp_s_rstb = 1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

/* -----------------------------------------------------------------
   power up/down TX lane sequence (applicable for 6858/65550 only)
   for link-cut test
-------------------------------------------------------------------- */

prog_seq_tbl powerdn_tx_lane[] = {
  //AFE TX power down
  {"PMD Transmit Disable",                     0x1,  0x0009, 0x0001, 0x0001},    //00:00 tx_disable_global = 1
  //{"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd082, 0x0030, 0x0030},    //05:04 afe_tx_pwrdn_frc_val = 1 ; afe_tx_pwrdn_frc = 1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl powerup_tx_lane[] = {
  //AFE TX power up
  {"PMD Transmit Disable",                     0x1,  0x0009, 0x0001, 0x0000},    //00:00 tx_disable_global = 1
  //{"LANE_AFE_RESET_PWRDWN_CONTROL_CONTROL",    0x1,  0xd082, 0x0030, 0x0000},    //05:04 afe_tx_pwrdn_frc_val = 0 ; afe_tx_pwrdn_frc = 0
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

prog_seq_tbl en_pmd_rx_lock_delay[] = {
  {"PMD RX Lock delay counter",                0x3,  0x9228, 0x0100, 0x0100},    //8:8 pmd_rx_lock_delay_sel = 1
  {"",                                         0x0,  0x0000, 0x0000, 0x0000}
}; 

