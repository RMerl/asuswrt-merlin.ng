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

#ifndef _RDPA_CPU_EX_H_
#define _RDPA_CPU_EX_H_

#include "rdpa_cpu_basic.h"
#include "rdp_cpu_ring.h"

#ifdef XRDP
rdpa_if rdpa_port_vport_to_rdpa_if(rdd_vport_id_t rdd_vport);
#endif

/** Max number of CPU interface meters per direction */
#ifndef XRDP
#define RDPA_CPU_MAX_DS_METERS   4      /**< Number of meters for packets received from WAN port(s) */
#define RDPA_CPU_MAX_US_METERS   16     /**< Number of meters for packets received from LAN port(s) */
#else
#define RDPA_CPU_MAX_DS_METERS   RDD_DS_CPU_RX_METER_TABLE_SIZE     /**< Number of meters for packets received from WAN port(s) */
#define RDPA_CPU_MAX_US_METERS   RDD_US_CPU_RX_METER_TABLE_SIZE     /**< Number of meters for packets received from LAN port(s) */
#endif

#if RDPA_CPU_MAX_DS_METERS > RDPA_CPU_MAX_US_METERS
#define RDPA_CPU_MAX_METERS RDPA_CPU_MAX_DS_METERS
#else
#define RDPA_CPU_MAX_METERS RDPA_CPU_MAX_US_METERS
#endif

/* Number of reasons supporting per-port metering */
#define RDPA_CPU_PER_PORT_REASON 3

/* tx_debug_mode - structure underlying tx_debug attribute */
typedef struct
{
    bdmf_boolean enable;    /* Turn debug on */
} cpu_tx_dump_mode_t;

#ifdef XRDP
#define RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ RDPA_CPU_MAX_QUEUES
#else
#define RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ RING_ID_NUM_OF
#endif

/* host object private data */
typedef struct {
    /* rx data */
    rdpa_cpu_port index;
    int cpu_id;     /* CPU id for SMP */
    uint32_t num_queues;
    /* Actual number of rings is less (RDPA_CPU_MAX_NUM_OF_QUEUES_PER_OBJ), but we store RING_ID_NUM_OF to simplify
     * access to ring and recycle rings */
    rdpa_cpu_rxq_cfg_t rxq_cfg[RING_ID_NUM_OF];
    rdpa_cpu_rx_stat_t rxq_stat[RING_ID_NUM_OF];
    rdpa_cpu_rx_stat_t accumulative_rxq_stat[RING_ID_NUM_OF];
    bdmf_boolean       rxq_allocated_by_rdp[RING_ID_NUM_OF];
    uint32_t reason_stat[2][rdpa_cpu_reason__num_of];
    reason_stat_extern_callback_t reason_stat_external_cb; /* Reason statistics external callback - optional */

    rdpa_cpu_tx_stat_t tx_stat;
    cpu_tx_dump_mode_t tx_dump;
    bdmf_object_handle port_obj;
    bdmf_object_handle sched_obj;

    /* parameters learned from system configuration - stored here for
     * optimization */
    uint32_t headroom_size;

    /* TC to RXQ mapping */
    uint8_t tc_to_rxq[RDPA_CPU_TC_NUM];

    /* RDD rxq mapping */
    uint8_t rxq_to_rdd_rxq[RING_ID_NUM_OF];
} cpu_drv_priv_t;

#define CPU_DUMP_TX_DATA(type, ptr, len, info) \
    do {\
        bdmf_session_print(NULL, \
            "Tx " type " : method:%s port:%s queue_id:%u flow:%d -> %d bytes\n",\
            bdmf_attr_get_enum_text_hlp(&cpu_tx_method_enum_table, info->method),\
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->port),\
            (int)info->x.wan.queue_id, (int)info->x.wan.flow, (int)len);\
        bdmf_session_hexdump(NULL, ptr, 0, len);\
    } while (0)


#define CPU_CHECK_DUMP_PBUF(pbuf, info) \
    do { \
        if (unlikely(cpu_object_data->tx_dump.enable)) \
            CPU_DUMP_TX_DATA("pbuf", pbuf->data, pbuf->length, info); \
    } while (0)

#define CPU_CHECK_DUMP_SYSB(sysb, info) \
    do { \
        if (unlikely(cpu_object_data->tx_dump.enable)) \
        { \
            CPU_DUMP_TX_DATA("sysb", bdmf_sysb_data(sysb), \
                bdmf_sysb_length(sysb), info); \
        } \
    } while (0)

#define CPU_CHECK_DUMP_RDD_RC(type, rdd_rc) \
    do {\
        if (cpu_object_data->tx_dump.enable) \
        { \
            bdmf_session_print(NULL, "Tx " type " completed. rdd_rc = %d\n", \
                rdd_rc);\
        } \
    } while (0)

int cpu_post_init_ex(struct bdmf_object *mo);
void cpu_destroy_ex(struct bdmf_object *mo);
rdpa_ports rdpa_ports_all_lan(void);

int cpu_drv_init_ex(struct bdmf_type *drv);
void cpu_drv_exit_ex(struct bdmf_type *drv);
void _dump_packet(char *name, rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info, uint32_t dst_ssid);

int cpu_attr_tc_to_rxq_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
int cpu_attr_tc_to_rxq_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);

/* Interrupts */
int cpu_attr_int_connect_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);
int cpu_attr_int_enabled_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);
int cpu_attr_int_enabled_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);
int cpu_attr_rxq_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);

int rdpa_cpu_int_connect_ex(cpu_drv_priv_t *cpu_data, int queue_id, uint32_t affinity_mask);
void rdpa_cpu_int_disconnect_ex(cpu_drv_priv_t *cpu_data, int queue_id);

int _check_queue_range(cpu_drv_priv_t *cpu_data, uint32_t queue_number);

void rdpa_cpu_ring_read_idx_sync(rdpa_cpu_port port, bdmf_index queue);

static inline rdpa_if rdpa_rdd_rx_srcport_to_rdpa_if(uint16_t rdd_srcport, int flow_id)
{
#ifndef XRDP
#ifndef G9991
    /* Special case for wifi packets: if src_port is PCI then need to set
     * SSID */
    return (rdd_srcport == BL_LILAC_RDD_PCI_BRIDGE_PORT) ?
        rdpa_if_ssid0 + flow_id : rdpa_rdd_bridge_port_to_if(rdd_srcport,
        RDPA_WIFI_SSID_INVALID);
#else
    switch (rdd_srcport)
    {
    case 0:
        return rdpa_if_wan0; /* FIXME: MULTI-WAN XPON */
    /* .. upto number-of-lan-ifs + 1 */
    case 1 ... rdpa_if_lan_max - rdpa_if_lan0 + 1 + 1:
        return rdpa_if_lan0 + rdd_srcport - 1;
    default:
        return rdpa_if_none;
    }
#endif
#else /* XRDP */
    return rdpa_port_vport_to_rdpa_if(rdd_srcport);
#endif
}

int cpu_is_per_port_metering_supported(rdpa_cpu_reason reason);
int cpu_reason_cfg_validate_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int cpu_reason_cfg_rdd_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
int cpu_meter_cfg_rdd(struct bdmf_object *mo, const rdpa_cpu_reason_index_t *rindex, int meter, rdpa_ports ports);
int cpu_meter_is_configured(rdpa_traffic_dir dir, int meter);
int cpu_per_port_reason_meter_cfg(struct bdmf_object *mo, const rdpa_cpu_reason_index_t *rindex,
    const rdpa_cpu_reason_cfg_t *reason_cfg);

int rdpa_cpu_loopback_packet_get(rdpa_cpu_loopback_type loopback_type, bdmf_index queue, bdmf_sysb *sysb,
    rdpa_cpu_rx_info_t *info);

int rdpa_cpu_packet_get(rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info);
int rdpa_cpu_packets_bulk_get(rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info, int max_count, int *count);

void cpu_rxq_cfg_params_init_ex(cpu_drv_priv_t *cpu_data, rdpa_cpu_rxq_cfg_t *rx_cfg, uint32_t *entry_size,
    uint32_t *init_write_idx);
void cpu_rxq_cfg_indecies_get(cpu_drv_priv_t *cpu_data, uint8_t *first_rxq_idx, uint8_t *last_rxq_idx);
int cpu_rxq_cfg_max_num_set(cpu_drv_priv_t *cpu_data);
uint8_t cpu_rdd_rxq_idx_get(cpu_drv_priv_t *cpu_data, bdmf_index rxq_idx);
int cpu_rxq_cfg_size_validate_ex(cpu_drv_priv_t *cpu_data, rdpa_cpu_rxq_cfg_t *rxq_cfg);
void rdpa_cpu_rx_meter_clean_stat_ex(bdmf_index counter_id);

#endif

