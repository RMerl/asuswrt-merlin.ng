/*
<:copyright-BRCM:2023:GPL/GPL:spu

   Copyright (c) 2023 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <net/xfrm.h>
#include <net/gro_cells.h>
#include "spu_blog.h"
#include "linux/bcm_log.h"
#include <linux/gbpm.h>
#include "pktHdr.h"

#define SPU_OVERFLOW_BIT  0
#define SPU_SOFT_LIMIT_BIT  1
#define SPU_HARD_LIMIT_BIT  2

struct spu_cspu_session_info {
	atomic_t set;
};

struct spu_offload_info
{
	struct gro_cells gro_cells;
	struct spu_cspu_session_info us_info[MAX_SPU_OFFLOAD_SESSIONS/2];
};

static struct spu_offload_info spu_offload_inst;
static int crossbow_max_crypto_size = 0;
static struct spu_offload_debug_info debug_info;

int spu_platform_offload_ds_session(int session_id, struct xfrm_state *xfrm,
				 	struct bcmspu_offload_parm *parm)
{
	struct spu_offload_parm_args args;
	bcmFun_t *flow_set = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_PROGRAM_PARM);

	if (flow_set == NULL)
		return -1;

	/* on crossbow, key_size refers to the entire SPU header with keys */
	parm->key_size += FMD_SIZE;

	spin_lock_bh(&xfrm->lock);
	parm->pkts = xfrm->curlft.packets;
	parm->bytes = xfrm->curlft.bytes;
	if (xfrm->replay_esn) {
		uint32_t i, nr, replay_window;

		replay_window = xfrm->replay_esn->replay_window;
		nr = replay_window >> 5;
		for (i = 0; i < nr; i++) {
			parm->bitmap[i] = xfrm->replay_esn->bmp[i];
		}
		parm->replay_size = replay_window;
		parm->long_bitmap = 1;
	} else {
		parm->long_bitmap = 0;
		parm->bitmap[0] = xfrm->replay.bitmap;
	}
	spin_unlock_bh(&xfrm->lock);
	if (parm->data_limit) {
		parm->limits[0] = xfrm->lft.soft_packet_limit;
		parm->limits[1] = xfrm->lft.soft_byte_limit;
		parm->limits[2] = xfrm->lft.hard_packet_limit;
		parm->limits[3] = xfrm->lft.hard_byte_limit;
	}

	args.session_id = parm->session_id;
	args.parm = parm;
	return flow_set(&args);
}

void spu_platform_offload_us_session(int session_id, struct xfrm_state *xfrm,
						struct bcmspu_offload_parm *parm)
{
	uint32_t us_id = parm->session_id & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);
	bcmFun_t *flow_set = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_PROGRAM_PARM);
	struct spu_offload_parm_args args;
	uint8_t *data_p;

	if (flow_set == NULL) {
		pr_err("offload function not supported\n");
		return;
	}

	if (atomic_read(&spu_offload_inst.us_info[us_id].set) != 0)
		pr_err("session already offloaded %d\n", parm->session_id);

	atomic_set(&spu_offload_inst.us_info[us_id].set, 1);
	spin_lock_bh(&xfrm->lock);
	parm->pkts = xfrm->curlft.packets;
	parm->bytes = xfrm->curlft.bytes;
	spin_unlock_bh(&xfrm->lock);

	if (parm->data_limit) {
		parm->limits[0] = xfrm->lft.soft_packet_limit;
		parm->limits[1] = xfrm->lft.soft_byte_limit;
		parm->limits[2] = xfrm->lft.hard_packet_limit;
		parm->limits[3] = xfrm->lft.hard_byte_limit;
	}
	parm->key_size += FMD_SIZE;

	/* put seq_hi after the keys for ESN case */
	if (parm->is_esn && parm->is_gcm == 0) {
		data_p = &parm->spu_header[parm->key_size];
		parm->key_size += BLOG_ESP_SEQNUM_HI_LEN;
		memcpy(data_p, &parm->seq_hi, BLOG_ESP_SEQNUM_HI_LEN);
	}
	args.session_id = parm->session_id;
	args.parm = parm;
	if (flow_set(&args) != 0) {	
		pr_err("offload failing\n");
	}
	return;
}

void spu_platform_offload_us_prepend(struct spu_offload_prephdr_args *a, uint8_t *datap, uint8_t ipv6)
{
	uint32_t word32 = 0;

	if(ipv6) {
		/* extract tos from outer header */
		if (datap)
			word32 = ((BlogIpv6Hdr_t *)datap)->u32[0];
		/* extract tos from blog */
		else if (a->blog_p)
			word32 = a->blog_p->esp6tx_tuple_p->word0;
	} else {
		/* extract tos from outer header */
		if (datap)
			word32 = ((BlogIpv4Hdr_t *)datap)->tos;
		/* extract tos from blog */
		else if (a->blog_p)
			word32 = a->blog_p->esptx_tuple_p->tos;
	}
	a->prepend_word = word32;
	a->prepend_size = sizeof(uint32_t);
}

void spu_platform_offload_free_session(uint32_t session_id)
{
	struct spu_offload_parm_args args;
	bcmFun_t *flow_set = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_PROGRAM_PARM);

	if (session_id >= MAX_SPU_OFFLOAD_SESSIONS/2) {
		uint32_t us_id = session_id & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);
		atomic_set(&spu_offload_inst.us_info[us_id].set, 0);
	}

	if (flow_set) {
		args.parm = NULL;
		args.session_id = session_id;
		flow_set(&args);
	}
}

static int spu_cspu_rx_handler(pNBuff_t pNBuff, int session_id, int enc, int exception)
{
	BlogAction_t blogAction;
	struct sk_buff *skb;
	FkBuff_t *fkb = NULL;
	BlogFcArgs_t fcArgs = {};
	struct spu_offload_rx_info info = {};
	void *recycle_hook;
	void *devp;
	uint32_t phyHdr, offset, len;
	uint8_t *data_p;
	int ret = 0;

	if (exception) {
		int data_limit = 0, overflow = 0;
		flow_log("exception %d on session %d enc %d\n", exception, session_id, enc);
		if (exception & (1 << SPU_OVERFLOW_BIT))
			overflow = 1;
		if (exception & ((1 << SPU_HARD_LIMIT_BIT) | (1 << SPU_SOFT_LIMIT_BIT)))
			data_limit = 1;
		spu_offload_handle_exception((session_id | (enc << 5)), data_limit, overflow);
		return -1;
	}

	if (spu_offload_get_rx_info((session_id | (enc << 5)), &info)) {
		/* session is likely freed already */
		debug_info.invalid_rx++;
		return -1;
	}

	if (enc) {
		fcArgs.esp_spi = info.spi;
		fcArgs.esp_ivsize = info.iv_size;
		fcArgs.esp_icvsize = info.digestsize;
		fcArgs.esp_over_udp = info.esp_over_udp;
		devp = iproc_priv.spu_dev_us;
		phyHdr = BLOG_SPU_US;
		debug_info.slowpath_us++;
	} else {
		fcArgs.esp_inner_pkt = 1;
		fcArgs.esp_ivsize = info.iv_size;
		fcArgs.esp_icvsize = info.digestsize;
		devp = iproc_priv.spu_dev_ds;
		phyHdr = BLOG_SPU_DS;
		debug_info.slowpath_ds++;
	}
	if (IS_SKBUFF_PTR(pNBuff)) {
		skb = PNBUFF_2_SKBUFF(pNBuff);
		data_p = skb->data;
		len = skb->len;
		offset = 0;
		blogAction = blog_sinit(skb, devp, TYPE_IP, info.blog_chan_id, phyHdr, &fcArgs);
	} else {
		fkb = PNBUFF_2_FKBUFF(pNBuff);
		data_p = fkb->data;
		len = fkb->len;
		offset = (int)(fkb->data - BCM_PFKBUFF_TO_PHEAD(fkb));
		skb = NULL;
		recycle_hook = fkb->recycle_hook;
		blogAction = blog_finit(fkb, devp, TYPE_IP, info.blog_chan_id, phyHdr, &fcArgs);
	}
	if (blogAction == PKT_DROP) {
		ret = -1;
		goto release_ptrs;
	}
	if (blogAction == PKT_NORM || blogAction == PKT_BLOG) {
		if (skb == NULL) {
			if (blogAction == PKT_NORM) {
				fkb = fkb_init(data_p, offset, data_p, len);
				fkb->recycle_hook = (RecycleFuncP)recycle_hook;
				fkb->recycle_context = 0;
			}
			skb = nbuff_xlate(pNBuff);
		}
		if (enc) {
			skb_dst_set(skb, dst_clone(info.dst_p));
			skb_reset_network_header(skb);
			skb->dev = iproc_priv.spu_dev_us;
			xfrm_output_resume(skb, 0);
		} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
			skb->sp = secpath_get((struct sec_path *)info.sp);
#else
			{
				struct sec_path *sp = secpath_set(skb);
				if (!sp) {
					pr_err("%s:%d - failed to set secpath for skb %p\n",
						__func__, __LINE__, skb);
					if (skb->recycle_hook)
						(*skb->recycle_hook)(SKBUFF_2_PNBUFF(skb), skb->recycle_context, skb->recycle_flags);
					goto release_ptrs;
				}
				xfrm_state_hold(info.ptr);
				flow_log("%s:%d held xfrm=%p refcnt=%d\n",
					__func__, __LINE__, info.ptr,
					refcount_read(&((struct xfrm_state *)info.ptr)->refcnt));

				sp->xvec[sp->len++] = info.ptr;
			}
#endif
			skb->dev = iproc_priv.spu_dev_ds;
			if ((*data_p & 0xF0) == 0x40)
				skb->protocol = htons(ETH_P_IP);
			else if ((*data_p & 0xF0) == 0x60)
				skb->protocol = htons(ETH_P_IPV6);
			else
				pr_info("unknown pkt type 0x%x\n", *data_p);
			skb_shinfo(skb)->dirty_p = data_p + len;
			gro_cells_receive(&spu_offload_inst.gro_cells, skb);
		}
	}

release_ptrs:
	if (enc) {
		dst_release(info.dst_p);
	} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
		secpath_put(info.sp);
#else
		xfrm_state_put(info.ptr);
#endif
	}
	return 0;
}

int spu_offload_insert_us_pkt(pNBuff_t pNBuff, int session_id, uint32_t digestsize, uint32_t *payloadlen)
{
	uint8_t *pdata;
	uint32_t buf_len, padlen, hdr_offset, fpi_tun_id;
	struct spu_offload_prephdr_args prep = {};
	int ret = -1, i;
	uint8_t nexthdr;
	bcmFun_t *pkt_send = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_US_SEND);

	if(pkt_send) {
		/* revert the outer IP header and padding done by the Blog layer */
		if (IS_SKBUFF_PTR(pNBuff)) {
			struct sk_buff *skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuff);
			pdata = skb->data;
                        buf_len = skb->len;
			if (skb_is_nonlinear(skb)) {
				if(skb_linearize(skb)) {
					debug_info.invalid_insert++;
					return -1;
				}
			}
		} else {
			FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);
			pdata = fkb->data;
			buf_len = fkb->len;
		}
		padlen = pdata[buf_len - digestsize - 2];
		nexthdr = pdata[buf_len - digestsize - 1];

		/* extract packet tos from outer header */
		spu_platform_offload_us_prepend(&prep, pdata, (nexthdr == BLOG_IPPROTO_IPV6));

		spu_offload_us_hdr_offset(session_id, &hdr_offset, &fpi_tun_id);
                pdata += (hdr_offset - BLOG_ETH_HDR_LEN);
		buf_len -= (hdr_offset + padlen + BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN + digestsize - BLOG_ETH_HDR_LEN);

		*payloadlen = buf_len - BLOG_ETH_HDR_LEN;

		/* fake an L2 header to meet crossbow requirement */
		/* pass in session id in second byte of the packet */
		pdata[0] = 0;
#if IS_ENABLED(CONFIG_BCM_FPI)
		pdata[1] = fpi_tun_id & (MAX_SPU_OFFLOAD_US_SESSIONS - 1);
#else
		pdata[1] = session_id & (MAX_SPU_OFFLOAD_US_SESSIONS - 1);
#endif
		if (*payloadlen > crossbow_max_crypto_size) {
			debug_info.invalid_insert++;
			return -1;
		}
		for (i=2; i < (2*BLOG_ETH_ADDR_LEN); i++) {
			pdata[i] = i;
		}
		if (nexthdr == IPPROTO_IPIP) {
			BlogIpv4Hdr_t *ipv4_p = (BlogIpv4Hdr_t *)&pdata[BLOG_ETH_HDR_LEN];
			uint16_t csum16 = ipv4_p->chkSum;
			uint16_t old16 = ipv4_p->u16[4];

			*(uint16_t *)(&pdata[2*BLOG_ETH_ADDR_LEN]) = htons(BLOG_ETH_P_IPV4);
			ipv4_p->ttl++; /* revert ttl decrement done by Linux */

			ipv4_p->chkSum = _compute_icsum16(csum16, old16, ipv4_p->u16[4]);

		} else if (nexthdr == BLOG_IPPROTO_IPV6) {
			BlogIpv6Hdr_t *ipv6_p = (BlogIpv6Hdr_t *)&pdata[BLOG_ETH_HDR_LEN];

			*(uint16_t *)(&pdata[2*BLOG_ETH_ADDR_LEN]) = htons(BLOG_ETH_P_IPV6);
			ipv6_p->hopLmt++; /* revert hopLmt decrement done by Linux */
		} else {
			debug_info.invalid_insert++;
			return ret;
		}

		/* put prepend info for the outer IP header at the end of the packet */
		memcpy(&pdata[buf_len], &prep.prepend_word, prep.prepend_size);
		buf_len += prep.prepend_size;

		if (IS_SKBUFF_PTR(pNBuff)) {
			struct sk_buff *skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuff);
			skb->data = pdata;
			skb_trim(skb, buf_len);
		} else {
			FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);
			fkb->data = pdata;
			fkb->len = buf_len;
		}

		pkt_send(pNBuff);

		ret = 0;
	}
	return ret;
}

void spu_platform_offload_stats(uint32_t session_id, struct spu_offload_tracker *curr,
				uint32_t *bitmap, uint8_t long_bitmap)
{
	struct spu_offload_stats_args a;
	bcmFun_t *get_stats = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_STATS);

	if (get_stats == NULL)
		return;

	a.session_id = session_id;
	a.stats = curr;
        a.bitmap = bitmap;

	if(get_stats(&a) != 0)
		pr_err("offload stats for %d invalid\n", session_id);
}

void spu_platform_offloaded(struct spu_offload_debug_info *info_p)
{
	struct spu_offload_stats_args a = {};
	bcmFun_t *get_stats = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_STATS);

	if (get_stats == NULL) {
		pr_info("offload stats not available\n");
		return;
	}
	a.session_id = -1;
	get_stats(&a);
	debug_info.requests = a.num_request;
	debug_info.complete = a.num_response;
	debug_info.icv_err = a.icv_err;
	debug_info.seqno_err = a.seqno_err;

	memcpy(info_p, &debug_info, sizeof(struct spu_offload_debug_info));
}

void spu_platform_offload_tun_map(uint32_t tun_id, int32_t session_id)
{
	bcmFun_t *tun_map = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_TUNNEL_MAP);
	struct spu_fpi_tun_map a;

	if (tun_map) {
		a.tun_id = tun_id;
		a.session_id = session_id;
		tun_map(&a);
	}
}

void spu_platform_offload_copy_fwd(uint32_t session_id2, uint32_t session_id1)
{
	bcmFun_t *sync_fwd = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_SYNC_FORWARD);
	struct spu_fpi_tun_map a;

	if (sync_fwd) {
		a.session_id1 = session_id1;
		a.session_id2 = session_id2;
		sync_fwd(&a);
	}
}

void spu_platform_offload_register(void)
{
	bcmFun_t *cspu_bind = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_OFFLOAD_BIND);
        int ret;

        flow_offload_enable = 0;
	if (cspu_bind) {
                ret = cspu_bind(spu_cspu_rx_handler);
		if (ret > 0) {
			crossbow_max_crypto_size = ret;
			pr_info("Crossbow SPU offload, max crypto len is %d\n", crossbow_max_crypto_size);
			/* crossbow cspu offload is enabled */
			flow_offload_enable = 3;
		}
	}
}

void spu_platform_offload_deregister(void)
{
	bcmFun_t *cspu_unbind = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_OFFLOAD_UNBIND);

	if (cspu_unbind)
		cspu_unbind(NULL);
}

int spu_platform_offload_init(void)
{
	return 0;
}

int spu_offload_postinit(void)
{
	int ret = gro_cells_init(&spu_offload_inst.gro_cells, iproc_priv.spu_dev_ds);
	if (ret)
		spu_offload_inst.gro_cells.cells = NULL;
	return ret;
}

void spu_offload_deinit(void)
{
	gro_cells_destroy(&spu_offload_inst.gro_cells);
	spu_offload_inst.gro_cells.cells = NULL;
}


