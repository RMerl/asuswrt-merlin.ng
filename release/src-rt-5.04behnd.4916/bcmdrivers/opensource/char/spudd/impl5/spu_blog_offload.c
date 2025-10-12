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
#include <linux/timer.h>

static spinlock_t spu_blog_offload_lock;
#define SESSION_ALLOCATED	0x1
#define SESSION_DEFINED		0x2

struct spu_blog_offload_session {
	atomic_t taken;
	atomic_t ref_count;
	atomic_t evict;
	atomic_t evict_rdy;
	unsigned long evict_time;
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
	atomic_t num_us;
	atomic_t num_ds;
	struct timer_list sched_timer;
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
	flow_log("%s spi 0x%08x dir %d rctx %px\n", __func__, spi, stream, rctx);
	return rctx;
}  /* spu_blog_lookup_ctx_byspi() */

static inline struct iproc_ctx_s *spu_blog_lookup_ctx_bychanid(uint8_t chan_idx, enum spu_stream_type stream)
{
	struct iproc_ctx_s *ctx;
	struct iproc_ctx_s *rctx = NULL;

	read_lock(&iproc_priv.ctxListLock[stream]);
	list_for_each_entry(ctx, &iproc_priv.ctxList[stream], entry)
	{
		if (ctx->blog_chan_id == chan_idx)
		{
			rctx = ctx;
			break;
		}
	}
	read_unlock(&iproc_priv.ctxListLock[stream]);
	return rctx;
}

static int create_spu_header(uint8_t *hdr_buff, struct iproc_ctx_s *ctx,
			     uint8_t is_encrypt, bool is_esn, uint32_t seq_hi, bool is_transport_mode, bool ipv6)
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
	cipher_parms.iv_len = ctx->iv_size + ctx->salt_len;

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
	if (ctx->cipher.mode == CIPHER_MODE_CBC) {
		if (is_encrypt == 0) {
			aead_parms.assoc_size += ctx->iv_size;
		} else {
			aead_parms.return_iv = true;
			aead_parms.ret_iv_len = cipher_parms.iv_len & 0xF;
		}
	} else if (ctx->cipher.mode == CIPHER_MODE_GCM) {
		/* iv for GCM is seq number related, include that in the header */
		if (ctx->esn) {
			/* gcm esn seq hi is part of assoc */
			aead_parms.assoc_size += BLOG_ESP_SEQNUM_HI_LEN;
			/* do not return IV for ESN encrypt */
		} else if (is_encrypt) {
			aead_parms.return_iv = true;
			aead_parms.ret_iv_len = ctx->iv_size;
			aead_parms.ret_iv_off = ctx->salt_len;
		}
	}

	aead_parms.iv_len = spu->spu_aead_ivlen(ctx->cipher.mode, ctx->alg->alg.aead.ivsize);

	if (ctx->auth.alg == HASH_ALG_AES)
		hash_parms.type = (enum hash_type)ctx->cipher_type;

	hdr_size = spu->spu_create_request(hdr_buff, &req_opts,
					   &cipher_parms, &hash_parms,
					   &aead_parms, ctx->digestsize + ctx->salt_len);

	fmd->ctrl1 &= ~(SPU2_RETURN_MD);
	if ((is_encrypt == 0) || (ctx->gcm && ctx->esn))
		fmd->ctrl1 &= ~(SPU2_RETURN_AAD2);

	if (is_encrypt) {
		/* when SPU interaction is offloaded to Runner,
		 * the SPU HW is used for padding and IV generation as well */

		/* enable padding */
		if (is_transport_mode) {
			fmd->ctrl0 |= (SPU2_CIPH_PAD_EN | ((uint64_t)BLOG_IPPROTO_UDP << SPU2_CIPH_PAD_SHIFT));
		}
		else {
			if (ipv6)
				fmd->ctrl0 |= (SPU2_CIPH_PAD_EN | ((uint64_t)BLOG_IPPROTO_IPV6 << SPU2_CIPH_PAD_SHIFT));
			else
				fmd->ctrl0 |= (SPU2_CIPH_PAD_EN | ((uint64_t)BLOG_IPPROTO_IPIP << SPU2_CIPH_PAD_SHIFT));
		}
		if (is_esn == false)
			fmd->ctrl0 |= (SPU2_PROSEL_IPSEC << SPU2_PROTO_SEL_SHIFT);

		if (ctx->cipher.mode == CIPHER_MODE_CBC) {
			fmd->ctrl1 |= SPU2_GENIV; 
			fmd->ctrl1 &= ~SPU2_IV_LEN;
		}
	}
	/* for gcm offloading, put the salt after the keys */
	if (ctx->cipher.mode == CIPHER_MODE_GCM) {
		memcpy(&hdr_buff[hdr_size], ctx->salt, ctx->salt_len);
		hdr_size += ctx->salt_len;
	}
	return hdr_size;
}

static int spu_offload_get_ds_session_id(uint32_t spi)
{
	struct spu_blog_offload_session *session;
	int i, hdr_size, offload_id, session_id = -1;
	struct iproc_ctx_s *ctx = NULL;
	struct xfrm_state *xfrm = NULL;
	bool is_esn = false;

	if ((flow_offload_enable & (1 << SPU_STREAM_DS)) == 0)
		return -1;

	ctx = spu_blog_lookup_ctx_byspi(spi, SPU_STREAM_DS);
	if (ctx != NULL)
		xfrm = ctx->xfrm;

	if (xfrm == NULL || xfrm->genid || xfrm->km.dying || xfrm->km.state == XFRM_STATE_DEAD ||
	    xfrm->lft.soft_use_expires_seconds || xfrm->lft.hard_use_expires_seconds) {
		flow_log("offload not supported for dying session spi 0x%x\n", spi);
		return -1;
	}

	/* sanity check, session should not have offloaded yet */
	offload_id = atomic_read(&ctx->offload_id);
	if (offload_id >= 0) {
		pr_err("session has already been offloaded\n");
		session = &state_g.sessions[offload_id];
		if (atomic_read(&session->taken) == SESSION_DEFINED &&
		    session->parm.esp_spi == spi)
			session_id = offload_id;
		return session_id;
	}

	if (atomic_read(&state_g.num_ds) == MAX_SPU_OFFLOAD_SESSIONS/2)
		return -1;

	/* offload a new DS session */
	session = NULL;
	spin_lock_bh (&spu_blog_offload_lock);
	/* bottom half of the sessions are dedicated for DS */
	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS/2; i++) {
		if (atomic_read(&state_g.sessions[state_g.next_ds].taken) == 0) {
			session_id = state_g.next_ds;
			session = &state_g.sessions[session_id];
			session->parm.esp_spi = spi;
			session->parm.u8_0 = 0;
			atomic_set(&session->taken, SESSION_ALLOCATED);
			/* make sure the xfrm remain valid while session is offloaded */
			xfrm_state_hold(xfrm);
		}
		state_g.next_ds = (state_g.next_ds + 1) & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);
		if (session)
			break;
	}
	if (session == NULL) {
		spin_unlock_bh (&spu_blog_offload_lock);
		return -1;
	}

	/* SPU ESN mode does not support GCM */
	if (xfrm->props.flags & XFRM_STATE_ESN) {
		session->parm.is_esn = 1;
		if (!ctx->gcm)
			is_esn = true;
	}

	hdr_size = create_spu_header(session->parm.spu_header, ctx, 0, is_esn, 0, 0, ctx->ipv6);

	if (hdr_size > MAX_SPU_HEADER_SIZE)
		goto exit_clean;

	memset(&session->last_update, 0, sizeof(struct spu_offload_tracker));

	session->parm.key_size = hdr_size - FMD_SIZE;

	if (is_esn)
		session->parm.key_size -= BLOG_ESP_SEQNUM_HI_LEN;
	session->parm.session_id = session_id;

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
	session->parm.iv_size = ctx->iv_size;
	session->parm.esp_o_udp = ctx->esp_over_udp;
	if (ctx->cipher.mode == CIPHER_MODE_GCM)
		session->parm.is_gcm = 1;
	session->parm.ipv6 = ctx->ipv6;
	if (ctx->ipv6)
		session->parm.outer_hdr_size = BLOG_IPV6_HDR_LEN;
	else
		session->parm.outer_hdr_size = BLOG_IPV4_HDR_LEN;
	if (ctx->esp_over_udp)
		session->parm.outer_hdr_size += BLOG_UDP_HDR_LEN;

	atomic_set(&session->evict, 0);

	if (spu_platform_offload_ds_session(session_id, xfrm, &session->parm) != 0)
		goto exit_clean;

	atomic_inc(&state_g.num_ds);

	atomic_set(&ctx->offload_id, session_id);

	atomic_set(&session->taken, SESSION_DEFINED);
	spin_unlock_bh (&spu_blog_offload_lock);
	return session_id;

exit_clean:
	atomic_set(&session->taken, 0);
	session->parm.esp_spi = 0;
	spin_unlock_bh (&spu_blog_offload_lock);
	return -1;
}


static void spu_offload_outer_hdr(uint8_t *hdr_p, Blog_t *blog_p, uint8_t ipv6)
{
	uint8_t *data_p = hdr_p;

	if(ipv6 == 0) {
		uint16_t base_chksm;
		BlogTuple_t *esptx_tuple_p = blog_p->esptx_tuple_p;
		/* outer IPv4 header */
		*((uint16_t *)data_p + 0) = htons(0x4500 | esptx_tuple_p->tos);
		*((uint16_t *)data_p + 1) = 0; /* length to be filled */
		*((uint16_t *)data_p + 2) = 0; /* id to be filled */
		*((uint16_t *)data_p + 3) = htons(BLOG_IP_FLAG_DF);

		if (TX_ESPoUDP(blog_p)) 
			*((uint16_t *)data_p + 4) = htons((esptx_tuple_p->ttl << 8) | (BLOG_IPPROTO_UDP & 0xFF));
		else
			*((uint16_t *)data_p + 4) = htons((esptx_tuple_p->ttl << 8) | (BLOG_IPPROTO_ESP & 0xFF));
		((BlogIpv4Hdr_t *)data_p)->chkSum = 0;
		/* fill source IP, destination IP */
		_u16cpy( (uint16_t*)&((BlogIpv4Hdr_t *)data_p)->sAddr, (uint16_t*)&esptx_tuple_p->saddr,
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
		}
	} else {
		BlogTupleV6_t *esp6tx_tuple_p = blog_p->esp6tx_tuple_p;

		/* outer IPv6 header */
		_write32_align16(((uint16_t *)data_p + 0), esp6tx_tuple_p->word0);
		((BlogIpv6Hdr_t*)data_p)->len = 0; /* length to be filled */
		((BlogIpv6Hdr_t*)data_p)->hopLmt = esp6tx_tuple_p->tx_hop_limit;
		if (TX_ESPoUDP(blog_p))
			((BlogIpv6Hdr_t *)data_p)->nextHdr = BLOG_IPPROTO_UDP;
		else
			((BlogIpv6Hdr_t *)data_p)->nextHdr = BLOG_IPPROTO_ESP;
		_u16cpy( (uint16_t *)&((BlogIpv6Hdr_t *)data_p)->sAddr,
			(uint16_t*)&esp6tx_tuple_p->saddr, 2 * sizeof(ip6_addr_t));
	}
}


void spu_offload_session_free(uint32_t session_id)
{
	struct spu_blog_offload_session *session;

	session = &state_g.sessions[session_id];

	spin_lock_bh (&spu_blog_offload_lock);
	if (atomic_read(&session->taken)) {
		atomic_inc(&session->evict);
		atomic_set(&session->ref_count, 0);
		atomic_set(&session->evict_rdy, 2);
		atomic_inc(&state_g.work_avail);
		SPU_WAKEUP_EVICTHDL();
	}
	spin_unlock_bh (&spu_blog_offload_lock);
}

#if defined(CC_CMD_PARM)
static int spu_offload_parm_ds(struct spu_offload_parm_args *a)
{
	struct iproc_ctx_s *ctx = NULL;
	Blog_t *blog_p = a->blog_p;
	struct spu_blog_offload_session *session;
	int session_id;
	uint32_t spi = 0;

	if (RX_IPV6(blog_p))
		spi = (RX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->tupleV6.esp_spi);
	else
		spi = (RX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->rx.tuple.esp_spi);

	session_id = spu_offload_get_ds_session_id(spi);
	if (session_id < 0)
		return -1;

	session = &state_g.sessions[session_id];
	blog_p->spu.offload_ds = 1;
	atomic_inc(&session->ref_count);
#if IS_ENABLED(CONFIG_BCM_FPI)
	flow_log("%s(): set blog esptx ivsize %d as ctx iv_size %d\n",
			__func__, blog_p->esptx.ivsize, ctx->iv_size);
	blog_p->esptx.ivsize = ctx->iv_size; 
#endif
	/* may not need this */
	ctx = spu_blog_lookup_ctx_byspi(spi, SPU_STREAM_DS);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	session->sp = secpath_get(blog_p->esprx.secPath_p);
#else
	session->ptr = ctx->xfrm;
#endif
	blog_p->spu.is_esn = session->parm.is_esn;
	if (session->parm.is_esn && session->parm.is_gcm)
		blog_p->spu.gcm_esn = 1;

	a->session_id = session_id;
	a->parm = &session->parm;
	return 0;
}

static int spu_offload_parm_us(struct spu_offload_parm_args *a)
{
	int session_id;
	uint32_t spi;
	struct spu_blog_offload_session *session;
	Blog_t *blog_p = a->blog_p;

	if (RX_IPV6(blog_p)) {
		spi = (TX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->esp6tx_tuple_p->esp_spi);
	} else {
		spi = (TX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->esptx_tuple_p->esp_spi);
	}

	session_id = spu_offload_get_us_session_id(spi, blog_p->esptx.dst_p);
	if (session_id >= 0) {
		session = &state_g.sessions[session_id];

		spu_offload_outer_hdr(session->parm.outer_header, blog_p, session->parm.ipv6);

		/* TODO: handle multiple outer headers */
		if (atomic_read(&session->ref_count) == 0) {
			if (spu_platform_offload_us_parm_set(&session->parm) != 0) {
				pr_err("cannot offload session %d\n", session_id);
				/* clean the offload session */
				spu_offload_session_free(session_id);
				return -1;
			}
		}

		blog_p->spu.offload_us = 1;
		atomic_inc(&session->ref_count);
		a->session_id = session_id;
		a->parm = &session->parm;
		return 0;
	}
	return -1;
}

static int spu_offload_parm(void *x)
{
	struct spu_offload_parm_args *a = (struct spu_offload_parm_args *)x;
	Blog_t *blog_p = a->blog_p;

	if (blog_p->tx.info.phyHdrType == BLOG_SPU_DS) {
		return spu_offload_parm_ds(a);
	}
	else if (blog_p->tx.info.phyHdrType == BLOG_SPU_US) {
		return spu_offload_parm_us(a);
	}
	return -1;
}
#endif

#if defined(CC_CMD_PREPEND)
static int spu_offload_ds_prepend_hdr(struct spu_offload_prephdr_args *a)
{
	struct iproc_ctx_s *ctx = NULL;
	Blog_t *blog_p = a->blog_p;
	struct spu_blog_offload_session *session;
	uint32_t spi;
	int session_id;

	a->prepend_size = 0;
	if (RX_IPV6(blog_p))
		spi = (RX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->tupleV6.esp_spi);
	else
		spi = (RX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->rx.tuple.esp_spi);

	session_id = spu_offload_get_ds_session_id(spi);
	if (session_id < 0)
		return -1;

	session = &state_g.sessions[session_id];
	a->session_id = session_id;

	blog_p->spu.offload_ds = 1;
	atomic_inc(&session->ref_count);
#if IS_ENABLED(CONFIG_BCM_FPI)
	flow_log("%s(): set blog esptx ivsize %d as ctx iv_size %d\n",
			__func__, blog_p->esptx.ivsize, ctx->iv_size);
	blog_p->esptx.ivsize = ctx->iv_size; 
#endif
	/* TODO: may not need this */
	ctx = spu_blog_lookup_ctx_byspi(spi, SPU_STREAM_DS);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	session->sp = secpath_get(blog_p->esprx.secPath_p);
#else
	session->ptr = ctx->xfrm;
#endif
	blog_p->spu.is_esn = session->parm.is_esn;
	if (session->parm.is_esn && session->parm.is_gcm)
		blog_p->spu.gcm_esn = 1;

	/* get the prepend header to put in cmdlist */
	spu_platform_offload_ds_prepend(&session->parm, a);

	return 0;
}


static int spu_offload_us_prepend_hdr(struct spu_offload_prephdr_args *a)
{
	struct spu_blog_offload_session *session;
	Blog_t *blog_p = a->blog_p;
	uint32_t spi;
	int ret = -1;

	if (RX_IPV6(blog_p)) {
		spi = (TX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->esp6tx_tuple_p->esp_spi);
	} else {
		spi = (TX_ESPoUDP(blog_p)) ? blog_p->esp_over_udp_spi : _read32_align16((uint16_t *)&blog_p->esptx_tuple_p->esp_spi);
	}

	ret = spu_offload_get_us_session_id(spi, blog_p->esptx.dst_p);

	a->prepend_size = 0;
	if (ret >= 0) {

		spin_lock_bh(&spu_blog_offload_lock);
		a->session_id = ret;
		session = &state_g.sessions[ret];

		spu_offload_outer_hdr(session->parm.outer_header, blog_p, session->parm.ipv6);

		blog_p->spu.offload_us = 1;

		spu_platform_offload_us_prepend(&session->parm, a);
		atomic_inc(&session->ref_count);
		ret = 0;
		spin_unlock_bh(&spu_blog_offload_lock);
	}

	return ret;
}

static int spu_offload_prepend_hdr(void *x)
{
	struct spu_offload_prephdr_args *a = (struct spu_offload_prephdr_args *)x;
	Blog_t *blog_p = a->blog_p;

	if (blog_p->tx.info.phyHdrType == BLOG_SPU_US)
		return spu_offload_us_prepend_hdr(a);
	else
		return spu_offload_ds_prepend_hdr(a);
}
#endif

int spu_offload_get_rx_info(uint32_t session_id, struct spu_offload_rx_info *info)
{
	int ret = -1;
	struct spu_blog_offload_session *session = &state_g.sessions[session_id];
	struct iproc_ctx_s *ctx = NULL;
	uint32_t spi;
	enum spu_stream_type stream;

	info->ptr = NULL;

	spin_lock_bh(&spu_blog_offload_lock);
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
	spin_unlock_bh(&spu_blog_offload_lock);

	if (info->ptr) {
		ctx = spu_blog_lookup_ctx_byspi(spi, stream);
		if (ctx) {
			info->spi = spi;
			info->iv_size = ctx->iv_size;
			info->digestsize = ctx->digestsize;
			info->blog_chan_id = ctx->blog_chan_id;
			info->esp_over_udp = ctx->esp_over_udp;
			info->ipv6 = ctx->ipv6;
			ret = 0;
		} else {
			if (stream == SPU_STREAM_US)
				dst_release(info->dst_p);
			else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
				secpath_put(info->sp);
#else
				xfrm_state_put(info->ptr);
#endif
			}
		}
	}
	return ret;
}

static void spu_update_xfrm_stats(int id, struct xfrm_state *xfrm, int xlocked)
{
	struct spu_offload_tracker curr_stats = {};
	uint32_t bitmap[MAX_REPLAY_WIN_SIZE/32];
	struct spu_blog_offload_session *session = &state_g.sessions[id];

	spu_platform_offload_stats(id, &curr_stats, bitmap, session->parm.long_bitmap); 

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
	struct iproc_ctx_s *ctx = NULL;

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

			ctx = spu_blog_lookup_ctx_byspi(session->parm.esp_spi, (i < MAX_SPU_OFFLOAD_SESSIONS/2) ? SPU_STREAM_DS : SPU_STREAM_US);
			if (ctx == NULL) {
				pr_err("failed to locate ctx session %d spi 0x%x taken %d\n",
					i, session->parm.esp_spi, atomic_read(&session->taken));
				continue;
			}

			spin_lock_bh (&spu_blog_offload_lock);
			if (atomic_read(&session->evict_rdy) == 2) {
				/* make sure we get the last bit of the offload stats */
				spu_update_xfrm_stats(i, ctx->xfrm, 0);
				if (atomic_read(&session->ref_count)) {
					pr_err("session %d 0x%x should not be cleaned\n",
						i, session->parm.esp_spi);
				} else {
					if (session->parm.is_enc == 0) {
						flow_log("cleaning ds session %d for spi 0x%x\n",
							session->parm.session_id, session->parm.esp_spi);
						atomic_dec(&state_g.num_ds);
						secpath_put(session->sp);
					} else {
						flow_log("cleaning us session %d for spi 0x%x ref %d taken %d\n",
							session->parm.session_id, session->parm.esp_spi,
							atomic_read(&session->ref_count), atomic_read(&session->taken));
						atomic_dec(&state_g.num_us);
						dst_release(session->dst_p);
					}
					spu_platform_offload_free_session(i);
					xfrm_state_put(ctx->xfrm);
					atomic_set(&ctx->offload_id, -1);
					atomic_set(&session->evict, 0);
					atomic_set(&session->evict_rdy, 0);
					atomic_set(&session->taken, 0);
					session->parm.esp_spi = 0;
					session->ptr = NULL;
				}
			}
			spin_unlock_bh (&spu_blog_offload_lock);

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
	spin_lock_bh(&spu_blog_offload_lock);
	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &state_g.sessions[i];
		if (atomic_read(&session->taken) == SESSION_DEFINED && session->parm.esp_spi == spi) {
			spu_update_xfrm_stats (i, xfrm, spin_is_locked(&xfrm->lock));
			session_id = i;
			break;
		}
	}
	spin_unlock_bh(&spu_blog_offload_lock);

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

int spu_offload_get_us_session_id(uint32_t spi, struct dst_entry *dst_p)
{
	struct iproc_ctx_s *ctx;
	struct spu_blog_offload_session *session;
	struct xfrm_state *xfrm;
	int ret = -1, session_id, i, hdr_size;
	bool is_esn = false;
	bool is_transport_mode = false;
	uint32_t seqhi;

	if ((flow_offload_enable & (1<<SPU_STREAM_US)) == 0)
		return -1;

	ctx = spu_blog_lookup_ctx_byspi(spi, SPU_STREAM_US);
	if (ctx == NULL)
		return -1;

	/* check if the session is already offloaded */
	spin_lock_bh (&spu_blog_offload_lock);
	session_id = atomic_read(&ctx->offload_id);
	if (session_id != -1) {
		session = &state_g.sessions[session_id];
		if (session->parm.esp_spi == spi) {
			int valid, evict_num = atomic_read(&session->evict);

			xfrm = ctx->xfrm;
			valid = 1;
			if(evict_num != 0) {
				if (xfrm->genid || xfrm->km.dying || xfrm->km.state == XFRM_STATE_DEAD) {
					valid = 0;
				} else {
					flow_log("new flows for us 0x%x pending evict %d taken %d\n",
						spi, evict_num, atomic_read(&session->taken));
					atomic_set(&session->evict, 0);
					atomic_set(&session->evict_rdy, 0);
				}
			}
			if (valid)
				ret = session_id;
		} else {
			pr_err("ctx offload session does not match 0x%x != 0x%x\n",
				session->parm.esp_spi, spi);
		}
	}

	if (ret >= 0) {
		if (dst_p && (session->dst_p == NULL))
			session->dst_p = dst_clone(dst_p);
		spin_unlock_bh (&spu_blog_offload_lock);
		return ret;
	}

	xfrm = ctx->xfrm;

#if !defined(CONFIG_BCM_FPI)
	/* we do not support offloading session without dst entry for non FPI loads
	 * this is because encrypted packet by offload engine will have no
	 * destination if not provided */
	if (dst_p == NULL) {
		spin_unlock_bh(&spu_blog_offload_lock);
		return -1;
	}
#endif

	/* session not offloaded, allocate a new session */
	if (xfrm == NULL || xfrm->genid || xfrm->km.dying || xfrm->km.state == XFRM_STATE_DEAD ||
	    xfrm->lft.soft_use_expires_seconds || xfrm->lft.hard_use_expires_seconds) {
		spin_unlock_bh (&spu_blog_offload_lock);
		/* missing xfrm structure, or it is a dying session, or this session has limited use time, skip */
		return -1;
	}

	if (atomic_read(&state_g.num_us) == MAX_SPU_OFFLOAD_SESSIONS/2) {
		spin_unlock_bh (&spu_blog_offload_lock);
		return -1;
	}

	/* need a new session, reserve a spot for it */
	/* US flows can only take session id 32 onwards */
	session = NULL;
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

			/* make sure the xfrm remain valid while session is offloaded */
			xfrm_state_hold(xfrm);
		}
		state_g.next_us = (state_g.next_us + 1) & (MAX_SPU_OFFLOAD_SESSIONS/2 - 1);
		if (session)
			break;
	}

	if (session == NULL) {
		spin_unlock_bh (&spu_blog_offload_lock);
		return -1;
	}
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
	if (xfrm->props.mode == XFRM_MODE_TRANSPORT) {
		is_transport_mode = true;
	}
	session->last_update.lft_pkts = xfrm->curlft.packets;
	session->last_update.lft_bytes = xfrm->curlft.bytes;
	spin_unlock_bh(&xfrm->lock);

	session->last_update.seq_lo = session->parm.seq_lo;
	session->last_update.seq_hi = session->parm.seq_hi;

	if (ctx->cipher.mode == CIPHER_MODE_GCM) {
		uint8_t *data_p;
		memcpy(session->parm.gcm_iv_aad2, ctx->iv, ctx->iv_size);
		data_p = &session->parm.gcm_iv_aad2[ctx->iv_size];
		*((uint32_t *)data_p) = spi;
		session->parm.is_gcm = 1;
	}
	/* SPU HW cannot handle GCM ESN mode */
	if (xfrm->props.flags & XFRM_STATE_ESN && !ctx->gcm)
		is_esn = true;

	seqhi = htonl(session->parm.seq_hi);
	hdr_size = create_spu_header(session->parm.spu_header, ctx, 1, is_esn, seqhi, is_transport_mode, ctx->ipv6);

	if (hdr_size > MAX_SPU_HEADER_SIZE) {
		spin_unlock_bh (&spu_blog_offload_lock);
		flow_log("hdr_size too large %d (max %x)\n", hdr_size, MAX_SPU_HEADER_SIZE);
		return -1;
	}
	session->parm.key_size = hdr_size - FMD_SIZE;
	if (is_esn)
		session->parm.key_size -= BLOG_ESP_SEQNUM_HI_LEN;

	session->parm.is_esn = ctx->esn;
	session->parm.blk_size = ctx->blk_size;
	session->parm.ipv6 = ctx->ipv6;
	session->parm.digest_size = ctx->digestsize;
	session->parm.iv_size = ctx->iv_size;
	session->parm.blog_chan_id = ctx->blog_chan_id;
	session->parm.esp_spi = spi;
	session->parm.session_id = session_id;
	if (ctx->ipv6 == 0) {
		session->parm.outer_hdr_size = BLOG_IPV4_HDR_LEN;
		if (xfrm->encap) {
			session->parm.esp_o_udp = 1;
			session->parm.outer_hdr_size += BLOG_UDP_HDR_LEN;
		}
	} else {
		session->parm.outer_hdr_size = BLOG_IPV6_HDR_LEN;
	}
	if (dst_p && (session->dst_p == NULL))
		session->dst_p = dst_clone(dst_p);

	spu_platform_offload_us_session(session_id, xfrm, &session->parm);

	atomic_set(&session->taken, SESSION_DEFINED);
	atomic_set(&ctx->offload_id, session_id);
	atomic_inc(&state_g.num_us);

	spin_unlock_bh (&spu_blog_offload_lock);
	return session_id;
}

void spu_offload_prep_us_ingress(uint32_t session_id, uint8_t *data_p, struct spu_offload_prephdr_args *a, uint32_t *hdr_offset)
{
	struct spu_blog_offload_session *session;

	a->prepend_size = 0;

	spin_lock_bh (&spu_blog_offload_lock);
	session = &state_g.sessions[session_id];
	if (atomic_read(&session->taken) == SESSION_DEFINED) {

		if (session->parm.ipv6 == 0)
		{
			uint16_t base_chksm;

			((BlogIpv4Hdr_t*)data_p)->chkSum = 0;
			((BlogIpv4Hdr_t*)data_p)->id = 0;
			((BlogIpv4Hdr_t*)data_p)->len = 0;
			base_chksm = ip_fast_csum(data_p, (BLOG_IPV4_HDR_LEN/4));
			((BlogIpv4Hdr_t *)data_p)->chkSum = base_chksm;
		}

		memcpy(session->parm.outer_header, data_p, session->parm.outer_hdr_size);

		*hdr_offset = session->parm.outer_hdr_size + session->parm.iv_size + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN;
		spu_platform_offload_us_prepend(&session->parm, a);
	}
	spin_unlock_bh (&spu_blog_offload_lock);
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
	ctx = spu_blog_lookup_ctx_byspi(session->parm.esp_spi, (session_id < MAX_SPU_OFFLOAD_SESSIONS/2) ? SPU_STREAM_DS : SPU_STREAM_US);
	xfrm = ctx->xfrm;

	spin_lock_bh (&spu_blog_offload_lock);
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
	spin_unlock_bh (&spu_blog_offload_lock);
}

ssize_t spu_offload_debug_info(char *buf, ssize_t out_count)
{
	int i;
	ssize_t out_offset = 0;
	struct spu_blog_offload_session *session;
	uint32_t req, cmpl;

	spu_platform_offloaded(&req, &cmpl);
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"Total offloaded reqquest %d cmpl %d\n", req, cmpl);

	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"offload evict_work %d \n%d DS offload sessions....(next_ds %d)\n",
				atomic_read(&state_g.work_avail), atomic_read(&state_g.num_ds), state_g.next_ds);

	spin_lock_bh (&spu_blog_offload_lock);
	for (i = 0; i < MAX_SPU_OFFLOAD_SESSIONS/2; i++) {
		session = &state_g.sessions[i];
		if (atomic_read(&session->taken) == 0)
			continue;
		out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"session %d spi 0x%x taken %d ref_count %d evict %d rdy %d\n", i, session->parm.esp_spi,
				atomic_read(&session->taken), atomic_read(&session->ref_count),
				atomic_read(&session->evict), atomic_read(&session->evict_rdy));
	}

	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"%d US offload sessions....(next_us %d)\n", atomic_read(&state_g.num_us), state_g.next_us);
	for (i = MAX_SPU_OFFLOAD_SESSIONS/2; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &state_g.sessions[i];
		if (atomic_read(&session->taken) == 0)
			continue;
		out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				"session %d spi 0x%x taken %d ref_count %d evict %d rdy %d\n", i, session->parm.esp_spi,
				atomic_read(&session->taken), atomic_read(&session->ref_count),
				atomic_read(&session->evict), atomic_read(&session->evict_rdy));
	}
	spin_unlock_bh (&spu_blog_offload_lock);
	return out_offset;
}

static void evict_sched_timer (struct timer_list *t)
{
	int i, flag = 0;
	struct spu_blog_offload_session *session;

	spin_lock_bh (&spu_blog_offload_lock);
	for (i = MAX_SPU_OFFLOAD_SESSIONS/2; i < MAX_SPU_OFFLOAD_SESSIONS; i++) {
		session = &state_g.sessions[i];
		if (atomic_read(&session->evict_rdy) == 1) {
			if (jiffies != session->evict_time) {
				atomic_set(&session->evict_rdy, 2);
				atomic_inc(&state_g.work_avail);
				flag = 1;
			}
		}
	}
	spin_unlock_bh (&spu_blog_offload_lock);

	if (flag)
		SPU_WAKEUP_EVICTHDL();
}

int spu_offload_init(void)
{
	char threadname[15] = {0};
	struct task_struct *thread;

	spin_lock_init(&spu_blog_offload_lock);

	memset(&state_g, 0, sizeof(struct spu_offload_state));

	timer_setup(&state_g.sched_timer, evict_sched_timer, 0);

	init_waitqueue_head(&state_g.evict_wqh);
	sprintf(threadname, "spu_evict_hdl");
	thread = kthread_create(spu_offload_evict_handler, NULL, threadname);

	if (IS_ERR(thread)){
		pr_err("Failed to create %s kthread\n", threadname);
		return -1;
	}
	atomic_set(&state_g.work_avail, 0);
	atomic_set(&state_g.num_us, 0);
	atomic_set(&state_g.num_ds, 0);
	state_g.evict_thread = thread;
	wake_up_process(thread);

	return spu_platform_offload_init();
}

static int spu_blog_offload_flowevent(struct notifier_block *nb, unsigned long event, void *info)
{
	enum spu_stream_type stream;
	BlogFlowEventInfo_t *einfo = info;
	struct iproc_ctx_s *ctx = NULL;
	int offload_id = -1;
	struct spu_blog_offload_session *session;

	if (einfo->spu_offload_us) {
		stream = SPU_STREAM_US;
	} else if (einfo->spu_offload_ds) {
		stream = SPU_STREAM_DS;
	} else {
		flow_log("not spu offload flows, no action required\n");
		goto notify_done;
	}

	if (event == FLOW_EVENT_DEACTIVATE && einfo->flow_event_type == FLOW_EVENT_TYPE_HW) {
		ctx = spu_blog_lookup_ctx_bychanid(einfo->tx_channel, stream);
		if (ctx)
			offload_id = atomic_read(&ctx->offload_id);
		if (ctx == NULL || offload_id == -1) {
			flow_log("ctx for channel %d invalid or not offloaded\n", einfo->tx_channel);
			goto notify_done;
		}

		session = &state_g.sessions[offload_id];

		spin_lock_bh (&spu_blog_offload_lock);
		if (atomic_read(&session->taken) != 0) {
			int old_ref;

			old_ref = atomic_read(&session->ref_count);

			atomic_dec_if_positive(&session->ref_count);
			if (old_ref && atomic_read(&session->ref_count) == 0) {
				atomic_inc(&session->evict);
				if (session->parm.is_enc == 0) {
					atomic_set(&session->evict_rdy, 2);
					atomic_inc(&state_g.work_avail);
					SPU_WAKEUP_EVICTHDL();
				} else {
					atomic_set(&session->evict_rdy, 1);
					session->evict_time = jiffies;
					mod_timer(&state_g.sched_timer,
						jiffies + 2);
				}
			}
		} else {
			pr_err("session %d spi 0x%x is not taken, cannot be freed\n",
				offload_id, session->parm.esp_spi);
		}
		spin_unlock_bh (&spu_blog_offload_lock);
	}
notify_done:
	return NOTIFY_OK;
}

static struct notifier_block _spu_blog_offload_notifier = {
	.notifier_call = spu_blog_offload_flowevent,
};

void spu_blog_offload_register(void)
{
	spu_platform_offload_register();
	blog_flowevent_register_notifier(&_spu_blog_offload_notifier);
	bcmFun_reg(BCM_FUN_ID_SPUOFFLOAD_STATS_UPDATE, spu_update_xfrm_offload_stats);
#if defined(CC_CMD_PARM)
	bcmFun_reg(BCM_FUN_ID_SPUOFFLOAD_SESSION_PARM, spu_offload_parm);
#endif
#if defined(CC_CMD_PREPEND)
	bcmFun_reg(BCM_FUN_ID_SPU_PREPEND_HDR, spu_offload_prepend_hdr);
#endif
}

void spu_blog_offload_deregister(void)
{
	int i;

	blog_flowevent_unregister_notifier(&_spu_blog_offload_notifier);
	/* sanity check, we should have no more offload sessions */
	for (i=0; i < MAX_SPU_OFFLOAD_SESSIONS; i++)
	{
		if (atomic_read(&state_g.sessions[i].taken))
			pr_err("spu offload active session %d spi 0x%x\n",
				i, state_g.sessions[i].parm.esp_spi);
	}
	bcmFun_dereg(BCM_FUN_ID_SPUOFFLOAD_STATS_UPDATE);
#if defined(CC_CMD_PARM)
	bcmFun_dereg(BCM_FUN_ID_SPUOFFLOAD_SESSION_PARM);
#endif
#if defined(CC_CMD_PREPEND)
	bcmFun_dereg(BCM_FUN_ID_SPU_PREPEND_HDR);
#endif
	spu_platform_offload_deregister();
}
