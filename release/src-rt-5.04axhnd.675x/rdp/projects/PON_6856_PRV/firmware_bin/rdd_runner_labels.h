/* IMAGE 0 LABELS */
#ifndef IMAGE_0_CODE_ADDRESSES
#define IMAGE_0_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_0_budget_allocator_1st_wakeup_request        (0x7fc)
#define image_0_debug_routine        (0x9c)
#define image_0_debug_routine_handler        (0x4)
#define image_0_ds_tx_task_wakeup_request        (0x46c)
#define image_0_flush_task_1st_wakeup_request        (0xcdc)
#define image_0_ghost_reporting_1st_wakeup_request        (0xde4)
#define image_0_initialization_task        (0x48)
#define image_0_start_task_budget_allocator_1st_wakeup_request        (0x7fc)
#define image_0_start_task_debug_routine        (0x9c)
#define image_0_start_task_ds_tx_task_wakeup_request        (0x46c)
#define image_0_start_task_flush_task_1st_wakeup_request        (0xcdc)
#define image_0_start_task_ghost_reporting_1st_wakeup_request        (0xde4)
#define image_0_start_task_initialization_task        (0x48)
#define image_0_start_task_update_fifo_ds_read_1st_wakeup_request        (0x1244)
#define image_0_update_fifo_ds_read_1st_wakeup_request        (0x1244)

#else

#define image_0_budget_allocator_1st_wakeup_request        (0x1ff)
#define image_0_debug_routine        (0x27)
#define image_0_debug_routine_handler        (0x1)
#define image_0_ds_tx_task_wakeup_request        (0x11b)
#define image_0_flush_task_1st_wakeup_request        (0x337)
#define image_0_ghost_reporting_1st_wakeup_request        (0x379)
#define image_0_initialization_task        (0x12)
#define image_0_start_task_budget_allocator_1st_wakeup_request        (0x1ff)
#define image_0_start_task_debug_routine        (0x27)
#define image_0_start_task_ds_tx_task_wakeup_request        (0x11b)
#define image_0_start_task_flush_task_1st_wakeup_request        (0x337)
#define image_0_start_task_ghost_reporting_1st_wakeup_request        (0x379)
#define image_0_start_task_initialization_task        (0x12)
#define image_0_start_task_update_fifo_ds_read_1st_wakeup_request        (0x491)
#define image_0_update_fifo_ds_read_1st_wakeup_request        (0x491)

#endif


#endif

/* IMAGE 1 LABELS */
#ifndef IMAGE_1_CODE_ADDRESSES
#define IMAGE_1_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_1_cpu_recycle_wakeup_request        (0x4f5c)
#define image_1_cpu_rx_copy_wakeup_request        (0x3dec)
#define image_1_cpu_rx_meter_budget_allocator_1st_wakeup_request        (0x4c8c)
#define image_1_cpu_rx_wakeup_request        (0x4968)
#define image_1_debug_routine        (0x3ce0)
#define image_1_debug_routine_handler        (0x4)
#define image_1_gpe_cmd_copy_bits_16        (0x33fc)
#define image_1_gpe_cmd_replace_16        (0x33bc)
#define image_1_gpe_cmd_replace_32        (0x33c4)
#define image_1_gpe_cmd_replace_bits_16        (0x33dc)
#define image_1_gpe_cmd_skip_if        (0x342c)
#define image_1_gpe_vlan_action_cmd_drop        (0x344c)
#define image_1_gpe_vlan_action_cmd_dscp        (0x3450)
#define image_1_gpe_vlan_action_cmd_mac_hdr_copy        (0x34a4)
#define image_1_initialization_task        (0x8)
#define image_1_interrupt_coalescing_1st_wakeup_request        (0x50b0)
#define image_1_processing_wakeup_request        (0xbb4)
#define image_1_start_task_cpu_recycle_wakeup_request        (0x4f5c)
#define image_1_start_task_cpu_rx_copy_wakeup_request        (0x3dec)
#define image_1_start_task_cpu_rx_meter_budget_allocator_1st_wakeup_request        (0x4c8c)
#define image_1_start_task_cpu_rx_wakeup_request        (0x4968)
#define image_1_start_task_debug_routine        (0x3ce0)
#define image_1_start_task_initialization_task        (0x8)
#define image_1_start_task_interrupt_coalescing_1st_wakeup_request        (0x50b0)
#define image_1_start_task_processing_wakeup_request        (0xbb4)
#define image_1_tcam_cmd_dst_ip        (0x2c44)
#define image_1_tcam_cmd_dst_ipv6_masked        (0x2c98)
#define image_1_tcam_cmd_dst_mac        (0x2d28)
#define image_1_tcam_cmd_dst_port        (0x2ce4)
#define image_1_tcam_cmd_ethertype        (0x2a68)
#define image_1_tcam_cmd_gem_flow        (0x2b78)
#define image_1_tcam_cmd_generic_l2        (0x2d54)
#define image_1_tcam_cmd_generic_l3        (0x2d88)
#define image_1_tcam_cmd_generic_l4        (0x2dbc)
#define image_1_tcam_cmd_ic_submit        (0x29f4)
#define image_1_tcam_cmd_ingress_port        (0x2b60)
#define image_1_tcam_cmd_inner_pbit        (0x2ac8)
#define image_1_tcam_cmd_inner_tpid        (0x2a50)
#define image_1_tcam_cmd_inner_vid        (0x2a98)
#define image_1_tcam_cmd_ip_protocol        (0x2af8)
#define image_1_tcam_cmd_ipv6_label        (0x2b90)
#define image_1_tcam_cmd_l3_protocol        (0x2b10)
#define image_1_tcam_cmd_network_layer        (0x2b40)
#define image_1_tcam_cmd_outer_pbit        (0x2ab0)
#define image_1_tcam_cmd_outer_tpid        (0x2a38)
#define image_1_tcam_cmd_outer_vid        (0x2a80)
#define image_1_tcam_cmd_src_ip        (0x2bbc)
#define image_1_tcam_cmd_src_ipv6_masked        (0x2c10)
#define image_1_tcam_cmd_src_mac        (0x2cfc)
#define image_1_tcam_cmd_src_port        (0x2ccc)
#define image_1_tcam_cmd_tos        (0x2b28)
#define image_1_tcam_cmd_vlan_num        (0x2ae0)

#else

#define image_1_cpu_recycle_wakeup_request        (0x13d7)
#define image_1_cpu_rx_copy_wakeup_request        (0xf7b)
#define image_1_cpu_rx_meter_budget_allocator_1st_wakeup_request        (0x1323)
#define image_1_cpu_rx_wakeup_request        (0x125a)
#define image_1_debug_routine        (0xf38)
#define image_1_debug_routine_handler        (0x1)
#define image_1_gpe_cmd_copy_bits_16        (0xcff)
#define image_1_gpe_cmd_replace_16        (0xcef)
#define image_1_gpe_cmd_replace_32        (0xcf1)
#define image_1_gpe_cmd_replace_bits_16        (0xcf7)
#define image_1_gpe_cmd_skip_if        (0xd0b)
#define image_1_gpe_vlan_action_cmd_drop        (0xd13)
#define image_1_gpe_vlan_action_cmd_dscp        (0xd14)
#define image_1_gpe_vlan_action_cmd_mac_hdr_copy        (0xd29)
#define image_1_initialization_task        (0x2)
#define image_1_interrupt_coalescing_1st_wakeup_request        (0x142c)
#define image_1_processing_wakeup_request        (0x2ed)
#define image_1_start_task_cpu_recycle_wakeup_request        (0x13d7)
#define image_1_start_task_cpu_rx_copy_wakeup_request        (0xf7b)
#define image_1_start_task_cpu_rx_meter_budget_allocator_1st_wakeup_request        (0x1323)
#define image_1_start_task_cpu_rx_wakeup_request        (0x125a)
#define image_1_start_task_debug_routine        (0xf38)
#define image_1_start_task_initialization_task        (0x2)
#define image_1_start_task_interrupt_coalescing_1st_wakeup_request        (0x142c)
#define image_1_start_task_processing_wakeup_request        (0x2ed)
#define image_1_tcam_cmd_dst_ip        (0xb11)
#define image_1_tcam_cmd_dst_ipv6_masked        (0xb26)
#define image_1_tcam_cmd_dst_mac        (0xb4a)
#define image_1_tcam_cmd_dst_port        (0xb39)
#define image_1_tcam_cmd_ethertype        (0xa9a)
#define image_1_tcam_cmd_gem_flow        (0xade)
#define image_1_tcam_cmd_generic_l2        (0xb55)
#define image_1_tcam_cmd_generic_l3        (0xb62)
#define image_1_tcam_cmd_generic_l4        (0xb6f)
#define image_1_tcam_cmd_ic_submit        (0xa7d)
#define image_1_tcam_cmd_ingress_port        (0xad8)
#define image_1_tcam_cmd_inner_pbit        (0xab2)
#define image_1_tcam_cmd_inner_tpid        (0xa94)
#define image_1_tcam_cmd_inner_vid        (0xaa6)
#define image_1_tcam_cmd_ip_protocol        (0xabe)
#define image_1_tcam_cmd_ipv6_label        (0xae4)
#define image_1_tcam_cmd_l3_protocol        (0xac4)
#define image_1_tcam_cmd_network_layer        (0xad0)
#define image_1_tcam_cmd_outer_pbit        (0xaac)
#define image_1_tcam_cmd_outer_tpid        (0xa8e)
#define image_1_tcam_cmd_outer_vid        (0xaa0)
#define image_1_tcam_cmd_src_ip        (0xaef)
#define image_1_tcam_cmd_src_ipv6_masked        (0xb04)
#define image_1_tcam_cmd_src_mac        (0xb3f)
#define image_1_tcam_cmd_src_port        (0xb33)
#define image_1_tcam_cmd_tos        (0xaca)
#define image_1_tcam_cmd_vlan_num        (0xab8)

#endif


#endif

/* IMAGE 2 LABELS */
#ifndef IMAGE_2_CODE_ADDRESSES
#define IMAGE_2_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_2_cpu_recycle_wakeup_request        (0x4508)
#define image_2_cpu_tx_read_ring_indices        (0x3dec)
#define image_2_cpu_tx_wakeup_request        (0x3dec)
#define image_2_debug_routine        (0x48)
#define image_2_debug_routine_handler        (0x4)
#define image_2_gpe_cmd_copy_bits_16        (0x3508)
#define image_2_gpe_cmd_replace_16        (0x34c8)
#define image_2_gpe_cmd_replace_32        (0x34d0)
#define image_2_gpe_cmd_replace_bits_16        (0x34e8)
#define image_2_gpe_cmd_skip_if        (0x3538)
#define image_2_gpe_vlan_action_cmd_drop        (0x3558)
#define image_2_gpe_vlan_action_cmd_dscp        (0x355c)
#define image_2_gpe_vlan_action_cmd_mac_hdr_copy        (0x35b0)
#define image_2_initialization_task        (0x8)
#define image_2_interrupt_coalescing_1st_wakeup_request        (0x49cc)
#define image_2_processing_wakeup_request        (0xcc0)
#define image_2_start_task_cpu_recycle_wakeup_request        (0x4508)
#define image_2_start_task_cpu_tx_wakeup_request        (0x3dec)
#define image_2_start_task_debug_routine        (0x48)
#define image_2_start_task_initialization_task        (0x8)
#define image_2_start_task_interrupt_coalescing_1st_wakeup_request        (0x49cc)
#define image_2_start_task_processing_wakeup_request        (0xcc0)
#define image_2_start_task_timer_common_task_wakeup_request        (0x465c)
#define image_2_tcam_cmd_dst_ip        (0x2d50)
#define image_2_tcam_cmd_dst_ipv6_masked        (0x2da4)
#define image_2_tcam_cmd_dst_mac        (0x2e34)
#define image_2_tcam_cmd_dst_port        (0x2df0)
#define image_2_tcam_cmd_ethertype        (0x2b74)
#define image_2_tcam_cmd_gem_flow        (0x2c84)
#define image_2_tcam_cmd_generic_l2        (0x2e60)
#define image_2_tcam_cmd_generic_l3        (0x2e94)
#define image_2_tcam_cmd_generic_l4        (0x2ec8)
#define image_2_tcam_cmd_ic_submit        (0x2b00)
#define image_2_tcam_cmd_ingress_port        (0x2c6c)
#define image_2_tcam_cmd_inner_pbit        (0x2bd4)
#define image_2_tcam_cmd_inner_tpid        (0x2b5c)
#define image_2_tcam_cmd_inner_vid        (0x2ba4)
#define image_2_tcam_cmd_ip_protocol        (0x2c04)
#define image_2_tcam_cmd_ipv6_label        (0x2c9c)
#define image_2_tcam_cmd_l3_protocol        (0x2c1c)
#define image_2_tcam_cmd_network_layer        (0x2c4c)
#define image_2_tcam_cmd_outer_pbit        (0x2bbc)
#define image_2_tcam_cmd_outer_tpid        (0x2b44)
#define image_2_tcam_cmd_outer_vid        (0x2b8c)
#define image_2_tcam_cmd_src_ip        (0x2cc8)
#define image_2_tcam_cmd_src_ipv6_masked        (0x2d1c)
#define image_2_tcam_cmd_src_mac        (0x2e08)
#define image_2_tcam_cmd_src_port        (0x2dd8)
#define image_2_tcam_cmd_tos        (0x2c34)
#define image_2_tcam_cmd_vlan_num        (0x2bec)
#define image_2_timer_common_task_wakeup_request        (0x465c)

#else

#define image_2_cpu_recycle_wakeup_request        (0x1142)
#define image_2_cpu_tx_read_ring_indices        (0xf7b)
#define image_2_cpu_tx_wakeup_request        (0xf7b)
#define image_2_debug_routine        (0x12)
#define image_2_debug_routine_handler        (0x1)
#define image_2_gpe_cmd_copy_bits_16        (0xd42)
#define image_2_gpe_cmd_replace_16        (0xd32)
#define image_2_gpe_cmd_replace_32        (0xd34)
#define image_2_gpe_cmd_replace_bits_16        (0xd3a)
#define image_2_gpe_cmd_skip_if        (0xd4e)
#define image_2_gpe_vlan_action_cmd_drop        (0xd56)
#define image_2_gpe_vlan_action_cmd_dscp        (0xd57)
#define image_2_gpe_vlan_action_cmd_mac_hdr_copy        (0xd6c)
#define image_2_initialization_task        (0x2)
#define image_2_interrupt_coalescing_1st_wakeup_request        (0x1273)
#define image_2_processing_wakeup_request        (0x330)
#define image_2_start_task_cpu_recycle_wakeup_request        (0x1142)
#define image_2_start_task_cpu_tx_wakeup_request        (0xf7b)
#define image_2_start_task_debug_routine        (0x12)
#define image_2_start_task_initialization_task        (0x2)
#define image_2_start_task_interrupt_coalescing_1st_wakeup_request        (0x1273)
#define image_2_start_task_processing_wakeup_request        (0x330)
#define image_2_start_task_timer_common_task_wakeup_request        (0x1197)
#define image_2_tcam_cmd_dst_ip        (0xb54)
#define image_2_tcam_cmd_dst_ipv6_masked        (0xb69)
#define image_2_tcam_cmd_dst_mac        (0xb8d)
#define image_2_tcam_cmd_dst_port        (0xb7c)
#define image_2_tcam_cmd_ethertype        (0xadd)
#define image_2_tcam_cmd_gem_flow        (0xb21)
#define image_2_tcam_cmd_generic_l2        (0xb98)
#define image_2_tcam_cmd_generic_l3        (0xba5)
#define image_2_tcam_cmd_generic_l4        (0xbb2)
#define image_2_tcam_cmd_ic_submit        (0xac0)
#define image_2_tcam_cmd_ingress_port        (0xb1b)
#define image_2_tcam_cmd_inner_pbit        (0xaf5)
#define image_2_tcam_cmd_inner_tpid        (0xad7)
#define image_2_tcam_cmd_inner_vid        (0xae9)
#define image_2_tcam_cmd_ip_protocol        (0xb01)
#define image_2_tcam_cmd_ipv6_label        (0xb27)
#define image_2_tcam_cmd_l3_protocol        (0xb07)
#define image_2_tcam_cmd_network_layer        (0xb13)
#define image_2_tcam_cmd_outer_pbit        (0xaef)
#define image_2_tcam_cmd_outer_tpid        (0xad1)
#define image_2_tcam_cmd_outer_vid        (0xae3)
#define image_2_tcam_cmd_src_ip        (0xb32)
#define image_2_tcam_cmd_src_ipv6_masked        (0xb47)
#define image_2_tcam_cmd_src_mac        (0xb82)
#define image_2_tcam_cmd_src_port        (0xb76)
#define image_2_tcam_cmd_tos        (0xb0d)
#define image_2_tcam_cmd_vlan_num        (0xafb)
#define image_2_timer_common_task_wakeup_request        (0x1197)

#endif


#endif

/* IMAGE 3 LABELS */
#ifndef IMAGE_3_CODE_ADDRESSES
#define IMAGE_3_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_3_budget_allocator_1st_wakeup_request        (0x1774)
#define image_3_debug_routine        (0x48)
#define image_3_debug_routine_handler        (0x4)
#define image_3_epon_tx_task_wakeup_request        (0x15d4)
#define image_3_epon_update_fifo_read_1st_wakeup_request        (0x214c)
#define image_3_flush_task_1st_wakeup_request        (0x1d00)
#define image_3_gpon_control_wakeup_request        (0x5b4)
#define image_3_initialization_task        (0x8)
#define image_3_ovl_budget_allocator_1st_wakeup_request        (0x1b50)
#define image_3_start_task_budget_allocator_1st_wakeup_request        (0x1774)
#define image_3_start_task_debug_routine        (0x48)
#define image_3_start_task_epon_tx_task_wakeup_request        (0x15d4)
#define image_3_start_task_epon_update_fifo_read_1st_wakeup_request        (0x214c)
#define image_3_start_task_flush_task_1st_wakeup_request        (0x1d00)
#define image_3_start_task_gpon_control_wakeup_request        (0x5b4)
#define image_3_start_task_initialization_task        (0x8)
#define image_3_start_task_ovl_budget_allocator_1st_wakeup_request        (0x1b50)
#define image_3_start_task_update_fifo_us_read_1st_wakeup_request        (0x1e7c)
#define image_3_start_task_us_tx_task_1st_wakeup_request        (0xb80)
#define image_3_update_fifo_us_read_1st_wakeup_request        (0x1e7c)
#define image_3_us_tx_task_1st_wakeup_request        (0xb80)
#define image_3_us_tx_task_wakeup_request        (0xb80)

#else

#define image_3_budget_allocator_1st_wakeup_request        (0x5dd)
#define image_3_debug_routine        (0x12)
#define image_3_debug_routine_handler        (0x1)
#define image_3_epon_tx_task_wakeup_request        (0x575)
#define image_3_epon_update_fifo_read_1st_wakeup_request        (0x853)
#define image_3_flush_task_1st_wakeup_request        (0x740)
#define image_3_gpon_control_wakeup_request        (0x16d)
#define image_3_initialization_task        (0x2)
#define image_3_ovl_budget_allocator_1st_wakeup_request        (0x6d4)
#define image_3_start_task_budget_allocator_1st_wakeup_request        (0x5dd)
#define image_3_start_task_debug_routine        (0x12)
#define image_3_start_task_epon_tx_task_wakeup_request        (0x575)
#define image_3_start_task_epon_update_fifo_read_1st_wakeup_request        (0x853)
#define image_3_start_task_flush_task_1st_wakeup_request        (0x740)
#define image_3_start_task_gpon_control_wakeup_request        (0x16d)
#define image_3_start_task_initialization_task        (0x2)
#define image_3_start_task_ovl_budget_allocator_1st_wakeup_request        (0x6d4)
#define image_3_start_task_update_fifo_us_read_1st_wakeup_request        (0x79f)
#define image_3_start_task_us_tx_task_1st_wakeup_request        (0x2e0)
#define image_3_update_fifo_us_read_1st_wakeup_request        (0x79f)
#define image_3_us_tx_task_1st_wakeup_request        (0x2e0)
#define image_3_us_tx_task_wakeup_request        (0x2e0)

#endif


#endif

/* IMAGE 4 LABELS */
#ifndef IMAGE_4_CODE_ADDRESSES
#define IMAGE_4_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_4_debug_routine        (0x48)
#define image_4_debug_routine_handler        (0x4)
#define image_4_gpe_cmd_copy_bits_16        (0x3508)
#define image_4_gpe_cmd_replace_16        (0x34c8)
#define image_4_gpe_cmd_replace_32        (0x34d0)
#define image_4_gpe_cmd_replace_bits_16        (0x34e8)
#define image_4_gpe_cmd_skip_if        (0x3538)
#define image_4_gpe_vlan_action_cmd_drop        (0x3558)
#define image_4_gpe_vlan_action_cmd_dscp        (0x355c)
#define image_4_gpe_vlan_action_cmd_mac_hdr_copy        (0x35b0)
#define image_4_initialization_task        (0x8)
#define image_4_processing_wakeup_request        (0xcc0)
#define image_4_start_task_debug_routine        (0x48)
#define image_4_start_task_initialization_task        (0x8)
#define image_4_start_task_processing_wakeup_request        (0xcc0)
#define image_4_tcam_cmd_dst_ip        (0x2d50)
#define image_4_tcam_cmd_dst_ipv6_masked        (0x2da4)
#define image_4_tcam_cmd_dst_mac        (0x2e34)
#define image_4_tcam_cmd_dst_port        (0x2df0)
#define image_4_tcam_cmd_ethertype        (0x2b74)
#define image_4_tcam_cmd_gem_flow        (0x2c84)
#define image_4_tcam_cmd_generic_l2        (0x2e60)
#define image_4_tcam_cmd_generic_l3        (0x2e94)
#define image_4_tcam_cmd_generic_l4        (0x2ec8)
#define image_4_tcam_cmd_ic_submit        (0x2b00)
#define image_4_tcam_cmd_ingress_port        (0x2c6c)
#define image_4_tcam_cmd_inner_pbit        (0x2bd4)
#define image_4_tcam_cmd_inner_tpid        (0x2b5c)
#define image_4_tcam_cmd_inner_vid        (0x2ba4)
#define image_4_tcam_cmd_ip_protocol        (0x2c04)
#define image_4_tcam_cmd_ipv6_label        (0x2c9c)
#define image_4_tcam_cmd_l3_protocol        (0x2c1c)
#define image_4_tcam_cmd_network_layer        (0x2c4c)
#define image_4_tcam_cmd_outer_pbit        (0x2bbc)
#define image_4_tcam_cmd_outer_tpid        (0x2b44)
#define image_4_tcam_cmd_outer_vid        (0x2b8c)
#define image_4_tcam_cmd_src_ip        (0x2cc8)
#define image_4_tcam_cmd_src_ipv6_masked        (0x2d1c)
#define image_4_tcam_cmd_src_mac        (0x2e08)
#define image_4_tcam_cmd_src_port        (0x2dd8)
#define image_4_tcam_cmd_tos        (0x2c34)
#define image_4_tcam_cmd_vlan_num        (0x2bec)

#else

#define image_4_debug_routine        (0x12)
#define image_4_debug_routine_handler        (0x1)
#define image_4_gpe_cmd_copy_bits_16        (0xd42)
#define image_4_gpe_cmd_replace_16        (0xd32)
#define image_4_gpe_cmd_replace_32        (0xd34)
#define image_4_gpe_cmd_replace_bits_16        (0xd3a)
#define image_4_gpe_cmd_skip_if        (0xd4e)
#define image_4_gpe_vlan_action_cmd_drop        (0xd56)
#define image_4_gpe_vlan_action_cmd_dscp        (0xd57)
#define image_4_gpe_vlan_action_cmd_mac_hdr_copy        (0xd6c)
#define image_4_initialization_task        (0x2)
#define image_4_processing_wakeup_request        (0x330)
#define image_4_start_task_debug_routine        (0x12)
#define image_4_start_task_initialization_task        (0x2)
#define image_4_start_task_processing_wakeup_request        (0x330)
#define image_4_tcam_cmd_dst_ip        (0xb54)
#define image_4_tcam_cmd_dst_ipv6_masked        (0xb69)
#define image_4_tcam_cmd_dst_mac        (0xb8d)
#define image_4_tcam_cmd_dst_port        (0xb7c)
#define image_4_tcam_cmd_ethertype        (0xadd)
#define image_4_tcam_cmd_gem_flow        (0xb21)
#define image_4_tcam_cmd_generic_l2        (0xb98)
#define image_4_tcam_cmd_generic_l3        (0xba5)
#define image_4_tcam_cmd_generic_l4        (0xbb2)
#define image_4_tcam_cmd_ic_submit        (0xac0)
#define image_4_tcam_cmd_ingress_port        (0xb1b)
#define image_4_tcam_cmd_inner_pbit        (0xaf5)
#define image_4_tcam_cmd_inner_tpid        (0xad7)
#define image_4_tcam_cmd_inner_vid        (0xae9)
#define image_4_tcam_cmd_ip_protocol        (0xb01)
#define image_4_tcam_cmd_ipv6_label        (0xb27)
#define image_4_tcam_cmd_l3_protocol        (0xb07)
#define image_4_tcam_cmd_network_layer        (0xb13)
#define image_4_tcam_cmd_outer_pbit        (0xaef)
#define image_4_tcam_cmd_outer_tpid        (0xad1)
#define image_4_tcam_cmd_outer_vid        (0xae3)
#define image_4_tcam_cmd_src_ip        (0xb32)
#define image_4_tcam_cmd_src_ipv6_masked        (0xb47)
#define image_4_tcam_cmd_src_mac        (0xb82)
#define image_4_tcam_cmd_src_port        (0xb76)
#define image_4_tcam_cmd_tos        (0xb0d)
#define image_4_tcam_cmd_vlan_num        (0xafb)

#endif


#endif

/* IMAGE 5 LABELS */
#ifndef IMAGE_5_CODE_ADDRESSES
#define IMAGE_5_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_5_budget_allocator_1st_wakeup_request        (0x48bc)
#define image_5_debug_routine        (0x3f48)
#define image_5_debug_routine_handler        (0x4)
#define image_5_gpe_cmd_copy_bits_16        (0x3664)
#define image_5_gpe_cmd_replace_16        (0x3624)
#define image_5_gpe_cmd_replace_32        (0x362c)
#define image_5_gpe_cmd_replace_bits_16        (0x3644)
#define image_5_gpe_cmd_skip_if        (0x3694)
#define image_5_gpe_vlan_action_cmd_drop        (0x36b4)
#define image_5_gpe_vlan_action_cmd_dscp        (0x36b8)
#define image_5_gpe_vlan_action_cmd_mac_hdr_copy        (0x370c)
#define image_5_initialization_task        (0x8)
#define image_5_processing_wakeup_request        (0xe1c)
#define image_5_service_queues_tx_task_wakeup_request        (0x450c)
#define image_5_service_queues_update_fifo_read_1st_wakeup_request        (0x4264)
#define image_5_start_task_budget_allocator_1st_wakeup_request        (0x48bc)
#define image_5_start_task_debug_routine        (0x3f48)
#define image_5_start_task_initialization_task        (0x8)
#define image_5_start_task_processing_wakeup_request        (0xe1c)
#define image_5_start_task_service_queues_tx_task_wakeup_request        (0x450c)
#define image_5_start_task_service_queues_update_fifo_read_1st_wakeup_request        (0x4264)
#define image_5_tcam_cmd_dst_ip        (0x2eac)
#define image_5_tcam_cmd_dst_ipv6_masked        (0x2f00)
#define image_5_tcam_cmd_dst_mac        (0x2f90)
#define image_5_tcam_cmd_dst_port        (0x2f4c)
#define image_5_tcam_cmd_ethertype        (0x2cd0)
#define image_5_tcam_cmd_gem_flow        (0x2de0)
#define image_5_tcam_cmd_generic_l2        (0x2fbc)
#define image_5_tcam_cmd_generic_l3        (0x2ff0)
#define image_5_tcam_cmd_generic_l4        (0x3024)
#define image_5_tcam_cmd_ic_submit        (0x2c5c)
#define image_5_tcam_cmd_ingress_port        (0x2dc8)
#define image_5_tcam_cmd_inner_pbit        (0x2d30)
#define image_5_tcam_cmd_inner_tpid        (0x2cb8)
#define image_5_tcam_cmd_inner_vid        (0x2d00)
#define image_5_tcam_cmd_ip_protocol        (0x2d60)
#define image_5_tcam_cmd_ipv6_label        (0x2df8)
#define image_5_tcam_cmd_l3_protocol        (0x2d78)
#define image_5_tcam_cmd_network_layer        (0x2da8)
#define image_5_tcam_cmd_outer_pbit        (0x2d18)
#define image_5_tcam_cmd_outer_tpid        (0x2ca0)
#define image_5_tcam_cmd_outer_vid        (0x2ce8)
#define image_5_tcam_cmd_src_ip        (0x2e24)
#define image_5_tcam_cmd_src_ipv6_masked        (0x2e78)
#define image_5_tcam_cmd_src_mac        (0x2f64)
#define image_5_tcam_cmd_src_port        (0x2f34)
#define image_5_tcam_cmd_tos        (0x2d90)
#define image_5_tcam_cmd_vlan_num        (0x2d48)

#else

#define image_5_budget_allocator_1st_wakeup_request        (0x122f)
#define image_5_debug_routine        (0xfd2)
#define image_5_debug_routine_handler        (0x1)
#define image_5_gpe_cmd_copy_bits_16        (0xd99)
#define image_5_gpe_cmd_replace_16        (0xd89)
#define image_5_gpe_cmd_replace_32        (0xd8b)
#define image_5_gpe_cmd_replace_bits_16        (0xd91)
#define image_5_gpe_cmd_skip_if        (0xda5)
#define image_5_gpe_vlan_action_cmd_drop        (0xdad)
#define image_5_gpe_vlan_action_cmd_dscp        (0xdae)
#define image_5_gpe_vlan_action_cmd_mac_hdr_copy        (0xdc3)
#define image_5_initialization_task        (0x2)
#define image_5_processing_wakeup_request        (0x387)
#define image_5_service_queues_tx_task_wakeup_request        (0x1143)
#define image_5_service_queues_update_fifo_read_1st_wakeup_request        (0x1099)
#define image_5_start_task_budget_allocator_1st_wakeup_request        (0x122f)
#define image_5_start_task_debug_routine        (0xfd2)
#define image_5_start_task_initialization_task        (0x2)
#define image_5_start_task_processing_wakeup_request        (0x387)
#define image_5_start_task_service_queues_tx_task_wakeup_request        (0x1143)
#define image_5_start_task_service_queues_update_fifo_read_1st_wakeup_request        (0x1099)
#define image_5_tcam_cmd_dst_ip        (0xbab)
#define image_5_tcam_cmd_dst_ipv6_masked        (0xbc0)
#define image_5_tcam_cmd_dst_mac        (0xbe4)
#define image_5_tcam_cmd_dst_port        (0xbd3)
#define image_5_tcam_cmd_ethertype        (0xb34)
#define image_5_tcam_cmd_gem_flow        (0xb78)
#define image_5_tcam_cmd_generic_l2        (0xbef)
#define image_5_tcam_cmd_generic_l3        (0xbfc)
#define image_5_tcam_cmd_generic_l4        (0xc09)
#define image_5_tcam_cmd_ic_submit        (0xb17)
#define image_5_tcam_cmd_ingress_port        (0xb72)
#define image_5_tcam_cmd_inner_pbit        (0xb4c)
#define image_5_tcam_cmd_inner_tpid        (0xb2e)
#define image_5_tcam_cmd_inner_vid        (0xb40)
#define image_5_tcam_cmd_ip_protocol        (0xb58)
#define image_5_tcam_cmd_ipv6_label        (0xb7e)
#define image_5_tcam_cmd_l3_protocol        (0xb5e)
#define image_5_tcam_cmd_network_layer        (0xb6a)
#define image_5_tcam_cmd_outer_pbit        (0xb46)
#define image_5_tcam_cmd_outer_tpid        (0xb28)
#define image_5_tcam_cmd_outer_vid        (0xb3a)
#define image_5_tcam_cmd_src_ip        (0xb89)
#define image_5_tcam_cmd_src_ipv6_masked        (0xb9e)
#define image_5_tcam_cmd_src_mac        (0xbd9)
#define image_5_tcam_cmd_src_port        (0xbcd)
#define image_5_tcam_cmd_tos        (0xb64)
#define image_5_tcam_cmd_vlan_num        (0xb52)

#endif


#endif

/* IMAGE 6 LABELS */
#ifndef IMAGE_6_CODE_ADDRESSES
#define IMAGE_6_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_6_debug_routine        (0x3ce0)
#define image_6_debug_routine_handler        (0x4)
#define image_6_gpe_cmd_copy_bits_16        (0x33fc)
#define image_6_gpe_cmd_replace_16        (0x33bc)
#define image_6_gpe_cmd_replace_32        (0x33c4)
#define image_6_gpe_cmd_replace_bits_16        (0x33dc)
#define image_6_gpe_cmd_skip_if        (0x342c)
#define image_6_gpe_vlan_action_cmd_drop        (0x344c)
#define image_6_gpe_vlan_action_cmd_dscp        (0x3450)
#define image_6_gpe_vlan_action_cmd_mac_hdr_copy        (0x34a4)
#define image_6_initialization_task        (0x8)
#define image_6_processing_wakeup_request        (0xbb4)
#define image_6_start_task_debug_routine        (0x3ce0)
#define image_6_start_task_initialization_task        (0x8)
#define image_6_start_task_processing_wakeup_request        (0xbb4)
#define image_6_tcam_cmd_dst_ip        (0x2c44)
#define image_6_tcam_cmd_dst_ipv6_masked        (0x2c98)
#define image_6_tcam_cmd_dst_mac        (0x2d28)
#define image_6_tcam_cmd_dst_port        (0x2ce4)
#define image_6_tcam_cmd_ethertype        (0x2a68)
#define image_6_tcam_cmd_gem_flow        (0x2b78)
#define image_6_tcam_cmd_generic_l2        (0x2d54)
#define image_6_tcam_cmd_generic_l3        (0x2d88)
#define image_6_tcam_cmd_generic_l4        (0x2dbc)
#define image_6_tcam_cmd_ic_submit        (0x29f4)
#define image_6_tcam_cmd_ingress_port        (0x2b60)
#define image_6_tcam_cmd_inner_pbit        (0x2ac8)
#define image_6_tcam_cmd_inner_tpid        (0x2a50)
#define image_6_tcam_cmd_inner_vid        (0x2a98)
#define image_6_tcam_cmd_ip_protocol        (0x2af8)
#define image_6_tcam_cmd_ipv6_label        (0x2b90)
#define image_6_tcam_cmd_l3_protocol        (0x2b10)
#define image_6_tcam_cmd_network_layer        (0x2b40)
#define image_6_tcam_cmd_outer_pbit        (0x2ab0)
#define image_6_tcam_cmd_outer_tpid        (0x2a38)
#define image_6_tcam_cmd_outer_vid        (0x2a80)
#define image_6_tcam_cmd_src_ip        (0x2bbc)
#define image_6_tcam_cmd_src_ipv6_masked        (0x2c10)
#define image_6_tcam_cmd_src_mac        (0x2cfc)
#define image_6_tcam_cmd_src_port        (0x2ccc)
#define image_6_tcam_cmd_tos        (0x2b28)
#define image_6_tcam_cmd_vlan_num        (0x2ae0)

#else

#define image_6_debug_routine        (0xf38)
#define image_6_debug_routine_handler        (0x1)
#define image_6_gpe_cmd_copy_bits_16        (0xcff)
#define image_6_gpe_cmd_replace_16        (0xcef)
#define image_6_gpe_cmd_replace_32        (0xcf1)
#define image_6_gpe_cmd_replace_bits_16        (0xcf7)
#define image_6_gpe_cmd_skip_if        (0xd0b)
#define image_6_gpe_vlan_action_cmd_drop        (0xd13)
#define image_6_gpe_vlan_action_cmd_dscp        (0xd14)
#define image_6_gpe_vlan_action_cmd_mac_hdr_copy        (0xd29)
#define image_6_initialization_task        (0x2)
#define image_6_processing_wakeup_request        (0x2ed)
#define image_6_start_task_debug_routine        (0xf38)
#define image_6_start_task_initialization_task        (0x2)
#define image_6_start_task_processing_wakeup_request        (0x2ed)
#define image_6_tcam_cmd_dst_ip        (0xb11)
#define image_6_tcam_cmd_dst_ipv6_masked        (0xb26)
#define image_6_tcam_cmd_dst_mac        (0xb4a)
#define image_6_tcam_cmd_dst_port        (0xb39)
#define image_6_tcam_cmd_ethertype        (0xa9a)
#define image_6_tcam_cmd_gem_flow        (0xade)
#define image_6_tcam_cmd_generic_l2        (0xb55)
#define image_6_tcam_cmd_generic_l3        (0xb62)
#define image_6_tcam_cmd_generic_l4        (0xb6f)
#define image_6_tcam_cmd_ic_submit        (0xa7d)
#define image_6_tcam_cmd_ingress_port        (0xad8)
#define image_6_tcam_cmd_inner_pbit        (0xab2)
#define image_6_tcam_cmd_inner_tpid        (0xa94)
#define image_6_tcam_cmd_inner_vid        (0xaa6)
#define image_6_tcam_cmd_ip_protocol        (0xabe)
#define image_6_tcam_cmd_ipv6_label        (0xae4)
#define image_6_tcam_cmd_l3_protocol        (0xac4)
#define image_6_tcam_cmd_network_layer        (0xad0)
#define image_6_tcam_cmd_outer_pbit        (0xaac)
#define image_6_tcam_cmd_outer_tpid        (0xa8e)
#define image_6_tcam_cmd_outer_vid        (0xaa0)
#define image_6_tcam_cmd_src_ip        (0xaef)
#define image_6_tcam_cmd_src_ipv6_masked        (0xb04)
#define image_6_tcam_cmd_src_mac        (0xb3f)
#define image_6_tcam_cmd_src_port        (0xb33)
#define image_6_tcam_cmd_tos        (0xaca)
#define image_6_tcam_cmd_vlan_num        (0xab8)

#endif


#endif

/* IMAGE 7 LABELS */
#ifndef IMAGE_7_CODE_ADDRESSES
#define IMAGE_7_CODE_ADDRESSES

#ifndef PC_ADDRESS_INST_IND

#define image_7_debug_routine        (0x3ce0)
#define image_7_debug_routine_handler        (0x4)
#define image_7_gpe_cmd_copy_bits_16        (0x33fc)
#define image_7_gpe_cmd_replace_16        (0x33bc)
#define image_7_gpe_cmd_replace_32        (0x33c4)
#define image_7_gpe_cmd_replace_bits_16        (0x33dc)
#define image_7_gpe_cmd_skip_if        (0x342c)
#define image_7_gpe_vlan_action_cmd_drop        (0x344c)
#define image_7_gpe_vlan_action_cmd_dscp        (0x3450)
#define image_7_gpe_vlan_action_cmd_mac_hdr_copy        (0x34a4)
#define image_7_initialization_task        (0x8)
#define image_7_processing_wakeup_request        (0xbb4)
#define image_7_start_task_debug_routine        (0x3ce0)
#define image_7_start_task_initialization_task        (0x8)
#define image_7_start_task_processing_wakeup_request        (0xbb4)
#define image_7_tcam_cmd_dst_ip        (0x2c44)
#define image_7_tcam_cmd_dst_ipv6_masked        (0x2c98)
#define image_7_tcam_cmd_dst_mac        (0x2d28)
#define image_7_tcam_cmd_dst_port        (0x2ce4)
#define image_7_tcam_cmd_ethertype        (0x2a68)
#define image_7_tcam_cmd_gem_flow        (0x2b78)
#define image_7_tcam_cmd_generic_l2        (0x2d54)
#define image_7_tcam_cmd_generic_l3        (0x2d88)
#define image_7_tcam_cmd_generic_l4        (0x2dbc)
#define image_7_tcam_cmd_ic_submit        (0x29f4)
#define image_7_tcam_cmd_ingress_port        (0x2b60)
#define image_7_tcam_cmd_inner_pbit        (0x2ac8)
#define image_7_tcam_cmd_inner_tpid        (0x2a50)
#define image_7_tcam_cmd_inner_vid        (0x2a98)
#define image_7_tcam_cmd_ip_protocol        (0x2af8)
#define image_7_tcam_cmd_ipv6_label        (0x2b90)
#define image_7_tcam_cmd_l3_protocol        (0x2b10)
#define image_7_tcam_cmd_network_layer        (0x2b40)
#define image_7_tcam_cmd_outer_pbit        (0x2ab0)
#define image_7_tcam_cmd_outer_tpid        (0x2a38)
#define image_7_tcam_cmd_outer_vid        (0x2a80)
#define image_7_tcam_cmd_src_ip        (0x2bbc)
#define image_7_tcam_cmd_src_ipv6_masked        (0x2c10)
#define image_7_tcam_cmd_src_mac        (0x2cfc)
#define image_7_tcam_cmd_src_port        (0x2ccc)
#define image_7_tcam_cmd_tos        (0x2b28)
#define image_7_tcam_cmd_vlan_num        (0x2ae0)

#else

#define image_7_debug_routine        (0xf38)
#define image_7_debug_routine_handler        (0x1)
#define image_7_gpe_cmd_copy_bits_16        (0xcff)
#define image_7_gpe_cmd_replace_16        (0xcef)
#define image_7_gpe_cmd_replace_32        (0xcf1)
#define image_7_gpe_cmd_replace_bits_16        (0xcf7)
#define image_7_gpe_cmd_skip_if        (0xd0b)
#define image_7_gpe_vlan_action_cmd_drop        (0xd13)
#define image_7_gpe_vlan_action_cmd_dscp        (0xd14)
#define image_7_gpe_vlan_action_cmd_mac_hdr_copy        (0xd29)
#define image_7_initialization_task        (0x2)
#define image_7_processing_wakeup_request        (0x2ed)
#define image_7_start_task_debug_routine        (0xf38)
#define image_7_start_task_initialization_task        (0x2)
#define image_7_start_task_processing_wakeup_request        (0x2ed)
#define image_7_tcam_cmd_dst_ip        (0xb11)
#define image_7_tcam_cmd_dst_ipv6_masked        (0xb26)
#define image_7_tcam_cmd_dst_mac        (0xb4a)
#define image_7_tcam_cmd_dst_port        (0xb39)
#define image_7_tcam_cmd_ethertype        (0xa9a)
#define image_7_tcam_cmd_gem_flow        (0xade)
#define image_7_tcam_cmd_generic_l2        (0xb55)
#define image_7_tcam_cmd_generic_l3        (0xb62)
#define image_7_tcam_cmd_generic_l4        (0xb6f)
#define image_7_tcam_cmd_ic_submit        (0xa7d)
#define image_7_tcam_cmd_ingress_port        (0xad8)
#define image_7_tcam_cmd_inner_pbit        (0xab2)
#define image_7_tcam_cmd_inner_tpid        (0xa94)
#define image_7_tcam_cmd_inner_vid        (0xaa6)
#define image_7_tcam_cmd_ip_protocol        (0xabe)
#define image_7_tcam_cmd_ipv6_label        (0xae4)
#define image_7_tcam_cmd_l3_protocol        (0xac4)
#define image_7_tcam_cmd_network_layer        (0xad0)
#define image_7_tcam_cmd_outer_pbit        (0xaac)
#define image_7_tcam_cmd_outer_tpid        (0xa8e)
#define image_7_tcam_cmd_outer_vid        (0xaa0)
#define image_7_tcam_cmd_src_ip        (0xaef)
#define image_7_tcam_cmd_src_ipv6_masked        (0xb04)
#define image_7_tcam_cmd_src_mac        (0xb3f)
#define image_7_tcam_cmd_src_port        (0xb33)
#define image_7_tcam_cmd_tos        (0xaca)
#define image_7_tcam_cmd_vlan_num        (0xab8)

#endif


#endif

/* COMMON LABELS */
#ifndef COMMON_CODE_ADDRESSES
#define COMMON_CODE_ADDRESSES

#define INVALID_LABEL_ADDRESS 0xFFFFFF

#ifndef PC_ADDRESS_INST_IND

#define TCAM_CMD_DST_IP_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2c44, 0x2d50, INVALID_LABEL_ADDRESS, 0x2d50, 0x2eac, 0x2c44, 0x2c44}
#define TCAM_CMD_DST_IPV6_MASKED_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2c98, 0x2da4, INVALID_LABEL_ADDRESS, 0x2da4, 0x2f00, 0x2c98, 0x2c98}
#define TCAM_CMD_DST_MAC_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2d28, 0x2e34, INVALID_LABEL_ADDRESS, 0x2e34, 0x2f90, 0x2d28, 0x2d28}
#define TCAM_CMD_DST_PORT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2ce4, 0x2df0, INVALID_LABEL_ADDRESS, 0x2df0, 0x2f4c, 0x2ce4, 0x2ce4}
#define TCAM_CMD_ETHERTYPE_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2a68, 0x2b74, INVALID_LABEL_ADDRESS, 0x2b74, 0x2cd0, 0x2a68, 0x2a68}
#define TCAM_CMD_GEM_FLOW_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2b78, 0x2c84, INVALID_LABEL_ADDRESS, 0x2c84, 0x2de0, 0x2b78, 0x2b78}
#define TCAM_CMD_GENERIC_L2_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2d54, 0x2e60, INVALID_LABEL_ADDRESS, 0x2e60, 0x2fbc, 0x2d54, 0x2d54}
#define TCAM_CMD_GENERIC_L3_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2d88, 0x2e94, INVALID_LABEL_ADDRESS, 0x2e94, 0x2ff0, 0x2d88, 0x2d88}
#define TCAM_CMD_GENERIC_L4_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2dbc, 0x2ec8, INVALID_LABEL_ADDRESS, 0x2ec8, 0x3024, 0x2dbc, 0x2dbc}
#define TCAM_CMD_IC_SUBMIT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x29f4, 0x2b00, INVALID_LABEL_ADDRESS, 0x2b00, 0x2c5c, 0x29f4, 0x29f4}
#define TCAM_CMD_INGRESS_PORT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2b60, 0x2c6c, INVALID_LABEL_ADDRESS, 0x2c6c, 0x2dc8, 0x2b60, 0x2b60}
#define TCAM_CMD_INNER_PBIT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2ac8, 0x2bd4, INVALID_LABEL_ADDRESS, 0x2bd4, 0x2d30, 0x2ac8, 0x2ac8}
#define TCAM_CMD_INNER_TPID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2a50, 0x2b5c, INVALID_LABEL_ADDRESS, 0x2b5c, 0x2cb8, 0x2a50, 0x2a50}
#define TCAM_CMD_INNER_VID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2a98, 0x2ba4, INVALID_LABEL_ADDRESS, 0x2ba4, 0x2d00, 0x2a98, 0x2a98}
#define TCAM_CMD_IP_PROTOCOL_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2af8, 0x2c04, INVALID_LABEL_ADDRESS, 0x2c04, 0x2d60, 0x2af8, 0x2af8}
#define TCAM_CMD_IPV6_LABEL_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2b90, 0x2c9c, INVALID_LABEL_ADDRESS, 0x2c9c, 0x2df8, 0x2b90, 0x2b90}
#define TCAM_CMD_L3_PROTOCOL_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2b10, 0x2c1c, INVALID_LABEL_ADDRESS, 0x2c1c, 0x2d78, 0x2b10, 0x2b10}
#define TCAM_CMD_NETWORK_LAYER_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2b40, 0x2c4c, INVALID_LABEL_ADDRESS, 0x2c4c, 0x2da8, 0x2b40, 0x2b40}
#define TCAM_CMD_OUTER_PBIT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2ab0, 0x2bbc, INVALID_LABEL_ADDRESS, 0x2bbc, 0x2d18, 0x2ab0, 0x2ab0}
#define TCAM_CMD_OUTER_TPID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2a38, 0x2b44, INVALID_LABEL_ADDRESS, 0x2b44, 0x2ca0, 0x2a38, 0x2a38}
#define TCAM_CMD_OUTER_VID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2a80, 0x2b8c, INVALID_LABEL_ADDRESS, 0x2b8c, 0x2ce8, 0x2a80, 0x2a80}
#define TCAM_CMD_SRC_IP_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2bbc, 0x2cc8, INVALID_LABEL_ADDRESS, 0x2cc8, 0x2e24, 0x2bbc, 0x2bbc}
#define TCAM_CMD_SRC_IPV6_MASKED_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2c10, 0x2d1c, INVALID_LABEL_ADDRESS, 0x2d1c, 0x2e78, 0x2c10, 0x2c10}
#define TCAM_CMD_SRC_MAC_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2cfc, 0x2e08, INVALID_LABEL_ADDRESS, 0x2e08, 0x2f64, 0x2cfc, 0x2cfc}
#define TCAM_CMD_SRC_PORT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2ccc, 0x2dd8, INVALID_LABEL_ADDRESS, 0x2dd8, 0x2f34, 0x2ccc, 0x2ccc}
#define TCAM_CMD_TOS_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2b28, 0x2c34, INVALID_LABEL_ADDRESS, 0x2c34, 0x2d90, 0x2b28, 0x2b28}
#define TCAM_CMD_VLAN_NUM_ADDR_ARR {INVALID_LABEL_ADDRESS, 0x2ae0, 0x2bec, INVALID_LABEL_ADDRESS, 0x2bec, 0x2d48, 0x2ae0, 0x2ae0}

#else

#define TCAM_CMD_DST_IP_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb11, 0xb54, INVALID_LABEL_ADDRESS, 0xb54, 0xbab, 0xb11, 0xb11}
#define TCAM_CMD_DST_IPV6_MASKED_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb26, 0xb69, INVALID_LABEL_ADDRESS, 0xb69, 0xbc0, 0xb26, 0xb26}
#define TCAM_CMD_DST_MAC_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb4a, 0xb8d, INVALID_LABEL_ADDRESS, 0xb8d, 0xbe4, 0xb4a, 0xb4a}
#define TCAM_CMD_DST_PORT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb39, 0xb7c, INVALID_LABEL_ADDRESS, 0xb7c, 0xbd3, 0xb39, 0xb39}
#define TCAM_CMD_ETHERTYPE_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xa9a, 0xadd, INVALID_LABEL_ADDRESS, 0xadd, 0xb34, 0xa9a, 0xa9a}
#define TCAM_CMD_GEM_FLOW_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xade, 0xb21, INVALID_LABEL_ADDRESS, 0xb21, 0xb78, 0xade, 0xade}
#define TCAM_CMD_GENERIC_L2_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb55, 0xb98, INVALID_LABEL_ADDRESS, 0xb98, 0xbef, 0xb55, 0xb55}
#define TCAM_CMD_GENERIC_L3_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb62, 0xba5, INVALID_LABEL_ADDRESS, 0xba5, 0xbfc, 0xb62, 0xb62}
#define TCAM_CMD_GENERIC_L4_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb6f, 0xbb2, INVALID_LABEL_ADDRESS, 0xbb2, 0xc09, 0xb6f, 0xb6f}
#define TCAM_CMD_IC_SUBMIT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xa7d, 0xac0, INVALID_LABEL_ADDRESS, 0xac0, 0xb17, 0xa7d, 0xa7d}
#define TCAM_CMD_INGRESS_PORT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xad8, 0xb1b, INVALID_LABEL_ADDRESS, 0xb1b, 0xb72, 0xad8, 0xad8}
#define TCAM_CMD_INNER_PBIT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xab2, 0xaf5, INVALID_LABEL_ADDRESS, 0xaf5, 0xb4c, 0xab2, 0xab2}
#define TCAM_CMD_INNER_TPID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xa94, 0xad7, INVALID_LABEL_ADDRESS, 0xad7, 0xb2e, 0xa94, 0xa94}
#define TCAM_CMD_INNER_VID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xaa6, 0xae9, INVALID_LABEL_ADDRESS, 0xae9, 0xb40, 0xaa6, 0xaa6}
#define TCAM_CMD_IP_PROTOCOL_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xabe, 0xb01, INVALID_LABEL_ADDRESS, 0xb01, 0xb58, 0xabe, 0xabe}
#define TCAM_CMD_IPV6_LABEL_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xae4, 0xb27, INVALID_LABEL_ADDRESS, 0xb27, 0xb7e, 0xae4, 0xae4}
#define TCAM_CMD_L3_PROTOCOL_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xac4, 0xb07, INVALID_LABEL_ADDRESS, 0xb07, 0xb5e, 0xac4, 0xac4}
#define TCAM_CMD_NETWORK_LAYER_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xad0, 0xb13, INVALID_LABEL_ADDRESS, 0xb13, 0xb6a, 0xad0, 0xad0}
#define TCAM_CMD_OUTER_PBIT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xaac, 0xaef, INVALID_LABEL_ADDRESS, 0xaef, 0xb46, 0xaac, 0xaac}
#define TCAM_CMD_OUTER_TPID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xa8e, 0xad1, INVALID_LABEL_ADDRESS, 0xad1, 0xb28, 0xa8e, 0xa8e}
#define TCAM_CMD_OUTER_VID_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xaa0, 0xae3, INVALID_LABEL_ADDRESS, 0xae3, 0xb3a, 0xaa0, 0xaa0}
#define TCAM_CMD_SRC_IP_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xaef, 0xb32, INVALID_LABEL_ADDRESS, 0xb32, 0xb89, 0xaef, 0xaef}
#define TCAM_CMD_SRC_IPV6_MASKED_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb04, 0xb47, INVALID_LABEL_ADDRESS, 0xb47, 0xb9e, 0xb04, 0xb04}
#define TCAM_CMD_SRC_MAC_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb3f, 0xb82, INVALID_LABEL_ADDRESS, 0xb82, 0xbd9, 0xb3f, 0xb3f}
#define TCAM_CMD_SRC_PORT_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xb33, 0xb76, INVALID_LABEL_ADDRESS, 0xb76, 0xbcd, 0xb33, 0xb33}
#define TCAM_CMD_TOS_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xaca, 0xb0d, INVALID_LABEL_ADDRESS, 0xb0d, 0xb64, 0xaca, 0xaca}
#define TCAM_CMD_VLAN_NUM_ADDR_ARR {INVALID_LABEL_ADDRESS, 0xab8, 0xafb, INVALID_LABEL_ADDRESS, 0xafb, 0xb52, 0xab8, 0xab8}

#endif


#endif

