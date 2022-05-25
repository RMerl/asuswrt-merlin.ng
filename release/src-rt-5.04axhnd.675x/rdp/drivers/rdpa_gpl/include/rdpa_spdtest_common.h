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

#ifndef _RDPA_SPDTEST_COMMON_H_
#define _RDPA_SPDTEST_COMMON_H_

#include "rdpa_types.h"

/** Minimum transmitted packet size for padding */
#define RDPA_SPDTEST_MIN_TX_PD_LEN 60

typedef enum
{
    rdpa_udpspdtest_proto_basic,
    rdpa_udpspdtest_proto_iperf3,
} rdpa_udpspdtest_proto_t;

#define RDPA_SPDTEST_SO_MARK_PREFIX         0xFEFEFC00 /* Include TCP and UDP socket mark */
#define RDPA_SPDTEST_SO_MARK_PREFIX_MASK    0xFFFFFC00 
#define RDPA_TCPSPDTEST_SO_MARK             0xFEFEFD00
#define RDPA_UDPSPDTEST_SO_MARK_PREFIX      0xFEFEFE00
#define RDPA_UDPSPDTEST_SO_MARK_PREFIX_MASK 0xFFFFFE00
#define RDPA_UDPSPDTEST_SO_MARK_BASIC       0xFEFEFE10
#define RDPA_UDPSPDTEST_SO_MARK_IPERF3      0xFEFEFE20
#define RDPA_UDPSPDTEST_SO_MARK_LAST        0xFEFEFE30
#define RDPA_UDPSPDTEST_SO_MARK_TYPE_MASK   0xFFFFFEF0
#define RDPA_SPDT_STREAM_ID_MASK                   0xF /* Mask includes up to 16 streams, today 4 streams are supported */

#define RDPA_UDPSPDTEST_DEF_MBS 2000


/** Speed Test Engine Reference Packet, includes both packet header and payload.\n
 * required by the Runner Speed Test Engine for TX tests.\n
 */
typedef struct
{
    uint16_t size;           /**< Reference packet size */
    void *data;              /**< Reference packet pointer */
    union {
        struct {
            uint16_t payload_offset; /**< Reference packet payload offset */
        } udp;
        uint16_t h;
    };
} rdpa_spdtest_ref_pkt_t;


/** UDP Speed Test RX parameters.\n
 */
typedef struct {
    bdmf_ip_t local_ip_addr;   /**< Local IP Address */
    uint16_t local_port_nbr;   /**< Local UDP Port Number */
    bdmf_ip_t remote_ip_addr;  /**< Remote IP Address */
    uint16_t remote_port_nbr;  /**< Remote UDP Port Number */
    bdmf_index flow_index[rdpa_if__number_of];  /**< Analyzer Unicast Flow Index */
} rdpa_udpspdtest_rx_params_t;

/** UDP Speed Test TX parameters.\n
 */
typedef struct {
    uint32_t kbps;           /**< Transmit Rate (Kbps) */
    uint32_t mbs;            /**< Maximum Burst Size (bytes) */
    uint64_t total_packets_to_send; /**< Total number of packets to send (optional) */
    bdmf_boolean iperf3_64bit_counters; /**< Iperf3 configuration: use 64-bit counters in UDP test packets */
} rdpa_udpspdtest_tx_params_t;

#endif /* _RDPA_SPDTEST_H_ */
