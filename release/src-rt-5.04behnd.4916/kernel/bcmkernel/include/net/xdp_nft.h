/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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
#ifndef __XDP_NFT_H__
#define __XDP_NFT_H__

#ifndef VLAN_MAX_DEPTH
#define VLAN_MAX_DEPTH 2
#endif

#define MAX_L2_HEADER 32    /* Max size of L2 header that will be used in XDP for each packet */

typedef struct l2_hdr {
    unsigned char hdr[MAX_L2_HEADER];
    int l_shift;
    int r_shift;
} l2_hdr_t;

typedef struct _xdp_fc_ctx_t {
    __u32 tx_ifindex;
    __u32 src_nat_ip;
    __u16 src_nat_port;
    __u32 dst_nat_ip;
    __u16 dst_nat_port;
    l2_hdr_t l2_header;
    unsigned long cookie;
} xdp_fc_ctx_t;

typedef struct _xdp_fc_key_t {
    __u32 src_ip;
    __u32 dst_ip;
    __u8 proto;
    __u8 reserved0;
    __u16 reserved1;
    __u16 src_port;
    __u16 dst_port;
    __u32 rx_ifindex;
} xdp_fc_key_t;

typedef struct _pkt_count_t {
    __u64 packets;
    __u64 bytes;
} pkt_count_t;

#define MIN_PACKET_SIZE 64

#define XFP_MAP_DIR "/sys/fs/bpf"

/* names must be similar to map names that are registere in SEC in xdp_fc.c file */
#define XDP_FC_TABLE_MAP_NAME "xdp_fc_table"
#define XDP_FC_TX_PORTS_MAP_NAME "tx_port"
#define XDP_FC_PACKETS_COUNT_MAP_NAME "packet_count"

#endif
