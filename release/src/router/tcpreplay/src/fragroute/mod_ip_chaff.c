/*
 * mod_ip_chaff.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"
#include "mod.h"
#include "pkt.h"
#include "randutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAFF_TYPE_DUP 1
#define CHAFF_TYPE_OPT 2
#define CHAFF_TYPE_TTL 3

struct ip_chaff_data {
    rand_t *rnd;
    int type;
    int ttl;
    struct pktq *pktq;
};

void *
ip_chaff_close(void *d)
{
    struct ip_chaff_data *data = (struct ip_chaff_data *)d;

    if (data != NULL) {
        rand_close(data->rnd);
        free(data);
    }
    return (NULL);
}

void *
ip_chaff_open(int argc, char *argv[])
{
    struct ip_chaff_data *data;

    if (argc < 2)
        return (NULL);

    if ((data = calloc(1, sizeof(*data))) == NULL)
        return (NULL);

    data->rnd = rand_open();

    if (strcasecmp(argv[1], "dup") == 0) {
        data->type = CHAFF_TYPE_DUP;
    } else if (strcasecmp(argv[1], "opt") == 0) {
        data->type = CHAFF_TYPE_OPT;
    } else if ((data->ttl = strtol(argv[1], NULL, 10)) >= 0 && data->ttl < 256) {
        data->type = CHAFF_TYPE_TTL;
    } else
        return (ip_chaff_close(data));

    return (data);
}

int
ip_chaff_apply(void *d, struct pktq *pktq)
{
    struct ip_chaff_data *data = (struct ip_chaff_data *)d;
    struct pkt *pkt, *new, *next;
    struct ip_opt opt;
    int i;

    for (pkt = TAILQ_FIRST(pktq); pkt != TAILQ_END(pktq); pkt = next) {
        next = TAILQ_NEXT(pkt, pkt_next);
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        if (pkt->pkt_ip_data == NULL)
            continue;

        new = pkt_dup(pkt);
        rand_strset(data->rnd, new->pkt_ip_data, new->pkt_end - new->pkt_ip_data + 1);

        switch (data->type) {
        case CHAFF_TYPE_DUP:
            new->pkt_ts.tv_usec = 1;
            if (eth_type == ETH_TYPE_IP) {
                ip_checksum(new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            }
            break;
        case CHAFF_TYPE_OPT:
            if (eth_type == ETH_TYPE_IP) {
                opt.opt_type = 0x42;
                opt.opt_len = IP_OPT_LEN;
                i = ip_add_option(new->pkt_ip, new->pkt_buf_size - ETH_HDR_LEN, IP_PROTO_IP, &opt, opt.opt_len);
                /* XXX - whack opt with random crap */
                *(uint32_t *)new->pkt_ip_data = rand_uint32(data->rnd);
                new->pkt_ip_data += i;
                new->pkt_end += i;
                ip_checksum(new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            } else if (eth_type == ETH_TYPE_IPV6) {
                continue;
            }
            /* fall through */
        case CHAFF_TYPE_TTL:
            if (eth_type == ETH_TYPE_IP) {
                new->pkt_ip->ip_ttl = data->ttl;
                ip_checksum(new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            } else if (eth_type == ETH_TYPE_IPV6) {
                pkt->pkt_ip6->ip6_hlim = data->ttl;
            }

            break;
        }
        /* Minimal random reordering - for ipv4 and ipv6 */
        if ((new->pkt_ip_data[0] & 1) == 0)
            TAILQ_INSERT_BEFORE(pkt, new, pkt_next);
        else
            TAILQ_INSERT_AFTER(pktq, pkt, new, pkt_next);
    }
    return (0);
}

struct mod mod_ip_chaff = {
        "ip_chaff",               /* name */
        "ip_chaff dup|opt|<ttl>", /* usage */
        ip_chaff_open,            /* open */
        ip_chaff_apply,           /* apply */
        ip_chaff_close            /* close */
};
