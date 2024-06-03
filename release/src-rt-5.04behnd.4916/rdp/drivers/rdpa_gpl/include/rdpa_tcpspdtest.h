/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
*
*    Copyright (c) 2014 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
* :>
*/

#ifndef _RDPA_TCPSPDTEST_H_
#define _RDPA_TCPSPDTEST_H_

/** \defgroup Tcp Speed Test
 * This object is used to manage the Tcp Speed Test \n
 * @{
 */

#include "rdd.h"
#include "rdpa_types.h"
#include "rdpa_spdtest_common.h"

/** Minimum transmitted packet size for padding */
#define RDPA_TCPSPDTEST_MIN_TX_PD_LEN  RDPA_SPDTEST_MIN_TX_PD_LEN
#define RDD_SPDTEST_ENGINE_REF_PKT_HDR_HDR_NUMBER 120

#define RDPA_TCPSPDTEST_RING_RTO_TX_SEQ_ELEMENT_SIZE  TCPSPDTEST_RING_RTO_TX_SEQ_ELEMENT_SIZE
#define RDPA_TCPSPDTEST_RING_RTO_TX_SEQ_SIZE  ((uint32_t)(TCPSPDTEST_RING_RTO_TX_SEQ_SIZE + sizeof(uint64_t))) /* Add rd/wr idx space */
#define RDPA_TCPSPDTEST_RING_RTO_TX_SEQ_MASK  (TCPSPDTEST_RING_RTO_TX_SEQ_SIZE - 1)
#define RDPA_TCPSPDTEST_RING_RTO_TX_SEQ_COALESCING_THR  TCPSPDTEST_RING_RTO_TX_SEQ_COALESCING_THR
#define RDPA_TCPSPDTEST_RING_RTO_RETRANS_ELEMENT_SIZE  TCPSPDTEST_RING_RTO_RETRANS_ELEMENT_SIZE
#define RDPA_TCPSPDTEST_RING_RTO_RETRANS_SIZE  ((uint32_t)(TCPSPDTEST_RING_RTO_RETRANS_SIZE + sizeof(uint64_t))) /* Add rd/wr idx space */
#define RDPA_TCPSPDTEST_RING_RTO_RETRANS_MASK  (TCPSPDTEST_RING_RTO_RETRANS_SIZE - 1)
#define RDPA_TCPSPDTEST_RING_SACK_OPT_ELEMENT_SIZE  TCPSPDTEST_RING_SACK_OPT_ELEMENT_SIZE
#define RDPA_TCPSPDTEST_RING_SACK_OPT_SIZE  ((uint32_t)(TCPSPDTEST_RING_SACK_OPT_SIZE + sizeof(uint64_t))) /* Add rd/wr idx space */
#define RDPA_TCPSPDTEST_RING_SACK_OPT_MASK  (TCPSPDTEST_RING_SACK_OPT_SIZE - 1)
#define RDPA_TCPSPDTEST_RING_PKT_DROP_RX_SEQ_ELEMENT_SIZE  TCPSPDTEST_RING_PKT_DROP_RX_SEQ_ELEMENT_SIZE
#define RDPA_TCPSPDTEST_RING_PKT_DROP_RX_SEQ_SIZE  ((uint32_t)(TCPSPDTEST_RING_PKT_DROP_RX_SEQ_SIZE + sizeof(uint64_t))) /* Add rd/wr idx space */
#define RDPA_TCPSPDTEST_RING_PKT_DROP_RX_SEQ_MASK  TCPSPDTEST_RING_PKT_DROP_RX_SEQ_MASK
#define RDPA_TCPSPDTEST_RING_PKT_DROP_STATE_NO_ERR  TCPSPDTEST_RING_PKT_DROP_STATE_NO_ERR
#define RDPA_TCPSPDTEST_RING_PKT_DROP_STATE_DRIVER_FIXING  TCPSPDTEST_RING_PKT_DROP_STATE_DRIVER_FIXING
#define RDPA_TCPSPDTEST_RING_PKT_DROP_STATE_DRIVER_FIXED  TCPSPDTEST_RING_PKT_DROP_STATE_DRIVER_FIXED  
#define RDPA_TCPSPDTEST_RING_PKT_DROP_STATE_STREAM_ENDED  TCPSPDTEST_RING_PKT_DROP_STATE_STREAM_ENDED

typedef enum {
    RDPA_TCPSPDTEST_FIELD_GLOBAL_INFO_ALL = 0,
    RDPA_TCPSPDTEST_FIELD_GLOBAL_INFO_FW_WAKEUP_VECTOR,
} rdpa_tcpspdtest_field_global_info_t;

typedef enum {
    RDPA_TCPSPDTEST_FIELD_TCB_ALL = 0,
    RDPA_TCPSPDTEST_FIELD_TCB_SEQS,
    RDPA_TCPSPDTEST_FIELD_TCB_BYTES,
    RDPA_TCPSPDTEST_FIELD_TCB_EXPECTED_BYTES,
    RDPA_TCPSPDTEST_FIELD_TCB_SACK_BYTES,
    RDPA_TCPSPDTEST_FIELD_TCB_DROP_DRIVER_WAKEUP,
} rdpa_tcpspdtest_field_tcb_t;

typedef enum {
    RDPA_TCPSPDTEST_FIELD_PKT_DROP_ALL = 0,
    RDPA_TCPSPDTEST_FIELD_PKT_DROP_STATE,
} rdpa_tcpspdtest_field_pkt_drop_t;

/** Tcp Speed Test Engine Global Information.\n
 * Contains the Global Information for Tcp Speed Test Engine. \n
 * required by the Runner Tcp Speed Test Engine.\n
 */
typedef struct {
    rdpa_tcpspdtest_field_global_info_t field;
    uint8_t  fw_wakeup_stream_idx;        /**< Download fw wakeup stream idx */
    uint8_t  num_streams;                 /**< Number of tCP streams */
    uint32_t up_bucket_tokens;            /**< Upload tcp tx leaky bucket num of tokens */
    uint32_t up_bucket_full_tokens;       /**< Upload tcp tx leaky bucket full num of tokens */
    uint16_t up_bucket_tokens_fill_rate;  /**< Upload tcp tx leaky bucket tokens per timer tick */
} rdpa_tcpspdtest_engine_global_info_t;

/** Tcp Speed Test Engine Connection Information.\n
 * Contains the Connection Information to Server \n
 * required by the Runner Tcp Speed Test Engine.\n
 */
typedef struct {
    uint8_t   l2_hdr_len;               /**< L2 header len */
    uint8_t   l3_hdr_len;               /**< L3 header len */
    rdpa_ic_l3_protocol  l3_protocol;   /**< L3 Protocol */
    uint8_t   l4_hdr_len;               /**< L4 header len */
    uint8_t   tx_hdr_len;               /**< Tx packet header len */
    uint32_t  cpu_rx_rdd_queue;         /**< CPU queue num */
    bdmf_object_handle port;            /**< WAN/LAN destination port */
    rdpa_flow wan_flow;                 /**< Destination flow for method=port, Source flow for method=bridge,port=wan; XXX: needed? */
    uint32_t  egress_queue_id;          /**< Egress queue id. method=port only */
    uint16_t  up_tx_mss;                /**< Upload tcp tx mss */
    uint16_t  up_tx_max_pd_len;         /**< Upload tcp max pd len */
    uint16_t  up_pppoe_hdr_ofs;         /**< Upload tcp pppoe header offset */
    uint8_t  up_peer_rx_scale;          /**< Upload tcp peer rx window scale */
    uint8_t   sack_permitted;           /**< is selective ack permitted */
} rdpa_tcpspdtest_engine_conn_info_t;

/** Tcp Speed Test Engine TCB.\n
 * Contains the Task Control State of the Runner Tcp Speed Test Engine \n
 * required by the Runner Tcp Speed Test Engine.\n
 */
typedef struct {
    rdpa_tcpspdtest_field_tcb_t field;
    uint32_t rx_pkts;                   /**< Received packets counter */
    uint32_t txed_pkts;                 /**< Num of txed packets */
    uint32_t freed_pkts;                /**< Num of freed packets */
    uint32_t ack_seq;                   /**< Calculated ack seq */
    uint16_t dynack_bytes_thr;          /**< Ack reply bytes threshold */
    uint8_t  dynack_thr_upd_rate;       /**< Ack reply threshold update rate */
    uint32_t tx_seq;                    /**< Last transmitted seq */
    uint16_t bad_pkts;                  /**< Num of received packets with unexpected seq */
    uint16_t no_dispatcher_credits;     /**< Num of dispatcher no credits */
    uint16_t no_pktgen_tx_credits;      /**< Num of pktgen_tx no credits */
    uint16_t src_port;                  /**< Tcp source port */
    uint64_t rx_bytes;                  /**< Received bytes */
    uint64_t expected_bytes;            /**< Expected bytes */
    uint64_t up_to_send_bytes;          /**< Upload num of remaining transmit bytes */
    uint32_t up_cwnd;                   /**< Upload tcp tx congestion window */
    uint32_t up_cwnd_thr;               /**< Upload tcp tx congestion window threshold */
    uint32_t up_cwnd_initial;           /**< Upload tcp initial tx congestion window */
    uint32_t up_cwnd_max;               /**< Upload tcp max tx congestion window */
    uint8_t  up_cwnd_rtt_factor;        /**< Upload tcp congestion window rtt factor */
    uint32_t up_last_ack_seq;           /**< Upload last rx ack sequence */
    uint32_t up_sack_bytes;             /**< Upload current number of acknowledged sack bytes */
    uint8_t dn_is_pkt_drop_driver_wakeup; /**< Download is pkt drop wakeup */
    uint8_t is_upload;                  /**< Is Upload stream content */
} rdpa_tcpspdtest_engine_tcb_t;

/** Tcp/Udp Speed Test Engine Reference Packet Header.\n
 * Contains the Acknowledge Refernce Packet for Tcp/Udp Speed Test Engine \n
 * required by the Runner Tcp/Udp Speed Test Engine.\n
 */
typedef struct
{
    uint16_t size;                                           /**< Reference packet header size */
    uint16_t offset;                                         /**< Packet offset for l3 8 bytes align */
    uint8_t  hdr[RDD_SPDTEST_ENGINE_REF_PKT_HDR_HDR_NUMBER]; /**< Reference packet header ptr */
} rdpa_spdtest_engine_ref_pkt_hdr_t;

typedef rdpa_spdtest_engine_ref_pkt_hdr_t rdpa_tcpspdtest_engine_ref_pkt_hdr_t;

/** Tcp Speed Test Engine Reference Packet, includes both packet header and payload.\n
 * required by the Runner Tcp Speed Test Engine for Upload test.\n
 */
typedef rdpa_spdtest_ref_pkt_t rdpa_tcpspdtest_engine_ref_pkt_t;


/** Tcp Speed Test Engine Packet Drop Handling.\n
 * Contains the Packet Drop Handling State for Tcp Speed Test Engine \n
 * required by the Runner Tcp Speed Test Engine.\n
 */
typedef struct
{
    rdpa_tcpspdtest_field_pkt_drop_t field;
    uint8_t   drop_state;    /**< Packets drop state */
    uint8_t   is_win_full;   /**< Is sequence window full */
    uint16_t  num_errs;      /**< Num of err events. One event for multiple burst lost */
    uint16_t  wr_ofs;        /**< Write offset for good frames window during pkt drops */
    uint16_t  rd_ofs;        /**< Read offset for good frames window during pkt drops */
} rdpa_tcpspdtest_engine_pkt_drop_t;

typedef struct
{
    uint32_t  addr_low;      /**< ring ddr phys addr low */
    uint32_t  addr_high;     /**< ring ddr phys addr high */
    uint32_t  send_cnt;      /**< ring send counter */
    uint32_t  drop_cnt;      /**< ring drop counter */
    uint32_t  receive_cnt;   /**< ring drop counter */
    void      *mem_ptr;      /**< ddr memory addr for runner txed seqs */
} rdpa_tcpspdtest_ring_ddr_t;

/** Tcp Speed Test Engine RTO Handling.\n
 * Contains the RTO Handling State for Tcp Speed Test Engine \n
 * required by the Runner Tcp Speed Test Engine.\n
 */
typedef struct
{
    rdpa_tcpspdtest_ring_ddr_t rto_tx_seq; /**< Ddr ring for rto txed seqs */
    rdpa_tcpspdtest_ring_ddr_t rto_retrans; /**< Ddr ring for retransmission seqs */
    rdpa_tcpspdtest_ring_ddr_t sack_opt; /**< Ddr ring for sack opt */
} rdpa_tcpspdtest_engine_ring_t;

/** Tcp Speed Test Engine Download SACK handling.\n
 * Contains the Download SACK Handling State for Tcp Speed Test Engine \n
 * required by the Runner Tcp Speed Test Engine.\n
 */
typedef struct
{
    rdpa_tcpspdtest_ring_ddr_t pkt_drop_rx_seq; /**< Ddr ring for pkt drop rx seqs */
} rdpa_tcpspdtest_engine_ring_dn_t;

typedef struct
{
    uint8_t stream_idx;
    uint8_t is_upload;
} rdpa_tcpspdtest_stream_info_t;

/** @} end of tcpspdtest Doxygen group. */

#endif /* _RDPA_TCPSPDTEST_H_ */
