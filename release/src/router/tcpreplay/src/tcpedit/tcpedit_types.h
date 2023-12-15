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
#include "common.h"
#include "tcpr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TCPEDIT_SOFT_ERROR -2
#define TCPEDIT_ERROR -1
#define TCPEDIT_OK 0
#define TCPEDIT_WARN 1

typedef enum { TCPEDIT_FIXLEN_OFF = 0, TCPEDIT_FIXLEN_PAD, TCPEDIT_FIXLEN_TRUNC, TCPEDIT_FIXLEN_DEL } tcpedit_fixlen;

typedef enum {
    TCPEDIT_TTL_MODE_OFF = 0,
    TCPEDIT_TTL_MODE_SET,
    TCPEDIT_TTL_MODE_ADD,
    TCPEDIT_TTL_MODE_SUB
} tcpedit_ttl_mode;

typedef enum { TCPEDIT_EDIT_BOTH = 0, TCPEDIT_EDIT_C2S, TCPEDIT_EDIT_S2C } tcpedit_direction;

typedef enum { BEFORE_PROCESS, AFTER_PROCESS } tcpedit_coder;

#define TCPEDIT_ERRSTR_LEN 1024
typedef struct {
    COUNTER packetnum;
    COUNTER total_bytes;
    COUNTER pkts_edited;
    int dlt1;
    int dlt2;
    char errstr[TCPEDIT_ERRSTR_LEN];
    char warnstr[TCPEDIT_ERRSTR_LEN];
#ifdef FORCE_ALIGN
    u_char *l3buff;
#endif
} tcpedit_runtime_t;

/*
 * need to track some packet info at runtime
 */
typedef struct {
    int l2len;
    int l3len;
    int datalen;
    u_int8_t l4proto;
    u_char *l4data;
    u_int16_t sport, dport;
    union {
        u_int32_t ipv4;
        struct tcpr_in6_addr ipv6;
    } sip, dip;
} tcpedit_packet_t;

/*
 * portmap data struct
 */
typedef struct tcpedit_portmap_s {
    long from;
    long to;
    struct tcpedit_portmap_s *next;
} tcpedit_portmap_t;

/*
 * all the arguments that the packet editing library supports
 */
typedef struct {
    bool validated; /* have we run tcpedit_validate()? */
    struct tcpeditdlt_s *dlt_ctx;

    /* runtime variables, don't mess with these */
    tcpedit_runtime_t runtime;

    /* skip rewriting IP/MAC's which are broadcast or multicast? */
    bool skip_broadcast;

    /* pad or truncate packets */
    tcpedit_fixlen fixlen;
    tcpedit_direction editdir;

    /* rewrite ip? */
    bool rewrite_ip;

    /* rewrite TCP seq/ack numbers? */
    u_int32_t tcp_sequence_enable;
    u_int32_t tcp_sequence_adjust;

    /* fix IP/TCP/UDP checksums */
    bool fixcsum;

    /* remove ethernet FCS */
    bool efcs;

    tcpedit_ttl_mode ttl_mode;
    u_int8_t ttl_value;

    /* TOS/DiffServ/ECN, -1 is disabled, else copy value */
    int tos;

    /* IPv6 FlowLabel, -1 is disabled, else copy value */
    int flowlabel;

    /* IPv6 TClass, -1 is disabled, else copy value */
    int tclass;

    /* rewrite end-point IP addresses between cidrmap1 & cidrmap2 */
    tcpr_cidrmap_t *cidrmap1; /* tcpprep cache data */
    tcpr_cidrmap_t *cidrmap2;

    /* src & dst IP mapping */
    tcpr_cidrmap_t *srcipmap;
    tcpr_cidrmap_t *dstipmap;

    /* pseudo-randomize IP addresses using a seed */
    uint32_t seed;

    /* rewrite tcp/udp ports */
    tcpedit_portmap_t *portmap;

    int mtu;           /* Deal with different MTU's */
    bool mtu_truncate; /* Should we truncate frames > MTU? */
    int maxpacket;     /* L2 header + MTU */

    uint32_t fuzz_seed;
    uint32_t fuzz_factor;
} tcpedit_t;

#ifdef __cplusplus
}
#endif
