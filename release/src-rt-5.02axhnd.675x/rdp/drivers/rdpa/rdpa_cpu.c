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

/*
 * rdpa_cpu.c
 *
 *  Created on: Nov 21, 2012
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdd_common.h"
#include "rdpa_rdd_inline.h"
#include "rdp_cpu_ring.h"
#include "rdpa_cpu_ex.h"
#include "rdd_cpu.h"
#ifdef LEGACY_RDP
#include "rdpa_rdd_map_legacy.h"
#else
#include "rdpa_rdd_map.h"
#endif
#include "rdpa_port_int.h"

/***************************************************************************
 * host object type
 **************************************************************************/

int cpu_max_meters_per_dir[2] = {
    [rdpa_dir_ds] = RDPA_CPU_MAX_DS_METERS,
    [rdpa_dir_us] = RDPA_CPU_MAX_US_METERS
};

rdpa_cpu_meter_cfg_t meter[2][RDPA_CPU_MAX_METERS] = { { } };
rdpa_ports us_meter_ports[RDPA_CPU_PER_PORT_REASON][RDPA_CPU_MAX_US_METERS] = { { } };
/* Reason to meter configuration. For RDP: also reason to queue configuration, ignored for XRDP */
rdpa_cpu_reason_cfg_t reason_cfg[2][rdpa_cpu_reason__num_of] = { { } };

struct bdmf_object *cpu_object[rdpa_cpu_port__num_of];

extern bdmf_attr_enum_table_t cpu_port_enum_table;

/* cpu_reason enum values */
const bdmf_attr_enum_table_t rdpa_cpu_reason_enum_table = {
    .type_name = "rdpa_cpu_reason",
    .values = {
        {"oam", rdpa_cpu_rx_reason_oam},
        {"omci", rdpa_cpu_rx_reason_omci},
        {"flow", rdpa_cpu_rx_reason_flow},
        {"mcast", rdpa_cpu_rx_reason_mcast},
        {"bcast", rdpa_cpu_rx_reason_bcast},
        {"igmp", rdpa_cpu_rx_reason_igmp},
        {"icmpv6", rdpa_cpu_rx_reason_icmpv6},
        {"trap0", rdpa_cpu_rx_reason_mac_trap_0},
        {"trap1", rdpa_cpu_rx_reason_mac_trap_1},
        {"trap2", rdpa_cpu_rx_reason_mac_trap_2},
        {"trap3", rdpa_cpu_rx_reason_mac_trap_3},
        {"dhcp", rdpa_cpu_rx_reason_dhcp},
        {"hdr_err", rdpa_cpu_rx_reason_hdr_err},
        {"sa_moved", rdpa_cpu_rx_reason_sa_moved},
        {"unknown_sa", rdpa_cpu_rx_reason_unknown_sa},
        {"unknown_da", rdpa_cpu_rx_reason_unknown_da},
        {"ip_frag", rdpa_cpu_rx_reason_ip_frag},
        {"local_ip", rdpa_cpu_rx_reason_local_ip},
        {"mac_spoofing", rdpa_cpu_rx_reason_mac_spoofing},
        {"direct_flow", rdpa_cpu_rx_reason_direct_flow},
        {"mcast_miss", rdpa_cpu_rx_reason_mcast_miss},
        {"ipsec", rdpa_cpu_rx_reason_ipsec},
        {"reserved0", rdpa_cpu_rx_reason_reserved_0},
        {"reserved1", rdpa_cpu_rx_reason_reserved_1},
        {"reserved2", rdpa_cpu_rx_reason_reserved_2},
        {"l2cp", rdpa_cpu_rx_reason_l2cp},
        {"cpu_mirroring", rdpa_cpu_rx_reason_cpu_mirroring},
        {"etype_udef_0", rdpa_cpu_rx_reason_etype_udef_0},
        {"etype_udef_1", rdpa_cpu_rx_reason_etype_udef_1},
        {"etype_udef_2", rdpa_cpu_rx_reason_etype_udef_2},
        {"etype_udef_3", rdpa_cpu_rx_reason_etype_udef_3},
        {"etype_pppoe_d", rdpa_cpu_rx_reason_etype_pppoe_d},
        {"etype_pppoe_s", rdpa_cpu_rx_reason_etype_pppoe_s},
        {"etype_arp", rdpa_cpu_rx_reason_etype_arp},
        {"etype_ptp_1588", rdpa_cpu_rx_reason_etype_ptp_1588},
        {"etype_802_1x", rdpa_cpu_rx_reason_etype_802_1x},
        {"etype_cfm", rdpa_cpu_rx_reason_etype_802_1ag_cfm},
        {"non_tcp_udp", rdpa_cpu_rx_reason_non_tcp_udp},
        {"ip_flow_miss", rdpa_cpu_rx_reason_ip_flow_miss},
        {"pci_ip_flow_miss1", rdpa_cpu_rx_reason_pci_ip_flow_miss_1},
        {"pci_ip_flow_miss2", rdpa_cpu_rx_reason_pci_ip_flow_miss_2},
        {"pci_ip_flow_miss3", rdpa_cpu_rx_reason_pci_ip_flow_miss_3},
        {"tcp_flags", rdpa_cpu_rx_reason_tcp_flags},
        {"ttl_expired", rdpa_cpu_rx_reason_ttl_expired},
        {"mtu_exceeded", rdpa_cpu_rx_reason_mtu_exceeded},
        {"l4_icmp", rdpa_cpu_rx_reason_l4_icmp},
        {"l4_esp", rdpa_cpu_rx_reason_l4_esp},
        {"l4_gre", rdpa_cpu_rx_reason_l4_gre},
        {"l4_ah", rdpa_cpu_rx_reason_l4_ah},
        {"parser_error", rdpa_cpu_rx_reason_parser_error},
        {"l4_ipv6", rdpa_cpu_rx_reason_l4_ipv6},
        {"l4_udef_0", rdpa_cpu_rx_reason_l4_udef_0},
        {"l4_udef_1", rdpa_cpu_rx_reason_l4_udef_1},
        {"l4_udef_2", rdpa_cpu_rx_reason_l4_udef_2},
        {"l4_udef_3", rdpa_cpu_rx_reason_l4_udef_3},
        {"cpu_redirect", rdpa_cpu_rx_reason_cpu_redirect},
        {"udef_0", rdpa_cpu_rx_reason_udef_0},
        {"udef_1", rdpa_cpu_rx_reason_udef_1},
        {"udef_2", rdpa_cpu_rx_reason_udef_2},
        {"udef_3", rdpa_cpu_rx_reason_udef_3},
        {"udef_4", rdpa_cpu_rx_reason_udef_4},
        {"udef_5", rdpa_cpu_rx_reason_udef_5},
        {"udef_6", rdpa_cpu_rx_reason_udef_6},
        {"udef_7", rdpa_cpu_rx_reason_udef_7},
        {NULL, 0}
    }
};

/** default rx isr handler */
static void _cpu_isr_dummy(long isr_priv)
{
    rdpa_cpu_port port = (rdpa_cpu_port)(isr_priv >> 16);
    bdmf_index queue = (bdmf_index)(isr_priv & 0xffff);
#ifndef RDP_SIM
    int sub_int;
#else
    rdpa_cpu_rx_info_t info = {};
#endif

    BUG_ON((unsigned)port >= rdpa_cpu_port__num_of);
    BUG_ON((unsigned)queue >= RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ);
#ifdef RDP_SIM
    /* Data queues isr dumps the packets into a TX file */
    BUG_ON(rdpa_cpu_packet_get(port, queue, &info));
#else
/*
#ifndef RUNNER_CPU_DQM
    if (cpu_irq_vector[port] == RDPA_PCI_MAIN_INTERRUPT_NUM_IN_RDD)
        sub_int = queue - RDPA_CPU_MAX_QUEUES;
    else
#endif
*/
        sub_int = queue;

    BDMF_TRACE_ERR("Unexpected interrupt: port=%d queue=%d. Disabled..\n",
        (int)port, (int)queue);

    rdpa_cpu_int_clear(port, sub_int);
    rdpa_cpu_int_disable(port, sub_int);
#endif
}

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int cpu_pre_init(struct bdmf_object *mo)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    cpu_data->index = rdpa_cpu_none;
    /* Assign drop callback to all queues */
    for (i = 0; i < RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ; i++)
    {
        cpu_data->rxq_cfg[i].rx_isr = _cpu_isr_dummy;
        cpu_data->rxq_cfg[i].ring_head = NULL;
        cpu_data->rxq_cfg[i].type = rdpa_ring_data;
        cpu_data->rxq_allocated_by_rdp[i] = 0;
    }
    for (i = 0; i < RDPA_CPU_TC_NUM; i++)
        cpu_data->tc_to_rxq[i] = (uint8_t)BDMF_INDEX_UNASSIGNED;

    return 0;
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int cpu_post_init(struct bdmf_object *mo)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle system_obj = NULL;
    rdpa_system_init_cfg_t system_init_cfg;
    rdpa_system_cfg_t system_cfg;
    int i, rc;

    if (cpu_data->index == rdpa_cpu_none)
    {
        for (i = rdpa_cpu_port_first; i < rdpa_cpu_port__num_of; i++)
        {
            if (!cpu_object[i])
            {
                cpu_data->index = i;
                break;
            }
        }
    }
    if (cpu_data->index >= rdpa_cpu_port__num_of)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many CPU objects or index %d is out of range\n", (int)cpu_data->index);

    /* set object name */
    if (cpu_object[cpu_data->index])
    {
        BDMF_TRACE_RET(BDMF_ERR_ALREADY,
            "CPU interface %s is already configured\n",
            bdmf_attr_get_enum_text_hlp(&cpu_port_enum_table, cpu_data->index));
    }
    snprintf(mo->name, sizeof(mo->name), "cpu/index=%s",
        bdmf_attr_get_enum_text_hlp(&cpu_port_enum_table, cpu_data->index));

    if (!cpu_data->num_queues)
    {
        rc = cpu_rxq_cfg_max_num_set(cpu_data);
        if (rc)
            return rc;
    }

    /* learn info from system */
    rc = rdpa_system_get(&system_obj);
    rc = rc ? rc : rdpa_system_init_cfg_get(system_obj, &system_init_cfg);
    rc = rc ? rc : rdpa_system_cfg_get(system_obj, &system_cfg);
    if (system_obj)
        bdmf_put(system_obj);
    if (rc < 0)
        return rc;
    cpu_data->headroom_size = system_cfg.headroom_size;

    /* Set-up priv data for default dropper ISR */
    for (i = 0; i < RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ; i++)
    {
        if (cpu_data->rxq_cfg[i].rx_isr == _cpu_isr_dummy)
            cpu_data->rxq_cfg[i].isr_priv = (cpu_data->index << 16) | i;
    }
#ifndef RUNNER_CPU_DQM_RX
    /* Initialize pbuf support */
    cpu_data->headroom_size += RDD_PACKET_HEADROOM_OFFSET;
    bdmf_pbuf_init(rdpa_bpm_buffer_size_get(), cpu_data->headroom_size);

    for (i = 0; i < bdmf_sysb_type__num_of; i++)
        bdmf_sysb_headroom_size_set(i, cpu_data->headroom_size);
#endif
    /*Note that IRQs are not connected at this point. IRQs need to be connected explicitly through
     *the int_connect CPU attribute*/

    cpu_object[cpu_data->index] = mo;
    return cpu_post_init_ex(mo);
}

static void cpu_destroy(struct bdmf_object *mo)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);

    if (mo != cpu_object[cpu_data->index])
        return;

    cpu_destroy_ex(mo);
    cpu_object[cpu_data->index] = NULL;
}

/*  cpu_reason_index aggregate type */
struct bdmf_aggr_type cpu_reason_index_type = {
    .name = "cpu_reason_index", .struct_name = "rdpa_cpu_reason_index_t",
    .help = "CPU trap reason index",
    .fields = (struct bdmf_attr[]) {
        { .name = "dir", .help = "Traffic direction", .type = bdmf_attr_enum,
            .size = sizeof(rdpa_traffic_dir),
            .offset = offsetof(rdpa_cpu_reason_index_t, dir),
            .ts.enum_table = &rdpa_traffic_dir_enum_table
        },
        { .name = "reason", .help = "Trap reason", .type = bdmf_attr_enum,
          .size = sizeof(rdpa_cpu_reason),
          .offset = offsetof(rdpa_cpu_reason_index_t, reason),
          .ts.enum_table = &rdpa_cpu_reason_enum_table
        },
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
        { .name = "table_index", .help = "Reason table index", .type = bdmf_attr_number,
          .size = sizeof(int),
          .offset = offsetof(rdpa_cpu_reason_index_t, table_index)
        },
#endif
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_reason_index_type);

/*  cpu_reason_cfg aggregate type */
struct bdmf_aggr_type cpu_reason_cfg_type = {
    .name = "cpu_reason_cfg", .struct_name = "rdpa_cpu_reason_cfg_t",
    .help = "CPU trap reason configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "queue", .help = "CPU queue, ignored for XRDP", .type = bdmf_attr_number,
            .size = sizeof(bdmf_index),
            .offset = offsetof(rdpa_cpu_reason_cfg_t, queue),
#if defined(CONFIG_BCM_DSL_RDP)
            .max_val = RDPA_CPU_MAX_QUEUES + RDPA_WLAN_MAX_QUEUES - 1
#else
            .max_val = RDPA_CPU_MAX_QUEUES - 1
#endif
        },
        { .name = "meter", .help = "CPU meter (policer) index",
            .type = bdmf_attr_number,
            .size = sizeof(bdmf_index),
            .offset = offsetof(rdpa_cpu_reason_cfg_t, meter),
            .min_val = BDMF_INDEX_UNASSIGNED, .max_val = RDPA_CPU_MAX_METERS - 1,
        },
        { .name = "meter_ports",
            .help = "Source ports for which CPU traffic is rate-limited",
            .type = bdmf_attr_enum_mask, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_ports),
            .offset = offsetof(rdpa_cpu_reason_cfg_t, meter_ports)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_reason_cfg_type);

/*  cpu_rx_stat aggregate type */
struct bdmf_aggr_type cpu_rx_stat_type = {
    .name = "cpu_rx_stat", .struct_name = "rdpa_cpu_rx_stat_t",
    .help = "CPU trap reason statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "received", .help = "Packets received",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_rx_stat_t, received)
        },
        { .name = "queued", .help = "Packets in the queue",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_rx_stat_t, queued)
        },
        { .name = "dropped", .help = "Packets dropped",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_rx_stat_t, dropped)
        },
        { .name = "interrupts", .help = "Interrupts", .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_rx_stat_t, interrupts)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_rx_stat_type);

/*  cpu_meter_cfg aggregate type */
struct bdmf_aggr_type cpu_meter_cfg_type = {
    .name = "cpu_meter_cfg", .struct_name = "rdpa_cpu_meter_cfg_t",
    .help = "CPU meter configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "sir", .help = "SiR", .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_meter_cfg_t, sir)
        },
        { .name = "burst_size", .help = "Burst size", .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_meter_cfg_t, burst_size)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_meter_cfg_type);

/* cpu_tx_method enum values */
bdmf_attr_enum_table_t rdpa_ring_type_enum_table = {
    .type_name = "rdpa_ring_type_t",
    .values = {
        { "data", rdpa_ring_data},
        { "recycle", rdpa_ring_recycle},
        { "feed", rdpa_ring_feed},
        { "cpu_tx", rdpa_ring_cpu_tx},
        { NULL, 0}
    }
};

/*  cpu_rxq_cfg aggregate type */
struct bdmf_aggr_type cpu_rxq_cfg_type = {
    .name = "cpu_rxq_cfg", .struct_name = "rdpa_cpu_rxq_cfg_t",
    .help = "CPU receive configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "rx_isr", .help = "Rx ISR", .type = bdmf_attr_pointer,
            .size = sizeof(void *),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, rx_isr)
        },
        { .name = "ring_head", .help = "Ring head pointer",
            .type = bdmf_attr_pointer,
            .size = sizeof(void *),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, ring_head),
        },
        { .name = "size", .help = "Queue size", .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, size)
        },
        { .name = "type", .help = "Queue type", .type = bdmf_attr_enum,
            .size = sizeof(rdpa_ring_type_t),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, type),
            .ts.enum_table = &rdpa_ring_type_enum_table
        },
        { .name = "dump_data", .help = "Dump Rx data",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, dump)
        },
        { .name = "isr_priv", .help = "Private data passed to rx_isr",
            .type = bdmf_attr_number, .size = sizeof(long),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, isr_priv)
        },
        { .name = "rx_dump_data_cb", .help = "RX data dump callback function", .type = bdmf_attr_pointer,
            .size = sizeof(void *),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, rx_dump_data_cb)
        },
        { .name = "rxq_stat", .help = "Statistics callback",
            .type = bdmf_attr_pointer,
            .size = sizeof(void *),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, rxq_stat),
        },
        { .name = "irq_affinity_mask", .help = "Queue IRQ affinity mask",
            .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, irq_affinity_mask),
        },
        { .name = "priority_q", .help = "Queue Priority",
            .type = bdmf_attr_number,
            .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_rxq_cfg_t, ring_prio),
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_rxq_cfg_type);


/* send as */
typedef enum {
    rdpa_cpu_dbg_send_raw,
    rdpa_cpu_dbg_send_skb,
} rdpa_cpu_dbg_send_as;

/* cpu_tx_send_as enum values */
static bdmf_attr_enum_table_t cpu_tx_send_as_enum_table = {
    .type_name = "rdpa_cpu_dbg_send_as",
    .values = {
        { "raw", rdpa_cpu_dbg_send_raw},
        { "skb", rdpa_cpu_dbg_send_skb},
        {NULL,      0}
    }
};

/*
 * Debugging support
 * Aggregate for "send_packet" attribute
 */
typedef struct {
    rdpa_cpu_tx_method method;  /* Packet transmit method */
    rdpa_if port;               /* Destination/source port */
    uint8_t ssid;               /* SSID, in use if source port is wlan */
    uint32_t queue_id;          /* egress queue id */
    rdpa_flow wan_flow;
    rdpa_cpu_dbg_send_as send_as;
    rdpa_discard_prty drop_precedence;
} rdpa_cpu_dbg_tx_info_t;

/* cpu_tx_method enum values */
bdmf_attr_enum_table_t cpu_tx_method_enum_table = {
    .type_name = "rdpa_cpu_tx_method",
    .values = {
        { "port", rdpa_cpu_tx_port},
        { "bridge", rdpa_cpu_tx_bridge},
        {NULL,      0}
    }
};

/*  cpu_tx_info aggregate type */
struct bdmf_aggr_type cpu_tx_info_type = {
    .name = "cpu_tx_info", .struct_name = "rdpa_cpu_dbg_tx_info_t",
    .help = "CPU Tx info (debugging support)",
    .fields = (struct bdmf_attr[]) {
        { .name = "method", .help = "Tx method", .type = bdmf_attr_enum,
            .size = sizeof(rdpa_cpu_tx_method),
            .offset = offsetof(rdpa_cpu_dbg_tx_info_t, method),
            .ts.enum_table = &cpu_tx_method_enum_table
        },
        { .name = "port",
            .help = "Source port for method=bridge, dst port otherwise",
            .type = bdmf_attr_enum, .size = sizeof(rdpa_if),
            .offset = offsetof(rdpa_cpu_dbg_tx_info_t, port),
            .ts.enum_table = &rdpa_if_enum_table
        },
        { .name = "ssid",
            .help = "SSID, in use if source port is wlan for method=bridge",
            .type = bdmf_attr_enum, .size = sizeof(uint8_t),
            .offset = offsetof(rdpa_cpu_dbg_tx_info_t, ssid),
            .ts.enum_table = &rdpa_wlan_ssid_enum_table
        },
        { .name = "queue_id", .help = "Egress queue id: for method=port",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_dbg_tx_info_t, queue_id)
        },
        { .name = "wan_flow", .help = "for port=wan only: Source flow for "
            "method=bridge, destination flow otherwise",
            .type = bdmf_attr_number, .size = sizeof(rdpa_flow),
            .offset = offsetof(rdpa_cpu_dbg_tx_info_t, wan_flow)
        },
        { .name = "as", .help = "Send using s/w interface",
            .type = bdmf_attr_enum, .ts.enum_table = &cpu_tx_send_as_enum_table,
            .size = sizeof(rdpa_cpu_dbg_send_as),
            .offset = offsetof(rdpa_cpu_dbg_tx_info_t, send_as)
        },
        { .name = "drop_precedence", .help = "Drop precedence", .size = sizeof(rdpa_discard_prty),
          .type = bdmf_attr_enum, .ts.enum_table = &rdpa_disc_prty_enum_table,
          .offset = offsetof(rdpa_cpu_dbg_tx_info_t, drop_precedence)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_tx_info_type);


/*  cpu_tx_dump aggregate type */
struct bdmf_aggr_type cpu_tx_dump_type = {
    .name = "cpu_tx_dump", .struct_name = "cpu_tx_dump_mode_t",
    .help = "CPU Tx dump filter (debugging support)",
    .fields = (struct bdmf_attr[]) {
        { .name = "enable", .help = "Enable dumping tx packets",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(cpu_tx_dump_mode_t, enable)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_tx_dump_type);

/*  cpu_tx_stat aggregate type */
struct bdmf_aggr_type cpu_tx_stat_type = {
    .name = "cpu_tx_stat", .struct_name = "rdpa_cpu_tx_stat_t",
    .help = "CPU transmit statistics (debugging support)",
    .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[]) {
        { .name = "ok", .help = "Successfully passed to RDD for transmission",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_tx_stat_t, tx_ok)
        },
        { .name = "invalid_queue",
            .help = "Discarded because transmit queue is not configured",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_tx_stat_t, tx_invalid_queue)
        },
        { .name = "no_buf",
            .help = "Discarded because couldn't allocate BPM buffer",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_tx_stat_t, tx_no_buf)
        },
        { .name = "too_long",
            .help = "Discarded packets with length > max MTU size",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_tx_stat_t, tx_too_long)
        },
        { .name = "rdd_error", .help = "Discarded because RDD returned error",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_cpu_tx_stat_t, tx_rdd_error)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_tx_stat_type);

static int _validate_cpu_meter_index(rdpa_dir_index_t *dindex)
{
    if (!dindex || *(bdmf_index *)dindex == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_PARM;
    if (dindex->index == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;
    if ((unsigned)dindex->index >= cpu_max_meters_per_dir[dindex->dir])
        return BDMF_ERR_RANGE;
    if (!meter[dindex->dir][dindex->index].sir)
        return BDMF_ERR_NOENT;

    return BDMF_ERR_OK;
}

/*
 * Attribute access functions
 */

/* "rx_cfg" attribute "read" callback */
static int cpu_attr_rxq_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_rxq_cfg_t *rx_cfg = (rdpa_cpu_rxq_cfg_t *)val;

    if (_check_queue_range(cpu_data, (unsigned)index))
        return BDMF_ERR_RANGE;

    *rx_cfg = cpu_data->rxq_cfg[index];

    return 0;
}

static int dump_data_change_only(rdpa_cpu_rxq_cfg_t *old, rdpa_cpu_rxq_cfg_t *new)
{
    rdpa_cpu_rxq_cfg_t tmp;

    if (old->dump == new->dump)
        return 0;
    memcpy(&tmp, old, sizeof(rdpa_cpu_rxq_cfg_t));
    tmp.dump = new->dump;
    return memcmp(&tmp, new, sizeof(rdpa_cpu_rxq_cfg_t)) ? 0 : 1;
}

/* "rxq_cfg" attribute "write" callback */
int cpu_attr_rxq_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_rxq_cfg_t *rx_cfg = (rdpa_cpu_rxq_cfg_t *)val;
    rdpa_cpu_rxq_cfg_t rxc, rxc_temp;
    int rc = 0;
    uint32_t write_idx, size_of_entry;

    if (_check_queue_range(cpu_data, (unsigned)index))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo, "Queue index %ld out of range\n",
            index);
    }

    rxc = *rx_cfg;
    rxc_temp = *rx_cfg;

    /* check there is a change */
    if (!memcmp(&cpu_data->rxq_cfg[index], &rxc, sizeof(*rx_cfg)))
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo,
            "Queue %ld is already configured with same attributes values\n",
            index);
    }

    if ((rxc.rx_isr != cpu_data->rxq_cfg[index].rx_isr) && (cpu_data->rxq_cfg[index].size != 0))
    {
        rxc_temp.rx_isr = cpu_data->rxq_cfg[index].rx_isr;
        if (!memcmp(&cpu_data->rxq_cfg[index], &rxc_temp, sizeof(*rx_cfg)))
        {
            cpu_data->rxq_cfg[index].rx_isr = rxc.rx_isr;
            return 0;
        }
    }

    rc = cpu_rxq_cfg_size_validate_ex(cpu_data, rx_cfg);
    if (rc)
    {
        return rc;
    }

    cpu_rxq_cfg_params_init_ex(cpu_data, rx_cfg, &size_of_entry, &write_idx);

    if (rxc.ic_cfg.ic_enable)
    {
        if (rxc.ic_cfg.ic_timeout_us == 0 ||
            rxc.ic_cfg.ic_timeout_us > RDPA_CPU_IC_MAX_TIMEOUT_IN_US)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo,
                "Interrupt coalescing timeout %d out of range 1 - %d\n",
                rxc.ic_cfg.ic_timeout_us, RDPA_CPU_IC_MAX_TIMEOUT_IN_US);
        }
        if (rxc.ic_cfg.ic_max_pktcnt == 0 ||
            rxc.ic_cfg.ic_max_pktcnt > RDPA_CPU_IC_MAX_PKT_CNT)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo,
                "Interrupt coalescing max pkt count %d > %d\n",
                rxc.ic_cfg.ic_max_pktcnt, RDPA_CPU_IC_MAX_PKT_CNT);
        }
    }

    /* Set default callbacks if both callbacks are NULL */
    if (!rxc.rx_isr)
    {
        rxc.rx_isr = _cpu_isr_dummy;
        rxc.isr_priv = (cpu_data->index << 16) | index;
    }

    if (dump_data_change_only(&cpu_data->rxq_cfg[index], &rxc))
    {
        cpu_data->rxq_cfg[index].dump = rxc.dump;

        /* Signal packet dump has been enabled on queue */
        if (cpu_data->rxq_cfg[index].rx_dump_data_cb)
            cpu_data->rxq_cfg[index].rx_dump_data_cb(index, rxc.dump);

        return 0;
    }

    /* Configure RDD */
    {
        uint8_t rdd_ring_idx = cpu_rdd_rxq_idx_get(cpu_data, index);
        bdmf_boolean allocated_by_rdp = cpu_data->rxq_allocated_by_rdp[index];
        bdmf_boolean ring_exists = (cpu_data->rxq_cfg[index].size && cpu_data->rxq_cfg[index].ring_head);
        bdmf_phys_addr_t rw_idx_addr = 0;
        uint8_t thr_tc;

        if (rxc.type != rdpa_ring_data || rxc.size != cpu_data->rxq_cfg[index].size)
        {
            if (rdd_ring_idx == (uint8_t)BDMF_INDEX_UNASSIGNED)
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo,
                    "rdd error %d when configuring queue %ld\n", rc, index);
            }

            /* If rxc.type == rdpa_ring_data, then we are changing the allocation or deleting the ring */
            if (rxc.type == rdpa_ring_data)
            {
#ifdef XRDP
                /* XRDP interrupt disconnect */
                if (cpu_data->rxq_cfg[index].size != 0)
                    rdpa_cpu_int_disconnect_ex(cpu_data, index);
#endif

                /* If an existing ring exists, then destroy it first. */
                if (ring_exists)
                {
                    rdd_ring_destroy(rdd_ring_idx);
                    /* If we allocated the buffers, then destroy them */
                    if (allocated_by_rdp)
                    {
                        rc = rdp_cpu_ring_delete_ring(rdd_ring_idx);
                        if (rxc.ring_head == cpu_data->rxq_cfg[index].ring_head)
                        {
                            rxc.ring_head = NULL;
                        }
                    }
                    /* Mark the ring as null and void & indicate we don't have buffers allocated by rdp */
                    cpu_data->rxq_allocated_by_rdp[index] = 0;
                    cpu_data->rxq_cfg[index].size = 0;
                    cpu_data->rxq_cfg[index].ring_head = NULL;
                }
            }

            /* Is ring allocation requested && we won't overwrite existing ring_head? */
            if (rxc.size)
            {
                if (!rxc.ring_head)
                {
                    rc = rc ? rc : rdp_cpu_ring_create_ring_ex(rdd_ring_idx, rxc.type, rxc.size,
                        (bdmf_phys_addr_t *)&rxc.ring_head, &rw_idx_addr, BCM_PKTBUF_SIZE, NULL, rxc.ring_prio);

                    if (rc)
                    {
                        BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo,
                            "rdd error %d when configuring queue %ld (rdd idx %d)\n", rc, index, rdd_ring_idx);
                    }
                    else
                    {
                        cpu_data->rxq_allocated_by_rdp[index] = 1;
                    }
                }
                cpu_data->rxq_cfg[index].size = rxc.size;
                cpu_data->rxq_cfg[index].ring_head = rxc.ring_head;
            }
            else
            {
                cpu_data->rxq_cfg[index].size = 0;
                cpu_data->rxq_cfg[index].ring_head = NULL;
                rxc.ring_head = NULL;
            }
        }
        thr_tc = _rdpa_system_high_prio_tc_thr_get();
        rdd_ring_init(rdd_ring_idx, rxc.type, (bdmf_phys_addr_t)(uintptr_t)rxc.ring_head, rxc.size,
            size_of_entry, rdd_ring_idx, write_idx, rw_idx_addr, thr_tc);

        if (rxc.ic_cfg.ic_enable)
        {
            rc = rdd_cpu_rx_interrupt_coalescing_config(rdd_ring_idx, rxc.ic_cfg.ic_timeout_us,
                rxc.ic_cfg.ic_max_pktcnt);
            if (rc)
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo,
                    "rdd error %d when configuring interrupt coalescing %ld (rdd idx %d)\n", rc, index,
                    rdd_ring_idx);
            }
        }

        if (rxc.size) /* XXX: Should be true for data only? */
        {
#ifdef XRDP
            rc = rdpa_cpu_int_connect_ex(cpu_data, index, rxc.irq_affinity_mask);
            if (rc)
            {
                rdd_ring_destroy(rdd_ring_idx);
                if (allocated_by_rdp)
                    rc = rdp_cpu_ring_delete_ring(rdd_ring_idx);
                cpu_data->rxq_allocated_by_rdp[index] = 0;
                cpu_data->rxq_cfg[index].size = 0;
                cpu_data->rxq_cfg[index].ring_head = NULL;
                BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                    "Error %d when connecting interrupt for queue %d (rdd idx %d)\n", rc, (int)index,
                    (int)rdd_ring_idx);
            }
#endif
        }
    }

    /* Signal packet dump has been enabled on queue */
    if (rxc.rx_dump_data_cb)
        rxc.rx_dump_data_cb(index, rxc.dump);

    cpu_data->rxq_cfg[index] = rxc;

    return 0;
}

/* "meter_cfg" attribute "read" callback */
static int cpu_attr_meter_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    rdpa_cpu_meter_cfg_t *meter_cfg = (rdpa_cpu_meter_cfg_t *)val;
    rdpa_dir_index_t *dindex = (rdpa_dir_index_t *)index;
    int rc;

    rc = _validate_cpu_meter_index(dindex);
    if (rc)
        return rc;

    *meter_cfg = meter[dindex->dir][dindex->index];

    return BDMF_ERR_OK;
}

/* "meter_stat" attribute "read" callback */
static int cpu_attr_meter_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
#ifndef RUNNER_CPU_DQM_RX
    uint32_t *stat = (uint32_t *)val;
    rdpa_dir_index_t *dindex = (rdpa_dir_index_t *)index;
    int rc;
    uint32_t drop_count;

    rc = _validate_cpu_meter_index(dindex);
    if (rc)
        return rc;

    rc = rdd_cpu_rx_meter_drop_counter_get(dindex->index, dindex->dir, &drop_count);
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "rdd_cpu_rx_meter_drop_counter_get failed. rc=%d\n", rc);

    *stat = drop_count;
    
#else
    BDMF_TRACE_RET_OBJ(BDMF_ERR_NOENT, mo, "meter counters not support in RUNNER_CPU_DQM_RX mode");
#endif
    return BDMF_ERR_OK;
}

/* "meter_stat" attribute "write" callback */
static int cpu_attr_meter_stat_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#ifndef RUNNER_CPU_DQM_RX
    rdpa_dir_index_t *dindex = (rdpa_dir_index_t *)index;
    rdpa_traffic_dir dir = dindex->dir;
    bdmf_index counter_id = dindex->index;
    int rc;

    rc = _validate_cpu_meter_index(dindex);
    if (rc)
        return rc;

    if (dir == rdpa_dir_us)
        counter_id += RDD_DS_CPU_RX_METER_TABLE_SIZE;

    rdpa_cpu_rx_meter_clean_stat_ex(counter_id);
#else
    BDMF_TRACE_RET_OBJ(BDMF_ERR_NOENT, mo, "meter counters not support in RUNNER_CPU_DQM_RX mode");
#endif
    return 0;
}

/* "meter_cfg" attribute "write" callback */
static int cpu_attr_meter_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    rdpa_cpu_meter_cfg_t *meter_cfg = (rdpa_cpu_meter_cfg_t *)val;
    rdpa_dir_index_t *dindex = (rdpa_dir_index_t *)index;

    if (!dindex || *(bdmf_index *)dindex == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_PARM;
    if ((unsigned)dindex->index >= cpu_max_meters_per_dir[dindex->dir])
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo,
            "meter index %d is out of range %d-%d\n",
            (int)dindex->index, (int)BDMF_INDEX_UNASSIGNED,
            cpu_max_meters_per_dir[dindex->dir]-1);
    }
    /* validate meter parameters */
    if ((meter_cfg->sir && meter_cfg->sir < RDPA_CPU_METER_MIN_SR) ||
        meter_cfg->sir > RDPA_CPU_METER_MAX_SR ||
        meter_cfg->sir % RDPA_CPU_METER_SR_QUANTA)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "SiR rate %d is invalid\n",
            meter_cfg->sir);
    }

    rdd_cpu_rx_meter_config(dindex->index, meter_cfg->sir, meter_cfg->burst_size, dindex->dir);
    meter[dindex->dir][dindex->index] = *meter_cfg;

    return 0;
}

/* "meter" attribute "get_next" callback */
static int cpu_attr_meter_get_next(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index *index)
{
    rdpa_dir_index_t *dindex = (rdpa_dir_index_t *)index;
    int max_meters = (!index || *index == BDMF_INDEX_UNASSIGNED) ?
        RDPA_CPU_MAX_DS_METERS : cpu_max_meters_per_dir[dindex->dir];

    return rdpa_dir_index_get_next(dindex, max_meters);
}

/* "rxq_flush" attribute "write" callback */
static int cpu_attr_rxq_flush_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t write_idx, size_of_entry;

    if (_check_queue_range(cpu_data, (unsigned)index))
        return BDMF_ERR_RANGE;
    if (!cpu_data->rxq_cfg[index].size)
        return BDMF_ERR_NOENT;

    cpu_rxq_cfg_params_init_ex(cpu_data, &cpu_data->rxq_cfg[index], &size_of_entry, &write_idx);

    /* flush in RDD */
    {
        /* Flush the ring */
        int rc;
        uint8_t rdd_ring_idx = cpu_rdd_rxq_idx_get(cpu_data, index);
        uint8_t thr_tc;

        rc = rdd_ring_destroy(rdd_ring_idx);
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_IO, mo, "Failed in rdd_ring_destroy, rc = %d\n", rc);
            return BDMF_ERR_IO;
        }
        rdp_cpu_ring_flush(rdd_ring_idx);
        thr_tc = _rdpa_system_high_prio_tc_thr_get();
        rdd_ring_init(rdd_ring_idx, cpu_data->rxq_cfg[index].type,
            (bdmf_phys_addr_t)(uintptr_t)cpu_data->rxq_cfg[index].ring_head,
            cpu_data->rxq_cfg[index].size, size_of_entry, rdd_ring_idx, write_idx, 0, thr_tc);
    }
    return 0;
}

/* Check if per-port metering is supported for the reason. return 1 if yes */
int cpu_is_per_port_metering_supported(rdpa_cpu_reason reason)
{
    if (reason == rdpa_cpu_rx_reason_bcast || reason == rdpa_cpu_rx_reason_mcast ||
        reason == rdpa_cpu_rx_reason_unknown_da)
    {
        return 1;
    }
    return 0;
}

int cpu_per_port_reason_index(rdpa_cpu_reason reason)
{
    if (reason == rdpa_cpu_rx_reason_bcast)
        return 0;
    if (reason == rdpa_cpu_rx_reason_mcast)
        return 1;
    if (reason == rdpa_cpu_rx_reason_unknown_da)
        return 2;
    BUG();
    return 0;
}


/* "reason_cfg" attribute "read" callback */
static int cpu_attr_reason_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;
    rdpa_cpu_reason_cfg_t *cfg = (rdpa_cpu_reason_cfg_t *)val;

    if (cpu_data->index != rdpa_cpu_host && cpu_data->index != rdpa_cpu_wlan0)
        return BDMF_ERR_NOT_SUPPORTED;
    if (!rindex || *(bdmf_index *)rindex == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;
    *cfg = reason_cfg[rindex->dir][rindex->reason];

    /* US reasons that support per-port metering require special handling.
     * For those reasons meter_ports mask is stored per meter
     */
    if (rindex->dir == rdpa_dir_us &&
        cpu_is_per_port_metering_supported(rindex->reason))
    {
        int ri = cpu_per_port_reason_index(rindex->reason);
        if ((unsigned)rindex->entry_index >= RDPA_CPU_MAX_US_METERS)
        {
            cfg->meter = BDMF_INDEX_UNASSIGNED;
            cfg->meter_ports = 0;
        }
        else
        {
            cfg->meter_ports = us_meter_ports[ri][rindex->entry_index];
            if (cfg->meter_ports)
                cfg->meter = rindex->entry_index;
            else
                cfg->meter = BDMF_INDEX_UNASSIGNED;
        }
    }
    return 0;
}

/* configure metering in RDD */
int cpu_meter_cfg_rdd(struct bdmf_object *mo, const rdpa_cpu_reason_index_t *rindex, int meter, rdpa_ports ports)
{
    uint32_t port_vector = (uint32_t)rdpa_ports_to_rdd_egress_port_vector(ports, 0);
    rdd_cpu_rx_meter rdd_meter = CPU_RX_METER_DISABLE;
    int rdd_rc;

    if (meter != BDMF_INDEX_UNASSIGNED)
        rdd_meter = meter;

    rdd_rc = rdd_cpu_reason_to_cpu_rx_meter(rindex->reason, rdd_meter, rindex->dir, port_vector);
    BDMF_TRACE_DBG_OBJ(mo,
        "rdd_cpu_reason_to_cpu_rx_meter(%d, %d, %d, 0x%x) --> %d\n",
        (int)rindex->reason, (int)rdd_meter, (int)rindex->dir,
        (unsigned)port_vector, rdd_rc);
    if (rdd_rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "rdd_cpu_reason_to_cpu_rx_meter(%d, %d, %d, 0x%x) --> %d\n",
            (int)rindex->reason, (int)rdd_meter, (int)rindex->dir,
            (int)port_vector, rdd_rc);
    }
    return 0;
}

/* configure per-port metering */
int cpu_per_port_reason_meter_cfg(struct bdmf_object *mo, const rdpa_cpu_reason_index_t *rindex,
    const rdpa_cpu_reason_cfg_t *reason_cfg)
{
    int ri = cpu_per_port_reason_index(rindex->reason);
    rdpa_ports other_ports = ~reason_cfg->meter_ports;
    rdpa_ports ports;
    int rc = 0;
    int i;

    /* Go over all "other" meters and "subtract" ports being configured for the
     * new meter */
    for (i = 0; i < RDPA_CPU_MAX_US_METERS; i++)
    {
        ports = us_meter_ports[ri][i] & other_ports;
        if (ports != us_meter_ports[ri][i] && i != reason_cfg->meter)
        {
            rc = rc ? rc : cpu_meter_cfg_rdd(mo, rindex, i, ports);
            if (!rc)
                us_meter_ports[ri][i] = ports;
        }
    }

    /* Handle "this" meter if any */
    if (reason_cfg->meter >= 0)
    {
        /* If new meter is set, disable all ports that used to be associated
         * with this meter, but not anymore */
        ports = us_meter_ports[ri][reason_cfg->meter] & other_ports;
        if (ports)
            rc = rc ? rc : cpu_meter_cfg_rdd(mo, rindex, -1, ports);
        /* Enable meter on ports if any */
        if (reason_cfg->meter_ports)
        {
            rc = rc ? rc : cpu_meter_cfg_rdd(mo, rindex, reason_cfg->meter,
                reason_cfg->meter_ports);
        }

        /* Update mask if ok */
        if (!rc)
        {
            us_meter_ports[ri][reason_cfg->meter] =
                reason_cfg->meter_ports;
        }
    }

    return rc;
}

/* "reason_cfg" attribute "write" callback */
int cpu_attr_reason_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;
    const rdpa_cpu_reason_cfg_t *cfg = (const rdpa_cpu_reason_cfg_t *)val;
    rdpa_ports all_lan_ports = rdpa_ports_all_lan();
    int rc;

    if (cpu_data->index != rdpa_cpu_host && cpu_data->index != rdpa_cpu_wlan0)
        return BDMF_ERR_NOT_SUPPORTED;

    if (!rindex || *(bdmf_index *)rindex == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;

    /* Verify meter configuration */
    if (cfg->meter != BDMF_INDEX_UNASSIGNED)
    {
        if ((unsigned)cfg->meter >= cpu_max_meters_per_dir[rindex->dir])
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_RANGE, mo,
                "CPU meter %ld is out of range %d-%d\n", cfg->meter,
                BDMF_INDEX_UNASSIGNED, cpu_max_meters_per_dir[rindex->dir] - 1);
        }

        if (!meter[rindex->dir][cfg->meter].sir)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOENT, mo,
                "CPU meter %ld is not configured\n", cfg->meter);
        }

        if (cfg->meter_ports)
        {
            if ((rindex->dir == rdpa_dir_ds &&
                (cfg->meter_ports & ~RDPA_PORT_ALL_WAN)) ||
                (rindex->dir == rdpa_dir_us &&
                (cfg->meter_ports & ~all_lan_ports)))
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                    "CPU meter_ports mask %lld is invalid or doesn't match "
                    "direction\n", (long long int)cfg->meter_ports);
            }

            /* per-port metering is only allowed for some reasons */
            if (rindex->dir == rdpa_dir_us &&
                !cpu_is_per_port_metering_supported(rindex->reason) &&
                cfg->meter_ports &&
                cfg->meter_ports != all_lan_ports)
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo,
                    "Per-port metering is not supported for reason %s (%d)\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_cpu_reason_enum_table,
                    rindex->reason), rindex->reason);
            }
        }
    }

    rc = cpu_reason_cfg_validate_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    rc = cpu_reason_cfg_rdd_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    reason_cfg[rindex->dir][rindex->reason] = *cfg;

    /* Clear meter_ports if meter is disabled */
    if (cfg->meter == BDMF_INDEX_UNASSIGNED)
        reason_cfg[rindex->dir][rindex->reason].meter_ports = 0;

    return 0;
}

/* "reason_cfg", "reason_stat" attribute "get_next" callback */
int cpu_attr_reason_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;

    if (!rindex)
        return BDMF_ERR_PARM;

    if (*(bdmf_index *)rindex == BDMF_INDEX_UNASSIGNED)
    {
        /* get first */
        rindex->dir = rdpa_dir_ds;
        rindex->reason = 0;
        rindex->entry_index = -1;
        return 0;
    }

    /* get next */
    do
    {
        ++rindex->reason;
        rindex->entry_index = -1;
        if (rindex->reason >= rdpa_cpu_reason__num_of)
        {
            if (rindex->dir == rdpa_dir_us)
                return BDMF_ERR_NO_MORE;
            rindex->dir = rdpa_dir_us;
            rindex->reason = -1;
        }
    } while (!bdmf_attr_get_enum_text_hlp(&rdpa_cpu_reason_enum_table,
        rindex->reason));

    return 0;
}

/* "reason_cfg" attribute "get_next" callback */
int cpu_attr_reason_entry_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;
    int rc, ri, i, old_entry_index;

    if (cpu_data->index != rdpa_cpu_host)
        return BDMF_ERR_NOT_SUPPORTED;

    if (!rindex)
        return BDMF_ERR_PARM;

    if (*(bdmf_index *)rindex == BDMF_INDEX_UNASSIGNED ||
        rindex->dir != rdpa_dir_us ||
        !cpu_is_per_port_metering_supported(rindex->reason) ||
        rindex->entry_index >= RDPA_CPU_MAX_US_METERS)
    {
        rc = cpu_attr_reason_get_next(mo, ad, index);

        /* If we ended up with reason that doesn't support per-port metering -
         * all done */
        if (rc == BDMF_ERR_NO_MORE || rindex->dir != rdpa_dir_us ||
            !cpu_is_per_port_metering_supported(rindex->reason))
        {
            return rc;
        }
    }

    /* We are dealing with reason that supports per-port metering.
     * Look for meter entry with non-zero port mask
     */
    ri = cpu_per_port_reason_index(rindex->reason);
    for (i = rindex->entry_index+1; i < RDPA_CPU_MAX_US_METERS; i++)
    {
        if (us_meter_ports[ri][i])
            break;
    }
    old_entry_index = rindex->entry_index;
    rindex->entry_index = i;

    /* Return ok if found active entry or if there were no active entries for
     * this reasons */
    if (i < RDPA_CPU_MAX_US_METERS || old_entry_index < 0)
        return 0;

    /* Done with this reason. Proceed to the next */
    return cpu_attr_reason_entry_get_next(mo, ad, index);
}

int cpu_attr_reason_cfg_val_to_s(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val, char *sbuf,
    uint32_t size)
{
    rdpa_cpu_reason_cfg_t *cfg = (rdpa_cpu_reason_cfg_t *)val;
    char tmp[256] = {}, meter_ports_str[256] = {}, *p;
    int n, rc;

    p = tmp;

#ifdef XRDP
    n = snprintf(p, size, "{meter=%d", (int)cfg->meter);
#else
    n = snprintf(p, size, "{queue=%d,meter=%d", (int)cfg->queue, (int)cfg->meter);
#endif
    p += n;
    size -= n;
    if (!size)
        return BDMF_ERR_OVERFLOW;

    if (cfg->meter_ports)
    {
        /* From cpu_reason_cfg aggregate definition, meter_ports mask is attribute #2. */
        rc = ad->aggr_type->fields[2].val_to_s(mo, &ad->aggr_type->fields[2], &cfg->meter_ports, meter_ports_str, 256);
        if (rc)
            return rc;
        n = snprintf(p, size, ",meter_ports=%s", meter_ports_str);
        p += n;
        size -= n;
        if (!size)
            return BDMF_ERR_OVERFLOW;
    }
    n = snprintf(p, size, "}");
    size -= n;
    if (!size)
        return BDMF_ERR_OVERFLOW;

    strcpy(sbuf, tmp);
    return 0;
}

/* "reason_stat" attribute "read" callback */
static int cpu_attr_reason_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;
    uint32_t *stat = (uint32_t *)val;

    if (!rindex || *(bdmf_index *)rindex == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;

#ifndef XRDP
    if (cpu_data->index != rdpa_cpu_host)
        return BDMF_ERR_NOT_SUPPORTED;
#endif

    if (cpu_data->reason_stat_external_cb)
        cpu_data->reason_stat_external_cb(stat, rindex);
    else
    {
        *stat = cpu_data->reason_stat[rindex->dir][rindex->reason];

        /* Clear after read */
        cpu_data->reason_stat[rindex->dir][rindex->reason] = 0;
    }
    return 0;
}

/* "reason_stat" attribute "write" callback */
static int cpu_attr_reason_stat_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;

    if (!rindex || *(bdmf_index *)rindex == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;

    if (cpu_data->index != rdpa_cpu_host)
        return BDMF_ERR_NOT_SUPPORTED;

    cpu_data->reason_stat[rindex->dir][rindex->reason] = 0;

    return 0;
}

static int cpu_attr_reason_stat_external_cb_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t *ptr = *(uint8_t **)val;

    cpu_data->reason_stat_external_cb = (reason_stat_extern_callback_t)ptr;

    return 0;
}

static void cpu_rxq_stat_read_add_from_rdd(cpu_drv_priv_t *cpu_data,
    bdmf_index index)
{
    uint16_t drop_counter = 0;
    rdpa_cpu_rx_stat_t *stat = &cpu_data->rxq_stat[index];
    uint8_t rdd_ring_idx = cpu_rdd_rxq_idx_get(cpu_data, index);

#ifndef RUNNER_CPU_DQM_RX
    stat->queued = rdp_cpu_ring_get_queued(rdd_ring_idx);
#else
    /* XXX: Read from DQM */
#endif
    rdd_cpu_rx_queue_discard_get(rdd_ring_idx, &drop_counter);
#if defined(XRDP)
    if (drv_cntr_get_cntr_non_accumulative())
        stat->dropped = 0;
#endif
    stat->dropped += drop_counter;
}

/* "rxq_stat" attribute "read" callback */
static int cpu_attr_rxq_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_rx_stat_t *stat = (rdpa_cpu_rx_stat_t *)val;

    if (_check_queue_range(cpu_data, (unsigned)index))
        return BDMF_ERR_PARM;

    /* Read and clear */
    cpu_rxq_stat_read_add_from_rdd(cpu_data, index);

    if (cpu_data->rxq_cfg[index].rxq_stat)
    {
        extern_rxq_stat_t priv = {};

        cpu_data->rxq_cfg[index].rxq_stat(index, &priv, 1);
        cpu_data->rxq_stat[index].received = priv.received;
        cpu_data->rxq_stat[index].dropped += priv.dropped;
        cpu_data->rxq_stat[index].queued += priv.queued;
    }
#if defined(XRDP)
    rdpa_common_update_cntr_results_uint32(stat, &(cpu_data->accumulative_rxq_stat[index]),
        offsetof(rdpa_cpu_rx_stat_t, received), cpu_data->rxq_stat[index].received);
    rdpa_common_update_cntr_results_uint32(stat, &(cpu_data->accumulative_rxq_stat[index]),
        offsetof(rdpa_cpu_rx_stat_t, dropped), cpu_data->rxq_stat[index].dropped);

    cpu_data->accumulative_rxq_stat[index].queued = cpu_data->rxq_stat[index].queued; /*no need accumulation of queued packets*/
    stat->queued = cpu_data->accumulative_rxq_stat[index].queued; /*no need accumulation of queued packets*/
    rdpa_common_update_cntr_results_uint32(stat, &(cpu_data->accumulative_rxq_stat[index]),
        offsetof(rdpa_cpu_rx_stat_t, interrupts), cpu_data->rxq_stat[index].interrupts);
#else
    *stat = cpu_data->rxq_stat[index];
#endif
    memset(&cpu_data->rxq_stat[index], 0, sizeof(*stat));

    return 0;
}

/* "rxq_stat" attribute "write" callback */
static int cpu_attr_rxq_stat_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_rx_stat_t *stat = (rdpa_cpu_rx_stat_t *)val;

    if (_check_queue_range(cpu_data, (unsigned)index))
        return BDMF_ERR_PARM;

    memset(&(cpu_data->accumulative_rxq_stat[index]), 0, sizeof(rdpa_cpu_rx_stat_t));
    memset(&cpu_data->rxq_stat[index], 0, sizeof(*stat));

    return 0;
}

#if defined(CPU_TX_SPEED_TEST)
int _size = 1024;
#endif
/* "send_packet" attribute "write" callback */
static int cpu_attr_send_packet_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc = 0;
#ifndef RUNNER_CPU_DQM_TX
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_dbg_tx_info_t *dbg_tx_info = (rdpa_cpu_dbg_tx_info_t *)index;
    rdpa_cpu_tx_info_t tx_info = {};
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();

    if (!dbg_tx_info || (*(bdmf_index *)dbg_tx_info == BDMF_INDEX_UNASSIGNED) ||
        !val || !size || (size > system_cfg->mtu_size - 4))
    {
        return BDMF_ERR_PARM;
    }

    /* Map tx info */
    tx_info.method = dbg_tx_info->method;
    tx_info.port = dbg_tx_info->port;
    tx_info.ssid = dbg_tx_info->ssid;
    tx_info.cpu_port = cpu_data->index;
    if (tx_info.method == rdpa_cpu_tx_port)
    {
        tx_info.x.wan.queue_id = dbg_tx_info->queue_id;
    }
    /* used for upstream dst wan flow, or bridged src wan_flow, otherwise
     * ignored */
    tx_info.x.wan.flow = dbg_tx_info->wan_flow;
    tx_info.drop_precedence = dbg_tx_info->drop_precedence;

    /* Transmit */
    if (dbg_tx_info->send_as == rdpa_cpu_dbg_send_raw)
    {
        tx_info.data_offset = RDD_PACKET_HEADROOM_OFFSET;
        rc = rdpa_cpu_send_raw((void *)val, size, &tx_info);
    }
    else
    {
#if defined(CPU_TX_SPEED_TEST) /* for speed test only */
        struct sk_buff *skb;
        
        skb = dev_alloc_skb(_size + RDD_PACKET_HEADROOM_OFFSET);

        bdmf_trace("%s:%d sending size %d (%p)\n", __FUNCTION__, __LINE__, _size, &_size);
        if (!skb)
            return BDMF_ERR_NOMEM;
        skb_put(skb, _size);
#else
        struct sk_buff *skb = dev_alloc_skb(size + RDD_PACKET_HEADROOM_OFFSET);

        if (!skb)
            return BDMF_ERR_NOMEM;
        skb_put(skb, size);
#endif
        skb_reserve(skb, RDD_PACKET_HEADROOM_OFFSET);
        memcpy(skb->data, val, size);
        rc = rdpa_cpu_send_sysb((bdmf_sysb)skb, &tx_info);
    }
#endif
    return rc;
}


/* "read_packet" attribute "write" callback */
static int cpu_attr_read_packet_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#ifndef RUNNER_CPU_DQM_RX
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t num_packets = *(uint32_t *)val;
    bdmf_boolean dump;
    int i;

    if (cpu_data->rxq_cfg[index].type != rdpa_ring_data)
        return BDMF_ERR_NOT_SUPPORTED;

    dump = cpu_data->rxq_cfg[index].dump;
    cpu_data->rxq_cfg[index].dump = 1;
    for (i = 0; i < num_packets; i++)
    {
        rdpa_cpu_rx_info_t info = {};
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
        rdpa_cpu_rx_ext_info_t ext_info = {};
        /* Always use rdpa_cpu_packet_get_redirected() here because it is a superset
         * of rdpa_cpu_packet_get()
         */
        if (rdpa_cpu_packet_get_redirected(cpu_data->index, index, &info, &ext_info))
            break;
#else
        if (rdpa_cpu_packet_get(cpu_data->index, index, &info))
            break;
#endif
        BUG_ON(info.data == NULL);
        bdmf_sysb_databuf_free((void *)info.data, 0);
    }
    rdpa_cpu_ring_read_idx_sync(cpu_data->index, index);
    cpu_data->rxq_cfg[index].dump = dump;
    BDMF_TRACE_INFO("%d packets read from queue %ld\n", i, index);
#endif
    return 0;
}

/* "tx_stat" attribute "read" callback */
static int cpu_attr_tx_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_cpu_tx_stat_t *stat = (rdpa_cpu_tx_stat_t *)val;

    *stat = cpu_data->tx_stat;

    /* Clear after read */
    memset(&cpu_data->tx_stat, 0, sizeof(cpu_data->tx_stat));

    return 0;
}

/* Object attribute descriptors */
static struct bdmf_attr cpu_attrs[] = {
    { .name = "index", .help = "CPU interface", .type = bdmf_attr_enum,
        .size = (sizeof(rdpa_cpu_port)), .ts.enum_table = &cpu_port_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .offset = offsetof(cpu_drv_priv_t, index)
    },
    { .name = "num_queues", .help = "Number of receive queues",
        .type = bdmf_attr_number, .size = (sizeof(uint32_t)),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .offset = offsetof(cpu_drv_priv_t, num_queues),
        .max_val = RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ
    },
    { .name = "rxq_cfg", .help = "Receive queue configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "cpu_rxq_cfg",
        .array_size = RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .offset = offsetof(cpu_drv_priv_t, rxq_cfg),
        .read = cpu_attr_rxq_cfg_read, .write = cpu_attr_rxq_cfg_write
    },
    { .name = "rxq_flush", .help = "Flush receive queue",
        .type = bdmf_attr_boolean, .array_size = RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = cpu_attr_rxq_flush_write
    },
    { .name = "rxq_stat", .help = "Rx statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "cpu_rx_stat",
        .array_size = RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK | BDMF_ATTR_WRITE,
        .read = cpu_attr_rxq_stat_read, .write = cpu_attr_rxq_stat_write
    },
    { .name = "meter_cfg", .help = "CPU meter configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "cpu_meter_cfg",
        .array_size = RDPA_CPU_MAX_METERS * 2,
        .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "rdpa_dir_index",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = cpu_attr_meter_cfg_read, .write = cpu_attr_meter_cfg_write,
        .get_next = cpu_attr_meter_get_next
    },
    { .name = "meter_stat", .help = "CPU meter drop counter",
        .type = bdmf_attr_number, .size = sizeof(uint32_t),
        .array_size = RDPA_CPU_MAX_METERS * 2,
        .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "rdpa_dir_index",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_WRITE,
        .read = cpu_attr_meter_stat_read, .get_next = cpu_attr_meter_get_next,
        .write = cpu_attr_meter_stat_write
    },
    { .name = "reason_cfg", .help = "Trap reason configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "cpu_reason_cfg",
        .array_size = rdpa_cpu_reason__num_of * 2,
        .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "cpu_reason_index",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = cpu_attr_reason_cfg_read, .write = cpu_attr_reason_cfg_write,
        .get_next = cpu_attr_reason_entry_get_next,
        .val_to_s = cpu_attr_reason_cfg_val_to_s
    },
    { .name = "reason_stat", .help = "Per trap reason statistics",
        .type = bdmf_attr_number, .array_size = rdpa_cpu_reason__num_of * 2,
        .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "cpu_reason_index", .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_WRITE,
        .read = cpu_attr_reason_stat_read, .get_next = cpu_attr_reason_get_next,
        .write = cpu_attr_reason_stat_write
    },
    { .name = "reason_stat_external_cb", .help = "Reason statistics external callback",
        .type = bdmf_attr_pointer, .size = sizeof(void *),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(cpu_drv_priv_t, reason_stat_external_cb),
        .write = cpu_attr_reason_stat_external_cb_write
    },
    { .name = "int_connect", .help = "Connect interrupts",
        .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .write = cpu_attr_int_connect_write
    },
    { .name = "int_enabled", .help = "Enable/disable interrupts",
        .type = bdmf_attr_boolean, .array_size = RDPA_CPU_MAX_QUEUES,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_NO_AUTO_GEN,
        .read = cpu_attr_int_enabled_read, .write = cpu_attr_int_enabled_write
    },
    { .name = "send_packet", .help = "Send packet (debug interface)",
        .type = bdmf_attr_buffer, .array_size = 1,
        .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "cpu_tx_info", .size = 256,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_NO_AUTO_GEN,
        .write = cpu_attr_send_packet_write
    },
    { .name = "read_packet", .help = "Read packet(s) (debug interface)",
        .type = bdmf_attr_number,
        .array_size = RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ,
        .size = (sizeof(uint32_t)),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_NO_AUTO_GEN,
        .write = cpu_attr_read_packet_write
    },
    { .name = "tx_dump", .help = "Enable/disable dumping transmit packets",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "cpu_tx_dump",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_NO_AUTO_GEN,
        .offset = offsetof(cpu_drv_priv_t, tx_dump)
    },
    { .name = "tx_stat", .help = "Transmit statistics (debug)",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "cpu_tx_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_NO_AUTO_GEN,
        .read = cpu_attr_tx_stat_read
    },
    { .name = "tc_to_rxq", .help = "TC to CPU RX queue mapping",
        .type = bdmf_attr_number, .size = sizeof(uint8_t),
        .array_size = RDPA_CPU_TC_NUM,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = cpu_attr_tc_to_rxq_read_ex, .write = cpu_attr_tc_to_rxq_write_ex
    },

    BDMF_ATTR_LAST
};

static int cpu_drv_init(struct bdmf_type *drv);
static void cpu_drv_exit(struct bdmf_type *drv);

struct bdmf_type cpu_drv = {
    .name = "cpu",
    .parent = "system",
    .description = "CPU Interface",
    .drv_init = cpu_drv_init,
    .drv_exit = cpu_drv_exit,
    .pre_init = cpu_pre_init,
    .post_init = cpu_post_init,
    .destroy = cpu_destroy,
    .extra_size = sizeof(cpu_drv_priv_t),
    .aattr = cpu_attrs,
    .max_objs = rdpa_cpu_port__num_of,
};
DECLARE_BDMF_TYPE(rdpa_cpu, cpu_drv);


/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get cpu object by key
 * \param[in] _index_    Object key
 * \param[out] cpu_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_cpu_get(rdpa_cpu_port _index_, bdmf_object_handle *cpu_obj)
{
    if (!cpu_object[_index_] || cpu_object[_index_]->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(cpu_object[_index_]);
    *cpu_obj = cpu_object[_index_];
    return 0;
}

/***************************************************************************
 * Additional manually-written functions
 **************************************************************************/

void *rdpa_cpu_data_get(int rdpa_cpu_type)
{
    bdmf_object_handle mo = cpu_object[rdpa_cpu_type];
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);

    return (void *)cpu_data;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_data_get);
#endif

/* Get the Time Of Day from the FW FIFO, by the ptp index */
int rdpa_cpu_ptp_1588_get_tod(uint16_t ptp_index, uint32_t *tod_h,
    uint32_t *tod_l, uint16_t *local_counter_delta)
{
#ifdef CONFIG_BCM_PTP_1588
#ifndef XRDP /* TBD */
    return rdd_1588_master_rx_get_entry(ptp_index, tod_h, tod_l, local_counter_delta);
#else
    return 0;
#endif
#else
    BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "ptp 1588 is not defined\n");
#endif /* CONFIG_BCM_PTP_1588 */
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_ptp_1588_get_tod);
#endif

/*
 * Internal helpers
 */
/* Get statistics swithout clearing them */
int _rdpa_cpu_stat_get(rdpa_cpu_port index, rdpa_stat_tx_rx_t *stat)
{
    bdmf_object_handle cpu_obj = cpu_object[index];
    cpu_drv_priv_t *cpu_data;
    int i;
    uint8_t first_index, last_index;

    if (!cpu_obj)
        return BDMF_ERR_NOENT;

    memset(stat, 0, sizeof(*stat));
    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_obj);

    cpu_rxq_cfg_indecies_get(cpu_data, &first_index, &last_index);
    for (i = first_index; i <= last_index; i++)
    {
        uint8_t rdd_ring_idx = cpu_rdd_rxq_idx_get(cpu_data, i);

        cpu_rxq_stat_read_add_from_rdd(cpu_data, i);
        if (cpu_data->rxq_cfg[i].rxq_stat)
        {
            extern_rxq_stat_t priv = {};

            cpu_data->rxq_cfg[i].rxq_stat(rdd_ring_idx, &priv, 0);
            cpu_data->rxq_stat[i].received = priv.received;
            cpu_data->rxq_stat[i].dropped += priv.dropped;
            cpu_data->rxq_stat[i].queued += priv.queued;
        }

        stat->rx.passed.packets += cpu_data->rxq_stat[i].received;
        stat->rx.discarded.packets += cpu_data->rxq_stat[i].dropped;
    }
    stat->tx.passed.packets = cpu_data->tx_stat.tx_ok;
    stat->tx.discarded.packets = cpu_data->tx_stat.tx_invalid_queue +
        cpu_data->tx_stat.tx_no_buf + cpu_data->tx_stat.tx_too_long +
        cpu_data->tx_stat.tx_rdd_error;
    return 0;
}


#if !defined(BDMF_DRIVER_GPL_LAYER) && !defined(RUNNER_CPU_DQM_RX)
EXPORT_SYMBOL(rdpa_cpu_packet_get);
#endif
#if !defined(BDMF_DRIVER_GPL_LAYER) && !defined(RUNNER_CPU_DQM_RX)
EXPORT_SYMBOL(rdpa_cpu_packet_get_redirected);
#endif
#if !defined(BDMF_DRIVER_GPL_LAYER)
EXPORT_SYMBOL(rdpa_cpu_loopback_packet_get);
#endif

#ifndef RUNNER_CPU_DQM_RX
int rdpa_cpu_queue_not_empty(rdpa_cpu_port port, bdmf_index queue)
{
    bdmf_object_handle mo = cpu_object[port];
    cpu_drv_priv_t *cpu_data;
    rdpa_cpu_rxq_cfg_t *rxq_cfg;

    if ((unsigned)port >= rdpa_cpu_port__num_of || !mo)
        return BDMF_ERR_PARM;

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);

    if (_check_queue_range(cpu_data, (unsigned)queue))
        return BDMF_ERR_RANGE;
    rxq_cfg = &cpu_data->rxq_cfg[queue];

    return rxq_cfg->size ? rdp_cpu_ring_not_empty(cpu_rdd_rxq_idx_get(cpu_data, queue)) : 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_queue_not_empty);
#endif
#endif

#ifndef RUNNER_CPU_DQM_RX
int rdpa_cpu_queue_is_full(rdpa_cpu_port port, bdmf_index queue)
{
    bdmf_object_handle mo = cpu_object[port];
    cpu_drv_priv_t *cpu_data;
    rdpa_cpu_rxq_cfg_t *rxq_cfg;

    if ((unsigned) port	>= rdpa_cpu_port__num_of || !mo)
        return 0;/*BDMF_ERR_PARM;*/

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);

    if (_check_queue_range(cpu_data, (unsigned)queue))
        return /*BDMF_ERR_RANGE;*/ 0;
    rxq_cfg = &cpu_data->rxq_cfg[queue];

    return rxq_cfg->size ? rdp_cpu_ring_is_full(queue) : 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_queue_is_full);
#endif
#endif

int rdpa_cpu_is_per_port_metering_supported(rdpa_cpu_reason reason)
{
    return cpu_is_per_port_metering_supported(reason);
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_is_per_port_metering_supported);
#endif

int cpu_meter_is_configured(rdpa_traffic_dir dir, int meter_idx)
{
    if ((unsigned)meter_idx >= cpu_max_meters_per_dir[dir])
        return 0;

    return (meter[dir][meter_idx].sir != 0);
}

/* Init/exit module. Cater for GPL layer */
#ifdef BDMF_DRIVER_GPL_LAYER
extern void (*f_rdpa_cpu_int_enable)(rdpa_cpu_port port, int queue);
extern void (*f_rdpa_cpu_int_disable)(rdpa_cpu_port port, int queue);
extern void (*f_rdpa_cpu_int_clear)(rdpa_cpu_port port, int queue);
extern void (*f_rdpa_rnr_int_enable)(uint8_t intr_idx);
extern void (*f_rdpa_rnr_int_disable)(uint8_t intr_idx);
extern void (*f_rdpa_rnr_int_clear)(uint8_t intr_idx);
extern int (*f_rdpa_cpu_packet_get)(rdpa_cpu_port port, bdmf_index queue,
                                    rdpa_cpu_rx_info_t *info);
extern int (*f_rdpa_cpu_packet_get_redirected)(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, rdpa_cpu_rx_ext_info_t *ext_info);
extern int (*f_rdpa_cpu_packets_bulk_get)(rdpa_cpu_port port, bdmf_index queue,
                                    rdpa_cpu_rx_info_t *info, int max_count, int *count);
extern int (*f_rdpa_cpu_loopback_packet_get)(rdpa_cpu_loopback_type loopback_type,
                                             bdmf_index queue, bdmf_sysb *sysb,
                                             rdpa_cpu_rx_info_t *info);
extern int (*f_rdpa_cpu_ptp_1588_get_tod)(uint16_t ptp_index, uint32_t *tod_h,
    uint32_t *tod_l, uint16_t *local_counter_delta);
extern int (*f_rdpa_cpu_send_sysb_ptp)(bdmf_sysb sysb,
    const rdpa_cpu_tx_info_t *info);
extern int (*f_rdpa_cpu_send_sysb)(bdmf_sysb sysb,
    const rdpa_cpu_tx_info_t *info);
extern int (*f_rdpa_cpu_send_sysb_fpm)(bdmf_sysb sysb,
    const rdpa_cpu_tx_info_t *info);
extern int (*f_rdpa_cpu_send_wfd_to_bridge)(bdmf_sysb sysb,
    const rdpa_cpu_tx_info_t *info, size_t offset_next);
extern int (*f_rdpa_cpu_wfd_bulk_fkb_get)(bdmf_index queue_id,
    unsigned int budget, void **rx_pkts, void *wfd_acc_info_p);
extern int (*f_rdpa_cpu_wfd_bulk_skb_get)(bdmf_index queue_id,
    unsigned int budget, void **rx_pkts, void *wfd_acc_info_p);
extern void *(*f_rdpa_cpu_data_get)(int rdpa_cpu_type);

extern int (*f_rdpa_cpu_send_raw)(void *data, uint32_t length,
    const rdpa_cpu_tx_info_t *info);
extern int (*f_rdpa_cpu_queue_not_empty)(rdpa_cpu_port port,
    bdmf_index queue);
extern int (*f_rdpa_cpu_queue_is_full)(rdpa_cpu_port port,
    bdmf_index queue);
extern int (*f_rdpa_cpu_send_epon_dying_gasp)(bdmf_sysb sysb,
    const rdpa_cpu_tx_info_t *info);
extern int (*f_rdpa_cpu_is_per_port_metering_supported)(rdpa_cpu_reason reason);
extern rdpa_ports (*f_rdpa_ports_all_lan)(void);
extern void (*f_rdpa_cpu_rx_dump_packet)(char *name, rdpa_cpu_port port,
    bdmf_index queue, rdpa_cpu_rx_info_t *info, uint32_t dst_ssid);
#endif

static int cpu_drv_init(struct bdmf_type *drv)
{
    rdpa_traffic_dir dir;
    int i;

    for (dir = 0; dir < 2; dir++)
    {
        for (i = 0; i < rdpa_cpu_reason__num_of; i++)
            reason_cfg[dir][i].meter = BDMF_INDEX_UNASSIGNED;
    }

#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_cpu_drv = rdpa_cpu_drv;
    f_rdpa_cpu_get = rdpa_cpu_get;
    f_rdpa_cpu_int_enable = rdpa_cpu_int_enable;
    f_rdpa_cpu_int_disable = rdpa_cpu_int_disable;
    f_rdpa_cpu_int_clear = rdpa_cpu_int_clear;
    f_rdpa_rnr_int_enable = rdpa_rnr_int_enable;
    f_rdpa_rnr_int_disable = rdpa_rnr_int_disable;
    f_rdpa_rnr_int_clear = rdpa_rnr_int_clear;
    f_rdpa_cpu_packet_get = rdpa_cpu_packet_get;
    f_rdpa_cpu_packet_get_redirected = rdpa_cpu_packet_get_redirected;
    f_rdpa_cpu_packets_bulk_get = rdpa_cpu_packets_bulk_get;
    f_rdpa_cpu_loopback_packet_get = rdpa_cpu_loopback_packet_get;
    f_rdpa_cpu_ptp_1588_get_tod = rdpa_cpu_ptp_1588_get_tod;
    f_rdpa_cpu_send_sysb_ptp = rdpa_cpu_send_sysb_ptp;
    f_rdpa_cpu_send_wfd_to_bridge = rdpa_cpu_send_wfd_to_bridge;
    f_rdpa_cpu_send_sysb = rdpa_cpu_send_sysb;
    f_rdpa_cpu_send_sysb_fpm = rdpa_cpu_send_sysb_fpm;
    f_rdpa_cpu_wfd_bulk_fkb_get = rdpa_cpu_wfd_bulk_fkb_get;
    f_rdpa_cpu_wfd_bulk_skb_get = rdpa_cpu_wfd_bulk_skb_get;
    f_rdpa_cpu_data_get = rdpa_cpu_data_get;
    f_rdpa_cpu_send_raw = rdpa_cpu_send_raw;
    f_rdpa_cpu_queue_not_empty = rdpa_cpu_queue_not_empty;
    f_rdpa_cpu_queue_is_full = rdpa_cpu_queue_is_full;
    f_rdpa_cpu_send_epon_dying_gasp = rdpa_cpu_send_epon_dying_gasp;
    f_rdpa_cpu_is_per_port_metering_supported =
        rdpa_cpu_is_per_port_metering_supported;
    f_rdpa_ports_all_lan = rdpa_ports_all_lan;
    f_rdpa_cpu_rx_dump_packet = _dump_packet;
#endif
    return cpu_drv_init_ex(drv);
}

static void cpu_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_cpu_drv = NULL;
    f_rdpa_cpu_get = NULL;
    f_rdpa_cpu_int_enable = NULL;
    f_rdpa_cpu_int_disable = NULL;
    f_rdpa_cpu_int_clear = NULL;
    f_rdpa_cpu_packet_get = NULL;
    f_rdpa_cpu_packet_get_redirected = NULL;
    f_rdpa_cpu_packets_bulk_get = NULL;
    f_rdpa_cpu_loopback_packet_get = NULL;
    f_rdpa_cpu_ptp_1588_get_tod = NULL;
    f_rdpa_cpu_send_sysb_ptp = NULL;
    f_rdpa_cpu_send_wfd_to_bridge = NULL;
    f_rdpa_cpu_send_sysb = NULL;
    f_rdpa_cpu_send_sysb_fpm = NULL;
    f_rdpa_cpu_wfd_bulk_fkb_get = NULL;
    f_rdpa_cpu_wfd_bulk_skb_get = NULL;
    f_rdpa_cpu_data_get = NULL;
    f_rdpa_cpu_send_raw = NULL;
    f_rdpa_cpu_queue_not_empty = NULL;
    f_rdpa_cpu_queue_is_full = NULL;
    f_rdpa_cpu_send_epon_dying_gasp = NULL;
    f_rdpa_cpu_is_per_port_metering_supported = NULL;
    f_rdpa_ports_all_lan = NULL;
    f_rdpa_cpu_rx_dump_packet = NULL;
#endif
    cpu_drv_exit_ex(drv);
}


