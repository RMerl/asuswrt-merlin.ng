/*
 *   Copyright (c) 2013 Fred Klassen <fklassen at appneta dot com> - AppNeta
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

#include <net/if.h>
#include <net/netmap.h>
#include <net/netmap_user.h>

#ifdef linux
#include <linux/ethtool.h>
#include <linux/sockios.h>
#endif /* linux */

#ifndef NETMAP_API
#define NETMAP_API 0
#endif

#if NETMAP_API >= 10
#define NETMAP_TX_RING_EMPTY(ring) (!nm_tx_pending(ring))
#define NETMAP_RING_NEXT(r, i) nm_ring_next(r, i)
#elif defined HAVE_NETMAP_RING_HEAD_TAIL
#define NETMAP_TX_RING_EMPTY(ring) (nm_ring_space(ring) >= (ring)->num_slots - 1)
#define NETMAP_RING_NEXT(r, i) nm_ring_next(r, i)
#else
#define nm_ring_space(ring) (ring->avail)
#endif

#ifndef HAVE_NETMAP_NR_REG
#define NR_REG_MASK 0xf /* values for nr_flags */
#if NETMAP_API < 11
enum {
    NR_REG_DEFAULT = 0, /* backward compat, used in older versions. */
    NR_REG_ALL_NIC,
    NR_REG_SW,
    NR_REG_NIC_SW,
    NR_REG_ONE_NIC,
    NR_REG_PIPE_MASTER,
    NR_REG_PIPE_SLAVE,
};
#endif /* NETMAP_API < 11 */
#endif

#ifndef NETMAP_HW_RING
#define NETMAP_HW_RING 0x4000 /* single NIC ring pair */
#endif
#ifndef NETMAP_SW_RING
#define NETMAP_SW_RING 0x2000 /* only host ring pair */
#endif
#ifndef NETMAP_RING_MASK
#define NETMAP_RING_MASK 0x0fff /* the ring number */
#endif
#ifndef NETMAP_NO_TX_POLL
#define NETMAP_NO_TX_POLL 0x1000 /* no automatic txsync on poll */
#endif
#ifndef NETMAP_DO_RX_POLL
#define NETMAP_DO_RX_POLL 0x8000 /* DO automatic rxsync on poll */
#endif
#ifndef NETMAP_BDG_ATTACH
#define NETMAP_BDG_ATTACH 1 /* attach the NIC */
#endif
#ifndef NETMAP_BDG_DETACH
#define NETMAP_BDG_DETACH 2 /* detach the NIC */
#endif
#ifndef NETMAP_BDG_LOOKUP_REG
#define NETMAP_BDG_LOOKUP_REG 3 /* register lookup function */
#endif
#ifndef NETMAP_BDG_LIST
#define NETMAP_BDG_LIST 4 /* get bridge's info */
#endif
#ifndef NETMAP_BDG_VNET_HDR
#define NETMAP_BDG_VNET_HDR 5 /* set the port virtio-net-hdr length */
#endif
#ifndef NETMAP_BDG_OFFSET
#define NETMAP_BDG_OFFSET NETMAP_BDG_VNET_HDR /* deprecated alias */
#endif
#ifndef NETMAP_BDG_HOST
#define NETMAP_BDG_HOST 1 /* attach the host stack on ATTACH */
#endif

#ifdef HAVE_NETMAP_NR_FLAGS
typedef struct nmreq nmreq_t;
#else
struct tcpr_nmreq {
    char nr_name[IFNAMSIZ];
    uint32_t nr_version;  /* API version */
    uint32_t nr_offset;   /* nifp offset in the shared region */
    uint32_t nr_memsize;  /* size of the shared region */
    uint32_t nr_tx_slots; /* slots in tx rings */
    uint32_t nr_rx_slots; /* slots in rx rings */
    uint16_t nr_tx_rings; /* number of tx rings */
    uint16_t nr_rx_rings; /* number of rx rings */
    uint16_t nr_ringid;   /* ring(s) we care about */
    uint16_t nr_cmd;
    uint16_t nr_arg1; /* reserve extra rings in NIOCREGIF */
    uint16_t nr_arg2;
    uint32_t nr_arg3; /* req. extra buffers in NIOCREGIF */
    uint32_t nr_flags;
    /* various modes, extends nr_ringid */
    uint32_t spare2[1];
};
typedef struct tcpr_nmreq nmreq_t;

#endif /* HAVE_NETMAP_NR_FLAGS */

int get_netmap_version(void);
void *sendpacket_open_netmap(const char *device, char *errbuf, void *arg);
void sendpacket_close_netmap(void *p);
bool netmap_tx_queues_empty(void *p);
int sendpacket_send_netmap(void *p, const u_char *data, size_t len);
