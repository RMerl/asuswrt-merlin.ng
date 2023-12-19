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

#include "bridge.h"
#include "config.h"
#include "common.h"
#include "tcpbridge.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

extern tcpreplay_stats_t stats;
volatile bool didsig;

static void live_callback(u_char *, const struct pcap_pkthdr *, const u_char *);

static void signal_catcher(int signo);

/**
 * First, prep our RB Tree which tracks where each (source)
 * MAC really lives so we don't create really nasty network
 * storms.
 */
static struct macsrc_t *new_node(void);

RB_HEAD(macsrc_tree, macsrc_t) macsrc_root;

static int
rbmacsrc_comp(struct macsrc_t *a, struct macsrc_t *b)
{
    return (memcmp(a->key, b->key, ETHER_ADDR_LEN));
}

RB_PROTOTYPE(macsrc_tree, macsrc_t, node, rbmacsrc_comp)
RB_GENERATE(macsrc_tree, macsrc_t, node, rbmacsrc_comp)

/**
 * create a new node... Malloc's memory
 */
struct macsrc_t *
new_node(void)
{
    struct macsrc_t *node;

    node = (struct macsrc_t *)safe_malloc(sizeof(struct macsrc_t));

    memset(node, '\0', sizeof(struct macsrc_t));
    return (node);
}

/**
 * main loop for bridging in only one direction
 * optimized to not use poll(), but rather libpcap's builtin pcap_loop()
 */
static void
do_bridge_unidirectional(tcpbridge_opt_t *options, tcpedit_t *tcpedit)
{
    struct live_data_t livedata;
    int retcode;

    assert(options);
    assert(tcpedit);

    livedata.tcpedit = tcpedit;
    livedata.source = PCAP_INT1;
    livedata.pcap = options->pcap1;
    livedata.options = options;

    if ((retcode = pcap_loop(options->pcap1, (int)options->limit_send, live_callback, (u_char *)&livedata)) < 0) {
        warnx("Error in %d pcap_loop(): %s", retcode, pcap_geterr(options->pcap1));
    }
}

/**
 * main loop for bridging in both directions.  Since we dealing with two handles
 * we need to poll() on them which isn't the most efficient
 */
static void
do_bridge_bidirectional(tcpbridge_opt_t *options, tcpedit_t *tcpedit)
{
    struct pollfd polls[2]; /* one for left & right pcap */
    int pollresult, pollcount, timeout;
    struct live_data_t livedata;

    assert(options);
    assert(tcpedit);

    livedata.tcpedit = tcpedit;
    livedata.options = options;

    /*
     * loop until ctrl-C or we've sent enough packets
     * note that if -L wasn't specified, limit_send is
     * set to 0 so this will loop infinately
     */
    while ((options->limit_send == 0) || (options->limit_send > stats.pkts_sent)) {
        if (didsig)
            break;

        dbgx(3, "limit_send: " COUNTER_SPEC " \t pkts_sent: " COUNTER_SPEC, options->limit_send, stats.pkts_sent);

        /* reset the result codes */
        polls[PCAP_INT1].revents = 0;
        polls[PCAP_INT1].events = POLLIN;
        polls[PCAP_INT1].fd = pcap_fileno(options->pcap1);

        polls[PCAP_INT2].revents = 0;
        polls[PCAP_INT2].events = POLLIN;
        polls[PCAP_INT2].fd = pcap_fileno(options->pcap2);

        timeout = options->poll_timeout;
        pollcount = 2;

        /* poll for a packet on the two interfaces */
        pollresult = poll(polls, pollcount, timeout);

        /* poll has returned, process the result */
        if (pollresult > 0) {
            dbgx(3, "pollresult: %d", pollresult);

            /* success, got one or more packets */
            if (polls[PCAP_INT1].revents > 0) {
                dbg(5, "Processing first interface");
                livedata.source = PCAP_INT1;
                livedata.pcap = options->pcap1;
                pcap_dispatch(options->pcap1, -1, (pcap_handler)live_callback, (u_char *)&livedata);
            }

            /* check the other interface?? */
            if (polls[PCAP_INT2].revents > 0) {
                dbg(5, "Processing second interface");
                livedata.source = PCAP_INT2;
                livedata.pcap = options->pcap2;
                pcap_dispatch(options->pcap2, -1, (pcap_handler)live_callback, (u_char *)&livedata);
            }

        } else if (pollresult == 0) {
            dbg(3, "poll timeout exceeded...");
            /* do something here? */
        } else {
            /* poll error, probably a Ctrl-C */
            warnx("poll() error: %s", strerror(errno));
        }

        /* go back to the top of the loop */
    }

} /* do_bridge_bidirectional() */

/**
 * Main entry point to bridging.  Does some initial setup and then calls the
 * correct loop (unidirectional or bidirectional)
 */
void
do_bridge(tcpbridge_opt_t *options, tcpedit_t *tcpedit)
{
    /* do we apply a bpf filter? */
    if (options->bpf.filter != NULL) {
        /* compile filter */
        dbgx(2, "Try to compile pcap bpf filter: %s", options->bpf.filter);
        if (pcap_compile(options->pcap1, &options->bpf.program, options->bpf.filter, options->bpf.optimize, 0) != 0) {
            errx(-1, "Error compiling BPF filter: %s", pcap_geterr(options->pcap1));
        }

        /* apply filter */
        pcap_setfilter(options->pcap1, &options->bpf.program);
        pcap_freecode(&options->bpf.program);

        /* same for other interface if applicable */
        if (options->unidir == 0) {
            /* compile filter */
            dbgx(2, "Try to compile pcap bpf filter: %s", options->bpf.filter);
            if (pcap_compile(options->pcap2, &options->bpf.program, options->bpf.filter, options->bpf.optimize, 0) !=
                0) {
                errx(-1, "Error compiling BPF filter: %s", pcap_geterr(options->pcap2));
            }

            /* apply filter */
            pcap_setfilter(options->pcap2, &options->bpf.program);
            pcap_freecode(&options->bpf.program);
        }
    }

    /* register signals */
    didsig = 0;
    (void)signal(SIGINT, signal_catcher);

    if (options->unidir == 1) {
        do_bridge_unidirectional(options, tcpedit);
    } else {
        do_bridge_bidirectional(options, tcpedit);
    }

    if (gettimeofday(&stats.end_time, NULL) < 0)
        errx(-1, "gettimeofday() failed: %s", strerror(errno));
    packet_stats(&stats);
}

/**
 * This is the callback we use with pcap_dispatch to process
 * each packet received by libpcap on the two interfaces.
 * Need to return > 0 to denote success
 */
static void
live_callback(u_char *usr_data, const struct pcap_pkthdr *const_pkthdr, const u_char *nextpkt)
{
    struct live_data_t *livedata = (struct live_data_t *)usr_data;
    struct pcap_pkthdr pkthdr_buf = *const_pkthdr;
    struct pcap_pkthdr *pkthdr = &pkthdr_buf;
    ipv4_hdr_t *ip_hdr = NULL;
    ipv6_hdr_t *ip6_hdr = NULL;
    pcap_t *send = NULL;
    static u_char *pktdata = NULL; /* full packet buffer */
    int cache_mode;
    static unsigned long packetnum = 0;
    struct macsrc_t *node, finder; /* rb tree nodes */
#ifdef DEBUG
    u_char dstmac[ETHER_ADDR_LEN];
#endif
    u_int16_t l2proto;

    packetnum++;
    dbgx(2, "packet %lu caplen %d", packetnum, pkthdr->caplen);

    /* only malloc the first time */
    if (pktdata == NULL) {
        /* create packet buffers */
        pktdata = (u_char *)safe_malloc(MAXPACKET);
    } else {
        /* zero out the old packet info */
        memset(pktdata, '\0', MAXPACKET);
    }

    /* copy the packet to our buffer */
    memcpy(pktdata, nextpkt, pkthdr->caplen);

#ifdef ENABLE_VERBOSE
    /* decode packet? */
    if (livedata->options->verbose)
        tcpdump_print(livedata->options->tcpdump, pkthdr, nextpkt);
#endif

    /* lookup our source MAC in the tree */
    memcpy(&finder.key, &pktdata[ETHER_ADDR_LEN], ETHER_ADDR_LEN);
#ifdef DEBUG
    memcpy(&dstmac, pktdata, ETHER_ADDR_LEN);
    dbgx(1, "SRC MAC: " MAC_FORMAT "\tDST MAC: " MAC_FORMAT, MAC_STR(finder.key), MAC_STR(dstmac));
#endif

    /* first, is this a packet sent locally?  If so, ignore it */
    if ((memcmp(livedata->options->intf1_mac, &finder.key, ETHER_ADDR_LEN)) == 0) {
        dbgx(1, "Packet matches the MAC of %s, skipping.", livedata->options->intf1);
        return;
    } else if ((memcmp(livedata->options->intf2_mac, &finder.key, ETHER_ADDR_LEN)) == 0) {
        dbgx(1, "Packet matches the MAC of %s, skipping.", livedata->options->intf2);
        return;
    }

    node = RB_FIND(macsrc_tree, &macsrc_root, &finder);

    /* if we can't find the node, build a new one */
    if (node == NULL) {
        dbg(1, "Unable to find MAC in the tree");
        node = new_node();
        node->source = livedata->source;
        memcpy(&node->key, &finder.key, ETHER_ADDR_LEN);
        RB_INSERT(macsrc_tree, &macsrc_root, node);
    }

    /* otherwise compare sources */
    else if (node->source != livedata->source) {
        dbg(1, "Found the dest MAC in the tree and it doesn't match this source NIC... skipping packet");
        /*
         * IMPORTANT!!!
         * Never send a packet out the same interface we sourced it on!
         */
        return;
    }

    /* what is our cache mode? */
    cache_mode = livedata->source == PCAP_INT1 ? TCPR_DIR_C2S : TCPR_DIR_S2C;

    l2proto = tcpedit_l3proto(livedata->tcpedit, BEFORE_PROCESS, pktdata, (int)pkthdr->len);
    dbgx(2, "Packet protocol: %04hx", l2proto);

    /* should we skip this packet based on CIDR match? */
    if (l2proto == ETHERTYPE_IP) {
        dbg(3, "Packet is IPv4");
        ip_hdr = (ipv4_hdr_t *)tcpedit_l3data(livedata->tcpedit, BEFORE_PROCESS, pktdata, (int)pkthdr->len);

        /* look for include or exclude CIDR match */
        if (livedata->options->xX.cidr != NULL) {
            if (!ip_hdr || !process_xX_by_cidr_ipv4(livedata->options->xX.mode, livedata->options->xX.cidr, ip_hdr)) {
                dbg(2, "Skipping IPv4 packet due to CIDR match");
                return;
            }
        }

    } else if (l2proto == ETHERTYPE_IP6) {
        dbg(3, "Packet is IPv6");
        ip6_hdr = (ipv6_hdr_t *)tcpedit_l3data(livedata->tcpedit, BEFORE_PROCESS, pktdata, (int)pkthdr->len);

        /* look for include or exclude CIDR match */
        if (livedata->options->xX.cidr != NULL) {
            if (!process_xX_by_cidr_ipv6(livedata->options->xX.mode, livedata->options->xX.cidr, ip6_hdr)) {
                dbg(2, "Skipping IPv6 packet due to CIDR match");
                return;
            }
        }
    }

    if (tcpedit_packet(livedata->tcpedit, &pkthdr, &pktdata, cache_mode) < 0)
        return;

    /*
     * send packets out the OTHER interface
     * and update the dst mac if necessary
     */
    switch (node->source) {
    case PCAP_INT1:
        dbgx(2, "Packet source was %s... sending out on %s", livedata->options->intf1, livedata->options->intf2);
        send = livedata->options->pcap2;
        break;

    case PCAP_INT2:
        dbgx(2, "Packet source was %s... sending out on %s", livedata->options->intf2, livedata->options->intf1);
        send = livedata->options->pcap1;
        break;

    default:
        errx(-1, "wtf?  our node->source != PCAP_INT1 and != PCAP_INT2: %c", node->source);
    }

    /*
     * write packet out on the network
     */
    if (pcap_sendpacket(send, pktdata, (int)pkthdr->caplen) < 0)
        errx(-1,
             "Unable to send packet out %s: %s",
             send == livedata->options->pcap1 ? livedata->options->intf1 : livedata->options->intf2,
             pcap_geterr(send));

    stats.bytes_sent += pkthdr->caplen;
    stats.pkts_sent++;

    dbgx(1, "Sent packet " COUNTER_SPEC, stats.pkts_sent);
} /* live_callback() */

static void
signal_catcher(int signo)
{
    /* stdio in signal handlers causes a race condition, instead set a flag */
    if (signo == SIGINT)
        didsig = true;
}
