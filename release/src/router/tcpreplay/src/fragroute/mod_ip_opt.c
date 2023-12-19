/*
 * mod_ip_opt.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include "iputil.h"
#include "mod.h"
#include "pkt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *
ip_opt_close(void *d)
{
    if (d != NULL)
        free(d);
    return (NULL);
}

void *
ip_opt_open(int argc, char *argv[])
{
    struct ip_opt *opt;
    struct addr addr;
    int i, j;

    if (argc < 4)
        return (NULL);

    if ((opt = calloc(1, sizeof(*opt))) == NULL)
        return (NULL);

    if (strcasecmp(argv[1], "lsrr") == 0) {
        opt->opt_type = IP_OPT_LSRR;
    } else if (strcasecmp(argv[1], "ssrr") == 0) {
        opt->opt_type = IP_OPT_SSRR;
    } else if (strcasecmp(argv[1], "raw") == 0) {
        if (raw_ip_opt_parse(argc - 2,
                             &argv[2],
                             &opt->opt_type,
                             &opt->opt_len,
                             &opt->opt_data.data8[0],
                             sizeof(opt->opt_data.data8)) != 0)
            return (ip_opt_close(opt));
        return opt;
    } else
        return (ip_opt_close(opt));

    if ((i = (int)strtol(argv[2], NULL, 10)) < 4 || i > 0xff) {
        warn("<ptr> must be >= 4, and should be a multiple of 4");
        return (ip_opt_close(opt));
    }
    opt->opt_data.rr.ptr = i;

    for (i = 3, j = 0; i < argc && j < 9; i++, j++) {
        if (addr_aton(argv[i], &addr) < 0) {
            return (ip_opt_close(opt));
        }
        opt->opt_data.rr.iplist[j] = addr.addr_ip;
    }
    opt->opt_len = IP_OPT_LEN + 1 + (IP_ADDR_LEN * j);

    return (opt);
}

int
ip_opt_apply(void *d, struct pktq *pktq)
{
    struct ip_opt *opt = (struct ip_opt *)d;
    struct pkt *pkt;
    size_t len;

    TAILQ_FOREACH(pkt, pktq, pkt_next)
    {
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        if (eth_type == ETH_TYPE_IP) {
            len = ip_add_option(pkt->pkt_ip, PKT_BUF_LEN - ETH_HDR_LEN, IP_PROTO_IP, opt, opt->opt_len);

            if (len > 0) {
                pkt->pkt_end += len;
                pkt_decorate(pkt);
                ip_checksum(pkt->pkt_ip, pkt->pkt_end - pkt->pkt_eth_data);
            }
        }
    }
    return (0);
}

struct mod mod_ip_opt = {
        "ip_opt",                                                       /* name */
        "ip_opt [lsrr|ssrr <ptr> <ip-addr> ...] | [raw <byte stream>]", /* usage */
        ip_opt_open,                                                    /* open */
        ip_opt_apply,                                                   /* apply */
        ip_opt_close                                                    /* close */
};
