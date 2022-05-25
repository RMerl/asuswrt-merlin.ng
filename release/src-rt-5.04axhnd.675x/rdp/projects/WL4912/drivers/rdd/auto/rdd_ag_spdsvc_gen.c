/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"

#include "rdd_ag_spdsvc_gen.h"

int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_get(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat)
{
    if(!udpspdt_stream_tx_stat || _entry >= RDD_UDPSPDT_STREAM_TX_STAT_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_1_READ_G(udpspdt_stream_tx_stat->tx_packets_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_0_READ_G(udpspdt_stream_tx_stat->tx_packets_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_DROPS_NO_SPBM_READ_G(udpspdt_stream_tx_stat->tx_drops_no_spbm, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_READ_G(udpspdt_stream_tx_stat->first_ts, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_1_READ_G(udpspdt_stream_tx_stat->last_ts_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_0_READ_G(udpspdt_stream_tx_stat->last_ts_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_SEC_READ_G(udpspdt_stream_tx_stat->iperf3_ts_sec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_USEC_READ_G(udpspdt_stream_tx_stat->iperf3_ts_usec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_BAD_PROTO_CNTR_READ_G(udpspdt_stream_tx_stat->bad_proto_cntr, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TIME_SLICE_EXPIRE_READ_G(udpspdt_stream_tx_stat->time_slice_expire, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_DROPS_NO_SBPM_TIMER_STOP_READ_G(udpspdt_stream_tx_stat->tx_drops_no_sbpm_timer_stop, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_SET_READ_G(udpspdt_stream_tx_stat->first_ts_set, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_set(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat)
{
    if(!udpspdt_stream_tx_stat || _entry >= RDD_UDPSPDT_STREAM_TX_STAT_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_1_WRITE_G(udpspdt_stream_tx_stat->tx_packets_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_0_WRITE_G(udpspdt_stream_tx_stat->tx_packets_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_DROPS_NO_SPBM_WRITE_G(udpspdt_stream_tx_stat->tx_drops_no_spbm, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_WRITE_G(udpspdt_stream_tx_stat->first_ts, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_1_WRITE_G(udpspdt_stream_tx_stat->last_ts_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_0_WRITE_G(udpspdt_stream_tx_stat->last_ts_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_SEC_WRITE_G(udpspdt_stream_tx_stat->iperf3_ts_sec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_USEC_WRITE_G(udpspdt_stream_tx_stat->iperf3_ts_usec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_BAD_PROTO_CNTR_WRITE_G(udpspdt_stream_tx_stat->bad_proto_cntr, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TIME_SLICE_EXPIRE_WRITE_G(udpspdt_stream_tx_stat->time_slice_expire, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_DROPS_NO_SBPM_TIMER_STOP_WRITE_G(udpspdt_stream_tx_stat->tx_drops_no_sbpm_timer_stop, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_SET_WRITE_G(udpspdt_stream_tx_stat->first_ts_set, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_tx_params_get(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params)
{
    if(!udpspdt_tx_params || _entry >= RDD_UDPSPDT_TX_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_TX_PARAMS_TOTAL_NUM_OF_PKTS_READ_G(udpspdt_tx_params->total_num_of_pkts, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_PACKET_SIZE_READ_G(udpspdt_tx_params->packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_CURR_BUCKET_READ_G(udpspdt_tx_params->curr_bucket, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_BUCKET_BUDGET_READ_G(udpspdt_tx_params->bucket_budget, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_MAX_BUCKET_SIZE_READ_G(udpspdt_tx_params->max_bucket_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_tx_params_set(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params)
{
    if(!udpspdt_tx_params || _entry >= RDD_UDPSPDT_TX_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_TX_PARAMS_TOTAL_NUM_OF_PKTS_WRITE_G(udpspdt_tx_params->total_num_of_pkts, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_PACKET_SIZE_WRITE_G(udpspdt_tx_params->packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_CURR_BUCKET_WRITE_G(udpspdt_tx_params->curr_bucket, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_BUCKET_BUDGET_WRITE_G(udpspdt_tx_params->bucket_budget, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_MAX_BUCKET_SIZE_WRITE_G(udpspdt_tx_params->max_bucket_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

