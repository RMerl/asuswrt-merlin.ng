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

#include "config.h"
#include "defines.h"
#include "common.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include "tcpreplay_api.h"
#include "send_packets.h"
#include "replay.h"

#ifdef TCPREPLAY_EDIT
#include "tcpreplay_edit_opts.h"
#else
#include "tcpreplay_opts.h"
#endif



/**
 * \brief Returns a string describing the last error.
 *
 * Value when the last call does not result in an error is undefined 
 * (may be NULL, may be garbage)
 */
char *
tcpreplay_geterr(tcpreplay_t *ctx)
{
    assert(ctx);
    return(ctx->errstr);
}

/**
 * \brief Returns a string describing the last warning.  
 *
 * Value when the last call does not result in an warning is undefined 
 * (may be NULL, may be garbage)
 */
char *
tcpreplay_getwarn(tcpreplay_t *ctx)
{
    assert(ctx);
    return(ctx->warnstr);
}

/**
 * \brief Initialize a new tcpreplay context
 *
 * Allocates memory and stuff like that.  Always returns a buffer or completely
 * fails by calling exit() on malloc failure.
 */
tcpreplay_t *
tcpreplay_init()
{
    tcpreplay_t *ctx;

    /* allocations will reset everything to zeros */
    ctx = safe_malloc(sizeof(tcpreplay_t));
    ctx->options = safe_malloc(sizeof(tcpreplay_opt_t));

    /* replay packets only once */
    ctx->options->loop = 1;

    /* Default mode is to replay pcap once in real-time */
    ctx->options->speed.mode = speed_multiplier;
    ctx->options->speed.multiplier = 1.0;

    /* Set the default timing method */
    ctx->options->accurate = accurate_gtod;

    /* set the default MTU size */
    ctx->options->mtu = DEFAULT_MTU;

    /* disable periodic statistics */
    ctx->options->stats = -1;

    /* disable limit send */
    ctx->options->limit_send = -1;

    /* default unique-loops */
    ctx->options->unique_loops = 1.0;

#ifdef ENABLE_VERBOSE
    /* clear out tcpdump struct */
    ctx->options->tcpdump = (tcpdump_t *)safe_malloc(sizeof(tcpdump_t));
#endif

    if (fcntl(STDERR_FILENO, F_SETFL, O_NONBLOCK) < 0)
        tcpreplay_setwarn(ctx, "Unable to set STDERR to non-blocking: %s", strerror(errno));

#ifdef ENABLE_PCAP_FINDALLDEVS
    ctx->intlist = get_interface_list();
#else
    ctx->intlist = NULL;
#endif

    /* set up flows - on by default*/
    ctx->options->flow_stats = 1;
    ctx->flow_hash_table = flow_hash_table_init(DEFAULT_FLOW_HASH_BUCKET_SIZE);

    ctx->sp_type = SP_TYPE_NONE;
    ctx->intf1dlt = -1;
    ctx->intf2dlt = -1;
    ctx->abort = false;
    ctx->first_time = true;
    return ctx;
}

/**
 * \brief Parses the GNU AutoOpts options for tcpreplay
 *
 * If you're using AutoOpts with tcpreplay_api, then just call this after
 * optionProcess() and it will parse all the options for you.  As always,
 * returns 0 on success, and -1 on error & -2 on warning.
 */
int
tcpreplay_post_args(tcpreplay_t *ctx, int argc)
{
    char *temp, *intname;
    char *ebuf;
    tcpreplay_opt_t *options;
    int warn = 0;
    float n;
    int ret = 0;

    options = ctx->options;

    dbg(2, "tcpreplay_post_args: parsing command arguments");
    ebuf = safe_malloc(SENDPACKET_ERRBUF_SIZE);
#ifdef DEBUG
    if (HAVE_OPT(DBUG))
        debug = OPT_VALUE_DBUG;
#else
    if (HAVE_OPT(DBUG)) {
        warn ++;
        tcpreplay_setwarn(ctx, "%s", "not configured with --enable-debug.  Debugging disabled.");
    }
#endif

    options->loop = OPT_VALUE_LOOP;
    options->loopdelay_ms = OPT_VALUE_LOOPDELAY_MS;

    if (HAVE_OPT(LIMIT))
        options->limit_send = OPT_VALUE_LIMIT;

    if (HAVE_OPT(DURATION))
        options->limit_time = OPT_VALUE_DURATION;

    if (HAVE_OPT(TOPSPEED)) {
        options->speed.mode = speed_topspeed;
        options->speed.speed = 0;
    } else if (HAVE_OPT(PPS)) {
        n = atof(OPT_ARG(PPS));
        if (!n) {
            tcpreplay_seterr(ctx, "invalid pps value '%s'", OPT_ARG(PPS));
            ret = -1;
            goto out;
        }
        options->speed.speed = (COUNTER)(n * 60.0 * 60.0); /* convert to packets per hour */
        options->speed.mode = speed_packetrate;
        options->speed.pps_multi = OPT_VALUE_PPS_MULTI;
    } else if (HAVE_OPT(ONEATATIME)) {
        options->speed.mode = speed_oneatatime;
        options->speed.speed = 0;
    } else if (HAVE_OPT(MBPS)) {
        n = atof(OPT_ARG(MBPS));
        if (n) {
            options->speed.mode = speed_mbpsrate;
            options->speed.speed = (COUNTER)(n * 1000000.0); /* convert to bps */
        } else {
            options->speed.mode = speed_topspeed;
            options->speed.speed = 0;
        }
    } else if (HAVE_OPT(MULTIPLIER)) {
        options->speed.mode = speed_multiplier;
        options->speed.multiplier = atof(OPT_ARG(MULTIPLIER));
    }

    if (HAVE_OPT(MAXSLEEP)) {
        options->maxsleep.tv_sec = OPT_VALUE_MAXSLEEP / 1000;
        options->maxsleep.tv_nsec = (OPT_VALUE_MAXSLEEP % 1000) * 1000 * 1000;
    }

#ifdef ENABLE_VERBOSE
    if (HAVE_OPT(VERBOSE))
        options->verbose = 1;

    if (HAVE_OPT(DECODE))
        options->tcpdump->args = safe_strdup(OPT_ARG(DECODE));
#endif

    if (HAVE_OPT(STATS))
        options->stats = OPT_VALUE_STATS;

    /*
     * preloading the pcap before the first run
     */

    if (HAVE_OPT(PRELOAD_PCAP)) {
        options->preload_pcap = true;
    }

#ifdef TCPREPLAY_EDIT
    if (HAVE_OPT(PRELOAD_PCAP) && OPT_VALUE_LOOP > 1) {
        tcpreplay_seterr(ctx,
                         "%s",
                         "tcpreplay_edit --loop (-l) and --preload_pcap (-K) options are mutually exclusive");
        ret = -1;
        goto out;
    }
#endif

    /* Dual file mode */
    if (HAVE_OPT(DUALFILE)) {
        options->dualfile = true;
        if (argc < 2) {
            tcpreplay_seterr(ctx, "%s", "--dualfile mode requires at least two pcap files");
            ret = -1;
            goto out;
        }
        if (argc % 2 != 0) {
            tcpreplay_seterr(ctx, "%s", "--dualfile mode requires an even number of pcap files");
            ret = -1;
            goto out;
        }
    }

#ifdef HAVE_NETMAP
    options->netmap_delay = OPT_VALUE_NM_DELAY;
#endif

    if (HAVE_OPT(NETMAP)) {
#ifdef HAVE_NETMAP
        options->netmap = 1;
        ctx->sp_type = SP_TYPE_NETMAP;
#else
         err(-1, "--netmap feature was not compiled in. See INSTALL.");
#endif
    }

    if (HAVE_OPT(UNIQUE_IP))
        options->unique_ip = 1;

    if (HAVE_OPT(UNIQUE_IP_LOOPS)) {
        options->unique_loops = atof(OPT_ARG(UNIQUE_IP_LOOPS));
        if (options->unique_loops < 1.0) {
            tcpreplay_seterr(ctx, "%s", "--unique-ip-loops requires loop count >= 1.0");
            ret = -1;
            goto out;
        }
    }

    /* flow statistics */
    if (HAVE_OPT(NO_FLOW_STATS))
        options->flow_stats = 0;

    if (HAVE_OPT(FLOW_EXPIRY)) {
        options->flow_expiry = OPT_VALUE_FLOW_EXPIRY;
    }

    if (HAVE_OPT(TIMER)) {
        if (strcmp(OPT_ARG(TIMER), "select") == 0) {
#ifdef HAVE_SELECT
            options->accurate = accurate_select;
#else
            tcpreplay_seterr(ctx, "%s", "tcpreplay_api not compiled with select support");
            ret = -1;
            goto out;
#endif
        } else if (strcmp(OPT_ARG(TIMER), "ioport") == 0) {
#if defined HAVE_IOPORT_SLEEP__
            options.accurate = ACCURATE_IOPORT;
            ioport_sleep_init();
#else
            err(-1, "tcpreplay not compiled with IO Port 0x80 support");
#endif
        } else if (strcmp(OPT_ARG(TIMER), "gtod") == 0) {
            options->accurate = accurate_gtod;
        } else if (strcmp(OPT_ARG(TIMER), "nano") == 0) {
            options->accurate = accurate_nanosleep;
        } else if (strcmp(OPT_ARG(TIMER), "abstime") == 0) {
            tcpreplay_seterr(ctx, "%s", "abstime is deprecated");
            ret = -1;
            goto out;
        } else {
            tcpreplay_seterr(ctx, "Unsupported timer mode: %s", OPT_ARG(TIMER));
            ret = -1;
            goto out;
        }
    }

#ifdef HAVE_RDTSC
    if (HAVE_OPT(RDTSC_CLICKS)) {
        rdtsc_calibrate(OPT_VALUE_RDTSC_CLICKS);
    }
#endif

    if (HAVE_OPT(PKTLEN)) {
        options->use_pkthdr_len = true;
        warn ++;
        tcpreplay_setwarn(ctx, "%s", "--pktlen may cause problems.  Use with caution.");
    }

    if ((intname = get_interface(ctx->intlist, OPT_ARG(INTF1))) == NULL) {
        if (!strncmp(OPT_ARG(INTF1), "netmap:", 7) || !strncmp(OPT_ARG(INTF1), "vale", 4))
            tcpreplay_seterr(ctx, "Unable to connect to netmap interface %s. Ensure netmap module is installed (see INSTALL).",
                    OPT_ARG(INTF1));
        else
            tcpreplay_seterr(ctx, "Invalid interface name/alias: %s", OPT_ARG(INTF1));

        ret = -1;
        goto out;
    }

    if (!strncmp(intname, "netmap:", 7) || !strncmp(intname, "vale:", 5)) {
#ifdef HAVE_NETMAP
        options->netmap = 1;
        ctx->sp_type = SP_TYPE_NETMAP;
#else
        tcpreplay_seterr(ctx, "%s", "tcpreplay_api not compiled with netmap support");
        ret = -1;
        goto out;
#endif
    }

    options->intf1_name = safe_strdup(intname);

    /* open interfaces for writing */
    if ((ctx->intf1 = sendpacket_open(options->intf1_name, ebuf, TCPR_DIR_C2S, ctx->sp_type, ctx)) == NULL) {
        tcpreplay_seterr(ctx, "Can't open %s: %s", options->intf1_name, ebuf);
        ret = -1;
        goto out;
    }

#if defined HAVE_NETMAP
    ctx->intf1->netmap_delay = ctx->options->netmap_delay;
#endif

    ctx->intf1dlt = sendpacket_get_dlt(ctx->intf1);

    if (HAVE_OPT(INTF2)) {
        if (!HAVE_OPT(CACHEFILE) && !HAVE_OPT(DUALFILE)) {
            tcpreplay_seterr(ctx, "--intf2=%s requires either --cachefile or --dualfile", OPT_ARG(INTF2));
            ret = -1;
            goto out;
        }
        if ((intname = get_interface(ctx->intlist, OPT_ARG(INTF2))) == NULL) {
            tcpreplay_seterr(ctx, "Invalid interface name/alias: %s", OPT_ARG(INTF2));
            ret = -1;
            goto out;
        }

        options->intf2_name = safe_strdup(intname);

        /* open interface for writing */
        if ((ctx->intf2 = sendpacket_open(options->intf2_name, ebuf, TCPR_DIR_S2C, ctx->sp_type, ctx)) == NULL) {
            tcpreplay_seterr(ctx, "Can't open %s: %s", options->intf2_name, ebuf);
        }

#if defined HAVE_NETMAP
        ctx->intf2->netmap_delay = ctx->options->netmap_delay;
#endif

        ctx->intf2dlt = sendpacket_get_dlt(ctx->intf2);
        if (ctx->intf2dlt != ctx->intf1dlt) {
            tcpreplay_seterr(ctx, "DLT type mismatch for %s (%s) and %s (%s)",
                options->intf1_name, pcap_datalink_val_to_name(ctx->intf1dlt),
                options->intf2_name, pcap_datalink_val_to_name(ctx->intf2dlt));
            ret = -1;
            goto out;
        }
    }

    if (HAVE_OPT(CACHEFILE)) {
        temp = safe_strdup(OPT_ARG(CACHEFILE));
        options->cache_packets = read_cache(&options->cachedata, temp,
            &options->comment);
        safe_free(temp);
    }

    /* return -2 on warnings */
    if (warn > 0)
        ret = -2;

out:
    safe_free(ebuf);

    return ret;
}

/**
 * Closes & free's all memory related to a tcpreplay context
 */
void
tcpreplay_close(tcpreplay_t *ctx)
{
    tcpreplay_opt_t *options;
    interface_list_t *intlist, *intlistnext;
    packet_cache_t *packet_cache, *next;

    assert(ctx);
    assert(ctx->options);
    options = ctx->options;

    safe_free(options->intf1_name);
    safe_free(options->intf2_name);
    sendpacket_close(ctx->intf1);
    if (ctx->intf2 != NULL)
        sendpacket_close(ctx->intf2);
    safe_free(options->cachedata);
    safe_free(options->comment);

#ifdef ENABLE_VERBOSE
    safe_free(options->tcpdump_args);
    tcpdump_close(options->tcpdump);
#endif

    /* free the flow hash table */
    flow_hash_table_release(ctx->flow_hash_table);

    /* free the file cache */
    packet_cache = options->file_cache->packet_cache;
    while (packet_cache != NULL) {
        next = packet_cache->next;
        safe_free(packet_cache->pktdata);
        safe_free(packet_cache);
        packet_cache = next;
    }

    /* free our interface list */
    if (ctx->intlist != NULL) {
        intlist = ctx->intlist;
        while (intlist != NULL) {
            intlistnext = intlist->next;
            safe_free(intlist);
            intlist = intlistnext;
        }
    }
}

/**
 * \brief Specifies an interface to use for sending.
 *
 * You may call this up to two (2) times with different interfaces
 * when using a tcpprep cache file or dualfile mode.  Note, both interfaces
 * must use the same DLT type
 */
int
tcpreplay_set_interface(tcpreplay_t *ctx, tcpreplay_intf intf, char *value)
{
    static int int1dlt = -1, int2dlt = -1;
    char *intname;
    char *ebuf;
    int ret = 0;

    assert(ctx);
    assert(value);

    ebuf = safe_malloc(SENDPACKET_ERRBUF_SIZE);

    if (intf == intf1) {
        if ((intname = get_interface(ctx->intlist, value)) == NULL) {
            if (!strncmp(OPT_ARG(INTF1), "netmap:", 7) || !strncmp(OPT_ARG(INTF1), "vale", 4))
                tcpreplay_seterr(ctx, "Unable to connect to netmap interface %s. Ensure netmap module is installed (see INSTALL).",
                        value);
            else
                tcpreplay_seterr(ctx, "Invalid interface name/alias: %s", value);
            ret = -1;
            goto out;
        }

        ctx->options->intf1_name = safe_strdup(intname);

        /* open interfaces for writing */
        if ((ctx->intf1 = sendpacket_open(ctx->options->intf1_name, ebuf, TCPR_DIR_C2S, ctx->sp_type, ctx)) == NULL) {
            tcpreplay_seterr(ctx, "Can't open %s: %s", ctx->options->intf1_name, ebuf);
            ret = -1;
            goto out;
        }

        int1dlt = sendpacket_get_dlt(ctx->intf1);
    } else if (intf == intf2) {
        if ((intname = get_interface(ctx->intlist, value)) == NULL) {
            tcpreplay_seterr(ctx, "Invalid interface name/alias: %s", ctx->options->intf2_name);
            ret = -1;
            goto out;
        }

        ctx->options->intf2_name = safe_strdup(intname);

        /* open interface for writing */
        if ((ctx->intf2 = sendpacket_open(ctx->options->intf2_name, ebuf, TCPR_DIR_S2C, ctx->sp_type, ctx)) == NULL) {
            tcpreplay_seterr(ctx, "Can't open %s: %s", ctx->options->intf2_name, ebuf);
            ret = -1;
            goto out;
        }
        int2dlt = sendpacket_get_dlt(ctx->intf2);
    }

    /*
     * If both interfaces are selected, then make sure both interfaces use
     * the same DLT type
     */
    if (int1dlt != -1 && int2dlt != -1) {
        if (int1dlt != int2dlt) {
            tcpreplay_seterr(ctx, "DLT type mismatch for %s (%s) and %s (%s)",
                ctx->options->intf1_name, pcap_datalink_val_to_name(int1dlt), 
                ctx->options->intf2_name, pcap_datalink_val_to_name(int2dlt));
            ret = -1;
            goto out;
        }
    }

out:
    safe_free(ebuf);
    return ret;
}

/**
 * Set the replay speed mode.
 */
int
tcpreplay_set_speed_mode(tcpreplay_t *ctx, tcpreplay_speed_mode value)
{
    assert(ctx);

    ctx->options->speed.mode = value;
    return 0;
}

/**
 * Set the approprate speed value.  Value is interpreted based on 
 * how tcpreplay_set_speed_mode() value
 */
int
tcpreplay_set_speed_speed(tcpreplay_t *ctx, COUNTER value)
{
    assert(ctx);
    ctx->options->speed.speed = value;
    return 0;
}


/**
 * Sending under packets/sec requires an integer value, not float.
 * you must first call tcpreplay_set_speed_mode(ctx, speed_packetrate)
 */
int
tcpreplay_set_speed_pps_multi(tcpreplay_t *ctx, int value)
{
    assert(ctx);
    ctx->options->speed.pps_multi = value;
    return 0;
}

/**
 * How many times should we loop through all the pcap files?
 */
int
tcpreplay_set_loop(tcpreplay_t *ctx, u_int32_t value)
{
    assert(ctx);
    ctx->options->loop = value;
    return 0;
}

/**
 * Set the unique IP address flag
 */
int
tcpreplay_set_unique_ip(tcpreplay_t *ctx, bool value)
{
    assert(ctx);
    ctx->options->unique_ip = value;
    return 0;
}

int
tcpreplay_set_unique_ip_loops(tcpreplay_t *ctx, int value)
{
    assert(ctx);
    ctx->options->unique_loops = value;
    return 0;
}

/**
 * Set netmap mode
 */
int
tcpreplay_set_netmap(_U_ tcpreplay_t *ctx, _U_ bool value)
{
    assert(ctx);
#ifdef HAVE_NETMAP
    ctx->options->netmap = value;
    return 0;
#else
    warn("netmap not compiled in");
    return -1;
#endif
}

/**
 * Tell tcpreplay to ignore the snaplen (default) and use the "actual"
 * packet len instead
 */
int
tcpreplay_set_use_pkthdr_len(tcpreplay_t *ctx, bool value)
{
    assert(ctx);
    ctx->options->use_pkthdr_len = value;
    return 0;
}

/**
 * Override the outbound MTU
 */
int
tcpreplay_set_mtu(tcpreplay_t *ctx, int value)
{
    assert(ctx);
    ctx->options->mtu = value;
    return 0;
}

/**
 * Sets the accurate timing mode
 */
int
tcpreplay_set_accurate(tcpreplay_t *ctx, tcpreplay_accurate value)
{
    assert(ctx);
    ctx->options->accurate = value;
    return 0;
}

/**
 * Sets the number of seconds between printing stats
 */
int
tcpreplay_set_stats(tcpreplay_t *ctx, int value)
{
    assert(ctx);
    ctx->options->stats = value;
    return 0;
}

/**
 * \brief Enable or disable dual file mode
 *
 * In dual file mode, we read two files at the same time and use
 * one file for each interface.
 */

int 
tcpreplay_set_dualfile(tcpreplay_t *ctx, bool value)
{
    assert(ctx);
    ctx->options->dualfile = value;
    return 0;
}

/**
 * \brief Enable or disable preloading the file cache 
 *
 * Note: This is a global option and forces all pcaps
 * to be preloaded for this context.  If you turn this
 * on, then it forces set_file_cache(true)
 */
int
tcpreplay_set_preload_pcap(tcpreplay_t *ctx, bool value)
{
    assert(ctx);
    ctx->options->preload_pcap = value;
    return 0;
}

/**
 * \brief Add a pcap file to be sent via tcpreplay
 *
 * One or more pcap files can be added.  Each file will be replayed
 * in order
 */
int
tcpreplay_add_pcapfile(tcpreplay_t *ctx, char *pcap_file)
{
    assert(ctx);
    assert(pcap_file);

    if (ctx->options->source_cnt < MAX_FILES) {
        ctx->options->sources[ctx->options->source_cnt].filename = safe_strdup(pcap_file);
        ctx->options->sources[ctx->options->source_cnt].type = source_filename;

        /*
         * prepare the cache info data struct.  This doesn't actually enable
         * file caching for this pcap (that is controlled globally via
         * tcpreplay_set_file_cache())
         */
        ctx->options->file_cache[ctx->options->source_cnt].index = ctx->options->source_cnt;
        ctx->options->file_cache[ctx->options->source_cnt].cached = false;
        ctx->options->file_cache[ctx->options->source_cnt].packet_cache = NULL;

        ctx->options->source_cnt += 1;


    } else {
        tcpreplay_seterr(ctx, "Unable to add more then %u files", MAX_FILES);
        return -1;
    }
    return 0;
}

/**
 * Limit the total number of packets to send
 */
int
tcpreplay_set_limit_send(tcpreplay_t *ctx, COUNTER value)
{
    assert(ctx);
    ctx->options->limit_send = value;
    return 0;
}

/**
 * \brief Specify the tcpprep cache file to use for replaying with two NICs
 *
 * Note: this only works if you have a single pcap file
 * returns -1 on error
 */
int
tcpreplay_set_tcpprep_cache(tcpreplay_t *ctx, char *file)
{
    assert(ctx);
    char *tcpprep_file;

    if (ctx->options->source_cnt > 1) {
        tcpreplay_seterr(ctx, "%s", "Unable to use tcpprep cache file with a single pcap file");
        return -1;
    }

    tcpprep_file = safe_strdup(file);
    ctx->options->cache_packets = read_cache(&ctx->options->cachedata, 
        tcpprep_file, &ctx->options->comment);

    free(tcpprep_file);

    return 0;
}



/*
 * Verbose mode requires fork() and tcpdump binary, hence won't work
 * under Win32 without Cygwin
 */

/**
 * Enable verbose mode
 */
int
tcpreplay_set_verbose(tcpreplay_t *ctx, bool value _U_)
{
    assert(ctx);
#ifdef ENABLE_VERBOSE
    ctx->options->verbose = value;
    return 0;
#else
    tcpreplay_seterr(ctx, "%s", "verbose mode not supported");
    return -1;
#endif
}

/**
 * \brief Set the arguments to be passed to tcpdump
 *
 * Specify the additional argument to be passed to tcpdump when enabling
 * verbose mode.  See TCPDUMP_ARGS in tcpdump.h for the default options
 */
int
tcpreplay_set_tcpdump_args(tcpreplay_t *ctx, char *value _U_)
{
    assert(ctx);
#ifdef ENABLE_VERBOSE
    assert(value);
    ctx->options->tcpdump_args = safe_strdup(value);
    return 0;
#else
    tcpreplay_seterr(ctx, "%s", "verbose mode not supported");
    return -1;
#endif
}

/**
 * \brief Set the path to the tcpdump binary
 *
 * In order to support the verbose feature, tcpreplay needs to know where
 * tcpdump lives
 */
int
tcpreplay_set_tcpdump(tcpreplay_t *ctx, tcpdump_t *value _U_)
{
    assert(ctx);
#ifdef ENABLE_VERBOSE
    assert(value);
    ctx->options->verbose = true;
    ctx->options->tcpdump = value;
    return 0;
#else
    tcpreplay_seterr(ctx, "%s", "verbose mode not supported");
    return -1;
#endif
}


/**
 * \brief Set the callback function for handing manual iteration
 *
 * Obviously for this to work, you need to first set speed_mode = speed_oneatatime
 * returns 0 on success, < 0 on error
 */
int
tcpreplay_set_manual_callback(tcpreplay_t *ctx, tcpreplay_manual_callback callback)
{
    assert(ctx);
    assert(callback);

    if (ctx->options->speed.mode != speed_oneatatime) {
        tcpreplay_seterr(ctx, "%s", 
                "Unable to set manual callback because speed mode is not 'speed_oneatatime'");
        return -1;
    }

    ctx->options->speed.manual_callback = callback;
    return 0;
}

/**
 * \brief return the number of packets sent so far
 */
COUNTER
tcpreplay_get_pkts_sent(tcpreplay_t *ctx)
{
    assert(ctx);

    ctx->static_stats.pkts_sent = ctx->stats.pkts_sent;
    return ctx->static_stats.pkts_sent;
}

/**
 * \brief return the number of bytes sent so far
 */
COUNTER
tcpreplay_get_bytes_sent(tcpreplay_t *ctx)
{
    assert(ctx);
    ctx->static_stats.bytes_sent = ctx->stats.bytes_sent;
    return ctx->static_stats.bytes_sent;
}

/**
 * \brief return the number of failed attempts to send a packet
 */
COUNTER
tcpreplay_get_failed(tcpreplay_t *ctx)
{
    assert(ctx);
    ctx->static_stats.failed = ctx->stats.failed;
    return ctx->static_stats.failed;
}

/**
 * \brief returns a pointer to the timeval structure of when replay first started
 */
const struct timeval *
tcpreplay_get_start_time(tcpreplay_t *ctx)
{
    assert(ctx);
    TIMEVAL_SET(&ctx->static_stats.start_time, &ctx->stats.start_time);
    return &ctx->static_stats.start_time;
}

/**
 * \brief returns a pointer to the timeval structure of when replay finished
 */
const struct timeval *
tcpreplay_get_end_time(tcpreplay_t *ctx)
{
    assert(ctx);
    TIMEVAL_SET(&ctx->static_stats.end_time, &ctx->stats.end_time);
    return &ctx->static_stats.end_time;
}


/**
 * \brief Internal function to set the tcpreplay error string
 *
 * Used to set the error string when there is an error, result is retrieved
 * using tcpedit_geterr().  You shouldn't ever actually call this, but use
 * tcpreplay_seterr() which is a macro wrapping this instead.
 */
void
__tcpreplay_seterr(tcpreplay_t *ctx, const char *func,
        const int line, const char *file, const char *fmt, ...)
{
    va_list ap;
    char errormsg[TCPREPLAY_ERRSTR_LEN - 32];

    assert(ctx);
    assert(file);
    assert(func);
    assert(line);

    va_start(ap, fmt);
    if (fmt != NULL)
        (void)vsnprintf(errormsg, sizeof(errormsg), fmt, ap);

    va_end(ap);

#ifdef DEBUG
    snprintf(ctx->errstr, sizeof(ctx->errstr), "From %s:%s() line %d:\n%s",
        file, func, line, errormsg);
#else
    snprintf(ctx->errstr, sizeof(ctx->errstr), "%s", errormsg);
#endif
}

/**
 * \brief Internal function to set the tcpedit warning string
 *
 * Used to set the warning string when there is an non-fatal issue, result is retrieved
 * using tcpedit_getwarn().
 */
void
tcpreplay_setwarn(tcpreplay_t *ctx, const char *fmt, ...)
{
    va_list ap;
    assert(ctx);

    va_start(ap, fmt);
    if (fmt != NULL)
        (void)vsnprintf(ctx->warnstr, sizeof(ctx->warnstr), fmt, ap);

    va_end(ap);
}


/**
 * \brief Does all the prep work before calling tcpreplay_replay()
 *
 * Technically this validates our config options, preloads the tcpprep
 * cache file, loads the packet cache and anything else which might
 * cause a delay for starting to send packets with tcpreplay_replay()
 */
int 
tcpreplay_prepare(tcpreplay_t *ctx)
{
    char *intname, *ebuf;
    int int1dlt, int2dlt, i;
    int ret = 0;

    assert(ctx);

    ebuf = safe_malloc(SENDPACKET_ERRBUF_SIZE);

    /*
     * First, process the validations, basically the same we do in 
     * tcpreplay_post_args() and AutoOpts
     */
    if (ctx->options->intf1_name == NULL) {
        tcpreplay_seterr(ctx, "%s", "You must specify at least one network interface");
        ret = -1;
        goto out;
    }

    if (ctx->options->source_cnt == 0) {
        tcpreplay_seterr(ctx, "%s", "You must specify at least one source pcap");
        ret = -1;
        goto out;
    }

    if (ctx->options->dualfile) {
        if (!(ctx->options->source_cnt >= 2)) {
            tcpreplay_seterr(ctx, "%s", "Dual file mode requires 2 or more pcap files");
            ret = -1;
            goto out;
        }

        if (ctx->options->source_cnt % 2 != 0) {
            tcpreplay_seterr(ctx, "%s", "Dual file mode requires an even number of pcap files");
            ret = -1;
            goto out;
        }
    }

    if (ctx->options->dualfile && ctx->options->cachedata != NULL) {
        tcpreplay_seterr(ctx, "%s", "Can't use dual file mode and tcpprep cache file together");
        ret = -1;
        goto out;
    }

    if ((ctx->options->dualfile || ctx->options->cachedata != NULL) && 
           ctx->options->intf2_name == NULL) {
        tcpreplay_seterr(ctx, "%s", "dual file mode and tcpprep cache files require two interfaces");
    }


#ifndef HAVE_SELECT
    if (ctx->options->accurate == accurate_select) {
        tcpreplay_seterr(ctx, "%s", "tcpreplay_api not compiled with select support");
        ret = -1;
        goto out;
    }
#endif

    if ((intname = get_interface(ctx->intlist, ctx->options->intf1_name)) == NULL) {
        if (!strncmp(OPT_ARG(INTF1), "netmap:", 7) || !strncmp(OPT_ARG(INTF1), "vale", 4))
            tcpreplay_seterr(ctx, "Unable to connect to netmap interface %s. Ensure netmap module is installed (see INSTALL).",
                    OPT_ARG(INTF1));
        else
            tcpreplay_seterr(ctx, "Invalid interface name/alias: %s", OPT_ARG(INTF1));

        ret = -1;
        goto out;
    }

    /* open interfaces for writing */
    if ((ctx->intf1 = sendpacket_open(ctx->options->intf1_name, ebuf, TCPR_DIR_C2S, ctx->sp_type, ctx)) == NULL) {
        tcpreplay_seterr(ctx, "Can't open %s: %s", ctx->options->intf1_name, ebuf);
        ret = -1;
        goto out;
    }

    int1dlt = sendpacket_get_dlt(ctx->intf1);

    if (ctx->options->intf2_name != NULL) {
        if ((intname = get_interface(ctx->intlist, ctx->options->intf2_name)) == NULL) {
            tcpreplay_seterr(ctx, "Invalid interface name/alias: %s", OPT_ARG(INTF2));
            ret = -1;
            goto out;
        }

        /* open interfaces for writing */
        if ((ctx->intf2 = sendpacket_open(ctx->options->intf2_name, ebuf, TCPR_DIR_C2S, ctx->sp_type, ctx)) == NULL) {
            tcpreplay_seterr(ctx, "Can't open %s: %s", ctx->options->intf2_name, ebuf);
            ret = -1;
            goto out;
        }

        int2dlt = sendpacket_get_dlt(ctx->intf2);
        if (int2dlt != int1dlt) {
            tcpreplay_seterr(ctx, "DLT type mismatch for %s (%s) and %s (%s)",
                ctx->options->intf1_name, pcap_datalink_val_to_name(int1dlt), 
                ctx->options->intf2_name, pcap_datalink_val_to_name(int2dlt));
            ret = -1;
            goto out;
        }
    }

    /*
     * Setup up the file cache, if required
     */
    if (ctx->options->preload_pcap) {
        /* Initialise each of the file cache structures */
        for (i = 0; i < ctx->options->source_cnt; i++) {
            ctx->options->file_cache[i].index = i;
            ctx->options->file_cache[i].cached = FALSE;
            ctx->options->file_cache[i].packet_cache = NULL;
        }
    }

out:
    safe_free(ebuf);
    return ret;
}

/**
 * \brief sends the traffic out the interfaces
 *
 * Designed to be called in a separate thread if you need to.  Blocks until
 * the replay is complete or you call tcpreplay_abort() in another thread.
 */
int
tcpreplay_replay(tcpreplay_t *ctx)
{
    int rcode;
    COUNTER loop, total_loops;

    assert(ctx);

    if (!ctx->options->source_cnt) {
        tcpreplay_seterr(ctx, "invalid source count: %d", ctx->options->source_cnt);
        return -1;
    }

    if (ctx->options->dualfile && ctx->options->source_cnt < 2) {
        tcpreplay_seterr(ctx, "invalid dualfile source count: %d", ctx->options->source_cnt);
        return -1;
    }

    init_timestamp(&ctx->stats.start_time);
    init_timestamp(&ctx->stats.time_delta);
    init_timestamp(&ctx->stats.end_time);
    init_timestamp(&ctx->stats.pkt_ts_delta);
    init_timestamp(&ctx->stats.last_print);

    ctx->running = true;
    total_loops = ctx->options->loop;
    loop = 0;

    /* main loop, when not looping forever (or until abort) */
    if (ctx->options->loop > 0) {
        while (ctx->options->loop-- && !ctx->abort) {  /* limited loop */
            ++loop;
            if (ctx->options->stats == 0) {
                if (!ctx->unique_iteration || loop == ctx->unique_iteration)
                    printf("Loop " COUNTER_SPEC " of " COUNTER_SPEC "...\n",
                            loop, total_loops);
                else
                    printf("Loop " COUNTER_SPEC " of " COUNTER_SPEC " (" COUNTER_SPEC " unique)...\n",
                            loop, total_loops,
                            ctx->unique_iteration);
            }
            if ((rcode = tcpr_replay_index(ctx)) < 0)
                return rcode;
            if (ctx->options->loop > 0) {
                if (!ctx->abort && ctx->options->loopdelay_ms > 0) {
                    usleep(ctx->options->loopdelay_ms * 1000);
                    gettimeofday(&ctx->stats.end_time, NULL);
                }

                if (ctx->options->stats == 0)
                    packet_stats(&ctx->stats);
            }
        }
    } else {
        while (!ctx->abort) { /* loop forever unless user aborts */
            ++loop;
            if (ctx->options->stats == 0) {
                if (!ctx->unique_iteration || loop == ctx->unique_iteration)
                    printf("Loop " COUNTER_SPEC "...\n", loop);
                else
                    printf("Loop " COUNTER_SPEC " (" COUNTER_SPEC " unique)...\n", loop,
                            ctx->unique_iteration);
            }
            if ((rcode = tcpr_replay_index(ctx)) < 0)
                return rcode;

            if (!ctx->abort && ctx->options->loopdelay_ms > 0) {
                usleep(ctx->options->loopdelay_ms * 1000);
                gettimeofday(&ctx->stats.end_time, NULL);
            }

            if (ctx->options->stats == 0 && !ctx->abort)
                packet_stats(&ctx->stats);
        }
    }

    ctx->running = false;

    if (ctx->options->stats >= 0) {
        char buf[64];

        if (format_date_time(&ctx->stats.end_time, buf, sizeof(buf)) > 0)
            printf("Test complete: %s\n", buf);
    }

    return 0;
}

/**
 * \brief Abort the tcpreplay_replay execution.
 *
 * This might take a little while since tcpreplay_replay() only checks this
 * once per packet (sleeping between packets can cause delays), however, 
 * this function returns once the signal has been sent and does not block
 */
int
tcpreplay_abort(tcpreplay_t *ctx)
{
    assert(ctx);
    ctx->abort = true;

    printf("sendpacket_abort\n");

    if (ctx->intf1 != NULL)
        sendpacket_abort(ctx->intf1);

    if (ctx->intf2 != NULL)
        sendpacket_abort(ctx->intf2);

    return 0;
}

/**
 * \brief Temporarily suspend tcpreplay_replay()
 *
 * This might take a little while since tcpreplay_replay() only checks this
 * once per packet (sleeping between packets can cause delays), however, 
 * this function returns once the signal has been sent and does not block 
 *
 * Note that suspending a running context can create odd timing 
 */
int
tcpreplay_suspend(tcpreplay_t *ctx)
{
    assert(ctx);
    ctx->suspend = true;
    return 0;
}

/**
 * \brief Restart tcpreplay_replay() after suspend
 *
 * Causes the worker thread to restart sending packets
 */
int
tcpreplay_restart(tcpreplay_t *ctx)
{
    assert(ctx);
    ctx->suspend = false;
    return 0;
}

/**
 * \brief Tells you if the given tcpreplay context is currently suspended
 *
 * Suspended == running, but not sending packets
 */
bool
tcpreplay_is_suspended(tcpreplay_t *ctx)
{
    assert(ctx);
    return ctx->suspend;
}

/**
 * \brief Tells you if the tcpreplay context is running (not yet finished)
 *
 * Returns true even if it is suspended
 */
bool 
tcpreplay_is_running(tcpreplay_t *ctx)
{
    assert(ctx);
    return ctx->running;
}

/**
 * \brief returns the current statistics during or after a replay
 *
 * For performance reasons, I don't bother to put a mutex around this and you
 * don't need to either.  Just realize that your values may be off by one until
 * tcreplay_replay() returns.
 */
const tcpreplay_stats_t *
tcpreplay_get_stats(tcpreplay_t *ctx)
{
    const tcpreplay_stats_t *ptr;

    assert(ctx);

    /* copy stats over so they don't change while caller is using the buffer */
    memcpy(&ctx->static_stats, &ctx->stats, sizeof(tcpreplay_stats_t));
    ptr = &ctx->static_stats;
    return ptr;
}


/**
 * \brief returns the current number of sources/files to be sent
 */
int
tcpreplay_get_source_count(tcpreplay_t *ctx)
{
    assert(ctx);
    return ctx->options->source_cnt;
}

/**
 * \brief Returns the current source id being replayed
 */
int
tcpreplay_get_current_source(tcpreplay_t *ctx)
{
    assert(ctx);
    return ctx->current_source;
}

/* vim: set tabstop=8 expandtab shiftwidth=4 softtabstop=4: */


/**
 * \brief Sets printing of flow statistics
 */
int tcpreplay_set_flow_stats(tcpreplay_t *ctx, bool value)
{
    assert(ctx);

    ctx->options->flow_stats = value;
    return 0;
}

/**
 * \brief Sets the flow expiry in seconds
 */
int tcpreplay_set_flow_expiry(tcpreplay_t *ctx, int value)
{
    assert(ctx);

    ctx->options->flow_expiry = value;
    return 0;
}

/**
 * \brief Get whether to printof flow statistics
 */
bool tcpreplay_get_flow_stats(tcpreplay_t *ctx)
{
    assert(ctx);

    return ctx->options->flow_stats;
}

/**
 * \brief Gets the flow expiry in seconds
 */
int tcpreplay_get_flow_expiry(tcpreplay_t *ctx)
{
    assert(ctx);

    return ctx->options->flow_expiry;
}
