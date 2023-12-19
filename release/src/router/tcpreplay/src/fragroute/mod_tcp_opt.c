/*
 * mod_tcp_opt.c
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
tcp_opt_close(void *d)
{
    if (d != NULL)
        free(d);
    return (NULL);
}

void *
tcp_opt_open(int argc, char *argv[])
{
    struct tcp_opt *opt;
    int i;

    if (argc < 3)
        return (NULL);

    if ((opt = calloc(1, sizeof(*opt))) == NULL)
        return (NULL);

    if (strcasecmp(argv[1], "mss") == 0) {
        opt->opt_type = TCP_OPT_MSS;
        opt->opt_len = TCP_OPT_LEN + 2;

        if ((i = (int)strtol(argv[2], NULL, 10)) <= 0 || i > 0xffff) {
            warn("mss <size> must be from 0-65535");
            return (tcp_opt_close(opt));
        }
        opt->opt_data.mss = htons(i);
    } else if (strcasecmp(argv[1], "wscale") == 0) {
        opt->opt_type = TCP_OPT_WSCALE;
        opt->opt_len = TCP_OPT_LEN + 2;

        if ((i = (int)strtol(argv[2], NULL, 10)) <= 0 || i > 0xff) {
            warn("wscale <size> must be from 0-255");
            return (tcp_opt_close(opt));
        }
        opt->opt_data.wscale = i;
    } else if (strcasecmp(argv[1], "raw") == 0) {
        if (raw_ip_opt_parse(argc - 2,
                             &argv[2],
                             &opt->opt_type,
                             &opt->opt_len,
                             &opt->opt_data.data8[0],
                             sizeof(opt->opt_data.data8)) != 0)
            return (tcp_opt_close(opt));
    } else
        return (tcp_opt_close(opt));

    return (opt);
}

int
tcp_opt_apply(void *d, struct pktq *pktq)
{
    struct tcp_opt *opt = (struct tcp_opt *)d;
    struct pkt *pkt;

    TAILQ_FOREACH(pkt, pktq, pkt_next)
    {
        size_t len;
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        len = inet_add_option(eth_type,
                              pkt->pkt_ip,
                              sizeof(pkt->pkt_data) - ETH_HDR_LEN,
                              IP_PROTO_TCP,
                              opt,
                              opt->opt_len);

        if (len > 0) {
            pkt->pkt_end += len;
            pkt_decorate(pkt);
            inet_checksum(eth_type, pkt->pkt_ip, pkt->pkt_end - pkt->pkt_eth_data);
        }
    }
    return (0);
}

struct mod mod_tcp_opt = {
        "tcp_opt",                                     /* name */
        "tcp_opt mss|wscale <size>|raw <byte stream>", /* usage */
        tcp_opt_open,                                  /* open */
        tcp_opt_apply,                                 /* apply */
        tcp_opt_close                                  /* close */
};
