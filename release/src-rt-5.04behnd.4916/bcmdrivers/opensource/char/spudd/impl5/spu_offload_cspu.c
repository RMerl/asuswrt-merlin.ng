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

#define SPU_OVERFLOW_BIT  0
#define SPU_SOFT_LIMIT_BIT  1
#define SPU_HARD_LIMIT_BIT  2

struct spu_cspu_session_info {
	atomic_t set;
	uint8_t ipv6;
};

struct spu_offload_info
{
	struct gro_cells gro_cells;
	struct spu_cspu_session_info us_info[MAX_SPU_OFFLOAD_SESSIONS/2];
};

static struct spu_offload_info spu_offload_inst;


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
	if (atomic_read(&spu_offload_inst.us_info[parm->session_id].set) != 0)
		pr_err("session already offloaded %d\n", parm->session_id);
}

int spu_platform_offload_us_parm_set(struct bcmspu_offload_parm *parm)
{
	uint8_t *data_p;
	struct spu_offload_parm_args args;
	bcmFun_t *flow_set = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_PROGRAM_PARM);
	uint32_t us_id = parm->session_id & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);

	if (flow_set == NULL)
		return -1;

	if (atomic_read(&spu_offload_inst.us_info[us_id].set) != 0){
		return 0;
	}
	atomic_set(&spu_offload_inst.us_info[us_id].set, 1);
	spu_offload_inst.us_info[us_id].ipv6 = parm->ipv6;

	/* on crossbow, key_size refers to the entire SPU header with keys */
	parm->key_size += FMD_SIZE;

	data_p = &parm->spu_header[parm->key_size];

	/* put seq_hi after the keys for ESN case */
	if (parm->is_esn && parm->is_gcm == 0) {
		parm->key_size += BLOG_ESP_SEQNUM_HI_LEN;
		memcpy(data_p, &parm->seq_hi, BLOG_ESP_SEQNUM_HI_LEN);
		data_p += BLOG_ESP_SEQNUM_HI_LEN;
	}
	/* for GCM, store the IV after the keys */
	if (parm->is_gcm) {
		memcpy(data_p, parm->gcm_iv_aad2, parm->iv_size);
		data_p += parm->iv_size; 
	}

	memcpy(data_p, parm->outer_header, parm->outer_hdr_size);

	args.session_id = parm->session_id;
	args.parm = parm;
	return flow_set(&args);
}

void spu_platform_offload_us_prepend(struct bcmspu_offload_parm *parm, struct spu_offload_prephdr_args *a)
{
	spu_platform_offload_us_parm_set(parm);
	return;
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
		flow_log("invalid session information for %d enc %d\n", session_id, enc);
		return -1;
	}

	if (enc) {
		fcArgs.esp_spi = info.spi;
		fcArgs.esp_ivsize = info.iv_size;
		fcArgs.esp_icvsize = info.digestsize;
		fcArgs.esp_over_udp = info.esp_over_udp;
		devp = iproc_priv.spu_dev_us;
		phyHdr = BLOG_SPU_US;
	} else {
		fcArgs.esp_inner_pkt = 1;
		devp = iproc_priv.spu_dev_ds;
		phyHdr = BLOG_SPU_DS;
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
		offset = (int)(fkb->data - PFKBUFF_TO_PHEAD(fkb));
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
			if (enc) {
				skb_dst_set(skb, dst_clone(info.dst_p));
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
							(*skb->recycle_hook(SKBUFF_2_PNBUFF(skb), skb->recycle_context, skb->recycle_flags);
						goto release_ptrs;
					}
					xfrm_state_hold(info.ptr);
					flow_log("%s:%d held xfrm=%p refcnt=%d\n",
						__func__, __LINE__, info.ptr,
						refcount_read(&((struct xfrm_state *)info.ptr)->refcnt));
   
					sp->xvec[sp->len++] = info.ptr;
				}
#endif
			}
		}
		if (enc) {
			skb_reset_network_header(skb);
			skb->dev = iproc_priv.spu_dev_us;
			xfrm_output_resume(skb, 0);
		} else {
			skb->dev = iproc_priv.spu_dev_ds;
			if (info.ipv6)
				skb->protocol = htons(ETH_P_IPV6);
			else
				skb->protocol = htons(ETH_P_IP);
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
	uint32_t buf_len, padlen, hdr_offset;
	struct spu_offload_prephdr_args prep = {};
	uint32_t us_id = session_id & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);
	int ret = -1, i;
	bcmFun_t *pkt_send = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_US_SEND);

	if(pkt_send) {
		/* revert the outer IP header and padding done by the Blog layer */
		if (IS_SKBUFF_PTR(pNBuff)) {
			struct sk_buff *skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuff);
			pdata = skb->data;
                        buf_len = skb->len;
		} else {
			FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);
			pdata = fkb->data;
			buf_len = fkb->len;
		}
		padlen = pdata[buf_len - digestsize - 2];

		spu_offload_prep_us_ingress(session_id, pdata, &prep, &hdr_offset);
                pdata += (hdr_offset - BLOG_ETH_HDR_LEN);
		buf_len -= (hdr_offset + padlen + BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN + digestsize - BLOG_ETH_HDR_LEN);

		*payloadlen = buf_len - BLOG_ETH_HDR_LEN;

		/* fake an L2 header to meet crossbow requirement */
		/* pass in session id in second byte of the packet */
		pdata[0] = 0;
		pdata[1] = session_id & (MAX_SPU_OFFLOAD_US_SESSIONS - 1);
		for (i=2; i < (2*BLOG_ETH_ADDR_LEN); i++) {
			pdata[i] = i;
		}
		if (spu_offload_inst.us_info[us_id].ipv6 == 0)
			*(uint16_t *)(&pdata[2*BLOG_ETH_ADDR_LEN]) = htons(BLOG_ETH_P_IPV4);
		else
			*(uint16_t *)(&pdata[2*BLOG_ETH_ADDR_LEN]) = htons(BLOG_ETH_P_IPV6);

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

	get_stats(&a);
}

void spu_platform_offloaded(uint32_t *req, uint32_t *cmpl)
{
	struct spu_offload_stats_args a = {};
	bcmFun_t *get_stats = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_STATS);

	if (get_stats == NULL)
		return;

	a.session_id = -1;
	get_stats(&a);
	*req = a.num_request;
	*cmpl = a.num_response;
}

void spu_platform_offload_register(void)
{
	bcmFun_t *cspu_bind = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_OFFLOAD_BIND);

	if (cspu_bind)
		cspu_bind(spu_cspu_rx_handler);
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


