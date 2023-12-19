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
#include "common.h"

typedef struct {
    char *active_pcap;
    COUNTER bytes_sent;
    COUNTER pkts_sent;
    COUNTER failed;
    struct timeval start_time;
    struct timeval time_delta;
    struct timeval end_time;
    struct timeval pkt_ts_delta;
    struct timeval last_print;
    COUNTER flow_non_flow_packets;
    COUNTER flows;
    COUNTER flows_unique;
    COUNTER flow_packets;
    COUNTER flows_expired;
    COUNTER flows_invalid_packets;
} tcpreplay_stats_t;

int read_hexstring(const char *l2string, u_char *hex, int hexlen);
void packet_stats(const tcpreplay_stats_t *stats);
int format_date_time(struct timeval *when, char *buf, size_t len);
uint32_t tcpr_random(uint32_t *seed);
void restore_stdin(void);

/* our "safe" implimentations of functions which allocate memory */
#define safe_malloc(x) our_safe_malloc(x, __FUNCTION__, __LINE__, __FILE__)
void *our_safe_malloc(size_t len, const char *, int, const char *);

#define safe_realloc(x, y) our_safe_realloc(x, y, __FUNCTION__, __LINE__, __FILE__)
void *our_safe_realloc(void *ptr, size_t len, const char *, int, const char *);

#define safe_strdup(x) our_safe_strdup(x, __FUNCTION__, __LINE__, __FILE__)
char *our_safe_strdup(const char *str, const char *, int, const char *);

#define safe_free(x) our_safe_free(x, __FUNCTION__, __LINE__, __FILE__)
void our_safe_free(void *ptr, const char *, int, const char *);

#define safe_pcap_next(x, y) our_safe_pcap_next(x, y, __FUNCTION__, __LINE__, __FILE__)
u_char *
our_safe_pcap_next(pcap_t *pcap, struct pcap_pkthdr *pkthdr, const char *funcname, int line, const char *file);

#define safe_pcap_next_ex(x, y, z) our_safe_pcap_next_ex(x, y, z, __FUNCTION__, __LINE__, __FILE__)
int our_safe_pcap_next_ex(pcap_t *pcap,
                           struct pcap_pkthdr **pkthdr,
                           const u_char **pktdata,
                           const char *funcname,
                           int line,
                           const char *file);

#define MAX_ARGS 128

#ifndef HAVE_INET_ATON
#define HAVE_INET_ATON
#define USE_CUSTOM_INET_ATON
int inet_aton(const char *name, struct in_addr *addr);
#endif

#if SIZEOF_LONG == 8
#define do_div(n, base)                                                                                                \
    ({                                                                                                                 \
        uint32_t __base = (base);                                                                                      \
        uint32_t __rem;                                                                                                \
        __rem = ((uint64_t)(n)) % __base;                                                                              \
        (n) = ((uint64_t)(n)) / __base;                                                                                \
        __rem;                                                                                                         \
    })
#elif SIZEOF_LONG == 4
extern uint32_t __div64_32(uint64_t *dividend, uint32_t divisor);
#define do_div(n, base)                                                                                                \
    ({                                                                                                                 \
        uint32_t __base = (base);                                                                                      \
        uint32_t __rem;                                                                                                \
        if (((n) >> 32) == 0) {                                                                                        \
            __rem = (uint32_t)(n) % __base;                                                                            \
            (n) = (uint32_t)(n) / __base;                                                                              \
        } else                                                                                                         \
            __rem = __div64_32(&(n), __base);                                                                          \
        __rem;                                                                                                         \
    })
#else /* SIZEOF_LONG == ?? */
#error do_div() does not yet support the C64
#endif /* SIZEOF_LONG  */
