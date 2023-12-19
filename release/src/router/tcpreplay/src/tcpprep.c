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
 *  Purpose:
 *  1) Remove the performance bottleneck in tcpreplay for choosing an NIC
 *  2) Separate code to make it more manageable
 *  3) Add additional features which require multiple passes of a pcap
 *
 *  Support:
 *  Right now we support matching source IP based upon on of the following:
 *  - Regular expression
 *  - IP address is contained in one of a list of CIDR blocks
 *  - Auto learning of CIDR block for servers (clients all other)
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include "tcpprep_api.h"
#include "tcpprep_opts.h"
#include "tree.h"
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * global variables
 */
#ifdef DEBUG
int debug = 0;
#endif

tcpprep_t *tcpprep;
int info = 0;
char *cidr = NULL;
tcpr_data_tree_t treeroot;

void print_comment(const char *);
void print_info(const char *);
void print_stats(const char *);
static int check_ipv4_regex(unsigned long ip);
static int check_ipv6_regex(const struct tcpr_in6_addr *addr);
static COUNTER process_raw_packets(pcap_t *pcap);
static int check_dst_port(ipv4_hdr_t *ip_hdr, ipv6_hdr_t *ip6_hdr, int len);

/*
 *  main()
 */
int
main(int argc, char *argv[])
{
    int out_file;
    COUNTER totpackets;
    char errbuf[PCAP_ERRBUF_SIZE];
    tcpprep_opt_t *options;

    tcpprep = tcpprep_init();
    options = tcpprep->options;

    optionProcess(&tcpprepOptions, argc, argv);
    tcpprep_post_args(tcpprep, argc, argv);

    /* open the cache file */
    if ((out_file = open(OPT_ARG(CACHEFILE),
                         O_WRONLY | O_CREAT | O_TRUNC,
                         S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP | S_IROTH)) == -1) {
        tcpprep_close(tcpprep);
        errx(-1, "Unable to open cache file %s for writing: %s", OPT_ARG(CACHEFILE), strerror(errno));
    }

readpcap:
    /* open the pcap file */
    if ((options->pcap = pcap_open_offline(OPT_ARG(PCAP), errbuf)) == NULL) {
        close(out_file);
        tcpprep_close(tcpprep);
        errx(-1, "Error opening libpcap: %s", errbuf);
    }

#ifdef HAVE_PCAP_SNAPSHOT
    if (pcap_snapshot(options->pcap) < 65535)
        warnx("%s was captured using a snaplen of %d bytes.  This may mean you have truncated packets.",
              OPT_ARG(PCAP),
              pcap_snapshot(options->pcap));
#endif

    /* make sure we support the DLT type */
    switch (pcap_datalink(options->pcap)) {
    case DLT_EN10MB:
    case DLT_LINUX_SLL:
    case DLT_RAW:
    case DLT_C_HDLC:
    case DLT_JUNIPER_ETHER:
    case DLT_PPP_SERIAL:
        break; /* do nothing because all is good */
    default:
        errx(-1, "Unsupported pcap DLT type: 0x%x", pcap_datalink(options->pcap));
    }

    /* Can only split based on MAC address for ethernet */
    if ((pcap_datalink(options->pcap) != DLT_EN10MB) && (options->mode == MAC_MODE)) {
        close(out_file);
        tcpprep_close(tcpprep);
        err(-1, "MAC mode splitting is only supported by DLT_EN10MB packet captures.");
    }

#ifdef ENABLE_VERBOSE
    if (HAVE_OPT(VERBOSE)) {
        tcpdump_open(&tcpprep->tcpdump, options->pcap);
    }
#endif

    /* do we apply a bpf filter? */
    if (options->bpf.filter != NULL) {
        if (pcap_compile(options->pcap, &options->bpf.program, options->bpf.filter, options->bpf.optimize, 0) != 0) {
            close(out_file);
            tcpprep_close(tcpprep);
            return 0;
            errx(-1, "Error compiling BPF filter: %s", pcap_geterr(options->pcap));
        }
        pcap_setfilter(options->pcap, &options->bpf.program);
        pcap_freecode(&options->bpf.program);
    }

    if ((totpackets = process_raw_packets(options->pcap)) == 0) {
        close(out_file);
        tcpprep_close(tcpprep);
        err(-1, "No packets were processed.  Filter too limiting?");
    }

    pcap_close(options->pcap);
    options->pcap = NULL;

#ifdef ENABLE_VERBOSE
    tcpdump_close(&tcpprep->tcpdump);
#endif

    /* we need to process the pcap file twice in HASH/AUTO mode */
    if (options->mode == AUTO_MODE) {
        options->mode = options->automode;
        if (options->mode == ROUTER_MODE) { /* do we need to convert TREE->CIDR? */
            if (info)
                notice("Building network list from pre-cache...\n");
            if (!process_tree()) {
                err(-1, "Error: unable to build a valid list of servers. Aborting.");
            }
        } else {
            /*
             * in bridge mode we need to calculate client/sever
             * manually since this is done automatically in
             * process_tree()
             */
            tree_calculate(&treeroot);
        }

        if (info)
            notice("Building cache file...\n");
        /*
         * re-process files, but this time generate
         * cache
         */
        goto readpcap;
    }
#ifdef DEBUG
    if (debug && (options->cidrdata != NULL))
        print_cidr(options->cidrdata);
#endif

    /* write cache data */
    totpackets = write_cache(options->cachedata, out_file, totpackets, options->comment);
    if (info)
        notice("Done.\nCached " COUNTER_SPEC " packets.\n", totpackets);

    /* close cache file */
    close(out_file);

    tcpprep_close(tcpprep);

    restore_stdin();
    return 0;
}

/**
 * checks the dst port to see if this is destined for a server port.
 * returns 1 for true, 0 for false
 */
static int
check_dst_port(ipv4_hdr_t *ip_hdr, ipv6_hdr_t *ip6_hdr, int len)
{
    tcp_hdr_t *tcp_hdr = NULL;
    udp_hdr_t *udp_hdr = NULL;
    tcpprep_opt_t *options = tcpprep->options;
    uint8_t proto;
    u_char *l4;

    if (ip_hdr) {
        if (len < ((ip_hdr->ip_hl * 4) + 4))
            return 0; /* not enough data in the packet to know */

        proto = ip_hdr->ip_p;
        l4 = get_layer4_v4(ip_hdr, (u_char *)ip_hdr + len);
    } else if (ip6_hdr) {
        if (len < (TCPR_IPV6_H + 4))
            return 0; /* not enough data in the packet to know */

        proto = get_ipv6_l4proto(ip6_hdr, (u_char *)ip6_hdr + len);
        dbgx(3, "Our layer4 proto is 0x%hhu", proto);
        if ((l4 = get_layer4_v6(ip6_hdr, (u_char *)ip6_hdr + len)) == NULL)
            return 0;

        dbgx(3,
             "Found proto %u at offset %p.  base %p (%p)",
             proto,
             (void *)l4,
             (void *)ip6_hdr,
             (void *)(l4 - (u_char *)ip6_hdr));
    } else {
        assert(0);
    }

    dbg(3, "Checking the destination port...");

    switch (proto) {
    case IPPROTO_TCP:
        tcp_hdr = (tcp_hdr_t *)l4;

        /* is a service? */
        if (options->services.tcp[ntohs(tcp_hdr->th_dport)]) {
            dbgx(1, "TCP packet is destined for a server port: %d", ntohs(tcp_hdr->th_dport));
            return 1;
        }

        /* nope */
        dbgx(1, "TCP packet is NOT destined for a server port: %d", ntohs(tcp_hdr->th_dport));
        return 0;

    case IPPROTO_UDP:
        udp_hdr = (udp_hdr_t *)l4;

        /* is a service? */
        if (options->services.udp[ntohs(udp_hdr->uh_dport)]) {
            dbgx(1, "UDP packet is destined for a server port: %d", ntohs(udp_hdr->uh_dport));
            return 1;
        }

        /* nope */
        dbgx(1, "UDP packet is NOT destined for a server port: %d", ntohs(udp_hdr->uh_dport));
        return 0;

    default:
        /* not a TCP or UDP packet... return as non_ip */
        dbg(1, "Packet isn't a UDP or TCP packet... no port to process.");
        return options->nonip;
    }
}

/**
 * checks to see if an ip address matches a regex.  Returns 1 for true
 * 0 for false
 */
static int
check_ipv4_regex(const unsigned long ip)
{
    int eflags = 0;
    u_char src_ip[16];
    size_t nmatch = 0;
    tcpprep_opt_t *options = tcpprep->options;

    memset(src_ip, '\0', sizeof(src_ip));
    strlcpy((char *)src_ip, (char *)get_addr2name4(ip, RESOLVE), sizeof(src_ip));
    if (regexec(&options->preg, (char *)src_ip, nmatch, NULL, eflags) == 0) {
        return 1;
    } else {
        return 0;
    }
}

static int
check_ipv6_regex(const struct tcpr_in6_addr *addr)
{
    int eflags = 0;
    u_char src_ip[INET6_ADDRSTRLEN];
    size_t nmatch = 0;
    tcpprep_opt_t *options = tcpprep->options;

    memset(src_ip, '\0', sizeof(src_ip));
    strlcpy((char *)src_ip, (char *)get_addr2name6(addr, RESOLVE), sizeof(src_ip));
    if (regexec(&options->preg, (char *)src_ip, nmatch, NULL, eflags) == 0) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * uses libpcap library to parse the packets and build
 * the cache file.
 */
static COUNTER
process_raw_packets(pcap_t *pcap)
{
    struct pcap_pkthdr pkthdr;
    const u_char *pktdata = NULL;
    COUNTER packetnum = 0;
    int l2len;
    u_char *ipbuff, *buffptr;
    tcpr_dir_t direction = TCPR_DIR_ERROR;
    tcpprep_opt_t *options = tcpprep->options;

    assert(pcap);

    ipbuff = safe_malloc(MAXPACKET);

    while ((pktdata = safe_pcap_next(pcap, &pkthdr)) != NULL) {
        ipv4_hdr_t *ip_hdr = NULL;
        ipv6_hdr_t *ip6_hdr = NULL;
        eth_hdr_t *eth_hdr = NULL;

        packetnum++;

        dbgx(1, "Packet " COUNTER_SPEC, packetnum);

        /* look for include or exclude LIST match */
        if (options->xX.list != NULL) {
            if (options->xX.mode < xXExclude) {
                /* include list */
                if (!check_list(options->xX.list, packetnum)) {
                    add_cache(&(options->cachedata), DONT_SEND, 0);
                    continue;
                }
            }
            /* exclude list */
            else if (check_list(options->xX.list, packetnum)) {
                add_cache(&(options->cachedata), DONT_SEND, 0);
                continue;
            }
        }

        /*
         * If the packet doesn't include an IPv4 header we should just treat
         * it as a non-IP packet, UNLESS we're in MAC mode, in which case
         * we should let the MAC matcher below handle it
         */
        if (options->mode != MAC_MODE) {
            dbg(3, "Looking for IPv4/v6 header in non-MAC mode");

            /* get the IP header (if any) */
            buffptr = ipbuff;

            /* first look for IPv4 */
            if ((ip_hdr = (ipv4_hdr_t *)get_ipv4(pktdata, (int)pkthdr.caplen, pcap_datalink(pcap), &buffptr)) != NULL) {
                dbg(2, "Packet is IPv4");
            } else if ((ip6_hdr = (ipv6_hdr_t *)get_ipv6(pktdata, (int)pkthdr.caplen, pcap_datalink(pcap), &buffptr)) !=
                       NULL) {
                /* IPv6 */
                dbg(2, "Packet is IPv6");
            } else {
                /* we're something else... */
                dbg(2, "Packet isn't IPv4/v6");

                /* we don't want to cache these packets twice */
                if (options->mode != AUTO_MODE) {
                    dbg(3, "Adding to cache using options for Non-IP packets");
                    add_cache(&options->cachedata, SEND, options->nonip);
                }

                /* go to next packet */
                continue;
            }

            l2len = get_l2len(pktdata, (int)pkthdr.caplen, pcap_datalink(pcap));
            if (l2len < 0) {
                /* go to next packet */
                continue;
            }

            /* look for include or exclude CIDR match */
            if (options->xX.cidr != NULL) {
                if (ip_hdr) {
                    if (!process_xX_by_cidr_ipv4(options->xX.mode, options->xX.cidr, ip_hdr)) {
                        add_cache(&options->cachedata, DONT_SEND, 0);
                        continue;
                    }
                } else if (ip6_hdr) {
                    if (!process_xX_by_cidr_ipv6(options->xX.mode, options->xX.cidr, ip6_hdr)) {
                        add_cache(&options->cachedata, DONT_SEND, 0);
                        continue;
                    }
                }
            }
        }

        switch (options->mode) {
        case REGEX_MODE:
            dbg(2, "processing regex mode...");
            if (ip_hdr) {
                direction = check_ipv4_regex(ip_hdr->ip_src.s_addr);
            } else if (ip6_hdr) {
                direction = check_ipv6_regex(&ip6_hdr->ip_src);
            }

            /* reverse direction? */
            if (HAVE_OPT(REVERSE) && (direction == TCPR_DIR_C2S || direction == TCPR_DIR_S2C))
                direction = direction == TCPR_DIR_C2S ? TCPR_DIR_S2C : TCPR_DIR_C2S;

            add_cache(&options->cachedata, SEND, direction);
            break;

        case CIDR_MODE:
            dbg(2, "processing cidr mode...");
            if (ip_hdr) {
                direction = check_ip_cidr(options->cidrdata, ip_hdr->ip_src.s_addr) ? TCPR_DIR_C2S : TCPR_DIR_S2C;
            } else if (ip6_hdr) {
                direction = check_ip6_cidr(options->cidrdata, &ip6_hdr->ip_src) ? TCPR_DIR_C2S : TCPR_DIR_S2C;
            }

            /* reverse direction? */
            if (HAVE_OPT(REVERSE) && (direction == TCPR_DIR_C2S || direction == TCPR_DIR_S2C))
                direction = direction == TCPR_DIR_C2S ? TCPR_DIR_S2C : TCPR_DIR_C2S;

            add_cache(&options->cachedata, SEND, direction);
            break;

        case MAC_MODE:
            dbg(2, "processing mac mode...");
            if (pkthdr.caplen < sizeof(*eth_hdr)) {
                dbg(2, "capture length too short for mac mode processing");
                break;
            }

            eth_hdr = (eth_hdr_t *)pktdata;
            direction = macinstring(options->maclist, (u_char *)eth_hdr->ether_shost);

            /* reverse direction? */
            if (HAVE_OPT(REVERSE) && (direction == TCPR_DIR_C2S || direction == TCPR_DIR_S2C))
                direction = direction == TCPR_DIR_C2S ? TCPR_DIR_S2C : TCPR_DIR_C2S;

            add_cache(&options->cachedata, SEND, direction);
            break;

        case AUTO_MODE:
            dbg(2, "processing first pass of auto mode...");
            /* first run through in auto mode: create tree */
            if (options->automode != FIRST_MODE) {
                if (ip_hdr) {
                    add_tree_ipv4(ip_hdr->ip_src.s_addr, pktdata, (int)pkthdr.caplen, pcap_datalink(pcap));
                } else if (ip6_hdr) {
                    add_tree_ipv6(&ip6_hdr->ip_src, pktdata, (int)pkthdr.caplen, pcap_datalink(pcap));
                }
            } else {
                if (ip_hdr) {
                    add_tree_first_ipv4(pktdata, (int)pkthdr.caplen, pcap_datalink(pcap));
                } else if (ip6_hdr) {
                    add_tree_first_ipv6(pktdata, (int)pkthdr.caplen, pcap_datalink(pcap));
                }
            }
            break;

        case ROUTER_MODE:
            /*
             * second run through in auto mode: create route
             * based cache
             */
            dbg(2, "processing second pass of auto: router mode...");
            if (ip_hdr) {
                add_cache(&options->cachedata, SEND, check_ip_tree(options->nonip, ip_hdr->ip_src.s_addr));
            } else {
                add_cache(&options->cachedata, SEND, check_ip6_tree(options->nonip, &ip6_hdr->ip_src));
            }
            break;

        case BRIDGE_MODE:
            /*
             * second run through in auto mode: create bridge
             * based cache
             */
            dbg(2, "processing second pass of auto: bridge mode...");
            if (ip_hdr) {
                add_cache(&options->cachedata, SEND, check_ip_tree(DIR_UNKNOWN, ip_hdr->ip_src.s_addr));
            } else {
                add_cache(&options->cachedata, SEND, check_ip6_tree(DIR_UNKNOWN, &ip6_hdr->ip_src));
            }
            break;

        case SERVER_MODE:
            /*
             * second run through in auto mode: create bridge
             * where unknowns are servers
             */
            dbg(2, "processing second pass of auto: server mode...");
            if (ip_hdr) {
                add_cache(&options->cachedata, SEND, check_ip_tree(DIR_SERVER, ip_hdr->ip_src.s_addr));
            } else {
                add_cache(&options->cachedata, SEND, check_ip6_tree(DIR_SERVER, &ip6_hdr->ip_src));
            }
            break;

        case CLIENT_MODE:
            /*
             * second run through in auto mode: create bridge
             * where unknowns are clients
             */
            dbg(2, "processing second pass of auto: client mode...");
            if (ip_hdr) {
                add_cache(&options->cachedata, SEND, check_ip_tree(DIR_CLIENT, ip_hdr->ip_src.s_addr));
            } else {
                add_cache(&options->cachedata, SEND, check_ip6_tree(DIR_CLIENT, &ip6_hdr->ip_src));
            }
            break;

        case PORT_MODE:
            /*
             * process ports based on their destination port
             */
            dbg(2, "processing port mode...");
            add_cache(&options->cachedata, SEND, check_dst_port(ip_hdr, ip6_hdr, (int)pkthdr.caplen - l2len));
            break;

        case FIRST_MODE:
            /*
             * First packet mode, looks at each host and picks clients
             * by the ones which send the first packet in a session
             */
            dbg(2, "processing second pass of auto: first packet mode...");
            if (ip_hdr) {
                add_cache(&options->cachedata, SEND, check_ip_tree(DIR_UNKNOWN, ip_hdr->ip_src.s_addr));
            } else {
                add_cache(&options->cachedata, SEND, check_ip6_tree(DIR_UNKNOWN, &ip6_hdr->ip_src));
            }
            break;

        default:
            errx(-1, "Whoops!  What mode are we in anyways? %d", options->mode);
        }
#ifdef ENABLE_VERBOSE
        if (options->verbose)
            tcpdump_print(&tcpprep->tcpdump, &pkthdr, pktdata);
#endif
    }

    safe_free(ipbuff);

    return packetnum;
}

/**
 * print the tcpprep cache file comment
 */
void
print_comment(const char *file)
{
    char *cachedata = NULL;
    char *comment = NULL;
    COUNTER count;

    count = read_cache(&cachedata, file, &comment);
    printf("tcpprep args: %s\n", comment);
    printf("Cache contains data for " COUNTER_SPEC " packets\n", count);

    exit(0);
}

/**
 * prints out the cache file details
 */
void
print_info(const char *file)
{
    char *cachedata = NULL;
    char *comment = NULL;
    COUNTER count, i;

    count = read_cache(&cachedata, file, &comment);
    if (count > 65535)
        exit(-1);

    for (i = 1; i <= count; i++) {
        switch (check_cache(cachedata, i)) {
        case TCPR_DIR_C2S:
            printf("Packet " COUNTER_SPEC " -> Primary\n", i);
            break;
        case TCPR_DIR_S2C:
            printf("Packet " COUNTER_SPEC " -> Secondary\n", i);
            break;
        case TCPR_DIR_NOSEND:
            printf("Packet " COUNTER_SPEC " -> Don't Send\n", i);
            break;
        default:
            err(-1, "Invalid cachedata value!");
        }
    }
    exit(0);
}

/**
 * Print the per-packet statistics
 */
void
print_stats(const char *file)
{
    char *cachedata = NULL;
    char *comment = NULL;
    COUNTER i, count;
    COUNTER pri = 0, sec = 0, nosend = 0;

    count = read_cache(&cachedata, file, &comment);
    for (i = 1; i <= count; i++) {
        int cacheval = check_cache(cachedata, i);
        switch (cacheval) {
        case TCPR_DIR_C2S:
            pri++;
            break;
        case TCPR_DIR_S2C:
            sec++;
            break;
        case TCPR_DIR_NOSEND:
            nosend++;
            break;
        default:
            errx(-1, "Unknown cache value: %d", cacheval);
        }
    }
    printf("Primary packets:\t" COUNTER_SPEC "\n", pri);
    printf("Secondary packets:\t" COUNTER_SPEC "\n", sec);
    printf("Skipped packets:\t" COUNTER_SPEC "\n", nosend);
    printf("------------------------------\n");
    printf("Total packets:\t\t" COUNTER_SPEC "\n", count);
    exit(0);
}
