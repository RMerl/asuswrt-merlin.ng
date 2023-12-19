/*
 * mod_ip6_opt.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 */

#include "config.h"
#include "iputil.h"
#include "mod.h"
#include "pkt.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ADDRS 32

#define OPT6_TYPE_ROUTE 1
#define OPT6_TYPE_RAW 2

struct ip6_opt_data_route {
    int segments;
    struct addr addr[MAX_ADDRS];
};

struct ip6_opt_data_raw {
    int len;
    uint8_t proto;
    uint8_t data8[512];
};

struct ip6_opt_data {
    int type;
    union {
        struct ip6_opt_data_route route;
        struct ip6_opt_data_raw raw;
    } u;
};

void *
ip6_opt_close(void *d)
{
    if (d != NULL)
        free(d);
    return (NULL);
}

void *
ip6_opt_open(int argc, char *argv[])
{
    struct ip6_opt_data *opt;

    if (argc < 4)
        return (NULL);

    if ((opt = calloc(1, sizeof(*opt))) == NULL)
        return (NULL);

    if (strcasecmp(argv[1], "route") == 0) {
        int i, j;

        opt->type = OPT6_TYPE_ROUTE;
        if ((opt->u.route.segments = (int)strtol(argv[2], NULL, 10)) < 1 || opt->u.route.segments > MAX_ADDRS) {
            warnx("<segments> must be >= 1");
            return (ip6_opt_close(opt));
        }

        i = 0;
        j = 3;

        if (opt->u.route.segments + 3 != argc) {
            return (ip6_opt_close(opt));
        }

        for (; j < argc; i++, j++) {
            if (addr_aton(argv[j], &opt->u.route.addr[i]) < 0 || opt->u.route.addr[i].addr_type != ADDR_TYPE_IP6) {
                return (ip6_opt_close(opt));
            }
        }
    } else if (strcasecmp(argv[1], "raw") == 0) {
        opt->type = OPT6_TYPE_RAW;

        if (raw_ip6_opt_parse(argc - 2,
                              &argv[2],
                              &opt->u.raw.proto,
                              &opt->u.raw.len,
                              &opt->u.raw.data8[2],
                              sizeof(opt->u.raw.data8) - 2) != 0)
            return (ip6_opt_close(opt));
        opt->u.raw.len += 2;
        opt->u.raw.data8[0] = 0;
        opt->u.raw.data8[1] = opt->u.raw.len / 8;
    } else {
        return (ip6_opt_close(opt));
    }

    return (opt);
}

int
ip6_opt_apply(void *d, struct pktq *pktq)
{
    struct ip6_opt_data *opt = (struct ip6_opt_data *)d;
    struct ip6_ext_hdr *ext;
    int offset, len;
    struct pkt *pkt;
    uint8_t nxt, iph_nxt;
    uint8_t *p;
    int i;

    TAILQ_FOREACH(pkt, pktq, pkt_next)
    {
        uint16_t eth_type = htons(pkt->pkt_eth->eth_type);

        if (eth_type != ETH_TYPE_IPV6) {
            continue;
        }

        nxt = pkt->pkt_ip6->ip6_nxt;
        ext = (struct ip6_ext_hdr *)(((u_char *)pkt->pkt_ip6) + IP6_HDR_LEN);

        if (opt->type == OPT6_TYPE_ROUTE) {
            offset = 8 + IP6_ADDR_LEN * opt->u.route.segments;
            memmove(((u_char *)ext) + offset, ext, pkt->pkt_end - (u_char *)ext);

            pkt->pkt_end += offset;
            pkt->pkt_ip_data += offset;

            len = (IP6_ADDR_LEN / 8) * opt->u.route.segments;

            ext->ext_data.routing.type = 0;
            ext->ext_data.routing.segleft = opt->u.route.segments;
            ((uint32_t *)ext)[1] = 0; /* reserved */

            iph_nxt = IP_PROTO_ROUTING;

            p = (uint8_t *)(ext) + 8;

            for (i = 0; i < opt->u.route.segments; ++i, p += IP6_ADDR_LEN) {
                memcpy(p, opt->u.route.addr[i].addr_data8, IP6_ADDR_LEN);
            }

        } else if (opt->type == OPT6_TYPE_RAW) {
            offset = opt->u.raw.len;
            memmove(((u_char *)ext) + offset, ext, pkt->pkt_end - (u_char *)ext);

            pkt->pkt_end += offset;
            pkt->pkt_ip_data += offset;

            iph_nxt = opt->u.raw.proto;

            p = (uint8_t *)ext;
            memcpy(p, opt->u.raw.data8, opt->u.raw.len);

#if 0
            printf("len: %d, data %02x %02x %02x %02x %02x %02x %02x %02x\n",
                    opt->u.raw.len, opt->u.raw.data8[0], opt->u.raw.data8[1],
                    opt->u.raw.data8[2], opt->u.raw.data8[3],
                    opt->u.raw.data8[4], opt->u.raw.data8[5],
                    opt->u.raw.data8[6], opt->u.raw.data8[7]);
#endif

            len = (opt->u.raw.len - 8) / 8;

        } else {
            continue;
        }

        ext->ext_nxt = nxt;
        ext->ext_len = len;

        pkt->pkt_ip6->ip6_nxt = iph_nxt;
        pkt->pkt_ip6->ip6_plen = htons(htons(pkt->pkt_ip6->ip6_plen) + offset);

        /* XXX: do we need it? */
        pkt_decorate(pkt);
        /* ip6_checksum(pkt->pkt_ip, pkt->pkt_end - pkt->pkt_eth_data); */
    }
    return (0);
}

struct mod mod_ip6_opt = {
        "ip6_opt",                                                                /* name */
        "ip6_opt [route <segments> <ip6-addr> ...] | [raw <type> <byte stream>]", /* usage */
        ip6_opt_open,                                                             /* open */
        ip6_opt_apply,                                                            /* apply */
        ip6_opt_close                                                             /* close */
};
