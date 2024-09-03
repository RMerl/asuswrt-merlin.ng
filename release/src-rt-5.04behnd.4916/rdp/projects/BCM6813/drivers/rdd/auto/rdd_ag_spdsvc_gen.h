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


#ifndef _RDD_AG_SPDSVC_GEN_H_
#define _RDD_AG_SPDSVC_GEN_H_

typedef struct rdd_udpspdt_stream_tx_stat_s
{
    uint32_t tx_packets_1;
    uint32_t tx_packets_0;
    uint32_t tx_no_sbpm;
    uint32_t first_ts;
    uint32_t last_ts_1;
    uint32_t last_ts_0;
    uint32_t iperf3_ts_sec;
    uint32_t iperf3_ts_usec;
    uint16_t bad_proto_cntr;
    uint16_t time_slice_expire;
    uint16_t tx_drops_no_sbpm_timer_stop;
    bdmf_boolean first_ts_set;
} rdd_udpspdt_stream_tx_stat_t;

typedef struct rdd_udpspdt_tx_params_s
{
    uint32_t total_num_of_pkts;
    uint16_t packet_size;
    uint16_t last_packet_size;
    uint32_t curr_bucket;
    uint32_t bucket_budget;
    uint32_t last_packet_start_idx;
    uint32_t max_bucket_size;
} rdd_udpspdt_tx_params_t;

int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_get(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat);
int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_set(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat);
int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_get_core(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat, int core_id);
int rdd_ag_spdsvc_gen_udpspdt_stream_tx_stat_set_core(uint32_t _entry, rdd_udpspdt_stream_tx_stat_t *udpspdt_stream_tx_stat, int core_id);
int rdd_ag_spdsvc_gen_udpspdt_tx_params_get(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params);
int rdd_ag_spdsvc_gen_udpspdt_tx_params_set(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params);
int rdd_ag_spdsvc_gen_udpspdt_tx_params_get_core(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params, int core_id);
int rdd_ag_spdsvc_gen_udpspdt_tx_params_set_core(uint32_t _entry, rdd_udpspdt_tx_params_t *udpspdt_tx_params, int core_id);

#endif /* _RDD_AG_SPDSVC_GEN_H_ */
