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

struct spu_offload_info
{
	struct gro_cells gro_cells;
};

static struct spu_offload_info spu_offload_inst;

int spu_platform_offload_session_ds_parm(int session_id, struct xfrm_state *xfrm,
					  struct bcmspu_offload_parm *parm)
{
	printk("ds offloading not supported yet\n");
	return -1;
}

int spu_platform_offload_session_us_parm(int session_id, struct xfrm_state *xfrm,
					  struct bcmspu_offload_parm *parm)
{
	uint8_t *data_p;
	struct spu_offload_parm_args args;
	bcmFun_t *flow_set = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_PROGRAM_PARM);

	if (flow_set == NULL)
		return -1;

	/* TODO */
	if (parm->is_esn || parm->esp_o_udp || parm->data_limit)
		return -1;

	/* on crossbow, key_size refers to the entire SPU header with keys */
	parm->key_size += FMD_SIZE;
	if (parm->is_esn)
		parm->key_size += BLOG_ESP_SEQNUM_HI_LEN;

	/* add the outer header after the SPU header */
	data_p = &parm->spu_header[parm->key_size];
	parm->outer_hdr_size = BLOG_IPV4_HDR_LEN;
	if (parm->esp_o_udp)
		parm->outer_hdr_size += BLOG_UDP_HDR_LEN;
	memcpy(data_p, parm->outer_header, parm->outer_hdr_size);

	data_p += parm->outer_hdr_size;
	/* add the ESP header after the outer header */
	*((uint32_t *)data_p) = parm->esp_spi;
        data_p += BLOG_ESP_SPI_LEN;
	*((uint32_t *)data_p) = 0;

	spin_lock_bh(&xfrm->lock);
	parm->pkts = xfrm->curlft.packets;
	parm->bytes = xfrm->curlft.bytes;
	spin_unlock_bh(&xfrm->lock);

	args.session_id = parm->session_id;
	args.parm = parm;
	return flow_set(&args);
}

void spu_platform_offload_free_session(uint32_t session_id)
{
	struct spu_offload_parm_args args;
	bcmFun_t *flow_set = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_PROGRAM_PARM);

	if (flow_set) {
		args.parm = NULL;
		args.session_id = session_id;
		flow_set(&args);
	}
}

static int spu_cspu_rx_handler(FkBuff_t *fkb, int session_id, int enc)
{
	BlogAction_t blogAction;
	struct sk_buff *skb;
	FkBuff_t *fkb;
	FkBuff_t *fkb = NULL;
	BlogFcArgs_t fcArgs = {};
	struct spu_offload_rx_info info = {};
	void *recycle_hook;
	void *devp;
	/* save fkb details in case have to recreate */
	uint8_t *data_p = fkb->data;
	uint32_t offset = fkb->data - (uint8_t *)fkb;
	uint32_t len = fkb->len;
	void * hook = (void *)fkb->recycle_hook;
	int ret = 0;

	if (spu_offload_get_rx_info(session_id | (enc << 5)), &info) {
		/* session is not available */
		flow_log("invalid session information for %d enc %d\n", session_id, enc);
		return -1;
	}

	fcArgs.esp_spi = info.spi;
	fcArgs.esp_ivsize = info.iv_size;
	fcArgs.esp_icvsize = info.digestsize;

	blogAction = blog_finit(fkb, iproc_priv.spu_dev_us, TYPE_IP,
				ctx->blog_chan_id, BLOG_SPU_US, &fcArgs);

	if (blogAction == PKT_DROP) {
		ret = -1;
		goto release_ptrs;
	}

	if (blogAction != PKT_DONE) {
		if (blogAction == PKT_NORM) {
			fkb = fkb_init(data_p, offset - PFKBUFF_PHEAD_OFFSET, data_p, len);
			fkb->recycle_hook = (RecycleFuncP)hook;
			fkb->recycle_context = 0;
		}
		skb = nbuff_xlate(FKBUFF_2_PNBUFF(fkb));

		skb_dst_set(skb, dst_clone(info.dst_p));
		skb_reset_network_header(skb);
		skb->dev = iproc_priv.spu_dev_us;
		xfrm_output_resume(skb, 0):
	}
release_ptrs:
	if (enc) {
		dst_release(info.dst_p);
	}

	return 0;
}

int spu_offload_insert_us_pkt(pNBuff_t pNBuff, int session_id, uint32_t digestsize, uint32_t *payloadlen)
{
	uint8_t *pdata;
	uint32_t padlen;
	int ret = -1, i;
	bcmFun_t *pkt_send = bcmFun_get(BCM_FUN_ID_ARCHER_CRYPTO_US_SEND);
	FkBuff_t *fkb;

	if(pkt_send) {
		/* revert the outer IP header and padding done by the Blog layer */
		if (IS_SKBUFF_PTR(pNBuff)) {
			struct sk_buff *skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuff);
			RecycleFuncP funcp = skb->recycle_hook;
			fkb = fkb_init(skb->head, 0, skb->data, skb->len);
			fkb->recycle_hook = funcp;
			fkb->recycle_context = 0;

			gbpm_free_skb(skb);
		} else {
			fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuff);
		}
		pdata = fkb->data;
		padlen = pdata[fkb->len - digestsize - 2];

		fkb->data = (pdata + BLOG_IPV4_HDR_LEN + CCM_AES_IV_SIZE + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN);
		fkb->len -= (BLOG_IPV4_HDR_LEN + CCM_AES_IV_SIZE + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN +
                             padlen + BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN + digestsize);

		*payloadlen = fkb->len;

		/* fake an L2 header to meet crossbow requirement */
		fkb->data -= BLOG_ETH_HDR_LEN;
		fkb->len += BLOG_ETH_HDR_LEN;
		*(uint16_t *)(&fkb->data[2*BLOG_ETH_ADDR_LEN]) = htons(BLOG_ETH_P_IPV4);

		/* pass in session id in second byte of the packet */
		fkb->data[1] = session_id & (MAX_SPU_OFFLOAD_US_SESSIONS - 1);
		pkt_send(FKBUFF_2_PNBUFF(fkb));

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

	get_stats(&a);
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


