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

#ifndef _RDPA_INT_H_
#define _RDPA_INT_H_

#include "rdpa_common.h"
#include "bdmf_dev.h"
#include "rdpa_system.h"
#ifdef LEGACY_RDP
#include "rdpa_rdd_map_legacy.h"
#else
#include <rdd_stubs.h>
#ifndef XRDP
#include <rdpa_stubs.h>
#include "rdpa_rdd_map.h"
#else
#include "rdd.h"
#include "rdpa_vlan_action.h"
#include "rdpa_tcont.h"
#endif
#include "rdd_data_structures_auto.h"
#endif
#ifdef XRDP
#include "rdp_drv_proj_cntr.h"
#endif
#include "rdpa_egress_tm.h"

/* default transparent vlan action entrance */
#define RDPA_DS_TRANSPARENT_VLAN_ACTION (RDPA_MAX_VLAN_ACTION - 1)
#define RDPA_US_TRANSPARENT_VLAN_ACTION (RDPA_MAX_VLAN_ACTION - 1)

/**< Max number of US ingress classification results */
#define RDPA_MAX_US_IC_RESULTS RDD_US_IC_CONTEXT_TABLE_SIZE
/**< Max number of DS ingress classification results */


#ifndef XRDP
#define RDD_DS_IC_CONTEXT_TABLE_SIZE   RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE
#define RDPA_MAX_DS_IC_RESULTS         RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE
#define RDPA_UNMATCHED_DS_IC_RESULT_ID (RDPA_MAX_DS_IC_RESULTS - 1)
#define RDPA_UNMATCHED_US_IC_RESULT_ID (RDPA_MAX_US_IC_RESULTS - 1)
/* Reserved ingress classification result for FC */
#define RDPA_FC_DS_IC_RESULT_ID (RDPA_MAX_DS_IC_RESULTS - 2)
#else
#define RDPA_MAX_IC_SHARED_RESULTS     (RDD_IC_SHARED_CONTEXT_TABLE_SIZE + RDD_DEFAULT_FLOW_CONTEXT_TABLE_SIZE + RDD_IPTV_DDR_CONTEXT_TABLE_SIZE)
#define RDPA_UNMATCHED_DS_IC_RESULT_ID 0xFFF
#endif /* XRDP */

#if defined(G9991) && defined(__OREN__)
#define G9991_SID_PORTS_DS 25
#define RDPA_USER_MAX_DS_IC_RESULTS (RDPA_MAX_DS_IC_RESULTS - 2 - G9991_SID_PORTS_DS)
#else

#ifndef XRDP
#define RDPA_USER_MAX_DS_IC_RESULTS (RDPA_MAX_DS_IC_RESULTS - 2)
#else
#define RDPA_USER_MAX_DS_IC_RESULTS (RDPA_MAX_IC_SHARED_RESULTS)
#endif
#endif
#ifndef XRDP
#define RDPA_USER_MAX_US_IC_RESULTS (RDPA_MAX_US_IC_RESULTS - 1)
#else
#define RDPA_USER_MAX_US_IC_RESULTS (RDPA_MAX_IC_SHARED_RESULTS)
#endif

typedef enum {
    rdpa_flow_ic_type = 0,
    rdpa_flow_iptv_type = 1,
    rdpa_flow_def_flow_type = 2,
} rdpa_flow_types;

#define MS_BYTE_TO_8_BYTE_RESOLUTION(address)  ((address) >> 3)

/* These constants define the min/max packet size selection indices */
#define RDPA_BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX 0
#define RDPA_BBH_RX_OMCI_MIN_PKT_SIZE_SELECTION_INDEX 1
#define RDPA_BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX 0
#define RDPA_BBH_RX_OMCI_MAX_PKT_SIZE_SELECTION_INDEX 1

/* IH classifier indices for broadcast and multicast traffic iptv destination */
#define RDPA_IH_CLASSIFIER_BCAST_IPTV 0
#define RDPA_IH_CLASSIFIER_IGMP_IPTV 1
#define RDPA_IH_CLASSIFIER_ICMPV6 2
#define RDPA_IH_CLASSIFIER_ROUTING_PROTO 3
#define RDPA_IH_CLASSIFIER_L2_PROTOCOL_UDEF_2_3  (RDPA_IH_CLASSIFIER_ROUTING_PROTO + 1)
#define RDPA_IH_CLASSIFIER_IPTV  (RDPA_IH_CLASSIFIER_L2_PROTOCOL_UDEF_2_3 + 1)

/* IPTV DA filter mask in IH */
#define RDPA_IPTV_IH_IP_FILTERS_NUM (RDPA_IH_IP_FILTERS_NUM-1)
#define RDPA_IPTV_FILTER_MASK_DA 0x3800
#define RDPA_IPTV_FILTER_MASK_BCAST  0x8000
#define RDPA_MASK_IH_CLASS_KEY_L4 0x3c0
#define RDPA_MASK_IH_CLASS_L2_UDEF_2_3 0xe
#define RDPA_IPTV_FILTER_MASK_IP_FILTER_ANYHIT  0x800000
#define RDPA_IH_DA_FILTER_IPTV_IPV4 0
#define RDPA_IH_DA_FILTER_IPTV_IPV6 1

bdmf_boolean rdpa_is_gpon_or_xgpon_mode(void);
bdmf_boolean rdpa_is_epon_or_xepon_mode(void);
rdpa_epon_mode _rdpa_epon_mode_get(void);
bdmf_boolean is_rdpa_epon_ctc_or_cuc_mode(void);
bdmf_boolean rdpa_is_epon_ae_mode(void);
bdmf_boolean rdpa_is_gbe_mode(void);
bdmf_boolean rdpa_is_dsl_mode(void);
bdmf_boolean rdpa_is_fttdp_mode(void);
bdmf_boolean rdpa_is_ext_switch_mode(void);
rdpa_emac rdpa_gbe_wan_emac(void);
bdmf_boolean rdpa_is_car_mode(void);
bdmf_boolean rdpa_ic_dbg_stats_enabled(void);
bdmf_boolean rdpa_if_is_active(rdpa_if port);
bdmf_boolean rdpa_is_ddr_offload_enable(rdpa_traffic_dir dir);
rdpa_bpm_buffer_size_t rdpa_bpm_buffer_size_get(void);
/*
 * egress_tm internal interface
 */
/* Set/Unset egress tm channel (e.g. tcont, wan/lan port, etc.). */
typedef struct
{
    bdmf_boolean fec_overhead;
    bdmf_boolean q_802_1ae;
    bdmf_boolean sci_overhead;
} channel_attr;

int _rdpa_egress_tm_channel_set(bdmf_object_handle tm_obj, bdmf_object_handle channel, rdpa_wan_type wan_type, int16_t channel_id);
int _rdpa_egress_tm_channel_group_set(bdmf_object_handle tm_obj, bdmf_object_handle owner, rdpa_wan_type wan_type,
    int group_id, int num_channels, const int16_t *channels);
int _rdpa_egress_tm_enable_set(bdmf_object_handle tm_obj, bdmf_boolean enable, bdmf_boolean flush);
int _rdpa_egress_tm_orl_prty_set(bdmf_object_handle tm_obj, rdpa_tm_orl_prty orl_prty);
int _rdpa_egress_tm_channel_queue_to_rdd(rdpa_traffic_dir dir, int channel, uint32_t queue_id,
    int *rc_id, int *queue);
bdmf_error_t _rdpa_egress_tm_tx_queue_drop_stat_read(int rdp_queue_index, rdpa_stat_1way_t *stat);
int egress_tm_svcq_queue_index_get(uint32_t queue_id);
int egress_tm_svcq_queue_id_get(uint32_t queue_index);
bdmf_error_t _rdpa_egress_tm_set_channel_attr(bdmf_object_handle tm_obj, channel_attr *attr, int channel_id);

/* wan_flow remains unchanged for GPON and GbE.
 * In case of EPON, channel_id is passed on input and llid_index returned on output
 */
int _rdpa_egress_tm_queue_id_by_wan_flow_rc_queue(int *wan_flow, int rc_id, int queue, uint32_t *queue_id);

int _rdpa_egress_tm_queue_id_by_channel_rc_queue(int channel_id, int rc_id, int queue, uint32_t *queue_id);
int _rdpa_egress_tm_queue_id_by_lan_port_queue(rdpa_if port, int queue, uint32_t *queue_id);
int _rdpa_egress_tm_channel_queue_enable_set(bdmf_object_handle tm_obj, int channel_id, uint32_t queue_index, bdmf_boolean enable);
int _rdpa_egress_tm_channel_enable_set(bdmf_object_handle tm_obj, int channel_id, bdmf_boolean enable);

#ifdef XRDP
int _rdpa_egress_tm_queue_id_by_wan_flow_qm_queue(int *wan_flow, int qm_queue, uint32_t *queue_id);
int _rdpa_egress_tm_queue_id_by_wan_flow_port_qm_queue(int *wan_flow, rdpa_if port, int qm_queue, uint32_t *queue_id);
int _rdpa_egress_tm_queue_id_by_lan_port_qm_queue(rdpa_if port, int qm_queue, uint32_t *queue_id);
#endif

/*
 * gpon driver internal interface
 */
bdmf_boolean _ds_gem_flow_check(bdmf_index gem_flow);
int _cfg_ds_gem_flow_hw(bdmf_boolean cfg_gem, bdmf_index gem_flow, uint16_t gem_port,
    rdpa_flow_destination destination, rdpa_discard_prty discard_prty,
    bdmf_index ds_def_flow);

/*
 * tcont driver internal interface
 */
int rdpa_tcont_channel_get(bdmf_object_handle mo, bdmf_number *channel_index);

int rdpa_tcont_is_mgmt(bdmf_object_handle mo);

/*
 * llid driver internal interface
 */
int rdpa_llid_queue_id_to_channel_id(int llid_id, int queue_id, int *channel_id);

/*
 * xtm driver internal interface
 */
int _cfg_ds_xtm_channel(bdmf_index channel_idx);

/* Map rdpa tcont index to rdd wan channel; tcont index shifted by 1 on gpon so tcont 1-8 can use 32 RC's */
static inline int _rdd_wan_channel(int rdpa_tcont_index)
{
    if (!rdpa_is_gpon_or_xgpon_mode())
       return rdpa_tcont_index;
    if (rdpa_tcont_index)
        return rdpa_tcont_index - 1;
    return RDPA_MAX_TCONT - 1;
}

/*
 * * __bitcount - MIT Hackmem count implementation which is O(1)
 * * Available from multiple sites under public domain and BSD-like licenses and originally reportedly found at
 * * http://infolab.stanford.edu/~manku/bitcount/bitcount.html
 * * Counts the number of 1s in a given unsigned int n
 * */
static inline int __bitcount(uint32_t n)
{
    uint32_t tmp;
    tmp = n - ((n >> 1) & 033333333333)
        - ((n >> 2) & 011111111111);
    return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}

int rdpa_map_from_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t  *result,
    rdd_ic_context_t *context, bdmf_boolean qos);
int rdpa_map_to_rdd_classifier(rdpa_traffic_dir dir, rdpa_ic_result_t *result,
    rdd_ic_context_t *context, bdmf_boolean iptv,
    bdmf_boolean is_init, rdpa_ic_type ic_type, bdmf_boolean skip_tm);
/* The function returns TCONT ID associated with given GEM flow. */
int rdpa_gem_flow_id_to_tcont_id(int gem_flow, int *tcont_id);
int rdpa_gem_flow_id_to_tcont_channel_id(int gem_flow, int *channel_id);

/* The function returns channel ID associated with given XTM flow. */
int rdpa_xtm_flow_id_to_channel_id(int xtm_flow, int *channel_id);

#ifndef XRDP
/* Get CPU statistics without clearing them */
int _rdpa_cpu_stat_get(rdpa_cpu_port port, rdpa_stat_tx_rx_t *stat);

/* Get forward eligibility matrix for source port */
rdpa_ports rdpa_bridge_fw_eligible(rdpa_if src_port);
void _rdpa_bridge_disable_lan_vid_aggregation(int vid_entry);
/* Update aggregation bit in all MAC entries related to a given port */
void _rdpa_bridge_update_aggregation_in_mac_table(rdd_bridge_port_t port, bdmf_boolean aggregation_enabled);
/* Handle the RDD LAN VID entry */
int _rdpa_handle_rdd_lan_vid_entry(uint16_t vid, bdmf_boolean is_add, void *const params_v, uint8_t *entry);
#else
int rdpa_cntr_id_alloc(uint32_t group_id, uint32_t *cntr_id);
void rdpa_cntr_id_dealloc(int group_id, cntr_sub_group_id_t sub_group, uint32_t entry_idx);
#endif /* XRDP */
int _rdpa_bridge_update_sa_miss_action(bdmf_object_handle bridge_obj);
int _rdpa_bridge_check_port_fdb_limit(bdmf_object_handle bridge_obj, rdpa_if port, rdpa_port_sa_limit_t *sa_limit);

/* Get/Put classification context index */
#ifdef XRDP
int classification_ctx_index_get(rdpa_traffic_dir dir, rdpa_flow_types flow_type, int *ctx_idx);
#else
int classification_ctx_index_get(rdpa_traffic_dir dir, bdmf_boolean is_iptv, int *index);
#endif
void classification_ctx_index_put(rdpa_traffic_dir dir, int index);

unsigned long rdpa_get_switch_lag_port_mask(void);
int port_action_write(bdmf_index rule, bdmf_object_handle vlan_action, rdpa_if port, bdmf_boolean drop);
int port_action_read(bdmf_index rule, bdmf_object_handle *vlan_action, rdpa_if port, bdmf_boolean *drop);

int _rdpa_tunnel_ref_count_increase(struct bdmf_object *mo);
int _rdpa_tunnel_ref_count_decrease(struct bdmf_object *mo);

bdmf_boolean is_lag_config_done(void);

bdmf_boolean is_sa_mac_use(void);
bdmf_boolean is_triple_tag_detect(void);
void sa_mac_use_count_up(void);
void sa_mac_use_count_down(void);

int _rdpa_system_num_wan_get(void);
int _rdpa_system_num_lan_get(void);
int _rdpa_system_resources_get(rdpa_system_resources_t *sys_res);
const rdpa_system_init_cfg_t *_rdpa_system_init_cfg_get(void);
const rdpa_qm_cfg_t *_rdpa_system_qm_cfg_get(void);
const rdpa_counter_cfg_t *_rdpa_system_counter_cfg_get(void);
rdpa_packet_buffer_cfg_t *_rdpa_system_packet_buffer_cfg_get(void);
const rdpa_cpu_tc _rdpa_system_high_prio_tc_thr_get(void);
const rdpa_system_cfg_t *_rdpa_system_cfg_get(void);
void _rdpa_system_gbe_wan_emac_set(rdpa_emac wan_emac);
rdpa_speed_type rdpa_wan_speed_get(rdpa_if if_);
rdpa_wan_type rdpa_wan_if_to_wan_type(rdpa_if wan_if);
void epon_get_mode(rdpa_epon_mode * const mode);
bdmf_object_handle _rdpa_system_get(void);
bdmf_error_t common_timer_init(void);
#ifdef BCM_PON_XRDP
int bbh_tx_reconfig(bbh_id_e bbh_id_gbe_wan, bbh_id_e bbh_id);
#endif

typedef union {
    struct {
#if defined(_BYTE_ORDER_LITTLE_ENDIAN_)
        uint32_t rdd_flow_id:24;
        uint32_t flow_type:3; 
        uint32_t unused:5;
#else
        uint32_t unused:5;
        uint32_t flow_type:3;
        uint32_t rdd_flow_id:24;
#endif
    };
    uint32_t word;
} rdpa_flow_id_t;

static inline uint8_t rdpa_flow_get_type(uint32_t rdpa_flow_id) 
{ 
    return ((rdpa_flow_id_t *)&rdpa_flow_id)->flow_type;
}
static inline uint32_t rdpa_flow_get_rdd_id(uint32_t rdpa_flow_id) 
{ 
    return ((rdpa_flow_id_t *)&rdpa_flow_id)->rdd_flow_id;
}
static inline uint32_t rdpa_build_flow_id(uint32_t rdd_flow_id, uint8_t flow_type) 
{ 
    rdpa_flow_id_t rdpa_flow_id = {.word = 0 };
    rdpa_flow_id.flow_type = flow_type;
    rdpa_flow_id.rdd_flow_id = rdd_flow_id;
    return rdpa_flow_id.word;
}

typedef union
{
    struct {
        bdmf_mac_t sa;
        bdmf_mac_t da;
        uint32_t vtag0;
        uint32_t vtag1;
        uint8_t vtag_num;
        uint16_t eth_type;
    } l2;
    struct {
        bdmf_ip_t sip;
        bdmf_ip_t dip;
    } l3;
} flow_display_info_t;

#endif /* _RDPA_INT_H_ */
