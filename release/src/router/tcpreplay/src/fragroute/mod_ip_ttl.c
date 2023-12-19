/*
 * mod_ip_ttl.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"
#include "argv.h"
#include "mod.h"
#include "pkt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ip_ttl_data {
    int ttl;
};

void *
ip_ttl_close(void *d)
{
    if (d != NULL)
        free(d);
    return (NULL);
}

void *
ip_ttl_open(int argc, char *argv[])
{
    struct ip_ttl_data *data;

    if (argc != 2)
        return (NULL);

    if ((data = calloc(1, sizeof(*data))) == NULL)
        return (NULL);

    if ((data->ttl = strtol(argv[1], NULL, 10)) <= 0 || data->ttl > 255)
        return (ip_ttl_close(data));

    return (data);
}

int
ip_ttl_apply(void *d, struct pktq *pktq)
{
    struct ip_ttl_data *data = (struct ip_ttl_data *)d;
    struct pkt *pkt;
    int ttldec;

    TAILQ_FOREACH(pkt, pktq, pkt_next)
    {
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        if (eth_type == ETH_TYPE_IP) {
            ttldec = pkt->pkt_ip->ip_ttl - data->ttl;
            pkt->pkt_ip->ip_ttl = data->ttl;

            if (pkt->pkt_ip->ip_sum >= htons(0xffff - (ttldec << 8)))
                pkt->pkt_ip->ip_sum += htons(ttldec << 8) + 1;
            else
                pkt->pkt_ip->ip_sum += htons(ttldec << 8);
        } else if (eth_type == ETH_TYPE_IPV6) {
            pkt->pkt_ip6->ip6_hlim = data->ttl;
        }
    }
    return (0);
}

struct mod mod_ip_ttl = {
        "ip_ttl",       /* name */
        "ip_ttl <ttl>", /* usage */
        ip_ttl_open,    /* open */
        ip_ttl_apply,   /* apply */
        ip_ttl_close    /* close */
};
