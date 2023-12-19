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
#include <common/interface.h>
#include <common/sendpacket.h>
#include <common/tcpdump.h>
#include <common/utils.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef ENABLE_DMALLOC
#include <dmalloc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct tcpreplay_s; /* forward declare */

/* in memory packet cache struct */
typedef struct packet_cache_s {
    struct pcap_pkthdr pkthdr;
    u_char *pktdata;
    struct packet_cache_s *next;
} packet_cache_t;

/* packet cache header */
typedef struct file_cache_s {
    int index;
    int cached;
    int dlt;
    packet_cache_t *packet_cache;
} file_cache_t;

/* speed mode selector */
typedef enum {
    speed_multiplier = 1,
    speed_mbpsrate,
    speed_packetrate,
    speed_topspeed,
    speed_oneatatime
} tcpreplay_speed_mode;

/* speed mode configuration */
typedef struct {
    /* speed modifiers */
    tcpreplay_speed_mode mode;
    COUNTER speed;
    float multiplier;
    int pps_multi;
    u_int32_t (*manual_callback)(struct tcpreplay_s *, char *, COUNTER);
} tcpreplay_speed_t;

/* accurate mode selector */
typedef enum {
    accurate_gtod,
    accurate_select,
    accurate_nanosleep,
    accurate_ioport,
} tcpreplay_accurate;

typedef enum { source_filename = 1, source_fd = 2, source_cache = 3 } tcpreplay_source_type;

typedef struct {
    tcpreplay_source_type type;
    int fd;
    char *filename;
} tcpreplay_source_t;

/* run-time options */
typedef struct tcpreplay_opt_s {
    /* input/output */
    char *intf1_name;
    char *intf2_name;

    tcpreplay_speed_t speed;
    COUNTER loop;
    u_int32_t loopdelay_ms;

    int stats;
    bool use_pkthdr_len;

    /* tcpprep cache data */
    COUNTER cache_packets;
    char *cachedata;
    char *comment; /* tcpprep comment */

    /* deal with MTU/packet len issues */
    int mtu;

    /* accurate mode to use */
    tcpreplay_accurate accurate;

    /* limit # of packets to send */
    COUNTER limit_send;
    COUNTER limit_time;

    /* maximum sleep time between packets */
    struct timespec maxsleep;

    /* pcap file caching */
    file_cache_t file_cache[MAX_FILES];
    bool preload_pcap;

    /* pcap files/sources to replay */
    int source_cnt;
    tcpreplay_source_t sources[MAX_FILES];

#ifdef ENABLE_VERBOSE
    /* tcpdump verbose printing */
    bool verbose;
    char *tcpdump_args;
    tcpdump_t *tcpdump;
#endif

    /* dual file mode */
    bool dualfile;

#ifdef HAVE_NETMAP
    int netmap;
    int netmap_delay;
#endif

    /* print flow statistic */
    bool flow_stats;
    int flow_expiry;

    int unique_ip;
    float unique_loops;
} tcpreplay_opt_t;

/* interface */
typedef enum { intf1 = 1, intf2 } tcpreplay_intf;

/* tcpreplay context variable */
#define TCPREPLAY_ERRSTR_LEN 1024
typedef struct tcpreplay_s {
    tcpreplay_opt_t *options;
    interface_list_t *intlist;
    sendpacket_t *intf1;
    sendpacket_t *intf2;
    int intf1dlt;
    int intf2dlt;
    COUNTER iteration;
    COUNTER unique_iteration;
    COUNTER last_unique_iteration;
    sendpacket_type_t sp_type;
    char errstr[TCPREPLAY_ERRSTR_LEN];
    char warnstr[TCPREPLAY_ERRSTR_LEN];
    /* status trackers */
    int current_source; /* current source input being replayed */

    /* sleep helpers */
    struct timespec nap;
    uint32_t skip_packets;
    bool first_time;

    /* counter stats */
    tcpreplay_stats_t stats;
    tcpreplay_stats_t static_stats; /* stats returned by tcpreplay_get_stats() */

    /* flow statistics */
    flow_hash_table_t *flow_hash_table;

    /* abort, suspend & running flags */
    volatile bool abort;
    volatile bool suspend;
    bool running;
} tcpreplay_t;

/*
 * manual callback definition:
 * ctx              = tcpreplay context
 * interface        = name of interface current packet will be sent out
 * current_packet   = packet number to be sent out
 *
 * Returns number of packets to send.  0 == send all remaining packets
 * Note: Your callback method is BLOCKING the main tcpreplay loop.  If you
 * call tcpreplay_abort() from inside of your callback, you still need to
 * return (any value) so that the main loop is released and can abort.
 */
typedef u_int32_t (*tcpreplay_manual_callback)(tcpreplay_t *ctx, char *interface, COUNTER current_packet);

char *tcpreplay_geterr(tcpreplay_t *);
char *tcpreplay_getwarn(tcpreplay_t *);

tcpreplay_t *tcpreplay_init();
void tcpreplay_close(tcpreplay_t *);

/* only valid for using with GNU Autogen/AutoOpts */
int tcpreplay_post_args(tcpreplay_t *, int argc);

/* all these configuration functions return 0 on success and < 0 on error. */
int tcpreplay_set_interface(tcpreplay_t *, tcpreplay_intf, char *);
int tcpreplay_set_speed_mode(tcpreplay_t *, tcpreplay_speed_mode);
int tcpreplay_set_speed_speed(tcpreplay_t *, COUNTER);
int tcpreplay_set_speed_pps_multi(tcpreplay_t *, int);
int tcpreplay_set_loop(tcpreplay_t *, u_int32_t);
int tcpreplay_set_unique_ip(tcpreplay_t *, bool);
int tcpreplay_set_unique_ip_loops(tcpreplay_t *, int);
int tcpreplay_set_netmap(tcpreplay_t *, bool);
int tcpreplay_set_use_pkthdr_len(tcpreplay_t *, bool);
int tcpreplay_set_mtu(tcpreplay_t *, int);
int tcpreplay_set_accurate(tcpreplay_t *, tcpreplay_accurate);
int tcpreplay_set_limit_send(tcpreplay_t *, COUNTER);
int tcpreplay_set_dualfile(tcpreplay_t *, bool);
int tcpreplay_set_tcpprep_cache(tcpreplay_t *, char *);
int tcpreplay_add_pcapfile(tcpreplay_t *, char *);
int tcpreplay_set_preload_pcap(tcpreplay_t *, bool);

/* information */
int tcpreplay_get_source_count(tcpreplay_t *);
int tcpreplay_get_current_source(tcpreplay_t *);
int tcpreplay_set_flow_stats(tcpreplay_t *, bool);
int tcpreplay_set_flow_expiry(tcpreplay_t *, int);
bool tcpreplay_get_flow_stats(tcpreplay_t *);
int tcpreplay_get_flow_expiry(tcpreplay_t *);

/* functions controlling execution */
int tcpreplay_prepare(tcpreplay_t *);
int tcpreplay_replay(tcpreplay_t *);
const tcpreplay_stats_t *tcpreplay_get_stats(tcpreplay_t *);
int tcpreplay_abort(tcpreplay_t *);
int tcpreplay_suspend(tcpreplay_t *);
int tcpreplay_restart(tcpreplay_t *);
bool tcpreplay_is_suspended(tcpreplay_t *);
bool tcpreplay_is_running(tcpreplay_t *);

/* set callback for manual stepping */
int tcpreplay_set_manual_callback(tcpreplay_t *ctx, tcpreplay_manual_callback);

/* statistic counts */
COUNTER tcpreplay_get_pkts_sent(tcpreplay_t *ctx);
COUNTER tcpreplay_get_bytes_sent(tcpreplay_t *ctx);
COUNTER tcpreplay_get_failed(tcpreplay_t *ctx);
const struct timeval *tcpreplay_get_start_time(tcpreplay_t *ctx);
const struct timeval *tcpreplay_get_end_time(tcpreplay_t *ctx);

int tcpreplay_set_verbose(tcpreplay_t *, bool);
int tcpreplay_set_tcpdump_args(tcpreplay_t *, char *);
int tcpreplay_set_tcpdump(tcpreplay_t *, tcpdump_t *);

/*
 * These functions are seen by the outside world, but nobody should ever use them
 * outside of internal tcpreplay API functions
 */

#define tcpreplay_seterr(x, y, ...) __tcpreplay_seterr(x, __FUNCTION__, __LINE__, __FILE__, y, __VA_ARGS__)
void __tcpreplay_seterr(tcpreplay_t *ctx, const char *func, const int line, const char *file, const char *fmt, ...);
void tcpreplay_setwarn(tcpreplay_t *ctx, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
