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

/*
 * Purpose: Modify packets in a pcap file based on rules provided by the
 * user to offload work from tcpreplay and provide an easier means of
 * reproducing traffic for testing purposes.
 */

#include "tcprewrite.h"
#include "config.h"
#include "common.h"
#include "tcpedit/fuzzing.h"
#include "tcpedit/tcpedit.h"
#include "tcprewrite_opts.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef DEBUG
int debug;
#endif

#ifdef ENABLE_VERBOSE
/* tcpdump handle */
tcpdump_t tcpdump;
#endif

tcprewrite_opt_t options;
tcpedit_t *tcpedit;

/* local functions */
void tcprewrite_init(void);
void post_args(int argc, char *argv[]);
int rewrite_packets(tcpedit_t *tcpedit_ctx, pcap_t *pin, pcap_dumper_t *pout);

int
main(int argc, char *argv[])
{
    int optct, rcode;
    pcap_t *dlt_pcap;
#ifdef ENABLE_FRAGROUTE
    char ebuf[FRAGROUTE_ERRBUF_LEN];
#endif
    tcprewrite_init();

    /* call autoopts to process arguments */
    optct = optionProcess(&tcprewriteOptions, argc, argv);
    argc -= optct;
    argv += optct;

    /* parse the tcprewrite args */
    post_args(argc, argv);

    /* init tcpedit context */
    if (tcpedit_init(&tcpedit, pcap_datalink(options.pin)) < 0) {
        err_no_exitx("Error initializing tcpedit: %s", tcpedit_geterr(tcpedit));
        tcpedit_close(&tcpedit);
        exit(-1);
    }

    /* parse the tcpedit args */
    rcode = tcpedit_post_args(tcpedit);
    if (rcode < 0) {
        err_no_exitx("Unable to parse args: %s", tcpedit_geterr(tcpedit));
        tcpedit_close(&tcpedit);
        exit(-1);
    } else if (rcode == 1) {
        warnx("%s", tcpedit_geterr(tcpedit));
    }

    if (tcpedit_validate(tcpedit) < 0) {
        err_no_exitx("Unable to edit packets given options:\n%s", tcpedit_geterr(tcpedit));
        tcpedit_close(&tcpedit);
        exit(-1);
    }

    /* fuzzing init */
    fuzzing_init(tcpedit->fuzz_seed, tcpedit->fuzz_factor);

    /* open up the output file */
    options.outfile = safe_strdup(OPT_ARG(OUTFILE));
    dbgx(1, "Rewriting DLT to %s", pcap_datalink_val_to_name(tcpedit_get_output_dlt(tcpedit)));
    if ((dlt_pcap = pcap_open_dead(tcpedit_get_output_dlt(tcpedit), 65535)) == NULL) {
        tcpedit_close(&tcpedit);
        err(-1, "Unable to open dead pcap handle.");
    }

    dbgx(1, "DLT of dlt_pcap is %s", pcap_datalink_val_to_name(pcap_datalink(dlt_pcap)));

#ifdef ENABLE_FRAGROUTE
    if (options.fragroute_args) {
        if ((options.frag_ctx = fragroute_init(65535, pcap_datalink(dlt_pcap), options.fragroute_args, ebuf)) == NULL) {
            err_no_exitx("%s", ebuf);
            tcpedit_close(&tcpedit);
            exit(-1);
        }
    }
#endif

#ifdef ENABLE_VERBOSE
    if (options.verbose) {
        tcpdump_open(&tcpdump, dlt_pcap);
    }
#endif

    if ((options.pout = pcap_dump_open(dlt_pcap, options.outfile)) == NULL) {
        err_no_exitx("Unable to open output pcap file: %s", pcap_geterr(dlt_pcap));
        tcpedit_close(&tcpedit);
        exit(-1);
    }

    pcap_close(dlt_pcap);

    /* rewrite packets */
    if (rewrite_packets(tcpedit, options.pin, options.pout) == TCPEDIT_ERROR) {
        err_no_exitx("Error rewriting packets: %s", tcpedit_geterr(tcpedit));
        tcpedit_close(&tcpedit);
        exit(-1);
    }

    /* clean up after ourselves */
    pcap_dump_close(options.pout);
    pcap_close(options.pin);
    tcpedit_close(&tcpedit);

#ifdef ENABLE_VERBOSE
    tcpdump_close(&tcpdump);
#endif

#ifdef ENABLE_FRAGROUTE
    if (options.frag_ctx) {
        fragroute_close(options.frag_ctx);
    }
#endif

#ifdef ENABLE_DMALLOC
    dmalloc_shutdown();
#endif

    restore_stdin();
    return 0;
}

void
tcprewrite_init(void)
{
    memset(&options, 0, sizeof(options));

#ifdef ENABLE_VERBOSE
    /* clear out tcpdump struct */
    memset(&tcpdump, '\0', sizeof(tcpdump_t));
#endif

    if (fcntl(STDERR_FILENO, F_SETFL, O_NONBLOCK) < 0)
        warnx("Unable to set STDERR to non-blocking: %s", strerror(errno));
}

/**
 * post AutoGen argument processing
 */
void
post_args(_U_ int argc, _U_ char *argv[])
{
    char ebuf[PCAP_ERRBUF_SIZE];

#ifdef DEBUG
    if (HAVE_OPT(DBUG))
        debug = OPT_VALUE_DBUG;
#else
    if (HAVE_OPT(DBUG))
        warn("not configured with --enable-debug.  Debugging disabled.");
#endif

#ifdef ENABLE_VERBOSE
    if (HAVE_OPT(VERBOSE))
        options.verbose = 1;

    if (HAVE_OPT(DECODE))
        tcpdump.args = safe_strdup(OPT_ARG(DECODE));
#endif

#ifdef ENABLE_FRAGROUTE
    if (HAVE_OPT(FRAGROUTE))
        options.fragroute_args = safe_strdup(OPT_ARG(FRAGROUTE));

    options.fragroute_dir = FRAGROUTE_DIR_BOTH;
    if (HAVE_OPT(FRAGDIR)) {
        if (strcmp(OPT_ARG(FRAGDIR), "c2s") == 0) {
            options.fragroute_dir = FRAGROUTE_DIR_C2S;
        } else if (strcmp(OPT_ARG(FRAGDIR), "s2c") == 0) {
            options.fragroute_dir = FRAGROUTE_DIR_S2C;
        } else if (strcmp(OPT_ARG(FRAGDIR), "both") == 0) {
            options.fragroute_dir = FRAGROUTE_DIR_BOTH;
        } else {
            errx(-1, "Unknown --fragdir value: %s", OPT_ARG(FRAGDIR));
        }
    }
#endif

    /* open up the input file */
    options.infile = safe_strdup(OPT_ARG(INFILE));
    if ((options.pin = pcap_open_offline(options.infile, ebuf)) == NULL)
        errx(-1, "Unable to open input pcap file: %s", ebuf);

#ifdef HAVE_PCAP_SNAPSHOT
    if (pcap_snapshot(options.pin) < 65535)
        warnx("%s was captured using a snaplen of %d bytes.  This may mean you have truncated packets.",
              options.infile,
              pcap_snapshot(options.pin));
#endif
}

/**
 * Main loop to rewrite packets
 */
int
rewrite_packets(tcpedit_t *tcpedit_ctx, pcap_t *pin, pcap_dumper_t *pout)
{
    tcpr_dir_t cache_result = TCPR_DIR_C2S; /* default to primary */
    struct pcap_pkthdr pkthdr, *pkthdr_ptr; /* packet header */
    const u_char *pktconst = NULL;          /* packet from libpcap */
    u_char **pktdata = NULL;
    static u_char *pktdata_buff;
    static char *frag = NULL;
    COUNTER packetnum = 0;
    int rcode;
#ifdef ENABLE_FRAGROUTE
    int frag_len, proto;
#endif

    pkthdr_ptr = &pkthdr;

    if (pktdata_buff == NULL)
        pktdata_buff = (u_char *)safe_malloc(MAXPACKET);

    pktdata = &pktdata_buff;

    if (frag == NULL)
        frag = (char *)safe_malloc(MAXPACKET);

    /* MAIN LOOP
     * Keep sending while we have packets or until
     * we've sent enough packets
     */
    while ((pktconst = safe_pcap_next(pin, pkthdr_ptr)) != NULL) {
        packetnum++;
        dbgx(2, "packet " COUNTER_SPEC " caplen %d", packetnum, pkthdr.caplen);

        if (pkthdr.caplen > MAX_SNAPLEN)
            errx(-1, "Frame too big, caplen %d exceeds %d", pkthdr.caplen, MAX_SNAPLEN);
        /*
         * copy over the packet so we can pad it out if necessary and
         * because pcap_next() returns a const ptr
         */
        memcpy(*pktdata, pktconst, pkthdr.caplen);

        /* Dual nic processing? */
        if (options.cachedata != NULL) {
            cache_result = check_cache(options.cachedata, packetnum);
        }

        /* sometimes we should not send the packet, in such cases
         * no point in editing this packet at all, just write it to the
         * output file (note, we can't just remove it, or the tcpprep cache
         * file will lose it's indexing
         */

        if (cache_result == TCPR_DIR_NOSEND)
            goto WRITE_PACKET; /* still need to write it so cache stays in sync */

        if ((rcode = tcpedit_packet(tcpedit_ctx, &pkthdr_ptr, pktdata, cache_result)) == TCPEDIT_ERROR) {
            return rcode;
        } else if ((rcode == TCPEDIT_SOFT_ERROR) && HAVE_OPT(SKIP_SOFT_ERRORS)) {
            /* don't write packet */
            dbgx(1, "Packet " COUNTER_SPEC " is suppressed from being written due to soft errors", packetnum);
            continue;
        }

#ifdef ENABLE_VERBOSE
        if (options.verbose)
            tcpdump_print(&tcpdump, pkthdr_ptr, *pktdata);
#endif

WRITE_PACKET:
#ifdef ENABLE_FRAGROUTE
        if (options.frag_ctx == NULL) {
            /* write the packet when there's no fragrouting to be done */
            if (pkthdr_ptr->caplen)
                pcap_dump((u_char *)pout, pkthdr_ptr, *pktdata);
        } else {
            /* get the L3 protocol of the packet */
            proto = tcpedit_l3proto(tcpedit_ctx, AFTER_PROCESS, *pktdata, pkthdr_ptr->caplen);

            /* packet is IPv4/IPv6 AND needs to be fragmented */
            if ((proto == ETHERTYPE_IP || proto == ETHERTYPE_IP6) &&
                ((options.fragroute_dir == FRAGROUTE_DIR_BOTH) ||
                 (cache_result == TCPR_DIR_C2S && options.fragroute_dir == FRAGROUTE_DIR_C2S) ||
                 (cache_result == TCPR_DIR_S2C && options.fragroute_dir == FRAGROUTE_DIR_S2C))) {
#ifdef DEBUG
                int i = 0;
#endif
                if (fragroute_process(options.frag_ctx, *pktdata, pkthdr_ptr->caplen) < 0)
                    errx(-1, "Error processing packet via fragroute: %s", options.frag_ctx->errbuf);

                while ((frag_len = fragroute_getfragment(options.frag_ctx, &frag)) > 0) {
                    /* frags get the same timestamp as the original packet */
                    dbgx(1, "processing packet " COUNTER_SPEC " frag: %u (%d)", packetnum, i++, frag_len);
                    pkthdr_ptr->caplen = frag_len;
                    pkthdr_ptr->len = frag_len;
                    if (pkthdr_ptr->caplen)
                        pcap_dump((u_char *)pout, pkthdr_ptr, (u_char *)frag);
                }
            } else {
                /* write the packet without fragroute */
                if (pkthdr_ptr->caplen)
                    pcap_dump((u_char *)pout, pkthdr_ptr, *pktdata);
            }
        }
#else
        /* write the packet when there's no fragrouting to be done */
        if (pkthdr_ptr->caplen)
            pcap_dump((u_char *)pout, pkthdr_ptr, *pktdata);

#endif
    } /* while() */
    return 0;
}
