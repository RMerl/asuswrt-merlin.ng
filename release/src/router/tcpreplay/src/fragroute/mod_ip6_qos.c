/*
 * mod_ip6_qos.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 */

#include "config.h"
#include "mod.h"
#include "pkt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ip6_qos_data {
    int ip6_tc;
    int ip6_fl;
};

static void *
ip6_qos_close(void *d)
{
    if (d != NULL)
        free(d);
    return (NULL);
}

static void *
ip6_qos_open(int argc, char *argv[])
{
    struct ip6_qos_data *data;

    if (argc != 3) {
        return NULL;
    }

    if ((data = calloc(1, sizeof(*data))) == NULL)
        return (NULL);

    if (sscanf(argv[1], "%x", (unsigned int *)&data->ip6_tc) != 1 || data->ip6_tc < 0 || data->ip6_tc > 255)
        return (ip6_qos_close(data));

    if (sscanf(argv[2], "%x", (unsigned int *)&data->ip6_fl) != 1 || data->ip6_fl < 0 || data->ip6_fl > 0x100000)
        return (ip6_qos_close(data));

    printf("init: %x\n", data->ip6_fl);

    return (data);
}

static int
ip6_qos_apply(void *d, struct pktq *pktq)
{
    struct ip6_qos_data *data = (struct ip6_qos_data *)d;
    struct pkt *pkt;

    TAILQ_FOREACH(pkt, pktq, pkt_next)
    {
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        if (eth_type == ETH_TYPE_IPV6) {
            if (data->ip6_tc || data->ip6_fl) {
                pkt->pkt_ip6->ip6_flow = htonl((uint32_t)data->ip6_tc << 20 | data->ip6_fl);
                pkt->pkt_ip6->ip6_vfc = (IP6_VERSION | (data->ip6_tc >> 4));
            }
        }
    }
    return (0);
}

struct mod mod_ip6_qos = {
        "ip6_qos",           /* name */
        "ip6_qos <tc> <fl>", /* usage */
        ip6_qos_open,        /* open */
        ip6_qos_apply,       /* apply */
        ip6_qos_close        /* close */
};
