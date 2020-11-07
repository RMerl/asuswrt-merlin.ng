/* 
 * <:copyright-BRCM:2013:proprietary:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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
#include "rdpa_api.h"
#include "rdpa_int.h"
#ifndef LEGACY_RDP
#include "rdpa_rdd_map.h"
#include "rdd_runner_proj_defs.h"
#else
#include "rdpa_rdd_map_legacy.h"
#endif
#include "rdd.h"
#ifndef LEGACY_RDP
#include "rdd_tm.h"
#include "rdd_cpu.h"
#include "rdd_l4_filters.h"
#include "rdd_ingress_filters.h"
#include "rdd_iptv_filter.h"
#endif
#include "data_path_init.h"
#include "rdp_drv_bpm.h"
#include "rdp_drv_bbh.h"
#include "rdp_drv_sbpm.h"
#include "rdpa_rdd_inline.h"
#ifndef BDMF_SYSTEM_SIM
#include "clk_rst.h"
#endif
#include "rdp_init.h"

#define RDPA_PCI_PORT_MAIN_INTERRUPT_NUM_IN_RDD 1
#define RDPA_PCI_PORT_SUB_INTERRUPT_NUM_IN_RDD 0
#define RDPA_CPU_RX_QUEUES_MAIN_INTERRUPT_NUM_IN_RDD 0

extern int emac_ports_headroom_hw_cfg(int headroom_size);
extern int emac_ports_mtu_hw_cfg(int mtu_size);
#ifndef LEGACY_RDP
extern rdd_module_t ds_flow_based_ingress_filters, us_flow_based_ingress_filters;
#endif

#ifndef BDMF_SYSTEM_SIM
extern int rdp_post_init(void);
int runner_reserved_memory_get(uint32_t *tm_base_addr, uint32_t *tm_base_addr_phys,
    uint32_t *mc_base_addr, uint32_t *mc_base_addr_phys, uint32_t *tm_size, uint32_t *mc_size);
extern int bcm_misc_g9991_debug_port_get(void);
#endif


#ifndef __KERNEL__
void rdp_get_init_params(rdp_init_params *init_params)
{
    init_params->bpm_buffer_size = RDPA_BPM_BUFFER_2K;
    init_params->bpm_buffers_number = DRV_BPM_GLOBAL_THRESHOLD_7_5K;
}
#else
static rdpa_bpm_buffer_size_t rdpa_mtu_to_bpm_buffer_size(const uint32_t max_frame_size)
{
    uint32_t buffer_size = max_frame_size+RDD_PACKET_HEADROOM_OFFSET;

    if (buffer_size <= RDPA_BPM_BUFFER_2K)
        return RDPA_BPM_BUFFER_2K;
    else if (buffer_size <= RDPA_BPM_BUFFER_4K)
        return RDPA_BPM_BUFFER_4K;
    else if (buffer_size <= RDPA_BPM_BUFFER_16K)
        return RDPA_BPM_BUFFER_16K;
    else
        return RDPA_BPM_BUFFER_16K;
}

static DRV_BPM_GLOBAL_THRESHOLD rdpa_buffers_number_get(const uint32_t buffers_amount)
{
    if (buffers_amount < 5000)
        return DRV_BPM_GLOBAL_THRESHOLD_2_5K;
    else if (buffers_amount < 7500)
        return DRV_BPM_GLOBAL_THRESHOLD_5K;
    else if (buffers_amount < 15000)
        return DRV_BPM_GLOBAL_THRESHOLD_7_5K;
    else
        return DRV_BPM_GLOBAL_THRESHOLD_15K;
}

void rdp_get_init_params(rdp_init_params *init_params)
{
    uint32_t tm_size, mc_size, buffers_amount;

    init_params->bpm_buffer_size = rdpa_mtu_to_bpm_buffer_size(RDPA_MTU);
    
    runner_reserved_memory_get(&init_params->runner_tm_base_addr, &init_params->runner_tm_base_addr_phys,
       &init_params->runner_mc_base_addr, &init_params->runner_mc_base_addr_phys, &tm_size, &mc_size);
    buffers_amount = (tm_size*1024*1024 - RDP_DDR_DATA_STRUCTURES_SIZE)/init_params->bpm_buffer_size;
    init_params->bpm_buffers_number = rdpa_buffers_number_get(buffers_amount);
}
#endif

rdpa_if if_forward_eligible_arr[] = {
    rdpa_if_wan0,
    rdpa_if_lan0,
    rdpa_if_lan1,
    rdpa_if_lan2,
    rdpa_if_lan3,
    rdpa_if_lan4,
    rdpa_if_wlan0
};


static void forwarding_matrix_cfg(void)
{
    rdd_bridge_port_t rdd_bridge_port, rdd_forwarding_eligibility_port;
    int i, j;

    for (i = 0; i < ARRAY_LENGTH(if_forward_eligible_arr); i++)
    {
        rdd_bridge_port = rdpa_if_to_rdd_bridge_port(if_forward_eligible_arr[i], NULL);
        rdd_wifi_ssid_forwarding_matrix_cfg(0xffff, rdd_bridge_port);
    }

    for (i = 0; i < ARRAY_LENGTH(if_forward_eligible_arr); i++)
    {
        for (j = 0; j < ARRAY_LENGTH(if_forward_eligible_arr); j++)
        {
            rdd_bridge_port = rdpa_if_to_rdd_bridge_port(if_forward_eligible_arr[i], NULL);
            rdd_forwarding_eligibility_port = rdpa_if_to_rdd_bridge_port(if_forward_eligible_arr[j], NULL);

            rdd_forwarding_matrix_cfg(rdd_bridge_port, rdd_forwarding_eligibility_port,
                i != j); /* Self Forwarding is blocked */
        }
    }

    rdd_forwarding_matrix_cfg(BL_LILAC_RDD_PCI_BRIDGE_PORT, BL_LILAC_RDD_PCI_BRIDGE_PORT, 1);
}

#ifdef LEGACY_RDP
void rdpa_lock_critical_section(bdmf_fastlock *lock)
{
    /* Disable interrupts */
    bdmf_fastlock_lock(lock);
}

void rdpa_unlock_critical_section(bdmf_fastlock *lock)
{
    /* Enable interrupts */
    bdmf_fastlock_unlock(lock);
}

void rdpa_lock_critical_section_irq(bdmf_fastlock *lock, unsigned long *flags)
{
    /* Disable interrupts */
    bdmf_fastlock_lock_irq(lock, *flags);
}

void rdpa_unlock_critical_section_irq(bdmf_fastlock *lock, unsigned long flags)
{
    /* Enable interrupts */
    bdmf_fastlock_unlock_irq(lock, flags);
}
#endif

static void runner_params_init(void)
{
    rdd_bridge_port_t rdd_bridge_port, bridge_ports_vector;
    /* Rate shaper should be set to -1, because even in case of routed traffic in mixed mode (which goes to FC),
     * RDPA_UNMATCHED_DS_IC_RESULT_ID or RDPA_UNMATCHED_US_IC_RESULT_ID may have effect, and rate
     * shaper from ingress classification result may be activated on the packet. */
    rdd_ic_context_t ic_ctx = {.rate_shaper = -1};
    const rdpa_system_init_cfg_t *init_cfg = _rdpa_system_init_cfg_get();
    const rdpa_system_cfg_t *cfg = _rdpa_system_cfg_get();
    rdd_rate_limit_params_t rdd_budget = {};
    rdd_vlan_cmd_param_t vlan_params = {};
    uint32_t i, j, table_size;

#ifdef LEGACY_RDP
    rdd_critical_section_config(rdpa_lock_critical_section, rdpa_unlock_critical_section,
        rdpa_lock_critical_section_irq, rdpa_unlock_critical_section_irq);
#endif

    /* mask the CPU Rx queues interrupts in the RDD */
    for (i = 0; i < RDPA_CPU_MAX_QUEUES; i++)
        rdd_interrupt_mask(0, RDPA_CPU_RX_QUEUES_MAIN_INTERRUPT_NUM_IN_RDD);

    /* mask the PCI bridge port interrupt in the RDD */
    rdd_interrupt_mask(RDPA_PCI_PORT_MAIN_INTERRUPT_NUM_IN_RDD, RDPA_PCI_PORT_SUB_INTERRUPT_NUM_IN_RDD);

    rdd_ipv6_enable(1);

    rdd_egress_ethertype_config(cfg->inner_tpid, cfg->outer_tpid);

    /* Bridge Init */
    if (!rdpa_is_fttdp_mode())
    {
        /* DA/SA Lookup configuration */
        bridge_port_sa_da_cfg();

        /* Init eligible_ports_matrix */
        forwarding_matrix_cfg();
    }

    ic_ctx.action = rdpa_forward_action_drop;
    ic_ctx.policer = BDMF_INDEX_UNASSIGNED;
    ic_ctx.rate_shaper = BDMF_INDEX_UNASSIGNED;

    for (i = 0; i < RDPA_MAX_US_IC_RESULTS; i++)
        rdd_ic_context_cfg(rdpa_dir_us, i, &ic_ctx);

    for (i = 0; i < RDPA_MAX_DS_IC_RESULTS; i++)
        rdd_ic_context_cfg(rdpa_dir_ds, i, &ic_ctx);

    /* Initialize VLAN action table to default: transparent */
    vlan_params.vlan_command = RDD_VLAN_CMD_TRANSPARENT;
    vlan_params.pbits_command = RDD_PBITS_CMD_TRANSPARENT;
    vlan_params.outer_vid = 0xffff;
    vlan_params.inner_vid = 0xffff;
    vlan_params.outer_pbits = 0xff;
    vlan_params.inner_pbits = 0xff;

    for (i = 0; i < RDPA_MAX_VLAN_ACTION; i++)
    {
        vlan_params.vlan_command_id = i;
        rdd_vlan_cmd_cfg(rdpa_dir_ds, &vlan_params);
        rdd_vlan_cmd_cfg(rdpa_dir_us, &vlan_params);
    }

    /* Initialize TCONT */
    table_size = (rdpa_is_gpon_or_xgpon_mode()) ? RDPA_MAX_TCONT : 1;

    for (i = 0; i < table_size; i++)
    {
        /* Configure RDD WAN scheduling */
        rdd_wan_channel_cfg(i, RDD_WAN_CHANNEL_SCHEDULE_PRIORITY, RDD_PEAK_SCHEDULE_MODE_ROUND_ROBIN);

        /* Set the overall US rate limiter to unlimited */
        rdd_wan_channel_rate_limiter_cfg(i, 0, rdpa_tm_orl_prty_low);
    }

    if (init_cfg->gbe_wan_emac != rdpa_emac_none)
    {
        /* In GBE mode, we sent packets from GEM 0 to either FC or IC. */
        rdd_ds_wan_flow_cfg(GBE_WAN_FLOW_ID, rdpa_cpu_reason_min, 1, RDPA_UNMATCHED_DS_IC_RESULT_ID);
    }
    else
    {
        /* In GPON mode, all GEM flows are dropped by default - we do not send them to FC or IC. But when a GEM is
         * configured, _cfg_ds_gem_flow_hw() will configure whether it will be mapped to FC or not. */
        for (i = 0; i < RDPA_MAX_GEM_FLOW; i++)
            rdd_ds_wan_flow_cfg(i, rdpa_cpu_reason_min, 0, RDPA_UNMATCHED_DS_IC_RESULT_ID);
    }

    /* set port default flow */
    for (i = RDD_EMAC_ID_START; i < RDD_EMAC_ID_COUNT; i++)
        rdd_us_ic_default_flows_cfg(i, RDPA_UNMATCHED_US_IC_RESULT_ID);

    /* in GBE mode configure internal us gem_flow */
    if (init_cfg->gbe_wan_emac != rdpa_emac_none)
        rdd_us_wan_flow_cfg(0, RDD_WAN_CHANNEL_0, 0, 0, 1, 0, 0, 0);

    for (i = 0; i < 8; i++)
    {
        /* TBD: we will have more than one table. */
        rdd_us_pbits_to_qos_entry_cfg(0, i, 0, 0);
    }

    if (!rdpa_is_fttdp_mode())
    {
        /* Initialize RDD DSCP to PBIT table  */
        for (i = 0; i < ARRAY_LENGTH(if_forward_eligible_arr); i++)
        {
            for (j = 0; j < 64 /*CE_BL_DSCP_TO_PBIT_TABLE_SIZE*/; j++)
            {
                rdd_bridge_port = rdpa_if_to_rdd_bridge_port(if_forward_eligible_arr[i], NULL);
                /* at first phase runner is not using the DSCP-to-PBIT table of IH */
#ifdef LEGACY_RDP
                rdd_dscp_to_pbits_cfg(rdd_bridge_port, 0xff, 0);
#else
                if (rdd_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT)
                    rdd_dscp_to_pbits_cfg(rdpa_dir_ds, rdd_bridge_port, 0xff, 0);
                else
                    rdd_dscp_to_pbits_cfg(rdpa_dir_us, rdd_bridge_port, 0xff, 0);
#endif
            }
        }
    }
    else
    {
        /* BL_LILAC_RDD_LAN0_BRIDGE_PORT is used for all US ports */
#ifdef LEGACY_RDP
        rdd_dscp_to_pbits_cfg(BL_LILAC_RDD_LAN0_BRIDGE_PORT, 0xff, 0);
        rdd_dscp_to_pbits_cfg(rdpa_if_to_rdd_bridge_port(rdpa_if_wan0, NULL), 0xff, 0); /* FIXME: MULTI-WAN XPON */
#else
        if (rdpa_if_to_rdd_bridge_port(rdpa_if_wan0, NULL) == BL_LILAC_RDD_WAN_BRIDGE_PORT) /* FIXME: MULTI-WAN XPON */
        {
            rdd_dscp_to_pbits_cfg(rdpa_dir_ds, BL_LILAC_RDD_LAN0_BRIDGE_PORT, 0xff, 0);
            rdd_dscp_to_pbits_cfg(rdpa_dir_ds, rdpa_if_to_rdd_bridge_port(rdpa_if_wan0, NULL), 0xff, 0);/* FIXME: MULTI-WAN XPON */
        }
        else
        {
            rdd_dscp_to_pbits_cfg(rdpa_dir_us, BL_LILAC_RDD_LAN0_BRIDGE_PORT, 0xff, 0);
            rdd_dscp_to_pbits_cfg(rdpa_dir_us, rdpa_if_to_rdd_bridge_port(rdpa_if_wan0, NULL), 0xff, 0);/* FIXME: MULTI-WAN XPON */
        }
#endif
    }

    /* Initiliaze DS pBit to QoS */
    for (i = BL_LILAC_RDD_LAN0_BRIDGE_PORT; i <= BL_LILAC_RDD_LAN4_BRIDGE_PORT; i++)
    {
        for (j = 0; j < 8; j++)
            rdd_ds_pbits_to_qos_entry_cfg(i, j, 0);
    }
    for (j = 0; j < 8; j++)
        rdd_ds_pbits_to_qos_entry_cfg(BL_LILAC_RDD_PCI_BRIDGE_PORT, j, 0);

    /* Default configuration of all RDD reasons to CPU_RX_QUEUE_ID_0 */
    for (i = 0; i < rdpa_cpu_reason__num_of; i++)
    {
        rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_ds);

        rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_us);
    }

    /* Initialize Ethernet priority queues */
    for (i = 0; i < 8; i++)
    {
        /* (-1) TBD : when runner will support EMAC_5 */
#ifndef LEGACY_RDP
        for (j = RDD_LAN0_VPORT; j < RDD_LAN4_VPORT; j++)
        {
            /* Configure queue size in RDD */
            rdd_lan_vport_tx_queue_cfg(j, i, 0, RDD_QUEUE_PROFILE_DISABLED);
        }
#else
        for (j = RDD_EMAC_ID_0; j < RDD_EMAC_ID_4; j++)
        {
            /* Configure queue size in RDD */
            rdd_eth_tx_queue_cfg(j, i, 0, RDD_QUEUE_PROFILE_DISABLED, INVALID_COUNTER_ID);
        }
#endif
    }
    /* Initialize Mac Table */
    rdd_mac_table_clear();

    rdd_iptv_lkp_method_cfg(iptv_lookup_method_mac);

    /* Set the default downstream connection miss action to TRAP */
    rdd_ds_conn_miss_action_filter_enable(1);

    /* pBit to pBit mapping table configuration */
    for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++)
    {
        for (j = 0; j < 8; j++)
            rdd_pbits_to_pbits_config(i, j, j);
    }

    /* IP fragment */
    rdd_ip_frag_filter_cfg(ACTION_TRAP, rdpa_cpu_rx_reason_ip_frag, rdpa_dir_ds);

    /* US - Isn't configurable by user */
    rdd_ip_frag_filter_cfg(ACTION_TRAP, rdpa_cpu_rx_reason_ip_frag, rdpa_dir_us);

#ifndef LEGACY_RDP

#ifndef G9991 /* XXX: Currently ingress Filters are not part of G9991 firmware */
    /* ICMPv6 */
    for (i = RDD_WAN0_VPORT; i < RDD_LAN4_VPORT; j++)
    {
        rdd_icmpv6_filter_cfg(i, rdpa_dir_us, 0, 0);
        rdd_icmpv6_filter_cfg(i, rdpa_dir_ds, 0, 0);
    }
    rdd_icmpv6_filter_cfg(RDD_WIFI_VPORT, rdpa_dir_ds, 0, 0);
    rdd_icmpv6_filter_cfg(RDD_WIFI_VPORT, rdpa_dir_us, 0, 0);

    /* TPID Filters  */
    rdd_tpid_detect_filter_value_cfg(&us_flow_based_ingress_filters, 0xffff);
    rdd_tpid_detect_filter_value_cfg(&ds_flow_based_ingress_filters, 0xffff);

    /* disable the tpid filters*/
    for (i = RDD_WAN0_VPORT; i < RDD_LAN4_VPORT; i++)
    {
        rdd_tpid_detect_filter_cfg(i, rdpa_dir_us, 0);
        rdd_tpid_detect_filter_cfg(i, rdpa_dir_ds, 0);
    }
    rdd_tpid_detect_filter_cfg(RDD_WIFI_VPORT, rdpa_dir_us, 0);
    rdd_tpid_detect_filter_cfg(RDD_WIFI_VPORT, rdpa_dir_ds, 0);
#endif

#else
    /* ICMPv6 - LAN */
    for (i = BL_LILAC_RDD_LAN0_BRIDGE_PORT; i <= BL_LILAC_RDD_LAN4_BRIDGE_PORT; i++)
        rdd_icmpv6_filter_config(i, BL_LILAC_RDD_SUBNET_FLOW_CACHE, BL_LILAC_RDD_FILTER_DISABLE);
    rdd_icmpv6_filter_config(BL_LILAC_RDD_PCI_BRIDGE_PORT, BL_LILAC_RDD_SUBNET_FLOW_CACHE, BL_LILAC_RDD_FILTER_DISABLE);

    /* ICMPv6 - WAN */
    rdd_icmpv6_filter_config(BL_LILAC_RDD_WAN_ROUTER_PORT, BL_LILAC_RDD_SUBNET_FLOW_CACHE, BL_LILAC_RDD_FILTER_DISABLE);
    rdd_icmpv6_filter_config(BL_LILAC_RDD_WAN_BRIDGE_PORT, 0, BL_LILAC_RDD_FILTER_DISABLE);
    rdd_icmpv6_filter_config(BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT, 0, BL_LILAC_RDD_FILTER_DISABLE);

    /* TPID Filters  */
    rdd_tpid_detect_filter_value_config(rdpa_dir_us, 0xffff);
    rdd_tpid_detect_filter_value_config(rdpa_dir_ds, 0xffff);

    /* disable the tpid filters*/
    for (i = 0; i < BL_LILAC_RDD_PCI_BRIDGE_PORT; i++)
        rdd_tpid_detect_filter_config(i, BL_LILAC_RDD_SUBNET_FLOW_CACHE, BL_LILAC_RDD_FILTER_DISABLE);

    rdd_tpid_detect_filter_config(BL_LILAC_RDD_WAN_ROUTER_PORT, BL_LILAC_RDD_SUBNET_FLOW_CACHE,
        BL_LILAC_RDD_FILTER_DISABLE);
#endif

    /* Configure L4 filters - Action: 'Drop' */

    /* Filter: 'Header Error' */
    rdd_hdr_err_filter_cfg(ACTION_DROP, rdpa_cpu_rx_reason_non_tcp_udp, rdpa_dir_ds);
    rdd_hdr_err_filter_cfg(ACTION_DROP, rdpa_cpu_rx_reason_hdr_err, rdpa_dir_us);

    /* Filter: 'IP Fragment' */
    rdd_ip_frag_filter_cfg(ACTION_DROP, rdpa_cpu_rx_reason_non_tcp_udp, rdpa_dir_ds);
    rdd_ip_frag_filter_cfg(ACTION_DROP, rdpa_cpu_rx_reason_ip_frag, rdpa_dir_us);

    if (init_cfg->switching_mode == rdpa_vlan_aware_switching)
        rdd_vlan_switching_config(1, 0);

    bridge_ports_vector = BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT | BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT |
        BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT | BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT |
        BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT | BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT;

    rdd_bridge_flooding_cfg(bridge_ports_vector, 0xffff);
    rdd_us_unknown_da_flooding_bridge_port_cfg(BL_LILAC_RDD_WAN_BRIDGE_PORT);

    /* Configure quasi policer for US bridge flooding */
    rdd_us_quasi_policer_cfg(BL_LILAC_RDD_LAN0_BRIDGE_PORT, 100000);
    rdd_us_quasi_policer_cfg(BL_LILAC_RDD_LAN1_BRIDGE_PORT, 100000);
    rdd_us_quasi_policer_cfg(BL_LILAC_RDD_LAN2_BRIDGE_PORT, 100000);
    rdd_us_quasi_policer_cfg(BL_LILAC_RDD_LAN3_BRIDGE_PORT, 100000);
    rdd_us_quasi_policer_cfg(BL_LILAC_RDD_LAN4_BRIDGE_PORT, 100000);
    rdd_us_quasi_policer_cfg(BL_LILAC_RDD_PCI_BRIDGE_PORT, 100000);

#ifdef LEGACY_RDP
    /* Although "subnet" is an object that should be invisible to the user, it is a leftover in firmware/rdd code, and
     * should still be configured. It determines how we decide if the packet is to be bridged or routed. In FC mode,
     * we directly map from ingress classifier to subnet 0, which is reserved by the firmware for FC. */
    rdd_subnet_classify_config(init_cfg->ip_class_method == rdpa_method_fc ?
        BL_LILAC_RDD_SUBNET_CLASSIFY_ETHERNET_FLOW : BL_LILAC_RDD_SUBNET_CLASSIFY_MAC_FILTER);
#endif

    /* Configure the internal lan scheduling mode to normal (related to ds overall rate limiter configuration) */
    /* XXX: Should be changed to RR if this is external switch. Port should be configured for SP or RR, for normal mode
     * 0 is expected. */
    rdd_inter_lan_schedule_mode_cfg(RDD_INTER_LAN_SCHEDULE_MODE_NORMAL, 0);

    /* Disable default policer (Importent in in flow cache mode) */
    rdd_policer_cfg(rdpa_dir_ds, -1, &rdd_budget);

    rdd_mtu_cfg(cfg->mtu_size);
}

static void bpm_sbpm_sp_enable(int with_fiber)
{
    DRV_SBPM_SP_ENABLE sbpm_sp_enable = {};
    const rdpa_system_init_cfg_t *init_cfg = _rdpa_system_init_cfg_get();

    sbpm_sp_enable.rnra_sp_enable = 1;
    sbpm_sp_enable.rnrb_sp_enable = 1;
    sbpm_sp_enable.eth0_sp_enable = init_cfg->enabled_emac & rdpa_emac_id(rdpa_emac0) ? 1 : 0;
    sbpm_sp_enable.eth1_sp_enable = init_cfg->enabled_emac & rdpa_emac_id(rdpa_emac1) ? 1 : 0;
    sbpm_sp_enable.eth2_sp_enable = init_cfg->enabled_emac & rdpa_emac_id(rdpa_emac2) ? 1 : 0;
    sbpm_sp_enable.eth3_sp_enable = init_cfg->enabled_emac & rdpa_emac_id(rdpa_emac3) ? 1 : 0;

    if (init_cfg->gbe_wan_emac == rdpa_emac4)
        sbpm_sp_enable.eth4_sp_enable = 0; /* packets from WAN are not directed to SRAM */
    else
        sbpm_sp_enable.eth4_sp_enable = init_cfg->enabled_emac & rdpa_emac_id(rdpa_emac4) ? 1 : 0;

    /* In non-fttdp do not direct packets from WAN to SRAM (and emac 5 is not
     * supported as LAN) */
    sbpm_sp_enable.gpon_or_eth5_sp_enable = with_fiber && rdpa_is_fttdp_mode();

    fi_bl_drv_sbpm_sp_enable(&sbpm_sp_enable);

    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_RNR_A, sbpm_sp_enable.rnra_sp_enable ? DRV_BPM_ENABLE : DRV_BPM_DISABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_RNR_B, sbpm_sp_enable.rnrb_sp_enable ? DRV_BPM_ENABLE : DRV_BPM_DISABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_EMAC0, sbpm_sp_enable.eth0_sp_enable ? DRV_BPM_ENABLE : DRV_BPM_DISABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_EMAC1, sbpm_sp_enable.eth1_sp_enable ? DRV_BPM_ENABLE : DRV_BPM_DISABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_EMAC2, sbpm_sp_enable.eth2_sp_enable ? DRV_BPM_ENABLE : DRV_BPM_DISABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_EMAC3, sbpm_sp_enable.eth3_sp_enable ? DRV_BPM_ENABLE : DRV_BPM_DISABLE);
    fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_EMAC4, init_cfg->enabled_emac & rdpa_emac_id(rdpa_emac4) ? 1 : 0);
    if (with_fiber)
        fi_bl_drv_bpm_sp_enable(DRV_BPM_SP_GPON,  DRV_BPM_ENABLE);
}

S_DPI_CFG oren_dpi_cfg;

int system_data_path_init(void)
{
    rdp_init_params init_params = {};
    const rdpa_system_init_cfg_t *system_init = _rdpa_system_init_cfg_get();
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();

    oren_dpi_cfg.ip_class_method = system_init->ip_class_method != rdpa_method_none;
    oren_dpi_cfg.bridge_fc_mode = system_init->ip_class_method == rdpa_method_fc;
#ifndef BDMF_SYSTEM_SIM
    if (get_rdp_freq(&oren_dpi_cfg.runner_freq))
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed to get runner freq from clk&rst\n");

    oren_dpi_cfg.g9991_debug_port = bcm_misc_g9991_debug_port_get();
    if (system_init->gbe_wan_emac == oren_dpi_cfg.g9991_debug_port)
        oren_dpi_cfg.g9991_debug_port = -1;

    rdp_get_init_params(&init_params);
#endif

    oren_dpi_cfg.runner_tm_base_addr = init_params.runner_tm_base_addr;
    oren_dpi_cfg.runner_tm_base_addr_phys = init_params.runner_tm_base_addr_phys;
    oren_dpi_cfg.runner_mc_base_addr = init_params.runner_mc_base_addr;
    oren_dpi_cfg.runner_mc_base_addr_phys = init_params.runner_mc_base_addr_phys;
    oren_dpi_cfg.bpm_buffer_size = init_params.bpm_buffer_size;
    oren_dpi_cfg.bpm_buffers_number = init_params.bpm_buffers_number;
    oren_dpi_cfg.enabled_port_map = system_init->enabled_emac;
    oren_dpi_cfg.us_ddr_queue_enable = system_init->us_ddr_queue_enable;
    oren_dpi_cfg.mtu_size = system_cfg->mtu_size;
    oren_dpi_cfg.headroom_size = system_cfg->headroom_size;
    oren_dpi_cfg.car_mode = system_cfg->car_mode;
    
    /* Set only if GBE, otherwise set temp value until fiber bbh is known - and set with system_data_path_init_fiber() */
    if (system_init->gbe_wan_emac != rdpa_emac_none && system_init->gbe_wan_emac != rdpa_emac5)
        oren_dpi_cfg.wan_bbh = system_init->gbe_wan_emac;
    else
        oren_dpi_cfg.wan_bbh = DRV_BBH_NUMBER_OF_PORTS;

    data_path_init(&oren_dpi_cfg);
    runner_params_init();
    data_path_go();

    bpm_sbpm_sp_enable(oren_dpi_cfg.wan_bbh != DRV_BBH_NUMBER_OF_PORTS);
#ifndef BDMF_SYSTEM_SIM
    rdp_post_init();
#endif

    return 0;
}

int system_data_path_init_fiber(rdpa_wan_type wan_type)
{
    const rdpa_system_init_cfg_t *system_init = _rdpa_system_init_cfg_get();

    switch (wan_type)
    {
    case rdpa_wan_gpon:
        data_path_init_fiber(DRV_BBH_GPON);
        break;
    case rdpa_wan_epon:
        data_path_init_fiber(DRV_BBH_EPON);
        break;
    case rdpa_wan_gbe:
        if (system_init->gbe_wan_emac != rdpa_emac5)
            return 0; /* Done already */
        
        data_path_init_fiber(DRV_BBH_EMAC_5);
        break;
    default:
        bdmf_trace("wrong RDPA wan type");
        return -1;
    }

    bpm_sbpm_sp_enable(1);

    return 0;
}

/* Update EMAC and BBH Frame length */
int headroom_hw_cfg(int headroom_size)
{
    int rc;
    rdpa_wan_type wan_type = rdpa_wan_none;
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration = {};
    DRV_BBH_PORT_INDEX port_index;
    
    rc = emac_ports_headroom_hw_cfg(headroom_size);
    /* 96838 support only single wan */ 
    wan_type = rdpa_wan_if_to_wan_type(rdpa_if_wan0);
    if ((wan_type != rdpa_wan_none) && (wan_type != rdpa_wan_gbe))
    {
        port_index = wan_type == rdpa_wan_epon ? DRV_BBH_EPON : DRV_BBH_GPON;

        fi_bl_drv_bbh_rx_get_configuration(port_index, &bbh_rx_configuration);
        bbh_rx_configuration.reassembly_offset_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(headroom_size);
        rc = fi_bl_drv_bbh_rx_set_configuration(port_index, &bbh_rx_configuration);
    }

    if (!rc)
    {
#ifdef LEGACY_RDP
        rdd_ddr_packet_headroom_size_cfg(headroom_size);
#else
        rdd_packet_headroom_size_cfg(headroom_size, 0);
#endif
    }
    else
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set headroom size to RDD, error %d\n", rc);

    return rc;
}

int mtu_hw_cfg(int mtu_size)
{
    bdmf_error_t rc;
    rdpa_wan_type wan_type = rdpa_wan_none;
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration = {};
    DRV_BBH_PORT_INDEX port_index;

    rc = emac_ports_mtu_hw_cfg(mtu_size);
    /* 96838 support only single wan */ 
    wan_type = rdpa_wan_if_to_wan_type(rdpa_if_wan0);
    if ((wan_type != rdpa_wan_none) && (wan_type != rdpa_wan_gbe))
    {
        port_index = wan_type == rdpa_wan_epon ? DRV_BBH_EPON : DRV_BBH_GPON;

        fi_bl_drv_bbh_rx_get_configuration(port_index, &bbh_rx_configuration);
#if RDPA_BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX == 0
        bbh_rx_configuration.maximum_packet_size_0 = mtu_size;
#else
#error problem with BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX
#endif
        /* maximum_packet_size 1-3 are not in use */
        bbh_rx_configuration.maximum_packet_size_1 = mtu_size;
        bbh_rx_configuration.maximum_packet_size_2 = mtu_size;
        bbh_rx_configuration.maximum_packet_size_3 = mtu_size;
        rc = fi_bl_drv_bbh_rx_set_configuration(port_index, &bbh_rx_configuration);
    } 
    if (!rc)
    {
        rc = rdd_mtu_cfg(mtu_size);
    }
    else
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to set MTU size to RDD, error %d\n", rc);
    return 0;
}

rdpa_bpm_buffer_size_t rdpa_bpm_buffer_size_get(void)
{
    rdp_init_params init_params = {};

    rdp_get_init_params(&init_params);
    return init_params.bpm_buffer_size;
}

