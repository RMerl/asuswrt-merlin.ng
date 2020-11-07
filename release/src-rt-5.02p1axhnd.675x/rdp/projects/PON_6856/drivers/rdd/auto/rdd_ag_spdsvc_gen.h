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


#ifndef _RDD_AG_SPDSVC_GEN_H_
#define _RDD_AG_SPDSVC_GEN_H_

#include "rdd.h"

int rdd_ag_spdsvc_gen_udpspdtest_stream_tx_stat_table_set(uint32_t tx_packets_0, uint32_t tx_packets_1, uint32_t tx_drops_no_spbm, uint32_t first_ts, uint32_t last_ts_0, uint32_t last_ts_1, uint32_t iperf3_ts_sec, uint32_t iperf3_ts_usec, bdmf_boolean first_ts_set);
int rdd_ag_spdsvc_gen_udpspdtest_stream_tx_stat_table_get(uint32_t *tx_packets_0, uint32_t *tx_packets_1, uint32_t *tx_drops_no_spbm, uint32_t *first_ts, uint32_t *last_ts_0, uint32_t *last_ts_1, uint32_t *iperf3_ts_sec, uint32_t *iperf3_ts_usec, bdmf_boolean *first_ts_set);

#endif /* _RDD_AG_SPDSVC_GEN_H_ */
