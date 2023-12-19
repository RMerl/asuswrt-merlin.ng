/*
 * mod_ip_frag.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include "lib/queue.h"
#include "mod.h"
#include "pkt.h"
#include "randutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define FAVOR_OLD 1
#define FAVOR_NEW 2

static int ip_frag_apply_ipv4(void *d, struct pktq *pktq);

static int ip_frag_apply_ipv6(void *d, struct pktq *pktq);

static struct ip_frag_data {
    rand_t *rnd;
    int size;
    int overlap;
    uint32_t ident;
} ip_frag_data;

void *
ip_frag_close(_U_ void *d)
{
    if (ip_frag_data.rnd != NULL)
        rand_close(ip_frag_data.rnd);
    ip_frag_data.size = 0;
    return (NULL);
}

void *
ip_frag_open(int argc, char *argv[])
{
    if (argc < 2) {
        warn("need fragment <size> in bytes");
        return (NULL);
    }
    ip_frag_data.rnd = rand_open();
    ip_frag_data.size = (int)strtol(argv[1], NULL, 10);

    if (ip_frag_data.size == 0 || (ip_frag_data.size % 8) != 0) {
        warn("fragment size must be a multiple of 8");
        return (ip_frag_close(&ip_frag_data));
    }
    if (argc == 3) {
        if (strcmp(argv[2], "old") == 0 || strcmp(argv[2], "win32") == 0)
            ip_frag_data.overlap = FAVOR_OLD;
        else if (strcmp(argv[2], "new") == 0 || strcmp(argv[2], "unix") == 0)
            ip_frag_data.overlap = FAVOR_NEW;
        else
            return (ip_frag_close(&ip_frag_data));
    }

    ip_frag_data.ident = rand_uint32(ip_frag_data.rnd);

    return (&ip_frag_data);
}

int
ip_frag_apply(void *d, struct pktq *pktq)
{
    struct pkt *pkt;

    /* Select eth protocol via first packet in queue: */
    pkt = TAILQ_FIRST(pktq);
    if (pkt != TAILQ_END(pktq)) {
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        if (eth_type == ETH_TYPE_IP) {
            ip_frag_apply_ipv4(d, pktq);
        } else if (eth_type == ETH_TYPE_IPV6) {
            ip_frag_apply_ipv6(d, pktq);
        }
        return 0;
    }
    return 0;
}

static int
ip_frag_apply_ipv4(_U_ void *d, struct pktq *pktq)
{
    struct pkt *pkt, *new, *next;
    int hl, fraglen, off;
    u_char *p, *p1, *p2;

    for (pkt = TAILQ_FIRST(pktq); pkt != TAILQ_END(pktq); pkt = next) {
        next = TAILQ_NEXT(pkt, pkt_next);

        if (pkt->pkt_ip == NULL || pkt->pkt_ip_data == NULL)
            continue;

        hl = pkt->pkt_ip->ip_hl << 2;

        /*
         * Preserve transport protocol header in first frag,
         * to bypass filters that block `short' fragments.
         */
        switch (pkt->pkt_ip->ip_p) {
        case IP_PROTO_ICMP:
            fraglen = MAX(ICMP_LEN_MIN, ip_frag_data.size);
            break;
        case IP_PROTO_UDP:
            fraglen = MAX(UDP_HDR_LEN, ip_frag_data.size);
            break;
        case IP_PROTO_TCP:
            fraglen = MAX(pkt->pkt_tcp->th_off << 2, ip_frag_data.size);
            break;
        default:
            fraglen = ip_frag_data.size;
            break;
        }
        if (fraglen & 7)
            fraglen = (fraglen & ~7) + 8;

        if (pkt->pkt_end - pkt->pkt_ip_data < fraglen)
            continue;

        for (p = pkt->pkt_ip_data; p < pkt->pkt_end;) {
            new = pkt_new(pkt->pkt_buf_size);
            memcpy(new->pkt_eth, pkt->pkt_eth, (u_char *)pkt->pkt_eth_data - (u_char *)pkt->pkt_eth);
            memcpy(new->pkt_ip, pkt->pkt_ip, hl);
            new->pkt_ip_data = new->pkt_eth_data + hl;

            p1 = p, p2 = NULL;
            off = (p - pkt->pkt_ip_data) >> 3;

            if (ip_frag_data.overlap != 0 && (off & 1) != 0 && p + (fraglen << 1) < pkt->pkt_end) {
                struct pkt tmp;
                u_char tmp_buf[pkt->pkt_buf_size];

                tmp.pkt_buf = tmp_buf;
                tmp.pkt_buf_size = pkt->pkt_buf_size;
                rand_strset(ip_frag_data.rnd, tmp.pkt_buf, fraglen);
                if (ip_frag_data.overlap == FAVOR_OLD) {
                    p1 = p + fraglen;
                    p2 = tmp.pkt_buf;
                } else if (ip_frag_data.overlap == FAVOR_NEW) {
                    p1 = tmp.pkt_buf;
                    p2 = p + fraglen;
                }
                new->pkt_ip->ip_off = htons(IP_MF | (off + (fraglen >> 3)));
            } else {
                new->pkt_ip->ip_off = htons(off | ((p + fraglen < pkt->pkt_end) ? IP_MF : 0));
            }
            new->pkt_ip->ip_len = htons(hl + fraglen);
            ip_checksum(new->pkt_ip, hl + fraglen);

            memcpy(new->pkt_ip_data, p1, fraglen);
            new->pkt_end = new->pkt_ip_data + fraglen;
            TAILQ_INSERT_BEFORE(pkt, new, pkt_next);

            if (p2 != NULL) {
                new = pkt_dup(new);
                new->pkt_ts.tv_usec = 1;
                new->pkt_ip->ip_off = htons(IP_MF | off);
                new->pkt_ip->ip_len = htons(hl + (fraglen << 1));
                ip_checksum(new->pkt_ip, hl + (fraglen << 1));

                memcpy(new->pkt_ip_data, p, fraglen);
                memcpy(new->pkt_ip_data + fraglen, p2, fraglen);
                new->pkt_end = new->pkt_ip_data + (fraglen << 1);
                TAILQ_INSERT_BEFORE(pkt, new, pkt_next);
                p += (fraglen << 1);
            } else
                p += fraglen;

            if ((fraglen = pkt->pkt_end - p) > ip_frag_data.size)
                fraglen = ip_frag_data.size;
        }
        TAILQ_REMOVE(pktq, pkt, pkt_next);
        pkt_free(pkt);
    }
    return (0);
}

static int
ip_frag_apply_ipv6(_U_ void *d, struct pktq *pktq)
{
    struct pkt *pkt, *new, *next;
    struct ip6_ext_hdr *ext;
    int hl, fraglen, off;
    u_char *p, *p1, *p2;
    uint8_t next_hdr;

    ip_frag_data.ident++;

    for (pkt = TAILQ_FIRST(pktq); pkt != TAILQ_END(pktq); pkt = next) {
        next = TAILQ_NEXT(pkt, pkt_next);

        if (pkt->pkt_ip == NULL || pkt->pkt_ip_data == NULL)
            continue;

        hl = IP6_HDR_LEN;

        /*
         * Preserve transport protocol header in first frag,
         * to bypass filters that block `short' fragments.
         */
        switch (pkt->pkt_ip->ip_p) {
        case IP_PROTO_ICMP:
            fraglen = MAX(ICMP_LEN_MIN, ip_frag_data.size);
            break;
        case IP_PROTO_UDP:
            fraglen = MAX(UDP_HDR_LEN, ip_frag_data.size);
            break;
        case IP_PROTO_TCP:
            fraglen = MAX(pkt->pkt_tcp->th_off << 2, ip_frag_data.size);
            break;
        default:
            fraglen = ip_frag_data.size;
            break;
        }
        if (fraglen & 7)
            fraglen = (fraglen & ~7) + 8;

        if (pkt->pkt_end - pkt->pkt_ip_data < fraglen)
            continue;

        next_hdr = pkt->pkt_ip6->ip6_nxt;

        for (p = pkt->pkt_ip_data; p < pkt->pkt_end;) {
            new = pkt_new(pkt->pkt_buf_size);
            memcpy(new->pkt_eth, pkt->pkt_eth, (u_char *)pkt->pkt_eth_data - (u_char *)pkt->pkt_eth);
            memcpy(new->pkt_ip, pkt->pkt_ip, hl);
            ext = (struct ip6_ext_hdr *)((u_char *)new->pkt_eth_data + hl);
            new->pkt_ip_data = (u_char *)(ext) + 2 + sizeof(struct ip6_ext_data_fragment);
            new->pkt_ip6->ip6_nxt = IP_PROTO_FRAGMENT;

            ext->ext_nxt = next_hdr;
            ext->ext_len = 0; /* ip6 fragf reserved */
            ext->ext_data.fragment.ident = ip_frag_data.ident;

            p1 = p, p2 = NULL;
            off = (p - pkt->pkt_ip_data) >> 3;

            if (ip_frag_data.overlap != 0 && (off & 1) != 0 && p + (fraglen << 1) < pkt->pkt_end) {
                struct pkt tmp;
                u_char tmp_buf[pkt->pkt_buf_size];

                tmp.pkt_buf = tmp_buf;
                tmp.pkt_buf_size = pkt->pkt_buf_size;
                rand_strset(ip_frag_data.rnd, tmp.pkt_buf, fraglen);
                if (ip_frag_data.overlap == FAVOR_OLD) {
                    p1 = p + fraglen;
                    p2 = tmp.pkt_buf;
                } else if (ip_frag_data.overlap == FAVOR_NEW) {
                    p1 = tmp.pkt_buf;
                    p2 = p + fraglen;
                }
                ext->ext_data.fragment.offlg = htons((off /*+ (fraglen >> 3)*/) << 3) | IP6_MORE_FRAG;
            } else {
                ext->ext_data.fragment.offlg = htons(off << 3) | ((p + fraglen < pkt->pkt_end) ? IP6_MORE_FRAG : 0);
            }
            new->pkt_ip6->ip6_plen = htons(fraglen + 8);

            memcpy(new->pkt_ip_data, p1, fraglen);
            new->pkt_end = new->pkt_ip_data + fraglen;
            TAILQ_INSERT_BEFORE(pkt, new, pkt_next);

            if (p2 != NULL) {
                new = pkt_dup(new);
                new->pkt_ts.tv_usec = 1;

                ext->ext_data.fragment.offlg = htons(off << 3) | IP6_MORE_FRAG;
                new->pkt_ip6->ip6_plen = htons((fraglen << 1) + 8);

                memcpy(new->pkt_ip_data, p, fraglen);
                memcpy(new->pkt_ip_data + fraglen, p2, fraglen);
                new->pkt_end = new->pkt_ip_data + (fraglen << 1);
                TAILQ_INSERT_BEFORE(pkt, new, pkt_next);
                p += (fraglen << 1);
            } else {
                p += fraglen;
            }

            if ((fraglen = pkt->pkt_end - p) > ip_frag_data.size)
                fraglen = ip_frag_data.size;
        }
        TAILQ_REMOVE(pktq, pkt, pkt_next);
        pkt_free(pkt);
    }
    return 0;
}

struct mod mod_ip_frag = {
        "ip_frag",                  /* name */
        "ip_frag <size> [old|new]", /* usage */
        ip_frag_open,               /* open */
        ip_frag_apply,              /* apply */
        ip_frag_close               /* close */
};
