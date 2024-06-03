/*
<:copyright-BRCM:2020:GPL/GPL:spu

   Copyright (c) 2020 Broadcom 
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <crypto/algapi.h>
#include <linux/rtnetlink.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/md5.h>
#include <crypto/scatterwalk.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <linux/ip.h>
#include <linux/if_arp.h>
#include <net/xfrm.h>
#include <net/dst.h>
#include <linux/bcm_version_compat.h>
#include <bcmspudrv.h>
#include <pktHdr.h>
#include <linux/platform_device.h>
#include "spu.h"
#include "cipher.h"
#include "util.h"
#include "spu_blog.h"
#include "linux/bcm_log.h"

static rwlock_t spu_blog_offload_lock;
#define SESSION_ALLOCATED	0x1
#define SESSION_DEFINED		0x2

struct spu_blog_offload_session {
	atomic_t taken;
	atomic_t ref_count;
	atomic_t evict;
	union {
		struct sec_path *sp;
		struct dst_entry *dst_p;
		void * ptr;
	};
	struct bcmspu_offload_parm parm;

	struct spu_offload_tracker last_update;
};

/* platform specific session memory is managed separatedly */
struct spu_offload_state {
	int next_ds;
	int next_us;

	wait_queue_head_t  evict_wqh;
	struct task_struct *evict_thread;
	atomic_t work_avail;

	struct spu_blog_offload_session  sessions[MAX_SPU_OFFLOAD_SESSIONS];
};

static struct spu_offload_state state_g;

#define SPU_WAKEUP_EVICTHDL() do { \
		wake_up_interruptible(&state_g.evict_wqh); \
	} while (0)

static void spu_update_xfrm_stats(int i, struct xfrm_state *xfrm, int xlocked);

static inline struct iproc_ctx_s *spu_blog_lookup_ctx_byspi(uint32_t spi, enum spu_stream_type stream)
{
	struct iproc_ctx_s *ctx;
	struct iproc_ctx_s *rctx = NULL;

	flow_log("%s\n", __func__);

	read_lock(&iproc_priv.ctxListLock[stream]);
	list_for_each_entry(ctx, &iproc_priv.ctxList[stream], entry) 
	{
		if (ctx->spi == spi)
		{
			rctx = ctx;
			break;
		}
	}
	read_unlock(&iproc_priv.ctxListLock[stream]);
	return rctx;
}  /* spu_blog_lookup_ctx_byspi() */

static inline struct xfrm_state *spu_lookup_offload_xfrm(uint32_t id)
{
	struct xfrm_state *xfrm = NULL;
	struct spu_blog_offload_session *session;

	read_lock (&spu_blog_offload_lock);
	session = &state_g.sessions[id];

	if (atomic_read(&session->taken) != SESSION_DEFINED)
		goto exit;

	if (session->parm.is_enc == 0) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
		if (session->sp && (session->sp->len == 1))
			xfrm = session->sp->xvec[session->sp->len-1];
#else
		xfrm = session->ptr;
#endif
	}
	else {
		xfrm = session->dst_p->xfrm;
	}
exit:
	read_unlock (&spu_blog_offload_lock);
	return xfrm;
}

static int create_spu_header(uint8_t *hdr_buff, struct iproc_ctx_s *ctx, Blog_t *blog_p,
			     uint8_t is_encrypt, bool is_esn, uint32_t seq_hi)
{
	uint32_t hdr_size = 0;
	struct spu_request_opts req_opts;
	struct spu_cipher_parms cipher_parms;
	struct spu_hash_parms hash_parms;
	struct spu_aead_parms aead_parms;
	struct spu_hw *spu = &iproc_priv.spu;
	struct SPU2_FMD *fmd = (struct SPU2_FMD *)hdr_buff;

	/* fill in the parameters will be used by the accelerator
	 * create the static header for the flow */
	memset(&req_opts, 0, sizeof(req_opts));
	memset(&hash_parms, 0, sizeof(hash_parms));
	memset(&aead_parms, 0, sizeof(aead_parms));
	memset(&cipher_parms, 0, sizeof(cipher_parms));

	req_opts.is_inbound = !is_encrypt; // inboud = DS = decrypt
	req_opts.auth_first = ctx->auth_first;
	req_opts.is_aead = true;
	req_opts.is_esp = ctx->is_esp;
	req_opts.ipsec_esn = is_esn;
	req_opts.seq_hi = seq_hi;

	cipher_parms.alg = ctx->cipher.alg;
	cipher_parms.mode = ctx->cipher.mode;
	cipher_parms.type = ctx->cipher_type;
	cipher_parms.key_buf = ctx->enckey;
	cipher_parms.key_len = ctx->enckeylen;
	cipher_parms.iv_len = blog_p->esptx.ivsize;

	hash_parms.alg = ctx->auth.alg;
	hash_parms.mode = ctx->auth.mode;
	hash_parms.type = HASH_TYPE_NONE;
	hash_parms.key_buf = (u8 *)ctx->authkey;
	hash_parms.key_len = ctx->authkeylen;
	hash_parms.digestsize = ctx->digestsize;

	if ((ctx->auth.alg == HASH_ALG_SHA224) &&
	    (ctx->authkeylen < SHA224_DIGEST_SIZE))
		hash_parms.key_len = SHA224_DIGEST_SIZE;

	aead_parms.assoc_size = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN;
	if (is_encrypt == 0)
		aead_parms.assoc_size += blog_p->esptx.ivsize;
	else
		aead_parms.return_iv = true;

	if (ctx->auth.alg == HASH_ALG_AES)
		hash_parms.type = (enum hash_type)ctx->cipher_type;

	hdr_size = spu->spu_create_request(hdr_buff, &req_opts,
					   &cipher_parms, &hash_parms,
					   &aead_parms, ctx->digestsize);
	// NOTE: CSPU need to return AAD2 for US
#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE))
	fmd->ctrl1 &= ~(SPU2_RETURN_MD | SPU2_RETURN_AAD2);
#else
	fmd->ctrl1 &= ~(SPU2_RETURN_MD);
	if (is_encrypt == 0)
		fmd->ctrl1 &= ~(SPU2_RETURN_AAD2);
#endif

	if (is_encrypt) {
		/* when SPU interaction is offloaded to Runner,
		 * the SPU HW is used for padding and IV generation as well */

		/* enable padding */
		fmd->ctrl0 |= (SPU2_CIPH_PAD_EN | ((uint64_t)4 << SPU2_CIPH_PAD_SHIFT));
		if (is_esn == false)
			fmd->ctrl0 |= (SPU2_PROSEL_IPSEC << SPU2_PROTO_SEL_SHIFT);
			
		fmd->ctrl1 |= SPU2_GENIV; 
		fmd->ctrl1 &= ~SPU2_IV_LEN;
	}
	return hdr_size;
}

static int spu_offload_parm_ds(struct spu_offload_parm_args *a)
{
	struct iproc_ctx_s *ctx = NULL;
	Blog_t *blog_p = a->blog_p;
	struct spu_blog_offload_session *session;

	uint32_t spi = 0;
	struct xfrm_state *xfrm = NULL;
	struct sec_path *secpath = NULL;
	int session_id;
	uint32_t hdr_size;
	bool is_esn = false;
	int i, ret = -1;

	if ((flow_offload_enable & (1<<SPU_STREAM_DS)) == 0)
		goto exit;

	if  (!blog_p->esprx.secPath_p)
		goto exit;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	secpath = secpath_get(blog_p->esprx.secPath_p);
	if (secpath && (secpath->len == 1))
		xfrm = secpath->xvec[secpath->len-1];
#else
	xfrm = blog_p->esprx.xfrm_st;
#endif
	spi = (RX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->rx.tuple.esp_spi);

	ctx = spu_blog_lookup_ctx_byspi(spi, SPU_STREAM_DS);

	if (!xfrm || !ctx || 
	    (ctx->cipher.alg != CIPHER_ALG_AES) || (ctx->cipher.mode != CIPHER_MODE_CBC)) {
		flow_log("unsupported xfrm or ctx\n");
		secpath_put(secpath);
		goto exit;
	}

	flow_log("secpath %p xfrm %p refcnt=%d\n",
		secpath, xfrm, refcount_read(&xfrm->refcnt));

	if (xfrm->genid || xfrm->km.dying || xfrm->km.state == XFRM_STATE_DEAD ||
	    xfrm->lft.soft_use_expires_seconds || xfrm->lft.hard_use_expires_seconds) {
		flow_log("offload not supported\n");
		secpath_put(secpath);
		goto exit;
	}

	session = NULL;

	write_lock (&spu_blog_offload_lock);
	/* bottom half of the sessions are dedicated for DS */
	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS/2; i++) {
		if (atomic_read(&state_g.sessions[state_g.next_ds].taken) == 0) {
			session_id = state_g.next_ds;
			session = &state_g.sessions[session_id];
			session->parm.esp_spi = spi;
			session->parm.u8_0 = 0;
			atomic_set(&session->taken, SESSION_ALLOCATED);
		}
		state_g.next_ds = (state_g.next_ds + 1) & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);
		if (session)
			break;
	}
	write_unlock (&spu_blog_offload_lock);

	if (session == NULL) {
		secpath_put(secpath);
		goto exit;
	}

	if (xfrm->props.flags & XFRM_STATE_ESN)
		is_esn = true;

	hdr_size = create_spu_header(session->parm.spu_header, ctx, blog_p, 0, is_esn, 0);

	if (hdr_size > MAX_SPU_HEADER_SIZE)
	{
		secpath_put(secpath);
		goto exit_clean;
	}

	memset(&session->last_update, 0, sizeof(struct spu_offload_tracker));

	session->parm.key_size = hdr_size - FMD_SIZE;

	if (is_esn) {
		session->parm.is_esn = 1;
		session->parm.key_size -= BLOG_ESP_SEQNUM_HI_LEN;
	}
	session->parm.session_id = session_id;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	session->sp = secpath;
#else
	session->ptr = xfrm;
#endif

	if (xfrm->lft.soft_byte_limit != XFRM_INF || xfrm->lft.soft_packet_limit != XFRM_INF ||
	    xfrm->lft.hard_byte_limit != XFRM_INF || xfrm->lft.hard_packet_limit != XFRM_INF)
		session->parm.data_limit = 1;

	spin_lock_bh(&xfrm->lock);
	if (xfrm->replay_esn) {
		session->parm.seq_lo = xfrm->replay_esn->seq;
		session->parm.seq_hi = xfrm->replay_esn->seq_hi;
		session->parm.long_bitmap = 1;
	}
	else {
		session->parm.seq_lo = xfrm->replay.seq;
		session->parm.seq_hi = 0;
	}
	session->last_update.lft_pkts = xfrm->curlft.packets;
	session->last_update.lft_bytes = xfrm->curlft.bytes;
	spin_unlock_bh(&xfrm->lock);

	session->last_update.seq_lo = session->parm.seq_lo;
	session->last_update.seq_hi = session->parm.seq_hi;

	session->parm.digest_size = ctx->digestsize;
	atomic_set(&session->evict, 0);

	ret = spu_platform_offload_session_ds_parm(session_id, xfrm, &session->parm);
	if (ret != 0)
		goto exit_clean;

	blog_p->spu.is_offload = 1;
	atomic_set(&ctx->offload_id, session_id);

	a->parm = &session->parm;

	atomic_set(&session->taken, SESSION_DEFINED);
	return 0;

exit_clean:
	write_lock (&spu_blog_offload_lock);
	atomic_set(&session->taken, 0);
	session->parm.esp_spi = 0;
	write_unlock (&spu_blog_offload_lock);
exit:
	return ret;

}

static int spu_offload_fill_us_parm(Blog_t *blog_p, uint32_t session_id)
{
	struct spu_blog_offload_session *session;
	struct iproc_ctx_s *ctx = NULL;
	struct dst_entry *dst;

	struct xfrm_state *xfrm = NULL;
	uint32_t spi;
	uint32_t seqhi = 0;

	uint32_t hdr_size;
	uint8_t *data_p;
	BlogTuple_t *esptx_tuple_p = blog_p->esptx_tuple_p;
	bool is_esn = false;
	uint16_t base_chksm;

	if (esptx_tuple_p == NULL) {
		flow_log("missing esptx_tuple in blog\n");
		return -1;
	}

	spi = (TX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->esptx_tuple_p->esp_spi);

	ctx = spu_blog_lookup_ctx_byspi(spi, SPU_STREAM_US);

	dst = blog_p->esptx.dst_p;
	if (dst)
		xfrm = dst->xfrm;
	if (!ctx || xfrm == NULL || (ctx->cipher.alg != CIPHER_ALG_AES) || 
	    (ctx->cipher.mode != CIPHER_MODE_CBC) || (blog_p->esptx.ivsize != CCM_AES_IV_SIZE)) {
		flow_log("offload not supported\n");
		return -1;
	}
	if (xfrm->props.flags & XFRM_STATE_ESN)
		is_esn = true;

	session = &state_g.sessions[session_id];
	memset(&session->last_update, 0, sizeof(struct spu_offload_tracker));

	if (xfrm->lft.soft_byte_limit != XFRM_INF || xfrm->lft.soft_packet_limit != XFRM_INF ||
	    xfrm->lft.hard_byte_limit != XFRM_INF || xfrm->lft.hard_packet_limit != XFRM_INF)
		session->parm.data_limit = 1;

	spin_lock_bh(&xfrm->lock);
	if (xfrm->replay_esn) {
		session->parm.seq_lo = xfrm->replay_esn->oseq;
		session->parm.seq_hi = xfrm->replay_esn->oseq_hi;
	}
	else {
		session->parm.seq_lo = xfrm->replay.oseq;
		session->parm.seq_hi = 0;
	}
	session->last_update.lft_pkts = xfrm->curlft.packets;
	session->last_update.lft_bytes = xfrm->curlft.bytes;
	spin_unlock_bh(&xfrm->lock);

	session->last_update.seq_lo = session->parm.seq_lo;
	session->last_update.seq_hi = session->parm.seq_hi;

	seqhi = htonl(session->parm.seq_hi);
	hdr_size = create_spu_header(session->parm.spu_header, ctx, blog_p, 1, is_esn, seqhi);

	if (hdr_size > MAX_SPU_HEADER_SIZE)
	{
		return -1;
	}
	session->parm.is_esn = is_esn;
	session->parm.key_size = hdr_size - FMD_SIZE;
	if (is_esn)
		session->parm.key_size -= BLOG_ESP_SEQNUM_HI_LEN;

	/* add the outer IP header after the SPU header */
	data_p = session->parm.outer_header;
	*((uint16_t *)data_p + 0) = htons(0x4500 | esptx_tuple_p->tos);
	*((uint16_t *)data_p + 1) = 0; /* length to be filled */
	*((uint16_t *)data_p + 2) = 0; /* id to be filled */
	*((uint16_t *)data_p + 3) = htons(BLOG_IP_FLAG_DF);

	if (TX_ESPoUDP(blog_p)) 
		*((uint16_t *)data_p + 4) = htons((esptx_tuple_p->ttl << 8) |
					    (BLOG_IPPROTO_UDP & 0xFF));
	else
		*((uint16_t *)data_p + 4) = htons((esptx_tuple_p->ttl << 8) |
					    (BLOG_IPPROTO_ESP & 0xFF));
	((BlogIpv4Hdr_t *)data_p)->chkSum = 0;
	/* fill source IP, destination IP */
	_u16cpy( (uint16_t*)&((BlogIpv4Hdr_t *)data_p)->sAddr,
              (uint16_t*)&esptx_tuple_p->saddr,
              sizeof(esptx_tuple_p->saddr) + sizeof(esptx_tuple_p->daddr) );

	base_chksm = ip_fast_csum(data_p, (BLOG_IPV4_HDR_LEN/4));
	((BlogIpv4Hdr_t *)data_p)->chkSum = base_chksm;

	data_p += BLOG_IPV4_HDR_LEN;
	/* add the UDP header in case of ESP over UDP */
	if (TX_ESPoUDP(blog_p)) {
		*((uint16_t *)data_p + 0) = esptx_tuple_p->port.source;
		*((uint16_t *)data_p + 1) = esptx_tuple_p->port.dest;
		*((uint16_t *)data_p + 2) = 0;
		*((uint16_t *)data_p + 3) = 0;
		data_p += BLOG_UDP_HDR_LEN;

		session->parm.esp_o_udp = 1;
	}

	session->parm.digest_size = ctx->digestsize;
	session->parm.iv_size = blog_p->esptx.ivsize;
	session->parm.blog_chan_id = ctx->blog_chan_id;
	session->parm.esp_spi = spi;
	session->parm.session_id = session_id;

	return spu_platform_offload_session_us_parm(session_id, xfrm, &session->parm);
}

static int spu_offload_parm_us(struct spu_offload_parm_args *a)
{
	int session_id;
	struct spu_blog_offload_session *session;
	struct iproc_ctx_s *ctx;
	Blog_t *blog_p = a->blog_p;

	uint32_t spi = (TX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->esptx_tuple_p->esp_spi);

	ctx = spu_blog_lookup_ctx_byspi(spi, SPU_STREAM_US);
	if (ctx == NULL)
		return -1;
	session_id = spu_offload_get_us_id(ctx, spi, blog_p);

	if (session_id < 0)
		return -1;

	session = &state_g.sessions[session_id];
	atomic_inc(&session->ref_count);
	a->parm = &state_g.sessions[session_id].parm;

	return 0;
}

void spu_offload_session_free(uint32_t session_id)
{
	struct spu_blog_offload_session *session;

	session = &state_g.sessions[session_id];
	if (atomic_read(&session->taken)) {
		atomic_inc(&session->evict);
		atomic_inc(&state_g.work_avail);
		SPU_WAKEUP_EVICTHDL();
	}
}

static inline void spu_update_session_offload_stats (int session_id)
{
	struct xfrm_state *xfrm = spu_lookup_offload_xfrm(session_id);

	if (!xfrm)
		return;

	read_lock (&spu_blog_offload_lock);
	spu_update_xfrm_stats (session_id, xfrm, 0);
	read_unlock (&spu_blog_offload_lock);
}

static int spu_offload_parm(void *x)
{
	struct spu_blog_offload_session *session;
	struct spu_offload_parm_args *a = (struct spu_offload_parm_args *)x;
	Blog_t *blog_p = a->blog_p;

	if (flow_offload_enable == 0)
		return -1;

	/* flow delete is marked with blog_p set to NULL */
	if (blog_p == NULL) {
		uint32_t session_id = a->session_id;
		if (a->parm != NULL)
			return -1;

		session = &state_g.sessions[session_id];
		if (atomic_read(&session->taken) == SESSION_DEFINED)
			spu_update_session_offload_stats(session_id);

		if (atomic_read(&session->taken) != 0) {
			struct iproc_ctx_s *ctx;

			ctx = spu_blog_lookup_ctx_byspi(session->parm.esp_spi,
							(session_id < MAX_SPU_OFFLOAD_SESSIONS/2) ? SPU_STREAM_DS : SPU_STREAM_US);

			if (session->parm.is_enc == 0 || atomic_dec_and_test(&session->ref_count)) {
				if (ctx)
					atomic_set(&ctx->offload_id, -1);

				flow_log("flow cleanup for session %d\n", session_id);
				atomic_inc(&session->evict);
				atomic_inc(&state_g.work_avail);
				SPU_WAKEUP_EVICTHDL();
			}
		} else {
			/* hightly likely the session already freed */
			flow_log("session %d spi 0x%x should not be freed\n", session_id, session->parm.esp_spi); 
		}
		return -1;
	}

	if (blog_p->tx.info.phyHdrType == BLOG_SPU_DS) {
		return spu_offload_parm_ds(a);
	}
	else if (blog_p->tx.info.phyHdrType == BLOG_SPU_US) {
		return spu_offload_parm_us(a);
	}
	return -1;
}

int spu_offload_get_rx_info(uint32_t session_id, struct spu_offload_rx_info *info)
{
	int ret = -1;
	struct spu_blog_offload_session *session = &state_g.sessions[session_id];
	struct iproc_ctx_s *ctx = NULL;
	uint32_t spi;
	enum spu_stream_type stream;

	info->ptr = NULL;

	read_lock(&spu_blog_offload_lock);
	if (atomic_read(&session->taken) == SESSION_DEFINED) {
		spi = session->parm.esp_spi;
		if (session->parm.is_enc) {
			info->dst_p = dst_clone(session->dst_p);
			stream = SPU_STREAM_US;
		}
		else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
			info->sp = secpath_get(session->sp);
#else
			xfrm_state_hold(session->ptr);
			info->ptr = session->ptr;
#endif
			stream = SPU_STREAM_DS;
		}
	}
	read_unlock(&spu_blog_offload_lock);

	if (info->ptr) {
		ctx = spu_blog_lookup_ctx_byspi(spi, stream);
		if (ctx) {
			info->spi = spi;
			info->iv_size = ctx->iv_size;
			info->digestsize = ctx->digestsize;
			info->blog_chan_id = ctx->blog_chan_id;
			info->esp_over_udp = ctx->esp_over_udp;
			ret = 0;
		} else {
			if (stream == SPU_STREAM_US)
				dst_release(info->dst_p);
			else
				secpath_put(info->sp);
		}
	}
	return ret;
}

static void spu_update_xfrm_stats(int i, struct xfrm_state *xfrm, int xlocked)
{
	struct spu_offload_tracker curr_stats = {};
	uint32_t bitmap[MAX_REPLAY_WIN_SIZE/32];
	struct spu_blog_offload_session *session = &state_g.sessions[i];

	spu_platform_offload_stats(i, &curr_stats, bitmap, session->parm.long_bitmap); 

	if (curr_stats.lft_pkts == session->last_update.lft_pkts &&
	    curr_stats.replay_window == session->last_update.replay_window &&
	    curr_stats.replay == session->last_update.replay)
		return;

	if (xlocked == 0)
		spin_lock(&xfrm->lock);
	if (session->parm.is_enc == 0) {
		xfrm->stats.replay_window += curr_stats.replay_window - session->last_update.replay_window;
		xfrm->stats.replay += curr_stats.replay - session->last_update.replay;
		if (xfrm->replay_esn) {
			uint32_t wsize = xfrm->replay_esn->replay_window;
			uint32_t i, nr = wsize >> 5;
			for (i=0; i < nr; i++) {
				xfrm->replay_esn->bmp[i] |= bitmap[i];
			}
			xfrm->replay_esn->seq = curr_stats.seq_lo ;
			xfrm->replay_esn->seq_hi = curr_stats.seq_hi;
		} else {
			/* combine the host information if applicable */
			if ((curr_stats.seq_lo - xfrm->replay.seq) < xfrm->props.replay_window)
				xfrm->replay.bitmap = bitmap[0] | 
						      (xfrm->replay.bitmap << (curr_stats.seq_lo - xfrm->replay.seq));
			else
				xfrm->replay.bitmap = bitmap[0];
			xfrm->replay.seq = curr_stats.seq_lo ;
		}

	} else {
		if (xfrm->replay_esn) {
			xfrm->replay_esn->oseq = curr_stats.seq_lo;
			xfrm->replay_esn->oseq_hi = curr_stats.seq_hi;
		} else {
			xfrm->replay.oseq = curr_stats.seq_lo;
		}

	}
	xfrm->curlft.bytes += (curr_stats.lft_bytes - session->last_update.lft_bytes);
	xfrm->curlft.packets += (curr_stats.lft_pkts - session->last_update.lft_pkts);

	if (xlocked == 0)
		spin_unlock(&xfrm->lock);

	memcpy(&session->last_update, &curr_stats, sizeof(struct spu_offload_tracker));
}

static int spu_offload_evict_handler(void *context)
{
	int i, done;
	struct spu_blog_offload_session *session;

	while (1) {
		wait_event_interruptible(state_g.evict_wqh,
					 atomic_read(&state_g.work_avail) ||
					 kthread_should_stop());
		if (kthread_should_stop()) {
			pr_err("%s kthread_should_stop\n", __func__);
			break;
		}
		done = 1;
		for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
			session = &state_g.sessions[i];

			if (atomic_read(&session->taken) == 0 ||
			    atomic_read(&session->evict) == 0)
				continue;

			if (atomic_dec_and_test(&session->evict)) {
				write_lock (&spu_blog_offload_lock);
				spu_platform_offload_free_session(i);
				atomic_set(&session->taken, 0);

				if (session->parm.is_enc == 0) {
					flow_log("cleaning ds session %d for spi 0x%x\n",
						session->parm.session_id, session->parm.esp_spi);
					secpath_put(session->sp);
				} else {
					flow_log("cleaning us session %d for spi 0x%x ref %d taken %d\n",
						session->parm.session_id, session->parm.esp_spi,
						atomic_read(&session->ref_count), atomic_read(&session->taken));
					dst_release(session->dst_p);
				}
				session->parm.esp_spi = 0;
				session->ptr = NULL;
				write_unlock(&spu_blog_offload_lock);
			} 

			atomic_dec(&state_g.work_avail);
			done = 0;
		}
		if (done)
			atomic_dec_if_positive(&state_g.work_avail);
	}
	return 0;
}

int spu_update_xfrm_offload_stats(void *x)
{
	int i, session_id = -1;
	struct spu_blog_offload_session *session;
	struct xfrm_state *xfrm = (struct xfrm_state *)x;
	uint32_t spi = xfrm->id.spi;

	/* look for offloaded xfrm */
	read_lock(&spu_blog_offload_lock);
	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &state_g.sessions[i];
		if (atomic_read(&session->taken) == SESSION_DEFINED && session->parm.esp_spi == spi) {
			spu_update_xfrm_stats (i, xfrm, spin_is_locked(&xfrm->lock));
			session_id = i;
			break;
		}
	}
	read_unlock(&spu_blog_offload_lock);

	/* end offloading for a dead tunnel */
	if (session_id != -1 && (xfrm->genid || xfrm->km.state == XFRM_STATE_DEAD)) {
		struct iproc_ctx_s *ctx = NULL;

		ctx = spu_blog_lookup_ctx_byspi(xfrm->id.spi, (session_id < MAX_SPU_OFFLOAD_SESSIONS/2) ? SPU_STREAM_DS : SPU_STREAM_US);
		if (ctx && atomic_read(&ctx->offload_id) == session_id) {
			flow_log("from stats session id %d spi 0x%x dying genid %d state %d\n",
				 session_id, xfrm->id.spi, xfrm->genid, xfrm->km.state);
			spu_blog_evict(ctx);
		}
	}
	return 0;
}

int spu_offload_get_us_id(struct iproc_ctx_s *ctx, uint32_t spi, Blog_t *blog_p)
{
	int i, session_id = -1, ret = -1;
	struct spu_blog_offload_session *session;
	struct dst_entry *dst;
	struct xfrm_state *xfrm = NULL;

	if ((flow_offload_enable & (1<<SPU_STREAM_US)) == 0)
		return -1;

	/* check if the session is already offloaded */
	session_id = atomic_read(&ctx->offload_id);
	if (session_id != -1) {
		read_lock (&spu_blog_offload_lock);
		session = &state_g.sessions[session_id];
		if (atomic_read(&session->taken) == SESSION_DEFINED) {
			if (session->parm.esp_spi == spi)
				ret = session_id;
		}
		read_unlock (&spu_blog_offload_lock);
	}

	if (session_id > 0 || blog_p == NULL || blog_p->esptx.dst_p == NULL) {
		/* a new session cannot be created when blog_p is NULL
		 * so we have to skip even a matching session is not found */
		goto exit;
	}
	dst = blog_p->esptx.dst_p;
	xfrm = dst->xfrm;
	if (xfrm == NULL || xfrm->props.mode == XFRM_MODE_TRANSPORT ||
	    xfrm->genid || xfrm->km.dying || xfrm->km.state == XFRM_STATE_DEAD ||
	    xfrm->lft.soft_use_expires_seconds || xfrm->lft.hard_use_expires_seconds) {
		/* missing xfrm structure, or it is a dying session, or this session has limited use time, skip */
		goto exit;
	}

	/* need a new session, reserve a spot for it */
	/* US flows can only take session id 32 onwards */
	session = NULL;
	ret = -1;

	write_lock(&spu_blog_offload_lock);
	/* recheck if a new session has already be created */
	for (i = MAX_SPU_OFFLOAD_SESSIONS/2; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		if (atomic_read(&state_g.sessions[i].taken) == SESSION_ALLOCATED &&
		    state_g.sessions[i].parm.esp_spi == spi) {
			/* session allocated by another CPU, break here */		
			write_unlock(&spu_blog_offload_lock);
			return i;
		}
	}
	/* really need to define a new session */
	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS/2; i++) {
		int taken = atomic_read(&state_g.sessions[state_g.next_us + MAX_SPU_OFFLOAD_SESSIONS/2].taken);
		if (taken == 0) {
			session_id = state_g.next_us + MAX_SPU_OFFLOAD_SESSIONS/2;
			session = &state_g.sessions[session_id];
			session->parm.u8_0 = 0;
			session->parm.is_enc = 1;
			session->parm.esp_spi = spi;
			atomic_set(&session->evict, 0);
			atomic_set(&session->taken, SESSION_ALLOCATED);
			atomic_set(&session->ref_count, 0);
		}
		state_g.next_us = (state_g.next_us + 1) & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);
		if (session)
			break;
	}

	if (session == NULL) {
		write_unlock(&spu_blog_offload_lock);
		goto exit;
	}

	ret = spu_offload_fill_us_parm(blog_p, session_id);

	if (ret == 0) {
		int ctx_id = atomic_read(&ctx->offload_id);

		atomic_set(&session->taken, SESSION_DEFINED);
		if (ctx_id != -1 && ctx_id != session_id)
			printk("ctx already has offload session %d new session %d\n", ctx_id, session_id);

		atomic_set(&ctx->offload_id, session_id);
		session->dst_p = dst_clone(dst);
		blog_p->spu.is_offload = 1;
		ret = session_id;
	} else {
		atomic_set(&session->taken, 0);
		session->parm.esp_spi = 0;
	}
	write_unlock(&spu_blog_offload_lock);
exit:
	return ret;
}

void spu_offload_get_fixed_hdr(int session_id, uint32_t *hdr_offset, uint8_t *size, uint8_t **hdr)
{
	struct spu_blog_offload_session *session;

	*hdr_offset = BLOG_IPV4_HDR_LEN + CCM_AES_IV_SIZE + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN;

	session = &state_g.sessions[session_id];
	if (session->parm.esp_o_udp)
		*hdr_offset += BLOG_UDP_HDR_LEN;

	*size = session->parm.fixed_hdr_size;
	*hdr = session->parm.spu_header;
}

void spu_offload_handle_exception(uint32_t session_id, uint8_t data_limit, uint8_t overflow)
{
	int ret;
	struct xfrm_state *xfrm = NULL;
	struct iproc_ctx_s *ctx = NULL;
	struct spu_blog_offload_session *session = &state_g.sessions[session_id];

	if (atomic_read(&session->taken) != SESSION_DEFINED) {
		flow_log("session %d not taken, cannot handle exception\n", session_id);
		return;
	}
	xfrm = spu_lookup_offload_xfrm (session_id);
	ctx = spu_blog_lookup_ctx_byspi(session->parm.esp_spi, (session_id < MAX_SPU_OFFLOAD_SESSIONS/2) ? SPU_STREAM_DS : SPU_STREAM_US);

	if (!xfrm && ctx) {
		spu_blog_evict(ctx);
		return;
	}
	read_lock (&spu_blog_offload_lock);
	/* update the session stats */
	spu_update_xfrm_stats (session_id, xfrm, 0);
	ret = 0;
	if (data_limit)
		ret |= xfrm_state_check_expire(xfrm);
	if (overflow)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
		ret |= xfrm->repl->overflow(xfrm, NULL);
#else
		ret |= xfrm_replay_overflow(xfrm, NULL);
#endif
	if (ret && ctx)
		spu_blog_evict(ctx);
	read_unlock (&spu_blog_offload_lock);
}

ssize_t spu_offload_debug_info(char *buf, ssize_t out_count)
{
	int i;
	ssize_t out_offset = 0;
	struct spu_blog_offload_session *session;

	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"offload evict_work %d \nDS offload sessions....(next_ds %d)\n",
				atomic_read(&state_g.work_avail), state_g.next_ds);

	read_lock (&spu_blog_offload_lock);
	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS/2; i++) {
		session = &state_g.sessions[i];
		if (atomic_read(&session->taken) == 0)
			continue;
		out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"session %d spi 0x%x evict %d\n", i, session->parm.esp_spi, atomic_read(&session->evict));
	}

	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"US offload sessions....(next_us %d)\n", state_g.next_us);
	for (i = MAX_SPU_OFFLOAD_SESSIONS/2; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &state_g.sessions[i];
		if (atomic_read(&session->taken) == 0)
			continue;
		out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"session %d spi 0x%x taken %d ref_count %d evict %d\n", i, session->parm.esp_spi,
				atomic_read(&session->taken), atomic_read(&session->ref_count), atomic_read(&session->evict));
	}
	read_unlock (&spu_blog_offload_lock);
	return out_offset;
}

int spu_offload_init(void)
{
	char threadname[15] = {0};
	struct task_struct *thread;

	spu_blog_offload_lock = __RW_LOCK_UNLOCKED (spu_blog_offload_lock);

	memset(&state_g, 0, sizeof(struct spu_offload_state));

	init_waitqueue_head(&state_g.evict_wqh);
	sprintf(threadname, "spu_evict_hdl");
	thread = kthread_create(spu_offload_evict_handler, NULL, threadname);

	if (IS_ERR(thread)){
		pr_err("Failed to create %s kthread\n", threadname);
		return -1;
	}
	atomic_set(&state_g.work_avail, 0);
	state_g.evict_thread = thread;
	wake_up_process(thread);

	return spu_platform_offload_init();
}

void spu_blog_offload_register(void)
{
	spu_platform_offload_register();
	bcmFun_reg(BCM_FUN_ID_SPUOFFLOAD_STATS_UPDATE, spu_update_xfrm_offload_stats);
	bcmFun_reg(BCM_FUN_ID_SPUOFFLOAD_SESSION_PARM, spu_offload_parm);
}

void spu_blog_offload_deregister(void)
{
	int i;

	/* sanity check, we should have no more offload sessions */
	for (i=0; i < MAX_SPU_OFFLOAD_SESSIONS; i++)
	{
		if (atomic_read(&state_g.sessions[i].taken))
			printk("spu offload active session %d spi 0x%x\n",
				i, state_g.sessions[i].parm.esp_spi);
	}
	bcmFun_dereg(BCM_FUN_ID_SPUOFFLOAD_STATS_UPDATE);
	bcmFun_dereg(BCM_FUN_ID_SPUOFFLOAD_SESSION_PARM);
	spu_platform_offload_deregister();
}
