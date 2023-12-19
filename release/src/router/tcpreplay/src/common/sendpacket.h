/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "defines.h"
#include "config.h"
#include <sys/socket.h>

#ifdef __NetBSD__
#include <net/if_ether.h>
#else
#include <netinet/if_ether.h>
#endif

#if defined HAVE_NETMAP
#include "common/netmap.h"
#include <net/netmap.h>
#endif

#ifdef HAVE_PF_PACKET
#include <netpacket/packet.h>
#endif

#ifdef HAVE_TX_RING
#include "txring.h"
#endif

#ifdef HAVE_LIBDNET
/* need to undef these which are pulled in via defines.h, prior to importing dnet.h */
#undef icmp_id
#undef icmp_seq
#undef icmp_data
#undef icmp_mask
#ifdef HAVE_DNET_H
#include <dnet.h>
#endif
#ifdef HAVE_DUMBNET_H
#include <dumbnet.h>
#endif
#endif

typedef enum sendpacket_type_e {
    SP_TYPE_NONE,
    SP_TYPE_LIBNET,
    SP_TYPE_LIBDNET,
    SP_TYPE_LIBPCAP,
    SP_TYPE_BPF,
    SP_TYPE_PF_PACKET,
    SP_TYPE_TX_RING,
    SP_TYPE_KHIAL,
    SP_TYPE_NETMAP,
    SP_TYPE_TUNTAP
} sendpacket_type_t;

/* these are the file_operations ioctls */
#define KHIAL_SET_DIRECTION (0x1)
#define KHIAL_GET_DIRECTION (0x2)

/* these are the directions */
typedef enum khial_direction_e {
    KHIAL_DIRECTION_RX = 0,
    KHIAL_DIRECTION_TX,
} khial_direction_t;

union sendpacket_handle {
    pcap_t *pcap;
    int fd;
#ifdef HAVE_LIBDNET
    eth_t *ldnet;
#endif
};

#define SENDPACKET_ERRBUF_SIZE 1024
#define MAX_IFNAMELEN 64

struct sendpacket_s {
    tcpr_dir_t cache_dir;
    int open;
    char device[MAX_IFNAMELEN];
    char errbuf[SENDPACKET_ERRBUF_SIZE];
    COUNTER retry_enobufs;
    COUNTER retry_eagain;
    COUNTER failed;
    COUNTER trunc_packets;
    COUNTER sent;
    COUNTER bytes_sent;
    COUNTER attempt;
    COUNTER flow_non_flow_packets;
    COUNTER flows;
    COUNTER flow_packets;
    COUNTER flows_unique;
    COUNTER flows_expired;
    COUNTER flows_invalid_packets;
    sendpacket_type_t handle_type;
    union sendpacket_handle handle;
    struct tcpr_ether_addr ether;
#if defined HAVE_NETMAP
    int first_packet;
    int netmap_delay;
#endif

#ifdef HAVE_NETMAP
    struct netmap_if *nm_if;
    nmreq_t nmr;
    void *mmap_addr;
    int mmap_size;
    uint32_t if_flags;
    uint32_t is_vale;
    int netmap_version;
    uint16_t first_tx_ring, last_tx_ring, cur_tx_ring;
#ifdef linux
    uint32_t data;
    uint32_t gso;
    uint32_t tso;
    uint32_t rxcsum;
    uint32_t txcsum;
#endif /* linux */
#endif /* HAVE_NETMAP */

#ifdef HAVE_PF_PACKET
    struct sockaddr_ll sa;
#ifdef HAVE_TX_RING
    txring_t *tx_ring;
#endif
#endif
    bool abort;
};

typedef struct sendpacket_s sendpacket_t;

int sendpacket(sendpacket_t *, const u_char *, size_t, struct pcap_pkthdr *);
void sendpacket_close(sendpacket_t *);
char *sendpacket_geterr(sendpacket_t *);
size_t sendpacket_getstat(sendpacket_t *, char *, size_t);
sendpacket_t *sendpacket_open(const char *, char *, tcpr_dir_t, sendpacket_type_t, void *arg);
struct tcpr_ether_addr *sendpacket_get_hwaddr(sendpacket_t *);
int sendpacket_get_dlt(sendpacket_t *);
const char *sendpacket_get_method(sendpacket_t *);
void sendpacket_abort(sendpacket_t *);
