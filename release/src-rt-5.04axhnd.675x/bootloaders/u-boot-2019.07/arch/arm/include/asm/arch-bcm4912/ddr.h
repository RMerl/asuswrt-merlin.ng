/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2021 Broadcom Ltd.
 */

#ifndef _4912_DDR_H
#define _4912_DDR_H

#define MEMC_BASE        0x80040000      /* DDR IO Buf Control */
#define PHY_BASE         0x80060000      /* DDR PHY base */


#define mc2_glb_acc                              0x00000000 
#define mc2_glb_vers                             0x00000004 
#define mc2_glb_gcfg                             0x00000008 
#define mc2_glb_auto_self_refresh                0x0000000c 
#define mc2_glb_pwr_mgr                          0x00000010 


#define mc2_axi_acc                              0x00000040 
#define mc2_axi_ver                              0x00000044 
#define mc2_axi_CFG                              0x00000048 
#define mc2_axi_REP_ARB_MODE                     0x0000004c 
#define mc2_axi_queue_cfg                        0x00000050 
#define mc2_axi_queue_size0                      0x00000054 
#define mc2_axi_queue_map0                       0x00000058 
#define mc2_axi_SCRATCH                          0x00000060 
#define mc2_axi_AXI_DEBUG_0_0                    0x00000064 
#define mc2_axi_AXI_DEBUG_1_0                    0x00000068 
#define mc2_axi_AXI_DEBUG_MISC                   0x0000006c 
#define mc2_axi_AXI_DEBUG_WBF_ID                 0x00000070 


#define mc2_ubus_acc                             0x00000080 
#define mc2_ubus_CFG                             0x00000084 
#define mc2_ubus_ESRCID_CFG                      0x00000088 
#define mc2_ubus_queue_cfg_queue_cfg             0x0000008c 
#define mc2_ubus_queue_cfg_queue_map0            0x00000090 
#define mc2_ubus_queue_cfg_queue_map1            0x00000094 
#define mc2_ubus_queue_cfg_queue_map2            0x00000098 
#define mc2_ubus_queue_cfg_queue_map3            0x0000009c 
#define mc2_ubus_queue_cfg_queue_size0           0x000000a0 
#define mc2_ubus_queue_cfg_queue_size1           0x000000a4 
#define mc2_ubus_queue_cfg_queue_size2           0x000000a8 
#define mc2_ubus_queue_cfg_queue_size3           0x000000ac 
#define mc2_ubus_diag_ctrl                       0x000000b0 
#define mc2_ubus_scratch                         0x000000b8 
#define mc2_ubus_debug_ro                        0x000000bc 
#define mc2_ubus_DEBUG_WBF_ID                    0x000000c0 


#define mc2_misc_acc                             0x00000200 
#define mc2_misc_ver                             0x00000204 
#define mc2_misc_cfg                             0x00000208 
#define mc2_misc_vq_cfg                          0x0000020c 
#define mc2_misc_edis_addr_rand                  0x00000210 
#define mc2_misc_misc_dbg                        0x00000214 
#define mc2_misc_wbfid_bkdr_cmd                  0x00000218


#define mc2_afx_acc                              0x00000300 
#define mc2_afx_ver                              0x00000304 
#define mc2_afx_sram_match_cfg_sram_start_addr_hi 0x00000310 
#define mc2_afx_sram_match_cfg_sram_start_addr_lo 0x00000314 
#define mc2_afx_sram_match_cfg_sram_end_addr_hi  0x00000318 
#define mc2_afx_sram_match_cfg_sram_end_addr_lo  0x0000031c 
#define mc2_afx_addr_fltr_cfg0_start_addr_hi     0x00000320 
#define mc2_afx_addr_fltr_cfg0_start_addr_lo     0x00000324 
#define mc2_afx_addr_fltr_cfg0_end_addr_hi       0x00000328 
#define mc2_afx_addr_fltr_cfg0_end_addr_lo       0x0000032c 
#define mc2_afx_addr_fltr_cfg1_start_addr_hi     0x00000330 
#define mc2_afx_addr_fltr_cfg1_start_addr_lo     0x00000334 
#define mc2_afx_addr_fltr_cfg1_end_addr_hi       0x00000338 
#define mc2_afx_addr_fltr_cfg1_end_addr_lo       0x0000033c 
#define mc2_afx_addr_fltr_cfg2_start_addr_hi     0x00000340 
#define mc2_afx_addr_fltr_cfg2_start_addr_lo     0x00000344 
#define mc2_afx_addr_fltr_cfg2_end_addr_hi       0x00000348 
#define mc2_afx_addr_fltr_cfg2_end_addr_lo       0x0000034c 
#define mc2_afx_addr_fltr_cfg3_start_addr_hi     0x00000350 
#define mc2_afx_addr_fltr_cfg3_start_addr_lo     0x00000354 
#define mc2_afx_addr_fltr_cfg3_end_addr_hi       0x00000358 
#define mc2_afx_addr_fltr_cfg3_end_addr_lo       0x0000035c 
#define mc2_afx_srcid_fltr_cfg0_srcid            0x00000360 
#define mc2_afx_srcid_fltr_cfg1_srcid            0x00000364 
#define mc2_afx_srcid_fltr_cfg2_srcid            0x00000368 
#define mc2_afx_srcid_fltr_cfg3_srcid            0x0000036c 
#define mc2_afx_row_xtr_cfg_row_19_16            0x00000380 
#define mc2_afx_row_xtr_cfg_row_15_12            0x00000384 
#define mc2_afx_row_xtr_cfg_row_11_8             0x00000388 
#define mc2_afx_row_xtr_cfg_row_7_4              0x0000038c 
#define mc2_afx_row_xtr_cfg_row_3_0              0x00000390 
#define mc2_afx_bg_xtr_cfg_bg_3_0                0x000003a0 
#define mc2_afx_bk_xtr_cfg_bk_3_0                0x000003a4 
#define mc2_afx_col_xtr_cfg_col_cfg              0x000003a8 
#define mc2_afx_cs_xtr_cfg_cs_3_0                0x000003b4 
#define mc2_afx_chn_xtr_cfg_chn_bit              0x000003b8 
#define mc2_afx_ddr_sz_chk                       0x000003bc 



#define mc2_rchk_acc                             0x00000400 
#define mc2_rchk_ver                             0x00000404 
#define mc2_rchk_range_lock                      0x00000410 
#define mc2_rchk_range_0_control                 0x00000420 
#define mc2_rchk_range_0_persrcid_port           0x00000424 
#define mc2_rchk_range_0_persrcid_port_upper     0x00000428 
#define mc2_rchk_range_0_base                    0x0000042c 
#define mc2_rchk_range_0_base_upper              0x00000430 
#define mc2_rchk_range_0_seclev_en               0x00000434 
#define mc2_rchk_range_1_control                 0x00000440 
#define mc2_rchk_range_1_persrcid_port           0x00000444 
#define mc2_rchk_range_1_persrcid_port_upper     0x00000448 
#define mc2_rchk_range_1_base                    0x0000044c 
#define mc2_rchk_range_1_base_upper              0x00000450 
#define mc2_rchk_range_1_seclev_en               0x00000454 
#define mc2_rchk_range_2_control                 0x00000460 
#define mc2_rchk_range_2_persrcid_port           0x00000464 
#define mc2_rchk_range_2_persrcid_port_upper     0x00000468 
#define mc2_rchk_range_2_base                    0x0000046c 
#define mc2_rchk_range_2_base_upper              0x00000470 
#define mc2_rchk_range_2_seclev_en               0x00000474 
#define mc2_rchk_range_3_control                 0x00000480 
#define mc2_rchk_range_3_persrcid_port           0x00000484 
#define mc2_rchk_range_3_persrcid_port_upper     0x00000488 
#define mc2_rchk_range_3_base                    0x0000048c 
#define mc2_rchk_range_3_base_upper              0x00000490 
#define mc2_rchk_range_3_seclev_en               0x00000494 
#define mc2_rchk_range_4_control                 0x000004a0 
#define mc2_rchk_range_4_persrcid_port           0x000004a4 
#define mc2_rchk_range_4_persrcid_port_upper     0x000004a8 
#define mc2_rchk_range_4_base                    0x000004ac 
#define mc2_rchk_range_4_base_upper              0x000004b0 
#define mc2_rchk_range_4_seclev_en               0x000004b4 
#define mc2_rchk_range_5_control                 0x000004c0 
#define mc2_rchk_range_5_persrcid_port           0x000004c4 
#define mc2_rchk_range_5_persrcid_port_upper     0x000004c8 
#define mc2_rchk_range_5_base                    0x000004cc 
#define mc2_rchk_range_5_base_upper              0x000004d0 
#define mc2_rchk_range_5_seclev_en               0x000004d4 
#define mc2_rchk_range_6_control                 0x000004e0 
#define mc2_rchk_range_6_persrcid_port           0x000004e4 
#define mc2_rchk_range_6_persrcid_port_upper     0x000004e8 
#define mc2_rchk_range_6_base                    0x000004ec 
#define mc2_rchk_range_6_base_upper              0x000004f0 
#define mc2_rchk_range_6_seclev_en               0x000004f4 
#define mc2_rchk_range_7_control                 0x00000500 
#define mc2_rchk_range_7_persrcid_port           0x00000504 
#define mc2_rchk_range_7_persrcid_port_upper     0x00000508 
#define mc2_rchk_range_7_base                    0x0000050c 
#define mc2_rchk_range_7_base_upper              0x00000510 
#define mc2_rchk_range_7_seclev_en               0x00000514 
#define mc2_rchk_log_info_0                      0x00000520 
#define mc2_rchk_log_info_1                      0x00000524 
#define mc2_rchk_log_info_2                      0x00000528

#define mc2_rlt_acc                              0x00000600 
#define mc2_rlt_vers                             0x00000604 
#define mc2_rlt_rate_limiter0_cfg_0              0x00000610 
#define mc2_rlt_rate_limiter0_cfg_1              0x00000614 
#define mc2_rlt_rate_limiter1_cfg_0              0x00000618 
#define mc2_rlt_rate_limiter1_cfg_1              0x0000061c 
#define mc2_rlt_rate_limiter2_cfg_0              0x00000620 
#define mc2_rlt_rate_limiter2_cfg_1              0x00000624 
#define mc2_rlt_rate_limiter3_cfg_0              0x00000628 
#define mc2_rlt_rate_limiter3_cfg_1              0x0000062c 
#define mc2_rlt_rate_limiter4_cfg_0              0x00000630 
#define mc2_rlt_rate_limiter4_cfg_1              0x00000634 
#define mc2_rlt_rate_limiter5_cfg_0              0x00000638 
#define mc2_rlt_rate_limiter5_cfg_1              0x0000063c 
#define mc2_rlt_monitor0_mon_0                   0x00000690 
#define mc2_rlt_monitor0_mon_1                   0x00000694 
#define mc2_rlt_monitor1_mon_0                   0x00000698 
#define mc2_rlt_monitor1_mon_1                   0x0000069c 
#define mc2_rlt_monitor2_mon_0                   0x000006a0 
#define mc2_rlt_monitor2_mon_1                   0x000006a4 
#define mc2_rlt_monitor3_mon_0                   0x000006a8 
#define mc2_rlt_monitor3_mon_1                   0x000006ac 
#define mc2_rlt_monitor4_mon_0                   0x000006b0 
#define mc2_rlt_monitor4_mon_1                   0x000006b4 
#define mc2_rlt_monitor5_mon_0                   0x000006b8 
#define mc2_rlt_monitor5_mon_1                   0x000006bc 

#define mc2_chn_ddr_acc                          0x00000900 
#define mc2_chn_ddr_ver                          0x00000904 
#define mc2_chn_ddr_chn_arb_cfg                  0x00000908 
#define mc2_chn_ddr_chn_arb_param                0x0000090c 
#define mc2_chn_ddr_chn_arb_param1               0x00000910 
#define mc2_chn_ddr_chn_arb_param2               0x00000914 
#define mc2_chn_ddr_chn_arb_param3               0x00000918 
#define mc2_chn_ddr_chn_arb_param4               0x0000091c 
#define mc2_chn_ddr_chn_arb_dbg                  0x00000920 
#define mc2_chn_ddr_chn_sch_cfg                  0x00000928 
#define mc2_chn_ddr_phy_st                       0x0000093c
#define mc2_chn_ddr_dram_cfg                     0x00000940
#define mc2_chn_ddr_dcmd                         0x00000944 
#define mc2_chn_ddr_dmode_0                      0x00000948 
#define mc2_chn_ddr_dmode_2                      0x0000094c 
#define mc2_chn_ddr_odt                          0x00000950 
#define mc2_chn_ddr_ddr_param_cmd0               0x00000954 
#define mc2_chn_ddr_ddr_param_cmd1               0x00000958 
#define mc2_chn_ddr_ddr_param_cmd2               0x0000095c 
#define mc2_chn_ddr_ddr_param_cmd3               0x00000960 
#define mc2_chn_ddr_ddr_param_dat0               0x00000964 
#define mc2_chn_ddr_ddr_param_dat1               0x00000968 
#define mc2_chn_ddr_ddr_param_pre0               0x0000096c 
#define mc2_chn_ddr_ddr_param_pwr0               0x00000970 
#define mc2_chn_ddr_ddr_param_zqc0               0x00000974 
#define mc2_chn_ddr_refresh_aref0                0x00000978 
#define mc2_chn_ddr_refresh_aref1                0x0000097c 
#define mc2_chn_ddr_auto_self_refresh            0x00000980 
#define mc2_chn_ddr_auto_zqcs                    0x0000098c 
#define mc2_chn_ddr_dfi_error                    0x0000099c 


#define mc2_chn_sram_acc                         0x00000a00 
#define mc2_chn_sram_ver                         0x00000a04 
#define mc2_chn_sram_ind_ctrl                    0x00000a08 
#define mc2_chn_sram_init                        0x00000a0c 
#define mc2_chn_sram_ind_data0                   0x00000a10 
#define mc2_chn_sram_ind_data1                   0x00000a14 
#define mc2_chn_sram_ind_data2                   0x00000a18 
#define mc2_chn_sram_ind_data3                   0x00000a1c 
#define mc2_chn_sram_wr_cmd_cnt                  0x00000a20 
#define mc2_chn_sram_rd_cmd_cnt                  0x00000a24 
#define mc2_chn_sram_cap_0_cnt                   0x00000a28 
#define mc2_chn_sram_cap_1_cnt                   0x00000a2c 


#define mc2_wbf_acc                              0x00000e80 
#define mc2_wbf_ver                              0x00000e84 
#define mc2_wbf_pri_cfg                          0x00000e88 
#define mc2_wbf_sta                              0x00000e8c 
#define mc2_wbf_bkdr_bkdr_cmd                    0x00000e90 
#define mc2_wbf_bkdr_bkdr_data0                  0x00000e94 
#define mc2_wbf_bkdr_bkdr_data1                  0x00000e98 
#define mc2_wbf_bkdr_bkdr_data2                  0x00000e9c 
#define mc2_wbf_bkdr_bkdr_data3                  0x00000ea0 
#define mc2_wbf_bkdr_bkdr_data4                  0x00000ea4 
#define mc2_wbf_bkdr_bkdr_data5                  0x00000ea8 
#define mc2_wbf_bkdr_bkdr_data6                  0x00000eac 
#define mc2_wbf_bkdr_bkdr_data7                  0x00000eb0 
#define mc2_wbf_id_bkdr_id_cmd                   0x00000ec0 
#define mc2_wbf_id_bkdr_id_data                  0x00000ec4 
#define mc2_wbf_id_bkdr_id_data2                 0x00000ec8 


#define mc2_rmx_acc                              0x00000ed0 
#define mc2_rmx_ver                              0x00000ed4 
#define mc2_rmx_pri_cfg                          0x00000ed8 


#define mc2_glb_acc_acc_eack_MASK                                  0x80000000
#define mc2_glb_acc_acc_eack_ALIGN                                 0
#define mc2_glb_acc_acc_eack_BITS                                  1
#define mc2_glb_acc_acc_eack_SHIFT                                 31
#define mc2_glb_acc_acc_eack_DEFAULT                               0x00000000


#define mc2_glb_acc_reserved0_MASK                                 0x7fffff00
#define mc2_glb_acc_reserved0_ALIGN                                0
#define mc2_glb_acc_reserved0_BITS                                 23
#define mc2_glb_acc_reserved0_SHIFT                                8


#define mc2_glb_acc_acc_sw_MASK                                    0x00000080
#define mc2_glb_acc_acc_sw_ALIGN                                   0
#define mc2_glb_acc_acc_sw_BITS                                    1
#define mc2_glb_acc_acc_sw_SHIFT                                   7
#define mc2_glb_acc_acc_sw_DEFAULT                                 0x00000001


#define mc2_glb_acc_acc_sr_MASK                                    0x00000040
#define mc2_glb_acc_acc_sr_ALIGN                                   0
#define mc2_glb_acc_acc_sr_BITS                                    1
#define mc2_glb_acc_acc_sr_SHIFT                                   6
#define mc2_glb_acc_acc_sr_DEFAULT                                 0x00000001


#define mc2_glb_acc_acc_nsw_MASK                                   0x00000020
#define mc2_glb_acc_acc_nsw_ALIGN                                  0
#define mc2_glb_acc_acc_nsw_BITS                                   1
#define mc2_glb_acc_acc_nsw_SHIFT                                  5
#define mc2_glb_acc_acc_nsw_DEFAULT                                0x00000001


#define mc2_glb_acc_acc_nsr_MASK                                   0x00000010
#define mc2_glb_acc_acc_nsr_ALIGN                                  0
#define mc2_glb_acc_acc_nsr_BITS                                   1
#define mc2_glb_acc_acc_nsr_SHIFT                                  4
#define mc2_glb_acc_acc_nsr_DEFAULT                                0x00000001


#define mc2_glb_acc_perm_sw_MASK                                   0x00000008
#define mc2_glb_acc_perm_sw_ALIGN                                  0
#define mc2_glb_acc_perm_sw_BITS                                   1
#define mc2_glb_acc_perm_sw_SHIFT                                  3
#define mc2_glb_acc_perm_sw_DEFAULT                                0x00000001


#define mc2_glb_acc_perm_sr_MASK                                   0x00000004
#define mc2_glb_acc_perm_sr_ALIGN                                  0
#define mc2_glb_acc_perm_sr_BITS                                   1
#define mc2_glb_acc_perm_sr_SHIFT                                  2
#define mc2_glb_acc_perm_sr_DEFAULT                                0x00000001


#define mc2_glb_acc_perm_nsw_MASK                                  0x00000002
#define mc2_glb_acc_perm_nsw_ALIGN                                 0
#define mc2_glb_acc_perm_nsw_BITS                                  1
#define mc2_glb_acc_perm_nsw_SHIFT                                 1
#define mc2_glb_acc_perm_nsw_DEFAULT                               0x00000001


#define mc2_glb_acc_perm_nsr_MASK                                  0x00000001
#define mc2_glb_acc_perm_nsr_ALIGN                                 0
#define mc2_glb_acc_perm_nsr_BITS                                  1
#define mc2_glb_acc_perm_nsr_SHIFT                                 0
#define mc2_glb_acc_perm_nsr_DEFAULT                               0x00000001




#define mc2_glb_vers_reserved0_MASK                                0xffff0000
#define mc2_glb_vers_reserved0_ALIGN                               0
#define mc2_glb_vers_reserved0_BITS                                16
#define mc2_glb_vers_reserved0_SHIFT                               16


#define mc2_glb_vers_VERSION_MAJOR_MASK                            0x0000ff00
#define mc2_glb_vers_VERSION_MAJOR_ALIGN                           0
#define mc2_glb_vers_VERSION_MAJOR_BITS                            8
#define mc2_glb_vers_VERSION_MAJOR_SHIFT                           8
#define mc2_glb_vers_VERSION_MAJOR_DEFAULT                         0x00000005


#define mc2_glb_vers_VERSION_MINOR_MASK                            0x000000ff
#define mc2_glb_vers_VERSION_MINOR_ALIGN                           0
#define mc2_glb_vers_VERSION_MINOR_BITS                            8
#define mc2_glb_vers_VERSION_MINOR_SHIFT                           0
#define mc2_glb_vers_VERSION_MINOR_DEFAULT                         0x00000003




#define mc2_glb_gcfg_dram_en_MASK                                  0x80000000
#define mc2_glb_gcfg_dram_en_ALIGN                                 0
#define mc2_glb_gcfg_dram_en_BITS                                  1
#define mc2_glb_gcfg_dram_en_SHIFT                                 31
#define mc2_glb_gcfg_dram_en_DEFAULT                               0x00000000


#define mc2_glb_gcfg_reserved0_MASK                                0x78000000
#define mc2_glb_gcfg_reserved0_ALIGN                               0
#define mc2_glb_gcfg_reserved0_BITS                                4
#define mc2_glb_gcfg_reserved0_SHIFT                               27


#define mc2_glb_gcfg_sref_slow_clk_en_MASK                         0x04000000
#define mc2_glb_gcfg_sref_slow_clk_en_ALIGN                        0
#define mc2_glb_gcfg_sref_slow_clk_en_BITS                         1
#define mc2_glb_gcfg_sref_slow_clk_en_SHIFT                        26
#define mc2_glb_gcfg_sref_slow_clk_en_DEFAULT                      0x00000000


#define mc2_glb_gcfg_phy_dfi_mode_MASK                             0x03000000
#define mc2_glb_gcfg_phy_dfi_mode_ALIGN                            0
#define mc2_glb_gcfg_phy_dfi_mode_BITS                             2
#define mc2_glb_gcfg_phy_dfi_mode_SHIFT                            24
#define mc2_glb_gcfg_phy_dfi_mode_DEFAULT                          0x00000001


#define mc2_glb_gcfg_reserved1_MASK                                0x00f00000
#define mc2_glb_gcfg_reserved1_ALIGN                               0
#define mc2_glb_gcfg_reserved1_BITS                                4
#define mc2_glb_gcfg_reserved1_SHIFT                               20


#define mc2_glb_gcfg_dpfe_block_dmem_read_MASK                     0x00080000
#define mc2_glb_gcfg_dpfe_block_dmem_read_ALIGN                    0
#define mc2_glb_gcfg_dpfe_block_dmem_read_BITS                     1
#define mc2_glb_gcfg_dpfe_block_dmem_read_SHIFT                    19
#define mc2_glb_gcfg_dpfe_block_dmem_read_DEFAULT                  0x00000000


#define mc2_glb_gcfg_dpfe_block_dmem_write_MASK                    0x00040000
#define mc2_glb_gcfg_dpfe_block_dmem_write_ALIGN                   0
#define mc2_glb_gcfg_dpfe_block_dmem_write_BITS                    1
#define mc2_glb_gcfg_dpfe_block_dmem_write_SHIFT                   18
#define mc2_glb_gcfg_dpfe_block_dmem_write_DEFAULT                 0x00000000


#define mc2_glb_gcfg_dpfe_block_imem_read_MASK                     0x00020000
#define mc2_glb_gcfg_dpfe_block_imem_read_ALIGN                    0
#define mc2_glb_gcfg_dpfe_block_imem_read_BITS                     1
#define mc2_glb_gcfg_dpfe_block_imem_read_SHIFT                    17
#define mc2_glb_gcfg_dpfe_block_imem_read_DEFAULT                  0x00000000


#define mc2_glb_gcfg_dpfe_block_imem_write_MASK                    0x00010000
#define mc2_glb_gcfg_dpfe_block_imem_write_ALIGN                   0
#define mc2_glb_gcfg_dpfe_block_imem_write_BITS                    1
#define mc2_glb_gcfg_dpfe_block_imem_write_SHIFT                   16
#define mc2_glb_gcfg_dpfe_block_imem_write_DEFAULT                 0x00000000


#define mc2_glb_gcfg_dpfe_disable_cpu_MASK                         0x00008000
#define mc2_glb_gcfg_dpfe_disable_cpu_ALIGN                        0
#define mc2_glb_gcfg_dpfe_disable_cpu_BITS                         1
#define mc2_glb_gcfg_dpfe_disable_cpu_SHIFT                        15
#define mc2_glb_gcfg_dpfe_disable_cpu_DEFAULT                      0x00000000


#define mc2_glb_gcfg_reserved2_MASK                                0x00004000
#define mc2_glb_gcfg_reserved2_ALIGN                               0
#define mc2_glb_gcfg_reserved2_BITS                                1
#define mc2_glb_gcfg_reserved2_SHIFT                               14


#define mc2_glb_gcfg_wr_data_mode_MASK                             0x00002000
#define mc2_glb_gcfg_wr_data_mode_ALIGN                            0
#define mc2_glb_gcfg_wr_data_mode_BITS                             1
#define mc2_glb_gcfg_wr_data_mode_SHIFT                            13
#define mc2_glb_gcfg_wr_data_mode_DEFAULT                          0x00000000


#define mc2_glb_gcfg_exe_notify_mode_MASK                          0x00001000
#define mc2_glb_gcfg_exe_notify_mode_ALIGN                         0
#define mc2_glb_gcfg_exe_notify_mode_BITS                          1
#define mc2_glb_gcfg_exe_notify_mode_SHIFT                         12
#define mc2_glb_gcfg_exe_notify_mode_DEFAULT                       0x00000000


#define mc2_glb_gcfg_shmoo_done_MASK                               0x00000800
#define mc2_glb_gcfg_shmoo_done_ALIGN                              0
#define mc2_glb_gcfg_shmoo_done_BITS                               1
#define mc2_glb_gcfg_shmoo_done_SHIFT                              11
#define mc2_glb_gcfg_shmoo_done_DEFAULT                            0x00000000


#define mc2_glb_gcfg_dfi_en_MASK                                   0x00000400
#define mc2_glb_gcfg_dfi_en_ALIGN                                  0
#define mc2_glb_gcfg_dfi_en_BITS                                   1
#define mc2_glb_gcfg_dfi_en_SHIFT                                  10
#define mc2_glb_gcfg_dfi_en_DEFAULT                                0x00000000


#define mc2_glb_gcfg_mclksrc_MASK                                  0x00000200
#define mc2_glb_gcfg_mclksrc_ALIGN                                 0
#define mc2_glb_gcfg_mclksrc_BITS                                  1
#define mc2_glb_gcfg_mclksrc_SHIFT                                 9
#define mc2_glb_gcfg_mclksrc_DEFAULT                               0x00000000


#define mc2_glb_gcfg_meminitdone_MASK                              0x00000100
#define mc2_glb_gcfg_meminitdone_ALIGN                             0
#define mc2_glb_gcfg_meminitdone_BITS                              1
#define mc2_glb_gcfg_meminitdone_SHIFT                             8
#define mc2_glb_gcfg_meminitdone_DEFAULT                           0x00000000


#define mc2_glb_gcfg_dpfe_uart_tx_share_en_MASK                    0x00000080
#define mc2_glb_gcfg_dpfe_uart_tx_share_en_ALIGN                   0
#define mc2_glb_gcfg_dpfe_uart_tx_share_en_BITS                    1
#define mc2_glb_gcfg_dpfe_uart_tx_share_en_SHIFT                   7
#define mc2_glb_gcfg_dpfe_uart_tx_share_en_DEFAULT                 0x00000000


#define mc2_glb_gcfg_dpfe_uart_tx_en_MASK                          0x00000040
#define mc2_glb_gcfg_dpfe_uart_tx_en_ALIGN                         0
#define mc2_glb_gcfg_dpfe_uart_tx_en_BITS                          1
#define mc2_glb_gcfg_dpfe_uart_tx_en_SHIFT                         6
#define mc2_glb_gcfg_dpfe_uart_tx_en_DEFAULT                       0x00000000


#define mc2_glb_gcfg_dpfe_uart_rx_mode_MASK                        0x00000030
#define mc2_glb_gcfg_dpfe_uart_rx_mode_ALIGN                       0
#define mc2_glb_gcfg_dpfe_uart_rx_mode_BITS                        2
#define mc2_glb_gcfg_dpfe_uart_rx_mode_SHIFT                       4
#define mc2_glb_gcfg_dpfe_uart_rx_mode_DEFAULT                     0x00000000


#define mc2_glb_gcfg_sec_intr_mask_lock_MASK                       0x00000008
#define mc2_glb_gcfg_sec_intr_mask_lock_ALIGN                      0
#define mc2_glb_gcfg_sec_intr_mask_lock_BITS                       1
#define mc2_glb_gcfg_sec_intr_mask_lock_SHIFT                      3
#define mc2_glb_gcfg_sec_intr_mask_lock_DEFAULT                    0x00000000


#define mc2_glb_gcfg_reserved3_MASK                                0x00000006
#define mc2_glb_gcfg_reserved3_ALIGN                               0
#define mc2_glb_gcfg_reserved3_BITS                                2
#define mc2_glb_gcfg_reserved3_SHIFT                               1


#define mc2_glb_gcfg_dpath_mode_MASK                               0x00000001
#define mc2_glb_gcfg_dpath_mode_ALIGN                              0
#define mc2_glb_gcfg_dpath_mode_BITS                               1
#define mc2_glb_gcfg_dpath_mode_SHIFT                              0
#define mc2_glb_gcfg_dpath_mode_DEFAULT                            0x00000000




#define mc2_glb_auto_self_refresh_enable_MASK                      0x80000000
#define mc2_glb_auto_self_refresh_enable_ALIGN                     0
#define mc2_glb_auto_self_refresh_enable_BITS                      1
#define mc2_glb_auto_self_refresh_enable_SHIFT                     31
#define mc2_glb_auto_self_refresh_enable_DEFAULT                   0x00000000


#define mc2_glb_auto_self_refresh_immediate_MASK                   0x40000000
#define mc2_glb_auto_self_refresh_immediate_ALIGN                  0
#define mc2_glb_auto_self_refresh_immediate_BITS                   1
#define mc2_glb_auto_self_refresh_immediate_SHIFT                  30
#define mc2_glb_auto_self_refresh_immediate_DEFAULT                0x00000000


#define mc2_glb_auto_self_refresh_idle_count_MASK                  0x3fffffff
#define mc2_glb_auto_self_refresh_idle_count_ALIGN                 0
#define mc2_glb_auto_self_refresh_idle_count_BITS                  30
#define mc2_glb_auto_self_refresh_idle_count_SHIFT                 0
#define mc2_glb_auto_self_refresh_idle_count_DEFAULT               0x00000000




#define mc2_glb_pwr_mgr_reserved0_MASK                             0xffff0000
#define mc2_glb_pwr_mgr_reserved0_ALIGN                            0
#define mc2_glb_pwr_mgr_reserved0_BITS                             16
#define mc2_glb_pwr_mgr_reserved0_SHIFT                            16


#define mc2_glb_pwr_mgr_idle_mask_MASK                             0x0000ffff
#define mc2_glb_pwr_mgr_idle_mask_ALIGN                            0
#define mc2_glb_pwr_mgr_idle_mask_BITS                             16
#define mc2_glb_pwr_mgr_idle_mask_SHIFT                            0
#define mc2_glb_pwr_mgr_idle_mask_DEFAULT                          0x00000000




#define mc2_axi_acc_acc_eack_MASK                                  0x80000000
#define mc2_axi_acc_acc_eack_ALIGN                                 0
#define mc2_axi_acc_acc_eack_BITS                                  1
#define mc2_axi_acc_acc_eack_SHIFT                                 31
#define mc2_axi_acc_acc_eack_DEFAULT                               0x00000000


#define mc2_axi_acc_reserved0_MASK                                 0x7fffff00
#define mc2_axi_acc_reserved0_ALIGN                                0
#define mc2_axi_acc_reserved0_BITS                                 23
#define mc2_axi_acc_reserved0_SHIFT                                8


#define mc2_axi_acc_acc_sw_MASK                                    0x00000080
#define mc2_axi_acc_acc_sw_ALIGN                                   0
#define mc2_axi_acc_acc_sw_BITS                                    1
#define mc2_axi_acc_acc_sw_SHIFT                                   7
#define mc2_axi_acc_acc_sw_DEFAULT                                 0x00000001


#define mc2_axi_acc_acc_sr_MASK                                    0x00000040
#define mc2_axi_acc_acc_sr_ALIGN                                   0
#define mc2_axi_acc_acc_sr_BITS                                    1
#define mc2_axi_acc_acc_sr_SHIFT                                   6
#define mc2_axi_acc_acc_sr_DEFAULT                                 0x00000001


#define mc2_axi_acc_acc_nsw_MASK                                   0x00000020
#define mc2_axi_acc_acc_nsw_ALIGN                                  0
#define mc2_axi_acc_acc_nsw_BITS                                   1
#define mc2_axi_acc_acc_nsw_SHIFT                                  5
#define mc2_axi_acc_acc_nsw_DEFAULT                                0x00000001


#define mc2_axi_acc_acc_nsr_MASK                                   0x00000010
#define mc2_axi_acc_acc_nsr_ALIGN                                  0
#define mc2_axi_acc_acc_nsr_BITS                                   1
#define mc2_axi_acc_acc_nsr_SHIFT                                  4
#define mc2_axi_acc_acc_nsr_DEFAULT                                0x00000001


#define mc2_axi_acc_perm_sw_MASK                                   0x00000008
#define mc2_axi_acc_perm_sw_ALIGN                                  0
#define mc2_axi_acc_perm_sw_BITS                                   1
#define mc2_axi_acc_perm_sw_SHIFT                                  3
#define mc2_axi_acc_perm_sw_DEFAULT                                0x00000001


#define mc2_axi_acc_perm_sr_MASK                                   0x00000004
#define mc2_axi_acc_perm_sr_ALIGN                                  0
#define mc2_axi_acc_perm_sr_BITS                                   1
#define mc2_axi_acc_perm_sr_SHIFT                                  2
#define mc2_axi_acc_perm_sr_DEFAULT                                0x00000001


#define mc2_axi_acc_perm_nsw_MASK                                  0x00000002
#define mc2_axi_acc_perm_nsw_ALIGN                                 0
#define mc2_axi_acc_perm_nsw_BITS                                  1
#define mc2_axi_acc_perm_nsw_SHIFT                                 1
#define mc2_axi_acc_perm_nsw_DEFAULT                               0x00000001


#define mc2_axi_acc_perm_nsr_MASK                                  0x00000001
#define mc2_axi_acc_perm_nsr_ALIGN                                 0
#define mc2_axi_acc_perm_nsr_BITS                                  1
#define mc2_axi_acc_perm_nsr_SHIFT                                 0
#define mc2_axi_acc_perm_nsr_DEFAULT                               0x00000001




#define mc2_axi_ver_reserved0_MASK                                 0xffffff00
#define mc2_axi_ver_reserved0_ALIGN                                0
#define mc2_axi_ver_reserved0_BITS                                 24
#define mc2_axi_ver_reserved0_SHIFT                                8


#define mc2_axi_ver_version_MASK                                   0x000000ff
#define mc2_axi_ver_version_ALIGN                                  0
#define mc2_axi_ver_version_BITS                                   8
#define mc2_axi_ver_version_SHIFT                                  0
#define mc2_axi_ver_version_DEFAULT                                0x00000000




#define mc2_axi_CFG_reserved0_MASK                                 0xe0000000
#define mc2_axi_CFG_reserved0_ALIGN                                0
#define mc2_axi_CFG_reserved0_BITS                                 3
#define mc2_axi_CFG_reserved0_SHIFT                                29


#define mc2_axi_CFG_CPQ_WRITE_IN_TRANSIT_MASK                      0x1f800000
#define mc2_axi_CFG_CPQ_WRITE_IN_TRANSIT_ALIGN                     0
#define mc2_axi_CFG_CPQ_WRITE_IN_TRANSIT_BITS                      6
#define mc2_axi_CFG_CPQ_WRITE_IN_TRANSIT_SHIFT                     23
#define mc2_axi_CFG_CPQ_WRITE_IN_TRANSIT_DEFAULT                   0x00000008


#define mc2_axi_CFG_CPU_WRITE_IN_TRANSIT_MASK                      0x007e0000
#define mc2_axi_CFG_CPU_WRITE_IN_TRANSIT_ALIGN                     0
#define mc2_axi_CFG_CPU_WRITE_IN_TRANSIT_BITS                      6
#define mc2_axi_CFG_CPU_WRITE_IN_TRANSIT_SHIFT                     17
#define mc2_axi_CFG_CPU_WRITE_IN_TRANSIT_DEFAULT                   0x00000008


#define mc2_axi_CFG_WRITE_ACK_MODE_MASK                            0x00010000
#define mc2_axi_CFG_WRITE_ACK_MODE_ALIGN                           0
#define mc2_axi_CFG_WRITE_ACK_MODE_BITS                            1
#define mc2_axi_CFG_WRITE_ACK_MODE_SHIFT                           16
#define mc2_axi_CFG_WRITE_ACK_MODE_DEFAULT                         0x00000000


#define mc2_axi_CFG_reserved1_MASK                                 0x0000f000
#define mc2_axi_CFG_reserved1_ALIGN                                0
#define mc2_axi_CFG_reserved1_BITS                                 4
#define mc2_axi_CFG_reserved1_SHIFT                                12


#define mc2_axi_CFG_MC_CAP_MUX_SEL_MASK                            0x00000f00
#define mc2_axi_CFG_MC_CAP_MUX_SEL_ALIGN                           0
#define mc2_axi_CFG_MC_CAP_MUX_SEL_BITS                            4
#define mc2_axi_CFG_MC_CAP_MUX_SEL_SHIFT                           8
#define mc2_axi_CFG_MC_CAP_MUX_SEL_DEFAULT                         0x00000000


#define mc2_axi_CFG_MC_CAP_MUX_VLD_SEL_MASK                        0x000000c0
#define mc2_axi_CFG_MC_CAP_MUX_VLD_SEL_ALIGN                       0
#define mc2_axi_CFG_MC_CAP_MUX_VLD_SEL_BITS                        2
#define mc2_axi_CFG_MC_CAP_MUX_VLD_SEL_SHIFT                       6
#define mc2_axi_CFG_MC_CAP_MUX_VLD_SEL_DEFAULT                     0x00000000


#define mc2_axi_CFG_MC_CAP_MUX_TRIG_SEL_MASK                       0x00000030
#define mc2_axi_CFG_MC_CAP_MUX_TRIG_SEL_ALIGN                      0
#define mc2_axi_CFG_MC_CAP_MUX_TRIG_SEL_BITS                       2
#define mc2_axi_CFG_MC_CAP_MUX_TRIG_SEL_SHIFT                      4
#define mc2_axi_CFG_MC_CAP_MUX_TRIG_SEL_DEFAULT                    0x00000000


#define mc2_axi_CFG_RPT_NON_SECURE_ERR_MASK                        0x00000008
#define mc2_axi_CFG_RPT_NON_SECURE_ERR_ALIGN                       0
#define mc2_axi_CFG_RPT_NON_SECURE_ERR_BITS                        1
#define mc2_axi_CFG_RPT_NON_SECURE_ERR_SHIFT                       3
#define mc2_axi_CFG_RPT_NON_SECURE_ERR_DEFAULT                     0x00000000


#define mc2_axi_CFG_RPT_SECURE_ERR_MASK                            0x00000004
#define mc2_axi_CFG_RPT_SECURE_ERR_ALIGN                           0
#define mc2_axi_CFG_RPT_SECURE_ERR_BITS                            1
#define mc2_axi_CFG_RPT_SECURE_ERR_SHIFT                           2
#define mc2_axi_CFG_RPT_SECURE_ERR_DEFAULT                         0x00000000


#define mc2_axi_CFG_CMD_WAIT_WDATA_MASK                            0x00000002
#define mc2_axi_CFG_CMD_WAIT_WDATA_ALIGN                           0
#define mc2_axi_CFG_CMD_WAIT_WDATA_BITS                            1
#define mc2_axi_CFG_CMD_WAIT_WDATA_SHIFT                           1
#define mc2_axi_CFG_CMD_WAIT_WDATA_DEFAULT                         0x00000001


#define mc2_axi_CFG_DIS_RBUS_MSTR_MASK                             0x00000001
#define mc2_axi_CFG_DIS_RBUS_MSTR_ALIGN                            0
#define mc2_axi_CFG_DIS_RBUS_MSTR_BITS                             1
#define mc2_axi_CFG_DIS_RBUS_MSTR_SHIFT                            0
#define mc2_axi_CFG_DIS_RBUS_MSTR_DEFAULT                          0x00000000




#define mc2_axi_REP_ARB_MODE_reserved0_MASK                        0xfffff000
#define mc2_axi_REP_ARB_MODE_reserved0_ALIGN                       0
#define mc2_axi_REP_ARB_MODE_reserved0_BITS                        20
#define mc2_axi_REP_ARB_MODE_reserved0_SHIFT                       12


#define mc2_axi_REP_ARB_MODE_RD_BUFFER_SIZE_MASK                   0x00000800
#define mc2_axi_REP_ARB_MODE_RD_BUFFER_SIZE_ALIGN                  0
#define mc2_axi_REP_ARB_MODE_RD_BUFFER_SIZE_BITS                   1
#define mc2_axi_REP_ARB_MODE_RD_BUFFER_SIZE_SHIFT                  11
#define mc2_axi_REP_ARB_MODE_RD_BUFFER_SIZE_DEFAULT                0x00000000


#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_LEN_MASK               0x00000780
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_LEN_ALIGN              0
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_LEN_BITS               4
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_LEN_SHIFT              7
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_LEN_DEFAULT            0x00000007


#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_EN_MASK                0x00000040
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_EN_ALIGN               0
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_EN_BITS                1
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_EN_SHIFT               6
#define mc2_axi_REP_ARB_MODE_BRESP_PIPELINE_EN_DEFAULT             0x00000000


#define mc2_axi_REP_ARB_MODE_DIS_RRESP_ERR_MODE_MASK               0x00000020
#define mc2_axi_REP_ARB_MODE_DIS_RRESP_ERR_MODE_ALIGN              0
#define mc2_axi_REP_ARB_MODE_DIS_RRESP_ERR_MODE_BITS               1
#define mc2_axi_REP_ARB_MODE_DIS_RRESP_ERR_MODE_SHIFT              5
#define mc2_axi_REP_ARB_MODE_DIS_RRESP_ERR_MODE_DEFAULT            0x00000000


#define mc2_axi_REP_ARB_MODE_RD_FIFO_MODE_MASK                     0x00000010
#define mc2_axi_REP_ARB_MODE_RD_FIFO_MODE_ALIGN                    0
#define mc2_axi_REP_ARB_MODE_RD_FIFO_MODE_BITS                     1
#define mc2_axi_REP_ARB_MODE_RD_FIFO_MODE_SHIFT                    4
#define mc2_axi_REP_ARB_MODE_RD_FIFO_MODE_DEFAULT                  0x00000000


#define mc2_axi_REP_ARB_MODE_RD_PACKET_MODE_MASK                   0x00000008
#define mc2_axi_REP_ARB_MODE_RD_PACKET_MODE_ALIGN                  0
#define mc2_axi_REP_ARB_MODE_RD_PACKET_MODE_BITS                   1
#define mc2_axi_REP_ARB_MODE_RD_PACKET_MODE_SHIFT                  3
#define mc2_axi_REP_ARB_MODE_RD_PACKET_MODE_DEFAULT                0x00000000


#define mc2_axi_REP_ARB_MODE_DIS_BRESP_ERR_MODE_MASK               0x00000004
#define mc2_axi_REP_ARB_MODE_DIS_BRESP_ERR_MODE_ALIGN              0
#define mc2_axi_REP_ARB_MODE_DIS_BRESP_ERR_MODE_BITS               1
#define mc2_axi_REP_ARB_MODE_DIS_BRESP_ERR_MODE_SHIFT              2
#define mc2_axi_REP_ARB_MODE_DIS_BRESP_ERR_MODE_DEFAULT            0x00000000


#define mc2_axi_REP_ARB_MODE_WR_FIFO_MODE_MASK                     0x00000002
#define mc2_axi_REP_ARB_MODE_WR_FIFO_MODE_ALIGN                    0
#define mc2_axi_REP_ARB_MODE_WR_FIFO_MODE_BITS                     1
#define mc2_axi_REP_ARB_MODE_WR_FIFO_MODE_SHIFT                    1
#define mc2_axi_REP_ARB_MODE_WR_FIFO_MODE_DEFAULT                  0x00000000


#define mc2_axi_REP_ARB_MODE_WR_REPLY_MODE_MASK                    0x00000001
#define mc2_axi_REP_ARB_MODE_WR_REPLY_MODE_ALIGN                   0
#define mc2_axi_REP_ARB_MODE_WR_REPLY_MODE_BITS                    1
#define mc2_axi_REP_ARB_MODE_WR_REPLY_MODE_SHIFT                   0
#define mc2_axi_REP_ARB_MODE_WR_REPLY_MODE_DEFAULT                 0x00000000




#define mc2_axi_queue_cfg_reserved0_MASK                           0xfffffc00
#define mc2_axi_queue_cfg_reserved0_ALIGN                          0
#define mc2_axi_queue_cfg_reserved0_BITS                           22
#define mc2_axi_queue_cfg_reserved0_SHIFT                          10


#define mc2_axi_queue_cfg_disable_backpressure_MASK                0x00000200
#define mc2_axi_queue_cfg_disable_backpressure_ALIGN               0
#define mc2_axi_queue_cfg_disable_backpressure_BITS                1
#define mc2_axi_queue_cfg_disable_backpressure_SHIFT               9
#define mc2_axi_queue_cfg_disable_backpressure_DEFAULT             0x00000000


#define mc2_axi_queue_cfg_overflow_drop_MASK                       0x00000100
#define mc2_axi_queue_cfg_overflow_drop_ALIGN                      0
#define mc2_axi_queue_cfg_overflow_drop_BITS                       1
#define mc2_axi_queue_cfg_overflow_drop_SHIFT                      8
#define mc2_axi_queue_cfg_overflow_drop_DEFAULT                    0x00000000


#define mc2_axi_queue_cfg_reserved1_MASK                           0x000000f0
#define mc2_axi_queue_cfg_reserved1_ALIGN                          0
#define mc2_axi_queue_cfg_reserved1_BITS                           4
#define mc2_axi_queue_cfg_reserved1_SHIFT                          4


#define mc2_axi_queue_cfg_queue_start_MASK                         0x0000000f
#define mc2_axi_queue_cfg_queue_start_ALIGN                        0
#define mc2_axi_queue_cfg_queue_start_BITS                         4
#define mc2_axi_queue_cfg_queue_start_SHIFT                        0
#define mc2_axi_queue_cfg_queue_start_DEFAULT                      0x00000000




#define mc2_axi_queue_size0_reserved0_MASK                         0x80000000
#define mc2_axi_queue_size0_reserved0_ALIGN                        0
#define mc2_axi_queue_size0_reserved0_BITS                         1
#define mc2_axi_queue_size0_reserved0_SHIFT                        31


#define mc2_axi_queue_size0_size3_MASK                             0x7f000000
#define mc2_axi_queue_size0_size3_ALIGN                            0
#define mc2_axi_queue_size0_size3_BITS                             7
#define mc2_axi_queue_size0_size3_SHIFT                            24
#define mc2_axi_queue_size0_size3_DEFAULT                          0x00000008


#define mc2_axi_queue_size0_reserved1_MASK                         0x00800000
#define mc2_axi_queue_size0_reserved1_ALIGN                        0
#define mc2_axi_queue_size0_reserved1_BITS                         1
#define mc2_axi_queue_size0_reserved1_SHIFT                        23


#define mc2_axi_queue_size0_size2_MASK                             0x007f0000
#define mc2_axi_queue_size0_size2_ALIGN                            0
#define mc2_axi_queue_size0_size2_BITS                             7
#define mc2_axi_queue_size0_size2_SHIFT                            16
#define mc2_axi_queue_size0_size2_DEFAULT                          0x00000008


#define mc2_axi_queue_size0_reserved2_MASK                         0x00008000
#define mc2_axi_queue_size0_reserved2_ALIGN                        0
#define mc2_axi_queue_size0_reserved2_BITS                         1
#define mc2_axi_queue_size0_reserved2_SHIFT                        15


#define mc2_axi_queue_size0_size1_MASK                             0x00007f00
#define mc2_axi_queue_size0_size1_ALIGN                            0
#define mc2_axi_queue_size0_size1_BITS                             7
#define mc2_axi_queue_size0_size1_SHIFT                            8
#define mc2_axi_queue_size0_size1_DEFAULT                          0x00000008


#define mc2_axi_queue_size0_reserved3_MASK                         0x00000080
#define mc2_axi_queue_size0_reserved3_ALIGN                        0
#define mc2_axi_queue_size0_reserved3_BITS                         1
#define mc2_axi_queue_size0_reserved3_SHIFT                        7


#define mc2_axi_queue_size0_size0_MASK                             0x0000007f
#define mc2_axi_queue_size0_size0_ALIGN                            0
#define mc2_axi_queue_size0_size0_BITS                             7
#define mc2_axi_queue_size0_size0_SHIFT                            0
#define mc2_axi_queue_size0_size0_DEFAULT                          0x00000008




#define mc2_axi_queue_map0_reserved0_MASK                          0xffffff00
#define mc2_axi_queue_map0_reserved0_ALIGN                         0
#define mc2_axi_queue_map0_reserved0_BITS                          24
#define mc2_axi_queue_map0_reserved0_SHIFT                         8


#define mc2_axi_queue_map0_cpq_rd_MASK                             0x000000c0
#define mc2_axi_queue_map0_cpq_rd_ALIGN                            0
#define mc2_axi_queue_map0_cpq_rd_BITS                             2
#define mc2_axi_queue_map0_cpq_rd_SHIFT                            6
#define mc2_axi_queue_map0_cpq_rd_DEFAULT                          0x00000003


#define mc2_axi_queue_map0_cpq_wr_MASK                             0x00000030
#define mc2_axi_queue_map0_cpq_wr_ALIGN                            0
#define mc2_axi_queue_map0_cpq_wr_BITS                             2
#define mc2_axi_queue_map0_cpq_wr_SHIFT                            4
#define mc2_axi_queue_map0_cpq_wr_DEFAULT                          0x00000002


#define mc2_axi_queue_map0_cpu_rd_MASK                             0x0000000c
#define mc2_axi_queue_map0_cpu_rd_ALIGN                            0
#define mc2_axi_queue_map0_cpu_rd_BITS                             2
#define mc2_axi_queue_map0_cpu_rd_SHIFT                            2
#define mc2_axi_queue_map0_cpu_rd_DEFAULT                          0x00000001


#define mc2_axi_queue_map0_cpu_wr_MASK                             0x00000003
#define mc2_axi_queue_map0_cpu_wr_ALIGN                            0
#define mc2_axi_queue_map0_cpu_wr_BITS                             2
#define mc2_axi_queue_map0_cpu_wr_SHIFT                            0
#define mc2_axi_queue_map0_cpu_wr_DEFAULT                          0x00000000


#define mc2_axi_SCRATCH_scratch_MASK                               0xffffffff
#define mc2_axi_SCRATCH_scratch_ALIGN                              0
#define mc2_axi_SCRATCH_scratch_BITS                               32
#define mc2_axi_SCRATCH_scratch_SHIFT                              0
#define mc2_axi_SCRATCH_scratch_DEFAULT                            0x00000000


#define mc2_chn_ddr_acc_acc_eack_MASK                              0x80000000
#define mc2_chn_ddr_acc_acc_eack_ALIGN                             0
#define mc2_chn_ddr_acc_acc_eack_BITS                              1
#define mc2_chn_ddr_acc_acc_eack_SHIFT                             31
#define mc2_chn_ddr_acc_acc_eack_DEFAULT                           0x00000000


#define mc2_chn_ddr_acc_reserved0_MASK                             0x7fffff00
#define mc2_chn_ddr_acc_reserved0_ALIGN                            0
#define mc2_chn_ddr_acc_reserved0_BITS                             23
#define mc2_chn_ddr_acc_reserved0_SHIFT                            8


#define mc2_chn_ddr_acc_acc_sw_MASK                                0x00000080
#define mc2_chn_ddr_acc_acc_sw_ALIGN                               0
#define mc2_chn_ddr_acc_acc_sw_BITS                                1
#define mc2_chn_ddr_acc_acc_sw_SHIFT                               7
#define mc2_chn_ddr_acc_acc_sw_DEFAULT                             0x00000001


#define mc2_chn_ddr_acc_acc_sr_MASK                                0x00000040
#define mc2_chn_ddr_acc_acc_sr_ALIGN                               0
#define mc2_chn_ddr_acc_acc_sr_BITS                                1
#define mc2_chn_ddr_acc_acc_sr_SHIFT                               6
#define mc2_chn_ddr_acc_acc_sr_DEFAULT                             0x00000001


#define mc2_chn_ddr_acc_acc_nsw_MASK                               0x00000020
#define mc2_chn_ddr_acc_acc_nsw_ALIGN                              0
#define mc2_chn_ddr_acc_acc_nsw_BITS                               1
#define mc2_chn_ddr_acc_acc_nsw_SHIFT                              5
#define mc2_chn_ddr_acc_acc_nsw_DEFAULT                            0x00000001


#define mc2_chn_ddr_acc_acc_nsr_MASK                               0x00000010
#define mc2_chn_ddr_acc_acc_nsr_ALIGN                              0
#define mc2_chn_ddr_acc_acc_nsr_BITS                               1
#define mc2_chn_ddr_acc_acc_nsr_SHIFT                              4
#define mc2_chn_ddr_acc_acc_nsr_DEFAULT                            0x00000001


#define mc2_chn_ddr_acc_perm_sw_MASK                               0x00000008
#define mc2_chn_ddr_acc_perm_sw_ALIGN                              0
#define mc2_chn_ddr_acc_perm_sw_BITS                               1
#define mc2_chn_ddr_acc_perm_sw_SHIFT                              3
#define mc2_chn_ddr_acc_perm_sw_DEFAULT                            0x00000001


#define mc2_chn_ddr_acc_perm_sr_MASK                               0x00000004
#define mc2_chn_ddr_acc_perm_sr_ALIGN                              0
#define mc2_chn_ddr_acc_perm_sr_BITS                               1
#define mc2_chn_ddr_acc_perm_sr_SHIFT                              2
#define mc2_chn_ddr_acc_perm_sr_DEFAULT                            0x00000001


#define mc2_chn_ddr_acc_perm_nsw_MASK                              0x00000002
#define mc2_chn_ddr_acc_perm_nsw_ALIGN                             0
#define mc2_chn_ddr_acc_perm_nsw_BITS                              1
#define mc2_chn_ddr_acc_perm_nsw_SHIFT                             1
#define mc2_chn_ddr_acc_perm_nsw_DEFAULT                           0x00000001


#define mc2_chn_ddr_acc_perm_nsr_MASK                              0x00000001
#define mc2_chn_ddr_acc_perm_nsr_ALIGN                             0
#define mc2_chn_ddr_acc_perm_nsr_BITS                              1
#define mc2_chn_ddr_acc_perm_nsr_SHIFT                             0
#define mc2_chn_ddr_acc_perm_nsr_DEFAULT                           0x00000001




#define mc2_chn_ddr_ver_reserved0_MASK                             0xffffff00
#define mc2_chn_ddr_ver_reserved0_ALIGN                            0
#define mc2_chn_ddr_ver_reserved0_BITS                             24
#define mc2_chn_ddr_ver_reserved0_SHIFT                            8


#define mc2_chn_ddr_ver_version_MASK                               0x000000ff
#define mc2_chn_ddr_ver_version_ALIGN                              0
#define mc2_chn_ddr_ver_version_BITS                               8
#define mc2_chn_ddr_ver_version_SHIFT                              0
#define mc2_chn_ddr_ver_version_DEFAULT                            0x00000000




#define mc2_chn_ddr_chn_arb_cfg_fifo_mode_MASK                     0x80000000
#define mc2_chn_ddr_chn_arb_cfg_fifo_mode_ALIGN                    0
#define mc2_chn_ddr_chn_arb_cfg_fifo_mode_BITS                     1
#define mc2_chn_ddr_chn_arb_cfg_fifo_mode_SHIFT                    31
#define mc2_chn_ddr_chn_arb_cfg_fifo_mode_DEFAULT                  0x00000000


#define mc2_chn_ddr_chn_arb_cfg_addr_chk_disable_MASK              0x40000000
#define mc2_chn_ddr_chn_arb_cfg_addr_chk_disable_ALIGN             0
#define mc2_chn_ddr_chn_arb_cfg_addr_chk_disable_BITS              1
#define mc2_chn_ddr_chn_arb_cfg_addr_chk_disable_SHIFT             30
#define mc2_chn_ddr_chn_arb_cfg_addr_chk_disable_DEFAULT           0x00000000


#define mc2_chn_ddr_chn_arb_cfg_reserved0_MASK                     0x3c000000
#define mc2_chn_ddr_chn_arb_cfg_reserved0_ALIGN                    0
#define mc2_chn_ddr_chn_arb_cfg_reserved0_BITS                     4
#define mc2_chn_ddr_chn_arb_cfg_reserved0_SHIFT                    26


#define mc2_chn_ddr_chn_arb_cfg_always_auto_precharge_MASK         0x02000000
#define mc2_chn_ddr_chn_arb_cfg_always_auto_precharge_ALIGN        0
#define mc2_chn_ddr_chn_arb_cfg_always_auto_precharge_BITS         1
#define mc2_chn_ddr_chn_arb_cfg_always_auto_precharge_SHIFT        25
#define mc2_chn_ddr_chn_arb_cfg_always_auto_precharge_DEFAULT      0x00000000


#define mc2_chn_ddr_chn_arb_cfg_auto_precharge_MASK                0x01000000
#define mc2_chn_ddr_chn_arb_cfg_auto_precharge_ALIGN               0
#define mc2_chn_ddr_chn_arb_cfg_auto_precharge_BITS                1
#define mc2_chn_ddr_chn_arb_cfg_auto_precharge_SHIFT               24
#define mc2_chn_ddr_chn_arb_cfg_auto_precharge_DEFAULT             0x00000000

#define mc2_chn_ddr_chn_arb_cfg_reserved1_MASK                     0x00f80000
#define mc2_chn_ddr_chn_arb_cfg_reserved1_ALIGN                    0
#define mc2_chn_ddr_chn_arb_cfg_reserved1_BITS                     5
#define mc2_chn_ddr_chn_arb_cfg_reserved1_SHIFT                    19


#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_rw_grouping_en_MASK     0x00040000
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_rw_grouping_en_ALIGN    0
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_rw_grouping_en_BITS     1
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_rw_grouping_en_SHIFT    18
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_rw_grouping_en_DEFAULT  0x00000001


#define mc2_chn_ddr_chn_arb_cfg_cci2_rw_grouping_en_MASK           0x00020000
#define mc2_chn_ddr_chn_arb_cfg_cci2_rw_grouping_en_ALIGN          0
#define mc2_chn_ddr_chn_arb_cfg_cci2_rw_grouping_en_BITS           1
#define mc2_chn_ddr_chn_arb_cfg_cci2_rw_grouping_en_SHIFT          17
#define mc2_chn_ddr_chn_arb_cfg_cci2_rw_grouping_en_DEFAULT        0x00000001


#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_en_MASK                 0x00010000
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_en_ALIGN                0
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_en_BITS                 1
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_en_SHIFT                16
#define mc2_chn_ddr_chn_arb_cfg_cci2_reuse_en_DEFAULT              0x00000001


#define mc2_chn_ddr_chn_arb_cfg_reserved2_MASK                     0x00008000
#define mc2_chn_ddr_chn_arb_cfg_reserved2_ALIGN                    0
#define mc2_chn_ddr_chn_arb_cfg_reserved2_BITS                     1
#define mc2_chn_ddr_chn_arb_cfg_reserved2_SHIFT                    15


#define mc2_chn_ddr_chn_arb_cfg_vq_sta_if_red_blocked_repurpose_MASK 0x00004000
#define mc2_chn_ddr_chn_arb_cfg_vq_sta_if_red_blocked_repurpose_ALIGN 0
#define mc2_chn_ddr_chn_arb_cfg_vq_sta_if_red_blocked_repurpose_BITS 1
#define mc2_chn_ddr_chn_arb_cfg_vq_sta_if_red_blocked_repurpose_SHIFT 14
#define mc2_chn_ddr_chn_arb_cfg_vq_sta_if_red_blocked_repurpose_DEFAULT 0x00000000


#define mc2_chn_ddr_chn_arb_cfg_reserved3_MASK                     0x00002000
#define mc2_chn_ddr_chn_arb_cfg_reserved3_ALIGN                    0
#define mc2_chn_ddr_chn_arb_cfg_reserved3_BITS                     1
#define mc2_chn_ddr_chn_arb_cfg_reserved3_SHIFT                    13


#define mc2_chn_ddr_chn_arb_cfg_rdwr_wrr_weight_MASK               0x00001f00
#define mc2_chn_ddr_chn_arb_cfg_rdwr_wrr_weight_ALIGN              0
#define mc2_chn_ddr_chn_arb_cfg_rdwr_wrr_weight_BITS               5
#define mc2_chn_ddr_chn_arb_cfg_rdwr_wrr_weight_SHIFT              8
#define mc2_chn_ddr_chn_arb_cfg_rdwr_wrr_weight_DEFAULT            0x00000010


#define mc2_chn_ddr_chn_arb_cfg_reserved4_MASK                     0x000000e0
#define mc2_chn_ddr_chn_arb_cfg_reserved4_ALIGN                    0
#define mc2_chn_ddr_chn_arb_cfg_reserved4_BITS                     3
#define mc2_chn_ddr_chn_arb_cfg_reserved4_SHIFT                    5


#define mc2_chn_ddr_chn_arb_cfg_extra_slot_en_MASK                 0x00000010
#define mc2_chn_ddr_chn_arb_cfg_extra_slot_en_ALIGN                0
#define mc2_chn_ddr_chn_arb_cfg_extra_slot_en_BITS                 1
#define mc2_chn_ddr_chn_arb_cfg_extra_slot_en_SHIFT                4
#define mc2_chn_ddr_chn_arb_cfg_extra_slot_en_DEFAULT              0x00000000


#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpq_MASK               0x00000008
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpq_ALIGN              0
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpq_BITS               1
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpq_SHIFT              3
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpq_DEFAULT            0x00000000


#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpu_MASK               0x00000004
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpu_ALIGN              0
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpu_BITS               1
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpu_SHIFT              2
#define mc2_chn_ddr_chn_arb_cfg_per_intf_rl_cpu_DEFAULT            0x00000000


#define mc2_chn_ddr_chn_arb_cfg_pri_elevate_dis_MASK               0x00000002
#define mc2_chn_ddr_chn_arb_cfg_pri_elevate_dis_ALIGN              0
#define mc2_chn_ddr_chn_arb_cfg_pri_elevate_dis_BITS               1
#define mc2_chn_ddr_chn_arb_cfg_pri_elevate_dis_SHIFT              1
#define mc2_chn_ddr_chn_arb_cfg_pri_elevate_dis_DEFAULT            0x00000000


#define mc2_chn_ddr_chn_arb_cfg_arb_mode_MASK                      0x00000001
#define mc2_chn_ddr_chn_arb_cfg_arb_mode_ALIGN                     0
#define mc2_chn_ddr_chn_arb_cfg_arb_mode_BITS                      1
#define mc2_chn_ddr_chn_arb_cfg_arb_mode_SHIFT                     0
#define mc2_chn_ddr_chn_arb_cfg_arb_mode_DEFAULT                   0x00000000


#define mc2_chn_ddr_chn_arb_param_reserved0_MASK                   0x80000000
#define mc2_chn_ddr_chn_arb_param_reserved0_ALIGN                  0
#define mc2_chn_ddr_chn_arb_param_reserved0_BITS                   1
#define mc2_chn_ddr_chn_arb_param_reserved0_SHIFT                  31


#define mc2_chn_ddr_chn_arb_param_vq2_timeout_reuse_MASK           0x7f000000
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_reuse_ALIGN          0
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_reuse_BITS           7
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_reuse_SHIFT          24
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_reuse_DEFAULT        0x00000000


#define mc2_chn_ddr_chn_arb_param_reserved1_MASK                   0x00800000
#define mc2_chn_ddr_chn_arb_param_reserved1_ALIGN                  0
#define mc2_chn_ddr_chn_arb_param_reserved1_BITS                   1
#define mc2_chn_ddr_chn_arb_param_reserved1_SHIFT                  23


#define mc2_chn_ddr_chn_arb_param_vq2_timeout_MASK                 0x007f0000
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_ALIGN                0
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_BITS                 7
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_SHIFT                16
#define mc2_chn_ddr_chn_arb_param_vq2_timeout_DEFAULT              0x00000008


#define mc2_chn_ddr_chn_arb_param_reserved2_MASK                   0x00008000
#define mc2_chn_ddr_chn_arb_param_reserved2_ALIGN                  0
#define mc2_chn_ddr_chn_arb_param_reserved2_BITS                   1
#define mc2_chn_ddr_chn_arb_param_reserved2_SHIFT                  15


#define mc2_chn_ddr_chn_arb_param_vq0_timeout_reuse_MASK           0x00007f00
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_reuse_ALIGN          0
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_reuse_BITS           7
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_reuse_SHIFT          8
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_reuse_DEFAULT        0x00000000


#define mc2_chn_ddr_chn_arb_param_reserved3_MASK                   0x00000080
#define mc2_chn_ddr_chn_arb_param_reserved3_ALIGN                  0
#define mc2_chn_ddr_chn_arb_param_reserved3_BITS                   1
#define mc2_chn_ddr_chn_arb_param_reserved3_SHIFT                  7


#define mc2_chn_ddr_chn_arb_param_vq0_timeout_MASK                 0x0000007f
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_ALIGN                0
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_BITS                 7
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_SHIFT                0
#define mc2_chn_ddr_chn_arb_param_vq0_timeout_DEFAULT              0x00000008




#define mc2_chn_ddr_chn_arb_param1_reserved0_MASK                  0xfc000000
#define mc2_chn_ddr_chn_arb_param1_reserved0_ALIGN                 0
#define mc2_chn_ddr_chn_arb_param1_reserved0_BITS                  6
#define mc2_chn_ddr_chn_arb_param1_reserved0_SHIFT                 26


#define mc2_chn_ddr_chn_arb_param1_wrr6_weight1_MASK               0x03c00000
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight1_ALIGN              0
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight1_BITS               4
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight1_SHIFT              22
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight1_DEFAULT            0x00000008


#define mc2_chn_ddr_chn_arb_param1_wrr5_weight1_MASK               0x003c0000
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight1_ALIGN              0
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight1_BITS               4
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight1_SHIFT              18
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight1_DEFAULT            0x00000008


#define mc2_chn_ddr_chn_arb_param1_wrr0_weight1_MASK               0x0003e000
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight1_ALIGN              0
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight1_BITS               5
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight1_SHIFT              13
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight1_DEFAULT            0x00000010


#define mc2_chn_ddr_chn_arb_param1_wrr6_weight0_MASK               0x00001e00
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight0_ALIGN              0
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight0_BITS               4
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight0_SHIFT              9
#define mc2_chn_ddr_chn_arb_param1_wrr6_weight0_DEFAULT            0x00000008


#define mc2_chn_ddr_chn_arb_param1_wrr5_weight0_MASK               0x000001e0
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight0_ALIGN              0
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight0_BITS               4
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight0_SHIFT              5
#define mc2_chn_ddr_chn_arb_param1_wrr5_weight0_DEFAULT            0x00000008


#define mc2_chn_ddr_chn_arb_param1_wrr0_weight0_MASK               0x0000001f
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight0_ALIGN              0
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight0_BITS               5
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight0_SHIFT              0
#define mc2_chn_ddr_chn_arb_param1_wrr0_weight0_DEFAULT            0x00000010




#define mc2_chn_ddr_chn_arb_param2_reserved0_MASK                  0xfc000000
#define mc2_chn_ddr_chn_arb_param2_reserved0_ALIGN                 0
#define mc2_chn_ddr_chn_arb_param2_reserved0_BITS                  6
#define mc2_chn_ddr_chn_arb_param2_reserved0_SHIFT                 26


#define mc2_chn_ddr_chn_arb_param2_wrr6_weight1_reuse_MASK         0x03c00000
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight1_reuse_ALIGN        0
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight1_reuse_BITS         4
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight1_reuse_SHIFT        22
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight1_reuse_DEFAULT      0x00000002


#define mc2_chn_ddr_chn_arb_param2_wrr5_weight1_reuse_MASK         0x003c0000
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight1_reuse_ALIGN        0
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight1_reuse_BITS         4
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight1_reuse_SHIFT        18
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight1_reuse_DEFAULT      0x00000002


#define mc2_chn_ddr_chn_arb_param2_wrr0_weight1_reuse_MASK         0x0003e000
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight1_reuse_ALIGN        0
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight1_reuse_BITS         5
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight1_reuse_SHIFT        13
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight1_reuse_DEFAULT      0x00000004


#define mc2_chn_ddr_chn_arb_param2_wrr6_weight0_reuse_MASK         0x00001e00
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight0_reuse_ALIGN        0
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight0_reuse_BITS         4
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight0_reuse_SHIFT        9
#define mc2_chn_ddr_chn_arb_param2_wrr6_weight0_reuse_DEFAULT      0x00000002


#define mc2_chn_ddr_chn_arb_param2_wrr5_weight0_reuse_MASK         0x000001e0
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight0_reuse_ALIGN        0
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight0_reuse_BITS         4
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight0_reuse_SHIFT        5
#define mc2_chn_ddr_chn_arb_param2_wrr5_weight0_reuse_DEFAULT      0x00000002


#define mc2_chn_ddr_chn_arb_param2_wrr0_weight0_reuse_MASK         0x0000001f
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight0_reuse_ALIGN        0
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight0_reuse_BITS         5
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight0_reuse_SHIFT        0
#define mc2_chn_ddr_chn_arb_param2_wrr0_weight0_reuse_DEFAULT      0x00000004




#define mc2_chn_ddr_chn_arb_param3_reserved0_MASK                  0xe0000000
#define mc2_chn_ddr_chn_arb_param3_reserved0_ALIGN                 0
#define mc2_chn_ddr_chn_arb_param3_reserved0_BITS                  3
#define mc2_chn_ddr_chn_arb_param3_reserved0_SHIFT                 29


#define mc2_chn_ddr_chn_arb_param3_rd_bk_subset_of_wr_bk_change_wrr_en_MASK 0x10000000
#define mc2_chn_ddr_chn_arb_param3_rd_bk_subset_of_wr_bk_change_wrr_en_ALIGN 0
#define mc2_chn_ddr_chn_arb_param3_rd_bk_subset_of_wr_bk_change_wrr_en_BITS 1
#define mc2_chn_ddr_chn_arb_param3_rd_bk_subset_of_wr_bk_change_wrr_en_SHIFT 28
#define mc2_chn_ddr_chn_arb_param3_rd_bk_subset_of_wr_bk_change_wrr_en_DEFAULT 0x00000000


#define mc2_chn_ddr_chn_arb_param3_reserved1_MASK                  0x0c000000
#define mc2_chn_ddr_chn_arb_param3_reserved1_ALIGN                 0
#define mc2_chn_ddr_chn_arb_param3_reserved1_BITS                  2
#define mc2_chn_ddr_chn_arb_param3_reserved1_SHIFT                 26


#define mc2_chn_ddr_chn_arb_param3_wrr6_weight1_subset_MASK        0x03c00000
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight1_subset_ALIGN       0
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight1_subset_BITS        4
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight1_subset_SHIFT       22
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight1_subset_DEFAULT     0x00000002


#define mc2_chn_ddr_chn_arb_param3_wrr5_weight1_subset_MASK        0x003c0000
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight1_subset_ALIGN       0
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight1_subset_BITS        4
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight1_subset_SHIFT       18
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight1_subset_DEFAULT     0x00000008


#define mc2_chn_ddr_chn_arb_param3_wrr0_weight1_subset_MASK        0x0003e000
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight1_subset_ALIGN       0
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight1_subset_BITS        5
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight1_subset_SHIFT       13
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight1_subset_DEFAULT     0x00000004


#define mc2_chn_ddr_chn_arb_param3_wrr6_weight0_subset_MASK        0x00001e00
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight0_subset_ALIGN       0
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight0_subset_BITS        4
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight0_subset_SHIFT       9
#define mc2_chn_ddr_chn_arb_param3_wrr6_weight0_subset_DEFAULT     0x00000002


#define mc2_chn_ddr_chn_arb_param3_wrr5_weight0_subset_MASK        0x000001e0
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight0_subset_ALIGN       0
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight0_subset_BITS        4
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight0_subset_SHIFT       5
#define mc2_chn_ddr_chn_arb_param3_wrr5_weight0_subset_DEFAULT     0x00000008


#define mc2_chn_ddr_chn_arb_param3_wrr0_weight0_subset_MASK        0x0000001f
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight0_subset_ALIGN       0
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight0_subset_BITS        5
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight0_subset_SHIFT       0
#define mc2_chn_ddr_chn_arb_param3_wrr0_weight0_subset_DEFAULT     0x00000010




#define mc2_chn_ddr_chn_arb_param4_reserved0_MASK                  0xffffffc0
#define mc2_chn_ddr_chn_arb_param4_reserved0_ALIGN                 0
#define mc2_chn_ddr_chn_arb_param4_reserved0_BITS                  26
#define mc2_chn_ddr_chn_arb_param4_reserved0_SHIFT                 6


#define mc2_chn_ddr_chn_arb_param4_stop_req_when_sch_has_enough_activeBanks_en_MASK 0x00000020
#define mc2_chn_ddr_chn_arb_param4_stop_req_when_sch_has_enough_activeBanks_en_ALIGN 0
#define mc2_chn_ddr_chn_arb_param4_stop_req_when_sch_has_enough_activeBanks_en_BITS 1
#define mc2_chn_ddr_chn_arb_param4_stop_req_when_sch_has_enough_activeBanks_en_SHIFT 5
#define mc2_chn_ddr_chn_arb_param4_stop_req_when_sch_has_enough_activeBanks_en_DEFAULT 0x00000000


#define mc2_chn_ddr_chn_arb_param4_stop_req_activeBanks_threshold_MASK 0x0000001f
#define mc2_chn_ddr_chn_arb_param4_stop_req_activeBanks_threshold_ALIGN 0
#define mc2_chn_ddr_chn_arb_param4_stop_req_activeBanks_threshold_BITS 5
#define mc2_chn_ddr_chn_arb_param4_stop_req_activeBanks_threshold_SHIFT 0
#define mc2_chn_ddr_chn_arb_param4_stop_req_activeBanks_threshold_DEFAULT 0x00000006




#define mc2_chn_ddr_chn_sch_cfg_opt_chn_hshk_dis_MASK              0x80000000
#define mc2_chn_ddr_chn_sch_cfg_opt_chn_hshk_dis_ALIGN             0
#define mc2_chn_ddr_chn_sch_cfg_opt_chn_hshk_dis_BITS              1
#define mc2_chn_ddr_chn_sch_cfg_opt_chn_hshk_dis_SHIFT             31
#define mc2_chn_ddr_chn_sch_cfg_opt_chn_hshk_dis_DEFAULT           0x00000000


#define mc2_chn_ddr_chn_sch_cfg_reserved0_MASK                     0x7ffffff8
#define mc2_chn_ddr_chn_sch_cfg_reserved0_ALIGN                    0
#define mc2_chn_ddr_chn_sch_cfg_reserved0_BITS                     28
#define mc2_chn_ddr_chn_sch_cfg_reserved0_SHIFT                    3


#define mc2_chn_ddr_chn_sch_cfg_num_cmdr_MASK                      0x00000007
#define mc2_chn_ddr_chn_sch_cfg_num_cmdr_ALIGN                     0
#define mc2_chn_ddr_chn_sch_cfg_num_cmdr_BITS                      3
#define mc2_chn_ddr_chn_sch_cfg_num_cmdr_SHIFT                     0
#define mc2_chn_ddr_chn_sch_cfg_num_cmdr_DEFAULT                   0x00000007




#define mc2_chn_ddr_phy_st_reserved0_MASK                          0xffffffe0
#define mc2_chn_ddr_phy_st_reserved0_ALIGN                         0
#define mc2_chn_ddr_phy_st_reserved0_BITS                          27
#define mc2_chn_ddr_phy_st_reserved0_SHIFT                         5


#define mc2_chn_ddr_phy_st_phy_st_phy_ready_MASK                   0x00000010
#define mc2_chn_ddr_phy_st_phy_st_phy_ready_ALIGN                  0
#define mc2_chn_ddr_phy_st_phy_st_phy_ready_BITS                   1
#define mc2_chn_ddr_phy_st_phy_st_phy_ready_SHIFT                  4
#define mc2_chn_ddr_phy_st_phy_st_phy_ready_DEFAULT                0x00000000


#define mc2_chn_ddr_phy_st_reserved1_MASK                          0x00000008
#define mc2_chn_ddr_phy_st_reserved1_ALIGN                         0
#define mc2_chn_ddr_phy_st_reserved1_BITS                          1
#define mc2_chn_ddr_phy_st_reserved1_SHIFT                         3


#define mc2_chn_ddr_phy_st_phy_st_sw_reset_MASK                    0x00000004
#define mc2_chn_ddr_phy_st_phy_st_sw_reset_ALIGN                   0
#define mc2_chn_ddr_phy_st_phy_st_sw_reset_BITS                    1
#define mc2_chn_ddr_phy_st_phy_st_sw_reset_SHIFT                   2
#define mc2_chn_ddr_phy_st_phy_st_sw_reset_DEFAULT                 0x00000000


#define mc2_chn_ddr_phy_st_phy_st_hw_reset_MASK                    0x00000002
#define mc2_chn_ddr_phy_st_phy_st_hw_reset_ALIGN                   0
#define mc2_chn_ddr_phy_st_phy_st_hw_reset_BITS                    1
#define mc2_chn_ddr_phy_st_phy_st_hw_reset_SHIFT                   1
#define mc2_chn_ddr_phy_st_phy_st_hw_reset_DEFAULT                 0x00000000


#define mc2_chn_ddr_phy_st_phy_st_phy_power_up_MASK                0x00000001
#define mc2_chn_ddr_phy_st_phy_st_phy_power_up_ALIGN               0
#define mc2_chn_ddr_phy_st_phy_st_phy_power_up_BITS                1
#define mc2_chn_ddr_phy_st_phy_st_phy_power_up_SHIFT               0
#define mc2_chn_ddr_phy_st_phy_st_phy_power_up_DEFAULT             0x00000000




#define mc2_chn_ddr_dram_cfg_reserved0_MASK                        0xfffe0000
#define mc2_chn_ddr_dram_cfg_reserved0_ALIGN                       0
#define mc2_chn_ddr_dram_cfg_reserved0_BITS                        15
#define mc2_chn_ddr_dram_cfg_reserved0_SHIFT                       17


#define mc2_chn_ddr_dram_cfg_dram_srx_cmd_MASK                     0x00010000
#define mc2_chn_ddr_dram_cfg_dram_srx_cmd_ALIGN                    0
#define mc2_chn_ddr_dram_cfg_dram_srx_cmd_BITS                     1
#define mc2_chn_ddr_dram_cfg_dram_srx_cmd_SHIFT                    16
#define mc2_chn_ddr_dram_cfg_dram_srx_cmd_DEFAULT                  0x00000000


#define mc2_chn_ddr_dram_cfg_dram_cfg_cs_mode_MASK                 0x0000c000
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs_mode_ALIGN                0
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs_mode_BITS                 2
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs_mode_SHIFT                14
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs_mode_DEFAULT              0x00000000


#define mc2_chn_ddr_dram_cfg_dram_cfg_cs1_enable_MASK              0x00002000
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs1_enable_ALIGN             0
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs1_enable_BITS              1
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs1_enable_SHIFT             13
#define mc2_chn_ddr_dram_cfg_dram_cfg_cs1_enable_DEFAULT           0x00000000


#define mc2_chn_ddr_dram_cfg_reserved1_MASK                        0x00001800
#define mc2_chn_ddr_dram_cfg_reserved1_ALIGN                       0
#define mc2_chn_ddr_dram_cfg_reserved1_BITS                        2
#define mc2_chn_ddr_dram_cfg_reserved1_SHIFT                       11


#define mc2_chn_ddr_dram_cfg_dram_cfg_2taddrcmd_MASK               0x00000400
#define mc2_chn_ddr_dram_cfg_dram_cfg_2taddrcmd_ALIGN              0
#define mc2_chn_ddr_dram_cfg_dram_cfg_2taddrcmd_BITS               1
#define mc2_chn_ddr_dram_cfg_dram_cfg_2taddrcmd_SHIFT              10
#define mc2_chn_ddr_dram_cfg_dram_cfg_2taddrcmd_DEFAULT            0x00000000


#define mc2_chn_ddr_dram_cfg_cmd_timeout_MASK                      0x000003f0
#define mc2_chn_ddr_dram_cfg_cmd_timeout_ALIGN                     0
#define mc2_chn_ddr_dram_cfg_cmd_timeout_BITS                      6
#define mc2_chn_ddr_dram_cfg_cmd_timeout_SHIFT                     4
#define mc2_chn_ddr_dram_cfg_cmd_timeout_DEFAULT                   0x0000003f


#define mc2_chn_ddr_dram_cfg_dram_cfg_dramtype_MASK                0x0000000f
#define mc2_chn_ddr_dram_cfg_dram_cfg_dramtype_ALIGN               0
#define mc2_chn_ddr_dram_cfg_dram_cfg_dramtype_BITS                4
#define mc2_chn_ddr_dram_cfg_dram_cfg_dramtype_SHIFT               0
#define mc2_chn_ddr_dram_cfg_dram_cfg_dramtype_DEFAULT             0x00000004




#define mc2_chn_ddr_dcmd_sch_sel_MASK                              0x80000000
#define mc2_chn_ddr_dcmd_sch_sel_ALIGN                             0
#define mc2_chn_ddr_dcmd_sch_sel_BITS                              1
#define mc2_chn_ddr_dcmd_sch_sel_SHIFT                             31
#define mc2_chn_ddr_dcmd_sch_sel_DEFAULT                           0x00000000


#define mc2_chn_ddr_dcmd_reserved0_MASK                            0x7ffe0000
#define mc2_chn_ddr_dcmd_reserved0_ALIGN                           0
#define mc2_chn_ddr_dcmd_reserved0_BITS                            14
#define mc2_chn_ddr_dcmd_reserved0_SHIFT                           17


#define mc2_chn_ddr_dcmd_dramcmdreq_MASK                           0x00010000
#define mc2_chn_ddr_dcmd_dramcmdreq_ALIGN                          0
#define mc2_chn_ddr_dcmd_dramcmdreq_BITS                           1
#define mc2_chn_ddr_dcmd_dramcmdreq_SHIFT                          16
#define mc2_chn_ddr_dcmd_dramcmdreq_DEFAULT                        0x00000000


#define mc2_chn_ddr_dcmd_reserved1_MASK                            0x0000fc00
#define mc2_chn_ddr_dcmd_reserved1_ALIGN                           0
#define mc2_chn_ddr_dcmd_reserved1_BITS                            6
#define mc2_chn_ddr_dcmd_reserved1_SHIFT                           10


#define mc2_chn_ddr_dcmd_dcmd_memcmdcs1_MASK                       0x00000200
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs1_ALIGN                      0
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs1_BITS                       1
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs1_SHIFT                      9
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs1_DEFAULT                    0x00000000


#define mc2_chn_ddr_dcmd_dcmd_memcmdcs0_MASK                       0x00000100
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs0_ALIGN                      0
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs0_BITS                       1
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs0_SHIFT                      8
#define mc2_chn_ddr_dcmd_dcmd_memcmdcs0_DEFAULT                    0x00000000


#define mc2_chn_ddr_dcmd_reserved2_MASK                            0x000000e0
#define mc2_chn_ddr_dcmd_reserved2_ALIGN                           0
#define mc2_chn_ddr_dcmd_reserved2_BITS                            3
#define mc2_chn_ddr_dcmd_reserved2_SHIFT                           5


#define mc2_chn_ddr_dcmd_dcmd_memcmd_MASK                          0x0000001f
#define mc2_chn_ddr_dcmd_dcmd_memcmd_ALIGN                         0
#define mc2_chn_ddr_dcmd_dcmd_memcmd_BITS                          5
#define mc2_chn_ddr_dcmd_dcmd_memcmd_SHIFT                         0
#define mc2_chn_ddr_dcmd_dcmd_memcmd_DEFAULT                       0x00000000




#define mc2_chn_ddr_dmode_0_reserved0_MASK                         0x80000000
#define mc2_chn_ddr_dmode_0_reserved0_ALIGN                        0
#define mc2_chn_ddr_dmode_0_reserved0_BITS                         1
#define mc2_chn_ddr_dmode_0_reserved0_SHIFT                        31


#define mc2_chn_ddr_dmode_0_dmode_modedata_MASK                    0x7fff0000
#define mc2_chn_ddr_dmode_0_dmode_modedata_ALIGN                   0
#define mc2_chn_ddr_dmode_0_dmode_modedata_BITS                    15
#define mc2_chn_ddr_dmode_0_dmode_modedata_SHIFT                   16
#define mc2_chn_ddr_dmode_0_dmode_modedata_DEFAULT                 0x00000000


#define mc2_chn_ddr_dmode_0_reserved1_MASK                         0x00008000
#define mc2_chn_ddr_dmode_0_reserved1_ALIGN                        0
#define mc2_chn_ddr_dmode_0_reserved1_BITS                         1
#define mc2_chn_ddr_dmode_0_reserved1_SHIFT                        15


#define mc2_chn_ddr_dmode_0_dmode_emodedata_MASK                   0x00007fff
#define mc2_chn_ddr_dmode_0_dmode_emodedata_ALIGN                  0
#define mc2_chn_ddr_dmode_0_dmode_emodedata_BITS                   15
#define mc2_chn_ddr_dmode_0_dmode_emodedata_SHIFT                  0
#define mc2_chn_ddr_dmode_0_dmode_emodedata_DEFAULT                0x00000000




#define mc2_chn_ddr_dmode_2_reserved0_MASK                         0x80000000
#define mc2_chn_ddr_dmode_2_reserved0_ALIGN                        0
#define mc2_chn_ddr_dmode_2_reserved0_BITS                         1
#define mc2_chn_ddr_dmode_2_reserved0_SHIFT                        31


#define mc2_chn_ddr_dmode_2_dmode_emode3data_MASK                  0x7fff0000
#define mc2_chn_ddr_dmode_2_dmode_emode3data_ALIGN                 0
#define mc2_chn_ddr_dmode_2_dmode_emode3data_BITS                  15
#define mc2_chn_ddr_dmode_2_dmode_emode3data_SHIFT                 16
#define mc2_chn_ddr_dmode_2_dmode_emode3data_DEFAULT               0x00000000


#define mc2_chn_ddr_dmode_2_reserved1_MASK                         0x00008000
#define mc2_chn_ddr_dmode_2_reserved1_ALIGN                        0
#define mc2_chn_ddr_dmode_2_reserved1_BITS                         1
#define mc2_chn_ddr_dmode_2_reserved1_SHIFT                        15


#define mc2_chn_ddr_dmode_2_dmode_emode2data_MASK                  0x00007fff
#define mc2_chn_ddr_dmode_2_dmode_emode2data_ALIGN                 0
#define mc2_chn_ddr_dmode_2_dmode_emode2data_BITS                  15
#define mc2_chn_ddr_dmode_2_dmode_emode2data_SHIFT                 0
#define mc2_chn_ddr_dmode_2_dmode_emode2data_DEFAULT               0x00000000




#define mc2_chn_ddr_odt_reserved0_MASK                             0xfffffc00
#define mc2_chn_ddr_odt_reserved0_ALIGN                            0
#define mc2_chn_ddr_odt_reserved0_BITS                             22
#define mc2_chn_ddr_odt_reserved0_SHIFT                            10


#define mc2_chn_ddr_odt_odt_dynodten_MASK                          0x00000200
#define mc2_chn_ddr_odt_odt_dynodten_ALIGN                         0
#define mc2_chn_ddr_odt_odt_dynodten_BITS                          1
#define mc2_chn_ddr_odt_odt_dynodten_SHIFT                         9
#define mc2_chn_ddr_odt_odt_dynodten_DEFAULT                       0x00000001


#define mc2_chn_ddr_odt_odt_csoddodten_MASK                        0x00000100
#define mc2_chn_ddr_odt_odt_csoddodten_ALIGN                       0
#define mc2_chn_ddr_odt_odt_csoddodten_BITS                        1
#define mc2_chn_ddr_odt_odt_csoddodten_SHIFT                       8
#define mc2_chn_ddr_odt_odt_csoddodten_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs1_odt_wr_cs1_MASK                        0x00000080
#define mc2_chn_ddr_odt_cs1_odt_wr_cs1_ALIGN                       0
#define mc2_chn_ddr_odt_cs1_odt_wr_cs1_BITS                        1
#define mc2_chn_ddr_odt_cs1_odt_wr_cs1_SHIFT                       7
#define mc2_chn_ddr_odt_cs1_odt_wr_cs1_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs1_odt_wr_cs0_MASK                        0x00000040
#define mc2_chn_ddr_odt_cs1_odt_wr_cs0_ALIGN                       0
#define mc2_chn_ddr_odt_cs1_odt_wr_cs0_BITS                        1
#define mc2_chn_ddr_odt_cs1_odt_wr_cs0_SHIFT                       6
#define mc2_chn_ddr_odt_cs1_odt_wr_cs0_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs1_odt_rd_cs1_MASK                        0x00000020
#define mc2_chn_ddr_odt_cs1_odt_rd_cs1_ALIGN                       0
#define mc2_chn_ddr_odt_cs1_odt_rd_cs1_BITS                        1
#define mc2_chn_ddr_odt_cs1_odt_rd_cs1_SHIFT                       5
#define mc2_chn_ddr_odt_cs1_odt_rd_cs1_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs1_odt_rd_cs0_MASK                        0x00000010
#define mc2_chn_ddr_odt_cs1_odt_rd_cs0_ALIGN                       0
#define mc2_chn_ddr_odt_cs1_odt_rd_cs0_BITS                        1
#define mc2_chn_ddr_odt_cs1_odt_rd_cs0_SHIFT                       4
#define mc2_chn_ddr_odt_cs1_odt_rd_cs0_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs0_odt_wr_cs1_MASK                        0x00000008
#define mc2_chn_ddr_odt_cs0_odt_wr_cs1_ALIGN                       0
#define mc2_chn_ddr_odt_cs0_odt_wr_cs1_BITS                        1
#define mc2_chn_ddr_odt_cs0_odt_wr_cs1_SHIFT                       3
#define mc2_chn_ddr_odt_cs0_odt_wr_cs1_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs0_odt_wr_cs0_MASK                        0x00000004
#define mc2_chn_ddr_odt_cs0_odt_wr_cs0_ALIGN                       0
#define mc2_chn_ddr_odt_cs0_odt_wr_cs0_BITS                        1
#define mc2_chn_ddr_odt_cs0_odt_wr_cs0_SHIFT                       2
#define mc2_chn_ddr_odt_cs0_odt_wr_cs0_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs0_odt_rd_cs1_MASK                        0x00000002
#define mc2_chn_ddr_odt_cs0_odt_rd_cs1_ALIGN                       0
#define mc2_chn_ddr_odt_cs0_odt_rd_cs1_BITS                        1
#define mc2_chn_ddr_odt_cs0_odt_rd_cs1_SHIFT                       1
#define mc2_chn_ddr_odt_cs0_odt_rd_cs1_DEFAULT                     0x00000000


#define mc2_chn_ddr_odt_cs0_odt_rd_cs0_MASK                        0x00000001
#define mc2_chn_ddr_odt_cs0_odt_rd_cs0_ALIGN                       0
#define mc2_chn_ddr_odt_cs0_odt_rd_cs0_BITS                        1
#define mc2_chn_ddr_odt_cs0_odt_rd_cs0_SHIFT                       0
#define mc2_chn_ddr_odt_cs0_odt_rd_cs0_DEFAULT                     0x00000000




#define mc2_chn_ddr_ddr_param_cmd0_tAL_MASK                        0xff000000
#define mc2_chn_ddr_ddr_param_cmd0_tAL_ALIGN                       0
#define mc2_chn_ddr_ddr_param_cmd0_tAL_BITS                        8
#define mc2_chn_ddr_ddr_param_cmd0_tAL_SHIFT                       24
#define mc2_chn_ddr_ddr_param_cmd0_tAL_DEFAULT                     0x00000000


#define mc2_chn_ddr_ddr_param_cmd0_tRCD_MASK                       0x00ff0000
#define mc2_chn_ddr_ddr_param_cmd0_tRCD_ALIGN                      0
#define mc2_chn_ddr_ddr_param_cmd0_tRCD_BITS                       8
#define mc2_chn_ddr_ddr_param_cmd0_tRCD_SHIFT                      16
#define mc2_chn_ddr_ddr_param_cmd0_tRCD_DEFAULT                    0x00000006


#define mc2_chn_ddr_ddr_param_cmd0_tCWL_MASK                       0x0000ff00
#define mc2_chn_ddr_ddr_param_cmd0_tCWL_ALIGN                      0
#define mc2_chn_ddr_ddr_param_cmd0_tCWL_BITS                       8
#define mc2_chn_ddr_ddr_param_cmd0_tCWL_SHIFT                      8
#define mc2_chn_ddr_ddr_param_cmd0_tCWL_DEFAULT                    0x00000004


#define mc2_chn_ddr_ddr_param_cmd0_tCL_MASK                        0x000000ff
#define mc2_chn_ddr_ddr_param_cmd0_tCL_ALIGN                       0
#define mc2_chn_ddr_ddr_param_cmd0_tCL_BITS                        8
#define mc2_chn_ddr_ddr_param_cmd0_tCL_SHIFT                       0
#define mc2_chn_ddr_ddr_param_cmd0_tCL_DEFAULT                     0x00000005




#define mc2_chn_ddr_ddr_param_cmd1_tCCD_L_MASK                     0xff000000
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_L_ALIGN                    0
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_L_BITS                     8
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_L_SHIFT                    24
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_L_DEFAULT                  0x00000008


#define mc2_chn_ddr_ddr_param_cmd1_tCCD_S_MASK                     0x00ff0000
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_S_ALIGN                    0
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_S_BITS                     8
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_S_SHIFT                    16
#define mc2_chn_ddr_ddr_param_cmd1_tCCD_S_DEFAULT                  0x00000008


#define mc2_chn_ddr_ddr_param_cmd1_tRRD_L_MASK                     0x0000ff00
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_L_ALIGN                    0
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_L_BITS                     8
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_L_SHIFT                    8
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_L_DEFAULT                  0x00000004


#define mc2_chn_ddr_ddr_param_cmd1_tRRD_S_MASK                     0x000000ff
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_S_ALIGN                    0
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_S_BITS                     8
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_S_SHIFT                    0
#define mc2_chn_ddr_ddr_param_cmd1_tRRD_S_DEFAULT                  0x00000004




#define mc2_chn_ddr_ddr_param_cmd2_reserved0_MASK                  0xffffff00
#define mc2_chn_ddr_ddr_param_cmd2_reserved0_ALIGN                 0
#define mc2_chn_ddr_ddr_param_cmd2_reserved0_BITS                  24
#define mc2_chn_ddr_ddr_param_cmd2_reserved0_SHIFT                 8


#define mc2_chn_ddr_ddr_param_cmd2_tCCDMW_MASK                     0x000000ff
#define mc2_chn_ddr_ddr_param_cmd2_tCCDMW_ALIGN                    0
#define mc2_chn_ddr_ddr_param_cmd2_tCCDMW_BITS                     8
#define mc2_chn_ddr_ddr_param_cmd2_tCCDMW_SHIFT                    0
#define mc2_chn_ddr_ddr_param_cmd2_tCCDMW_DEFAULT                  0x00000020




#define mc2_chn_ddr_ddr_param_cmd3_tFAW_MASK                       0xff000000
#define mc2_chn_ddr_ddr_param_cmd3_tFAW_ALIGN                      0
#define mc2_chn_ddr_ddr_param_cmd3_tFAW_BITS                       8
#define mc2_chn_ddr_ddr_param_cmd3_tFAW_SHIFT                      24
#define mc2_chn_ddr_ddr_param_cmd3_tFAW_DEFAULT                    0x00000000


#define mc2_chn_ddr_ddr_param_cmd3_tRPpb_MASK                      0x00ff0000
#define mc2_chn_ddr_ddr_param_cmd3_tRPpb_ALIGN                     0
#define mc2_chn_ddr_ddr_param_cmd3_tRPpb_BITS                      8
#define mc2_chn_ddr_ddr_param_cmd3_tRPpb_SHIFT                     16
#define mc2_chn_ddr_ddr_param_cmd3_tRPpb_DEFAULT                   0x00000000


#define mc2_chn_ddr_ddr_param_cmd3_tRPab_MASK                      0x0000ff00
#define mc2_chn_ddr_ddr_param_cmd3_tRPab_ALIGN                     0
#define mc2_chn_ddr_ddr_param_cmd3_tRPab_BITS                      8
#define mc2_chn_ddr_ddr_param_cmd3_tRPab_SHIFT                     8
#define mc2_chn_ddr_ddr_param_cmd3_tRPab_DEFAULT                   0x00000000


#define mc2_chn_ddr_ddr_param_cmd3_tRAS_MASK                       0x000000ff
#define mc2_chn_ddr_ddr_param_cmd3_tRAS_ALIGN                      0
#define mc2_chn_ddr_ddr_param_cmd3_tRAS_BITS                       8
#define mc2_chn_ddr_ddr_param_cmd3_tRAS_SHIFT                      0
#define mc2_chn_ddr_ddr_param_cmd3_tRAS_DEFAULT                    0x00000011




#define mc2_chn_ddr_ddr_param_dat0_tWTR_L_MASK                     0xff000000
#define mc2_chn_ddr_ddr_param_dat0_tWTR_L_ALIGN                    0
#define mc2_chn_ddr_ddr_param_dat0_tWTR_L_BITS                     8
#define mc2_chn_ddr_ddr_param_dat0_tWTR_L_SHIFT                    24
#define mc2_chn_ddr_ddr_param_dat0_tWTR_L_DEFAULT                  0x00000002


#define mc2_chn_ddr_ddr_param_dat0_tWTR_S_MASK                     0x00ff0000
#define mc2_chn_ddr_ddr_param_dat0_tWTR_S_ALIGN                    0
#define mc2_chn_ddr_ddr_param_dat0_tWTR_S_BITS                     8
#define mc2_chn_ddr_ddr_param_dat0_tWTR_S_SHIFT                    16
#define mc2_chn_ddr_ddr_param_dat0_tWTR_S_DEFAULT                  0x00000002


#define mc2_chn_ddr_ddr_param_dat0_tWR_MASK                        0x0000ff00
#define mc2_chn_ddr_ddr_param_dat0_tWR_ALIGN                       0
#define mc2_chn_ddr_ddr_param_dat0_tWR_BITS                        8
#define mc2_chn_ddr_ddr_param_dat0_tWR_SHIFT                       8
#define mc2_chn_ddr_ddr_param_dat0_tWR_DEFAULT                     0x00000020


#define mc2_chn_ddr_ddr_param_dat0_tDQSCK_MASK                     0x000000f0
#define mc2_chn_ddr_ddr_param_dat0_tDQSCK_ALIGN                    0
#define mc2_chn_ddr_ddr_param_dat0_tDQSCK_BITS                     4
#define mc2_chn_ddr_ddr_param_dat0_tDQSCK_SHIFT                    4
#define mc2_chn_ddr_ddr_param_dat0_tDQSCK_DEFAULT                  0x00000007


#define mc2_chn_ddr_ddr_param_dat0_reserved0_MASK                  0x0000000c
#define mc2_chn_ddr_ddr_param_dat0_reserved0_ALIGN                 0
#define mc2_chn_ddr_ddr_param_dat0_reserved0_BITS                  2
#define mc2_chn_ddr_ddr_param_dat0_reserved0_SHIFT                 2


#define mc2_chn_ddr_ddr_param_dat0_tWPST_MASK                      0x00000002
#define mc2_chn_ddr_ddr_param_dat0_tWPST_ALIGN                     0
#define mc2_chn_ddr_ddr_param_dat0_tWPST_BITS                      1
#define mc2_chn_ddr_ddr_param_dat0_tWPST_SHIFT                     1
#define mc2_chn_ddr_ddr_param_dat0_tWPST_DEFAULT                   0x00000000


#define mc2_chn_ddr_ddr_param_dat0_tWPRE_MASK                      0x00000001
#define mc2_chn_ddr_ddr_param_dat0_tWPRE_ALIGN                     0
#define mc2_chn_ddr_ddr_param_dat0_tWPRE_BITS                      1
#define mc2_chn_ddr_ddr_param_dat0_tWPRE_SHIFT                     0
#define mc2_chn_ddr_ddr_param_dat0_tWPRE_DEFAULT                   0x00000000




#define mc2_chn_ddr_ddr_param_dat1_tRPST_MASK                      0x80000000
#define mc2_chn_ddr_ddr_param_dat1_tRPST_ALIGN                     0
#define mc2_chn_ddr_ddr_param_dat1_tRPST_BITS                      1
#define mc2_chn_ddr_ddr_param_dat1_tRPST_SHIFT                     31
#define mc2_chn_ddr_ddr_param_dat1_tRPST_DEFAULT                   0x00000001


#define mc2_chn_ddr_ddr_param_dat1_tRPRE_MASK                      0x40000000
#define mc2_chn_ddr_ddr_param_dat1_tRPRE_ALIGN                     0
#define mc2_chn_ddr_ddr_param_dat1_tRPRE_BITS                      1
#define mc2_chn_ddr_ddr_param_dat1_tRPRE_SHIFT                     30
#define mc2_chn_ddr_ddr_param_dat1_tRPRE_DEFAULT                   0x00000000


#define mc2_chn_ddr_ddr_param_dat1_reserved0_MASK                  0x3f000000
#define mc2_chn_ddr_ddr_param_dat1_reserved0_ALIGN                 0
#define mc2_chn_ddr_ddr_param_dat1_reserved0_BITS                  6
#define mc2_chn_ddr_ddr_param_dat1_reserved0_SHIFT                 24


#define mc2_chn_ddr_ddr_param_dat1_tW2R_MASK                       0x00ff0000
#define mc2_chn_ddr_ddr_param_dat1_tW2R_ALIGN                      0
#define mc2_chn_ddr_ddr_param_dat1_tW2R_BITS                       8
#define mc2_chn_ddr_ddr_param_dat1_tW2R_SHIFT                      16
#define mc2_chn_ddr_ddr_param_dat1_tW2R_DEFAULT                    0x00000000


#define mc2_chn_ddr_ddr_param_dat1_tR2R_MASK                       0x0000f000
#define mc2_chn_ddr_ddr_param_dat1_tR2R_ALIGN                      0
#define mc2_chn_ddr_ddr_param_dat1_tR2R_BITS                       4
#define mc2_chn_ddr_ddr_param_dat1_tR2R_SHIFT                      12
#define mc2_chn_ddr_ddr_param_dat1_tR2R_DEFAULT                    0x00000003


#define mc2_chn_ddr_ddr_param_dat1_tW2W_MASK                       0x00000f00
#define mc2_chn_ddr_ddr_param_dat1_tW2W_ALIGN                      0
#define mc2_chn_ddr_ddr_param_dat1_tW2W_BITS                       4
#define mc2_chn_ddr_ddr_param_dat1_tW2W_SHIFT                      8
#define mc2_chn_ddr_ddr_param_dat1_tW2W_DEFAULT                    0x00000008


#define mc2_chn_ddr_ddr_param_dat1_tR2W_MASK                       0x000000f0
#define mc2_chn_ddr_ddr_param_dat1_tR2W_ALIGN                      0
#define mc2_chn_ddr_ddr_param_dat1_tR2W_BITS                       4
#define mc2_chn_ddr_ddr_param_dat1_tR2W_SHIFT                      4
#define mc2_chn_ddr_ddr_param_dat1_tR2W_DEFAULT                    0x00000000


#define mc2_chn_ddr_ddr_param_dat1_reserved1_MASK                  0x0000000f
#define mc2_chn_ddr_ddr_param_dat1_reserved1_ALIGN                 0
#define mc2_chn_ddr_ddr_param_dat1_reserved1_BITS                  4
#define mc2_chn_ddr_ddr_param_dat1_reserved1_SHIFT                 0




#define mc2_chn_ddr_ddr_param_pre0_autoprecharge_MASK              0x80000000
#define mc2_chn_ddr_ddr_param_pre0_autoprecharge_ALIGN             0
#define mc2_chn_ddr_ddr_param_pre0_autoprecharge_BITS              1
#define mc2_chn_ddr_ddr_param_pre0_autoprecharge_SHIFT             31
#define mc2_chn_ddr_ddr_param_pre0_autoprecharge_DEFAULT           0x00000000


#define mc2_chn_ddr_ddr_param_pre0_reserved0_MASK                  0x7fff0000
#define mc2_chn_ddr_ddr_param_pre0_reserved0_ALIGN                 0
#define mc2_chn_ddr_ddr_param_pre0_reserved0_BITS                  15
#define mc2_chn_ddr_ddr_param_pre0_reserved0_SHIFT                 16


#define mc2_chn_ddr_ddr_param_pre0_tPPD_MASK                       0x0000ff00
#define mc2_chn_ddr_ddr_param_pre0_tPPD_ALIGN                      0
#define mc2_chn_ddr_ddr_param_pre0_tPPD_BITS                       8
#define mc2_chn_ddr_ddr_param_pre0_tPPD_SHIFT                      8
#define mc2_chn_ddr_ddr_param_pre0_tPPD_DEFAULT                    0x00000002


#define mc2_chn_ddr_ddr_param_pre0_tRTP_MASK                       0x000000ff
#define mc2_chn_ddr_ddr_param_pre0_tRTP_ALIGN                      0
#define mc2_chn_ddr_ddr_param_pre0_tRTP_BITS                       8
#define mc2_chn_ddr_ddr_param_pre0_tRTP_SHIFT                      0
#define mc2_chn_ddr_ddr_param_pre0_tRTP_DEFAULT                    0x00000002




#define mc2_chn_ddr_ddr_param_pwr0_tXP_MASK                        0xfff00000
#define mc2_chn_ddr_ddr_param_pwr0_tXP_ALIGN                       0
#define mc2_chn_ddr_ddr_param_pwr0_tXP_BITS                        12
#define mc2_chn_ddr_ddr_param_pwr0_tXP_SHIFT                       20
#define mc2_chn_ddr_ddr_param_pwr0_tXP_DEFAULT                     0x00000003


#define mc2_chn_ddr_ddr_param_pwr0_tXSR_MASK                       0x000fff00
#define mc2_chn_ddr_ddr_param_pwr0_tXSR_ALIGN                      0
#define mc2_chn_ddr_ddr_param_pwr0_tXSR_BITS                       12
#define mc2_chn_ddr_ddr_param_pwr0_tXSR_SHIFT                      8
#define mc2_chn_ddr_ddr_param_pwr0_tXSR_DEFAULT                    0x00000002


#define mc2_chn_ddr_ddr_param_pwr0_tSR_MASK                        0x000000ff
#define mc2_chn_ddr_ddr_param_pwr0_tSR_ALIGN                       0
#define mc2_chn_ddr_ddr_param_pwr0_tSR_BITS                        8
#define mc2_chn_ddr_ddr_param_pwr0_tSR_SHIFT                       0
#define mc2_chn_ddr_ddr_param_pwr0_tSR_DEFAULT                     0x00000003




#define mc2_chn_ddr_ddr_param_zqc0_reserved0_MASK                  0xf0000000
#define mc2_chn_ddr_ddr_param_zqc0_reserved0_ALIGN                 0
#define mc2_chn_ddr_ddr_param_zqc0_reserved0_BITS                  4
#define mc2_chn_ddr_ddr_param_zqc0_reserved0_SHIFT                 28


#define mc2_chn_ddr_ddr_param_zqc0_tZQCL_MASK                      0x0fff0000
#define mc2_chn_ddr_ddr_param_zqc0_tZQCL_ALIGN                     0
#define mc2_chn_ddr_ddr_param_zqc0_tZQCL_BITS                      12
#define mc2_chn_ddr_ddr_param_zqc0_tZQCL_SHIFT                     16
#define mc2_chn_ddr_ddr_param_zqc0_tZQCL_DEFAULT                   0x00000100


#define mc2_chn_ddr_ddr_param_zqc0_reserved1_MASK                  0x0000fe00
#define mc2_chn_ddr_ddr_param_zqc0_reserved1_ALIGN                 0
#define mc2_chn_ddr_ddr_param_zqc0_reserved1_BITS                  7
#define mc2_chn_ddr_ddr_param_zqc0_reserved1_SHIFT                 9


#define mc2_chn_ddr_ddr_param_zqc0_tZQCS_MASK                      0x000001ff
#define mc2_chn_ddr_ddr_param_zqc0_tZQCS_ALIGN                     0
#define mc2_chn_ddr_ddr_param_zqc0_tZQCS_BITS                      9
#define mc2_chn_ddr_ddr_param_zqc0_tZQCS_SHIFT                     0
#define mc2_chn_ddr_ddr_param_zqc0_tZQCS_DEFAULT                   0x00000040




#define mc2_chn_ddr_refresh_aref0_refmaxdelay_MASK                 0xf0000000
#define mc2_chn_ddr_refresh_aref0_refmaxdelay_ALIGN                0
#define mc2_chn_ddr_refresh_aref0_refmaxdelay_BITS                 4
#define mc2_chn_ddr_refresh_aref0_refmaxdelay_SHIFT                28
#define mc2_chn_ddr_refresh_aref0_refmaxdelay_DEFAULT              0x00000004


#define mc2_chn_ddr_refresh_aref0_reserved0_MASK                   0x0e000000
#define mc2_chn_ddr_refresh_aref0_reserved0_ALIGN                  0
#define mc2_chn_ddr_refresh_aref0_reserved0_BITS                   3
#define mc2_chn_ddr_refresh_aref0_reserved0_SHIFT                  25


#define mc2_chn_ddr_refresh_aref0_pbref_ordered_MASK               0x01000000
#define mc2_chn_ddr_refresh_aref0_pbref_ordered_ALIGN              0
#define mc2_chn_ddr_refresh_aref0_pbref_ordered_BITS               1
#define mc2_chn_ddr_refresh_aref0_pbref_ordered_SHIFT              24
#define mc2_chn_ddr_refresh_aref0_pbref_ordered_DEFAULT            0x00000000


#define mc2_chn_ddr_refresh_aref0_pbref_intlv_MASK                 0x00ff0000
#define mc2_chn_ddr_refresh_aref0_pbref_intlv_ALIGN                0
#define mc2_chn_ddr_refresh_aref0_pbref_intlv_BITS                 8
#define mc2_chn_ddr_refresh_aref0_pbref_intlv_SHIFT                16
#define mc2_chn_ddr_refresh_aref0_pbref_intlv_DEFAULT              0x00000000


#define mc2_chn_ddr_refresh_aref0_abref_num_MASK                   0x0000f000
#define mc2_chn_ddr_refresh_aref0_abref_num_ALIGN                  0
#define mc2_chn_ddr_refresh_aref0_abref_num_BITS                   4
#define mc2_chn_ddr_refresh_aref0_abref_num_SHIFT                  12
#define mc2_chn_ddr_refresh_aref0_abref_num_DEFAULT                0x00000001


#define mc2_chn_ddr_refresh_aref0_reserved1_MASK                   0x00000c00
#define mc2_chn_ddr_refresh_aref0_reserved1_ALIGN                  0
#define mc2_chn_ddr_refresh_aref0_reserved1_BITS                   2
#define mc2_chn_ddr_refresh_aref0_reserved1_SHIFT                  10


#define mc2_chn_ddr_refresh_aref0_tREFI_MASK                       0x000003ff
#define mc2_chn_ddr_refresh_aref0_tREFI_ALIGN                      0
#define mc2_chn_ddr_refresh_aref0_tREFI_BITS                       10
#define mc2_chn_ddr_refresh_aref0_tREFI_SHIFT                      0
#define mc2_chn_ddr_refresh_aref0_tREFI_DEFAULT                    0x00000103




#define mc2_chn_ddr_refresh_aref1_refdisable_MASK                  0x80000000
#define mc2_chn_ddr_refresh_aref1_refdisable_ALIGN                 0
#define mc2_chn_ddr_refresh_aref1_refdisable_BITS                  1
#define mc2_chn_ddr_refresh_aref1_refdisable_SHIFT                 31
#define mc2_chn_ddr_refresh_aref1_refdisable_DEFAULT               0x00000000


#define mc2_chn_ddr_refresh_aref1_reserved0_MASK                   0x70000000
#define mc2_chn_ddr_refresh_aref1_reserved0_ALIGN                  0
#define mc2_chn_ddr_refresh_aref1_reserved0_BITS                   3
#define mc2_chn_ddr_refresh_aref1_reserved0_SHIFT                  28


#define mc2_chn_ddr_refresh_aref1_tRFCpb_MASK                      0x0fff0000
#define mc2_chn_ddr_refresh_aref1_tRFCpb_ALIGN                     0
#define mc2_chn_ddr_refresh_aref1_tRFCpb_BITS                      12
#define mc2_chn_ddr_refresh_aref1_tRFCpb_SHIFT                     16
#define mc2_chn_ddr_refresh_aref1_tRFCpb_DEFAULT                   0x0000008d


#define mc2_chn_ddr_refresh_aref1_reserved1_MASK                   0x0000f000
#define mc2_chn_ddr_refresh_aref1_reserved1_ALIGN                  0
#define mc2_chn_ddr_refresh_aref1_reserved1_BITS                   4
#define mc2_chn_ddr_refresh_aref1_reserved1_SHIFT                  12


#define mc2_chn_ddr_refresh_aref1_tRFCab_MASK                      0x00000fff
#define mc2_chn_ddr_refresh_aref1_tRFCab_ALIGN                     0
#define mc2_chn_ddr_refresh_aref1_tRFCab_BITS                      12
#define mc2_chn_ddr_refresh_aref1_tRFCab_SHIFT                     0
#define mc2_chn_ddr_refresh_aref1_tRFCab_DEFAULT                   0x0000011a




#define mc2_chn_ddr_auto_self_refresh_enable_MASK                  0x80000000
#define mc2_chn_ddr_auto_self_refresh_enable_ALIGN                 0
#define mc2_chn_ddr_auto_self_refresh_enable_BITS                  1
#define mc2_chn_ddr_auto_self_refresh_enable_SHIFT                 31
#define mc2_chn_ddr_auto_self_refresh_enable_DEFAULT               0x00000000


#define mc2_chn_ddr_auto_self_refresh_immediate_MASK               0x40000000
#define mc2_chn_ddr_auto_self_refresh_immediate_ALIGN              0
#define mc2_chn_ddr_auto_self_refresh_immediate_BITS               1
#define mc2_chn_ddr_auto_self_refresh_immediate_SHIFT              30
#define mc2_chn_ddr_auto_self_refresh_immediate_DEFAULT            0x00000000


#define mc2_chn_ddr_auto_self_refresh_idle_count_MASK              0x3fffffff
#define mc2_chn_ddr_auto_self_refresh_idle_count_ALIGN             0
#define mc2_chn_ddr_auto_self_refresh_idle_count_BITS              30
#define mc2_chn_ddr_auto_self_refresh_idle_count_SHIFT             0
#define mc2_chn_ddr_auto_self_refresh_idle_count_DEFAULT           0x00000000




#define mc2_chn_ddr_auto_zqcs_enable_MASK                          0x80000000
#define mc2_chn_ddr_auto_zqcs_enable_ALIGN                         0
#define mc2_chn_ddr_auto_zqcs_enable_BITS                          1
#define mc2_chn_ddr_auto_zqcs_enable_SHIFT                         31
#define mc2_chn_ddr_auto_zqcs_enable_DEFAULT                       0x00000000


#define mc2_chn_ddr_auto_zqcs_timer_count_MASK                     0x7fffffe0
#define mc2_chn_ddr_auto_zqcs_timer_count_ALIGN                    0
#define mc2_chn_ddr_auto_zqcs_timer_count_BITS                     26
#define mc2_chn_ddr_auto_zqcs_timer_count_SHIFT                    5
#define mc2_chn_ddr_auto_zqcs_timer_count_DEFAULT                  0x00000000


#define mc2_chn_ddr_auto_zqcs_reserved0_MASK                       0x0000001f
#define mc2_chn_ddr_auto_zqcs_reserved0_ALIGN                      0
#define mc2_chn_ddr_auto_zqcs_reserved0_BITS                       5
#define mc2_chn_ddr_auto_zqcs_reserved0_SHIFT                      0




#define mc2_chn_ddr_dfi_error_dfi_error_b_vld_MASK                 0x80000000
#define mc2_chn_ddr_dfi_error_dfi_error_b_vld_ALIGN                0
#define mc2_chn_ddr_dfi_error_dfi_error_b_vld_BITS                 1
#define mc2_chn_ddr_dfi_error_dfi_error_b_vld_SHIFT                31
#define mc2_chn_ddr_dfi_error_dfi_error_b_vld_DEFAULT              0x00000000


#define mc2_chn_ddr_dfi_error_dfi_error_b_clr_MASK                 0x40000000
#define mc2_chn_ddr_dfi_error_dfi_error_b_clr_ALIGN                0
#define mc2_chn_ddr_dfi_error_dfi_error_b_clr_BITS                 1
#define mc2_chn_ddr_dfi_error_dfi_error_b_clr_SHIFT                30
#define mc2_chn_ddr_dfi_error_dfi_error_b_clr_DEFAULT              0x00000000


#define mc2_chn_ddr_dfi_error_reserved0_MASK                       0x3f000000
#define mc2_chn_ddr_dfi_error_reserved0_ALIGN                      0
#define mc2_chn_ddr_dfi_error_reserved0_BITS                       6
#define mc2_chn_ddr_dfi_error_reserved0_SHIFT                      24


#define mc2_chn_ddr_dfi_error_dfi_error_b_info_MASK                0x00ff0000
#define mc2_chn_ddr_dfi_error_dfi_error_b_info_ALIGN               0
#define mc2_chn_ddr_dfi_error_dfi_error_b_info_BITS                8
#define mc2_chn_ddr_dfi_error_dfi_error_b_info_SHIFT               16
#define mc2_chn_ddr_dfi_error_dfi_error_b_info_DEFAULT             0x00000000


#define mc2_chn_ddr_dfi_error_dfi_error_a_vld_MASK                 0x00008000
#define mc2_chn_ddr_dfi_error_dfi_error_a_vld_ALIGN                0
#define mc2_chn_ddr_dfi_error_dfi_error_a_vld_BITS                 1
#define mc2_chn_ddr_dfi_error_dfi_error_a_vld_SHIFT                15
#define mc2_chn_ddr_dfi_error_dfi_error_a_vld_DEFAULT              0x00000000


#define mc2_chn_ddr_dfi_error_dfi_error_a_clr_MASK                 0x00004000
#define mc2_chn_ddr_dfi_error_dfi_error_a_clr_ALIGN                0
#define mc2_chn_ddr_dfi_error_dfi_error_a_clr_BITS                 1
#define mc2_chn_ddr_dfi_error_dfi_error_a_clr_SHIFT                14
#define mc2_chn_ddr_dfi_error_dfi_error_a_clr_DEFAULT              0x00000000


#define mc2_chn_ddr_dfi_error_reserved1_MASK                       0x00003f00
#define mc2_chn_ddr_dfi_error_reserved1_ALIGN                      0
#define mc2_chn_ddr_dfi_error_reserved1_BITS                       6
#define mc2_chn_ddr_dfi_error_reserved1_SHIFT                      8


#define mc2_chn_ddr_dfi_error_dfi_error_a_info_MASK                0x000000ff
#define mc2_chn_ddr_dfi_error_dfi_error_a_info_ALIGN               0
#define mc2_chn_ddr_dfi_error_dfi_error_a_info_BITS                8
#define mc2_chn_ddr_dfi_error_dfi_error_a_info_SHIFT               0
#define mc2_chn_ddr_dfi_error_dfi_error_a_info_DEFAULT             0x00000000


#define mc2_afx_acc_acc_eack_MASK                                  0x80000000
#define mc2_afx_acc_acc_eack_ALIGN                                 0
#define mc2_afx_acc_acc_eack_BITS                                  1
#define mc2_afx_acc_acc_eack_SHIFT                                 31
#define mc2_afx_acc_acc_eack_DEFAULT                               0x00000000


#define mc2_afx_acc_reserved0_MASK                                 0x7fffff00
#define mc2_afx_acc_reserved0_ALIGN                                0
#define mc2_afx_acc_reserved0_BITS                                 23
#define mc2_afx_acc_reserved0_SHIFT                                8


#define mc2_afx_acc_acc_sw_MASK                                    0x00000080
#define mc2_afx_acc_acc_sw_ALIGN                                   0
#define mc2_afx_acc_acc_sw_BITS                                    1
#define mc2_afx_acc_acc_sw_SHIFT                                   7
#define mc2_afx_acc_acc_sw_DEFAULT                                 0x00000001


#define mc2_afx_acc_acc_sr_MASK                                    0x00000040
#define mc2_afx_acc_acc_sr_ALIGN                                   0
#define mc2_afx_acc_acc_sr_BITS                                    1
#define mc2_afx_acc_acc_sr_SHIFT                                   6
#define mc2_afx_acc_acc_sr_DEFAULT                                 0x00000001


#define mc2_afx_acc_acc_nsw_MASK                                   0x00000020
#define mc2_afx_acc_acc_nsw_ALIGN                                  0
#define mc2_afx_acc_acc_nsw_BITS                                   1
#define mc2_afx_acc_acc_nsw_SHIFT                                  5
#define mc2_afx_acc_acc_nsw_DEFAULT                                0x00000001


#define mc2_afx_acc_acc_nsr_MASK                                   0x00000010
#define mc2_afx_acc_acc_nsr_ALIGN                                  0
#define mc2_afx_acc_acc_nsr_BITS                                   1
#define mc2_afx_acc_acc_nsr_SHIFT                                  4
#define mc2_afx_acc_acc_nsr_DEFAULT                                0x00000001


#define mc2_afx_acc_perm_sw_MASK                                   0x00000008
#define mc2_afx_acc_perm_sw_ALIGN                                  0
#define mc2_afx_acc_perm_sw_BITS                                   1
#define mc2_afx_acc_perm_sw_SHIFT                                  3
#define mc2_afx_acc_perm_sw_DEFAULT                                0x00000001


#define mc2_afx_acc_perm_sr_MASK                                   0x00000004
#define mc2_afx_acc_perm_sr_ALIGN                                  0
#define mc2_afx_acc_perm_sr_BITS                                   1
#define mc2_afx_acc_perm_sr_SHIFT                                  2
#define mc2_afx_acc_perm_sr_DEFAULT                                0x00000001


#define mc2_afx_acc_perm_nsw_MASK                                  0x00000002
#define mc2_afx_acc_perm_nsw_ALIGN                                 0
#define mc2_afx_acc_perm_nsw_BITS                                  1
#define mc2_afx_acc_perm_nsw_SHIFT                                 1
#define mc2_afx_acc_perm_nsw_DEFAULT                               0x00000001


#define mc2_afx_acc_perm_nsr_MASK                                  0x00000001
#define mc2_afx_acc_perm_nsr_ALIGN                                 0
#define mc2_afx_acc_perm_nsr_BITS                                  1
#define mc2_afx_acc_perm_nsr_SHIFT                                 0
#define mc2_afx_acc_perm_nsr_DEFAULT                               0x00000001




#define mc2_afx_ver_reserved0_MASK                                 0xffffff00
#define mc2_afx_ver_reserved0_ALIGN                                0
#define mc2_afx_ver_reserved0_BITS                                 24
#define mc2_afx_ver_reserved0_SHIFT                                8


#define mc2_afx_ver_version_MASK                                   0x000000ff
#define mc2_afx_ver_version_ALIGN                                  0
#define mc2_afx_ver_version_BITS                                   8
#define mc2_afx_ver_version_SHIFT                                  0
#define mc2_afx_ver_version_DEFAULT                                0x00000000




#define mc2_afx_sram_match_cfg_sram_start_addr_hi_enable_MASK      0x80000000
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_enable_ALIGN     0
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_enable_BITS      1
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_enable_SHIFT     31
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_enable_DEFAULT   0x00000001


#define mc2_afx_sram_match_cfg_sram_start_addr_hi_reserved0_MASK   0x7fffff00
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_reserved0_ALIGN  0
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_reserved0_BITS   23
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_reserved0_SHIFT  8


#define mc2_afx_sram_match_cfg_sram_start_addr_hi_addr_hi_MASK     0x000000ff
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_addr_hi_ALIGN    0
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_addr_hi_BITS     8
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_addr_hi_SHIFT    0
#define mc2_afx_sram_match_cfg_sram_start_addr_hi_addr_hi_DEFAULT  0x00000001




#define mc2_afx_sram_match_cfg_sram_start_addr_lo_addr_lo_MASK     0xffffffff
#define mc2_afx_sram_match_cfg_sram_start_addr_lo_addr_lo_ALIGN    0
#define mc2_afx_sram_match_cfg_sram_start_addr_lo_addr_lo_BITS     32
#define mc2_afx_sram_match_cfg_sram_start_addr_lo_addr_lo_SHIFT    0
#define mc2_afx_sram_match_cfg_sram_start_addr_lo_addr_lo_DEFAULT  0x7fff0000




#define mc2_afx_sram_match_cfg_sram_end_addr_hi_reserved0_MASK     0xffffff00
#define mc2_afx_sram_match_cfg_sram_end_addr_hi_reserved0_ALIGN    0
#define mc2_afx_sram_match_cfg_sram_end_addr_hi_reserved0_BITS     24
#define mc2_afx_sram_match_cfg_sram_end_addr_hi_reserved0_SHIFT    8


#define mc2_afx_sram_match_cfg_sram_end_addr_hi_addr_hi_MASK       0x000000ff
#define mc2_afx_sram_match_cfg_sram_end_addr_hi_addr_hi_ALIGN      0
#define mc2_afx_sram_match_cfg_sram_end_addr_hi_addr_hi_BITS       8
#define mc2_afx_sram_match_cfg_sram_end_addr_hi_addr_hi_SHIFT      0
#define mc2_afx_sram_match_cfg_sram_end_addr_hi_addr_hi_DEFAULT    0x00000001




#define mc2_afx_sram_match_cfg_sram_end_addr_lo_addr_lo_MASK       0xffffffff
#define mc2_afx_sram_match_cfg_sram_end_addr_lo_addr_lo_ALIGN      0
#define mc2_afx_sram_match_cfg_sram_end_addr_lo_addr_lo_BITS       32
#define mc2_afx_sram_match_cfg_sram_end_addr_lo_addr_lo_SHIFT      0
#define mc2_afx_sram_match_cfg_sram_end_addr_lo_addr_lo_DEFAULT    0x7fffffff



#define mc2_afx_addr_fltr_cfg0_start_addr_hi_enable_MASK           0xf0000000
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_enable_ALIGN          0
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_enable_BITS           4
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_enable_SHIFT          28
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_enable_DEFAULT        0x00000000


#define mc2_afx_addr_fltr_cfg0_start_addr_hi_reserved0_MASK        0x0fffff00
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_reserved0_ALIGN       0
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_reserved0_BITS        20
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_reserved0_SHIFT       8


#define mc2_afx_addr_fltr_cfg0_start_addr_hi_addr_hi_MASK          0x000000ff
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_addr_hi_ALIGN         0
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_addr_hi_BITS          8
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_addr_hi_SHIFT         0
#define mc2_afx_addr_fltr_cfg0_start_addr_hi_addr_hi_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg0_start_addr_lo_addr_lo_MASK          0xffffffff
#define mc2_afx_addr_fltr_cfg0_start_addr_lo_addr_lo_ALIGN         0
#define mc2_afx_addr_fltr_cfg0_start_addr_lo_addr_lo_BITS          32
#define mc2_afx_addr_fltr_cfg0_start_addr_lo_addr_lo_SHIFT         0
#define mc2_afx_addr_fltr_cfg0_start_addr_lo_addr_lo_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg0_end_addr_hi_reserved0_MASK          0xffffff00
#define mc2_afx_addr_fltr_cfg0_end_addr_hi_reserved0_ALIGN         0
#define mc2_afx_addr_fltr_cfg0_end_addr_hi_reserved0_BITS          24
#define mc2_afx_addr_fltr_cfg0_end_addr_hi_reserved0_SHIFT         8


#define mc2_afx_addr_fltr_cfg0_end_addr_hi_addr_hi_MASK            0x000000ff
#define mc2_afx_addr_fltr_cfg0_end_addr_hi_addr_hi_ALIGN           0
#define mc2_afx_addr_fltr_cfg0_end_addr_hi_addr_hi_BITS            8
#define mc2_afx_addr_fltr_cfg0_end_addr_hi_addr_hi_SHIFT           0
#define mc2_afx_addr_fltr_cfg0_end_addr_hi_addr_hi_DEFAULT         0x00000000




#define mc2_afx_addr_fltr_cfg0_end_addr_lo_addr_lo_MASK            0xffffffff
#define mc2_afx_addr_fltr_cfg0_end_addr_lo_addr_lo_ALIGN           0
#define mc2_afx_addr_fltr_cfg0_end_addr_lo_addr_lo_BITS            32
#define mc2_afx_addr_fltr_cfg0_end_addr_lo_addr_lo_SHIFT           0
#define mc2_afx_addr_fltr_cfg0_end_addr_lo_addr_lo_DEFAULT         0x00000000




#define mc2_afx_addr_fltr_cfg1_start_addr_hi_enable_MASK           0xf0000000
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_enable_ALIGN          0
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_enable_BITS           4
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_enable_SHIFT          28
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_enable_DEFAULT        0x00000000


#define mc2_afx_addr_fltr_cfg1_start_addr_hi_reserved0_MASK        0x0fffff00
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_reserved0_ALIGN       0
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_reserved0_BITS        20
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_reserved0_SHIFT       8


#define mc2_afx_addr_fltr_cfg1_start_addr_hi_addr_hi_MASK          0x000000ff
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_addr_hi_ALIGN         0
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_addr_hi_BITS          8
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_addr_hi_SHIFT         0
#define mc2_afx_addr_fltr_cfg1_start_addr_hi_addr_hi_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg1_start_addr_lo_addr_lo_MASK          0xffffffff
#define mc2_afx_addr_fltr_cfg1_start_addr_lo_addr_lo_ALIGN         0
#define mc2_afx_addr_fltr_cfg1_start_addr_lo_addr_lo_BITS          32
#define mc2_afx_addr_fltr_cfg1_start_addr_lo_addr_lo_SHIFT         0
#define mc2_afx_addr_fltr_cfg1_start_addr_lo_addr_lo_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg1_end_addr_hi_reserved0_MASK          0xffffff00
#define mc2_afx_addr_fltr_cfg1_end_addr_hi_reserved0_ALIGN         0
#define mc2_afx_addr_fltr_cfg1_end_addr_hi_reserved0_BITS          24
#define mc2_afx_addr_fltr_cfg1_end_addr_hi_reserved0_SHIFT         8


#define mc2_afx_addr_fltr_cfg1_end_addr_hi_addr_hi_MASK            0x000000ff
#define mc2_afx_addr_fltr_cfg1_end_addr_hi_addr_hi_ALIGN           0
#define mc2_afx_addr_fltr_cfg1_end_addr_hi_addr_hi_BITS            8
#define mc2_afx_addr_fltr_cfg1_end_addr_hi_addr_hi_SHIFT           0
#define mc2_afx_addr_fltr_cfg1_end_addr_hi_addr_hi_DEFAULT         0x00000000




#define mc2_afx_addr_fltr_cfg1_end_addr_lo_addr_lo_MASK            0xffffffff
#define mc2_afx_addr_fltr_cfg1_end_addr_lo_addr_lo_ALIGN           0
#define mc2_afx_addr_fltr_cfg1_end_addr_lo_addr_lo_BITS            32
#define mc2_afx_addr_fltr_cfg1_end_addr_lo_addr_lo_SHIFT           0
#define mc2_afx_addr_fltr_cfg1_end_addr_lo_addr_lo_DEFAULT         0x00000000




#define mc2_afx_addr_fltr_cfg2_start_addr_hi_enable_MASK           0xf0000000
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_enable_ALIGN          0
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_enable_BITS           4
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_enable_SHIFT          28
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_enable_DEFAULT        0x00000000


#define mc2_afx_addr_fltr_cfg2_start_addr_hi_reserved0_MASK        0x0fffff00
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_reserved0_ALIGN       0
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_reserved0_BITS        20
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_reserved0_SHIFT       8


#define mc2_afx_addr_fltr_cfg2_start_addr_hi_addr_hi_MASK          0x000000ff
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_addr_hi_ALIGN         0
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_addr_hi_BITS          8
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_addr_hi_SHIFT         0
#define mc2_afx_addr_fltr_cfg2_start_addr_hi_addr_hi_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg2_start_addr_lo_addr_lo_MASK          0xffffffff
#define mc2_afx_addr_fltr_cfg2_start_addr_lo_addr_lo_ALIGN         0
#define mc2_afx_addr_fltr_cfg2_start_addr_lo_addr_lo_BITS          32
#define mc2_afx_addr_fltr_cfg2_start_addr_lo_addr_lo_SHIFT         0
#define mc2_afx_addr_fltr_cfg2_start_addr_lo_addr_lo_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg2_end_addr_hi_reserved0_MASK          0xffffff00
#define mc2_afx_addr_fltr_cfg2_end_addr_hi_reserved0_ALIGN         0
#define mc2_afx_addr_fltr_cfg2_end_addr_hi_reserved0_BITS          24
#define mc2_afx_addr_fltr_cfg2_end_addr_hi_reserved0_SHIFT         8


#define mc2_afx_addr_fltr_cfg2_end_addr_hi_addr_hi_MASK            0x000000ff
#define mc2_afx_addr_fltr_cfg2_end_addr_hi_addr_hi_ALIGN           0
#define mc2_afx_addr_fltr_cfg2_end_addr_hi_addr_hi_BITS            8
#define mc2_afx_addr_fltr_cfg2_end_addr_hi_addr_hi_SHIFT           0
#define mc2_afx_addr_fltr_cfg2_end_addr_hi_addr_hi_DEFAULT         0x00000000




#define mc2_afx_addr_fltr_cfg2_end_addr_lo_addr_lo_MASK            0xffffffff
#define mc2_afx_addr_fltr_cfg2_end_addr_lo_addr_lo_ALIGN           0
#define mc2_afx_addr_fltr_cfg2_end_addr_lo_addr_lo_BITS            32
#define mc2_afx_addr_fltr_cfg2_end_addr_lo_addr_lo_SHIFT           0
#define mc2_afx_addr_fltr_cfg2_end_addr_lo_addr_lo_DEFAULT         0x00000000




#define mc2_afx_addr_fltr_cfg3_start_addr_hi_enable_MASK           0xf0000000
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_enable_ALIGN          0
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_enable_BITS           4
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_enable_SHIFT          28
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_enable_DEFAULT        0x00000000


#define mc2_afx_addr_fltr_cfg3_start_addr_hi_reserved0_MASK        0x0fffff00
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_reserved0_ALIGN       0
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_reserved0_BITS        20
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_reserved0_SHIFT       8


#define mc2_afx_addr_fltr_cfg3_start_addr_hi_addr_hi_MASK          0x000000ff
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_addr_hi_ALIGN         0
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_addr_hi_BITS          8
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_addr_hi_SHIFT         0
#define mc2_afx_addr_fltr_cfg3_start_addr_hi_addr_hi_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg3_start_addr_lo_addr_lo_MASK          0xffffffff
#define mc2_afx_addr_fltr_cfg3_start_addr_lo_addr_lo_ALIGN         0
#define mc2_afx_addr_fltr_cfg3_start_addr_lo_addr_lo_BITS          32
#define mc2_afx_addr_fltr_cfg3_start_addr_lo_addr_lo_SHIFT         0
#define mc2_afx_addr_fltr_cfg3_start_addr_lo_addr_lo_DEFAULT       0x00000000




#define mc2_afx_addr_fltr_cfg3_end_addr_hi_reserved0_MASK          0xffffff00
#define mc2_afx_addr_fltr_cfg3_end_addr_hi_reserved0_ALIGN         0
#define mc2_afx_addr_fltr_cfg3_end_addr_hi_reserved0_BITS          24
#define mc2_afx_addr_fltr_cfg3_end_addr_hi_reserved0_SHIFT         8


#define mc2_afx_addr_fltr_cfg3_end_addr_hi_addr_hi_MASK            0x000000ff
#define mc2_afx_addr_fltr_cfg3_end_addr_hi_addr_hi_ALIGN           0
#define mc2_afx_addr_fltr_cfg3_end_addr_hi_addr_hi_BITS            8
#define mc2_afx_addr_fltr_cfg3_end_addr_hi_addr_hi_SHIFT           0
#define mc2_afx_addr_fltr_cfg3_end_addr_hi_addr_hi_DEFAULT         0x00000000




#define mc2_afx_addr_fltr_cfg3_end_addr_lo_addr_lo_MASK            0xffffffff
#define mc2_afx_addr_fltr_cfg3_end_addr_lo_addr_lo_ALIGN           0
#define mc2_afx_addr_fltr_cfg3_end_addr_lo_addr_lo_BITS            32
#define mc2_afx_addr_fltr_cfg3_end_addr_lo_addr_lo_SHIFT           0
#define mc2_afx_addr_fltr_cfg3_end_addr_lo_addr_lo_DEFAULT         0x00000000




#define mc2_afx_srcid_fltr_cfg0_srcid_enable_MASK                  0xf0000000
#define mc2_afx_srcid_fltr_cfg0_srcid_enable_ALIGN                 0
#define mc2_afx_srcid_fltr_cfg0_srcid_enable_BITS                  4
#define mc2_afx_srcid_fltr_cfg0_srcid_enable_SHIFT                 28
#define mc2_afx_srcid_fltr_cfg0_srcid_enable_DEFAULT               0x00000000


#define mc2_afx_srcid_fltr_cfg0_srcid_start_srcid_MASK             0x0fff0000
#define mc2_afx_srcid_fltr_cfg0_srcid_start_srcid_ALIGN            0
#define mc2_afx_srcid_fltr_cfg0_srcid_start_srcid_BITS             12
#define mc2_afx_srcid_fltr_cfg0_srcid_start_srcid_SHIFT            16
#define mc2_afx_srcid_fltr_cfg0_srcid_start_srcid_DEFAULT          0x00000000


#define mc2_afx_srcid_fltr_cfg0_srcid_reserved0_MASK               0x0000f000
#define mc2_afx_srcid_fltr_cfg0_srcid_reserved0_ALIGN              0
#define mc2_afx_srcid_fltr_cfg0_srcid_reserved0_BITS               4
#define mc2_afx_srcid_fltr_cfg0_srcid_reserved0_SHIFT              12


#define mc2_afx_srcid_fltr_cfg0_srcid_end_srcid_MASK               0x00000fff
#define mc2_afx_srcid_fltr_cfg0_srcid_end_srcid_ALIGN              0
#define mc2_afx_srcid_fltr_cfg0_srcid_end_srcid_BITS               12
#define mc2_afx_srcid_fltr_cfg0_srcid_end_srcid_SHIFT              0
#define mc2_afx_srcid_fltr_cfg0_srcid_end_srcid_DEFAULT            0x00000000




#define mc2_afx_srcid_fltr_cfg1_srcid_enable_MASK                  0xf0000000
#define mc2_afx_srcid_fltr_cfg1_srcid_enable_ALIGN                 0
#define mc2_afx_srcid_fltr_cfg1_srcid_enable_BITS                  4
#define mc2_afx_srcid_fltr_cfg1_srcid_enable_SHIFT                 28
#define mc2_afx_srcid_fltr_cfg1_srcid_enable_DEFAULT               0x00000000


#define mc2_afx_srcid_fltr_cfg1_srcid_start_srcid_MASK             0x0fff0000
#define mc2_afx_srcid_fltr_cfg1_srcid_start_srcid_ALIGN            0
#define mc2_afx_srcid_fltr_cfg1_srcid_start_srcid_BITS             12
#define mc2_afx_srcid_fltr_cfg1_srcid_start_srcid_SHIFT            16
#define mc2_afx_srcid_fltr_cfg1_srcid_start_srcid_DEFAULT          0x00000000


#define mc2_afx_srcid_fltr_cfg1_srcid_reserved0_MASK               0x0000f000
#define mc2_afx_srcid_fltr_cfg1_srcid_reserved0_ALIGN              0
#define mc2_afx_srcid_fltr_cfg1_srcid_reserved0_BITS               4
#define mc2_afx_srcid_fltr_cfg1_srcid_reserved0_SHIFT              12


#define mc2_afx_srcid_fltr_cfg1_srcid_end_srcid_MASK               0x00000fff
#define mc2_afx_srcid_fltr_cfg1_srcid_end_srcid_ALIGN              0
#define mc2_afx_srcid_fltr_cfg1_srcid_end_srcid_BITS               12
#define mc2_afx_srcid_fltr_cfg1_srcid_end_srcid_SHIFT              0
#define mc2_afx_srcid_fltr_cfg1_srcid_end_srcid_DEFAULT            0x00000000




#define mc2_afx_srcid_fltr_cfg2_srcid_enable_MASK                  0xf0000000
#define mc2_afx_srcid_fltr_cfg2_srcid_enable_ALIGN                 0
#define mc2_afx_srcid_fltr_cfg2_srcid_enable_BITS                  4
#define mc2_afx_srcid_fltr_cfg2_srcid_enable_SHIFT                 28
#define mc2_afx_srcid_fltr_cfg2_srcid_enable_DEFAULT               0x00000000


#define mc2_afx_srcid_fltr_cfg2_srcid_start_srcid_MASK             0x0fff0000
#define mc2_afx_srcid_fltr_cfg2_srcid_start_srcid_ALIGN            0
#define mc2_afx_srcid_fltr_cfg2_srcid_start_srcid_BITS             12
#define mc2_afx_srcid_fltr_cfg2_srcid_start_srcid_SHIFT            16
#define mc2_afx_srcid_fltr_cfg2_srcid_start_srcid_DEFAULT          0x00000000


#define mc2_afx_srcid_fltr_cfg2_srcid_reserved0_MASK               0x0000f000
#define mc2_afx_srcid_fltr_cfg2_srcid_reserved0_ALIGN              0
#define mc2_afx_srcid_fltr_cfg2_srcid_reserved0_BITS               4
#define mc2_afx_srcid_fltr_cfg2_srcid_reserved0_SHIFT              12


#define mc2_afx_srcid_fltr_cfg2_srcid_end_srcid_MASK               0x00000fff
#define mc2_afx_srcid_fltr_cfg2_srcid_end_srcid_ALIGN              0
#define mc2_afx_srcid_fltr_cfg2_srcid_end_srcid_BITS               12
#define mc2_afx_srcid_fltr_cfg2_srcid_end_srcid_SHIFT              0
#define mc2_afx_srcid_fltr_cfg2_srcid_end_srcid_DEFAULT            0x00000000




#define mc2_afx_srcid_fltr_cfg3_srcid_enable_MASK                  0xf0000000
#define mc2_afx_srcid_fltr_cfg3_srcid_enable_ALIGN                 0
#define mc2_afx_srcid_fltr_cfg3_srcid_enable_BITS                  4
#define mc2_afx_srcid_fltr_cfg3_srcid_enable_SHIFT                 28
#define mc2_afx_srcid_fltr_cfg3_srcid_enable_DEFAULT               0x00000000


#define mc2_afx_srcid_fltr_cfg3_srcid_start_srcid_MASK             0x0fff0000
#define mc2_afx_srcid_fltr_cfg3_srcid_start_srcid_ALIGN            0
#define mc2_afx_srcid_fltr_cfg3_srcid_start_srcid_BITS             12
#define mc2_afx_srcid_fltr_cfg3_srcid_start_srcid_SHIFT            16
#define mc2_afx_srcid_fltr_cfg3_srcid_start_srcid_DEFAULT          0x00000000


#define mc2_afx_srcid_fltr_cfg3_srcid_reserved0_MASK               0x0000f000
#define mc2_afx_srcid_fltr_cfg3_srcid_reserved0_ALIGN              0
#define mc2_afx_srcid_fltr_cfg3_srcid_reserved0_BITS               4
#define mc2_afx_srcid_fltr_cfg3_srcid_reserved0_SHIFT              12


#define mc2_afx_srcid_fltr_cfg3_srcid_end_srcid_MASK               0x00000fff
#define mc2_afx_srcid_fltr_cfg3_srcid_end_srcid_ALIGN              0
#define mc2_afx_srcid_fltr_cfg3_srcid_end_srcid_BITS               12
#define mc2_afx_srcid_fltr_cfg3_srcid_end_srcid_SHIFT              0
#define mc2_afx_srcid_fltr_cfg3_srcid_end_srcid_DEFAULT            0x00000000




#define mc2_afx_row_xtr_cfg_row_19_16_reserved0_MASK               0xffffffc0
#define mc2_afx_row_xtr_cfg_row_19_16_reserved0_ALIGN              0
#define mc2_afx_row_xtr_cfg_row_19_16_reserved0_BITS               26
#define mc2_afx_row_xtr_cfg_row_19_16_reserved0_SHIFT              6


#define mc2_afx_row_xtr_cfg_row_19_16_bit_16_MASK                  0x0000003f
#define mc2_afx_row_xtr_cfg_row_19_16_bit_16_ALIGN                 0
#define mc2_afx_row_xtr_cfg_row_19_16_bit_16_BITS                  6
#define mc2_afx_row_xtr_cfg_row_19_16_bit_16_SHIFT                 0
#define mc2_afx_row_xtr_cfg_row_19_16_bit_16_DEFAULT               0x0000001e




#define mc2_afx_row_xtr_cfg_row_15_12_reserved0_MASK               0xc0000000
#define mc2_afx_row_xtr_cfg_row_15_12_reserved0_ALIGN              0
#define mc2_afx_row_xtr_cfg_row_15_12_reserved0_BITS               2
#define mc2_afx_row_xtr_cfg_row_15_12_reserved0_SHIFT              30


#define mc2_afx_row_xtr_cfg_row_15_12_bit_15_MASK                  0x3f000000
#define mc2_afx_row_xtr_cfg_row_15_12_bit_15_ALIGN                 0
#define mc2_afx_row_xtr_cfg_row_15_12_bit_15_BITS                  6
#define mc2_afx_row_xtr_cfg_row_15_12_bit_15_SHIFT                 24
#define mc2_afx_row_xtr_cfg_row_15_12_bit_15_DEFAULT               0x0000001d


#define mc2_afx_row_xtr_cfg_row_15_12_reserved1_MASK               0x00c00000
#define mc2_afx_row_xtr_cfg_row_15_12_reserved1_ALIGN              0
#define mc2_afx_row_xtr_cfg_row_15_12_reserved1_BITS               2
#define mc2_afx_row_xtr_cfg_row_15_12_reserved1_SHIFT              22


#define mc2_afx_row_xtr_cfg_row_15_12_bit_14_MASK                  0x003f0000
#define mc2_afx_row_xtr_cfg_row_15_12_bit_14_ALIGN                 0
#define mc2_afx_row_xtr_cfg_row_15_12_bit_14_BITS                  6
#define mc2_afx_row_xtr_cfg_row_15_12_bit_14_SHIFT                 16
#define mc2_afx_row_xtr_cfg_row_15_12_bit_14_DEFAULT               0x0000001c


#define mc2_afx_row_xtr_cfg_row_15_12_reserved2_MASK               0x0000c000
#define mc2_afx_row_xtr_cfg_row_15_12_reserved2_ALIGN              0
#define mc2_afx_row_xtr_cfg_row_15_12_reserved2_BITS               2
#define mc2_afx_row_xtr_cfg_row_15_12_reserved2_SHIFT              14


#define mc2_afx_row_xtr_cfg_row_15_12_bit_13_MASK                  0x00003f00
#define mc2_afx_row_xtr_cfg_row_15_12_bit_13_ALIGN                 0
#define mc2_afx_row_xtr_cfg_row_15_12_bit_13_BITS                  6
#define mc2_afx_row_xtr_cfg_row_15_12_bit_13_SHIFT                 8
#define mc2_afx_row_xtr_cfg_row_15_12_bit_13_DEFAULT               0x0000001b


#define mc2_afx_row_xtr_cfg_row_15_12_reserved3_MASK               0x000000c0
#define mc2_afx_row_xtr_cfg_row_15_12_reserved3_ALIGN              0
#define mc2_afx_row_xtr_cfg_row_15_12_reserved3_BITS               2
#define mc2_afx_row_xtr_cfg_row_15_12_reserved3_SHIFT              6


#define mc2_afx_row_xtr_cfg_row_15_12_bit_12_MASK                  0x0000003f
#define mc2_afx_row_xtr_cfg_row_15_12_bit_12_ALIGN                 0
#define mc2_afx_row_xtr_cfg_row_15_12_bit_12_BITS                  6
#define mc2_afx_row_xtr_cfg_row_15_12_bit_12_SHIFT                 0
#define mc2_afx_row_xtr_cfg_row_15_12_bit_12_DEFAULT               0x0000001a




#define mc2_afx_row_xtr_cfg_row_11_8_reserved0_MASK                0xc0000000
#define mc2_afx_row_xtr_cfg_row_11_8_reserved0_ALIGN               0
#define mc2_afx_row_xtr_cfg_row_11_8_reserved0_BITS                2
#define mc2_afx_row_xtr_cfg_row_11_8_reserved0_SHIFT               30


#define mc2_afx_row_xtr_cfg_row_11_8_bit_11_MASK                   0x3f000000
#define mc2_afx_row_xtr_cfg_row_11_8_bit_11_ALIGN                  0
#define mc2_afx_row_xtr_cfg_row_11_8_bit_11_BITS                   6
#define mc2_afx_row_xtr_cfg_row_11_8_bit_11_SHIFT                  24
#define mc2_afx_row_xtr_cfg_row_11_8_bit_11_DEFAULT                0x00000019


#define mc2_afx_row_xtr_cfg_row_11_8_reserved1_MASK                0x00c00000
#define mc2_afx_row_xtr_cfg_row_11_8_reserved1_ALIGN               0
#define mc2_afx_row_xtr_cfg_row_11_8_reserved1_BITS                2
#define mc2_afx_row_xtr_cfg_row_11_8_reserved1_SHIFT               22


#define mc2_afx_row_xtr_cfg_row_11_8_bit_10_MASK                   0x003f0000
#define mc2_afx_row_xtr_cfg_row_11_8_bit_10_ALIGN                  0
#define mc2_afx_row_xtr_cfg_row_11_8_bit_10_BITS                   6
#define mc2_afx_row_xtr_cfg_row_11_8_bit_10_SHIFT                  16
#define mc2_afx_row_xtr_cfg_row_11_8_bit_10_DEFAULT                0x00000018


#define mc2_afx_row_xtr_cfg_row_11_8_reserved2_MASK                0x0000c000
#define mc2_afx_row_xtr_cfg_row_11_8_reserved2_ALIGN               0
#define mc2_afx_row_xtr_cfg_row_11_8_reserved2_BITS                2
#define mc2_afx_row_xtr_cfg_row_11_8_reserved2_SHIFT               14


#define mc2_afx_row_xtr_cfg_row_11_8_bit_9_MASK                    0x00003f00
#define mc2_afx_row_xtr_cfg_row_11_8_bit_9_ALIGN                   0
#define mc2_afx_row_xtr_cfg_row_11_8_bit_9_BITS                    6
#define mc2_afx_row_xtr_cfg_row_11_8_bit_9_SHIFT                   8
#define mc2_afx_row_xtr_cfg_row_11_8_bit_9_DEFAULT                 0x00000017


#define mc2_afx_row_xtr_cfg_row_11_8_reserved3_MASK                0x000000c0
#define mc2_afx_row_xtr_cfg_row_11_8_reserved3_ALIGN               0
#define mc2_afx_row_xtr_cfg_row_11_8_reserved3_BITS                2
#define mc2_afx_row_xtr_cfg_row_11_8_reserved3_SHIFT               6


#define mc2_afx_row_xtr_cfg_row_11_8_bit_8_MASK                    0x0000003f
#define mc2_afx_row_xtr_cfg_row_11_8_bit_8_ALIGN                   0
#define mc2_afx_row_xtr_cfg_row_11_8_bit_8_BITS                    6
#define mc2_afx_row_xtr_cfg_row_11_8_bit_8_SHIFT                   0
#define mc2_afx_row_xtr_cfg_row_11_8_bit_8_DEFAULT                 0x00000016




#define mc2_afx_row_xtr_cfg_row_7_4_reserved0_MASK                 0xc0000000
#define mc2_afx_row_xtr_cfg_row_7_4_reserved0_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_7_4_reserved0_BITS                 2
#define mc2_afx_row_xtr_cfg_row_7_4_reserved0_SHIFT                30


#define mc2_afx_row_xtr_cfg_row_7_4_bit_7_MASK                     0x3f000000
#define mc2_afx_row_xtr_cfg_row_7_4_bit_7_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_7_4_bit_7_BITS                     6
#define mc2_afx_row_xtr_cfg_row_7_4_bit_7_SHIFT                    24
#define mc2_afx_row_xtr_cfg_row_7_4_bit_7_DEFAULT                  0x00000015


#define mc2_afx_row_xtr_cfg_row_7_4_reserved1_MASK                 0x00c00000
#define mc2_afx_row_xtr_cfg_row_7_4_reserved1_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_7_4_reserved1_BITS                 2
#define mc2_afx_row_xtr_cfg_row_7_4_reserved1_SHIFT                22


#define mc2_afx_row_xtr_cfg_row_7_4_bit_6_MASK                     0x003f0000
#define mc2_afx_row_xtr_cfg_row_7_4_bit_6_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_7_4_bit_6_BITS                     6
#define mc2_afx_row_xtr_cfg_row_7_4_bit_6_SHIFT                    16
#define mc2_afx_row_xtr_cfg_row_7_4_bit_6_DEFAULT                  0x00000014


#define mc2_afx_row_xtr_cfg_row_7_4_reserved2_MASK                 0x0000c000
#define mc2_afx_row_xtr_cfg_row_7_4_reserved2_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_7_4_reserved2_BITS                 2
#define mc2_afx_row_xtr_cfg_row_7_4_reserved2_SHIFT                14


#define mc2_afx_row_xtr_cfg_row_7_4_bit_5_MASK                     0x00003f00
#define mc2_afx_row_xtr_cfg_row_7_4_bit_5_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_7_4_bit_5_BITS                     6
#define mc2_afx_row_xtr_cfg_row_7_4_bit_5_SHIFT                    8
#define mc2_afx_row_xtr_cfg_row_7_4_bit_5_DEFAULT                  0x00000013


#define mc2_afx_row_xtr_cfg_row_7_4_reserved3_MASK                 0x000000c0
#define mc2_afx_row_xtr_cfg_row_7_4_reserved3_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_7_4_reserved3_BITS                 2
#define mc2_afx_row_xtr_cfg_row_7_4_reserved3_SHIFT                6


#define mc2_afx_row_xtr_cfg_row_7_4_bit_4_MASK                     0x0000003f
#define mc2_afx_row_xtr_cfg_row_7_4_bit_4_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_7_4_bit_4_BITS                     6
#define mc2_afx_row_xtr_cfg_row_7_4_bit_4_SHIFT                    0
#define mc2_afx_row_xtr_cfg_row_7_4_bit_4_DEFAULT                  0x00000012




#define mc2_afx_row_xtr_cfg_row_3_0_reserved0_MASK                 0xc0000000
#define mc2_afx_row_xtr_cfg_row_3_0_reserved0_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_3_0_reserved0_BITS                 2
#define mc2_afx_row_xtr_cfg_row_3_0_reserved0_SHIFT                30


#define mc2_afx_row_xtr_cfg_row_3_0_bit_3_MASK                     0x3f000000
#define mc2_afx_row_xtr_cfg_row_3_0_bit_3_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_3_0_bit_3_BITS                     6
#define mc2_afx_row_xtr_cfg_row_3_0_bit_3_SHIFT                    24
#define mc2_afx_row_xtr_cfg_row_3_0_bit_3_DEFAULT                  0x00000011


#define mc2_afx_row_xtr_cfg_row_3_0_reserved1_MASK                 0x00c00000
#define mc2_afx_row_xtr_cfg_row_3_0_reserved1_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_3_0_reserved1_BITS                 2
#define mc2_afx_row_xtr_cfg_row_3_0_reserved1_SHIFT                22


#define mc2_afx_row_xtr_cfg_row_3_0_bit_2_MASK                     0x003f0000
#define mc2_afx_row_xtr_cfg_row_3_0_bit_2_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_3_0_bit_2_BITS                     6
#define mc2_afx_row_xtr_cfg_row_3_0_bit_2_SHIFT                    16
#define mc2_afx_row_xtr_cfg_row_3_0_bit_2_DEFAULT                  0x00000010


#define mc2_afx_row_xtr_cfg_row_3_0_reserved2_MASK                 0x0000c000
#define mc2_afx_row_xtr_cfg_row_3_0_reserved2_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_3_0_reserved2_BITS                 2
#define mc2_afx_row_xtr_cfg_row_3_0_reserved2_SHIFT                14


#define mc2_afx_row_xtr_cfg_row_3_0_bit_1_MASK                     0x00003f00
#define mc2_afx_row_xtr_cfg_row_3_0_bit_1_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_3_0_bit_1_BITS                     6
#define mc2_afx_row_xtr_cfg_row_3_0_bit_1_SHIFT                    8
#define mc2_afx_row_xtr_cfg_row_3_0_bit_1_DEFAULT                  0x0000000f


#define mc2_afx_row_xtr_cfg_row_3_0_reserved3_MASK                 0x000000c0
#define mc2_afx_row_xtr_cfg_row_3_0_reserved3_ALIGN                0
#define mc2_afx_row_xtr_cfg_row_3_0_reserved3_BITS                 2
#define mc2_afx_row_xtr_cfg_row_3_0_reserved3_SHIFT                6


#define mc2_afx_row_xtr_cfg_row_3_0_bit_0_MASK                     0x0000003f
#define mc2_afx_row_xtr_cfg_row_3_0_bit_0_ALIGN                    0
#define mc2_afx_row_xtr_cfg_row_3_0_bit_0_BITS                     6
#define mc2_afx_row_xtr_cfg_row_3_0_bit_0_SHIFT                    0
#define mc2_afx_row_xtr_cfg_row_3_0_bit_0_DEFAULT                  0x0000000e




#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved0_MASK                   0xc0000000
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved0_ALIGN                  0
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved0_BITS                   2
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved0_SHIFT                  30


#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_3_MASK                       0x3f000000
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_3_ALIGN                      0
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_3_BITS                       6
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_3_SHIFT                      24
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_3_DEFAULT                    0x00000000


#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved1_MASK                   0x00c00000
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved1_ALIGN                  0
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved1_BITS                   2
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved1_SHIFT                  22


#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_2_MASK                       0x003f0000
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_2_ALIGN                      0
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_2_BITS                       6
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_2_SHIFT                      16
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_2_DEFAULT                    0x00000000


#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved2_MASK                   0x0000c000
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved2_ALIGN                  0
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved2_BITS                   2
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved2_SHIFT                  14


#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_1_MASK                       0x00003f00
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_1_ALIGN                      0
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_1_BITS                       6
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_1_SHIFT                      8
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_1_DEFAULT                    0x00000000


#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved3_MASK                   0x000000c0
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved3_ALIGN                  0
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved3_BITS                   2
#define mc2_afx_bg_xtr_cfg_bg_3_0_reserved3_SHIFT                  6


#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_0_MASK                       0x0000003f
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_0_ALIGN                      0
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_0_BITS                       6
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_0_SHIFT                      0
#define mc2_afx_bg_xtr_cfg_bg_3_0_bit_0_DEFAULT                    0x00000000




#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved0_MASK                   0xc0000000
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved0_ALIGN                  0
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved0_BITS                   2
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved0_SHIFT                  30


#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_3_MASK                       0x3f000000
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_3_ALIGN                      0
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_3_BITS                       6
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_3_SHIFT                      24
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_3_DEFAULT                    0x00000000


#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved1_MASK                   0x00c00000
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved1_ALIGN                  0
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved1_BITS                   2
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved1_SHIFT                  22


#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_2_MASK                       0x003f0000
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_2_ALIGN                      0
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_2_BITS                       6
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_2_SHIFT                      16
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_2_DEFAULT                    0x0000000d


#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved2_MASK                   0x0000c000
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved2_ALIGN                  0
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved2_BITS                   2
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved2_SHIFT                  14


#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_1_MASK                       0x00003f00
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_1_ALIGN                      0
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_1_BITS                       6
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_1_SHIFT                      8
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_1_DEFAULT                    0x0000000c


#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved3_MASK                   0x000000c0
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved3_ALIGN                  0
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved3_BITS                   2
#define mc2_afx_bk_xtr_cfg_bk_3_0_reserved3_SHIFT                  6


#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_0_MASK                       0x0000003f
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_0_ALIGN                      0
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_0_BITS                       6
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_0_SHIFT                      0
#define mc2_afx_bk_xtr_cfg_bk_3_0_bit_0_DEFAULT                    0x0000000b




#define mc2_afx_col_xtr_cfg_col_cfg_reserved0_MASK                 0xffffffc0
#define mc2_afx_col_xtr_cfg_col_cfg_reserved0_ALIGN                0
#define mc2_afx_col_xtr_cfg_col_cfg_reserved0_BITS                 26
#define mc2_afx_col_xtr_cfg_col_cfg_reserved0_SHIFT                6


#define mc2_afx_col_xtr_cfg_col_cfg_col_bit_start_MASK             0x00000030
#define mc2_afx_col_xtr_cfg_col_cfg_col_bit_start_ALIGN            0
#define mc2_afx_col_xtr_cfg_col_cfg_col_bit_start_BITS             2
#define mc2_afx_col_xtr_cfg_col_cfg_col_bit_start_SHIFT            4
#define mc2_afx_col_xtr_cfg_col_cfg_col_bit_start_DEFAULT          0x00000001


#define mc2_afx_col_xtr_cfg_col_cfg_col_mode_MASK                  0x0000000c
#define mc2_afx_col_xtr_cfg_col_cfg_col_mode_ALIGN                 0
#define mc2_afx_col_xtr_cfg_col_cfg_col_mode_BITS                  2
#define mc2_afx_col_xtr_cfg_col_cfg_col_mode_SHIFT                 2
#define mc2_afx_col_xtr_cfg_col_cfg_col_mode_DEFAULT               0x00000000


#define mc2_afx_col_xtr_cfg_col_cfg_num_col_bits_MASK              0x00000003
#define mc2_afx_col_xtr_cfg_col_cfg_num_col_bits_ALIGN             0
#define mc2_afx_col_xtr_cfg_col_cfg_num_col_bits_BITS              2
#define mc2_afx_col_xtr_cfg_col_cfg_num_col_bits_SHIFT             0
#define mc2_afx_col_xtr_cfg_col_cfg_num_col_bits_DEFAULT           0x00000001




#define mc2_afx_cs_xtr_cfg_cs_3_0_enable_MASK                      0x80000000
#define mc2_afx_cs_xtr_cfg_cs_3_0_enable_ALIGN                     0
#define mc2_afx_cs_xtr_cfg_cs_3_0_enable_BITS                      1
#define mc2_afx_cs_xtr_cfg_cs_3_0_enable_SHIFT                     31
#define mc2_afx_cs_xtr_cfg_cs_3_0_enable_DEFAULT                   0x00000000


#define mc2_afx_cs_xtr_cfg_cs_3_0_reserved0_MASK                   0x7fffffc0
#define mc2_afx_cs_xtr_cfg_cs_3_0_reserved0_ALIGN                  0
#define mc2_afx_cs_xtr_cfg_cs_3_0_reserved0_BITS                   25
#define mc2_afx_cs_xtr_cfg_cs_3_0_reserved0_SHIFT                  6


#define mc2_afx_cs_xtr_cfg_cs_3_0_bit_0_MASK                       0x0000003f
#define mc2_afx_cs_xtr_cfg_cs_3_0_bit_0_ALIGN                      0
#define mc2_afx_cs_xtr_cfg_cs_3_0_bit_0_BITS                       6
#define mc2_afx_cs_xtr_cfg_cs_3_0_bit_0_SHIFT                      0
#define mc2_afx_cs_xtr_cfg_cs_3_0_bit_0_DEFAULT                    0x0000000b




#define mc2_afx_chn_xtr_cfg_chn_bit_enable_MASK                    0x80000000
#define mc2_afx_chn_xtr_cfg_chn_bit_enable_ALIGN                   0
#define mc2_afx_chn_xtr_cfg_chn_bit_enable_BITS                    1
#define mc2_afx_chn_xtr_cfg_chn_bit_enable_SHIFT                   31
#define mc2_afx_chn_xtr_cfg_chn_bit_enable_DEFAULT                 0x00000001


#define mc2_afx_chn_xtr_cfg_chn_bit_reserved0_MASK                 0x7ffffc00
#define mc2_afx_chn_xtr_cfg_chn_bit_reserved0_ALIGN                0
#define mc2_afx_chn_xtr_cfg_chn_bit_reserved0_BITS                 21
#define mc2_afx_chn_xtr_cfg_chn_bit_reserved0_SHIFT                10


#define mc2_afx_chn_xtr_cfg_chn_bit_chn_mode_MASK                  0x00000300
#define mc2_afx_chn_xtr_cfg_chn_bit_chn_mode_ALIGN                 0
#define mc2_afx_chn_xtr_cfg_chn_bit_chn_mode_BITS                  2
#define mc2_afx_chn_xtr_cfg_chn_bit_chn_mode_SHIFT                 8
#define mc2_afx_chn_xtr_cfg_chn_bit_chn_mode_DEFAULT               0x00000000


#define mc2_afx_chn_xtr_cfg_chn_bit_reserved1_MASK                 0x000000c0
#define mc2_afx_chn_xtr_cfg_chn_bit_reserved1_ALIGN                0
#define mc2_afx_chn_xtr_cfg_chn_bit_reserved1_BITS                 2
#define mc2_afx_chn_xtr_cfg_chn_bit_reserved1_SHIFT                6


#define mc2_afx_chn_xtr_cfg_chn_bit_bit_0_MASK                     0x0000003f
#define mc2_afx_chn_xtr_cfg_chn_bit_bit_0_ALIGN                    0
#define mc2_afx_chn_xtr_cfg_chn_bit_bit_0_BITS                     6
#define mc2_afx_chn_xtr_cfg_chn_bit_bit_0_SHIFT                    0
#define mc2_afx_chn_xtr_cfg_chn_bit_bit_0_DEFAULT                  0x0000000a




#define mc2_afx_ddr_sz_chk_reserved0_MASK                          0xffffff00
#define mc2_afx_ddr_sz_chk_reserved0_ALIGN                         0
#define mc2_afx_ddr_sz_chk_reserved0_BITS                          24
#define mc2_afx_ddr_sz_chk_reserved0_SHIFT                         8


#define mc2_afx_ddr_sz_chk_dram_size_limit_MASK                    0x000000f0
#define mc2_afx_ddr_sz_chk_dram_size_limit_ALIGN                   0
#define mc2_afx_ddr_sz_chk_dram_size_limit_BITS                    4
#define mc2_afx_ddr_sz_chk_dram_size_limit_SHIFT                   4
#define mc2_afx_ddr_sz_chk_dram_size_limit_DEFAULT                 0x0000000c


#define mc2_afx_ddr_sz_chk_reserved1_MASK                          0x0000000f
#define mc2_afx_ddr_sz_chk_reserved1_ALIGN                         0
#define mc2_afx_ddr_sz_chk_reserved1_BITS                          4
#define mc2_afx_ddr_sz_chk_reserved1_SHIFT                         0


#define mc2_wbf_acc_acc_eack_MASK                                  0x80000000
#define mc2_wbf_acc_acc_eack_ALIGN                                 0
#define mc2_wbf_acc_acc_eack_BITS                                  1
#define mc2_wbf_acc_acc_eack_SHIFT                                 31
#define mc2_wbf_acc_acc_eack_DEFAULT                               0x00000000


#define mc2_wbf_acc_reserved0_MASK                                 0x7fffff00
#define mc2_wbf_acc_reserved0_ALIGN                                0
#define mc2_wbf_acc_reserved0_BITS                                 23
#define mc2_wbf_acc_reserved0_SHIFT                                8


#define mc2_wbf_acc_acc_sw_MASK                                    0x00000080
#define mc2_wbf_acc_acc_sw_ALIGN                                   0
#define mc2_wbf_acc_acc_sw_BITS                                    1
#define mc2_wbf_acc_acc_sw_SHIFT                                   7
#define mc2_wbf_acc_acc_sw_DEFAULT                                 0x00000001


#define mc2_wbf_acc_acc_sr_MASK                                    0x00000040
#define mc2_wbf_acc_acc_sr_ALIGN                                   0
#define mc2_wbf_acc_acc_sr_BITS                                    1
#define mc2_wbf_acc_acc_sr_SHIFT                                   6
#define mc2_wbf_acc_acc_sr_DEFAULT                                 0x00000001


#define mc2_wbf_acc_acc_nsw_MASK                                   0x00000020
#define mc2_wbf_acc_acc_nsw_ALIGN                                  0
#define mc2_wbf_acc_acc_nsw_BITS                                   1
#define mc2_wbf_acc_acc_nsw_SHIFT                                  5
#define mc2_wbf_acc_acc_nsw_DEFAULT                                0x00000001


#define mc2_wbf_acc_acc_nsr_MASK                                   0x00000010
#define mc2_wbf_acc_acc_nsr_ALIGN                                  0
#define mc2_wbf_acc_acc_nsr_BITS                                   1
#define mc2_wbf_acc_acc_nsr_SHIFT                                  4
#define mc2_wbf_acc_acc_nsr_DEFAULT                                0x00000001


#define mc2_wbf_acc_perm_sw_MASK                                   0x00000008
#define mc2_wbf_acc_perm_sw_ALIGN                                  0
#define mc2_wbf_acc_perm_sw_BITS                                   1
#define mc2_wbf_acc_perm_sw_SHIFT                                  3
#define mc2_wbf_acc_perm_sw_DEFAULT                                0x00000001


#define mc2_wbf_acc_perm_sr_MASK                                   0x00000004
#define mc2_wbf_acc_perm_sr_ALIGN                                  0
#define mc2_wbf_acc_perm_sr_BITS                                   1
#define mc2_wbf_acc_perm_sr_SHIFT                                  2
#define mc2_wbf_acc_perm_sr_DEFAULT                                0x00000001


#define mc2_wbf_acc_perm_nsw_MASK                                  0x00000002
#define mc2_wbf_acc_perm_nsw_ALIGN                                 0
#define mc2_wbf_acc_perm_nsw_BITS                                  1
#define mc2_wbf_acc_perm_nsw_SHIFT                                 1
#define mc2_wbf_acc_perm_nsw_DEFAULT                               0x00000001


#define mc2_wbf_acc_perm_nsr_MASK                                  0x00000001
#define mc2_wbf_acc_perm_nsr_ALIGN                                 0
#define mc2_wbf_acc_perm_nsr_BITS                                  1
#define mc2_wbf_acc_perm_nsr_SHIFT                                 0
#define mc2_wbf_acc_perm_nsr_DEFAULT                               0x00000001




#define mc2_wbf_ver_reserved0_MASK                                 0xffffff00
#define mc2_wbf_ver_reserved0_ALIGN                                0
#define mc2_wbf_ver_reserved0_BITS                                 24
#define mc2_wbf_ver_reserved0_SHIFT                                8


#define mc2_wbf_ver_version_MASK                                   0x000000ff
#define mc2_wbf_ver_version_ALIGN                                  0
#define mc2_wbf_ver_version_BITS                                   8
#define mc2_wbf_ver_version_SHIFT                                  0
#define mc2_wbf_ver_version_DEFAULT                                0x00000000




#define mc2_wbf_pri_cfg_wbf_en_MASK                                0x80000000
#define mc2_wbf_pri_cfg_wbf_en_ALIGN                               0
#define mc2_wbf_pri_cfg_wbf_en_BITS                                1
#define mc2_wbf_pri_cfg_wbf_en_SHIFT                               31
#define mc2_wbf_pri_cfg_wbf_en_DEFAULT                             0x00000000


#define mc2_wbf_pri_cfg_reserved0_MASK                             0x70000000
#define mc2_wbf_pri_cfg_reserved0_ALIGN                            0
#define mc2_wbf_pri_cfg_reserved0_BITS                             3
#define mc2_wbf_pri_cfg_reserved0_SHIFT                            28


#define mc2_wbf_pri_cfg_prefetch_en_MASK                           0x0f000000
#define mc2_wbf_pri_cfg_prefetch_en_ALIGN                          0
#define mc2_wbf_pri_cfg_prefetch_en_BITS                           4
#define mc2_wbf_pri_cfg_prefetch_en_SHIFT                          24
#define mc2_wbf_pri_cfg_prefetch_en_DEFAULT                        0x00000000


#define mc2_wbf_pri_cfg_reserved1_MASK                             0x00c00000
#define mc2_wbf_pri_cfg_reserved1_ALIGN                            0
#define mc2_wbf_pri_cfg_reserved1_BITS                             2
#define mc2_wbf_pri_cfg_reserved1_SHIFT                            22


#define mc2_wbf_pri_cfg_wbf_shared_fifo0_MASK                      0x00200000
#define mc2_wbf_pri_cfg_wbf_shared_fifo0_ALIGN                     0
#define mc2_wbf_pri_cfg_wbf_shared_fifo0_BITS                      1
#define mc2_wbf_pri_cfg_wbf_shared_fifo0_SHIFT                     21
#define mc2_wbf_pri_cfg_wbf_shared_fifo0_DEFAULT                   0x00000001



#define mc2_wbf_pri_cfg_wbf_pslc_buf_mode_MASK                     0x00100000
#define mc2_wbf_pri_cfg_wbf_pslc_buf_mode_ALIGN                    0
#define mc2_wbf_pri_cfg_wbf_pslc_buf_mode_BITS                     1
#define mc2_wbf_pri_cfg_wbf_pslc_buf_mode_SHIFT                    20
#define mc2_wbf_pri_cfg_wbf_pslc_buf_mode_DEFAULT                  0x00000000


#define mc2_wbf_pri_cfg_high_pri_chn_MASK                          0x000f0000
#define mc2_wbf_pri_cfg_high_pri_chn_ALIGN                         0
#define mc2_wbf_pri_cfg_high_pri_chn_BITS                          4
#define mc2_wbf_pri_cfg_high_pri_chn_SHIFT                         16
#define mc2_wbf_pri_cfg_high_pri_chn_DEFAULT                       0x00000003


#define mc2_wbf_pri_cfg_reserved2_MASK                             0x0000fff0
#define mc2_wbf_pri_cfg_reserved2_ALIGN                            0
#define mc2_wbf_pri_cfg_reserved2_BITS                             12
#define mc2_wbf_pri_cfg_reserved2_SHIFT                            4


#define mc2_wbf_pri_cfg_high_pri_ifm_MASK                          0x0000000f
#define mc2_wbf_pri_cfg_high_pri_ifm_ALIGN                         0
#define mc2_wbf_pri_cfg_high_pri_ifm_BITS                          4
#define mc2_wbf_pri_cfg_high_pri_ifm_SHIFT                         0
#define mc2_wbf_pri_cfg_high_pri_ifm_DEFAULT                       0x00000003




#define mc2_wbf_sta_bq_full_MASK                                   0x80000000
#define mc2_wbf_sta_bq_full_ALIGN                                  0
#define mc2_wbf_sta_bq_full_BITS                                   1
#define mc2_wbf_sta_bq_full_SHIFT                                  31


#define mc2_wbf_sta_bq_empty_MASK                                  0x40000000
#define mc2_wbf_sta_bq_empty_ALIGN                                 0
#define mc2_wbf_sta_bq_empty_BITS                                  1
#define mc2_wbf_sta_bq_empty_SHIFT                                 30


#define mc2_wbf_sta_reserved0_MASK                                 0x3ffc0000
#define mc2_wbf_sta_reserved0_ALIGN                                0
#define mc2_wbf_sta_reserved0_BITS                                 12
#define mc2_wbf_sta_reserved0_SHIFT                                18


#define mc2_wbf_sta_wbf_buf_level_MASK                             0x0003ff00
#define mc2_wbf_sta_wbf_buf_level_ALIGN                            0
#define mc2_wbf_sta_wbf_buf_level_BITS                             10
#define mc2_wbf_sta_wbf_buf_level_SHIFT                            8


#define mc2_wbf_sta_bq_count_MASK                                  0x000000ff
#define mc2_wbf_sta_bq_count_ALIGN                                 0
#define mc2_wbf_sta_bq_count_BITS                                  8
#define mc2_wbf_sta_bq_count_SHIFT                                 0




#define mc2_wbf_bkdr_bkdr_cmd_bkdr_go_MASK                         0x80000000
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_go_ALIGN                        0
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_go_BITS                         1
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_go_SHIFT                        31
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_go_DEFAULT                      0x00000000


#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wr_MASK                         0x40000000
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wr_ALIGN                        0
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wr_BITS                         1
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wr_SHIFT                        30
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wr_DEFAULT                      0x00000000


#define mc2_wbf_bkdr_bkdr_cmd_reserved0_MASK                       0x3ff80000
#define mc2_wbf_bkdr_bkdr_cmd_reserved0_ALIGN                      0
#define mc2_wbf_bkdr_bkdr_cmd_reserved0_BITS                       11
#define mc2_wbf_bkdr_bkdr_cmd_reserved0_SHIFT                      19


#define mc2_wbf_bkdr_bkdr_cmd_bkdr_slice_MASK                      0x00070000
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_slice_ALIGN                     0
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_slice_BITS                      3
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_slice_SHIFT                     16


#define mc2_wbf_bkdr_bkdr_cmd_pslc_wbf_buf_sel_MASK                0x0000ff00
#define mc2_wbf_bkdr_bkdr_cmd_pslc_wbf_buf_sel_ALIGN               0
#define mc2_wbf_bkdr_bkdr_cmd_pslc_wbf_buf_sel_BITS                8
#define mc2_wbf_bkdr_bkdr_cmd_pslc_wbf_buf_sel_SHIFT               8
#define mc2_wbf_bkdr_bkdr_cmd_pslc_wbf_buf_sel_DEFAULT             0x00000000


#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wbf_id_MASK                     0x000000ff
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wbf_id_ALIGN                    0
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wbf_id_BITS                     8
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wbf_id_SHIFT                    0
#define mc2_wbf_bkdr_bkdr_cmd_bkdr_wbf_id_DEFAULT                  0x00000000



#define mc2_wbf_bkdr_bkdr_datai_ARRAY_BASE                         0x00000e94
#define mc2_wbf_bkdr_bkdr_datai_ARRAY_START                        0
#define mc2_wbf_bkdr_bkdr_datai_ARRAY_END                          7
#define mc2_wbf_bkdr_bkdr_datai_ARRAY_ELEMENT_SIZE                 32




#define mc2_wbf_bkdr_bkdr_datai_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_datai_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_datai_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_datai_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_datai_data_DEFAULT                       0x00000000





#define mc2_wbf_bkdr_bkdr_data0_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data0_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data0_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data0_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data0_data_DEFAULT                       0x00000000




#define mc2_wbf_bkdr_bkdr_data1_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data1_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data1_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data1_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data1_data_DEFAULT                       0x00000000




#define mc2_wbf_bkdr_bkdr_data2_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data2_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data2_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data2_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data2_data_DEFAULT                       0x00000000




#define mc2_wbf_bkdr_bkdr_data3_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data3_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data3_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data3_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data3_data_DEFAULT                       0x00000000




#define mc2_wbf_bkdr_bkdr_data4_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data4_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data4_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data4_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data4_data_DEFAULT                       0x00000000




#define mc2_wbf_bkdr_bkdr_data5_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data5_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data5_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data5_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data5_data_DEFAULT                       0x00000000




#define mc2_wbf_bkdr_bkdr_data6_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data6_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data6_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data6_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data6_data_DEFAULT                       0x00000000




#define mc2_wbf_bkdr_bkdr_data7_data_MASK                          0xffffffff
#define mc2_wbf_bkdr_bkdr_data7_data_ALIGN                         0
#define mc2_wbf_bkdr_bkdr_data7_data_BITS                          32
#define mc2_wbf_bkdr_bkdr_data7_data_SHIFT                         0
#define mc2_wbf_bkdr_bkdr_data7_data_DEFAULT                       0x00000000




#define mc2_wbf_id_bkdr_id_cmd_id_freeze_MASK                      0x80000000
#define mc2_wbf_id_bkdr_id_cmd_id_freeze_ALIGN                     0
#define mc2_wbf_id_bkdr_id_cmd_id_freeze_BITS                      1
#define mc2_wbf_id_bkdr_id_cmd_id_freeze_SHIFT                     31
#define mc2_wbf_id_bkdr_id_cmd_id_freeze_DEFAULT                   0x00000000


#define mc2_wbf_id_bkdr_id_cmd_reserved0_MASK                      0x70000000
#define mc2_wbf_id_bkdr_id_cmd_reserved0_ALIGN                     0
#define mc2_wbf_id_bkdr_id_cmd_reserved0_BITS                      3
#define mc2_wbf_id_bkdr_id_cmd_reserved0_SHIFT                     28


#define mc2_wbf_id_bkdr_id_cmd_id_go_MASK                          0x0f000000
#define mc2_wbf_id_bkdr_id_cmd_id_go_ALIGN                         0
#define mc2_wbf_id_bkdr_id_cmd_id_go_BITS                          4
#define mc2_wbf_id_bkdr_id_cmd_id_go_SHIFT                         24
#define mc2_wbf_id_bkdr_id_cmd_id_go_DEFAULT                       0x00000000


#define mc2_wbf_id_bkdr_id_cmd_reserved1_MASK                      0x00fe0000
#define mc2_wbf_id_bkdr_id_cmd_reserved1_ALIGN                     0
#define mc2_wbf_id_bkdr_id_cmd_reserved1_BITS                      7
#define mc2_wbf_id_bkdr_id_cmd_reserved1_SHIFT                     17


#define mc2_wbf_id_bkdr_id_cmd_id_wr_MASK                          0x00010000
#define mc2_wbf_id_bkdr_id_cmd_id_wr_ALIGN                         0
#define mc2_wbf_id_bkdr_id_cmd_id_wr_BITS                          1
#define mc2_wbf_id_bkdr_id_cmd_id_wr_SHIFT                         16
#define mc2_wbf_id_bkdr_id_cmd_id_wr_DEFAULT                       0x00000000


#define mc2_wbf_id_bkdr_id_cmd_reserved2_MASK                      0x0000ff00
#define mc2_wbf_id_bkdr_id_cmd_reserved2_ALIGN                     0
#define mc2_wbf_id_bkdr_id_cmd_reserved2_BITS                      8
#define mc2_wbf_id_bkdr_id_cmd_reserved2_SHIFT                     8


#define mc2_wbf_id_bkdr_id_cmd_id_addr_MASK                        0x000000ff
#define mc2_wbf_id_bkdr_id_cmd_id_addr_ALIGN                       0
#define mc2_wbf_id_bkdr_id_cmd_id_addr_BITS                        8
#define mc2_wbf_id_bkdr_id_cmd_id_addr_SHIFT                       0
#define mc2_wbf_id_bkdr_id_cmd_id_addr_DEFAULT                     0x00000000




#define mc2_wbf_id_bkdr_id_data_wbf_id_wr_ptr_MASK                 0xff000000
#define mc2_wbf_id_bkdr_id_data_wbf_id_wr_ptr_ALIGN                0
#define mc2_wbf_id_bkdr_id_data_wbf_id_wr_ptr_BITS                 8
#define mc2_wbf_id_bkdr_id_data_wbf_id_wr_ptr_SHIFT                24
#define mc2_wbf_id_bkdr_id_data_wbf_id_wr_ptr_DEFAULT              0x00000000


#define mc2_wbf_id_bkdr_id_data_wbf_id_rd_ptr_MASK                 0x00ff0000
#define mc2_wbf_id_bkdr_id_data_wbf_id_rd_ptr_ALIGN                0
#define mc2_wbf_id_bkdr_id_data_wbf_id_rd_ptr_BITS                 8
#define mc2_wbf_id_bkdr_id_data_wbf_id_rd_ptr_SHIFT                16
#define mc2_wbf_id_bkdr_id_data_wbf_id_rd_ptr_DEFAULT              0x00000000


#define mc2_wbf_id_bkdr_id_data_bq_count_wr_MASK                   0x0000ff00
#define mc2_wbf_id_bkdr_id_data_bq_count_wr_ALIGN                  0
#define mc2_wbf_id_bkdr_id_data_bq_count_wr_BITS                   8
#define mc2_wbf_id_bkdr_id_data_bq_count_wr_SHIFT                  8
#define mc2_wbf_id_bkdr_id_data_bq_count_wr_DEFAULT                0x00000000


#define mc2_wbf_id_bkdr_id_data_dbg_MASK                           0x000000ff
#define mc2_wbf_id_bkdr_id_data_dbg_ALIGN                          0
#define mc2_wbf_id_bkdr_id_data_dbg_BITS                           8
#define mc2_wbf_id_bkdr_id_data_dbg_SHIFT                          0
#define mc2_wbf_id_bkdr_id_data_dbg_DEFAULT                        0x00000000




#define mc2_wbf_id_bkdr_id_data2_reserved0_MASK                    0xfffffe00
#define mc2_wbf_id_bkdr_id_data2_reserved0_ALIGN                   0
#define mc2_wbf_id_bkdr_id_data2_reserved0_BITS                    23
#define mc2_wbf_id_bkdr_id_data2_reserved0_SHIFT                   9


#define mc2_wbf_id_bkdr_id_data2_wbf_id_MASK                       0x000001ff
#define mc2_wbf_id_bkdr_id_data2_wbf_id_ALIGN                      0
#define mc2_wbf_id_bkdr_id_data2_wbf_id_BITS                       9
#define mc2_wbf_id_bkdr_id_data2_wbf_id_SHIFT                      0
#define mc2_wbf_id_bkdr_id_data2_wbf_id_DEFAULT                    0x00000000

#endif
