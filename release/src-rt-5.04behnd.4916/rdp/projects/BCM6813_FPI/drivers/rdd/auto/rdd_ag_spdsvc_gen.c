/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
    RDD_UDPSPDT_STREAM_TX_STAT_TX_NO_SBPM_READ_G(udpspdt_stream_tx_stat->tx_no_sbpm, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
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
    RDD_UDPSPDT_STREAM_TX_STAT_TX_NO_SBPM_WRITE_G(udpspdt_stream_tx_stat->tx_no_sbpm, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry);
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

int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_get_core(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat, int core_id)
{
    if(!udpspdt_stream_tx_stat || _entry >= RDD_UDPSPDT_STREAM_TX_STAT_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_1_READ_CORE(udpspdt_stream_tx_stat->tx_packets_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_0_READ_CORE(udpspdt_stream_tx_stat->tx_packets_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_NO_SBPM_READ_CORE(udpspdt_stream_tx_stat->tx_no_sbpm, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_READ_CORE(udpspdt_stream_tx_stat->first_ts, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_1_READ_CORE(udpspdt_stream_tx_stat->last_ts_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_0_READ_CORE(udpspdt_stream_tx_stat->last_ts_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_SEC_READ_CORE(udpspdt_stream_tx_stat->iperf3_ts_sec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_USEC_READ_CORE(udpspdt_stream_tx_stat->iperf3_ts_usec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_BAD_PROTO_CNTR_READ_CORE(udpspdt_stream_tx_stat->bad_proto_cntr, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TIME_SLICE_EXPIRE_READ_CORE(udpspdt_stream_tx_stat->time_slice_expire, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_DROPS_NO_SBPM_TIMER_STOP_READ_CORE(udpspdt_stream_tx_stat->tx_drops_no_sbpm_timer_stop, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_SET_READ_CORE(udpspdt_stream_tx_stat->first_ts_set, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_set_core(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat, int core_id)
{
    if(!udpspdt_stream_tx_stat || _entry >= RDD_UDPSPDT_STREAM_TX_STAT_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_1_WRITE_CORE(udpspdt_stream_tx_stat->tx_packets_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_PACKETS_0_WRITE_CORE(udpspdt_stream_tx_stat->tx_packets_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_NO_SBPM_WRITE_CORE(udpspdt_stream_tx_stat->tx_no_sbpm, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_WRITE_CORE(udpspdt_stream_tx_stat->first_ts, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_1_WRITE_CORE(udpspdt_stream_tx_stat->last_ts_1, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_LAST_TS_0_WRITE_CORE(udpspdt_stream_tx_stat->last_ts_0, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_SEC_WRITE_CORE(udpspdt_stream_tx_stat->iperf3_ts_sec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_IPERF3_TS_USEC_WRITE_CORE(udpspdt_stream_tx_stat->iperf3_ts_usec, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_BAD_PROTO_CNTR_WRITE_CORE(udpspdt_stream_tx_stat->bad_proto_cntr, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TIME_SLICE_EXPIRE_WRITE_CORE(udpspdt_stream_tx_stat->time_slice_expire, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_TX_DROPS_NO_SBPM_TIMER_STOP_WRITE_CORE(udpspdt_stream_tx_stat->tx_drops_no_sbpm_timer_stop, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_STREAM_TX_STAT_FIRST_TS_SET_WRITE_CORE(udpspdt_stream_tx_stat->first_ts_set, RDD_UDPSPDT_STREAM_TX_STAT_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_tx_params_get(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params)
{
    if(!udpspdt_tx_params || _entry >= RDD_UDPSPDT_TX_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_TX_PARAMS_TOTAL_NUM_OF_PKTS_READ_G(udpspdt_tx_params->total_num_of_pkts, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_PACKET_SIZE_READ_G(udpspdt_tx_params->packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_SIZE_READ_G(udpspdt_tx_params->last_packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_CURR_BUCKET_READ_G(udpspdt_tx_params->curr_bucket, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_BUCKET_BUDGET_READ_G(udpspdt_tx_params->bucket_budget, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_START_IDX_READ_G(udpspdt_tx_params->last_packet_start_idx, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_MAX_BUCKET_SIZE_READ_G(udpspdt_tx_params->max_bucket_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_tx_params_set(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params)
{
    if(!udpspdt_tx_params || _entry >= RDD_UDPSPDT_TX_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_TX_PARAMS_TOTAL_NUM_OF_PKTS_WRITE_G(udpspdt_tx_params->total_num_of_pkts, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_PACKET_SIZE_WRITE_G(udpspdt_tx_params->packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_SIZE_WRITE_G(udpspdt_tx_params->last_packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_CURR_BUCKET_WRITE_G(udpspdt_tx_params->curr_bucket, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_BUCKET_BUDGET_WRITE_G(udpspdt_tx_params->bucket_budget, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_START_IDX_WRITE_G(udpspdt_tx_params->last_packet_start_idx, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);
    RDD_UDPSPDT_TX_PARAMS_MAX_BUCKET_SIZE_WRITE_G(udpspdt_tx_params->max_bucket_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_tx_params_get_core(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params, int core_id)
{
    if(!udpspdt_tx_params || _entry >= RDD_UDPSPDT_TX_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_TX_PARAMS_TOTAL_NUM_OF_PKTS_READ_CORE(udpspdt_tx_params->total_num_of_pkts, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_PACKET_SIZE_READ_CORE(udpspdt_tx_params->packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_SIZE_READ_CORE(udpspdt_tx_params->last_packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_CURR_BUCKET_READ_CORE(udpspdt_tx_params->curr_bucket, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_BUCKET_BUDGET_READ_CORE(udpspdt_tx_params->bucket_budget, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_START_IDX_READ_CORE(udpspdt_tx_params->last_packet_start_idx, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_MAX_BUCKET_SIZE_READ_CORE(udpspdt_tx_params->max_bucket_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_spdsvc_gen_udpspdt_tx_params_set_core(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params, int core_id)
{
    if(!udpspdt_tx_params || _entry >= RDD_UDPSPDT_TX_PARAMS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_UDPSPDT_TX_PARAMS_TOTAL_NUM_OF_PKTS_WRITE_CORE(udpspdt_tx_params->total_num_of_pkts, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_PACKET_SIZE_WRITE_CORE(udpspdt_tx_params->packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_SIZE_WRITE_CORE(udpspdt_tx_params->last_packet_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_CURR_BUCKET_WRITE_CORE(udpspdt_tx_params->curr_bucket, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_BUCKET_BUDGET_WRITE_CORE(udpspdt_tx_params->bucket_budget, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_LAST_PACKET_START_IDX_WRITE_CORE(udpspdt_tx_params->last_packet_start_idx, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UDPSPDT_TX_PARAMS_MAX_BUCKET_SIZE_WRITE_CORE(udpspdt_tx_params->max_bucket_size, RDD_UDPSPDT_TX_PARAMS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

