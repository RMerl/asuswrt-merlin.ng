/*
* <:copyright-BRCM:2018:proprietary:standard
*
*    Copyright (c) 2018 Broadcom
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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_udpspdtest_ex.h"
#include "rdpa_spdtest_common_ex.h"

struct bdmf_object *udpspdtest_object;
static int flows_obj_created_here;
struct bdmf_object *flows_obj; /* ip_class for PON, ucast for DSL */

static int udpspdtest_pre_init(struct bdmf_object *mo)
{
    udpspdtest_drv_priv_t *spdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);

    spdtest->cfg.proto = rdpa_udpspdtest_proto_basic;
    spdtest->cfg.so_mark = RDPA_UDPSPDTEST_SO_MARK_BASIC;
    return udpspdtest_pre_init_ex(mo);
}

static int udpspdtest_post_init(struct bdmf_object *mo)
{
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    int i, rc;

    flows_obj_created_here = 0;
    rc = flow_object_get(&flows_obj, &flows_obj_created_here);
    if (rc)
    {
        BDMF_TRACE_RET(rc, "Failed to retrieve flows object handle\n");
        return rc;
    }

    for (i = 0; i < UDPSPTD_MAX_NUM_OF_STREAMS; i++)
    {
        udpspdtest->rx_params[i].us_flow_index = BDMF_INDEX_UNASSIGNED;
        udpspdtest->rx_params[i].ds_flow_index = BDMF_INDEX_UNASSIGNED;
    }

    udpspdtest_object = mo;
    snprintf(mo->name, sizeof(mo->name), "udpspdtest");
    return 0;
}

static void udpspdtest_destroy(struct bdmf_object *mo)
{
    udpspdtest_destroy_ex(mo);
    if (flows_obj)
    {
        if (flows_obj_created_here)
            bdmf_destroy(flows_obj);
        else
            bdmf_put(flows_obj);
    }

    flows_obj = NULL;
    udpspdtest_object = NULL;
}

static int udpspdtest_get(struct bdmf_type *drv, struct bdmf_object *owner, const char *discr, struct bdmf_object **pmo)
{
    if (!udpspdtest_object)
        return BDMF_ERR_NOENT;
    *pmo = udpspdtest_object;
    return 0;
}

const bdmf_attr_enum_table_t rdpa_udpspdtest_proto_table =
{
    .type_name = "rdpa_udpspdtest_proto_t", .help = "UDP Speed Test protocol",
    .values = {
        {"basic", rdpa_udpspdtest_proto_basic},
        {"iperf3", rdpa_udpspdtest_proto_iperf3},
        {NULL, 0}
    }
};

/*  Engine reference packet header aggregate type */
struct bdmf_aggr_type udpspdtest_cfg_type = {
    .name = "udpspdtest_cfg", .struct_name = "rdpa_udpspdtest_cfg_t",
    .help = "UDP Speed Test Configuration",
    .fields = (struct bdmf_attr[])
    {
        { .name = "proto",
            .help = "UDP Speed Test protocol", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_udpspdtest_proto_table,
            .size = sizeof(rdpa_udpspdtest_proto_t),
            .offset = offsetof(rdpa_udpspdtest_cfg_t, proto)
        },
        { .name = "num_of_streams", .help = "Number of currently configured streams",
            .type = bdmf_attr_number, .size = sizeof(uint8_t),
            .offset = offsetof(rdpa_udpspdtest_cfg_t, num_of_streams),
            .flags = BDMF_ATTR_READ
        },
        { .name = "so_mark", .help = "Socket Mark",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_udpspdtest_cfg_t, so_mark),
            .flags = BDMF_ATTR_READ | BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(udpspdtest_cfg_type);

/* Write new configuration for UDP Speed Test */
static int udpspdtest_attr_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    udpspdtest_drv_priv_t *spdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_udpspdtest_cfg_t *cfg = (rdpa_udpspdtest_cfg_t *)val;
    int rc;

    /* Don't allow to add new configuration if number of streams != 0. */
    if (spdtest->cfg.num_of_streams)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot change configuration unless all streams are released "
            "(number of active streams %d)\n", spdtest->cfg.num_of_streams);
    }
    rc = udpspdtest_attr_cfg_write_ex(spdtest, cfg);
    if (rc)
        return rc;

    /* RDD configuration completed, store in object context */
    spdtest->cfg.proto = cfg->proto;
    
    /* XXX: Change to switch or mapping table when more protocols will be added */
    if (cfg->proto == rdpa_udpspdtest_proto_basic)
        spdtest->cfg.so_mark = RDPA_UDPSPDTEST_SO_MARK_BASIC;
    else if (cfg->proto == rdpa_udpspdtest_proto_iperf3)
        spdtest->cfg.so_mark = RDPA_UDPSPDTEST_SO_MARK_IPERF3;

    return 0;
}

struct bdmf_aggr_type udpspdtest_basic_stat_type = {
    .name = "udpsdptest_basic_stat", .struct_name = "rdpa_udpspdtest_basic_stat_t",
    .help = "UDP Speed Test Basic Statistics",
    .fields = (struct bdmf_attr[])
    {
        { .name = "running", .help = "Running Flag", .size = sizeof(bdmf_boolean),
            .type = bdmf_attr_boolean, .offset = offsetof(rdpa_udpspdtest_basic_stat_t, running)
        },
        { .name = "rx_packets", .help = "Packets received by the Analyzer", .size = sizeof(uint64_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_basic_stat_t, rx_packets)
        },
        { .name = "rx_bytes", .help = "Bytes received by the Analyzer", .size = sizeof(uint64_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_basic_stat_t, rx_bytes)
        },
        { .name = "rx_time_usec", .help = "Receive Time in microseconds", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_basic_stat_t, rx_time_usec)
        },
        { .name = "tx_packets", .help = "Packets transmitted by the Generator", .size = sizeof(uint64_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_basic_stat_t, tx_packets)
        },
        { .name = "tx_discards", .help = "Packets discarded by the Generator", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_basic_stat_t, tx_discards)
        },
        { .name = "tx_time_usec", .help = "Transmit Time in microseconds", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_basic_stat_t, tx_time_usec)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(udpspdtest_basic_stat_type);


struct bdmf_aggr_type iperf3_ext_result_type = {
    .name = "iperf3_ext_result", .struct_name = "rdpa_udpspdtest_iperf3_ext_stat_t",
    .help = "Iperf3 Extention Test Result",
    .fields = (struct bdmf_attr[])
    {
        { .name = "out_of_order_pkts", .help = "Out of Order Packets detected by the Analyzer",
            .size = sizeof(uint64_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_iperf3_ext_stat_t, out_of_order_pkts)
        },
        { .name = "error_cnt_pkts", .help = "Error counte packet", .size = sizeof(uint64_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_iperf3_ext_stat_t, error_cnt_pkts)
        },
        { .name = "jitter", .help = "Jitter", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_iperf3_ext_stat_t, jitter)
        },
        { .name = "tx_time_sec", .help = "Transmit Time: seconds part", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_iperf3_ext_stat_t, tx_time_sec)
        },
        { .name = "tx_time_usec", .help = "Transmit Time: microseconds part", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_udpspdtest_iperf3_ext_stat_t, tx_time_usec)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(iperf3_ext_result_type);


/* tm_queue_cfg aggregate type : queue configuration */
struct bdmf_aggr_type udpspdtest_stat_type = {
    .name = "udpspdtest_stat", .struct_name = "rdpa_udpspdtest_stat_t",
    .help = "UDP Speed Test stream statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "basic_stat", .help = "Basic UDP Speed Test statistics",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "udpsdptest_basic_stat",
            .offset = offsetof(rdpa_udpspdtest_stat_t, basic)
        },
        { .name = "iperf3_ext", .help = "Basic UDP Speed Test statistics",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "iperf3_ext_result",
            .offset = offsetof(rdpa_udpspdtest_stat_t, iperf3_ext)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(udpspdtest_stat_type);

/*  spdsvc_analyzer aggregate type */
struct bdmf_aggr_type udpspdtest_rx_params_type = {
    .name = "udpspdtest_rx_params", .struct_name = "rdpa_udpspdtest_rx_params_t",
    .help = "UDP Speed Test RX Parameters",
    .fields = (struct bdmf_attr[])
    {
        { .name = "local_ip_addr", .help = "Local IP Address",
          .type = bdmf_attr_ip_addr, .size = sizeof(bdmf_ip_t),
          .offset = offsetof(rdpa_udpspdtest_rx_params_t, local_ip_addr),
          .flags = BDMF_ATTR_MANDATORY
        },
        { .name = "local_port_nbr", .help = "Local UDP Port Number",
          .type = bdmf_attr_number, .size = sizeof(uint16_t),
          .offset = offsetof(rdpa_udpspdtest_rx_params_t, local_port_nbr),
          .flags = BDMF_ATTR_MANDATORY | BDMF_ATTR_UNSIGNED
        },
        { .name = "remote_ip_addr", .help = "Remote IP Address",
          .type = bdmf_attr_ip_addr, .size = sizeof(bdmf_ip_t),
          .offset = offsetof(rdpa_udpspdtest_rx_params_t, remote_ip_addr),
          .flags = BDMF_ATTR_MANDATORY
        },
        { .name = "remote_port_nbr", .help = "Remote UDP Port Number",
          .type = bdmf_attr_number, .size = sizeof(uint16_t),
          .offset = offsetof(rdpa_udpspdtest_rx_params_t, remote_port_nbr),
          .flags = BDMF_ATTR_MANDATORY | BDMF_ATTR_UNSIGNED
        },
        { .name = "us_flow_index", .help = "Analyzer Flow Index",
          .type = bdmf_attr_number, .size = sizeof(bdmf_index),
          .offset = offsetof(rdpa_udpspdtest_rx_params_t, us_flow_index),
          .flags = BDMF_ATTR_READ
        },
        { .name = "ds_flow_index", .help = "Analyzer Flow Index",
          .type = bdmf_attr_number, .size = sizeof(bdmf_index),
          .offset = offsetof(rdpa_udpspdtest_rx_params_t, ds_flow_index),
          .flags = BDMF_ATTR_READ
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(udpspdtest_rx_params_type);

static void dump_rx_params(rdpa_udpspdtest_rx_params_t *rx_params)
{
    int is_v4;
    
    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    is_v4 = rx_params->remote_ip_addr.family == bdmf_ip_family_ipv4;

    bdmf_trace("RX Params, family: %s\n", is_v4 ? "ipv4" : "ipv6");
    if (is_v4)
    {
        uint32_t local_ipv4, remote_ipv4;

        remote_ipv4 = ntohl(rx_params->remote_ip_addr.addr.ipv4);
        local_ipv4 = ntohl(rx_params->local_ip_addr.addr.ipv4);

        bdmf_trace("Remote IP: %pI4, Remote Port: %d\n", &remote_ipv4, rx_params->remote_port_nbr);
        bdmf_trace("Local IP: %pI4, Local Port: %d\n", &local_ipv4, rx_params->local_port_nbr);
    }
    else
    {
        bdmf_trace("Remote IP: %pI6, Remote Port: %d\n", &rx_params->remote_ip_addr.addr.ipv6,
            rx_params->remote_port_nbr);
        bdmf_trace("Local IP: %pI6, Local Port: %d\n", &rx_params->local_ip_addr.addr.ipv6, rx_params->local_port_nbr);
    }
}

static int udpspdtest_attr_rx_params_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);

    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    memcpy(val, &udpspdtest->rx_params[index], sizeof(rdpa_udpspdtest_rx_params_t));
    return 0;
}

static int udpspdtest_attr_rx_params_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_udpspdtest_rx_params_t *rx_params = (rdpa_udpspdtest_rx_params_t *)val;
    int rc;

    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    dump_rx_params(rx_params);

    rc = udpspdtest_attr_rx_params_write_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    memcpy(&udpspdtest->rx_params[index], rx_params, sizeof(rdpa_udpspdtest_rx_params_t));
    return 0;
}

/*  spdsvc_generator aggregate type */
struct bdmf_aggr_type udpspdtest_tx_params_type = {
    .name = "udpspdtest_tx_params", .struct_name = "rdpa_udpspdtest_tx_params_t",
    .help = "UDP Speed Test TX Parameters",
    .fields = (struct bdmf_attr[])
    {
        { .name = "kbps", .help = "Transmit Rate (Kbps)",
          .type = bdmf_attr_number, .size = sizeof(uint32_t),
          .offset = offsetof(rdpa_udpspdtest_tx_params_t, kbps),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "mbs", .help = "Maximum Burst Size (bytes)",
          .type = bdmf_attr_number, .size = sizeof(uint32_t),
          .offset = offsetof(rdpa_udpspdtest_tx_params_t, mbs),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "total_packets_to_send", .help = "Total packets to send by the test (optional)",
          .type = bdmf_attr_number, .size = sizeof(uint64_t),
          .offset = offsetof(rdpa_udpspdtest_tx_params_t, total_packets_to_send),
          .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "iperf3_64bit_counters", .help = "Iperf3 configuration: use 64-bit counters in UDP test packets",
          .type = bdmf_attr_boolean, .size = sizeof(bdmf_attr_boolean),
          .offset = offsetof(rdpa_udpspdtest_tx_params_t, iperf3_64bit_counters)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(udpspdtest_tx_params_type);

static int udpspdtest_attr_tx_params_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);

    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    memcpy(val, &udpspdtest->tx_params[index], sizeof(rdpa_udpspdtest_tx_params_t));
    return 0;
}

/* Object attribute descriptors */
static struct bdmf_attr udpspdtest_attrs[] = {
    {
        .name = "cfg", .help = "UDP Speed Test configuration", /* Protocol:RW, num_of_pkts:R, so_mark:R */
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "udpspdtest_cfg",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(udpspdtest_drv_priv_t, cfg),
        .write = udpspdtest_attr_cfg_write
    },
    /* Super-set of all possible stats, invoked according to proto */
    { .name = "stream_stat", .help = "UDP Speed Test stream statistics", 
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "udpspdtest_stat",
        .array_size = UDPSPTD_MAX_NUM_OF_STREAMS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
        .read = udpspdtest_attr_flow_stat_read_ex
    },
    {
        .name = "rx_params", .help = "Stream RX parameters",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "udpspdtest_rx_params",
        .array_size = UDPSPTD_MAX_NUM_OF_STREAMS,
        .index_type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = udpspdtest_attr_rx_params_read,
        .write = udpspdtest_attr_rx_params_write
    },
    {
        .name = "rx_start", .help = "Start packets receive on a stream",
        .type = bdmf_attr_number, .size = sizeof(uint8_t),
        .min_val = 0, .max_val = UDPSPTD_MAX_NUM_OF_STREAMS - 1,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = udpspdtest_attr_rx_start_write_ex
    },
    {
        .name = "rx_stop", .help = "Stop packets receive on a steam",
        .type = bdmf_attr_number, .size = sizeof(uint8_t),
        .min_val = 0, .max_val = UDPSPTD_MAX_NUM_OF_STREAMS - 1,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = udpspdtest_attr_rx_stop_write_ex
    },
    {
        .name = "ref_pkt", .help = "UDP Speed Test TX reference packet",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "spdtest_ref_pkt",
        .array_size = PKTGEN_TX_NUM_OF_DATA_PKTS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG | BDMF_ATTR_WRITE,
        .read = udpspdtest_attr_engine_ref_pkt_read_ex,
        .write = udpspdtest_attr_engine_ref_pkt_write_ex
    },
    {
        .name = "tx_params", .help = "Stream TX parametes",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "udpspdtest_tx_params",
        .array_size = UDPSPTD_MAX_NUM_OF_STREAMS,
        .index_type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = udpspdtest_attr_tx_params_read,
        .write = udpspdtest_attr_tx_params_write_ex
    },
    {
        .name = "tx_start", .help = "Start packets transmission on a stream",
        .type = bdmf_attr_number, .size = sizeof(uint8_t), 
        .min_val = 0, .max_val = UDPSPTD_MAX_NUM_OF_STREAMS - 1,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = udpspdtest_attr_tx_start_write_ex
    },
    {
        .name = "tx_stop", .help = "Stop packets transmission on a steam",
        .type = bdmf_attr_number, .size = sizeof(uint8_t), 
        .min_val = 0, .max_val = UDPSPTD_MAX_NUM_OF_STREAMS - 1,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = udpspdtest_attr_tx_stop_write_ex
    },
    BDMF_ATTR_LAST
};

static int udpspdtest_drv_init(struct bdmf_type *drv);
static void udpspdtest_drv_exit(struct bdmf_type *drv);

struct bdmf_type udpspdtest_drv = {
    .name = "udpspdtest",
    .parent = "system",
    .description = "Runner UDP Speed Test Manager",
    .drv_init = udpspdtest_drv_init,
    .drv_exit = udpspdtest_drv_exit,
    .pre_init = udpspdtest_pre_init,
    .post_init = udpspdtest_post_init,
    .destroy = udpspdtest_destroy,
    .get = udpspdtest_get,
    .extra_size = sizeof(udpspdtest_drv_priv_t),
    .aattr = udpspdtest_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_udpspdtest, udpspdtest_drv);

/* Init/exit module. Cater for GPL layer */
static int udpspdtest_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_udpspdtest_drv = rdpa_udpspdtest_drv;
    f_rdpa_udpspdtest_get = rdpa_udpspdtest_get;
#endif
    return 0;
}

static void udpspdtest_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_udpspdtest_drv = NULL;
    f_rdpa_udpspdtest_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get udpspdtest object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_udpspdtest_get(bdmf_object_handle *_obj_)
{
    if (!udpspdtest_object || udpspdtest_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(udpspdtest_object);
    *_obj_ = udpspdtest_object;
    return 0;
}

