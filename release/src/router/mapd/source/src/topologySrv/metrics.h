/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  metrics query/response
 *
 *  Abstract:
 *  metrics query/response
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the metrics query/response
 * */

#ifndef METRICS_H
#define METRICS_H
#include "interface.h"

struct GNU_PACKED bss_db {
        unsigned char bssid[ETH_ALEN];
        SLIST_ENTRY(bss_db) bss_entry;
};

#if defined(MAP_R2) || defined(CENT_STR)
struct GNU_PACKED radio_identifier {
	unsigned char identifier[ETH_ALEN];
};
#endif
struct GNU_PACKED mrsp_db {
        unsigned char bssid[ETH_ALEN];
        unsigned char ch_util;
        unsigned short assoc_sta_cnt;
        SLIST_ENTRY(mrsp_db) mrsp_entry;
        unsigned char esp_cnt;
        SLIST_HEAD(list_head_esp, esp_db) esp_head;
};

struct traffic_stats_db {
        unsigned char identifier[ETH_ALEN];
        SLIST_ENTRY(traffic_stats_db) traffic_stats_entry;
        unsigned char sta_cnt;
        SLIST_HEAD(list_head_stats, stats_db) stats_head;
};
#ifdef MAP_R2
struct sta_ext_info_db {
	u32 last_data_ul_rate;
	u32 last_data_dl_rate;
	u32 utilization_rx;
	u32 utilization_tx;
};
#endif
struct metrics_db {
        unsigned char mac[ETH_ALEN];
        unsigned char bssid[ETH_ALEN];
        unsigned int time_delta;
        unsigned int erate_downlink;
        unsigned int erate_uplink;
        unsigned char rssi_uplink;
#ifdef MAP_R2
	struct sta_ext_info_db sta_ext_info;
#endif
        SLIST_ENTRY(metrics_db) metrics_entry;
};      

struct GNU_PACKED link_metrics_db {
        unsigned char identifier[ETH_ALEN];
        SLIST_ENTRY(link_metrics_db) link_metrics_entry;
        unsigned char sta_cnt;
        SLIST_HEAD(list_head_metrics, metrics_db) metrics_head;
};

struct GNU_PACKED unlink_metrics_db {
        unsigned char mac[ETH_ALEN];
        unsigned char ch;
        unsigned int time_delta;
        unsigned char uplink_rssi;
        SLIST_ENTRY(unlink_metrics_db) unlink_metrics_entry;
};

struct unlink_metrics_info {
        unsigned char oper_class;
        unsigned char sta_num;
        SLIST_HEAD(list_head_unlink_metrics, unlink_metrics_db) unlink_metrics_head;
};

struct metrics_info {
        unsigned char metrics_query_cnt;
        SLIST_HEAD(list_head_metrics_query, bss_db) metrics_query_head;
        unsigned char metrics_rsp_cnt;
        SLIST_HEAD(list_head_metrics_rsp, mrsp_db) metrics_rsp_head;
        SLIST_HEAD(list_head_traffic_stats, traffic_stats_db) traffic_stats_head;
        unsigned char assoc_sta[ETH_ALEN];
        struct metrics_db assoc_sta_link_metrics;
        struct unlink_metrics_query *unlink_query;
        struct unlink_metrics_info unlink_info;
        struct beacon_metrics_query *bcn_query;
        struct beacon_metrics_rsp *bcn_rsp;
	struct backhaul_link_info *bh;
#if defined(MAP_R2) || defined(CENT_STR)
	struct radio_identifier radio_id[MAX_NUM_OF_RADIO];
	u16 total_radio_band;
#endif
};
#endif
