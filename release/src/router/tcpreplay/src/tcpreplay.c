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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_FTS_H
#include <fts.h>
#endif
#include <errno.h>

#include "tcpreplay.h"
#include "tcpreplay_api.h"

#ifdef TCPREPLAY_EDIT
#include "tcpreplay_edit_opts.h"
#include "tcpedit/tcpedit.h"
#include "tcpedit/fuzzing.h"
tcpedit_t *tcpedit;
#else
#include "tcpreplay_opts.h"
#endif

#include "send_packets.h"
#include "signal_handler.h"

#ifdef DEBUG
int debug = 0;
#endif

tcpreplay_t *ctx;

static void flow_stats(const tcpreplay_t *tcpr_ctx);

int
main(int argc, char *argv[])
{
    int i, optct;
    int rcode;

    fflush(NULL);

    ctx = tcpreplay_init();
    optct = optionProcess(&tcpreplayOptions, argc, argv);
    argc -= optct;
    argv += optct;

    fflush(NULL);
    rcode = tcpreplay_post_args(ctx, argc);
    if (rcode <= -2) {
        warnx("%s", tcpreplay_getwarn(ctx));
    } else if (rcode == -1) {
        errx(-1, "Unable to parse args: %s", tcpreplay_geterr(ctx));
    }

    fflush(NULL);
#ifdef TCPREPLAY_EDIT
    /* init tcpedit context */
    if (tcpedit_init(&tcpedit, sendpacket_get_dlt(ctx->intf1)) < 0) {
        errx(-1, "Error initializing tcpedit: %s", tcpedit_geterr(tcpedit));
    }

    /* parse the tcpedit args */
    rcode = tcpedit_post_args(tcpedit);
    if (rcode < 0) {
        tcpedit_close(&tcpedit);
        errx(-1, "Unable to parse args: %s", tcpedit_geterr(tcpedit));
    } else if (rcode == 1) {
        warnx("%s", tcpedit_geterr(tcpedit));
    }

    if (tcpedit_validate(tcpedit) < 0) {
        tcpedit_close(&tcpedit);
        errx(-1, "Unable to edit packets given options:\n%s",
               tcpedit_geterr(tcpedit));
    }
#endif

    if (ctx->options->preload_pcap && ! HAVE_OPT(QUIET)) {
        notice("File Cache is enabled");
    }

   /*
    * Check if remaining args are directories or files
    */
    for (i = 0; i < argc; i++) {
#ifdef HAVE_FTS_H
        struct stat statbuf;

        if (!strcmp(argv[i], "-")) {
            tcpreplay_add_pcapfile(ctx, argv[i]);
            continue;
        }

        if (stat(argv[i], &statbuf) != 0) {
            errx(-1,
                 "Unable to retrieve information from file %s: %s",
                 argv[i],
                 strerror(errno));
        }

        /* If it is a directory, walk the file tree and treat only pcap files */
        if (S_ISDIR(statbuf.st_mode)) {
            FTSENT *entry = NULL;
            FTS *fts = fts_open(&argv[i], FTS_NOCHDIR | FTS_LOGICAL, NULL);
            if (fts == NULL) {
                errx(-1, "Unable to open %s", argv[1]);
            }

            while ((entry = fts_read(fts)) != NULL) {
                switch (entry->fts_info) {
                case FTS_F:
                    if (entry->fts_path) {
                        tcpreplay_add_pcapfile(ctx, entry->fts_path);
                    }
                    break;
                default:
                    break;
                }
            }

            fts_close(fts);
        } else {
            tcpreplay_add_pcapfile(ctx, argv[i]);
        }
#else
        tcpreplay_add_pcapfile(ctx, argv[i]);
#endif
    }

    /*
     * Setup up the file cache, if required
     */
    if (ctx->options->preload_pcap) {
        /* Initialize each of the file cache structures */
        for (i = 0; i < ctx->options->source_cnt; i++) {
            ctx->options->file_cache[i].index = i;
            ctx->options->file_cache[i].cached = FALSE;
            ctx->options->file_cache[i].packet_cache = NULL;
            /* preload our pcap file */
            preload_pcap_file(ctx, i);
        }
    }

#ifdef TCPREPLAY_EDIT
    /* fuzzing init */
    fuzzing_init(tcpedit->fuzz_seed, tcpedit->fuzz_factor);
#endif

    /* init the signal handlers */
    init_signal_handlers();

    /* main loop */
    rcode = tcpreplay_replay(ctx);

    if (rcode < 0) {
        notice("\nFailed: %s\n", tcpreplay_geterr(ctx));
#ifdef TCPREPLAY_EDIT
        tcpedit_close(&tcpedit);
#endif
        exit(rcode);
    } else if (rcode == 1) {
        notice("\nWarning: %s\n", tcpreplay_getwarn(ctx));
    }

    if (ctx->stats.bytes_sent > 0) {
        char buf[1024];

        packet_stats(&ctx->stats);
        if (ctx->options->flow_stats)
            flow_stats(ctx);
        sendpacket_getstat(ctx->intf1, buf, sizeof(buf));
        printf("%s", buf);
        if (ctx->intf2 != NULL) {
            sendpacket_getstat(ctx->intf2, buf, sizeof(buf));
            printf("%s", buf);
        }
    }

#ifdef TIMESTAMP_TRACE
    dump_timestamp_trace_array(&ctx->stats.start_time, &ctx->stats.end_time,
            ctx->options->speed.speed);
#endif
#ifdef TCPREPLAY_EDIT
    tcpedit_close(&tcpedit);
#endif
    tcpreplay_close(ctx);
    restore_stdin();
    return 0;
}   /* main() */

/**
 * Print various flow statistics
 */
static void flow_stats(const tcpreplay_t *tcpr_ctx)
{
    struct timeval diff;
    COUNTER diff_us;
    const tcpreplay_stats_t *stats = &tcpr_ctx->stats;
    const tcpreplay_opt_t *options = tcpr_ctx->options;
    COUNTER flows_total = stats->flows;
    COUNTER flows_unique = stats->flows_unique;
    COUNTER flows_expired = stats->flows_expired;
    COUNTER flow_packets = stats->flow_packets;
    COUNTER flow_non_flow_packets = stats->flow_non_flow_packets;
    COUNTER flows_sec = 0;
    u_int32_t flows_sec_100ths = 0;

    timersub(&stats->end_time, &stats->start_time, &diff);
    diff_us = TIMEVAL_TO_MICROSEC(&diff);

    if (!flows_total || !tcpr_ctx->iteration)
        return;

    /*
     * When packets are read into cache, flows
     * are only counted in first iteration
     * If flows are unique from one loop iteration
     * to the next then multiply by the number of
     * successful iterations.
     */
    if (options->preload_pcap && tcpr_ctx->last_unique_iteration) {
        flows_total *= tcpr_ctx->last_unique_iteration;
        flows_unique *= tcpr_ctx->last_unique_iteration;
        flows_expired *= tcpr_ctx->last_unique_iteration;
        flow_packets *= tcpr_ctx->last_unique_iteration;
        flow_non_flow_packets *= tcpr_ctx->last_unique_iteration;
    } else {
        /* adjust for --unique-ip-loops */
        flow_packets = (flow_packets * (tcpr_ctx->last_unique_iteration ?: tcpr_ctx->iteration)) / tcpr_ctx->iteration;
    }

#ifdef TCPREPLAY_EDIT
    if (tcpedit->seed) {
        flow_non_flow_packets *= tcpr_ctx->iteration;
        flows_total *= tcpr_ctx->iteration;
        flows_unique *= tcpr_ctx->iteration;
        flows_expired *= tcpr_ctx->iteration;
    }
#endif

    if (diff_us) {
        COUNTER flows_sec_X100;

        flows_sec_X100 = (flows_total * 100 * 1000 * 1000) / diff_us;
        flows_sec = flows_sec_X100 / 100;
        flows_sec_100ths = flows_sec_X100 % 100;
    }

    if (tcpr_ctx->options->flow_expiry)
        printf("Flows: " COUNTER_SPEC " flows, " COUNTER_SPEC " unique, "COUNTER_SPEC " expired, %llu.%02u fps, " COUNTER_SPEC " unique flow packets, " COUNTER_SPEC " unique non-flow packets\n",
                flows_total, flows_unique, flows_expired, flows_sec, flows_sec_100ths, flow_packets,
                flow_non_flow_packets);
    else
        printf("Flows: " COUNTER_SPEC " flows, %llu.%02u fps, " COUNTER_SPEC " unique flow packets, " COUNTER_SPEC " unique non-flow packets\n",
                flows_total, flows_sec, flows_sec_100ths, flow_packets,
                flow_non_flow_packets);
}

/* vim: set tabstop=8 expandtab shiftwidth=4 softtabstop=4: */
