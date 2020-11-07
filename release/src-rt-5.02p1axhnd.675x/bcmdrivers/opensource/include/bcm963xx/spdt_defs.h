/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

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

#ifndef __SPDT_DEFS_H__
#define __SPDT_DEFS_H__

#include <linux/socket.h>
#include <linux/types.h>

/********* Definitons and Data Structures **************/

#define SPDT_NUM_OF_STREAMS           4

/*< Generic data structures to open connections (streams) */

/*< Supported applications and protocols */
typedef enum
{
    SPDT_NONE,
    SPDT_OOKLA,
    SPDT_HTTP,
    SPDT_FTP,
    SPDT_TCP_BASIC = SPDT_FTP, /*< Basic TCP data */
    SPDT_IPERF3_TCP,
    SPDT_UDP_BASIC, /*< Basic UDP data */
    SPDT_IPERF3_UDP, /* Not supported yet, placeholder for future support */
    SPDT_MAX,
} spdt_proto_t;

static inline int is_spdt_proto_udp(spdt_proto_t proto)
{
    return proto == SPDT_UDP_BASIC || proto == SPDT_IPERF3_UDP;
}

static inline int is_spdt_proto_tcp(spdt_proto_t proto)
{
    return proto == SPDT_OOKLA || proto == SPDT_HTTP || proto == SPDT_FTP || proto == SPDT_TCP_BASIC
        || proto == SPDT_IPERF3_TCP;
}

/*< Connection parameters for each stream (=socket) */
typedef struct
{
    struct sockaddr_storage server_addr;
    struct sockaddr_storage local_addr;
    uint8_t tos;
} __attribute__ ((packed)) spdt_conn_params_t;


/*< Data structures to start/stop the speed tests on the opened connections */

/*< Transmit parameters */
typedef struct
{
    union {

        struct {
            uint64_t size;
        } tcp;

        struct {
            uint32_t kbps; /*< Data rate */
            uint32_t data_buf_len; /*< UDP packet payload; PPS will be calculated from data rate and packet payload length. The length plus packet header should not exceed max MTU size. */
            uint32_t max_burst_size; /*< Optional, can be 0 */

            void *data_buf; /*< Data buffer to be transmitted. If not set, emtpy data buffer will be allocated according to data_buf_len */
            uint64_t total_bytes_to_send; /*< Total number of bytes to be sent. Optional, when not set, trasmit should be stopped explicitly */
            uint8_t iperf3_64bit_counters; /*< Iperf3 configuration: use 64-bit counters in UDP test packets */
        } udp;
    } proto;
} __attribute__ ((packed)) spdt_tx_params_t;


/*< Receive parameters */
typedef struct
{
    union {

        struct {
            uint64_t size;
            char *file_name; /* For http GET request */
        } tcp;

        /* Currently not in use for UDP */
    } proto;
} __attribute__ ((packed)) spdt_rx_params_t;



/*< Definitions and Data structures collect speed tests statistics */

/*< TR-143 timestamp reports */
typedef enum
{
    SPDT_TR143_TS_REPORT_ROM_TIME,
    SPDT_TR143_TS_REPORT_BOM_TIME,
    SPDT_TR143_TS_REPORT_EOM_TIME,
    SPDT_TR143_TS_REPORT_MAX
} spdt_tr143_timestamp_report_t;

/*< Time report */
typedef struct
{
    uint32_t tv_sec;
    uint32_t tv_usec;
} __attribute__ ((packed)) tcp_spdt_time_rep_t;

/*< TCP Speed report */
typedef struct
{
    uint32_t rate;
    uint64_t num_bytes;
    uint64_t expected_bytes;
    uint32_t time_ms;
    tcp_spdt_time_rep_t tr143_ts[SPDT_TR143_TS_REPORT_MAX];
    int32_t status;
} __attribute__ ((packed)) tcp_spdt_rep_t; 

/* Iperf3 UDP statistics. Not supported yet, placeholder for future usage */
typedef struct
{
    struct {
        uint64_t packets;
        uint64_t bytes;
    } rx;
    struct {
        uint64_t packets;
        uint64_t bytes;
    } tx;
    uint64_t out_of_order_pkts;
    uint64_t error_cnt_pkts;
    uint32_t jitter;
} __attribute__ ((packed)) udp_spdt_iperf3_stat_t;

typedef struct
{
    struct {
        uint64_t packets; 
        uint64_t bytes; 
    } rx;
    struct {
        uint64_t packets;
        uint64_t bytes; 
    } tx;
    uint64_t rx_usec;
    uint64_t tx_usec;
} __attribute__ ((packed)) udp_spdt_basic_stat_t;

typedef struct
{
    union
    {
        udp_spdt_basic_stat_t udp_basic; 
        udp_spdt_iperf3_stat_t udp_iperf3; /* Not supported yet, placeholder for future usage */
        tcp_spdt_rep_t tcp_speed_rep;
    } proto_ext;
} __attribute__ ((packed)) spdt_stat_t;

typedef enum
{
    SPDT_DIR_NONE,
    SPDT_DIR_TX,
    SPDT_DIR_RX
} spdt_stream_dir_t;

typedef struct
{
    spdt_proto_t protocol;
    spdt_conn_params_t conn_params;
    int sock_fd;
    int is_v6;
    spdt_stream_dir_t dir;
    uint32_t tx_data_buf_len; /* Data buffer length for TX, used to calculate UDP statistics */
} __attribute__ ((packed)) spdt_stream_params_t;

#endif /* __SPDT_DEFS_H__ */

