/*
 * DHD nbuff (fkb/skb) utilities
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_nbuff.c 619878 2016-02-18 19:37:44Z $
 */

#if defined(BCM_NBUFF)

#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <linux_osl.h>

#include <dhd_nbuff.h>

/* for fkb/multicast pool use */
DEFINE_SPINLOCK(g_dhd_pkttags_pool_lock);
uint g_dhd_pkttags_pool_free = 0;
dhd_pkttag_fd_t *g_dhd_pkttags_pool_free_p = NULL;
dhd_pkttag_fd_t *g_dhd_pkttags_pool_p = NULL;

/* PKTLIST */

void dhd_pkt_queue_head_init(PKT_LIST *list)
{
	dll_init(list);
}

void dhd_pkt_queue_head(PKT_LIST *list,	void *newpkt)
{
	(DHD_PKTTAG_FD(newpkt))->pkt = newpkt;
	dll_append(list, &(DHD_PKTTAG_FD(newpkt)->node));
}

void dhd_pkt_unlink(PKT_LIST *list, void *pkt)
{
	dll_delete(&(DHD_PKTTAG_FD(pkt)->node));
}

void *dhd_pkt_dequeue(PKT_LIST *list)
{
	void *pkt = NULL;
	dll_t *item;

	item = dll_head_p(list);
	if (item) {
		pkt = ((dhd_pkttag_fd_t*)item)->pkt;
		dll_delete(dll_head_p(list));
	}
	return pkt;
}

void dhd_pkt_queue_purge(osl_t *osh, PKT_LIST *list)
{
	dll_t *item, *next;
	void *pkt;

	for (item = dll_head_p(list); !dll_end(list, item); item = next) {
		next = dll_next_p(item);
		pkt = ((dhd_pkttag_fd_t*)item)->pkt;
		PKTFREE(osh, pkt, FALSE);
	}
}

#ifdef DSLCPE
/* Add nbuff dump output to a buffer */
void
dhd_nbuff_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf)
{
	bcm_bprintf(strbuf, "tx_nodup %lu\n", PKT_NODUP_CNT(dhdp->osh));
	bcm_bprintf(strbuf, "tx_tag_nobuf %lu free tags in the pool %lu\n",
		PKT_PKTTAG_NOBUF_CNT(dhdp->osh), g_dhd_pkttags_pool_free);

	return;
}
#endif

#endif /* BCM_NBUFF */
