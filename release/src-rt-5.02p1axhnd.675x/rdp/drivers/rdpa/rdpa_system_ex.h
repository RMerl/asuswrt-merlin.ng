/* 
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

#ifndef _RDPA_SYSTEM_EX_H_
#define _RDPA_SYSTEM_EX_H_
/* system object private data */
typedef struct
{
    rdpa_sw_version_t sw_version;
    rdpa_system_init_cfg_t init_cfg;
    rdpa_system_cfg_t cfg;
    rdpa_qm_cfg_t qm_cfg;
    rdpa_counter_cfg_t counter_cfg;
    rdpa_packet_buffer_cfg_t packet_buffer_cfg;
    uint16_t dp_bitmask[2];
    rdpa_tpid_detect_cfg_t tpids_detect[rdpa_tpid_detect__num_of];
    rdpa_cpu_tc  high_prio_tc_thresh;
} system_drv_priv_t;

int system_attr_tod_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_dp_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index);
int system_attr_dp_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_dp_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, 
    void *val, uint32_t size);
int system_attr_debug_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_attr_debug_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_pre_init_ex(struct bdmf_object *mo);
int system_post_init_ex(struct bdmf_object *mo);
int system_attr_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, 
    const void *val, uint32_t size);
int system_attr_clock_gate_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_attr_clock_gate_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);

int system_data_path_init(void);
#if defined(__OREN__) || defined(XRDP)
int system_data_path_init_fiber(rdpa_wan_type wan_type);
#endif
#ifdef XRDP
int system_data_path_init_gbe(rdpa_emac wan_emac);
#endif

#ifdef BCM63158
int system_data_path_init_dsl(void);
#endif

int _tpid_detect_cfg(struct bdmf_object * const mo, rdpa_tpid_detect_t tpid_detect,
    const rdpa_tpid_detect_cfg_t * const tpid_detect_cfg);
int system_post_init_enumerate_emacs(struct bdmf_object *mo);

/** System-level drop statistics. */
#ifdef XRDP

typedef struct
{
    uint32_t protocol_us_ipv4;                  /**< Drop due to IPV4 protocol filter  */
    uint32_t protocol_us_ipv6;                  /**< Drop due to IPV6 protocol filter */
    uint32_t protocol_us_pppoe;                 /**< Drop due to PPPOE protocol filter */
    uint32_t protocol_us_non_ip;                /**< Drop due to none IP protocol filter */
    uint32_t mirror_us_no_sbpm;                 /**< Drop due to no SBPM for mirroring */
    uint32_t mirror_us_no_dispatch_token;       /**< Drop due to no dispatcher token for mirroring */
} rdpa_system_us_stat_t;

typedef struct
{
    uint32_t connection_action;                 /**< Connection action == drop */
    uint32_t cpu_rx_ring_congestion;            /**< CPU RX Ring congestion */
    uint32_t cpu_rx_qm_queue_drop_norm;         /**< Drop in QM for CPU RX normal queue*/
    uint32_t cpu_rx_qm_queue_drop_excl;         /**< Drop in QM for CPU RX exclusive queue*/
    uint32_t cpu_recycle_ring_congestion;       /**< CPU Recycle Ring congestion */
    uint32_t cpu_tx_copy_no_fpm;                /**< Drop due to no FPM when CPU TX copy */
    uint32_t cpu_tx_copy_no_sbpm;               /**< Drop due to no SBPM when CPU TX copy */
    uint32_t flow_drop_action;                  /**< Flow action = drop */
    uint32_t rx_mirror_cpu_mcast_exception;     /**< Drop due to RX mirroring or CPU/WLAN MCAST exception */
    uint32_t cpu_rx_meter_drop;                 /**< Counts total packets number dropped by cpu rx meters */
    uint32_t ingress_resources_congestion;      /**< Drop due to XRDP resources congestion ingress */
    uint32_t egress_resources_congestion;       /**< Drop due to XRDP resources congestion egress */
    uint32_t ingress_isolation_drop;            /**< Ingress VLAN isolation drop */
    uint32_t egress_isolation_drop;             /**< Egress VLAN isolation drop */
    uint32_t disabled_tx_flow;                  /**< TX flow is not defined */
    uint32_t cpu_rx_tc_to_rxq_map;              /**< Drop due to CPU RX TC to RXQ map */
    uint32_t cpu_rx_vport_to_cpu_obj_map;       /**< Drop due to CPU RX vport to CPU object map */
    uint32_t da_lookup_miss;                    /**< Drop due to Bridge ARL miss on DA MAC lookup */
    uint32_t sa_lookup_miss;                    /**< Drop due to Bridge ARL miss on SA MAC lookup */
    uint32_t bridge_fw_eligability;             /**< Drop due to ingress and egress ports don't belong to the same bridge */
    uint32_t da_lookup_match_drop;              /**< Drop due to DA lookup miss no matched entry */
    uint32_t sa_lookup_match_drop;              /**< Drop due to SA lookup miss no matched entry */
    uint32_t cpu_tx_disabled_q_drop;            /**< Drop due to disabled queue */
#if defined(G9991)
    uint32_t g9991_sof_after_sof;               /**< DPU drop due to start of packet (SOF) received after SOF */
    uint32_t g9991_mof_eof_without_sof;         /**< DPU drop due to received end or middle of packet without start of packet */
    uint32_t g9991_reassembly_error;            /**< DPU drop due to re-assembly error */
#endif
    uint32_t ingress_rate_limit_drop;           /**< Drop when ingress_rate limit emac port exceeded */
    uint32_t loopback_drop;                     /**< Drop when packet in this flow is not loopbacked */
    uint32_t undefined_queue_drop;              /**< Drop flow control on emac port */
    uint32_t qm_wred_drop;                      /**< Total drops from all queues due to WRED */
    uint32_t qm_fpm_cong_pool_drop[4];          /**< Drop due to FPM pool priority thresholds violation, counts per buffer's size pools (x1, x2, x4,x8) */
    uint32_t qm_fpm_cong;                       /**< Drop due to FPM congestion */
    uint32_t qm_fpm_grp_drop[4];                /**< Drop due to FPM user group priority threshold violation. Per user group */
    uint32_t qm_ddr_pd_cong_drop;               /**< Drop due to DDR PD congestion */
    uint32_t qm_pd_cong_drop;                   /**< Drop due to PD congestion */
    uint32_t qm_psram_egress_cong;              /**< Drop due to PSSRAM egress congestion */
#if !defined(BCM63158)
    uint32_t tx_mirroring_drop;                 /**< Drop tx mirrored packet due to no dispatcher token for mirroring */
    uint32_t tunnel_no_sbpm_drop;               /**< Drop US tunnel packet, no SBPM packet */
#endif
} rdpa_system_common_stat_t;

typedef struct
{
    uint32_t protocol_ds_ipv4;                  /**< Drop due to IPV4 protocol filter  */
    uint32_t protocol_ds_ipv6;                  /**< Drop due to IPV6 protocol filter */
    uint32_t protocol_ds_pppoe;                 /**< Drop due to PPPOE protocol filter */
    uint32_t protocol_ds_non_ip;                /**< Drop due to none IP protocol filter */
    uint32_t mirror_ds_no_sbpm;                 /**< Drop due to no SBPM for mirroring */
    uint32_t mirror_ds_no_dispatch_token;                /**< Drop due to no dispatcher token for mirroring */
} rdpa_system_ds_stat_t;

#else

/** US system-level drop statistics. */
typedef struct
{
    uint16_t eth_flow_action;                   /**< Flow action = drop */
    uint16_t sa_lookup_failure;                 /**< SA lookup failure */
    uint16_t da_lookup_failure;                 /**< DA lookup failure */
    uint16_t sa_action;                         /**< SA action == drop */
    uint16_t da_action;                         /**< DA action == drop */
    uint16_t forwarding_matrix_disabled;        /**< Disabled in forwarding matrix */
    uint16_t connection_action;                 /**< Connection action == drop */
    uint16_t parsing_exception;                 /**< Parsing exception */
    uint16_t parsing_error;                     /**< Parsing error */
    uint16_t local_switching_congestion;        /**< Local switching congestion */
    uint16_t vlan_switching;                    /**< VLAN switching */
    uint16_t tpid_detect;                       /**< Invalid TPID */
    uint16_t invalid_subnet_ip;                 /**< Invalid subnet */
    uint16_t acl_oui;                           /**< Dropped by OUI ACL */
    uint16_t acl;                               /**< Dropped by ACL */
} rdpa_system_us_stat_t;

/** DS system-level drop statistics */
typedef struct
{
    uint16_t eth_flow_action;                   /**< Flow action = drop */
    uint16_t sa_lookup_failure;                 /**< SA lookup failure */
    uint16_t da_lookup_failure;                 /**< DA lookup failure */
    uint16_t sa_action;                         /**< SA action == drop */
    uint16_t da_action;                         /**< DA action == drop */
    uint16_t forwarding_matrix_disabled;        /**< Disabled in forwarding matrix */
    uint16_t connection_action;                 /**< Connection action == drop */
    uint16_t policer;                           /**< Dropped by policer */
    uint16_t parsing_exception;                 /**< Parsing exception */
    uint16_t parsing_error;                     /**< Parsing error */
    uint16_t iptv_layer3;                       /**< IPTV L3 drop */
    uint16_t vlan_switching;                    /**< VLAN switching */
    uint16_t tpid_detect;                       /**< Invalid TPID */
    uint16_t dual_stack_lite_congestion;        /**< DSLite congestion */
    uint16_t invalid_subnet_ip;                 /**< Invalid subnet */
    uint16_t invalid_layer2_protocol;           /**< Invalid L2 protocol */
    uint16_t firewall;                          /**< Dropped by firewall */
    uint16_t dst_mac_non_router;                /**< DST MAC is not equal to the router's MAC */
} rdpa_system_ds_stat_t;
#endif /* !XRDP */

typedef struct
{
    uint32_t tm_pd_not_valid_id;                 /**< TM PD not valid */
    uint32_t tm_action_not_valid_id;             /**< TM action not valid */
    uint32_t epon_tm_pd_not_valid_id;           /**< Epon TM PD not valid */
    uint32_t g9991_tm_pd_not_valid_id;          /**< DPU TM PD not valid */
    uint32_t processing_action_not_valid_id;    /**< Processing action not valid */
    uint32_t sbpm_lib_disp_cong;                /**< SBPM LIB Dispatcher Congestion */
    uint32_t bridge_flooding;                   /**< Bridge flooding */
    uint32_t ingress_congestion_flow_cntr_lan;  /**< Drop Ingress_congestion pause frame requests on lan */
} rdpa_debug_stat_t;

typedef struct
{
    rdpa_system_us_stat_t us;                   /**< Upstream drop statistics */
    rdpa_system_ds_stat_t ds;                   /**< Downstream drop statistics */
#ifdef XRDP
    rdpa_system_common_stat_t common;           /**< Common drop statistic */
#endif
} rdpa_system_stat_t;

int system_attr_cpu_reason_to_tc_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_cpu_reason_to_tc_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
/* "counter_cfg" attribute "write" callback */
int system_counter_cfg_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_attr_fpm_isr_delay_timer_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_fpm_isr_delay_timer_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_attr_natc_counter_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_natc_counter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_attr_ih_cong_threshold_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_ih_cong_threshold_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int system_attr_ing_cong_ctrl_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int system_attr_ing_cong_ctrl_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int rdpa_system_tpid_idx_get(uint16_t tpid_val);
uint16_t rdpa_system_tpid_get_by_idx(int tpid_idx);
int _packet_buffer_cfg(const rdpa_packet_buffer_cfg_t *pb_cfg);
void rdpa_system_set_global_token_allocation(const rdpa_packet_buffer_cfg_t *pb_cfg);
void rdpa_system_set_global_token_allocation_ug_on(uint8_t enable);

#define NUM_TABLE_ENTRIES_256  256
#define NUM_TABLE_ENTRIES_512  512
#define NUM_TABLE_ENTRIES_1024 1024

#endif

