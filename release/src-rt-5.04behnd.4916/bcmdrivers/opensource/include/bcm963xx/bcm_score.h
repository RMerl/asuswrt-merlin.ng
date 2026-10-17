#ifndef __BCM_SCORE_H_INCLUDED__
#define __BCM_SCORE_H_INCLUDED__

/*
  Copyright (c) 2024 Broadcom Corporation
  All Rights Reserved

  <:label-BRCM:2024:DUAL/GPL:standard

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

/*
*******************************************************************************
* File Name  : bcm_score.h
*
* Description: This file contains the specification of some common definitions
*      and interfaces to other modules. This file may be included by both
*      Kernel and userapp (C only).
*
*******************************************************************************
*/

#include "bcm_score_defs.h"
#include <datapath_score_autogen.h>
#include "bcm_async_queue.h"

#define SCORE_DEV_NAME             "score"
#define SCORE_DRV_MAJOR            399
#define SCORE_DRV_NAME             SCORE_DEV_NAME
#define SCORE_DRV_DEVICE_NAME      "/dev/" SCORE_DEV_NAME


typedef enum {
    SCORE_IOCTL_SCORE_CTRL_INIT = 100,
    SCORE_IOCTL_SCORE_DCP_ADD,
    SCORE_IOCTL_SCORE_DCP_DELETE,
    SCORE_IOCTL_SCORE_MONITOR_START,
    SCORE_IOCTL_SCORE_MONITOR_STOP,
    SCORE_IOCTL_SCORE_QSTAT_REPORT_CFG,
    SCORE_IOCTL_SCORE_QSTAT_REPORT_DEL,
    SCORE_IOCTL_SCORE_QSTAT_RESULT,
} score_ioctl_t;

typedef enum {
    SCORE_MONITOR_START_NOTIF,
    SCORE_MONITOR_STOP_NOTIF,
} score_ctrl_t;

typedef enum {
    IF_INVALID,
    IF_WLAN_DHD,
    IF_WLAN_NIC,
    IF_WIRED,
} qstat_if_type_t;

typedef struct score_queue_info {
    uint32_t num_entries;                   /* IN (sdr) / OUT (ctrl) : allocated queue size in entries */
    uint32_t entry_size;                    /* OUT : size of a single entry */
    uint32_t alloc_size;                    /* OUT : total size of memory allocated for the queue */
} score_queue_info_t;

typedef struct score_cfg_data {
    score_queue_info_t              queue_info;
} score_cfg_data_t;

typedef struct score_dcp_cfg_data {
    score_dcp_common_cfg_data_t     info;
    score_dcp_data_collection_t     collect;
    score_queue_info_t              queue_info;
} score_dcp_cfg_data_t;

typedef struct score_dcp_del_data {
    uint32_t dcp_id;
} score_dcp_del_data_t;


typedef struct score_bin_cfg {
    uint32_t limits[QSTAT_MAX_BIN_LIMITS];
} bin_cfg_t;

typedef struct score_qstat_cfg_report {
    bin_cfg_t bin_cfg;
    /*if 0 - allocate new, change configuration otherwise*/
    uint8_t qstat_cfg_id;
} score_qstat_cfg_report_t;

typedef struct score_mon_start_data {
    score_stream_tuple_t stream_tuple; /*TODO evaluate using struct nf_conntrack_tuple */
    uint16_t ct_zone_id;
    /* TODO is namespaceid needed */
    uint8_t dcp_id;
    uint8_t notification_req;
    /*TODO add valid flags for data passed from user */
    union {
        score_bnl_stream_cfg_t bnl_cfg;
        uint32_t qstat_profile;
    } stream_data;
    union {
        void *ct; /*internal to score dirver*/
        uint64_t ct_64;
    };
    /*return data*/
    uint16_t score_stream_id; 
} score_mon_start_data_t;

typedef struct score_mon_stop_data {
    uint16_t score_stream_id;
    uint8_t dcp_id;
    uint8_t notification_req;
} score_mon_stop_data_t;

typedef struct qstat_streamkey_result {
    union {
        struct {
            qstat_telemetry_data_t pkt_latency;
            qstat_hist_t pkt_latency_hist;
        } wlan_stats;
        struct {
            qstat_counters_t transmitted;
            qstat_counters_t aqm_dropped;

            qstat_telemetry_data_t latency;
            qstat_hist_t latency_hist;
            qstat_counters_t cq_latency_remark;
            qstat_counters_t lq_latency_remark;
        } wired_stats;
    };
} qstat_streamkey_result_t;

typedef struct score_qstat_result {
    uint16_t score_stream_id;
    uint8_t dcp_id;
    uint8_t reset_on_read; /* reset results on read */
    /* return values */
    qstat_if_type_t dir_type[2];    /* per direction */
    qstat_streamkey_result_t dir_results[2]; /* per direction */
} score_qstat_result_t;

typedef union {
    score_cfg_data_t cfg;
    score_dcp_cfg_data_t dcp_cfg;
    score_dcp_del_data_t dcp_del;
    score_mon_start_data_t mon_start;
    score_mon_stop_data_t mon_stop;
    score_qstat_cfg_report_t qstat_cfg;
    uint8_t qstat_cfg_id;
    score_qstat_result_t qstat_result;
} score_ioctl_data_t;

typedef struct {
    uint32_t msg_type;
    uint32_t reserved;
    union {
        score_mon_start_data_t start;
        score_mon_stop_data_t  stop;
        uint64_t               msg_data;
    } __attribute__((aligned(8)));
} score_ctrl_msg_t;

typedef struct score_report_field_db {
    uint8_t valid;
    uint16_t offset;
    uint16_t size;
} score_report_field_db_t;

typedef enum {
    SCORE_REPORT_FIELD_FIRST,

    /* 32xN bit fields */
    SCORE_REPORT_HEADER = SCORE_REPORT_FIELD_FIRST,    /* must be first becaus it contains the lengths of the regions */
    SCORE_REPORT_FIELD_REGION_1,
    SCORE_REPORT_FIELD_REGION_2,
    SCORE_REPORT_FIELD_REGION_3,
    SCORE_REPORT_FIELD_TCP_FIELDS,

    /* 16xN bit fields */

    /* 8 bit fields */
    SCORE_REPORT_FIELD_TOS,
    SCORE_REPORT_FIELD_TTL,

    SCORE_REPORT_FIELD_LAST = SCORE_REPORT_FIELD_TTL,

    /* Keep these two last */
    SCORE_REPORT_INITIALIZED,   /* Used to indicate if the parser DB is initialized or not */
    SCORE_REPORT_FIELD_MAX,
} score_report_fields_t;

#define SCORE_REPORT_MAX_REGION_LENGTH  (64)
#define SCORE_REPORT_MAX_REGION_OFFSET  (110 /* PACKET_HEADER_LENGTH */ - 4 /* Minimum data region size */)

typedef struct score_report_parsed {
    uint8_t ttl;
    uint8_t tos;
    uint16_t length;
    uint32_t tcp_seq_num;
    uint16_t tcp_window_size;
    uint8_t tcp_flags;
    score_report_header_t header;
    uint8_t data1[SCORE_REPORT_MAX_REGION_LENGTH];
    uint8_t data2[SCORE_REPORT_MAX_REGION_LENGTH];
    uint8_t data3[SCORE_REPORT_MAX_REGION_LENGTH];
} score_report_parsed_t;

/* This structure is a superset of the raw report the Runner will generate if
 * it would include all possible fields.
 * When fields are left out, the structure that would actually be used is the
 * same one without those fields. To make this work the fields are ordered by
 * bit widths in descending order so that padding isn't required and the
 * structure is kept as is.
 * 
 * In order to use this table and avoid the parsing function:
 *   1) Create your own copy of the table.
 *   2) Remove irrelevant fields.
 *   3) Modify data1/data2/data3 lengths to match requested length if relevant.
 * 
 * Notes:
 *   1) data is big-endian
 *   2) The rest of the fields are in little endian
 * 
 * Internal note:
 *  This strucutre is a mirror of SCORE_REPORT_HEADER and the SCORE report
 *  that is built in data_collection()
 */
typedef struct __attribute__((packed)) score_report_raw {
    score_report_header_t header;
    uint8_t data1[SCORE_REPORT_MAX_REGION_LENGTH];  /* 4-byte aligned */
    uint8_t data2[SCORE_REPORT_MAX_REGION_LENGTH];  /* 4-byte aligned */
    uint8_t data3[SCORE_REPORT_MAX_REGION_LENGTH];  /* 4-byte aligned */
    score_report_tcp_fields_t tcp_fields;
    uint16_t length;    /* L2, L3, L4 or L5 packet length */
    uint8_t tos;
    uint8_t ttl;
} score_report_raw_t;

#endif  /* defined(__BCM_SCORE_H_INCLUDED__) */
