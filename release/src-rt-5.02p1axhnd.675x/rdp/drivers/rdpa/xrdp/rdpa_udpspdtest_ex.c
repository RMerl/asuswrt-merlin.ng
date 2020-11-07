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
#include "rdpa_udpspdtest.h"
#include "rdpa_udpspdtest_ex.h"
#include "rdpa_spdtest_common_ex.h"
#include "rdd_spdsvc.h"
#include "rdd_ag_cpu_rx.h"
#include "rdd_ag_spdsvc_gen.h"
#include "rdd_ag_pktgen_tx.h"
#include "rdd_tcpspdtest.h"
#include "xrdp_drv_rnr_regs_ag.h"
#include "rdpa_port_int.h"

extern struct bdmf_object *flows_obj;
extern struct bdmf_object *udpspdtest_object;

int udpspdtest_pre_init_ex(struct bdmf_object *mo)
{
    udpspdtest_drv_priv_t *spdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    
    spdt_tx_sbpm_bn_reset(&spdtest->ref_pkt);
    return 0;
}

void udpspdtest_destroy_ex(struct bdmf_object *mo)
{
    udpspdtest_drv_priv_t *spdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);

    spdt_tx_ref_pkts_free(&spdtest->ref_pkt);
}

int udpspdtest_attr_cfg_write_ex(udpspdtest_drv_priv_t *spdtest, rdpa_udpspdtest_cfg_t *cfg)
{
    switch (cfg->proto)
    {
    case rdpa_udpspdtest_proto_basic:
        RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_UDP_BASIC, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
        break;
    case rdpa_udpspdtest_proto_iperf3:
        RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_UDP_IPERF3, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
        break;
    default:
        BDMF_TRACE_ERR("Unknown UDP Protoco (%d), resetting", (int)cfg->proto);
        RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(SPDTEST_NONE, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
        break;
    }

    return 0;
}

static int is_stream_running(bdmf_index index)
{
    uint8_t is_on;

    /* XXX: Add support for multi-stream */
    RDD_SPDTEST_GEN_CFG_IS_ON_READ_G(is_on, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    return is_on;
}

int udpspdtest_attr_rx_params_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    rdpa_udpspdtest_rx_params_t *rx_params = (rdpa_udpspdtest_rx_params_t *)val;
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    int rc;

    if (is_stream_running(index))
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot set/reset rx parameters for a running stream (idx = %d)\n",
            (int)index);
    }

    /* Delete existing flows. Can be invoked in two siturations: either we got empty RX params struct, or we 
     * got a new one, so we should clean up previously configured flows. For empty struct we allow either NULL or empty
     * local_ip_addr (as NULL pointer cannot be forwarded from the user-space auto-generated APIs) */
    if (udpspdtest->rx_params[index].us_flow_index != BDMF_INDEX_UNASSIGNED || !rx_params ||
        bdmf_ip_is_zero(&rx_params->local_ip_addr))
    {
        spdt_analyzer_flow_delete(flows_obj, udpspdtest->rx_params[index].us_flow_index);
    }
    if (udpspdtest->rx_params[index].ds_flow_index != BDMF_INDEX_UNASSIGNED || !rx_params ||
        bdmf_ip_is_zero(&rx_params->local_ip_addr))
    {
        spdt_analyzer_flow_delete(flows_obj, udpspdtest->rx_params[index].ds_flow_index);
    }

    if (!rx_params || bdmf_ip_is_zero(&rx_params->local_ip_addr))
        return 0;

    /* Add flows; since we have no indication if this is US or DS flow, add both */
    rc = spdt_analyzer_flow_add(flows_obj, rdpa_dir_us, (rdpa_spdsvc_analyzer_t *)rx_params);
    if (rc)
    {
        BDMF_TRACE_RET(rc, "Failed to create US flow for UDP Speed Test\n");
    }
    rc = spdt_analyzer_flow_add(flows_obj, rdpa_dir_ds, (rdpa_spdsvc_analyzer_t *)rx_params);
    if (rc)
    {
        spdt_analyzer_flow_delete(flows_obj, rx_params->us_flow_index);
        BDMF_TRACE_RET(rc, "Failed to create DS flow for UDP Speed Test\n");
    }

    return 0;
}

int udpspdtest_attr_rx_start_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    /* Reset statistics and tell speed service that can start count */
    rdd_ag_cpu_rx_udpspdtest_stream_rx_stat_table_set(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0);
    RDD_SPDTEST_GEN_CFG_NOT_VALID_LICENSE_WRITE_G(0, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(1, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    return 0;
}

int udpspdtest_attr_rx_stop_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(0, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    return 0;
}

int udpspdtest_attr_engine_ref_pkt_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    return 0;
}

static int __udpspdtest_engine_ref_pkt_set(udpspdtest_drv_priv_t *udpspdtest,
    rdpa_spdtest_ref_pkt_t *ref_pkt)
{
    int i, rc;
    uint16_t csum;

    /* XXX: Add support for multistream */

    /* XXX: Change pkt offset so UDP payload offset can be aligned to 8 */
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_UDP_PAYLOAD_OFFSET_WRITE_G(
        ref_pkt->udp.payload_offset + RDD_PACKET_HEADROOM_OFFSET, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, 0);

    csum = htons(*(uint16_t *)((char *)ref_pkt->data + ref_pkt->udp.payload_offset - 2));
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_UDP_CSUM_WRITE_G(csum, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, 0);

    /* Update reference packet header. XXX: Move to rdd_pktgen and rename */
    rdd_tcpspdtest_engine_ref_pkt_hdr_set(MIN(ref_pkt->size,
        RDD_SPDTEST_ENGINE_REF_PKT_HDR_HDR_NUMBER - RDD_PACKET_HEADROOM_OFFSET), RDD_PACKET_HEADROOM_OFFSET,
        ref_pkt->data, 0);

    for (i = 0; i < PKTGEN_TX_NUM_OF_DATA_PKTS; i++)
    {
        rc = spdt_tx_ref_pkt_set(&udpspdtest->ref_pkt, i, ref_pkt);
        if (rc)
        {
            if (i > 0)
                spdt_tx_ref_pkts_free(&udpspdtest->ref_pkt);
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
                "Failed to allocate SBPMs for reference packet header, rc = %d, i = %d\n", rc, i);
        }
    }
    return 0;
}

int udpspdtest_attr_engine_ref_pkt_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_spdtest_ref_pkt_t *ref_pkt = (rdpa_spdtest_ref_pkt_t *)val;

    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    return __udpspdtest_engine_ref_pkt_set(udpspdtest, ref_pkt);
}

int udpspdtest_attr_tx_params_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);

    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    if (is_stream_running(index))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Cannot change running stream %d settings\n", (int)index);

    memcpy(&udpspdtest->tx_params[index], val, sizeof(rdpa_udpspdtest_tx_params_t));
    if (!udpspdtest->tx_params[index].mbs)
        udpspdtest->tx_params[index].mbs = RDPA_UDPSPDTEST_DEF_MBS; 

    return 0;
}

int udpspdtest_tx_start(pbuf_t *pbuf, const rdpa_cpu_tx_info_t *info,
    RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx)
{
    /* XXX: Add support for multi-test, use so_mark to detect test object */
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(udpspdtest_object);
    rdpa_udpspdtest_tx_params_t *tx_params;
    uint32_t index, fpm_ug;
    int spdt_type;
    uint16_t wan_flow = 0, queue;

    spdt_type = spdt_so_mark_to_test_type(info->spdt_so_mark);
    if (spdt_type == SPDTEST_NONE)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Unsupported SO mark 0x%x\n", info->spdt_so_mark);

    /* XXX: Add support for multi-stream */
    index = info->spdt_so_mark & 0xf;
    if (index > 0)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Bad Stream ID %d\n", (int)index);

    if (is_stream_running(index))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Stream %d is already running\n", (int)index);

    /* Pktgen part */

    /* Initialize pktgen settings */
    spdt_tx_defs_init(SPDSVC_GEN_THREAD_NUMBER);
    if (rdpa_if_is_wan(info->port))
        fpm_ug = 1; /* Upstream */
    else
        fpm_ug = 0; /* Downstream */

    wan_flow = rdpa_port_rx_flow_src_port_get(rdpa_if_cpu0, 1);
    queue = cpu_tx->first_level_q;
    spdt_tx_fpm_ug_budget_set(fpm_ug);
    /* XXX: Temporary */
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_WAN_FLOW_WRITE_G(wan_flow, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, index);
    RDD_PKTGEN_TX_STREAM_ENTRY_ENTRY_PARMS_TX_QM_QUEUE_WRITE_G(queue, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, index);

    /* Speed Test params */

    /* As first implementaion, with single stream support, we re-use the speed service generator. */ 
    tx_params = &udpspdtest->tx_params[index];
    rdd_spdsvc_gen_params_init(tx_params->kbps, tx_params->mbs, udpspdtest->ref_pkt.pkt[0].size,
        tx_params->total_packets_to_send);

    RDD_SPDTEST_GEN_CFG_TEST_TYPE_WRITE_G(spdt_type, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    if (!tx_params->total_packets_to_send)
        RDD_SPDTEST_GEN_CFG_IS_ENDLESS_TX_WRITE_G(1, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    RDD_SPDTEST_GEN_CFG_IPERF3_64BIT_PKTID_WRITE_G(tx_params->iperf3_64bit_counters, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    /* Reset TX statistics */
    rdd_ag_spdsvc_gen_udpspdtest_stream_tx_stat_table_set(0, 0, 0, 0, 0, 0, 0, 0, 0);
    RDD_SPDTEST_GEN_CFG_NOT_VALID_LICENSE_WRITE_G(0, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(1, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    /* Kick speed service task */
    WMB();
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(spdsvc_gen_runner_image), SPDSVC_GEN_THREAD_NUMBER);

    return 0;
}

int udpspdtest_attr_tx_start_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    /* XXX: Todo - consider to add support later in order to test from CLI. At the first stage, start can
     * be triggered from the cpu_tx interface only */
    return BDMF_ERR_NOT_SUPPORTED;
}

int udpspdtest_attr_tx_stop_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    udpspdtest_drv_priv_t *spdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);

    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    RDD_SPDTEST_GEN_CFG_IS_ENDLESS_TX_WRITE_G(0, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(0, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);

    WMB();

    spdt_tx_ref_pkts_free(&spdtest->ref_pkt);

    return 0;
}

static uint32_t time_usec_calc(uint32_t ts_first, uint32_t ts_last_0, uint32_t ts_last_1)
{
    uint64_t ts_last;
    uint32_t time_usec;

    ts_last = ts_last_1;
    ts_last <<= 32;
    ts_last += ts_last_0;

    time_usec = (uint32_t)((ts_last - ts_first) / TIMER_TICKS_PER_USEC);
    return time_usec;
}

static void udp_basic_rx_flow_stat_get(rdpa_udpspdtest_stat_t *stat)
{
    uint32_t rx_bytes_0, rx_bytes_1, rx_packets_0, rx_packets_1;
    uint32_t ts_first, ts_last_0, ts_last_1;
    uint32_t rx_packet_lost_0, rx_packet_lost_1;
    uint32_t rx_out_of_order_0, rx_out_of_order_1;
    bdmf_boolean ts_first_set;
    uint16_t bad_proto_cntr;
 
    rdd_ag_cpu_rx_udpspdtest_stream_rx_stat_table_get(&rx_bytes_0, &rx_bytes_1, &rx_packets_0, &rx_packets_1,
        &ts_first, &ts_last_0, &ts_last_1, &ts_first_set, &bad_proto_cntr, &rx_packet_lost_0, &rx_packet_lost_1,
        &rx_out_of_order_0, &rx_out_of_order_1);

    stat->basic.rx_time_usec = time_usec_calc(ts_first, ts_last_0, ts_last_1);

    stat->basic.rx_bytes = (uint64_t)rx_bytes_1 << 32 | rx_bytes_0;
    stat->basic.rx_packets = (uint64_t)rx_packets_1 << 32 | rx_packets_0;
    stat->iperf3_ext.out_of_order_pkts = (uint64_t)rx_out_of_order_1 << 32 | rx_out_of_order_0;
    stat->iperf3_ext.error_cnt_pkts = (uint64_t)rx_packet_lost_1 << 32 | rx_packet_lost_0; 
}

static void udp_tx_flow_stat_get(rdpa_udpspdtest_stat_t *stat)
{
    uint32_t tx_packets_0, tx_packets_1;
    uint32_t tx_discards;
    uint32_t ts_first, ts_last_0, ts_last_1, iperf3_tx_sec, iperf3_tx_usec;
    bdmf_boolean ts_first_set;
 
    rdd_ag_spdsvc_gen_udpspdtest_stream_tx_stat_table_get(&tx_packets_0, &tx_packets_1,
        &tx_discards, &ts_first, &ts_last_0, &ts_last_1, &iperf3_tx_sec, &iperf3_tx_usec, &ts_first_set);

    /* XXX: Add support for throughput bytes count */
    
    stat->basic.tx_packets = tx_packets_1;
    stat->basic.tx_packets <<= 32;
    stat->basic.tx_packets |= tx_packets_0;
    stat->basic.tx_discards = tx_discards;
    
    stat->basic.tx_time_usec = time_usec_calc(ts_first, ts_last_0, ts_last_1);

    /* Iperf3 TX extention */
    stat->iperf3_ext.tx_time_sec = iperf3_tx_sec;
    stat->iperf3_ext.tx_time_usec = iperf3_tx_usec;
}

#ifdef __KERNEL__
#define DUMP_FMT "%llu"
#else
#define DUMP_FMT "%lu"
#endif

static void basic_flow_stat_dump(rdpa_udpspdtest_stat_t *stat, char *title)
{
    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("%s RX Stats:\n", title);
    bdmf_trace("\tRX Packets: " DUMP_FMT "\n", stat->basic.rx_packets);
    bdmf_trace("\tRX Bytes: " DUMP_FMT "\n", stat->basic.rx_bytes);
    bdmf_trace("\tRX time (usec) %u\n\n", stat->basic.rx_time_usec);

    bdmf_trace("%s TX Stats:\n", title);
    bdmf_trace("\tTX Packets: " DUMP_FMT "\n", stat->basic.tx_packets);
    bdmf_trace("\tTX Discards: " DUMP_FMT "\n", (uint64_t)stat->basic.tx_discards);
    bdmf_trace("=======================================\n");
}

int udpspdtest_attr_flow_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    udpspdtest_drv_priv_t *udpspdtest = (udpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_udpspdtest_stat_t *stat = (rdpa_udpspdtest_stat_t *)val; 
    rdpa_udpspdtest_stat_t new_stat = {}, *prev_stat;

    /* XXX: Add support for multi-stream */
    if (index > 0)
        return BDMF_ERR_RANGE;

    memset(stat, 0, sizeof(rdpa_udpspdtest_stat_t));

    stat->basic.running = is_stream_running(index);

    prev_stat = &udpspdtest->accumulative_stat;
    basic_flow_stat_dump(prev_stat, "Previous");

    udp_basic_rx_flow_stat_get(&new_stat);
    udp_tx_flow_stat_get(&new_stat);
    basic_flow_stat_dump(&new_stat, "Last accumulative");

    /* RX: Calc delta statistics */
    stat->basic.rx_time_usec = new_stat.basic.rx_time_usec - prev_stat->basic.rx_time_usec;
    stat->basic.rx_bytes = new_stat.basic.rx_bytes - prev_stat->basic.rx_bytes;
    stat->basic.rx_packets = new_stat.basic.rx_packets - prev_stat->basic.rx_packets;
    basic_flow_stat_dump(stat, "Clear on read (relative)");

    /* TX: Calc delta statistics */
    stat->basic.tx_packets = new_stat.basic.tx_packets - prev_stat->basic.tx_packets;
    stat->basic.tx_discards = new_stat.basic.tx_discards - prev_stat->basic.tx_discards;
    stat->basic.tx_time_usec = new_stat.basic.tx_time_usec - prev_stat->basic.tx_time_usec;

    /* Iperf3 extention */
    stat->iperf3_ext.tx_time_sec = new_stat.iperf3_ext.tx_time_sec;
    stat->iperf3_ext.tx_time_usec = new_stat.iperf3_ext.tx_time_usec;
    /* Iperf3 extention RX: Calc delta statistics */
    stat->iperf3_ext.error_cnt_pkts = new_stat.iperf3_ext.error_cnt_pkts - prev_stat->iperf3_ext.error_cnt_pkts; 
    stat->iperf3_ext.out_of_order_pkts = new_stat.iperf3_ext.out_of_order_pkts - prev_stat->iperf3_ext.out_of_order_pkts; 

    /* Update new accumulative stat for the next iteration */
    memcpy(&udpspdtest->accumulative_stat, &new_stat, sizeof(rdpa_udpspdtest_stat_t));

    return 0;
}

