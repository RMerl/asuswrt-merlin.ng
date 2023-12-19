/*
 * mod_tcp_chaff.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"
#include "iputil.h"
#include "mod.h"
#include "pkt.h"
#include "randutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAFF_TYPE_CKSUM 1
#define CHAFF_TYPE_NULL 2
#define CHAFF_TYPE_PAWS 3
#define CHAFF_TYPE_REXMIT 4
#define CHAFF_TYPE_SEQ 5
#define CHAFF_TYPE_SYN 6
#define CHAFF_TYPE_TTL 7

struct tcp_chaff_data {
    rand_t *rnd;
    int type;
    int ttl;
};

void *
tcp_chaff_close(void *d)
{
    struct tcp_chaff_data *data = (struct tcp_chaff_data *)d;

    if (data != NULL) {
        rand_close(data->rnd);
        free(data);
    }
    return (NULL);
}

void *
tcp_chaff_open(int argc, char *argv[])
{
    struct tcp_chaff_data *data;

    if (argc < 2)
        return (NULL);

    if ((data = calloc(1, sizeof(*data))) == NULL)
        return (NULL);

    data->rnd = rand_open();

    if (strcasecmp(argv[1], "cksum") == 0)
        data->type = CHAFF_TYPE_CKSUM;
    else if (strcasecmp(argv[1], "null") == 0)
        data->type = CHAFF_TYPE_NULL;
    else if (strcasecmp(argv[1], "paws") == 0)
        data->type = CHAFF_TYPE_PAWS;
    else if (strcasecmp(argv[1], "rexmit") == 0)
        data->type = CHAFF_TYPE_REXMIT;
    else if (strcasecmp(argv[1], "seq") == 0)
        data->type = CHAFF_TYPE_SEQ;
    else if (strcasecmp(argv[1], "syn") == 0)
        data->type = CHAFF_TYPE_SYN;
    else if ((data->ttl = (int)strtol(argv[1], NULL, 10)) > 0 && data->ttl < 256)
        data->type = CHAFF_TYPE_TTL;
    else
        return (tcp_chaff_close(data));

    return (data);
}

int
tcp_chaff_apply(void *d, struct pktq *pktq)
{
    struct tcp_chaff_data *data = (struct tcp_chaff_data *)d;
    struct pkt *pkt, *new, *next;

    for (pkt = TAILQ_FIRST(pktq); pkt != TAILQ_END(pktq); pkt = next) {
        struct tcp_opt opt;
        int i;
        uint16_t eth_type;
        uint8_t nxt;

        next = TAILQ_NEXT(pkt, pkt_next);
        eth_type = htons(pkt->pkt_eth->eth_type);
        if (pkt->pkt_ip == NULL)
            continue;

        if (eth_type == ETH_TYPE_IP) {
            nxt = pkt->pkt_ip->ip_p;
        } else if (eth_type == ETH_TYPE_IPV6) {
            nxt = pkt->pkt_ip6->ip6_nxt;
        } else {
            continue;
        }

        if (nxt != IP_PROTO_TCP || pkt->pkt_tcp == NULL || pkt->pkt_tcp_data == NULL ||
            (pkt->pkt_tcp->th_flags & TH_ACK) == 0)
            continue;

        new = pkt_dup(pkt);
        rand_strset(data->rnd, new->pkt_tcp_data, new->pkt_end - new->pkt_tcp_data + 1);

        switch (data->type) {
        case CHAFF_TYPE_CKSUM:
            inet_checksum(eth_type, new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            new->pkt_tcp->th_sum = rand_uint16(data->rnd);
            break;
        case CHAFF_TYPE_NULL:
            new->pkt_tcp->th_flags = 0;
            inet_checksum(eth_type, new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            break;
        case CHAFF_TYPE_PAWS:
            /* Delete any existing TCP options. */
            i = (new->pkt_tcp->th_off << 2) - TCP_HDR_LEN;
            new->pkt_tcp->th_off = 5;
            new->pkt_end -= i;
            new->pkt_ip->ip_len = htons(new->pkt_end - new->pkt_eth_data);

            /* Insert initial timestamp, for PAWS elimination. */
            opt.opt_type = TCP_OPT_TIMESTAMP;
            opt.opt_len = TCP_OPT_LEN + 8;
            opt.opt_data.timestamp[0] = 0;
            opt.opt_data.timestamp[1] = 0;
            if ((i = (int)inet_add_option(eth_type,
                                          new->pkt_ip,
                                          PKT_BUF_LEN - ETH_HDR_LEN,
                                          IP_PROTO_TCP,
                                          &opt,
                                          opt.opt_len)) < 0) {
                pkt_free(new);
                continue;
            }
            new->pkt_end += i;
            inet_checksum(eth_type, new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            pkt_decorate(new);
            break;
        case CHAFF_TYPE_REXMIT:
            new->pkt_ts.tv_usec = 1;
            inet_checksum(eth_type, new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            break;
        case CHAFF_TYPE_SEQ:
            /* XXX - dunno recv window? */
            new->pkt_tcp->th_seq = htonl(666);
            new->pkt_tcp->th_ack = htonl(666);
            inet_checksum(eth_type, new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            break;
        case CHAFF_TYPE_SYN:
            new->pkt_tcp->th_flags = TH_SYN;
            new->pkt_tcp->th_seq = rand_uint32(data->rnd);
            new->pkt_tcp->th_ack = 0;
            new->pkt_end = new->pkt_tcp_data;
            new->pkt_tcp_data = NULL;
            new->pkt_ip->ip_len = htons(new->pkt_end - new->pkt_eth_data);
            inet_checksum(eth_type, new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            break;
        case CHAFF_TYPE_TTL:
            if (eth_type == ETH_TYPE_IP) {
                new->pkt_ip->ip_ttl = data->ttl;
                ip_checksum(new->pkt_ip, new->pkt_ip_data - new->pkt_eth_data);
            } else if (eth_type == ETH_TYPE_IPV6) {
                new->pkt_ip6->ip6_hlim = data->ttl;
            }
            break;
        }
        /* Minimal random reordering. */
        if ((new->pkt_tcp->th_sum & 1) == 0)
            TAILQ_INSERT_BEFORE(pkt, new, pkt_next);
        else
            TAILQ_INSERT_AFTER(pktq, pkt, new, pkt_next);
    }
    return (0);
}

struct mod mod_tcp_chaff = {
        "tcp_chaff",                                      /* name */
        "tcp_chaff cksum|null|paws|rexmit|seq|syn|<ttl>", /* usage */
        tcp_chaff_open,                                   /* open */
        tcp_chaff_apply,                                  /* apply */
        tcp_chaff_close                                   /* close */
};
