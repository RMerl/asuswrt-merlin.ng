/*
 * DHD nbuff utilities - header file
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
 * $Id: dhd_nbuff.h 613984 2016-01-20 19:24:03Z $
 */

#ifndef __DHD_NBUFF_H__
#define __DHD_NBUFF_H__

#if defined(BCM_NBUFF)

#ifdef mips
#undef ABS
#endif

#include <linux/nbuff.h>
#include <dngl_stats.h>
#include <dhd.h>

#ifdef DSLCPE
/* dhd pkttags pool size.
	Check FKBC clone pool(2080) to have more that this when you change it to bigger num.
*/
#define DHD_PKTTAGS_POOL_SIZE	(1280)
#define  DHD_PKTTAG_ENTRY_SIZE	(sizeof(dhd_pkttag_fd_t))
extern uint g_dhd_pkttags_pool_free;
extern dhd_pkttag_fd_t *g_dhd_pkttags_pool_free_p;
extern dhd_pkttag_fd_t *g_dhd_pkttags_pool_p;
extern struct list_head  dhd_pkttags_pool_head;
extern spinlock_t  g_dhd_pkttags_pool_lock;

extern void inline osl_pkt_clear_tag(void *pkt);

static inline void
DHD_PKTTAGS_POOL_PUT(void *tag)
{
	dhd_pkttag_fd_t *tag_p = (dhd_pkttag_fd_t *)tag;
	spin_lock_bh(&g_dhd_pkttags_pool_lock);
	tag_p->list = g_dhd_pkttags_pool_free_p;
	g_dhd_pkttags_pool_free_p = tag_p;
	g_dhd_pkttags_pool_free++;
	spin_unlock_bh(&g_dhd_pkttags_pool_lock);
}

static inline dhd_pkttag_fd_t *
DHD_PKTTAGS_POOL_GET(void)
{
	dhd_pkttag_fd_t *ret_p = NULL;
	spin_lock_bh(&g_dhd_pkttags_pool_lock);
	if (g_dhd_pkttags_pool_free_p != NULL)
	{
		ret_p = g_dhd_pkttags_pool_free_p;
		g_dhd_pkttags_pool_free_p = g_dhd_pkttags_pool_free_p->list;
		g_dhd_pkttags_pool_free--;
		ret_p->list = NULL;
	}
	spin_unlock_bh(&g_dhd_pkttags_pool_lock);
	return ret_p;
}
#endif


/* PKTLIST */
void dhd_pkt_queue_purge(osl_t *osh, PKT_LIST *list);
void *dhd_pkt_dequeue(PKT_LIST *list);
void dhd_pkt_unlink(PKT_LIST *list, void *pkt);
void dhd_pkt_queue_head(PKT_LIST *list,	void *newpkt);
void dhd_pkt_queue_head_init(PKT_LIST *list);

extern void dhd_nbuff_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf);

#endif /* BCM_NBUFF */

#endif /* __DHD_NBUFF_H__ */
