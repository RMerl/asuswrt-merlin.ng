/*
 * fragroute.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 * Copyright (c) 2007-2008 Aaron Turner.
 * Copyright (c) 2013-2022 Fred Klassen - AppNeta
 * $Id$
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include "lib/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIBDNET
/* need to undef these which are pulled in via defines.h, prior to importing dnet.h */
#undef icmp_id
#undef icmp_seq
#undef icmp_data
#undef icmp_mask
#ifdef HAVE_DNET_H
#include <dnet.h>
#endif
#ifdef HAVE_DUMBNET_H
#include <dumbnet.h>
#endif
#endif

#include "fragroute.h"
#include "mod.h"
#include "pkt.h"

void
fragroute_close(fragroute_t *ctx)
{
    assert(ctx);
    free(ctx->pktq);
    free(ctx);
    pkt_close();
}

int
fragroute_process(fragroute_t *ctx, void *buf, size_t len)
{
    struct pkt *pkt;
    assert(ctx);
    assert(buf);

    ctx->first_packet = 0;
    /* save the l2 header of the original packet for later */
    ctx->l2len = get_l2len(buf, (int)len, ctx->dlt);
    memcpy(ctx->l2header, buf, ctx->l2len);

    if ((pkt = pkt_new(len)) == NULL) {
        strcpy(ctx->errbuf, "unable to pkt_new()");
        return -1;
    }

    memcpy(pkt->pkt_data, buf, len);
    pkt->pkt_end = pkt->pkt_data + len;

    pkt_decorate(pkt);

    if (pkt->pkt_ip == NULL) {
        strcpy(ctx->errbuf, "skipping non-IP packet");
        return -1;
    }
    /*  Don't always checksum packets before being fragged
        if (pkt->pkt_eth && htons(pkt->pkt_eth->eth_type) == ETH_TYPE_IP) {
            ip_checksum(pkt->pkt_ip, len);
        }
    */

    TAILQ_INIT(ctx->pktq);
    TAILQ_INSERT_TAIL(ctx->pktq, pkt, pkt_next);

    mod_apply(ctx->pktq);

    return 0;
}

/*
 * keep calling this after fragroute_process() to get all the fragments.
 * Each call returns the fragment length which is stored in **packet.
 * Returns 0 when no more fragments remain or -1 on error
 */
int
fragroute_getfragment(fragroute_t *ctx, char **packet)
{
    static struct pkt *pkt = NULL;
    static struct pkt *next = NULL;
    char *pkt_data = *packet;
    u_int32_t length;

    if (ctx->first_packet != 0) {
        pkt = next;
    } else {
        ctx->first_packet = 1;
        pkt = TAILQ_FIRST(ctx->pktq);
    }

    if (pkt != TAILQ_END(&(ctx->pktq))) {
        next = TAILQ_NEXT(pkt, pkt_next);
        memcpy(pkt_data, pkt->pkt_data, pkt->pkt_end - pkt->pkt_data);

        /* return the original L2 header */
        memcpy(pkt_data, ctx->l2header, ctx->l2len);
        length = pkt->pkt_end - pkt->pkt_data;
        pkt = next;
        return (int)length;
    }

    return 0; // nothing
}

fragroute_t *
fragroute_init(const int mtu, const int dlt, const char *config, char *errbuf)
{
    fragroute_t *ctx;

    if (dlt != DLT_EN10MB) {
        sprintf(errbuf, "Fragroute only supports DLT_EN10MB pcap files");
        return NULL;
    }

    ctx = (fragroute_t *)safe_malloc(sizeof(fragroute_t));
    ctx->pktq = (struct pktq *)safe_malloc(sizeof(struct pktq));
    ctx->dlt = dlt;

    pkt_init(128);

    ctx->mtu = mtu;

    /* parse the config */
    if (mod_open(config, errbuf) < 0) {
        fragroute_close(ctx);
        return NULL;
    }

    return ctx;
}
