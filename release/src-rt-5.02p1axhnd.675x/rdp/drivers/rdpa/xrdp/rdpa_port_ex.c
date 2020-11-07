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
 :>
*/

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd_stubs.h"
#include "rdd_init.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_port_int.h"
#include "rdpa_cpu_ex.h"
#include "rdpa_vlan_ex.h"
#include "rdpa_bridge_ex.h"
#if defined(BCM63158)
#include "rdpa_xtm_ex.h"
#endif
#include "rdpa_ingress_class_int.h"
#include "rdd_ghost_reporting.h"
#include "rdd.h"
#include "rdp_drv_bbh_tx.h"
#include "rdp_drv_bbh_rx.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_qm.h"
#include "rdp_drv_cnpl.h"
#include "rdp_drv_dis_reor.h"
#include "rdd_data_structures_auto.h"
#include "rdd_runner_proj_defs.h"
#include "rdp_drv_proj_cntr.h"
#include "rdd_scheduling.h"
#include "rdd_mirroring.h"
#include "rdd_cpu_rx.h"
#include "rdd_ag_processing.h"
#if !defined(BCM63158)
#include "rdd_bridge.h"
#endif
#ifdef INGRESS_FILTERS
#include "rdpa_filter_ex.h"
#endif
#ifdef FLOW_CTRL_SUPPORT
#include "rdd_ag_timer_common.h"
#endif
#ifdef G9991
#include "rdd_ag_reas.h"
#endif

/* we support only one mirror output port to all rx ports */
/* meanwhile for rx: mirroring enable from lan/wan ports to single lan port */
/* for tx mirroring enable for wan to lan only */
typedef struct
{
    uint32_t rx_ref_cntr; /* reference counter of rx mirrored ports */
    rdpa_port_mirror_cfg_t port_mirror_cfg; /* mirroring lan for rx/tx */
    uint32_t tx_ref_cntr; /* reference counter of tx mirrored ports  */
    /* temporary: should be deleted when tx mirroring of sbpm packets is available */
    int32_t tx_mirror_copy_to_ddr_prev[rdpa_if__number_of];
} mirroring_cfg_t;

static mirroring_cfg_t mirroring_cfg = { .rx_ref_cntr = 0, .tx_ref_cntr = 0,
    .port_mirror_cfg.rx_dst_port = NULL, .port_mirror_cfg.tx_dst_port = NULL };

static rdpa_port_debug_stat_t  accumulative_port_debug_stat[rdpa_if__number_of];
static rdpa_port_pkt_size_stat_t accumulate_port_pkt_size_stat[rdpa_if__number_of];
#ifndef BCM63158
static bdmf_boolean global_mac_addr_idx_is_set[RDPA_PORT_MAX_MAC] = {1, 0, 0, 0, 0, 0, 0, 0}; /* Index 0 is saved to default MAC addr 00:00:00:00:00:00 */
#endif

extern rdd_bb_id rdpa_emac_to_bb_id_rx[rdpa_emac__num_of];
extern rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of];
extern rdd_vport_id_t rx_flow_to_vport[RX_FLOW_CONTEXTS_NUMBER];
extern rdpa_port_stat_t accumulative_port_stat[rdpa_if__number_of];
extern rdpa_emac rdpa_if_to_port_emac_map[rdpa_if__number_of];
extern bbh_id_e rdpa_emac_to_bbh_id_e[rdpa_emac__num_of];

extern void rdpa_rx_def_flow_rdd_ic_context_idx_get(bdmf_index index, RDD_RULE_BASED_CONTEXT_ENTRY_DTS *entry,
    uint16_t *ctx_idx);
rdd_vport_id_t rdpa_if_to_rdd_vport(rdpa_if port, rdpa_wan_type wan_type);
extern void _rdpa_port_emac_set(rdpa_emac emac, int val);
extern rdpa_if _rdpa_port_emac_to_rdpa_if(rdpa_emac emac);
extern void _rdpa_port_rdpa_if_to_emac_set(rdpa_if port, bdmf_boolean val);
static int _validate_port_index(rdpa_if index);
static int read_stat_from_hw(port_drv_priv_t *port, rdd_vport_pm_counters_t *rdd_port_counters,
    bbh_rx_counters_t *bbh_rx_counters, bbh_tx_counters_t *bbh_tx_counters);

#define USE_CNPL_HW_COUNTER(port) (rdpa_if_is_lan_or_cpu(port) || (rdpa_wan_if_to_wan_type(port) == rdpa_wan_gbe) || \
    (rdpa_if_is_wan(port) && rdpa_is_epon_ae_mode()))
#define FW_LOOPBACK_EN(cfg_ptr) ((cfg_ptr)->op == rdpa_loopback_op_local && (cfg_ptr)->type == rdpa_loopback_type_fw)

#define BBH_TIMER_MIN (BBH_FREQUENCY / 1000) /* freq * 1ms */
#define _OFFSET_(name) offsetof(rdpa_port_stat_t, name) /* find offset of variable in structure rdpa_port_stat_t*/
#define _OFFSET_DBG_(name) offsetof(rdpa_port_debug_stat_t, name) /* find offset of variable in structure rdpa_port_debug_stat_t*/
#define _OFFSET_PKT_SZ_(name) offsetof(rdpa_port_pkt_size_stat_t, name) /* find offset of variable in structure rdpa_port_pkt_size_stat_t*/

static int port_rdd_mirror_cfg(port_drv_priv_t *port, rdpa_port_mirror_cfg_t *mirror_cfg, rdd_mirroring_cfg_t *rdd_mirroring_cfg);
static bbh_id_e rdpa_port_to_bbh_id(port_drv_priv_t *port);

static int _port_mirror_cfg_tx(port_drv_priv_t *port, rdpa_port_mirror_cfg_t *mirror_cfg, rdd_mirroring_cfg_t *rdd_mirroring_cfg);
static int _port_mirror_cfg_rx(port_drv_priv_t *port, rdpa_port_mirror_cfg_t *mirror_cfg, rdd_mirroring_cfg_t *rdd_mirroring_cfg);
static inline int _delete_mirrored_port_cfg(port_drv_priv_t *port, bdmf_boolean is_rx_mirror);

int update_port_bridge_and_vlan_lookup_method_ex(rdpa_if port)
{
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port);
    bdmf_boolean ingress_isolation_en;
    bdmf_boolean egress_isolation_en;
    int rc;
    uint8_t ingress_method = BRIDGE_AND_VLAN_CTX_LOOKUP_METHOD_VPORT_ONLY;
    uint8_t egress_method = BRIDGE_AND_VLAN_CTX_LOOKUP_METHOD_VPORT_ONLY;

    rc = rdd_ag_processing_vport_cfg_table_ingress_isolation_en_get(vport, &ingress_isolation_en);
    rc = rc ? rc : rdd_ag_processing_vport_cfg_table_egress_isolation_en_get(vport, &egress_isolation_en);
    if (rc)
       return rc;
    if (ingress_isolation_en)
    {
        ingress_method = BRIDGE_AND_VLAN_CTX_LOOKUP_METHOD_VPORT_VID;
    }
    if (egress_isolation_en)
    {
        egress_method = BRIDGE_AND_VLAN_CTX_LOOKUP_METHOD_VPORT_VID;
    }
#ifdef CONFIG_RNR_BRIDGE
    {
        bdmf_object_handle port_obj = NULL, vlan_obj = NULL;
        bdmf_boolean global_vlan_counters_enabled;

        RDD_SYSTEM_CONFIGURATION_ENTRY_VLAN_STATS_ENABLE_READ_G(global_vlan_counters_enabled, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);

        /* If port has any vlan childs (even if isolation is off for black list implementation) */
        rc = rdpa_port_get(port, &port_obj);
        if (rc)
        {
            BDMF_TRACE_RET(rc, "port object %s not found, error = %d\n", 
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), rc);
        }

        /* iterate on VLAN childs and check if any is linked to Q-bridge */
        while ((vlan_obj = bdmf_get_next_child(port_obj, "vlan", vlan_obj)) != NULL)
        {
            vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(vlan_obj);

            /* If any of VIDs is enabled, change lookup method */
            if (vlan_is_any_vid_enabled(vlan))
            {
                /* ingress_isolation is off -> black list, therefore lookup is hash even if vlan counters are off */
                ingress_method = BRIDGE_AND_VLAN_CTX_LOOKUP_METHOD_VPORT_VID;

                /* egress_isolation is off, therefore lookup is hash only if vlan counters are on */
                if (global_vlan_counters_enabled)
                    egress_method = BRIDGE_AND_VLAN_CTX_LOOKUP_METHOD_VPORT_VID;

                bdmf_put(vlan_obj);
                break;
            }
        }
        bdmf_put(port_obj);
    }
#endif

    rc = rdd_ag_processing_vport_cfg_table_bridge_and_vlan_ingress_lookup_method_set(vport, ingress_method);
    rc = rc ? rc : rdd_ag_processing_vport_cfg_table_bridge_and_vlan_egress_lookup_method_set(vport, egress_method);
    
    return rc;
}

static inline void _init_rdd_mirroring_cfg(port_drv_priv_t *port, rdd_mirroring_cfg_t *rdd_mirroring_cfg)
{
    rdd_mirroring_cfg->rx_dst_queue = QM_NUM_QUEUES + 1;
    rdd_mirroring_cfg->tx_dst_queue = QM_NUM_QUEUES + 1;
    rdd_mirroring_cfg->rx_dst_vport = 0;
    rdd_mirroring_cfg->tx_dst_vport = 0;
    
    if (!(rdpa_if_is_wlan(port->index)))
    {
        rdd_mirroring_cfg->src_tx_bbh_id = rdpa_if_to_port_emac_map[port->index];
    }
    else
    {
        rdd_mirroring_cfg->src_tx_bbh_id = BBH_ID_NULL;
    }
}

static uint32_t rdpa_port_tx_counter_entry_get(rdpa_if port)
{
    if (rdpa_if_is_wan(port)) 
#if defined(BCM63158)
        return RDD_TM_FLOW_CNTR_TABLE_SIZE - 1;
#else
        return 0;
#endif
    return RDD_TM_FLOW_CNTR_TABLE_SIZE + rdpa_port_rdpa_if_to_vport(port);
}

uint32_t rdpa_port_rx_flow_src_port_get(rdpa_if port, int set_lan_indication)
{
    uint32_t rx_flow = 0;
#ifndef G9991
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port);
#endif

    if (rdpa_if_is_wan(port) && rdpa_wan_if_to_wan_type(port) != rdpa_wan_gbe) 
        rx_flow = 0; /* XXX: Should be GEM for GPON? */
    else
    {
#ifndef G9991
        if (rdpa_if_is_cpu_port(port))
            rx_flow = BB_ID_CPU0 + vport - RDD_CPU0_VPORT;
        else
        {
            if ((rdpa_is_fttdp_mode() || (!rdpa_if_is_lag_and_switch(port) && rdpa_is_ext_switch_mode())) && !rdpa_if_is_wan(port))
#else
        {
            if (((rdpa_is_fttdp_mode() && port != rdpa_if_lan29) || (!rdpa_if_is_lag_and_switch(port) && 
                rdpa_is_ext_switch_mode())) && !rdpa_if_is_wan(port))
#endif
#ifndef BCM63158
                rx_flow = port - rdpa_if_lan0;
#else
                rx_flow = vport;
#endif
            else
                rx_flow = rdpa_emac_to_bb_id_rx[rdpa_if_to_port_emac_map[port]];
        }

        if (set_lan_indication)
            rx_flow += RDD_NUM_OF_RX_WAN_FLOWS;
    }

    return rx_flow;
}

static uint32_t rdpa_port_rx_flow_index_get(rdpa_if port)
{
#if defined(BCM63158)
    if (rdpa_if_is_wan(port))
        return rdpa_port_rx_flow_src_port_get(port, 0);
    else
#endif
    return rdpa_port_rx_flow_src_port_get(port, 1);
}

static inline void rdpa_port_clear_octets_counters(uint32_t port_index)
{
#if defined(BCM_PON_XRDP)
    int i;

    for (i = 0; i < RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE; i++)
        rdd_ag_us_tm_tx_octets_counters_table_packets_set(i, 0);

    memset(&accumulate_port_pkt_size_stat[port_index], 0, sizeof(rdpa_port_pkt_size_stat_t));
#endif
}

void port_flow_add(rdpa_if port)
{
    uint32_t cntr_id;
    bdmf_index flow_index = 0;
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port);
    RDD_RX_FLOW_ENTRY_DTS rx_flow_entry;

    if (!(rdpa_if_is_lan_or_cpu(port) || (rdpa_wan_if_to_wan_type(port) == rdpa_wan_gbe) || (rdpa_if_is_wan(port) && rdpa_is_epon_ae_mode())))
    {   
#if !defined(BCM63158)
        /* update all rx_flow to vport 0 incase of wan port */
        uint32_t i;
        for (i = 0; i < RDD_NUM_OF_RX_WAN_FLOWS; i++)
        {
            rdd_rx_flow_entry_get(i, &rx_flow_entry);
            rx_flow_entry.virtual_port = RDD_WAN0_VPORT;
            rx_flow_entry.is_lan = 0;
            rdd_rx_flow_entry_set(i, &rx_flow_entry);
        }
#else
        /* 63158: used for trapping ploam, rx flow #255 */
        rdd_rx_flow_entry_get(RDD_WAN_FLOW_PLOAM, &rx_flow_entry);
        rx_flow_entry.virtual_port = RDD_PON_WAN_VPORT;
        rdd_rx_flow_entry_set(RDD_WAN_FLOW_PLOAM, &rx_flow_entry);
#endif
        return;
    }

    /* lan port / GBE port / AE port */
    flow_index = rdpa_port_rx_flow_index_get(port);

#if !defined(BCM63158)
    /* Remap in case of GBE */
    if ((rdpa_if_is_wan(port)) && (rdpa_wan_if_to_wan_type(port) == rdpa_wan_gbe))
        rx_flow_to_vport[flow_index] = vport;

#ifdef G9991
    rdd_ag_cpu_rx_vport_to_flow_idx_set(vport, vport-1);  /* used for wan loopback mapping vport to sid */
#else
    rdd_ag_cpu_rx_vport_to_flow_idx_set(vport, flow_index);  /* used for wan loopback mapping vport to rx_flow_idx (bb_id) */
#endif
#endif

    rdpa_cntr_id_alloc(CNTR_GROUP_RX_FLOW, &cntr_id);

    rx_flow_entry.flow_dest = FLOW_DEST_ETH_ID;
    rx_flow_entry.cntr_id = cntr_id;
    rx_flow_entry.virtual_port = vport;
#if !defined(BCM63158)    
    rx_flow_entry.is_lan = !rdpa_if_is_wan(port);
#endif    
    rdd_rx_flow_entry_set(flow_index, &rx_flow_entry);

    rdpa_cntr_id_alloc(CNTR_GROUP_TX_FLOW, &cntr_id);
    rdd_tm_flow_cntr_cfg(rdpa_port_tx_counter_entry_get(port), cntr_id);
}

void port_flow_del(port_drv_priv_t *port)
{
    uint32_t tx_cntr_entry_index;
    bdmf_index flow_index = 0;
    rdd_ic_context_t context = {.cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR};

    if (!USE_CNPL_HW_COUNTER(port->index))
        return;

    flow_index = rdpa_port_rx_flow_index_get(port->index);
    rdpa_cntr_id_dealloc(CNTR_GROUP_RX_FLOW, NONE_CNTR_SUB_GROUP_ID, flow_index);

    if (port->default_cfg_exist)
    {
        rdpa_cntr_id_dealloc(CNTR_GROUP_TCAM_DEF, DEF_FLOW_CNTR_SUB_GROUP_ID, flow_index);
        rdd_rx_default_flow_cfg(flow_index, 0, &context);
        port->def_flow_index = BDMF_INDEX_UNASSIGNED;
        port->default_cfg_exist = 0;
        return;
    }

    rdd_rx_flow_del(flow_index);
        
    tx_cntr_entry_index = rdpa_port_tx_counter_entry_get(port->index);
    rdpa_cntr_id_dealloc(CNTR_GROUP_TX_FLOW, NONE_CNTR_SUB_GROUP_ID, tx_cntr_entry_index);
    rdd_tm_flow_cntr_cfg(tx_cntr_entry_index, TX_FLOW_CNTR_GROUP_INVLID_CNTR);
}

#if !defined(BCM63158)
int port_def_flow_cntr_add(port_drv_priv_t *port)
{
    rdpa_traffic_dir dir = rdpa_if_is_wan(port->index) ? rdpa_dir_ds : rdpa_dir_us;
    bdmf_index rx_flow_index = 0;
    int rc, ctx_idx;
    rdd_ic_context_t context = {};
    
    if (!USE_CNPL_HW_COUNTER(port->index))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Default flow should be set to gem\n");

    rx_flow_index = rdpa_port_rx_flow_index_get(port->index);

    if (port->def_flow_index == BDMF_INDEX_UNASSIGNED)
    {
        rc = classification_ctx_index_get(dir, rdpa_flow_def_flow_type, &ctx_idx);
        if (rc < 0)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't get free context index\n");
        rdpa_cntr_id_alloc(CNTR_GROUP_TCAM_DEF, &context.cntr_id);
    }
    else
    {
        /* already assigned */
        ctx_idx = port->def_flow_index;
        rdpa_ic_rdd_context_get(dir, ctx_idx, &context);
    }

    if (dir == rdpa_dir_us)
        rc = rdpa_map_to_rdd_classifier(rdpa_dir_us, &port->default_cfg, &context, 0, 1, RDPA_IC_TYPE_FLOW, 0);
    else
        rc = rdpa_map_to_rdd_classifier(rdpa_dir_ds, &port->default_cfg, &context, 0, 0, RDPA_IC_TYPE_FLOW, 0);
    if (rc)
        return rc;

    rc = rdpa_ic_rdd_context_cfg(dir, ctx_idx, &context);
    if (rc)
        return rc;

    port->def_flow_index = ctx_idx;
    rdd_rx_default_flow_cfg(rx_flow_index, ctx_idx, &context);
    return rc;
}
#else
int port_def_flow_cntr_add(port_drv_priv_t *port)
{
    /* TBD. */
    return 0;
}
#endif

extern int rdpa_if_to_rdd_vport_set[rdpa_if__number_of];
extern rdd_vport_id_t rdpa_if_to_rdd_vport_map[rdpa_if__number_of];
extern int rdd_vport_to_rdpa_if_set[PROJ_DEFS_RDD_VPORT_LAST + 1];
extern rdpa_if rdd_vport_to_rdpa_if_map[PROJ_DEFS_RDD_VPORT_LAST + 1];
extern rdpa_if physical_port_to_rdpa_if[rdpa_physical_none];

rdd_vport_id_t rdpa_port_rdpa_if_to_vport(rdpa_if port)
{
    BUG_ON(port >= rdpa_if__number_of);

    if (port == rdpa_if_any)
        return PROJ_DEFS_RDD_VPORT_ANY;
        
    if (!rdpa_if_to_rdd_vport_set[port])
        return PROJ_DEFS_RDD_VPORT_LAST + 1;

    return rdpa_if_to_rdd_vport_map[port];
}

rdpa_if rdpa_port_vport_to_rdpa_if(rdd_vport_id_t rdd_vport)
{
    if (rdd_vport == PROJ_DEFS_RDD_VPORT_ANY)
        return rdpa_if_any;

    if (!rdd_vport_to_rdpa_if_set[rdd_vport])
        return rdpa_if_none;

    return rdd_vport_to_rdpa_if_map[rdd_vport];
}

void rdpa_port_rdpa_if_to_vport_set(rdpa_if port, rdd_vport_id_t rdd_vport, bdmf_boolean set)
{
    if (set)
    {
        rdpa_if_to_rdd_vport_map[port] = rdd_vport;
        rdpa_if_to_rdd_vport_set[port] = (int)set;
        rdd_vport_to_rdpa_if_map[rdd_vport] = port;
        rdd_vport_to_rdpa_if_set[rdd_vport] = (int)set;
    }
    else
    {
        /* when unset, the only needed input parameter is rdpa_if */
        rdpa_if_to_rdd_vport_set[port] = (int)set;
        rdd_vport_to_rdpa_if_set[rdpa_if_to_rdd_vport_map[port]] = (int)set;
    }
}

extern rdpa_system_init_cfg_t *sys_init_cfg;

static void _port_cpu_obj_set(port_drv_priv_t *port, struct bdmf_object *cpu_obj)
{
    uint8_t cpu_obj_idx = (uint8_t)BDMF_INDEX_UNASSIGNED;

    if (cpu_obj)
    {
        cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_obj);
        cpu_obj_idx = (uint8_t)cpu_data->index;
    }

    rdd_cpu_vport_cpu_obj_set(rdpa_port_rdpa_if_to_vport(port->index), cpu_obj_idx);
}

static void port_rx_exception_cfg(port_drv_priv_t *port, uint32_t flag, bdmf_boolean is_set);
static int _port_ls_fc_cfg_ex(port_drv_priv_t *port, rdpa_port_dp_cfg_t *cfg);
static int _proto_filters_update(port_drv_priv_t *port, uint32_t proto_filters);
#ifdef BCM_PON_XRDP
static int _pbit_to_dp_map_rdd_set(port_drv_priv_t *port);
#endif

static void _tx_mirror_set_copy_to_ddr(port_drv_priv_t *port, bdmf_boolean force_copy)
{
    int channel;
    int rc, rc_id, rdpa_queue;
    uint8_t num_queues, qid;
    qm_q_context queue_cfg = {};
    uint32_t is_epon_wan = 0;

    if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe || !rdpa_if_is_lan(port->index))
        return; /* operation is needed for lan port only */

    channel = (int)port->index - rdpa_if_lan0;

    rc = rdpa_egress_tm_num_queues_get(port->tm_cfg.sched, &num_queues);
    if (rc)
    {
        BDMF_TRACE_ERR("can't locate tm with port index = %d\n", port->index);
        return;
    }

    if (rdpa_if_is_wan(port->index) && (rdpa_is_epon_ae_mode() || rdpa_is_epon_or_xepon_mode()))
        is_epon_wan = 1;
    else
        _rdpa_egress_tm_enable_set(port->tm_cfg.sched, 0, 1); /* disable and flush queues */

    for (qid = 0; qid < num_queues; qid++)
    {
        rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_ds, channel, qid, &rc_id, &rdpa_queue);
        if (!rc)
        {
            if (mirroring_cfg.tx_mirror_copy_to_ddr_prev[port->index] == -1)
            {
                drv_qm_queue_get_config(rdpa_queue, &queue_cfg);
                mirroring_cfg.tx_mirror_copy_to_ddr_prev[port->index] = (int)queue_cfg.copy_to_ddr;
            }
            if (force_copy)
                force_copy_ddr_on_queue(rdpa_queue, force_copy, is_epon_wan);
            else /* return previous mirror configuration */
            {
                force_copy_ddr_on_queue(rdpa_queue, mirroring_cfg.tx_mirror_copy_to_ddr_prev[port->index], is_epon_wan);
                mirroring_cfg.tx_mirror_copy_to_ddr_prev[port->index] = -1;
            }
        }
    }
    if (!is_epon_wan)
        _rdpa_egress_tm_enable_set(port->tm_cfg.sched, 1, 0); /* enable queues */
}

int port_post_init_ex(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_mirror_cfg_t *mirror_cfg = &port->mirror_cfg;
    rdd_mirroring_cfg_t rdd_mirroring_cfg = 
    {.rx_dst_queue = QM_NUM_QUEUES + 1,
     .tx_dst_queue = QM_NUM_QUEUES + 1,
     .src_tx_bbh_id = rdpa_emac_to_bbh_id_e[rdpa_if_to_port_emac_map[port->index]] };

#ifdef INGRESS_FILTERS
    rdpa_filter_ctrl_t ingress_filters_init_ctrl[RDPA_FILTERS_QUANT] = {};
#endif
    bbh_id_e bbh_id;
#if defined(BCM_PON_XRDP)
    rdd_bb_id  bb_dest;
#if !defined(G9991)
    bbh_id_e bbh_id_gbe_wan;
    uint32_t ingress_counter, egress_counter, bbh_table_index;
#endif
#endif
    int rc = 0, i;

#if !defined(G9991) && !defined(XRDP_EMULATION)
    if (port->cfg.emac == rdpa_emac_none && port->has_emac)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "%s EMAC is not set\n", bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }
#endif

    /* Only SA/DA lookup should be configured in fttdp mode */
    if (!rdpa_is_fttdp_mode())
        rc = rdpa_cfg_sa_da_lookup(port, &port->cfg, port->cfg.sal_enable, 0);
    else if (rdpa_if_is_lag_and_switch(port->index) && port->index != rdpa_if_switch)
        rc = rdpa_update_da_sa_searches(port->index, 0);
    if (rc)
        return rc;

    /* Physical port id for external switch */
    if (rdpa_if_is_lan(port->index))
    {
        if (rdpa_is_ext_switch_mode() && port->cfg.physical_port != rdpa_physical_none)
        {
            rdd_vport_id_t rdd_port = rdpa_if_to_rdd_vport(port->index, port->wan_type);
            rc = rdd_broadcom_switch_ports_mapping_table_config(rdd_port, port->cfg.physical_port);
            if (rc)
                BDMF_TRACE_RET_OBJ(rc, mo, "rdd_broadcom_switch_ports_mapping_table_config failed\n");
        }

        else if (port->cfg.physical_port == rdpa_physical_none)
        {
#if defined(BCM63158) /* On 63158 the table physical_port_to_rdpa_if is hard mapped */
            /* Make sure this rdpa-if is assigned a physical port */
            rdpa_physical_port physical_port = rdpa_physical_port0;
            for (; physical_port < rdpa_physical_none; physical_port++)
            {
                if (physical_port_to_rdpa_if[physical_port] == port->index)
                {
                    port->cfg.physical_port = physical_port;
                    break;
                }
            }
            if (physical_port >= rdpa_physical_none)
            {
                rc = BDMF_ERR_PARM;
                BDMF_TRACE_RET_OBJ(rc, mo, "rdpa_if_lanX is not mapped to physical_port\n");
            }
#else
            /* Backward compatibility */
            port->cfg.physical_port = (rdpa_physical_port)(port->cfg.emac);

            /* Can break rdpa_physical_port_to_rdpa_if if input is not 1:1 physical<->rdpa mapping */
            physical_port_to_rdpa_if[port->cfg.physical_port] = port->index;
#endif
        }
#ifdef G9991
        if (rdpa_is_fttdp_mode() && (port->index != rdpa_if_lan29))
            rdd_g9991_vport_to_emac_mapping_cfg(port->index - rdpa_if_lan0, port->cfg.physical_port);
#endif
    }
    else if (port->wan_type == rdpa_wan_gbe && port->cfg.physical_port == rdpa_physical_none)
    {
        /* Backward compatibility */
        port->cfg.physical_port = (rdpa_physical_port)(port->cfg.emac);
    }

    /* Update all_ports mask. Notify RDD if necessary */
    rc = port_update_all_ports_set(mo, 1);
    if (rc)
        return rc;

    rdpa_port_rdpa_if_to_vport_set(port->index, rdpa_if_to_rdd_vport(port->index, port->wan_type), 1);

#if defined(BCM63158)
    /* For 63158, we need to enable multicast whitelist for GPON/XGPON interface */
    if ((port->wan_type == rdpa_wan_gpon) || (port->wan_type == rdpa_wan_xgpon))
    {
        bdmf_object_handle mcast_whitelist_obj = NULL;

        /* if RDPA Mcast_whitelist object exists, set it to enable by default */
        rc = rdpa_mcast_whitelist_get(&mcast_whitelist_obj);
        if (!rc && mcast_whitelist_obj != NULL)
        {
            rdpa_mcast_whitelist_port_enable_set(mcast_whitelist_obj, rdpa_wan_type_to_if(port->wan_type), 1);
            bdmf_put(mcast_whitelist_obj);
        }
    }
#endif

    /* Set up ingress filters if configured */
    for (i = 0; i < RDPA_FILTERS_QUANT; i++)
    {
        if (!port->ingress_filters[i].enabled)
            continue;
#ifdef INGRESS_FILTERS
        rc = ingress_filter_entry_set((rdpa_filter)i, mo, ingress_filters_init_ctrl,
            &port->ingress_filters[i], &port->ingress_filters_profile);
#endif
        if (rc)
            break;
    }

    if (!rc)
        rc = rdpa_cfg_sa_da_lookup(port, &port->cfg, port->cfg.sal_enable, 1);

    /* If failed to configure rdd, revert cfg to default*/
    if (rc)
    {
        if (is_triple_tag_detect())
            port->cfg.sal_enable = 0;
        else
            port->cfg.sal_enable = 1;

        port->cfg.dal_enable = 1;
        port->cfg.sal_miss_action = rdpa_forward_action_host;
        port->cfg.dal_miss_action = rdpa_forward_action_host;
    }

    rc = _port_ls_fc_cfg_ex(port, &port->cfg);
    if (rc)
    {
        /*If failed to configure rdd, revert ls cfg to default*/
        port->cfg.ls_fc_enable = rdpa_if_is_wlan(port->index) ? 1 : 0;
    }
#ifdef CONFIG_RNR_BRIDGE
    /* set bridge ID to default (-1) */
    rdd_ag_processing_vport_cfg_table_bridge_id_set(rdpa_port_rdpa_if_to_vport(port->index), NO_BRIDGE_ID);
#endif
    if (rdpa_if_is_lan(port->index))
        rc = port_vlan_isolation_cfg_ex(mo, &port->vlan_isolation, 1);

    /* If failed to configure rdd or tried to configure a non lan port, revert isolation cfg to default */
    if (rc || (!rdpa_if_is_lan(port->index)))
    {
        port->vlan_isolation.ds = 0;
        port->vlan_isolation.us = 0;
    }
    port_flow_add(port->index);
#if defined(BCM_PON_XRDP)
    if (rdpa_if_is_wan(port->index))
    {
        if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe)
        {
#if defined(CONFIG_MULTI_WAN_SUPPORT)
            RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(rdpa_emac_to_bb_id_tx[port->cfg.emac], RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, BBH_QUEUE_WAN_1_ENTRY_ID);
            RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(0, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, BBH_QUEUE_WAN_1_ENTRY_ID);
#elif defined(XRDP_BBH_PER_LAN_PORT)
            RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(rdpa_emac_to_bb_id_tx[port->cfg.emac], RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, 0);
            RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(rdpa_gbe_wan_emac(), RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, rdpa_gbe_wan_emac());
#else
            RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_LAN, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, rdpa_gbe_wan_emac());
            RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(rdpa_gbe_wan_emac(), RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, rdpa_gbe_wan_emac());
#endif
        }
        else
        {
            if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_epon)
                bb_dest = BB_ID_TX_PON_ETH_STAT;
            else
                bb_dest = BB_ID_TX_PON_ETH_PD;
            for (i = 0; i < BBH_QUEUE_WAN_1_ENTRY_ID; i++)
            {
                RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(bb_dest, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, i);
            }
        }
    }
    else
    {
        /* avoiding initialization of ports that don't have emac */
        if (port->cfg.emac < rdpa_emac__num_of)
        {
#ifdef XRDP_BBH_PER_LAN_PORT
            RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(rdpa_emac_to_bb_id_tx[port->cfg.emac], RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, port->cfg.emac);
            RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(0, RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, port->cfg.emac);
#else

            RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(BB_ID_TX_LAN, RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, port->cfg.emac);
            RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(port->cfg.emac, RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, port->cfg.emac);
#endif
        }
    }
#endif    

#if !defined(BCM63158) && !defined(G9991)
    if (rdpa_if_is_wan(port->index))
    {
        if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe)
        {
          /* we are keeping ingress and egress counters in ds tables always updated, so can copy from it */
          /* update egress and ingress counters table */
          bbh_table_index = port->cfg.emac;
          RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_READ_G(ingress_counter, RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, bbh_table_index);
#if defined(BCM6858)
          RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ_G(egress_counter, RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, bbh_table_index);
#else
          RDD_BYTE_1_BITS_READ_G(egress_counter, RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, bbh_table_index);
#endif
        }
        else
        {
          bbh_table_index = 0;
          /* should be good enough for PON and AE, as only 1st value gets overwrtten */
          RDD_BYTE_1_BITS_READ_G(ingress_counter, RDD_BACKUP_BBH_INGRESS_COUNTERS_TABLE_ADDRESS_ARR, bbh_table_index);
          RDD_BYTE_1_BITS_READ_G(egress_counter, RDD_BACKUP_BBH_EGRESS_COUNTERS_TABLE_ADDRESS_ARR, bbh_table_index);
        }
        
#if defined(XRDP_BBH_PER_LAN_PORT)
        bbh_table_index = 0;
#endif
        RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_WRITE_G(ingress_counter, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, bbh_table_index);
        RDD_BYTE_1_BITS_WRITE_G(egress_counter, RDD_US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, bbh_table_index);
    }
#endif

    if (rdpa_if_is_wan(port->index) && (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe))
    {
#if defined(BCM63158)
        /* tx_flow_table entry for GBE WAN port is shared with DS in the VPORT-specific table */
        rdd_tx_flow_enable(rdpa_port_rdpa_if_to_vport(port->index), rdpa_dir_ds, 1);
#else
#if defined(CONFIG_MULTI_WAN_SUPPORT)
        port_rx_exception_cfg(port, EXCEPTION_PORT_WAN_GBE, 1);
        rdd_tx_flow_enable(rdpa_port_rdpa_if_to_vport(port->index), rdpa_dir_ds, 1);
#else
        port_rx_exception_cfg(port, EXCEPTION_PORT_WAN_GBE, 1);
        rdd_tx_flow_enable(0, rdpa_dir_us, 1);
#endif
#endif
    }
    else if (rdpa_if_is_wan(port->index) && rdpa_is_epon_ae_mode())
    {
        rdd_tx_flow_enable(0, rdpa_dir_us, 1);
    }
    else
        rdd_tx_flow_enable(rdpa_port_rdpa_if_to_vport(port->index), rdpa_dir_ds, 1);

    if (port->default_cfg_exist && !port->default_cfg.disable_stat)
        rc = port_def_flow_cntr_add(port);

#if !defined(BCM63158) && !defined(G9991)
    if ((rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe) || rdpa_if_is_lan(port->index))
    {
        bbh_id_gbe_wan = rdpa_gbe_wan_emac() != rdpa_emac_none ? rdpa_emac_to_bbh_id_e[rdpa_gbe_wan_emac()] : BBH_ID_NULL;
        rc = rc ? rc : bbh_tx_reconfig(bbh_id_gbe_wan, rdpa_emac_to_bbh_id_e[port->cfg.emac]);
    }
    else if ((rdpa_if_is_wan(port->index)) && (rdpa_is_epon_ae_mode()))
    {
       /* currently data_path_init_fiber taking care -- need to check if not destructive */
       /* rc = rc ? rc : bbh_tx_pon_reconfig(); */
    }
    else
    {
      /* currently data_path_init_fiber taking care -- need to check if not destructive */
      /* TODO: PON mode */
    }
#endif

    if (rdpa_if_is_wan(port->index))
        rc = rc ? rc : rdd_us_budget_allocation_timer_set();

    if (!rc)
        _port_cpu_obj_set(port, port->cpu_obj);

    if (!rc && (mirror_cfg->rx_dst_port || mirror_cfg->tx_dst_port))
    {
        rc = port_rdd_mirror_cfg(port, mirror_cfg, &rdd_mirroring_cfg);
        if (!rc && mirror_cfg->rx_dst_port)
            rc = _port_mirror_cfg_rx(port, mirror_cfg, &rdd_mirroring_cfg);
        if (!rc && mirror_cfg->tx_dst_port)
            rc = _port_mirror_cfg_tx(port, mirror_cfg, &rdd_mirroring_cfg);
    }

    if (!rc)
        rc = _proto_filters_update(port, port->proto_filters);

#ifdef BCM_PON_XRDP
    if (!rc)
        rc = _pbit_to_dp_map_rdd_set(port); 
#endif

    if (rc)
    {
        /* rollback */
        rdpa_port_rdpa_if_to_vport_set(port->index, 0, 0);
    }
#ifdef DISPATCHER_UNIFIED_MULTICAST_BBMSG
    rdd_ag_processing_vport_cfg_ex_table_viq_set(rdpa_port_rdpa_if_to_vport(port->index), rdpa_port_to_bbh_id(port));
    if (rdpa_if_is_wan(port->index))
#if defined(BCM63158)
        GROUP_MWRITE_8(RDD_DIRECT_PROCESSING_WAN_VIQ_EXCLUSIVE_ADDRESS_ARR, 0, 
            rdpa_port_to_bbh_id(port) + DISP_REOR_VIQ_BBH_RX0_EXCL);
#else	
        GROUP_MWRITE_8(RDD_DIRECT_PROCESSING_WAN_VIQ_EXCLUSIVE_ADDRESS_ARR, 0, DISP_REOR_VIQ_BBH_WAN_EXCL);
#endif		
#endif

#ifdef G9991
    if (rdpa_if_is_lag_and_switch(port->index) && port->index != rdpa_if_switch) {
        rdd_g9991_control_sid_set(port->cfg.control_sid - rdpa_if_lan0, port->cfg.emac - rdpa_emac0);
        port->cfg.physical_port = port->cfg.emac - rdpa_emac0;
    }

    /* System port marked as exception since its packets are not reassembled,
       lan29 is temporarily used as the system port  */
    if (port->index == rdpa_if_lan29) {
        port_rx_exception_cfg(port, EXCEPTION_PORT_SYSTEM, 1);
        rdd_g9991_system_port_set(port->cfg.emac);
    }
#endif
    /* clean accumulative data of port */
    memset(&accumulative_port_stat[port->index], 0, sizeof(rdpa_port_stat_t));
    memset(&accumulative_port_debug_stat[port->index], 0, sizeof(rdpa_port_debug_stat_t));
    memset(&accumulate_port_pkt_size_stat[port->index], 0, sizeof(rdpa_port_pkt_size_stat_t));

    /* temporaly till tx mirroring sbpm packets will be supported */
    mirroring_cfg.tx_mirror_copy_to_ddr_prev[port->index] = -1; /* -1 - prev value is not set */

#ifndef RDP_SIM
    /* clear HW counters by read section - not supported by simulator (socket not connected yet)*/
    {
        bbh_rx_counters_t bbh_rx_counters = {};
        bbh_tx_counters_t bbh_tx_counters = {};
        rdd_vport_pm_counters_t rdd_port_counters = {};

        if (!(_validate_port_index(port->index)))
        {
            /* clear hw port counters by clear on read*/
            read_stat_from_hw(port, &rdd_port_counters, &bbh_rx_counters, &bbh_tx_counters);
        }
    }
#endif

    /* enable BBH Rx */
    bbh_id = rdpa_port_to_bbh_id(port);
    if (bbh_id != BBH_ID_NULL)
    {
        ag_drv_bbh_rx_general_configuration_enable_set(bbh_id, 1, 1);
    }
    
    return rc;
}

int port_attr_def_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    int rc = 0;
#if !defined(BCM63158)
    rdpa_traffic_dir dir = rdpa_if_is_wan(port->index) ? rdpa_dir_ds : rdpa_dir_us;
    bdmf_index rx_flow_index = 0;
    rdd_ic_context_t context = {.cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR};
#endif

    if (rdpa_if_is_lag_and_switch(port->index))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "can't set default flow on lag ports port\n");

    if (rdpa_if_is_wan(port->index) && rdpa_wan_if_to_wan_type(port->index) != rdpa_wan_gbe)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Default flow should be set to gem\n");

    if (cfg == NULL)
    {
#if !defined(BCM63158)
        /* FIXME!! Wen.. not sure what the following does.. does it apply to 63158? */
        rx_flow_index = rdpa_port_rx_flow_index_get(port->index);
        rdd_rx_default_flow_cfg(rx_flow_index, port->def_flow_index, &context);

        rdpa_cntr_id_dealloc(CNTR_GROUP_TCAM_DEF, DEF_FLOW_CNTR_SUB_GROUP_ID, rx_flow_index);

        classification_ctx_index_put(dir, port->def_flow_index);
#endif
        port->def_flow_index = BDMF_INDEX_UNASSIGNED;
        memset(&port->default_cfg, 0, sizeof(rdpa_ic_result_t));

        return rc;
    }

    port->default_cfg_exist = 1;
    memcpy(&port->default_cfg, cfg, sizeof(rdpa_ic_result_t));

    if (mo->state == bdmf_state_active && !cfg->disable_stat)
        rc = port_def_flow_cntr_add(port);

    return rc;
}

#if !defined(BCM63158)
int port_attr_def_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    rdpa_traffic_dir dir = rdpa_if_is_wan(port->index) ? rdpa_dir_ds : rdpa_dir_us;
    rdd_ic_context_t context = {};

    if (port->def_flow_index == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;
    rdpa_ic_rdd_context_get(dir, port->def_flow_index, &context);

    return rdpa_map_from_rdd_classifier(dir, cfg, &context, 0);
}
#else
int port_attr_def_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    /* memset to prevent val_to_s errors when displaying default ic fields */
    memset(val, 0, sizeof(rdpa_ic_result_t));
    cfg->qos_method = rdpa_qos_method_flow;
    cfg->action = rdpa_forward_action_host;
    cfg->forw_mode = rdpa_forwarding_mode_flow;
    cfg->egress_port = rdpa_if_none;

    return 0;
}
#endif

int mac_lkp_cfg_validate_ex(rdpa_mac_lookup_cfg_t *mac_lkp_cfg, port_drv_priv_t *port, int ls_fc_enable)
{
    if (mac_lkp_cfg->sal_enable)
    {
        if (mac_lkp_cfg->sal_miss_action == rdpa_forward_action_flood)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Flooding can be configured as SA lookup miss action only\n");
        if (mac_lkp_cfg->sal_miss_action == rdpa_forward_action_forward)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Forward action is not allowed for SA lookup.\n");
    }
    if (mac_lkp_cfg->dal_enable)
    {
        if (mac_lkp_cfg->dal_miss_action == rdpa_forward_action_forward && rdpa_if_is_wan(port->index))
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Forward action is not allowed for DA lookup on wan port.\n");
    }

    return BDMF_ERR_OK;
}

int rdpa_cfg_sa_da_lookup_ex(port_drv_priv_t *port, rdpa_port_dp_cfg_t *cfg)
{
#ifdef CONFIG_RNR_BRIDGE
    rdd_vport_id_t rdd_port = rdpa_port_rdpa_if_to_vport(port->index);
    rdd_vport_cfg_entry_t vport_entry = {};
    int rc;

    rc = rdd_ag_processing_vport_cfg_entry_get(rdd_port, &vport_entry);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Failed to get vport_cfg for sa&da lookup, vport=%d\n", rdd_port);

    vport_entry.da_lookup_en = cfg->dal_enable;
    vport_entry.sa_lookup_en = cfg->sal_enable;
    vport_entry.sa_lookup_miss_action = rdpa_forward_action2rdd_action(cfg->sal_miss_action);
    if (cfg->dal_miss_action != rdpa_forward_action_drop_low_pri)
        vport_entry.da_lookup_miss_action = rdpa_forward_action2rdd_action(cfg->dal_miss_action);
    else
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Illegal DA miss action\n");
    }    

    rc = rdd_ag_processing_vport_cfg_entry_set(rdd_port, &vport_entry);

    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Failed to set vport_cfg for sa&da lookup, vport=%d\n", rdd_port);

    /* TBD: update all vlans not linked to bridge */
    return rc;

#else
    return BDMF_ERR_OK;
#endif
}

int rdpa_update_da_sa_searches(rdpa_if port, bdmf_boolean dal)
{
    return BDMF_ERR_OK;
}

void update_port_tag_size(rdpa_emac emac, drv_rnr_prop_tag_size_t new_tag_size)
{
    rnr_quad_parser_core_configuration_prop_tag_cfg prop_cfg = {};
    uint8_t i;

    /* TODO: should be to all profiles? */
    prop_cfg.size_profile_0 = new_tag_size;
    prop_cfg.size_profile_1 = new_tag_size;
    prop_cfg.size_profile_2 = new_tag_size;

    /* TODO: should be for all rnr quads? */
    for (i = 0; i < NUM_OF_RNR_QUADS; ++i)
        ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(i, &prop_cfg);
}

void update_broadcom_tag_size(void)
{
    rnr_quad_parser_core_configuration_prop_tag_cfg prop_cfg = {};
    uint8_t i;

    /* profile 1 is upstream */
    prop_cfg.size_profile_1 = 4;

    /* TODO: should be for all rnr quads? */
    for (i = 0; i < NUM_OF_RNR_QUADS; ++i)
        ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(i, &prop_cfg);
}

int rdpa_if_to_rdpa_physical_port(rdpa_if port, rdpa_physical_port *physical_port)
{
    if (port <= rdpa_if_lan6)
        *physical_port = port + rdpa_physical_port0;
    else
        BDMF_TRACE_RET(BDMF_ERR_PERM, "can't map rdpa_if % d to rdpa_physical_port\n", port);

    return BDMF_ERR_OK;
}

static bbh_id_e rdpa_port_to_bbh_id(port_drv_priv_t *port)
{
    bbh_id_e bbh_id;
    rdpa_emac emac = port->cfg.emac;
    int is_emac_valid;

    if (port->cfg.emac == rdpa_emac_none && !rdpa_if_is_wan(port->index))
    {
        bbh_id = BBH_ID_NULL;
        goto exit;
    }

#if !defined(G9991)
    is_emac_valid = port->has_emac;
    emac = rdpa_if_to_port_emac_map[port->index];
#else
    is_emac_valid = (port->cfg.emac != rdpa_emac_none);
#endif

    if (is_emac_valid)
    {
        bbh_id = rdpa_emac_to_bbh_id_e[emac];
        goto exit;
    }

#if defined(BCM63158)
    if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_dsl)
        bbh_id = BBH_ID_DSL;
    else
        bbh_id = BBH_ID_PON;

    goto exit;    
#endif

    if (rdpa_if_is_wan(port->index) && (rdpa_wan_if_to_wan_type(port->index) != rdpa_wan_gbe))
        bbh_id = BBH_ID_PON;
    else
        bbh_id = BBH_ID_NULL;

exit:

    return bbh_id;
}

int port_attr_mtu_size_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t mtu;
    uint8_t min_pkt_size;
    bbh_id_e bbh_id;

    bbh_id = rdpa_port_to_bbh_id(port);
    if (bbh_id == BBH_ID_NULL)
        return BDMF_ERR_INTERNAL;

    /* Since we assume that all selections will have same configuration, enough to read only first selection */
    drv_bbh_rx_pkt_size_get(bbh_id, 0, &min_pkt_size, &mtu);

    *(uint32_t *)val = mtu;

    return BDMF_ERR_OK;
}

int port_attr_mtu_size_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t mtu_size = *(uint32_t *)val;
    uint16_t tmp;
    uint8_t min_pkt_size;
    bbh_id_e bbh_id;
    int rc = 0;

    bbh_id = rdpa_port_to_bbh_id(port);

    if (bbh_id == BBH_ID_NULL)
        return BDMF_ERR_INTERNAL;

    drv_bbh_rx_pkt_size_get(bbh_id, 0, &min_pkt_size, &tmp);
    rc = drv_bbh_rx_pkt_size_set(bbh_id, 0, min_pkt_size, (uint16_t)mtu_size);

    return rc ? BDMF_ERR_INTERNAL : BDMF_ERR_OK;
}

bdmf_error_t rdpa_port_tm_discard_prty_cfg_ex(struct bdmf_object *mo, rdpa_port_tm_cfg_t *tm_cfg)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
#ifndef BCM63158 
    rdd_vport_id_t rdd_port = rdpa_port_rdpa_if_to_vport(port->index);

    BDMF_TRACE_DBG("Going to rdpa_port_tm_discard_prty_cfg_ex, port index=%d, vport=%d discard priority=%d\n", (int)port->index, rdd_port, (int)tm_cfg->discard_prty);

    RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_WRITE_G(tm_cfg->discard_prty, RDD_VPORT_CFG_TABLE_ADDRESS_ARR, rdd_port);
#endif    
    port->tm_cfg.discard_prty = tm_cfg->discard_prty;	
    return rc;
}

#ifdef G9991 /*read multicast/broadcast counters for g9991 project*/
static bdmf_error_t read_mcst_bcst_stat_from_hw(port_drv_priv_t *port, rdd_vport_pm_counters_t *rdd_vport_mc_bc_counters)
{
    uint32_t vport_id;
    uint32_t cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    int rc = 0;

    vport_id = rdpa_port_rdpa_if_to_vport(port->index);
    cntr_id =  vport_id + PORT_MCST_BCST_GROUP_RX_MULTICAST_PKT;
    rc = drv_cntr_counter_read(CNTR_GROUP_PORT_MCST_BCST, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read PORT_MCST_BCST_GTOUP_ID for entry %d. err: %d\n", cntr_id, rc);

    rdd_vport_mc_bc_counters->rx_multicast_pkt = cntr_arr[0];

    cntr_id = vport_id + PORT_MCST_BCST_GROUP_RX_BROADCAST_PKT;
    rc = drv_cntr_counter_read(CNTR_GROUP_PORT_MCST_BCST, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read PORT_MCST_BCST_GTOUP_ID for entry %d. err: %d\n", cntr_id, rc);

    rdd_vport_mc_bc_counters->rx_broadcast_pkt  = cntr_arr[0];

    cntr_id =  vport_id + PORT_MCST_BCST_GROUP_TX_MULTICAST_PKT;
    rc = drv_cntr_counter_read(CNTR_GROUP_PORT_MCST_BCST, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read PORT_MCST_BCST_GTOUP_ID for entry %d. err: %d\n", cntr_id, rc);

    rdd_vport_mc_bc_counters->tx_multicast_pkt = cntr_arr[0];

    cntr_id = vport_id + PORT_MCST_BCST_GROUP_TX_BROADCAST_PKT;
    rc = drv_cntr_counter_read(CNTR_GROUP_PORT_MCST_BCST, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read PORT_MCST_BCST_GTOUP_ID for entry %d. err: %d\n", cntr_id, rc);

    rdd_vport_mc_bc_counters->tx_broadcast_pkt  = cntr_arr[0];

    return rc;
}
#endif

static int read_stat_from_hw(port_drv_priv_t *port, rdd_vport_pm_counters_t *rdd_port_counters,
    bbh_rx_counters_t *bbh_rx_counters, bbh_tx_counters_t *bbh_tx_counters)
{
    uint32_t flow_index = 0, tx_cntr_entry_index;
    uint32_t cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    int rc = 0;
    bbh_id_e bbh_id;
#if !defined(BCM63158)
    uint16_t cntr_wan_drop_pkt;
#endif

    /* BBH counters */
    bbh_id = rdpa_port_to_bbh_id(port); 

    if (bbh_id != BBH_ID_NULL)
        drv_bbh_rx_get_stat(bbh_id, bbh_rx_counters);

#if defined(BCM63158)
    if (rdpa_if_is_wan(port->index) && rdpa_wan_if_to_wan_type(port->index) != rdpa_wan_gbe && !rdpa_is_epon_ae_mode())
    {
        if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_dsl)
            ag_drv_bbh_tx_debug_counters_get(BBH_TX_ID_DSL, &(bbh_tx_counters->debug_cntrs));
        else
            ag_drv_bbh_tx_debug_counters_get(BBH_TX_ID_PON, &(bbh_tx_counters->debug_cntrs));
#else /* BCM63158 */
    if (rdpa_if_is_wan(port->index))
    {
        if (rdpa_wan_if_to_wan_type(port->index) != rdpa_wan_gbe)
            bbh_id = BBH_TX_WAN_ID; /*set bbh_id FOR epon/gpon*/

        /* read wan0 drop counter */
        rc = drv_cntr_various_counter_get(COUNTER_RX_WAN_PORT_DROP, &cntr_wan_drop_pkt);
        if (rc)
        {
            BDMF_TRACE_ERR("Could not read DROP_GROUP_RX_WAN_PORT counter for flow. rc: %d\n", rc);
            cntr_wan_drop_pkt = 0; /* this will keep the counter value */
        }                          /* as last known accumulative one*/

        /* Read bbh tx counters in bytes */
#if defined(BCM6858)
        ag_drv_bbh_tx_debug_counters_srambyte_get(bbh_id, &(bbh_tx_counters->srambyte));
        ag_drv_bbh_tx_debug_counters_ddrbyte_get(bbh_id, &(bbh_tx_counters->ddrbyte));
        ag_drv_bbh_tx_debug_counters_get(bbh_id, &(bbh_tx_counters->debug_cntrs));
#else
        if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe)
        {
            /*in case of GBE on chips 6846 and 6856 */
            /*bbh_tx bus is united for lans, therefore we should read from different registers*/
            rc = ag_drv_bbh_tx_debug_counters_unifiedpkt_get(0, bbh_id, &(bbh_tx_counters->debug_cntrs.srampkt));
            rc = rc ? rc : ag_drv_bbh_tx_debug_counters_unifiedbyte_get(0, bbh_id, &(bbh_tx_counters->srambyte));
        }
        else
        {
            ag_drv_bbh_tx_debug_counters_srambyte_get(bbh_id, &(bbh_tx_counters->srambyte));
            ag_drv_bbh_tx_debug_counters_ddrbyte_get(bbh_id, &(bbh_tx_counters->ddrbyte));
            ag_drv_bbh_tx_debug_counters_get(bbh_id, &(bbh_tx_counters->debug_cntrs));
        }
#endif /* BCM6858 */
        if (rdpa_is_epon_ae_mode())
        {
            flow_index = 0; /* we assume that in case of AE flow index is always 0*/
            cntr_id = rdd_rx_flow_cntr_id_get(flow_index);
            rc = drv_cntr_counter_read(CNTR_GROUP_RX_FLOW, cntr_id, cntr_arr);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR counters for AE flow %d. err: %d\n", flow_index, rc);

            rdd_port_counters->rx_bytes = cntr_arr[1];
        }
        rdd_port_counters->rx_drop_pkt = cntr_wan_drop_pkt;
#ifdef G9991
        {
            int rc1;

            rc1 = read_mcst_bcst_stat_from_hw(port, rdd_port_counters);
            if (rc1)
            {
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read MC BC counters for port %d. err: %d\n", 
                    rdpa_port_rdpa_if_to_vport(port->index), rc);
            }
            if (!rc)
                rc = rc1;
        }
#endif
#endif /* - not BCM63158 */
        return rc;
    }

    /* RX PM counters */
    flow_index = rdpa_port_rx_flow_index_get(port->index);
    cntr_id = rdd_rx_flow_cntr_id_get(flow_index);

    if (cntr_id == RX_FLOW_CNTR_GROUP_INVLID_CNTR)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "can't get counter id. RX flow index - %d\n", flow_index);

    rc = drv_cntr_counter_read(CNTR_GROUP_RX_FLOW, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR counters for flow %d. err: %d\n", flow_index, rc);

    rdd_port_counters->rx_packets = cntr_arr[0];
    rdd_port_counters->rx_bytes = cntr_arr[1];
    rdd_port_counters->rx_drop_pkt = cntr_arr[2];

    /* TX PM counters */
    tx_cntr_entry_index = rdpa_port_tx_counter_entry_get(port->index);
    cntr_id = rdd_tm_flow_cntr_id_get(tx_cntr_entry_index);

    if (cntr_id == TX_FLOW_CNTR_GROUP_INVLID_CNTR)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "can't get counter id. TX flow index - %d\n", tx_cntr_entry_index);

    rc = drv_cntr_counter_read(CNTR_GROUP_TX_FLOW, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR counters for entry %d. err: %d\n", tx_cntr_entry_index, rc);

    rdd_port_counters->tx_packets = cntr_arr[0];
    rdd_port_counters->tx_bytes = cntr_arr[1];

#ifdef G9991
    rc = read_mcst_bcst_stat_from_hw(port, rdd_port_counters);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read MC BC counters for port %d. err: %d\n", 
            rdpa_port_rdpa_if_to_vport(port->index), rc);
    }

#endif
    return rc;
}

static int _validate_port_index(rdpa_if index)
{
    if (rdpa_if_is_lag_and_switch(index))
    {
        /* ToDo */
        return BDMF_ERR_NOT_SUPPORTED;
    }

    if (rdpa_if_is_cpu_port(index))
    {
        /* ToDo: need to accumalte Ring counters */
        return BDMF_ERR_NOT_SUPPORTED;
    }

    if (index > rdpa_if_lan_max)
        return BDMF_ERR_PARM;

    return BDMF_ERR_OK;
}

/* "debug_stat" attribute "read" callback */
int port_attr_debug_stat_read_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_port_debug_stat_t *stat = (rdpa_port_debug_stat_t *)val;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    bbh_id_e bbh_id;
    bbh_rx_pm_counters pm_counters = {};
    bbh_rx_error_pm_counters error_pm_counters = {};
    int rc, rc_to_ret = 0;

    memset(stat, 0, sizeof(rdpa_port_debug_stat_t));

    rc = _validate_port_index(port->index);
    if (rc)
    {
        rc = (rc == BDMF_ERR_NOT_SUPPORTED ? BDMF_ERR_OK : rc);
        return rc;
    }

    bbh_id = rdpa_port_to_bbh_id(port);

    if (bbh_id == BBH_ID_NULL)
        return BDMF_ERR_INTERNAL;

    rc = ag_drv_bbh_rx_error_pm_counters_get(bbh_id, &error_pm_counters);
    if (rc == BDMF_ERR_OK)
    {
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_debug_stat[port->index],
            _OFFSET_DBG_(bbh_rx_crc_err_ploam_drop), error_pm_counters.crc_err_ploam);
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_debug_stat[port->index],
            _OFFSET_DBG_(bbh_rx_third_flow_drop), error_pm_counters.third_flow);
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_debug_stat[port->index],
            _OFFSET_DBG_(bbh_rx_sop_after_sop_drop), error_pm_counters.sop_after_sop);
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_debug_stat[port->index],
            _OFFSET_DBG_(bbh_rx_no_sbpm_bn_ploam_drop), error_pm_counters.no_sbpm_bn_ploam);
    }
    else
    {
    	rc_to_ret = rc;
    	BDMF_TRACE_ERR("Failed to read bbh_rx_error_pm_counters stats rc: %d\n", rc);
    }

    rc = ag_drv_bbh_rx_pm_counters_get(bbh_id, &pm_counters);
    if (rc == BDMF_ERR_OK)
    {
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_debug_stat[port->index],
            _OFFSET_DBG_(bbh_rx_no_sdma_cd_drop), pm_counters.no_sdma_cd);
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_debug_stat[port->index],
            _OFFSET_DBG_(bbh_rx_ploam_no_sdma_cd_drop), pm_counters.ploam_no_sdma_cd);
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_debug_stat[port->index],
             _OFFSET_DBG_(bbh_rx_ploam_disp_cong_drop), pm_counters.ploam_disp_cong);
    }
    else
    {
    	rc_to_ret = rc;
    	BDMF_TRACE_ERR("Failed to read bbh_rx_pm_counters stats rc: %d\n", rc);
    }

    return rc_to_ret;
}

/* "debug_stat" attribute "write" callback */
int port_attr_debug_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc;

    rc = _validate_port_index(port->index);
    if (rc)
    {
        rc = (rc == BDMF_ERR_NOT_SUPPORTED ? BDMF_ERR_OK : rc);
        return rc;
    }

    memset(&accumulative_port_debug_stat[port->index], 0, sizeof(rdpa_port_debug_stat_t));
    return BDMF_ERR_OK;
}

int port_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_port_stat_t *stat = (rdpa_port_stat_t *)val;
    bbh_rx_counters_t bbh_rx_counters = {};
    bbh_tx_counters_t bbh_tx_counters = {};
    rdd_vport_pm_counters_t rdd_port_counters = {};
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
    rdpa_wan_type wan_type = rdpa_wan_if_to_wan_type(port->index);
#if defined(BCM63158)
    rdpa_stat_1way_t q_stat = {};
    rdpa_stat_t tx_discard = {};
    int chan_id, rc_id, rdpa_queue;
    uint8_t num_queues;
    uint32_t qid;
    rdpa_if lan_if;
#endif

    memset(stat, 0, sizeof(rdpa_port_stat_t));
    
    rc = _validate_port_index(port->index);
    if (rc)
    {
        rc = (rc == BDMF_ERR_NOT_SUPPORTED ? BDMF_ERR_OK : rc);
        return rc;
    }
    
    /*in case of BCM6846 and BSM6856 these two will not be filled */
    bbh_tx_counters.ddrbyte = 0;
    bbh_tx_counters.debug_cntrs.ddrpkt = 0;
    rc = read_stat_from_hw(port, &rdd_port_counters, &bbh_rx_counters, &bbh_tx_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't clear PM port counters, error = %d\n", rc);

    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(rx_discard_min_length), bbh_rx_counters.pm_counters.too_short);
    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(rx_discard_max_length), bbh_rx_counters.pm_counters.too_long);

    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(rx_discard_2), bbh_rx_counters.pm_counters.no_sbpm_sbn);
    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(bbh_drop_2), bbh_rx_counters.pm_counters.no_sdma_cd);

    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(bbh_drop_3), bbh_rx_counters.pm_counters.disp_cong);

    /* do not display crc errors on  port_stat in case of wan port EPON */
    if (rdpa_if_is_wan(port->index) && (wan_type == rdpa_wan_epon || wan_type == rdpa_wan_xepon))
    {
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(rx_crc_error_pkt), 0);
    }
    else
    {
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(rx_crc_error_pkt), bbh_rx_counters.pm_counters.crc_err);
    }

    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(discard_pkt), rdd_port_counters.rx_drop_pkt);
    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(tx_discard), 0);

#if !defined(BCM63158)    
#ifdef G9991
    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(rx_broadcast_pkt), rdd_port_counters.rx_broadcast_pkt);
    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(rx_multicast_pkt), rdd_port_counters.rx_multicast_pkt);

    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(tx_broadcast_pkt), rdd_port_counters.tx_broadcast_pkt);
    rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
        _OFFSET_(tx_multicast_pkt), rdd_port_counters.tx_multicast_pkt);
#endif
    if (rdpa_if_is_wan(port->index))
    {
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(rx_valid_pkt), bbh_rx_counters.pm_counters.inpkt);

        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_valid_pkt), (bbh_tx_counters.debug_cntrs.ddrpkt + bbh_tx_counters.debug_cntrs.srampkt));
      
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_valid_bytes), (bbh_tx_counters.srambyte + bbh_tx_counters.ddrbyte));

       if (rdpa_is_epon_ae_mode())
       {
            rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
                _OFFSET_(rx_valid_bytes), rdd_port_counters.rx_bytes);
       }
    }
#else
    if (rdpa_if_is_wan(port->index) && rdpa_wan_if_to_wan_type(port->index) != rdpa_wan_gbe && !rdpa_is_epon_ae_mode())
    {
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(rx_valid_pkt), bbh_rx_counters.pm_counters.inpkt);

        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_valid_pkt), (bbh_tx_counters.debug_cntrs.ddrpkt + bbh_tx_counters.debug_cntrs.srampkt));
    }
#endif 
    else
    {
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(rx_valid_pkt), rdd_port_counters.rx_packets);
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(rx_valid_bytes), rdd_port_counters.rx_bytes);

        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_valid_pkt), rdd_port_counters.tx_packets);
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_valid_bytes), rdd_port_counters.tx_bytes);

#ifdef G9991
        /* read g9991_reassembly_discard_frag counter from SRAM */
        if (rdpa_if_is_lan(port->index))
        {
            uint32_t g9991_discard_frag = 0;

            rdd_ag_reas_g9991_reassembly_error_cntrs_table_packets_get((port->index - rdpa_if_lan0), &g9991_discard_frag);
            rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
                _OFFSET_(rx_frag_discard), g9991_discard_frag);
            rdd_ag_reas_g9991_reassembly_error_cntrs_table_packets_set((port->index - rdpa_if_lan0), 0); /* clear counter on read */
        }
#endif
    }

#if defined(BCM63158)
    if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe)
    {
        rc = rdpa_port_get_egress_tm_channel_from_port_ex(rdpa_wan_gbe, port->index, &chan_id);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "unable to convert port index to a proper channel ID, index = %d\n", port->index);

        rc = rdpa_egress_tm_num_queues_get(port->tm_cfg.sched, &num_queues);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't locate tm with port index = %d\n", port->index);
        memset(&tx_discard, 0, sizeof(rdpa_stat_t));
        for (qid = 0; qid < num_queues; qid++)
        {
            rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, chan_id, qid, &rc_id, &rdpa_queue);
            if (!rc)
                rc = _rdpa_egress_tm_tx_queue_drop_stat_read(rdpa_queue, &q_stat);
            if (!rc)
            {
                /* Add up all possible tm_queues belong to this port */
                tx_discard.packets += q_stat.discarded.packets;
                /* The tm_queue drop stat is accumulated at RDPA egress_tm level */
                accumulative_port_stat[port->index].tx_discard = tx_discard.packets;
            }
        }
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_discard), bbh_tx_counters.debug_cntrs.pddrop);
    }

    else if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_dsl)
    {
        rc = rdpa_xtm_txdrop_stats_get(&tx_discard);
        if (!rc)
        {
            accumulative_port_stat[port->index].tx_discard = tx_discard.packets;
        }
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_discard), bbh_tx_counters.debug_cntrs.pddrop);
    }

    else if (rdpa_if_is_lan(port->index))
    {
        lan_if = port->index - rdpa_if_lan0;
        rc = rdpa_egress_tm_num_queues_get(port->tm_cfg.sched, &num_queues);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't locate tm with port index = %d\n", port->index);

        memset(&tx_discard, 0, sizeof(rdpa_stat_t));
        for (qid = 0; qid < num_queues; qid++)
        {
            rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_ds, lan_if, qid, &rc_id, &rdpa_queue);
            if (!rc)
                rc = _rdpa_egress_tm_tx_queue_drop_stat_read(rdpa_queue, &q_stat);
            if (!rc)
            {
                tx_discard.packets += q_stat.discarded.packets;
                /* The tm_queue drop stat is accumulated at RDPA egress_tm level */
                accumulative_port_stat[port->index].tx_discard = tx_discard.packets;
            }
        }
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_port_stat[port->index],
            _OFFSET_(tx_discard), 0);
    }
#endif
    return BDMF_ERR_OK;
}

/* "stat" attribute "write" callback */
int port_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    int rc = 0;
    uint32_t flow_index = 0, cntr_id, tx_cntr_entry_index;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    if (!USE_CNPL_HW_COUNTER(port->index))
        return rc;

    /* clear RX counter */
    flow_index = rdpa_port_rx_flow_index_get(port->index);
    cntr_id = rdd_rx_flow_cntr_id_get(flow_index);

    if (cntr_id == RX_FLOW_CNTR_GROUP_INVLID_CNTR)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "can't get counter id. RX flow index - %d\n", flow_index);

    /* clear TX counter */
    tx_cntr_entry_index = rdpa_port_tx_counter_entry_get(port->index);
    cntr_id = rdd_tm_flow_cntr_id_get(tx_cntr_entry_index);

    if (cntr_id == TX_FLOW_CNTR_GROUP_INVLID_CNTR)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "can't get counter id. TX flow index - %d\n", tx_cntr_entry_index);

    memset(&accumulative_port_stat[port->index], 0, sizeof(rdpa_port_stat_t));
    return rc;
}

/* "pkt_size_stat" attribute "read" callback */
int port_attr_pkt_size_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
#if defined(BCM_PON_XRDP)
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_pkt_size_stat_t *stat = (rdpa_port_pkt_size_stat_t *)val;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    memset(stat, 0, sizeof(rdpa_port_pkt_size_stat_t));

    if (!rdpa_if_is_wan(port->index) || port->wan_type == rdpa_wan_gbe || !port->pkt_size_stat_en)
    /*if (!rdpa_if_is_wan(port->index) || !port->pkt_size_stat_en)*/ /*for debug*/
        return BDMF_ERR_NOT_SUPPORTED; /*do not show counters if feature not enable*/

#if !defined(G9991)
    /* TX counters are not clear on read meanwhile in SRAM*/
    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_64_OCTETS_PKTS, cntr_arr);
    rdpa_common_update_cntr_results_uint32(stat, &accumulate_port_pkt_size_stat[port->index],
        _OFFSET_PKT_SZ_(rx_pkts_64_octets), cntr_arr[0]);

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_65_TO_127_OCTETS_PKTS, cntr_arr);
    rdpa_common_update_cntr_results_uint32(stat, &accumulate_port_pkt_size_stat[port->index],
        _OFFSET_PKT_SZ_(rx_pkts_65to127_octets), cntr_arr[0]);

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_128_TO_255_OCTETS_PKTS, cntr_arr);
    rdpa_common_update_cntr_results_uint32(stat, &accumulate_port_pkt_size_stat[port->index],
        _OFFSET_PKT_SZ_(rx_pkts_128to255_octets), cntr_arr[0]);

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_256_TO_511_OCTETS_PKTS, cntr_arr);
    rdpa_common_update_cntr_results_uint32(stat, &accumulate_port_pkt_size_stat[port->index],
        _OFFSET_PKT_SZ_(rx_pkts_256to511_octets), cntr_arr[0]);

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_512_TO_1023_OCTETS_PKTS, cntr_arr);
    rdpa_common_update_cntr_results_uint32(stat, &accumulate_port_pkt_size_stat[port->index],
        _OFFSET_PKT_SZ_(rx_pkts_512to1023_octets), cntr_arr[0]);

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_1024_TO_1522_OCTETS_PKTS, cntr_arr);
    rdpa_common_update_cntr_results_uint32(stat, &accumulate_port_pkt_size_stat[port->index],
        _OFFSET_PKT_SZ_(rx_pkts_1024to1522_octets), cntr_arr[0]);

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_1523_TO_MTU_OCTETS_PKTS, cntr_arr);
    rdpa_common_update_cntr_results_uint32(stat, &accumulate_port_pkt_size_stat[port->index],
        _OFFSET_PKT_SZ_(rx_pkts_1523tomtu_octets), cntr_arr[0]);
#endif /*G9991*/

    rdd_ag_us_tm_tx_octets_counters_table_packets_get(1, &cntr_arr[0]);
    accumulate_port_pkt_size_stat[port->index].tx_pkts_64_octets = cntr_arr[0];

    rdd_ag_us_tm_tx_octets_counters_table_packets_get(2, &cntr_arr[0]);
    accumulate_port_pkt_size_stat[port->index].tx_pkts_65to127_octets = cntr_arr[0];

    rdd_ag_us_tm_tx_octets_counters_table_packets_get(3, &cntr_arr[0]);
    accumulate_port_pkt_size_stat[port->index].tx_pkts_128to255_octets = cntr_arr[0];

    rdd_ag_us_tm_tx_octets_counters_table_packets_get(4, &cntr_arr[0]);
    accumulate_port_pkt_size_stat[port->index].tx_pkts_256to511_octets = cntr_arr[0];

    rdd_ag_us_tm_tx_octets_counters_table_packets_get(5, &cntr_arr[0]);
    accumulate_port_pkt_size_stat[port->index].tx_pkts_512to1023_octets = cntr_arr[0];

    rdd_ag_us_tm_tx_octets_counters_table_packets_get(6, &cntr_arr[0]);
    accumulate_port_pkt_size_stat[port->index].tx_pkts_1024to1518_octets = cntr_arr[0];

    rdd_ag_us_tm_tx_octets_counters_table_packets_get(7, &cntr_arr[0]);
    accumulate_port_pkt_size_stat[port->index].tx_pkts_1519tomtu_octets = cntr_arr[0];

    if (drv_cntr_get_cntr_non_accumulative())
    {
        int i;
        for (i = 0; i < RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE; i++)
            rdd_ag_us_tm_tx_octets_counters_table_packets_set(i, 0);
    }

    memcpy(stat, &accumulate_port_pkt_size_stat[port->index], sizeof(rdpa_port_pkt_size_stat_t));

    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif /*BCM_PON_XRDP*/
}

/* "pkt_size_stat" attribute "write" callback */
int port_attr_pkt_size_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
#if !defined(BCM63158)
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    memset(&accumulate_port_pkt_size_stat[port->index], 0, sizeof(rdpa_port_pkt_size_stat_t));
    if (rdpa_if_is_wan(port->index) && port->wan_type != rdpa_wan_gbe)
    /*if (rdpa_if_is_wan(port->index))*/ /*for debug*/
        rdpa_port_clear_octets_counters(port->index);
#endif 
    return BDMF_ERR_OK;
}


int port_attr_wan_type_write_ex(port_drv_priv_t *port, rdpa_wan_type wan_type)
{
    return 0;
}

static int _port_ls_fc_cfg_ex(port_drv_priv_t *port, rdpa_port_dp_cfg_t *cfg)
{
    return rdd_ag_processing_vport_cfg_table_ls_fc_cfg_set(rdpa_port_rdpa_if_to_vport(port->index), cfg->ls_fc_enable);
}

int port_ls_fc_cfg_ex(struct bdmf_object *mo, rdpa_port_dp_cfg_t *cfg)
{
#if !defined(BCM63158)
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);

    /* If object is not active, rdpa_if_to_vport mapping is not valid*/
    if (mo->state != bdmf_state_active)
        return BDMF_ERR_OK;

    return _port_ls_fc_cfg_ex(port, cfg);
#endif
    return BDMF_ERR_OK;
}

static int _port_first_queue_in_egress_tm(bdmf_object_handle mirror_dst_port, uint32_t *queue_id)
{
    bdmf_number egress_tm_index;
    rdpa_tm_queue_cfg_t queue_cfg;
    rdpa_tm_level_type level;
    int i, rc = BDMF_ERR_OK;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mirror_dst_port);
    bdmf_object_handle egress_tm =  port->tm_cfg.sched;
  
    if (egress_tm == NULL) 
        BDMF_TRACE_RET(BDMF_ERR_NOENT, "Egress TM not configured for mirror destination port (%d)\n", (int)port->index);
    
    rc = rdpa_egress_tm_level_get(egress_tm, &level);
    if (level == rdpa_tm_level_egress_tm)
    {
         BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, 
             "Mirroring is not supported for mirror destination port (%d) with two level scheduling\n", 
             (int)port->index);
    }
    rc = rc ? rc : rdpa_egress_tm_index_get(egress_tm, &egress_tm_index);
    if (rc)
        BDMF_TRACE_RET(rc, "No egress TM is configured for mirror destination port (%d)\n", (int)port->index);
    for (i = 0; i < RDPA_DFT_NUM_EGRESS_QUEUES; i++)
    {
        rc = rdpa_egress_tm_queue_cfg_get(egress_tm, i, &queue_cfg);
        if (rc)
            continue;		
        if (queue_cfg.queue_id != BDMF_INDEX_UNASSIGNED)
        {
            *queue_id = queue_cfg.queue_id;
            return BDMF_ERR_OK;
        }
    }

    return BDMF_ERR_NOENT;
}

static int _port_mirror_rdpa_to_rdd_queue_cfg(port_drv_priv_t *port, bdmf_object_handle mirror_dst_port,
    uint16_t *qm_queue)
{
    int rc, rc_id, _qm_queue;
    rdpa_if port_index;
    uint32_t queue_id = 0;

    if (mirror_dst_port == NULL)
    {
        *qm_queue = QM_NUM_QUEUES + 1;
        return 0;
    }
    rdpa_port_index_get(mirror_dst_port, &port_index);

    if (port->index == port_index)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Destination mirroring port (%d) is the same as configured port\n", (int)port_index);

    if ((!rdpa_if_is_lan(port_index)) && (!rdpa_if_is_cpu_not_wlan(port_index)))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Destination mirroring port (%d) is WAN or WLAN\n", (int)port_index);
    
    if (rdpa_if_is_cpu_not_wlan(port_index))
    {
      /* For CPU QM queue is known */
      *qm_queue = 0;
    }
    else
    {
      rc = _port_first_queue_in_egress_tm(mirror_dst_port, &queue_id);
      rc = rc ? rc : _rdpa_egress_tm_lan_port_queue_to_rdd(port_index, queue_id, &rc_id, &_qm_queue);
      if (rc)
         BDMF_TRACE_RET(rc, "No valid queue is configured for mirroring destination port (%d)\n", (int)port_index);
      *qm_queue = (uint16_t)_qm_queue;
    }

    return 0;
}

static void port_rx_exception_cfg(port_drv_priv_t *port, uint32_t flag, bdmf_boolean is_set)
{
    uint32_t rx_flow_id;

    if (is_set)
        port->exception_flags |= flag;
    else
        port->exception_flags &= ~flag;

    for (rx_flow_id = 0; rx_flow_id < RX_FLOW_CONTEXTS_NUMBER; rx_flow_id++)
    {
        if (rx_flow_to_vport[rx_flow_id] == rdpa_port_rdpa_if_to_vport(port->index))
            rdd_rx_flow_exception_cfg(rx_flow_id, port->exception_flags ? 1 : 0);
    }
}

static void rx_mirroring_port_cfg(port_drv_priv_t *port, bdmf_boolean enable)
{
    rdd_rx_mirroring_cfg(rdpa_port_rdpa_if_to_vport(port->index), enable);
    port_rx_exception_cfg(port, EXCEPTION_RX_MIRROR, enable);
    if (rdpa_if_is_wan(port->index))
        rdd_rx_mirroring_direct_cfg(enable);
}

static int port_rdd_mirror_cfg(port_drv_priv_t *port, rdpa_port_mirror_cfg_t *mirror_cfg,
    rdd_mirroring_cfg_t *rdd_mirroring_cfg)
{
    int rc;

    rdd_mirroring_cfg->lan = rdpa_if_is_lan(port->index);
    rdd_mirroring_cfg->wlan_radio_idx = -1;
    if (rdpa_if_is_wlan(port->index))
    {
        rdd_mirroring_cfg->wlan_radio_idx = port->index - rdpa_if_wlan0;
    }

    rc = _port_mirror_rdpa_to_rdd_queue_cfg(port, mirror_cfg->rx_dst_port,
         &(rdd_mirroring_cfg->rx_dst_queue));
    rc = rc ? rc : _port_mirror_rdpa_to_rdd_queue_cfg(port, mirror_cfg->tx_dst_port,
         &(rdd_mirroring_cfg->tx_dst_queue));
    if (rc)
        return rc;

    if (rdd_mirroring_cfg->rx_dst_queue <= QM_NUM_QUEUES) /* queue_id = QM_NUM_QUEUES comes for drop */
    {
        rdpa_if rx_dst_port_idx;

        rc = rdpa_port_index_get(mirror_cfg->rx_dst_port, &rx_dst_port_idx);
        if (!rc)
        {
            rdd_mirroring_cfg->rx_dst_vport = rdpa_if_to_rdd_vport_map[rx_dst_port_idx];
        }
    }

    if (rc)
        return rc;

    if (rdd_mirroring_cfg->tx_dst_queue <= QM_NUM_QUEUES)
    {
        rdpa_if tx_dst_port_idx;

        rc = rdpa_port_index_get(mirror_cfg->tx_dst_port, &tx_dst_port_idx);
        if (!rc)
        {
            rdd_mirroring_cfg->tx_dst_vport = rdpa_if_to_rdd_vport_map[tx_dst_port_idx];
        }
    }

    return rc;
}

int port_mirror_cfg_ex(struct bdmf_object *mo, port_drv_priv_t *port, rdpa_port_mirror_cfg_t *mirror_cfg)
{
    int rc = 0;
    rdd_mirroring_cfg_t rdd_mirroring_cfg;
    _init_rdd_mirroring_cfg(port, &rdd_mirroring_cfg);
    
    if (mo->state == bdmf_state_active)
    {
        rc = port_rdd_mirror_cfg(port, mirror_cfg, &rdd_mirroring_cfg);
        rc = rc ? rc : _port_mirror_cfg_tx(port, mirror_cfg, &rdd_mirroring_cfg);
        rc = rc ? rc : _port_mirror_cfg_rx(port, mirror_cfg, &rdd_mirroring_cfg);
    }
    return rc;
}

int port_attr_loopback_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_port_loopback_t *lb_req = (rdpa_port_loopback_t *)val;
    int rc_id, qm_queue, channel, tc = 0, rc = 0;

    if (lb_req->op == rdpa_loopback_op_remote)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Remote loopback not supported\n");

    if (lb_req->type == rdpa_loopback_type_mac || lb_req->type == rdpa_loopback_type_phy)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "RDPA supports only firmware based loopback\n");

    if (lb_req->type == port->loopback_cfg.type && lb_req->op == port->loopback_cfg.op && 
        lb_req->queue == port->loopback_cfg.queue)
        return 0;

    if (rdpa_if_is_wan(port->index))
    {
        rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(port->index, lb_req->wan_flow, lb_req->queue, &channel, &rc_id, &qm_queue, &tc);
        if (!rc)
            rdd_loopback_wan_flow_set(lb_req->wan_flow);
    }
    else
        rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port->index, lb_req->queue, &rc_id, &qm_queue);
    if (rc)
       BDMF_TRACE_RET(rc, "Invalid queue ID\n");

    rdd_loopback_queue_set(rdpa_port_rdpa_if_to_vport(port->index), qm_queue);
    rdd_loopback_cfg(rdpa_port_rdpa_if_to_vport(port->index), FW_LOOPBACK_EN(lb_req));

    port_rx_exception_cfg(port, EXCEPTION_PORT_FW_LOOPBACK, FW_LOOPBACK_EN(lb_req));

    /* save in private data */
    port->loopback_cfg.type = lb_req->type;
    port->loopback_cfg.op = lb_req->op;
    port->loopback_cfg.queue = lb_req->queue;
    port->loopback_cfg.wan_flow = lb_req->wan_flow;

    return 0;
}

int port_attr_options_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
#if !defined(BCM63158)
    uint32_t options = *(uint32_t *)val;
#endif

    rdd_vport_id_t rdd_port = rdpa_port_rdpa_if_to_vport(port->index);
    rdd_vport_cfg_entry_t vport_entry = {};

    rc = rdd_ag_processing_vport_cfg_entry_get(rdd_port, &vport_entry);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Failed to get vport_cfg for options, vport=%d\n", rdd_port);

#if !defined(BCM63158)
    vport_entry.anti_spoofing_bypass = ((options & (1 << rdpa_anti_spoofing_bypass_option)) >> rdpa_anti_spoofing_bypass_option);
    port->options = options;
#endif
    rc = rdd_ag_processing_vport_cfg_entry_set(rdd_port, &vport_entry);

    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Failed to set vport_cfg for options, vport=%d\n", rdd_port);

    return rc;
}

int port_attr_pfc_tx_enable_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
#if defined(BCM63158)
    uint8_t *v = val;
    rdd_ag_us_tm_pfc_tx_enable_get(v);
    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int port_attr_pfc_tx_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#if defined(BCM63158)
    uint8_t v = *(uint8_t *)val;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    bbh_id_e bbh_id = rdpa_port_to_bbh_id(port);
    int rc = 0, i;

    rc = ag_drv_bbh_rx_exc_en_set(bbh_id, v);
    rc = rc ? rc : ag_drv_bbh_rx_pfc_en_set(bbh_id, v);

    rc = rc ? : rdd_ag_us_tm_pfc_tx_enable_set(v);
    /* Clear PFC status if it is disable, to cover current no timer limitation */
    if (v == 0)
    {
        for (i = 0; i < 8; i++)
            rc = rc ? rc : rdd_ag_us_tm_pfc_tx_status_table_set(i, 0);
    }
    return rc;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int port_attr_pfc_tx_timer_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
#if defined(BCM63158)
    uint32_t *v = val;
    int rc;

    rc = rdd_ag_us_tm_pfc_tx_status_table_get(index, v);
    return rc;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int port_attr_pfc_tx_timer_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#if defined(BCM63158)
    uint32_t *v = (uint32_t *)val;
    int rc;

    rc = rdd_ag_us_tm_pfc_tx_status_table_set(index, *v);

    return rc;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int port_vlan_isolation_cfg_ex(struct bdmf_object *mo,
    rdpa_port_vlan_isolation_t *vlan_isolation_cfg, bdmf_boolean is_active)
{
    int rc = 0;

#ifdef CONFIG_RNR_BRIDGE
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdd_vport_id_t vport;
    int ingress_iso, egress_iso;

    /* rdpa_if to vport mapping is valid only for active port object */
    if (!is_active)
        return 0;

    vport = rdpa_port_rdpa_if_to_vport(port->index);

    if (rdpa_if_is_wan(port->index))
    {
        ingress_iso = vlan_isolation_cfg->ds;
        egress_iso = vlan_isolation_cfg->us;
    }
    else
    {
        ingress_iso = vlan_isolation_cfg->us;
        egress_iso = vlan_isolation_cfg->ds;
    }

    rc = rdd_ag_processing_vport_cfg_table_egress_isolation_en_set(vport, egress_iso ? 1 : 0); 
    rc = rc ? rc : rdd_ag_processing_vport_cfg_table_ingress_isolation_en_set(vport, ingress_iso ? 1 : 0); 
    if (rc)
    {
        BDMF_TRACE_RET(rc, "Failed to configure VLAN isolation for port %s, rc = %d\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
    }

    /* For all VLAN objects owned by this port, update the eligibility matrix if isolation is turned on. */
    if (port->bridge_obj)
    {
        bridge_drv_priv_t *bridge = (bridge_drv_priv_t *)bdmf_obj_data(port->bridge_obj);
        bdmf_object_handle vlan_obj = NULL;
        rdpa_ports fwd_mask;

        /* iterate on VLAN childs and update forwarding vector if necessary */
        fwd_mask = (ingress_iso || egress_iso) ? bridge->port_fw_elig[port->index] : 0;
        while ((vlan_obj = bdmf_get_next_child(mo, "vlan", vlan_obj)) != NULL)
        {
            vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(vlan_obj);

            if (vlan->linked_bridge)
                continue;

            rc = vlan_ctx_update_invoke(vlan_obj, rdpa_vlan_isolation_vector_modify_cb, &fwd_mask);
            if (rc)
            {
                bdmf_put(vlan_obj);
                BDMF_TRACE_RET(rc, "Failed to configure new isolation vector for VLAN object '%s', rc %d\n",
                    vlan_obj->name, rc);
            }
        }
    }
    
    rc = update_port_bridge_and_vlan_lookup_method_ex(port->index);
    if (rc)
    {
        BDMF_TRACE_RET(rc, "Failed to update VLAN lookup method for port %s, rc = %d\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index), rc);
    }
#endif
    
    return rc;
}

int port_ingress_rate_limit_cfg_ex(port_drv_priv_t *port, rdpa_port_ingress_rate_limit_t *ingress_rate_limit)
{
    bdmf_error_t rc = BDMF_ERR_OK;
#ifndef BCM63158
    rdd_vport_id_t vport_idx = rdpa_port_rdpa_if_to_vport(port->index);
    policer_drv_priv_t *policer;

    /* When policer deleted all rate limit values initialized */
    if (!ingress_rate_limit->policer)
    {
        port->ingress_rate_limit.policer = NULL;
        port->ingress_rate_limit.traffic_types = 0;
        rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_multicast_set(vport_idx, 0);
        rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_broadcast_set(vport_idx, 0);
        rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_unknown_da_set(vport_idx, 0);
        rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_all_traffic_set(vport_idx, 0);
        port_rx_exception_cfg(port, EXCEPTION_PORT_RATE_LIMIT_ALL_TRAFFIC, 0);
        return rc;
    }

    policer = (policer_drv_priv_t *)bdmf_obj_data(ingress_rate_limit->policer);
    if (policer->cfg.type != rdpa_tm_policer_single_token_bucket)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Only single bucket policer type allowed in port ingress_rate_limit!\n");

    /* Write the policer index to VPORT_CFG table */
    rc = rdd_ag_processing_vport_cfg_ex_table_policer_idx_set(vport_idx, policer->hw_index);
    /* Write the traffic types to VPORT_CFG table */
    rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_broadcast_set(vport_idx,
                    ((ingress_rate_limit->traffic_types) & RDPA_RATE_LIMIT_MASK_BROADCAST) >> RDPA_RATE_LIMIT_BROADCAST);
    rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_multicast_set(vport_idx,
                    ((ingress_rate_limit->traffic_types) & RDPA_RATE_LIMIT_MASK_MULTICAST) >> RDPA_RATE_LIMIT_MULTICAST);
    rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_unknown_da_set(vport_idx,
                    ((ingress_rate_limit->traffic_types) & RDPA_RATE_LIMIT_MASK_UNKNOWN_DA) >> RDPA_RATE_LIMIT_UNKNOWN_DA);
    rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_rate_limit_all_traffic_set(vport_idx,
                    ((ingress_rate_limit->traffic_types) & RDPA_RATE_LIMIT_MASK_ALL_TRAFFIC) >> RDPA_RATE_LIMIT_ALL_TRAFFIC);
    port_rx_exception_cfg(port, EXCEPTION_PORT_RATE_LIMIT_ALL_TRAFFIC,
                            ((ingress_rate_limit->traffic_types) & RDPA_RATE_LIMIT_MASK_ALL_TRAFFIC) >> RDPA_RATE_LIMIT_ALL_TRAFFIC);

    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PARM, " Write ingress rate limit configuration failed!\n");
    /* Save configuration */
    port->ingress_rate_limit.traffic_types = ingress_rate_limit->traffic_types;
    port->ingress_rate_limit.policer = ingress_rate_limit->policer;
#endif
    return rc;
}

int port_flow_control_cfg_ex(port_drv_priv_t *port, rdpa_port_flow_ctrl_t *flow_ctrl)
{
    bdmf_error_t rc = BDMF_ERR_OK;
#ifndef BCM63158
    rdd_vport_id_t vport_idx = rdpa_port_rdpa_if_to_vport(port->index);
    bbh_id_e bbh_id = rdpa_port_to_bbh_id(port);

    if (flow_ctrl->ingress_congestion) {
#ifdef FLOW_CTRL_SUPPORT
        /* disable rate limit flow control */
        port_rx_exception_cfg(port, EXCEPTION_PORT_FLOW_CTRL, 0);
#endif
        if ((!rdpa_is_fttdp_mode() && (rdpa_if_is_lan(port->index))) ||
            ((rdpa_if_is_wan(port->index)) && (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe))) {
            rc = rdd_ag_processing_vport_cfg_table_ingress_congestion_set(vport_idx, flow_ctrl->ingress_congestion);
            rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, (uint32_t)((BBH_FREQUENCY / 1000000) * 20));  /* 20 usec */
        }
#ifdef G9991
        else if (rdpa_if_is_lag_and_switch(port->index)) 
        {
            rdd_bb_id bb_id_rx = rdpa_emac_to_bb_id_rx[port->cfg.emac];

            rc = rdd_g9991_ingress_congestion_flow_control_enable(bb_id_rx, flow_ctrl->ingress_congestion);

    	    rc =  (bbh_id == BBH_ID_NULL) ? BDMF_ERR_INTERNAL : rc;

            rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, (uint32_t)((BBH_FREQUENCY / 1000000) * 20));  /* 20 usec */
        }
        else if (rdpa_is_fttdp_mode() && rdpa_if_is_lan(port->index) && (port->index == rdpa_if_lan29))
        {
            rdd_bb_id bb_id_rx = rdpa_emac_to_bb_id_rx[port->cfg.emac];
            rc = rdd_g9991_ingress_congestion_flow_control_enable(bb_id_rx, flow_ctrl->ingress_congestion);
        }
#endif
    }
    else
    {
        if ((!rdpa_is_fttdp_mode() && (rdpa_if_is_lan(port->index))) ||
            ((rdpa_if_is_wan(port->index)) && (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe))) 
        {
            rc = rdd_ag_processing_vport_cfg_table_ingress_congestion_set(vport_idx, flow_ctrl->ingress_congestion);
            rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, (uint32_t)(0x1000));   /* reset value */
        }
#ifdef G9991
        else if (rdpa_if_is_lag_and_switch(port->index))
        {
            rdd_bb_id bb_id_rx = rdpa_emac_to_bb_id_rx[port->cfg.emac];
            rc = rdd_g9991_ingress_congestion_flow_control_enable(bb_id_rx, flow_ctrl->ingress_congestion);

    	    rc =  (bbh_id == BBH_ID_NULL) ? BDMF_ERR_INTERNAL : rc;
            rc = rc ? rc : ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, (uint32_t)(0x1000));   /* reset value */
        }
        else if (rdpa_is_fttdp_mode() && rdpa_if_is_lan(port->index) && (port->index == rdpa_if_lan29)) {
            rdd_bb_id bbh_id_rx = rdpa_emac_to_bb_id_rx[port->cfg.emac];

            rc = rdd_g9991_ingress_congestion_flow_control_enable(bbh_id_rx, flow_ctrl->ingress_congestion);
        }
#endif
    }
#endif

    /* Save configuration */
    port->flow_ctrl.ingress_congestion = flow_ctrl->ingress_congestion;

#ifdef FLOW_CTRL_SUPPORT /* flow control not relevant for 63158 chip */
    if (!flow_ctrl->ingress_congestion) {
       uint8_t emacs_flow_ctrl_vec;

       /* Flow Ctrl */
       if (flow_ctrl->rate > 0)
       {
           port_rx_exception_cfg(port, EXCEPTION_PORT_FLOW_CTRL, 1);
           rc = rc ? rc : rdd_ag_processing_vport_cfg_table_flow_control_set(vport_idx, 1);
           rc = rc ? rc : rdd_ag_processing_emac_flow_ctrl_rate_set(port->cfg.emac, flow_ctrl->rate);
           rc = rc ? rc : rdd_ag_processing_emac_flow_ctrl_max_burst_size_set(port->cfg.emac, flow_ctrl->mbs);
           rc = rc ? rc : rdd_ag_processing_emac_flow_ctrl_threshold_set(port->cfg.emac, flow_ctrl->threshold);
           rc = rc ? rc : rdd_ag_timer_common_emac_flow_ctrl_budget_budget_set(port->cfg.emac, ((flow_ctrl->rate /
               (CNPL_SECOND_TO_US/TIMER_COMMON_PERIOD_IN_USEC))/8));
           /* In low rates (<10Mbps) need to add budget remainder for policing accuracy */
           if (flow_ctrl->rate < 10000000)
           {
               rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_remainder_set(port->cfg.emac, ((((((flow_ctrl->rate * 1000) /
                          (CNPL_SECOND_TO_US/TIMER_COMMON_PERIOD_IN_USEC))/8) % 1000) * CNTR_REMAINDER_PERIOD) / 1000));
           }
           /* clear the counter */
           rc = rc ? rc : drv_cntr_counter_clr(CNTR_GROUP_EMAC_FLOW_CTRL, port->cfg.emac);
           /* wake up timer for periodic flow ctrl task if wasn't already waked up and update the vector*/
           rc = rc ? rc : rdd_ag_timer_common_emac_flow_ctrl_vector_get(&emacs_flow_ctrl_vec);
           if (!emacs_flow_ctrl_vec)
               rc = rc ? rc : common_timer_init();
           emacs_flow_ctrl_vec = emacs_flow_ctrl_vec | (1 << port->cfg.emac);
           rc = rc ? rc : rdd_ag_timer_common_emac_flow_ctrl_vector_set(emacs_flow_ctrl_vec);
           /* Write emac number to SRAM */
           rc = rc ? rc : rdd_ag_processing_vport_cfg_ex_table_emac_idx_set(vport_idx, port->cfg.emac);
           /* config BBH timer due to rate */
           rc = rc ? rc : port_set_bbh_timer_clock_ex(port);
       }
       else
       {
           /* Reset flow ctrl configuration */
           port_rx_exception_cfg(port, EXCEPTION_PORT_FLOW_CTRL, 0);
           rc = rc ? rc : rdd_ag_processing_vport_cfg_table_flow_control_set(vport_idx, 0);
           rc = rc ? rc : rdd_ag_timer_common_emac_flow_ctrl_vector_get(&emacs_flow_ctrl_vec);
           emacs_flow_ctrl_vec = emacs_flow_ctrl_vec & (0xFF ^ (1 << port->cfg.emac));
           rc = rc ? rc : rdd_ag_timer_common_emac_flow_ctrl_vector_set(emacs_flow_ctrl_vec);
           rc = rc ? rc : rdd_ag_timer_common_emac_flow_ctrl_budget_budget_set(port->cfg.emac, 0);
           rc = rc ? rc : rdd_ag_timer_common_fw_policer_budget_remainder_set(port->cfg.emac, 0);
       }

       if (rc)
           BDMF_TRACE_RET(BDMF_ERR_PARM, " Write flow control configuration failed!\n");
       /* Save configuration */
       port->flow_ctrl.rate = flow_ctrl->rate;
       port->flow_ctrl.mbs = flow_ctrl->mbs;
       port->flow_ctrl.threshold = flow_ctrl->threshold;
    }
#endif
    return rc;
}

#ifndef BCM63158
bdmf_error_t common_timer_init(void)
{
    RDD_TIMER_COMMON_TIMER_VALUE_DTS *entry;
    bdmf_error_t rc = BDMF_ERR_OK;

    entry = RDD_TIMER_COMMON_TIMER_VALUE_PTR(get_runner_idx(timer_common_runner_image));
    RDD_BYTES_2_BITS_WRITE(TIMER_COMMON_PERIOD, entry);

    /* Make sure the timer configured before the cpu weakeup */
    WMB();

    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(timer_common_runner_image), TIMER_COMMON_THREAD_NUMBER);
    return rc;
}
#endif

int port_set_bbh_timer_clock_ex(port_drv_priv_t *port)
{
    bbh_id_e bbh_id;
    /* Configure the BBH FC timeout */
    bbh_id = rdpa_port_to_bbh_id(port);
    if (bbh_id == BBH_ID_NULL)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "port to bbh_id failed\n");
    if (ag_drv_bbh_rx_flow_ctrl_timer_set(bbh_id, (uint32_t)BBH_TIMER_MIN))
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Flow control timer configuration failed\n");
    return 0;
}

/* NOTE : Reverse mapping - rdd_vport to rdpa_if is NOT used
 * if required in future, it is better that we provide it
 * from the array rdd_vport_to_rdpa_if_map[] instead of reversing the logic */

rdd_vport_id_t rdpa_if_to_rdd_vport(rdpa_if port, rdpa_wan_type wan_type)
{
    rdd_vport_id_t vport;

    switch (port)
    {
#if defined(BCM63158)
    case rdpa_if_wan0 ... rdpa_if_wan_max:
        {
            switch (wan_type)
            {
            case rdpa_wan_epon:
            case rdpa_wan_xepon:
            case rdpa_wan_gpon:
            case rdpa_wan_xgpon:
                vport = RDD_PON_WAN_VPORT;
                break;
            case rdpa_wan_dsl:
                vport = RDD_DSL_WAN_VPORT;
                break;
            case rdpa_wan_gbe:
                vport = RDD_ETH_WAN_VPORT;
                break;
            default:
                BDMF_TRACE_ERR("Can't map rdpa_if %d wan_type %d to rdd vport\n", port, wan_type);
                vport = RDD_WAN0_VPORT;
                break;
            }
        }
        break;
#else
    case rdpa_if_wan0:
#if defined(CONFIG_MULTI_WAN_SUPPORT) && defined(RDD_WAN1_VPORT)
        if (wan_type == rdpa_wan_gbe)
            vport = RDD_WAN1_VPORT;
        else
            vport = RDD_WAN0_VPORT;
#else
        vport = RDD_WAN0_VPORT;
#endif
        break;
#if defined(RDD_WAN1_VPORT)
    case rdpa_if_wan1:
#if defined(CONFIG_MULTI_WAN_SUPPORT)
        if (wan_type == rdpa_wan_gbe)
            vport = RDD_WAN1_VPORT;
        else
            vport = RDD_WAN0_VPORT;
#else
        vport = RDD_WAN1_VPORT;
#endif
        break;
#endif /* RDD_WAN1_VPORT */
#if defined(RDD_WAN2_VPORT)
    case rdpa_if_wan2:
        vport = RDD_WAN2_VPORT;
        break;
#endif /* RDD_WAN2_VPORT */
#endif
    case rdpa_if_lan0 ... rdpa_if_lan_max:
        vport = (port - rdpa_if_lan0) + RDD_LAN0_VPORT;
        break;
    case rdpa_if_cpu_first ... rdpa_if_cpu_last:
        vport = RDD_CPU_VPORT_FIRST + port - rdpa_if_cpu_first;
        break;
#if defined(RDD_VIRTUAL_VPORT)
    case rdpa_if_lag0 ... rdpa_if_lag4:
        vport = RDD_VIRTUAL_VPORT;
        break;
    case rdpa_if_switch:
        vport = RDD_VIRTUAL_VPORT;
        break;
    case rdpa_if_bond0 ... rdpa_if_bond2:
        vport = RDD_VIRTUAL_VPORT;
        break;
#endif /* RDD_VIRTUAL_VPORT */
    default:
        if (port != rdpa_if_none)
            BDMF_TRACE_ERR("Can't map rdpa_if %d to rdd vport\n", port);
        vport = RDD_WAN0_VPORT;
        break;
    }

    return vport;
}

int rdpa_port_lag_link_ex(port_drv_priv_t *lag_port)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

void rdpa_port_lag_unlink_ex(port_drv_priv_t *lag_port)
{
    /* XXX: Temporary not supported */
}

int rdpa_port_bond_link_ex(rdpa_physical_port physical_port)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_port_bond_unlink_ex(rdpa_physical_port physical_port)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_port_get_egress_tm_channel_from_port_ex(rdpa_wan_type wan_type, rdpa_if port, int *channel_id)
{
    if (rdpa_if_is_wan(port))
    {
#if defined(BCM63158)
        if (wan_type == rdpa_wan_gbe)
        {
            if (rdpa_port_rdpa_if_to_emac(port) == rdpa_emac4)
                *channel_id = RDD_US_CHANNEL_OFFSET_AE10;
            else /* if (rdpa_port_rdpa_if_to_emac(port) == rdpa_emac5) */
                *channel_id = RDD_US_CHANNEL_OFFSET_AE2P5;
        }
        else
#endif
        *channel_id = port - rdpa_if_wan0;
    }
    else
    {
#ifndef G9991
        *channel_id = rdpa_port_rdpa_if_to_emac(port);
#else /* XRDP - G9991 */
        *channel_id = port - rdpa_if_lan0;
#endif
    }
    return 0;
}

int port_attr_cpu_obj_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle cpu_obj = *(bdmf_object_handle *)val;

    if (mo->state != bdmf_state_init)
    {
        /* VPORT is already set, configure */
        _port_cpu_obj_set(port, cpu_obj);
    }
    port->cpu_obj = cpu_obj;
    return 0;
}

int port_attr_cpu_meter_write_ex(struct bdmf_object *mo, rdpa_traffic_dir dir, rdpa_if intf, bdmf_index meter_idx)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port->index);
    rdd_cpu_rx_meter rdd_meter = (meter_idx != BDMF_INDEX_UNASSIGNED) ? meter_idx : CPU_RX_METER_DISABLE;

    if (rdpa_if_is_wan(port->index) && !rdpa_is_epon_ae_mode() && (rdpa_wan_if_to_wan_type(port->index) != rdpa_wan_gbe))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Can't configure meter on WAN port\n");
    }
    
    if (vport == PROJ_DEFS_RDD_VPORT_LAST)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Can't translate to vport\n");
    }

    GROUP_MWRITE_8(RDD_CPU_VPORT_TO_METER_TABLE_ADDRESS_ARR, vport, rdd_meter);

    return BDMF_ERR_OK;
}

int port_attr_ingress_filter_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
#ifdef INGRESS_FILTERS
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_ctrl_t *ctrl = (rdpa_filter_ctrl_t *)val;

    if (mo->state == bdmf_state_active)
    {
        return ingress_filter_entry_set((rdpa_filter)index, mo, port->ingress_filters, ctrl,
            &port->ingress_filters_profile);
    }
    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

uint32_t disabled_proto_mask_get(uint32_t proto_filters_mask)
{
    uint32_t dis_proto_mask = 0;

    if (proto_filters_mask & (1 << rdpa_proto_filter_any))
        return 0;

    dis_proto_mask = ~proto_filters_mask;
    dis_proto_mask &= (1 << rdpa_proto_filter_last) - 1;

    return dis_proto_mask;
}

static int _proto_filters_update(port_drv_priv_t *port, uint32_t proto_filters)
{
    uint32_t dis_proto_mask = disabled_proto_mask_get(proto_filters);
    rdd_vport_id_t rdd_vport = rdpa_port_rdpa_if_to_vport(port->index);

    return rdd_ag_processing_vport_cfg_table_protocol_filters_dis_set(rdd_vport, dis_proto_mask);
}

int port_attr_proto_filters_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t proto_filters = *(uint32_t *)val;
    int rc = 0;
   
    if (mo->state == bdmf_state_active)
        rc = _proto_filters_update(port, proto_filters);
    if (!rc)
        port->proto_filters = proto_filters;

    return rc;
}

int _rdpa_port_set_linked_bridge_ex(struct bdmf_object *mo, bdmf_object_handle bridge_obj)
{
#ifdef CONFIG_RNR_BRIDGE
    bridge_drv_priv_t *bridge;
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port->index);
    bdmf_index bridge_id;
    bdmf_object_handle vlan_obj = NULL;
    int rc;

    if (bridge_obj) /* Link to new bridge */
    {
        bridge = (bridge_drv_priv_t *)bdmf_obj_data(bridge_obj);
        bridge_id = bridge->index;

        if (port->bridge_obj)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "Can't link port. Already linked to another bridge\n");

        rc = rdd_ag_processing_vport_cfg_table_bridge_id_set(vport, bridge_id);

        if (rc)
            return rc;
    }
    else /* Unlink */
    {
        if (!port->bridge_obj)
            return 0;

        bridge_id = NO_BRIDGE_ID;

        rc = rdd_ag_processing_vport_cfg_table_bridge_id_set(vport, bridge_id);

        if (rc)
            return rc;
    }

    /* iterate on VLAN childs and all which are not linked to a Q-bridge */
    while ((vlan_obj = bdmf_get_next_child(mo, "vlan", vlan_obj)) != NULL)
    {
        vlan_drv_priv_t *vlan = (vlan_drv_priv_t *)bdmf_obj_data(vlan_obj);

        /* If Vlan object is not linked to any bridge */
        if (!vlan->linked_bridge)
        {
            rc = vlan_ctx_update_invoke(vlan_obj, rdpa_vlan_bridge_id_modify_cb, &bridge_id);
            if (rc)
            {
                rdd_ag_processing_vport_cfg_table_bridge_id_set(vport, NO_BRIDGE_ID);
                return rc;
            }
        }
    }
#endif
    return BDMF_ERR_OK;
}

int rdpa_port_cfg_min_packet_size_get_ex(port_drv_priv_t *port, uint8_t *min_packet_size)
{
    uint16_t max_packet_size;
    bbh_id_e bbh_id = rdpa_port_to_bbh_id(port);
    int rc;

    if (bbh_id == BBH_ID_NULL)
    {
        BDMF_TRACE_DBG("Can't get min packet size; Illegal BBH_ID; port - %s!\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
        return BDMF_ERR_NOT_SUPPORTED;  
    }

    rc = drv_bbh_rx_pkt_size_get(bbh_id, 0, min_packet_size, &max_packet_size);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't get min packet size; port - %s!\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
    }
    return BDMF_ERR_OK;    
}

int rdpa_port_cfg_min_packet_size_set_ex(port_drv_priv_t *port, uint8_t min_packet_size)
{
    uint8_t min_packet_size_tmp;
    uint16_t max_packet_size;
    bbh_id_e bbh_id = rdpa_port_to_bbh_id(port);
    int rc;

    if (bbh_id == BBH_ID_NULL)
    {
        BDMF_TRACE_DBG("Can't set min packet size; Illegal BBH_ID; port - %s!\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
        return BDMF_ERR_NOT_SUPPORTED;  
    }

    rc = drv_bbh_rx_pkt_size_get(bbh_id, 0, &min_packet_size_tmp, &max_packet_size);
    if (rc)
    {
        BDMF_TRACE_DBG("Can't get min packet size; port - %s!\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
        return rc;  
    }
    rc = drv_bbh_rx_pkt_size_set(bbh_id, 0, min_packet_size, max_packet_size);
    if (rc)
    {
        BDMF_TRACE_DBG("Can't set min packet size; port - %s!\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));
        return rc;  
    }
    return BDMF_ERR_OK;    
}

static inline int _delete_mirrored_port_cfg(port_drv_priv_t *port, bdmf_boolean is_rx_mirror)
{
    int wlan_radio_idx = -1;
    if (is_rx_mirror)
    {
        if (port->mirror_cfg.rx_dst_port)
        {
            /* port mirror disable is required */
            BUG_ON(mirroring_cfg.rx_ref_cntr == 0);
            mirroring_cfg.rx_ref_cntr--;
            rx_mirroring_port_cfg(port, 0);
            if (!mirroring_cfg.rx_ref_cntr)
                mirroring_cfg.port_mirror_cfg.rx_dst_port = NULL;
        }
        else
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Invalid rx_dst_port configuration, port %s was not configured for mirroring before\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));                    
        }
    }
    else if (port->mirror_cfg.tx_dst_port) /* tx mirroring case */
    {
        /* port mirror disable is required */
        BUG_ON(mirroring_cfg.tx_ref_cntr == 0);
        mirroring_cfg.tx_ref_cntr--;
        if (rdpa_if_is_wlan(port->index))
        {
            wlan_radio_idx = rdpa_if_id(port->index) - rdpa_if_id(rdpa_if_wlan0);
        }
        rdd_mirroring_set_tx_src_en(rdpa_if_is_lan(port->index), wlan_radio_idx, rdpa_port_to_bbh_id(port), 0);
        if (!mirroring_cfg.tx_ref_cntr)
            mirroring_cfg.port_mirror_cfg.tx_dst_port = NULL;

        if (mirroring_cfg.tx_mirror_copy_to_ddr_prev[port->index] != -1 && rdpa_if_is_lan(port->index))
        {
            _tx_mirror_set_copy_to_ddr(port, 0);
        }
    }
    else
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Invalid tx_dst_port configuration, port %s was not configured for mirroring before\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));                    
    }
    return BDMF_ERR_OK;
}

/* set up tx mirroring configuration */
static int _port_mirror_cfg_tx(port_drv_priv_t *port, rdpa_port_mirror_cfg_t *new_port_mirror,
    rdd_mirroring_cfg_t *rdd_mirroring_cfg)
{
    int rc = BDMF_ERR_OK;

    if (port->mirror_cfg.tx_dst_port == new_port_mirror->tx_dst_port)
        return BDMF_ERR_OK; /* tx_mirroring is the same */

    if (!new_port_mirror->tx_dst_port)
    {
        rc = _delete_mirrored_port_cfg(port, TX_MIRRORING);
        goto exit;
    }

    /* Changing mirror port is allowed: only if one instance of port mirroring was set 
     * and we are changing this instance */
    if (mirroring_cfg.port_mirror_cfg.tx_dst_port != new_port_mirror->tx_dst_port && mirroring_cfg.tx_ref_cntr)
    {
        if (port->mirror_cfg.tx_dst_port == mirroring_cfg.port_mirror_cfg.tx_dst_port && mirroring_cfg.tx_ref_cntr == 1)
            goto exit; /* changing of mirroring port is enabled for only one port connected */

        BDMF_TRACE_RET(BDMF_ERR_PARM, "Not supported changing of already configured TX mirror port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, 
            ((port_drv_priv_t *)(bdmf_obj_data(mirroring_cfg.port_mirror_cfg.tx_dst_port)))->index));    
    }

    mirroring_cfg.tx_ref_cntr++;


exit:
    /*not needed rdd_mirroring_set_tx_src_en(rdpa_if_is_lan(port->index), rdpa_port_to_bbh_id(port), (new_port_mirror->tx_dst_port != NULL));*/

    /* Save configuration */
    if (mirroring_cfg.tx_ref_cntr)
    {
        if (new_port_mirror->tx_dst_port)
        {
            mirroring_cfg.port_mirror_cfg.tx_dst_port = new_port_mirror->tx_dst_port; 
            rdd_mirroring_set_tx(rdd_mirroring_cfg);
            _tx_mirror_set_copy_to_ddr(port, 1);
        }
    }
    else
    {
        mirroring_cfg.port_mirror_cfg.tx_dst_port = NULL; 
        rdd_mirror_tx_disable(); /* disable tx mirroring in FW */
    }

    return rc;
}

/* set up port rx mirroring configuration */
static int _port_mirror_cfg_rx(port_drv_priv_t *port, rdpa_port_mirror_cfg_t *new_port_mirror,
    rdd_mirroring_cfg_t *rdd_mirroring_cfg)
{
    int rc = BDMF_ERR_OK;

#if defined(G9991)
    if (!rdpa_if_is_wan(port->index) && port->index != rdpa_if_lan29 &&  !rdpa_if_is_cpu_not_wlan(port->index))
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "This port %s is not allowed for RX mirroring operation\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port->index));    
    }
#endif

    if (port->mirror_cfg.rx_dst_port == new_port_mirror->rx_dst_port)
        return BDMF_ERR_OK; /* rx_mirroring is the same */

    if (!new_port_mirror->rx_dst_port)
    {
        rc = _delete_mirrored_port_cfg(port, RX_MIRRORING);
        goto exit;
    }

    /* Changing mirror port is allowed: only if one instance of port mirroring was set 
     * and we are changing this instance */
    if (mirroring_cfg.port_mirror_cfg.rx_dst_port != new_port_mirror->rx_dst_port && mirroring_cfg.rx_ref_cntr)
    {
        if (port->mirror_cfg.rx_dst_port == mirroring_cfg.port_mirror_cfg.rx_dst_port && mirroring_cfg.rx_ref_cntr == 1)
            goto exit; /* changing of mirroring port is enabled for only one port connected */

        BDMF_TRACE_RET(BDMF_ERR_PARM, "Not supported changing of already configured mirror port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table,
            ((port_drv_priv_t *)(bdmf_obj_data(mirroring_cfg.port_mirror_cfg.rx_dst_port)))->index));
    }

    mirroring_cfg.rx_ref_cntr++;

exit:
    rx_mirroring_port_cfg(port, (new_port_mirror->rx_dst_port != NULL));

    /* Save configuration */
    if (mirroring_cfg.rx_ref_cntr)
    {
        if (new_port_mirror->rx_dst_port)
        {
            mirroring_cfg.port_mirror_cfg.rx_dst_port = new_port_mirror->rx_dst_port; 
            rdd_mirroring_set_rx(rdd_mirroring_cfg);
        }
    }

    return rc;
}

static void _delete_mirror_cfg_from_ports(port_drv_priv_t *port, bdmf_boolean is_rx_mirror)
{
    bdmf_object_handle next_port_obj = NULL;
    port_drv_priv_t *next_port;

    while (1)
    {
        next_port_obj = bdmf_get_next(rdpa_port_drv(), next_port_obj, NULL);
        if (!next_port_obj)
            break;

        next_port = (port_drv_priv_t *)bdmf_obj_data(next_port_obj);
        if (is_rx_mirror)
        {
           if (next_port->mirror_cfg.rx_dst_port)
           {
               _delete_mirrored_port_cfg(next_port, is_rx_mirror);
               port->mirror_cfg.rx_dst_port = NULL;
           }
        }
        else
        {
           if (next_port->mirror_cfg.tx_dst_port)
           {
               _delete_mirrored_port_cfg(next_port, is_rx_mirror);
               port->mirror_cfg.tx_dst_port = NULL;
           }            
        }
    }
    if (is_rx_mirror)
        mirroring_cfg.port_mirror_cfg.rx_dst_port = NULL;
    else
        mirroring_cfg.port_mirror_cfg.tx_dst_port = NULL;
}

#ifdef BCM_PON_XRDP
static int _pbit_to_dp_map_rdd_set(port_drv_priv_t *port)
{
    int i;
    uint8_t map = 0;
    rdd_vport_id_t vport = rdpa_port_rdpa_if_to_vport(port->index);

    for (i = 0; i < 8; i++)
    {
        if (port->pbit_to_dp_map[i] == rdpa_discard_prty_high)
            map |= (1 << i);
    }
    return rdd_ag_processing_vport_pbit_to_discard_prio_vector_set(vport, map);
}
#endif

int port_attr_pbit_to_dp_map_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
#ifdef BCM_PON_XRDP
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_discard_prty dp = *(rdpa_discard_prty *)val;

    port->pbit_to_dp_map[index] = dp;
    if (mo->state == bdmf_state_active)
        return _pbit_to_dp_map_rdd_set(port);

    return 0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int port_attr_mac_write_ex(struct bdmf_object *mo, bdmf_mac_t *mac)
{
    int rc = BDMF_ERR_OK;
#ifndef BCM63158
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    rdd_vport_id_t rdd_vport;
    int i;

    /* Set new mac filter in parser */
    if (!bdmf_mac_is_zero(mac))
    {
        if (port->mac_addr_idx != BDMF_INDEX_UNASSIGNED)
            global_mac_addr_idx_is_set[port->mac_addr_idx] = 0;
        /* Search for empty mac address */
        for (i = 0; i < RDPA_PORT_MAX_MAC; i++)
        {
            if (global_mac_addr_idx_is_set[i] == 0)
            {
                 port->mac_addr_idx = i;
                 global_mac_addr_idx_is_set[i] = 1;
                 break;
            }
        }
        if ((unsigned)port->mac_addr_idx >= RDPA_PORT_MAX_MAC)
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many port macs configured! Exceeded %d allowed macs\n", RDPA_PORT_MAX_MAC);
        /* Save port mac in SRAM for multicast control addresses change in wan loopback */
        rdd_vport = rdpa_if_to_rdd_vport(port->index, port->wan_type);
        rc = rdd_ag_processing_vport_cfg_ex_table_port_mac_addr_idx_set(rdd_vport, port->mac_addr_idx);
        rc = rc ? rc : rdd_ag_processing_port_mac_set(port->mac_addr_idx, mac->b[0], mac->b[1], mac->b[2], mac->b[3], mac->b[4], mac->b[5]);
    }
    else
    {
        rdd_vport = rdpa_port_rdpa_if_to_vport(port->index);
        rc = rdd_ag_processing_vport_cfg_ex_table_port_mac_addr_idx_set(rdd_vport, 0);
        rc = rc ? rc : rdd_ag_processing_port_mac_set(port->mac_addr_idx, 0, 0, 0, 0, 0, 0);
        global_mac_addr_idx_is_set[port->mac_addr_idx] = 0;
    }
#endif
    return rc;
}

/* "port_destroy" driver callback */
void port_destroy_ex(struct bdmf_object *mo)
{
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
#ifndef BCM63158
    rdd_vport_id_t rdd_vport;
    rdd_vport_cfg_entry_t vport_cfg_entry;
#if !defined(G9991)
    uint8_t emacs_flow_ctrl_vec;
    uint32_t bbh_table_index, ingress_counter, egress_counter;
#endif
#endif
    
#ifndef RDP_SIM
    bbh_rx_counters_t bbh_rx_counters = {};
    bbh_tx_counters_t bbh_tx_counters = {};
    rdd_vport_pm_counters_t rdd_port_counters = {};
    
    if (!(_validate_port_index(port->index)))
    {
      /* clear hw port counters by clear on read*/
      read_stat_from_hw(port, &rdd_port_counters, &bbh_rx_counters, &bbh_tx_counters);
    }
    
    port_flow_del(port);
#endif
    
#ifndef BCM63158
    if (rdpa_if_is_wan(port->index))
    {
        /* disable ghost reporting task */
        rdd_ghost_reporting_set_disable(1);
    }
    
    rdd_vport = rdpa_port_rdpa_if_to_vport(port->index);
    
    /* delete ingress_filter profile */
    rdpa_filter_obj_delete_notify_ex(mo);
    /* Delete ingress filters */
    memset(port->ingress_filters, 0, sizeof(port->ingress_filters));
#ifdef INGRESS_FILTERS
    port->ingress_filters_profile = INVALID_PROFILE_IDX;
#endif
    
    memset(&vport_cfg_entry, 0, sizeof(rdd_vport_cfg_entry_t));
    
    /* reset vport and vport_ex contexts */
    rdd_ag_processing_vport_cfg_entry_set(rdd_vport, &vport_cfg_entry);
    
    rdd_ag_processing_vport_cfg_ex_table_set(rdd_vport, 0, 0, INVALID_PROFILE_IDX, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    if (global_mac_addr_idx_is_set[port->mac_addr_idx] == 1)
    {
        rdd_ag_processing_port_mac_set(port->mac_addr_idx, 0, 0, 0, 0, 0, 0);
        global_mac_addr_idx_is_set[port->mac_addr_idx] = 0;
    }

    /* Reset flow ctrl configuration */
    port_rx_exception_cfg(port, 0xffffffff, 0);
    rdd_ag_processing_vport_cfg_table_flow_control_set(rdd_vport, 0);
#if !defined(G9991)
    rdd_ag_timer_common_emac_flow_ctrl_vector_get(&emacs_flow_ctrl_vec);
    emacs_flow_ctrl_vec = emacs_flow_ctrl_vec & (0xFF ^ (1 << port->cfg.emac));
    rdd_ag_timer_common_emac_flow_ctrl_vector_set(emacs_flow_ctrl_vec);
#endif
    
#if !defined(G9991)
    if (rdpa_if_is_wan(port->index))
    {
      bbh_table_index = 0;
      /* if removing wan gbe - update ds lan tabless with ingress and egress counters */
      if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe)
      {
#if defined(XRDP_BBH_PER_LAN_PORT)
        bbh_table_index = 0;
#else
        bbh_table_index = port->cfg.emac;
#endif
      }

      RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_READ_G(ingress_counter, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, bbh_table_index);
      RDD_BYTE_1_BITS_READ_G(egress_counter, RDD_US_TM_WAN_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, bbh_table_index);

      if (rdpa_wan_if_to_wan_type(port->index) == rdpa_wan_gbe)
      {
        RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_WRITE_G(ingress_counter, RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, port->cfg.emac);
#if defined(BCM6858)
          RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_WRITE_G(egress_counter, RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, port->cfg.emac);
#else
          RDD_BYTE_1_BITS_WRITE_G(egress_counter, RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR, port->cfg.emac);
#endif
      }
      else
      {
        /* should be good enough for PON and AE, as only 1st value gets overwrtten */
        RDD_BYTE_1_BITS_WRITE_G(ingress_counter, RDD_BACKUP_BBH_INGRESS_COUNTERS_TABLE_ADDRESS_ARR, 0);
        RDD_BYTE_1_BITS_WRITE_G(egress_counter, RDD_BACKUP_BBH_EGRESS_COUNTERS_TABLE_ADDRESS_ARR, 0);
      }
    }
#endif
#endif
    /* disable tx flow */
    if (rdpa_if_to_rdd_vport_set[port->index])
        rdd_tx_flow_enable(rdpa_port_rdpa_if_to_vport(port->index), rdpa_dir_ds, 0);

    if (port->cfg.emac != rdpa_emac_none && _rdpa_port_emac_to_rdpa_if(port->cfg.emac))
    {
        _rdpa_port_emac_set(port->cfg.emac, 0);
        _rdpa_port_rdpa_if_to_emac_set(port->index, 0);
        rdpa_port_rdpa_if_to_vport_set(port->index, 0, 0);
    }
    
    /* clean mirroring configuration if was set*/
    if (port->mirror_cfg.rx_dst_port)
    {
        _delete_mirrored_port_cfg(port, RX_MIRRORING);
        port->mirror_cfg.rx_dst_port = NULL;
    }

    if (port->mirror_cfg.tx_dst_port)
    {
        _delete_mirrored_port_cfg(port, TX_MIRRORING);
        port->mirror_cfg.tx_dst_port = NULL;
    }

    if (mirroring_cfg.port_mirror_cfg.rx_dst_port)
    {
        port_drv_priv_t *rx_mirror_port = (port_drv_priv_t *)bdmf_obj_data(mirroring_cfg.port_mirror_cfg.rx_dst_port);

        /* check if deleted port is rx mirroring one*/
        if (port->index == rx_mirror_port->index)
        {
            _delete_mirror_cfg_from_ports(port, 1);
        }
    }

    if (mirroring_cfg.port_mirror_cfg.tx_dst_port)
    {
        port_drv_priv_t *tx_mirror_port = (port_drv_priv_t *)bdmf_obj_data(mirroring_cfg.port_mirror_cfg.tx_dst_port);

        /* check if deleted port is tx mirroring one*/
        if (port->index == tx_mirror_port->index)
        {
            _delete_mirror_cfg_from_ports(port, 0);
        }
    }
    mirroring_cfg.tx_mirror_copy_to_ddr_prev[port->index] = -1; /* -1 - prev value is not set */ 
    
#ifndef BCM63158
    /* clean accumulative data of port */
    memset(&accumulative_port_stat[port->index], 0, sizeof(rdpa_port_stat_t));
    memset(&accumulative_port_debug_stat[port->index], 0, sizeof(rdpa_port_debug_stat_t));
    memset(&accumulate_port_pkt_size_stat[port->index], 0, sizeof(rdpa_port_pkt_size_stat_t));
    
    /* set filters for port */
    _proto_filters_update(port, rdpa_proto_filter_any_mask);
    
    /* Reset mappings */         
    physical_port_to_rdpa_if[port->cfg.physical_port] = rdpa_if_none; 
    rdpa_port_rdpa_if_to_vport_set(port->index, 0, 0);
    _rdpa_port_emac_set(port->cfg.emac, 0);
    _rdpa_port_rdpa_if_to_emac_set(port->index, 0);
#endif
}

int port_attr_pkt_size_stat_en_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
#if defined(BCM_PON_XRDP)
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc;
    bdmf_boolean pkt_size_stat_en, pkt_size_stat_en_prev;

    if (!rdpa_if_is_wan(port->index) || port->wan_type == rdpa_wan_gbe)
    /*if (!rdpa_if_is_wan(port->index))*/ /*for debug*/
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "Supported only for WAN non GBE port\n");
    rc = rdd_ag_processing_vport_cfg_ex_table_port_dbg_stat_en_get(port->index, &pkt_size_stat_en_prev);

    if (rc)
        return rc;

    if (!pkt_size_stat_en_prev && *(bdmf_boolean *)val)
        rdpa_port_clear_octets_counters(port->index);

    pkt_size_stat_en = *(bdmf_boolean *)val;
#if !defined(G9991)
    port_rx_exception_cfg(port, EXCEPTION_PORT_DBG_EN, pkt_size_stat_en);
    rc = rdd_ag_processing_vport_cfg_ex_table_port_dbg_stat_en_set(port->index, pkt_size_stat_en);
#endif
    RDD_TX_EXCEPTION_ENTRY_DBG_STAT_EN_WRITE_G(pkt_size_stat_en, RDD_TX_EXCEPTION_ADDRESS_ARR, 0);
    port->pkt_size_stat_en = pkt_size_stat_en;

    return rc;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

int rdpa_port_reconfig_rx_mirroring_ex(port_drv_priv_t *port)
{
    rdd_mirroring_cfg_t rdd_mirroring_cfg;
    int rc;

    _init_rdd_mirroring_cfg(port, &rdd_mirroring_cfg); 
    rc = port_rdd_mirror_cfg(port, &(port->mirror_cfg), &rdd_mirroring_cfg);
    if (!rc)
    {
        rx_mirroring_port_cfg(port, (port->mirror_cfg.rx_dst_port != NULL));

        if (port->mirror_cfg.rx_dst_port)
            rdd_mirroring_set_rx(&rdd_mirroring_cfg);
    }
    return rc;
}

/* "port_attr_uninit_ex" attribute "write" callback */
int port_attr_uninit_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
#if defined(BCM_PON_XRDP)
    port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
    bdmf_boolean port_empty = 1;
    bbh_id_e bbh_id;
    
     /* function called before starting destroying a port and egress Tm*/
     /* disable BBH Rx for port*/
     bbh_id = rdpa_port_to_bbh_id(port);
     if (bbh_id != BBH_ID_NULL)
     {
         ag_drv_bbh_rx_general_configuration_enable_set(bbh_id, 0, 0);
     }
     
     /*  wait Xms to packets in the system to be processed */
     bdmf_usleep(100);
     
     /* validate system is clean */
     if (port->tm_cfg.sched)
     {
        rc = egress_tm_is_empty_on_channel(port->tm_cfg.sched, port->channel, &port_empty);
        BDMF_TRACE_DBG("port is_empty = %d\n", port_empty);
     }

     if (rc)
        return rc;
     
     if (port_empty)
        return 0;
     else
        return BDMF_ERR_INTERNAL;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

