/* SPDX-License-Identifier: GPL-2.0+ */
 /*
  * Copyright 2019-2021 Broadcom.
  */

#ifndef _BNXT_HSI_H_
#define _BNXT_HSI_H_

/* input (size:128b/16B) */
struct input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
};

/* output (size:64b/8B) */
struct output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
};

/* hwrm_short_input (size:128b/16B) */
struct hwrm_short_input {
	__le16   req_type;
	__le16   signature;
#define SHORT_REQ_SIGNATURE_SHORT_CMD 0x4321UL
	__le16   unused_0;
	__le16   size;
	__le64   req_addr;
};

#define HWRM_VER_GET                              0x0UL
#define HWRM_FUNC_RESET                           0x11UL
#define HWRM_FUNC_QCAPS                           0x15UL
#define HWRM_FUNC_QCFG                            0x16UL
#define HWRM_FUNC_CFG                             0x17UL
#define HWRM_FUNC_DRV_UNRGTR                      0x1aUL
#define HWRM_FUNC_DRV_RGTR                        0x1dUL
#define HWRM_PORT_PHY_CFG                         0x20UL
#define HWRM_PORT_MAC_CFG                         0x21UL
#define HWRM_PORT_PHY_QCFG                        0x27UL
#define HWRM_VNIC_ALLOC                           0x40UL
#define HWRM_VNIC_FREE                            0x41UL
#define HWRM_VNIC_CFG                             0x42UL
#define HWRM_RING_ALLOC                           0x50UL
#define HWRM_RING_FREE                            0x51UL
#define HWRM_RING_GRP_ALLOC                       0x60UL
#define HWRM_RING_GRP_FREE                        0x61UL
#define HWRM_CFA_L2_FILTER_ALLOC                  0x90UL
#define HWRM_CFA_L2_FILTER_FREE                   0x91UL
#define HWRM_CFA_L2_SET_RX_MASK                   0x93UL
#define HWRM_STAT_CTX_ALLOC                       0xb0UL
#define HWRM_STAT_CTX_FREE                        0xb1UL
#define HWRM_FUNC_RESOURCE_QCAPS                  0x190UL
#define HWRM_NVM_FLUSH                            0xfff0UL
#define HWRM_NVM_GET_VARIABLE                     0xfff1UL
#define HWRM_NVM_SET_VARIABLE                     0xfff2UL

#define HWRM_NA_SIGNATURE ((__le32)(-1))
#define HWRM_MAX_REQ_LEN 128
#define HWRM_VERSION_MAJOR 1
#define HWRM_VERSION_MINOR 10
#define HWRM_VERSION_UPDATE 0

/* hwrm_ver_get_input (size:192b/24B) */
struct hwrm_ver_get_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	u8       hwrm_intf_maj;
	u8       hwrm_intf_min;
	u8       hwrm_intf_upd;
	u8       unused_0[5];
};

/* hwrm_ver_get_output (size:1408b/176B) */
struct hwrm_ver_get_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	u8       hwrm_intf_maj_8b;
	u8       hwrm_intf_min_8b;
	u8       hwrm_intf_upd_8b;
	u8       hwrm_intf_rsvd_8b;
	u8       hwrm_fw_maj_8b;
	u8       hwrm_fw_min_8b;
	u8       hwrm_fw_bld_8b;
	u8       hwrm_fw_rsvd_8b;
	u8       mgmt_fw_maj_8b;
	u8       mgmt_fw_min_8b;
	u8       mgmt_fw_bld_8b;
	u8       mgmt_fw_rsvd_8b;
	u8       netctrl_fw_maj_8b;
	u8       netctrl_fw_min_8b;
	u8       netctrl_fw_bld_8b;
	u8       netctrl_fw_rsvd_8b;
	__le32   dev_caps_cfg;
#define VER_GET_RESP_DEV_CAPS_CFG_SHORT_CMD_SUPPORTED                      0x4UL
#define VER_GET_RESP_DEV_CAPS_CFG_SHORT_CMD_REQUIRED                       0x8UL
	u8       roce_fw_maj_8b;
	u8       roce_fw_min_8b;
	u8       roce_fw_bld_8b;
	u8       roce_fw_rsvd_8b;
	char     hwrm_fw_name[16];
	char     mgmt_fw_name[16];
	char     netctrl_fw_name[16];
	u8       reserved2[16];
	char     roce_fw_name[16];
	__le16   chip_num;
	u8       chip_rev;
	u8       chip_metal;
	u8       chip_bond_id;
	u8       chip_platform_type;
	__le16   max_req_win_len;
	__le16   max_resp_len;
	__le16   def_req_timeout;
	u8       flags;
	u8       unused_0[2];
	u8       always_1;
	__le16   hwrm_intf_major;
	__le16   hwrm_intf_minor;
	__le16   hwrm_intf_build;
	__le16   hwrm_intf_patch;
	__le16   hwrm_fw_major;
	__le16   hwrm_fw_minor;
	__le16   hwrm_fw_build;
	__le16   hwrm_fw_patch;
	__le16   mgmt_fw_major;
	__le16   mgmt_fw_minor;
	__le16   mgmt_fw_build;
	__le16   mgmt_fw_patch;
	__le16   netctrl_fw_major;
	__le16   netctrl_fw_minor;
	__le16   netctrl_fw_build;
	__le16   netctrl_fw_patch;
	__le16   roce_fw_major;
	__le16   roce_fw_minor;
	__le16   roce_fw_build;
	__le16   roce_fw_patch;
	__le16   max_ext_req_len;
	u8       unused_1[5];
	u8       valid;
};

/* hwrm_async_event_cmpl (size:128b/16B) */
struct hwrm_async_event_cmpl {
	__le16   type;
	__le16   event_id;
#define ASYNC_EVENT_CMPL_EVENT_ID_LINK_STATUS_CHANGE         0x0UL
	__le32   event_data2;
	u8       opaque_v;
	u8       timestamp_lo;
	__le16   timestamp_hi;
	__le32   event_data1;
};

/* hwrm_func_reset_input (size:192b/24B) */
struct hwrm_func_reset_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   enables;
	__le16   vf_id;
	u8       func_reset_level;
#define FUNC_RESET_REQ_FUNC_RESET_LEVEL_RESETME       0x1UL
	u8       unused_0;
};

/* hwrm_func_qcaps_input (size:192b/24B) */
struct hwrm_func_qcaps_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le16   fid;
	u8       unused_0[6];
};

/* hwrm_func_qcaps_output (size:640b/80B) */
struct hwrm_func_qcaps_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le16   fid;
	__le16   port_id;
	__le32   flags;
	u8       mac_address[6];
	__le16   max_rsscos_ctx;
	__le16   max_cmpl_rings;
	__le16   max_tx_rings;
	__le16   max_rx_rings;
	__le16   max_l2_ctxs;
	__le16   max_vnics;
	__le16   first_vf_id;
	__le16   max_vfs;
	__le16   max_stat_ctx;
	__le32   max_encap_records;
	__le32   max_decap_records;
	__le32   max_tx_em_flows;
	__le32   max_tx_wm_flows;
	__le32   max_rx_em_flows;
	__le32   max_rx_wm_flows;
	__le32   max_mcast_filters;
	__le32   max_flow_id;
	__le32   max_hw_ring_grps;
	__le16   max_sp_tx_rings;
	u8       unused_0;
	u8       valid;
};

/* hwrm_func_qcfg_input (size:192b/24B) */
struct hwrm_func_qcfg_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le16   fid;
	u8       unused_0[6];
};

/* hwrm_func_qcfg_output (size:704b/88B) */
struct hwrm_func_qcfg_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le16   fid;
	__le16   port_id;
	__le16   vlan;
	__le16   flags;
#define FUNC_QCFG_RESP_FLAGS_MULTI_HOST                   0x20UL
	u8       mac_address[6];
	__le16   pci_id;
	__le16   alloc_rsscos_ctx;
	__le16   alloc_cmpl_rings;
	__le16   alloc_tx_rings;
	__le16   alloc_rx_rings;
	__le16   alloc_l2_ctx;
	__le16   alloc_vnics;
	__le16   mtu;
	__le16   mru;
	__le16   stat_ctx_id;
	u8       port_partition_type;
#define FUNC_QCFG_RESP_PORT_PARTITION_TYPE_NPAR1_0 0x2UL
	u8       port_pf_cnt;
	__le16   dflt_vnic_id;
	__le16   max_mtu_configured;
	__le32   min_bw;
	__le32   max_bw;
	u8       evb_mode;
	u8       options;
	__le16   alloc_vfs;
	__le32   alloc_mcast_filters;
	__le32   alloc_hw_ring_grps;
	__le16   alloc_sp_tx_rings;
	__le16   alloc_stat_ctx;
	__le16   alloc_msix;
	__le16   registered_vfs;
	u8       unused_1[3];
	u8       always_1;
	__le32   reset_addr_poll;
	u8       unused_2[3];
	u8       valid;
};

/* hwrm_func_cfg_input (size:704b/88B) */
struct hwrm_func_cfg_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le16   fid;
	__le16   num_msix;
	__le32   flags;
	__le32   enables;
#define FUNC_CFG_REQ_ENABLES_NUM_CMPL_RINGS          0x8UL
#define FUNC_CFG_REQ_ENABLES_NUM_TX_RINGS            0x10UL
#define FUNC_CFG_REQ_ENABLES_NUM_RX_RINGS            0x20UL
#define FUNC_CFG_REQ_ENABLES_NUM_STAT_CTXS           0x100UL
#define FUNC_CFG_REQ_ENABLES_ASYNC_EVENT_CR          0x4000UL
#define FUNC_CFG_REQ_ENABLES_NUM_HW_RING_GRPS        0x80000UL
	__le16   mtu;
	__le16   mru;
	__le16   num_rsscos_ctxs;
	__le16   num_cmpl_rings;
	__le16   num_tx_rings;
	__le16   num_rx_rings;
	__le16   num_l2_ctxs;
	__le16   num_vnics;
	__le16   num_stat_ctxs;
	__le16   num_hw_ring_grps;
	u8       dflt_mac_addr[6];
	__le16   dflt_vlan;
	__be32   dflt_ip_addr[4];
	__le32   min_bw;
	__le32   max_bw;
	__le16   async_event_cr;
	u8       vlan_antispoof_mode;
	u8       allowed_vlan_pris;
	u8       evb_mode;
	u8       options;
	__le16   num_mcast_filters;
};

/* hwrm_func_drv_rgtr_input (size:896b/112B) */
struct hwrm_func_drv_rgtr_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   flags;
	__le32   enables;
#define FUNC_DRV_RGTR_REQ_ENABLES_OS_TYPE             0x1UL
#define FUNC_DRV_RGTR_REQ_ENABLES_VER                 0x2UL
#define FUNC_DRV_RGTR_REQ_ENABLES_ASYNC_EVENT_FWD     0x10UL
	__le16   os_type;
#define FUNC_DRV_RGTR_REQ_OS_TYPE_OTHER     0x1UL
	u8       ver_maj_8b;
	u8       ver_min_8b;
	u8       ver_upd_8b;
	u8       unused_0[3];
	__le32   timestamp;
	u8       unused_1[4];
	__le32   vf_req_fwd[8];
	__le32   async_event_fwd[8];
	__le16   ver_maj;
	__le16   ver_min;
	__le16   ver_upd;
	__le16   ver_patch;
};

/* hwrm_func_drv_unrgtr_input (size:192b/24B) */
struct hwrm_func_drv_unrgtr_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   flags;
#define FUNC_DRV_UNRGTR_REQ_FLAGS_PREPARE_FOR_SHUTDOWN     0x1UL
	u8       unused_0[4];
};

/* hwrm_func_resource_qcaps_input (size:192b/24B) */
struct hwrm_func_resource_qcaps_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le16   fid;
	u8       unused_0[6];
};

/* hwrm_func_resource_qcaps_output (size:448b/56B) */
struct hwrm_func_resource_qcaps_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le16   max_vfs;
	__le16   max_msix;
	__le16   vf_reservation_strategy;
	__le16   min_rsscos_ctx;
	__le16   max_rsscos_ctx;
	__le16   min_cmpl_rings;
	__le16   max_cmpl_rings;
	__le16   min_tx_rings;
	__le16   max_tx_rings;
	__le16   min_rx_rings;
	__le16   max_rx_rings;
	__le16   min_l2_ctxs;
	__le16   max_l2_ctxs;
	__le16   min_vnics;
	__le16   max_vnics;
	__le16   min_stat_ctx;
	__le16   max_stat_ctx;
	__le16   min_hw_ring_grps;
	__le16   max_hw_ring_grps;
	__le16   max_tx_scheduler_inputs;
	__le16   flags;
	u8       unused_0[5];
	u8       valid;
};

/* hwrm_func_vlan_qcfg_input (size:192b/24B) */
struct hwrm_func_vlan_qcfg_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le16   fid;
	u8       unused_0[6];
};

/* hwrm_port_phy_cfg_input (size:448b/56B) */
struct hwrm_port_phy_cfg_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   flags;
#define PORT_PHY_CFG_REQ_FLAGS_RESET_PHY                0x1UL
#define PORT_PHY_CFG_REQ_FLAGS_FORCE                    0x4UL
	__le32   enables;
#define PORT_PHY_CFG_REQ_ENABLES_AUTO_MODE                0x1UL
#define PORT_PHY_CFG_REQ_ENABLES_AUTO_DUPLEX              0x2UL
#define PORT_PHY_CFG_REQ_ENABLES_AUTO_PAUSE               0x4UL
#define PORT_PHY_CFG_REQ_ENABLES_AUTO_LINK_SPEED_MASK     0x10UL
	__le16   port_id;
	__le16   force_link_speed;
#define PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_1GB   0xaUL
#define PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_10GB  0x64UL
#define PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_25GB  0xfaUL
#define PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_40GB  0x190UL
#define PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_50GB  0x1f4UL
#define PORT_PHY_CFG_REQ_FORCE_LINK_SPEED_100GB 0x3e8UL
	u8       auto_mode;
#define PORT_PHY_CFG_REQ_AUTO_MODE_SPEED_MASK   0x4UL
	u8       auto_duplex;
#define PORT_PHY_CFG_REQ_AUTO_DUPLEX_BOTH 0x2UL
	u8       auto_pause;
#define PORT_PHY_CFG_REQ_AUTO_PAUSE_TX                0x1UL
#define PORT_PHY_CFG_REQ_AUTO_PAUSE_RX                0x2UL
	u8       unused_0;
	__le16   auto_link_speed;
	__le16   auto_link_speed_mask;
#define PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_100MB       0x2UL
#define PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_1GB         0x8UL
#define PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_10GB        0x40UL
#define PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_25GB        0x100UL
#define PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_40GB        0x200UL
#define PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_50GB        0x400UL
#define PORT_PHY_CFG_REQ_AUTO_LINK_SPEED_MASK_100GB       0x800UL
	u8       wirespeed;
	u8       lpbk;
	u8       force_pause;
	u8       unused_1;
	__le32   preemphasis;
	__le16   eee_link_speed_mask;
	u8       unused_2[2];
	__le32   tx_lpi_timer;
	__le32   unused_3;
};

/* hwrm_port_phy_qcfg_input (size:192b/24B) */
struct hwrm_port_phy_qcfg_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le16   port_id;
	u8       unused_0[6];
};

/* hwrm_port_phy_qcfg_output (size:768b/96B) */
struct hwrm_port_phy_qcfg_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	u8       link;
#define PORT_PHY_QCFG_RESP_LINK_LINK    0x2UL
	u8       unused_0;
	__le16   link_speed;
#define PORT_PHY_QCFG_RESP_LINK_SPEED_100MB 0x1UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_1GB   0xaUL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_2GB   0x14UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_2_5GB 0x19UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_10GB  0x64UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_20GB  0xc8UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_25GB  0xfaUL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_40GB  0x190UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_50GB  0x1f4UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_100GB 0x3e8UL
#define PORT_PHY_QCFG_RESP_LINK_SPEED_10MB  0xffffUL
	u8       duplex_cfg;
	u8       pause;
	__le16   support_speeds;
#define PORT_QCFG_SUPPORT_SPEEDS_100MBHD     0x1UL
#define PORT_QCFG_SUPPORT_SPEEDS_100MB       0x2UL
#define PORT_QCFG_SUPPORT_SPEEDS_1GBHD       0x4UL
#define PORT_QCFG_SUPPORT_SPEEDS_1GB         0x8UL
#define PORT_QCFG_SUPPORT_SPEEDS_2GB         0x10UL
#define PORT_QCFG_SUPPORT_SPEEDS_2_5GB       0x20UL
#define PORT_QCFG_SUPPORT_SPEEDS_10GB        0x40UL
#define PORT_QCFG_SUPPORT_SPEEDS_20GB        0x80UL
#define PORT_QCFG_SUPPORT_SPEEDS_25GB        0x100UL
#define PORT_QCFG_SUPPORT_SPEEDS_50GB        0x400UL
#define PORT_QCFG_SUPPORT_SPEEDS_100GB       0x800UL
#define PORT_QCFG_SUPPORT_SPEEDS_200GB       0x4000UL
	__le16   force_link_speed;
	u8       auto_mode;
	u8       auto_pause;
	__le16   auto_link_speed;
	__le16   auto_link_speed_mask;
	u8       wirespeed;
	u8       lpbk;
	u8       force_pause;
	u8       module_status;
	__le32   preemphasis;
	u8       phy_maj;
	u8       phy_min;
	u8       phy_bld;
	u8       phy_type;
	u8       media_type;
	u8       xcvr_pkg_type;
	u8       eee_config_phy_addr;
	u8       parallel_detect;
	__le16   link_partner_adv_speeds;
	u8       link_partner_adv_auto_mode;
	u8       link_partner_adv_pause;
	__le16   adv_eee_link_speed_mask;
	__le16   link_partner_adv_eee_link_speed_mask;
	__le32   xcvr_identifier_type_tx_lpi_timer;
	__le16   fec_cfg;
	u8       duplex_state;
	u8       option_flags;
	char     phy_vendor_name[16];
	char     phy_vendor_partnumber[16];
	u8       unused_2[7];
	u8       valid;
};

/* hwrm_port_mac_cfg_input (size:320b/40B) */
struct hwrm_port_mac_cfg_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   flags;
	__le32   enables;
	__le16   port_id;
	u8       ipg;
	u8       lpbk;
#define PORT_MAC_CFG_REQ_LPBK_NONE   0x0UL
	u8       vlan_pri2cos_map_pri;
	u8       reserved1;
	u8       tunnel_pri2cos_map_pri;
	u8       dscp2pri_map_pri;
	__le16   rx_ts_capture_ptp_msg_type;
	__le16   tx_ts_capture_ptp_msg_type;
	u8       cos_field_cfg;
	u8       unused_0[3];
};

/* hwrm_vnic_alloc_input (size:192b/24B) */
struct hwrm_vnic_alloc_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   flags;
#define VNIC_ALLOC_REQ_FLAGS_DEFAULT     0x1UL
	u8       unused_0[4];
};

/* hwrm_vnic_alloc_output (size:128b/16B) */
struct hwrm_vnic_alloc_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le32   vnic_id;
	u8       unused_0[3];
	u8       valid;
};

/* hwrm_vnic_free_input (size:192b/24B) */
struct hwrm_vnic_free_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   vnic_id;
	u8       unused_0[4];
};

/* hwrm_vnic_cfg_input (size:320b/40B) */
struct hwrm_vnic_cfg_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   flags;
	__le32   enables;
#define VNIC_CFG_REQ_ENABLES_DFLT_RING_GRP            0x1UL
#define VNIC_CFG_REQ_ENABLES_MRU                      0x10UL
	__le16   vnic_id;
	__le16   dflt_ring_grp;
	__le16   rss_rule;
	__le16   cos_rule;
	__le16   lb_rule;
	__le16   mru;
	__le16   default_rx_ring_id;
	__le16   default_cmpl_ring_id;
};

/* hwrm_ring_alloc_input (size:704b/88B) */
struct hwrm_ring_alloc_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   enables;
#define RING_ALLOC_REQ_ENABLES_RX_BUF_SIZE_VALID     0x100UL
	u8       ring_type;
#define RING_ALLOC_REQ_RING_TYPE_L2_CMPL   0x0UL
#define RING_ALLOC_REQ_RING_TYPE_TX        0x1UL
#define RING_ALLOC_REQ_RING_TYPE_RX        0x2UL
	u8       unused_0;
	__le16   flags;
	__le64   page_tbl_addr;
	__le32   fbo;
	u8       page_size;
	u8       page_tbl_depth;
	u8       unused_1[2];
	__le32   length;
	__le16   logical_id;
	__le16   cmpl_ring_id;
	__le16   queue_id;
	__le16   rx_buf_size;
	__le16   rx_ring_id;
	__le16   nq_ring_id;
	__le16   ring_arb_cfg;
	__le16   unused_3;
	__le32   reserved3;
	__le32   stat_ctx_id;
	__le32   reserved4;
	__le32   max_bw;
	u8       int_mode;
#define RING_ALLOC_REQ_INT_MODE_POLL   0x3UL
	u8       unused_4[3];
	__le64   cq_handle;
};

/* hwrm_ring_alloc_output (size:128b/16B) */
struct hwrm_ring_alloc_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le16   ring_id;
	__le16   logical_ring_id;
	u8       unused_0[3];
	u8       valid;
};

/* hwrm_ring_free_input (size:192b/24B) */
struct hwrm_ring_free_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	u8       ring_type;
#define RING_FREE_REQ_RING_TYPE_L2_CMPL   0x0UL
#define RING_FREE_REQ_RING_TYPE_TX        0x1UL
#define RING_FREE_REQ_RING_TYPE_RX        0x2UL
	u8       unused_0;
	__le16   ring_id;
	u8       unused_1[4];
};

/* hwrm_ring_grp_alloc_input (size:192b/24B) */
struct hwrm_ring_grp_alloc_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le16   cr;
	__le16   rr;
	__le16   ar;
	__le16   sc;
};

/* hwrm_ring_grp_alloc_output (size:128b/16B) */
struct hwrm_ring_grp_alloc_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le32   ring_group_id;
	u8       unused_0[3];
	u8       valid;
};

/* hwrm_ring_grp_free_input (size:192b/24B) */
struct hwrm_ring_grp_free_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   ring_group_id;
	u8       unused_0[4];
};

/* hwrm_cfa_l2_filter_alloc_input (size:768b/96B) */
struct hwrm_cfa_l2_filter_alloc_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   flags;
#define CFA_L2_FILTER_ALLOC_REQ_FLAGS_PATH_RX             0x1UL
	__le32   enables;
#define CFA_L2_FILTER_ALLOC_REQ_ENABLES_L2_ADDR             0x1UL
#define CFA_L2_FILTER_ALLOC_REQ_ENABLES_L2_ADDR_MASK        0x2UL
#define CFA_L2_FILTER_ALLOC_REQ_ENABLES_DST_ID              0x8000UL
	u8       l2_addr[6];
	u8       unused_0[2];
	u8       l2_addr_mask[6];
	__le16   l2_ovlan;
	__le16   l2_ovlan_mask;
	__le16   l2_ivlan;
	__le16   l2_ivlan_mask;
	u8       unused_1[2];
	u8       t_l2_addr[6];
	u8       unused_2[2];
	u8       t_l2_addr_mask[6];
	__le16   t_l2_ovlan;
	__le16   t_l2_ovlan_mask;
	__le16   t_l2_ivlan;
	__le16   t_l2_ivlan_mask;
	u8       src_type;
#define CFA_L2_FILTER_ALLOC_REQ_SRC_TYPE_NPORT 0x0UL
	u8       unused_3;
	__le32   src_id;
	u8       tunnel_type;
	u8       unused_4;
	__le16   dst_id;
	__le16   mirror_vnic_id;
	u8       pri_hint;
	u8       unused_5;
	__le32   unused_6;
	__le64   l2_filter_id_hint;
};

/* hwrm_cfa_l2_filter_alloc_output (size:192b/24B) */
struct hwrm_cfa_l2_filter_alloc_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le64   l2_filter_id;
	__le32   flow_id;
	u8       unused_0[3];
	u8       valid;
};

/* hwrm_cfa_l2_filter_free_input (size:192b/24B) */
struct hwrm_cfa_l2_filter_free_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le64   l2_filter_id;
};

/* hwrm_cfa_l2_set_rx_mask_input (size:448b/56B) */
struct hwrm_cfa_l2_set_rx_mask_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   vnic_id;
	__le32   mask;
#define CFA_L2_SET_RX_MASK_REQ_MASK_MCAST               0x2UL
#define CFA_L2_SET_RX_MASK_REQ_MASK_ALL_MCAST           0x4UL
#define CFA_L2_SET_RX_MASK_REQ_MASK_BCAST               0x8UL
#define CFA_L2_SET_RX_MASK_REQ_MASK_PROMISCUOUS         0x10UL
	__le64   mc_tbl_addr;
	__le32   num_mc_entries;
	u8       unused_0[4];
	__le64   vlan_tag_tbl_addr;
	__le32   num_vlan_tags;
	u8       unused_1[4];
};

/* hwrm_stat_ctx_alloc_input (size:256b/32B) */
struct hwrm_stat_ctx_alloc_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le64   stats_dma_addr;
	__le32   update_period_ms;
	u8       stat_ctx_flags;
	u8       unused_0[3];
};

/* hwrm_stat_ctx_alloc_output (size:128b/16B) */
struct hwrm_stat_ctx_alloc_output {
	__le16   error_code;
	__le16   req_type;
	__le16   seq_id;
	__le16   resp_len;
	__le32   stat_ctx_id;
	u8       unused_0[3];
	u8       valid;
};

/* hwrm_stat_ctx_free_input (size:192b/24B) */
struct hwrm_stat_ctx_free_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le32   stat_ctx_id;
	u8       unused_0[4];
};

/* hwrm_nvm_flush_input (size:128b/16B) */
struct hwrm_nvm_flush_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
};

/* hwrm_nvm_get_variable_input (size:320b/40B) */
struct hwrm_nvm_get_variable_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le64   dest_data_addr;
	__le16   data_len;
	__le16   option_num;
	__le16   dimensions;
	__le16   index_0;
	__le16   index_1;
	__le16   index_2;
	__le16   index_3;
	u8       flags;
	u8       unused_0;
};

/* hwrm_nvm_set_variable_input (size:320b/40B) */
struct hwrm_nvm_set_variable_input {
	__le16   req_type;
	__le16   cmpl_ring;
	__le16   seq_id;
	__le16   target_id;
	__le64   resp_addr;
	__le64   src_data_addr;
	__le16   data_len;
	__le16   option_num;
	__le16   dimensions;
	__le16   index_0;
	__le16   index_1;
	__le16   index_2;
	__le16   index_3;
	u8       flags;
	u8       unused_0;
};

#endif /* _BNXT_HSI_H_ */
