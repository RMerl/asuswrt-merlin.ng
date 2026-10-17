#ifndef __BCM_SCORE_DEFS_H_INCLUDED__
#define __BCM_SCORE_DEFS_H_INCLUDED__

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
* File Name  : bcm_score_defs.h
*
* Description: This file contains global score based definitions used
*              across modules. Shouldn't require any inclusion/dependency
* 
*******************************************************************************
*/

/* This file must not have dependency on any other file exept regular data-types (int,uint etc) */

#define SCORE_NUM_OF_STREAM_IDX     1024    /* stream_id = 0 is invalid */
#define SCORE_NUM_OF_INCARNS        8       /* max number of incarnations */
#define SCORE_NUM_OF_STREAM_IDS     (SCORE_NUM_OF_STREAM_IDX * SCORE_NUM_OF_INCARNS)
#define SCORE_NUM_OF_STREAM_KEYS    (SCORE_NUM_OF_STREAM_IDX * SCORE_NUM_OF_INCARNS * 2) /*per direction */

#define STREAM_IDX_MAX_BITS         (10) /*1024 */
#define STREAM_ID_MAX_ENTRIES       (1 << STREAM_IDX_MAX_BITS)

#if STREAM_ID_MAX_ENTRIES != SCORE_NUM_OF_STREAM_IDX
#error "STREAM_ID_MAX_ENTRIES != SCORE_NUM_OF_STREAM_IDX"
#endif

#define STREAM_ID_INCARN_BITS       (3)
#define STREAM_ID_MAX_BITS          (STREAM_IDX_MAX_BITS + STREAM_ID_INCARN_BITS)
#define STREAM_KEY_RESERVE_BITS     (16 - STREAM_ID_MAX_BITS - 1) /* direction+incaranation+idx */

#define SCORE_STREAM_ID_INVALID     0

#define SCORE_MAX_DCP_NUM           8
#define SCORE_DCP_ID_INVALID        0xFF

#define SCORE_MAX_SDRS              SCORE_MAX_DCP_NUM
#define SCORE_SDR_ID_INVALID        SCORE_DCP_ID_INVALID

#define QSTAT_MAX_BIN_NUMBER        16
#define QSTAT_MAX_BIN_LIMITS        (QSTAT_MAX_BIN_NUMBER - 1)

#define QSTAT_MAX_CFG_NUMBER        8

#define QSTAT_TYPE_WLAN_DHD_INFO    1
#define QSTAT_TYPE_WIRED_INFO       2
#define QSTAT_TYPE_WLAN_NIC_INFO    3

#define BNL_MAX_BINS                8
#define BNL_MAX_BIN_LIMITS          (BNL_MAX_BINS -1)

typedef enum {
    SCORE_ERR_OK=0,
    SCORE_ERR_INTERNAL=-1,
    SCORE_ERR_STREAM_ID_ALLOC_FAIL=-2,
    SCORE_ERR_STREAM_ID_INVALID=-3,
    SCORE_ERR_STREAM_NO_TCP_TIMESTAMP=-4,
    SCORE_ERR_STREAM_MCAST=-5,
    SCORE_ERR_STREAM_UNSUPPORTED_L4PROTO=-6,
    SCORE_ERR_STREAM_CT_NOTFOUND=-7,
    SCORE_ERR_DCP_ALLOC_FAIL=-8,
    SCORE_ERR_DCP_ID_INVALID=-9,
    SCORE_ERR_DCP_TYPE_INVALID=-10,
    SCORE_ERR_SCORE_LICENSE_INVALID=-11,
    SCORE_ERR_QSTAT_LICENSE_INVALID=-12,
    SCORE_ERR_BNL_LICENSE_INVALID=-13,
    SCORE_ERR_CFG_INVALID=-14,
    SCORE_ERR_MAX=-15
} score_error_t;

typedef struct {
    union {
       struct {
           uint16_t id:STREAM_ID_MAX_BITS;
           uint16_t dir:1;
           uint16_t reserved:STREAM_KEY_RESERVE_BITS;
       };
       struct {
           uint16_t idx:STREAM_IDX_MAX_BITS;
           uint16_t incarn:STREAM_ID_INCARN_BITS;
           uint16_t direction:1;
           uint16_t reserved1:STREAM_KEY_RESERVE_BITS;
       };
       uint16_t val;
    };
} score_stream_key_t;

/** SCORE.\n
 * Contains stream stat parameters \n
 */

typedef enum {
    DCP_TYPE_FIRST,

    DCP_TYPE_SCORE = DCP_TYPE_FIRST,
    DCP_TYPE_QSTAT,
    DCP_TYPE_BNL,

    DCP_TYPE_LAST = DCP_TYPE_BNL,
    DCP_TYPE_MAX
} score_dcp_type_t;

typedef enum {
    SCORE_DCP_CMD_STREAM_ADD=1,
    SCORE_DCP_CMD_STREAM_DELETE,
    SCORE_DCP_CMD_STREAM_UPDATEINFO,
    SCORE_DCP_CMD_STREAM_GETINFO,
    SCORE_DCP_CMD_STREAM_MAX
} score_dcp_stream_cmd_t;

typedef enum {
    SCORE_FIVE_TUPLE ,
    SCORE_SIX_TUPLE,
    SCORE_SEVEN_TUPLE,
} score_stream_tuple_type_t;

typedef enum {
    SCORE_FAMILY_IPV4,
    SCORE_FAMILY_IPV6,
    SCORE_FAMILY_MAX,
} score_ip_family_t;

typedef union {
    uint8_t  u8[4];
    uint16_t u16[2];
    uint32_t u32;
} score_ipv4_addr_t;

typedef union {
    uint8_t  u8[16];
    uint16_t u16[8];
    uint32_t u32[4];
} score_ipv6_addr_t;

typedef enum {
    SCORE_DIRECTION_ORIG,
    SCORE_DIRECTION_REPLY,
    SCORE_DIRECTION_MAX,
} score_direction_t;

typedef struct {
    score_ip_family_t family;
    union {
        score_ipv4_addr_t ipv4;
        score_ipv6_addr_t ipv6;
    };
} score_ip_addr_t;

typedef struct {
    score_ip_family_t family;
    union {
        score_ipv4_addr_t ipv4;
        score_ipv6_addr_t ipv6;
    } source_ip_addr;
    union {
        score_ipv4_addr_t ipv4;
        score_ipv6_addr_t ipv6;
    } dest_ip_addr;
    union {
        struct {
            uint16_t source_port;  /* src port */
            uint16_t dest_port;  /* dst port */
        };
        uint32_t ports;
    };
    uint8_t proto;
    score_direction_t dir;                  /* direction - optional, according to tuple_type */
    uint16_t dscp;                          /* dscp - optional, according to tuple_type */
} score_stream_tuple_t;

typedef enum {
    SCORE_REPORT_L2_OFFSET,
    SCORE_REPORT_L3_OFFSET,
    SCORE_REPORT_L4_OFFSET,
    SCORE_REPORT_MAX_OFFSET,
} score_report_offset_t;

typedef enum {
    SCORE_REPORT_LENGTH_TYPE_L2,
    SCORE_REPORT_LENGTH_TYPE_L3,
    SCORE_REPORT_LENGTH_TYPE_L4,
    SCORE_REPORT_LENGTH_TYPE_L5,
    SCORE_REPORT_LENGTH_TYPE_MAX,
} score_report_length_type_t;

typedef enum {
    score_include_data1_bit_num = 0,
    score_include_data2_bit_num,
    score_include_data3_bit_num,
    score_include_tos_bit_num,
    score_include_ttl_bit_num,
    score_include_tcp_fields_bit_num,
} score_report_bitmask_type_t;

/** SCORE.\n
 * Contains region parameters \n
 */

typedef struct {
    score_report_offset_t type;
    uint16_t offset;
    uint16_t length;                        /* must be multiple of 4 */
} score_region_t;

typedef struct _score_dcp_common_cfg_t {
    uint8_t dcp_id;                         /* allocated DCP ID */
    uint8_t reserved1[3];
    score_dcp_type_t dcp_type;              /* Profile type */
    score_stream_tuple_type_t tuple_type;   /* 5/6/7 tuple. All data in network order */
    uint8_t ring_type;                      /* 0 - common, 1 - per stream */
    uint32_t report_size;
    uint8_t reserved2[3];
    uint16_t coalescing_num;                /* number of reports to coalesce */
    uint16_t coalescing_timeout;            /* coalescing timeout in ms */
} score_dcp_common_cfg_data_t;


typedef struct _score_dcp_data_collection_t {
    union
    {
        uint32_t report_bitmask;
        struct {
            uint32_t  include_data1:1;
            uint32_t  include_data2:1;
            uint32_t  include_data3:1;
            uint32_t  include_tos:1;
            uint32_t  include_ttl:1;
            uint32_t  include_tcp_fields:1;
            uint32_t  reserved:26;
        };
    };
    score_region_t data1;              /**< region1 data in report */
    score_region_t data2;              /**< region2 data in report */
    score_region_t data3;              /**< region3 data in report */
    /* TODO: should be union with collection data */
    uint8_t qstat_cfg_id[2];           /** configuration for orig and reply directions */
    score_report_length_type_t length_type; /**< The length type of the report */
} score_dcp_data_collection_t;

typedef struct _score_sdr_cfg_t {
    void *   sdr_addr;                      /* allocated SDR addr */
    uint32_t num_entries;                   /* allocated SDR size in entries */
    uint32_t entry_size;                    /* size of a single entry */
    uint32_t offset_to_read_write;          /* location of read_write control wrt sdr_addr*/
} score_sdr_cfg_t;

typedef struct _score_sdr_dbg_t {
    uint16_t wr_index;                      /* debug info. read only */
    uint16_t rd_index;                      /* debug info. read only */
} score_sdr_dbg_t;

typedef struct _score_sdr_stat_t {
    uint32_t total_reports;
    uint32_t dropped_reports;
} score_sdr_stat_t;

/** SCORE.\n
 * Contains stream configuration parameters \n
 */
typedef struct {
    uint16_t dcp_bitmask;           /**< data collection profiled bitmask */
} score_stream_cfg_t;

/** SCORE.\n
 * Contains stream info parameters \n
 */

typedef struct {
    uint32_t id;                    /**< stream id */
    score_stream_cfg_t cfg;
} score_stream_info_t;

/** SCORE.\n
 * Contains profile configuration parameters \n
 */
typedef struct {
    score_dcp_common_cfg_data_t common;
    score_dcp_data_collection_t collection;
} score_profile_cfg_t;

/** SCORE.\n
 * Contains SDR info parameters \n
 */

typedef struct {
    uint32_t id;                    /**< SDR id */
    score_sdr_cfg_t  cfg;
    score_sdr_dbg_t  dbg;
    score_sdr_stat_t stat;
} score_sdr_info_t;

/** SCORE descriptor data for score driver */
typedef struct score_data
{
    uint16_t pending_msg_bitmask;
    uint16_t irq_cnt;
} score_data_t;

typedef struct wlan_dhd_tlm_info
{
    uint8_t radio_idx;
    uint8_t reserved1;
    uint16_t fr_idx;
    uint32_t pkt_latency;
    uint32_t bq_latency;
    uint32_t bq_amount;
} wlan_dhd_tlm_info_t;

typedef struct wired_tlm_info
{
    uint16_t qm_queue;
    uint32_t pkt_count;
    uint16_t pkt_length;
    uint32_t pkt_latency;
    union{
        struct {
            uint8_t aqm_drop    :1;
            uint8_t cq_latency_remark   :1;
            uint8_t lq_latency_remark   :1;
            uint8_t reserved    :5;
        };
        uint8_t aqm_flags; 
    };
} wired_tlm_info_t;

typedef struct wlan_nic_tlm_info
{
    /* for NIC, uint16_t (in 100us) should be enough */
    uint16_t pkt_latency;  /* egress_ts - ingress_ts */
    uint16_t hw_q_latency; /* hw_q_ts - ingress_ts : Debug*/
} wlan_nic_tlm_info_t;

/** QSTAT descriptor data for score driver */
typedef struct qstat_data
{
    uint32_t timestamp;
    uint32_t report_seq_num;
    uint16_t score_stream_id;
    uint8_t type;
    uint8_t reserved;
    union
    {
        wlan_dhd_tlm_info_t wlan_dhd;
        wired_tlm_info_t    wired;
        wlan_nic_tlm_info_t wlan_nic;
    };
} score_qstat_data_t; 

/* Below layout is used between NP & Host
 * meaning of timestamp in above struct is not clear
 * It may not be applicalbe for NIC */
typedef struct score_hw_qstat_record_wlan_nic
{
    uint16_t score_stream_key;  /* Score Key as configured in flow */
    uint16_t ingress_ts;        /* enqueue timestamp */
    uint16_t egress_ts;         /* dequeue timestamp */
    uint16_t hw_q_ts;           /* hw_q dequeue timestamp : Debug */
}score_hw_qstat_record_wlan_nic_t;

typedef struct qstat_telemetry_data {
    uint32_t reports_num;
    uint32_t last_value;
    uint32_t max_value;
    uint32_t min_value;
    uint64_t acc_value;
    uint64_t acc_sq_value;
} qstat_telemetry_data_t;

typedef struct {
    uint64_t bytes;
    uint64_t packets;
} qstat_counters_t;

typedef struct {
    uint32_t val[QSTAT_MAX_BIN_NUMBER];
} qstat_hist_t;

typedef struct {
    union{
        struct {
            uint16_t use_bincfg_ids:1;
            uint16_t use_binlimits:1;
            uint16_t ignore_pure_ack:1;
        };
        uint16_t cfg_flags; 
    };
    uint16_t app_group_id; /* opaque value from APP*/

    uint8_t rtt_orig_bincfg_id; /* per direction */
    uint8_t rtt_reply_bincfg_id; 
    /* bin limits are passed by user when use_binlimits is set
     * returned by driver when bincfg_ids is set or default bins are used 
     */
    uint32_t rtt_orig_bin_limits[BNL_MAX_BIN_LIMITS];
    uint32_t rtt_reply_bin_limits[BNL_MAX_BIN_LIMITS];
} score_bnl_stream_cfg_t;

/** BNL descriptor data for BNL driver */
typedef struct rdpa_bnl_data
{
    uint8_t protocol_is_tcp;
    uint8_t tcp_pure_ack;
    uint8_t udp_spin_bit;
    uint8_t score_stream_incarn;
    uint8_t score_stream_dir;
    uint16_t score_stream_idx;
    uint32_t tcp_seq;
    uint32_t tcp_ack;
    uint32_t tcp_tsval;
    uint32_t tcp_tsecr;
    uint64_t timestamp;/*use 64bit even thougg HW value is smaller, to keep record same as SW, linux ktime value is 64 bit */
} score_bnl_data_t;

typedef struct score_hw_archer_qstat_data_dhd
{
    uint16_t score_stream_id;
    uint16_t ingress_ts;
    uint16_t backup_queue_ts;
    uint16_t flow_ring_ts;
    uint16_t egress_ts;
    uint16_t backup_queue_level;
    uint16_t radio_idx;
    uint16_t flow_ring_id;
} __attribute__ ((__packed__)) score_hw_archer_qstat_data_dhd_t;

#endif  /* defined(__BCM_SCORE_DEFS_H_INCLUDED__) */
