/*
<:copyright-BRCM:2016:GPL/GPL:spu

   Copyright (c) 2016 Broadcom
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

struct esp_skb_cb {
	struct xfrm_skb_cb xfrm;
	void *tmp;
};

static int spu_blog_us_dev_xmit_args (pNBuff_t pNBuf, struct net_device *dev, BlogFcArgs_t *args);
static int spu_blog_ds_dev_xmit_args (pNBuff_t pNBuf, struct net_device *dev, BlogFcArgs_t *args);

#define ESP_SKB_CB(__skb) ((struct esp_skb_cb *)&((__skb)->cb[0]))

static u8 spu_blog_chan_id_lookup(u32 *bitmap)
{
    int i, pos;
    u8 idx = BLOG_CHAN_IDX_UNDEF;

    for (i=0; i < 8; i++)
    {
        if (bitmap[i] == 0xffffffff)
            continue;

        /* find a channel idx that is free */
        pos = ffz(bitmap[i]);

        if (pos < 32)
        {
            idx = ((i << 5) + pos);
            break;
        }
    }
    /* set the channel idx */
    if (idx < BLOG_CHAN_IDX_UNDEF)
    {
        bitmap[i] |= 1 << pos;
    }

    return idx;
}

void spu_blog_ctx_add(struct iproc_ctx_s *ctx)
{
    flow_log("%s: ctx %p\n", __func__, ctx);
    if (ctx->cipher.mode == CIPHER_MODE_CBC ||
        ctx->cipher.mode == CIPHER_MODE_GCM ||
        ctx->cipher.mode == CIPHER_MODE_NONE)
    {
        write_lock(&iproc_priv.ctxListLock[ctx->stream]);
	ctx->blog_chan_id = spu_blog_chan_id_lookup(iproc_priv.blog_chan_bitmap[ctx->stream]);
        atomic_set(&ctx->offload_id, -1);
        list_add(&ctx->entry, &iproc_priv.ctxList[ctx->stream]);
        write_unlock(&iproc_priv.ctxListLock[ctx->stream]);
    }
    flow_log("%s - ctx added stream %d spi 0x%x chan_id %d\n", __func__, ctx->stream, ctx->spi, ctx->blog_chan_id);
}

static void flush_notify_callback(void *data)
{
    kfree(data);
}

void spu_blog_evict(struct iproc_ctx_s *ctx)
{
    BlogFlushParams_t *params;
    struct net_device *dev;

    if (ctx->stream == SPU_STREAM_US)
        dev = iproc_priv.spu_dev_us;
    else
        dev = iproc_priv.spu_dev_ds;

#if defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    {
        int offload_id = atomic_read(&ctx->offload_id);

        if(offload_id != -1) {
            atomic_set(&ctx->offload_id, -1);
            spu_offload_session_free(offload_id);
        }
    }
#endif
    params = (BlogFlushParams_t *)kzalloc(sizeof(BlogFlushParams_t), GFP_KERNEL);
    if (params == NULL) {
        pr_err("failed to allocate params memory for evict\n");
        return;
    }
    params->flush_dev = 1;
    params->devid = dev->ifindex;
    params->flush_chan = 1;
    params->chan_id = ctx->blog_chan_id;
    blog_notify_async(FLUSH, dev, (unsigned long)params, 0, flush_notify_callback, params);
}

void spu_blog_ctx_del(struct iproc_ctx_s *ctx)
{
    struct iproc_ctx_s *sctx;
    struct iproc_ctx_s *pos;
    int evict_id = BLOG_CHAN_IDX_UNDEF;

    flow_log("%s: ctx %p\n", __func__, ctx);

    write_lock(&iproc_priv.ctxListLock[ctx->stream]);
    /* if a ctx does not get used it won't be in the list */
    list_for_each_entry_safe(sctx, pos, &iproc_priv.ctxList[ctx->stream], entry)
    {
        if (sctx == ctx)
        {
            if (atomic_read(&ctx->offload_id) != -1)
                flow_log("ctx for spi 0x%x still holding offload session %d\n",
                         ctx->spi, atomic_read(&ctx->offload_id));
            if (ctx->blog_chan_id < BLOG_CHAN_IDX_UNDEF)
            {
                evict_id = ctx->blog_chan_id;
                iproc_priv.blog_chan_bitmap[ctx->stream][ctx->blog_chan_id >> 5] &= ~(1 << (ctx->blog_chan_id & 0x1F));
            }
            list_del(&ctx->entry);
            break;
        }
    }
    write_unlock(&iproc_priv.ctxListLock[ctx->stream]);

    if (evict_id < BLOG_CHAN_IDX_UNDEF)
        spu_blog_evict(ctx);
}

static inline struct iproc_ctx_s *spu_blog_lookup_ctx(uint32_t spi, enum spu_stream_type stream)
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
}  /* spu_blog_lookup_ctx() */

int spu_blog_emit_aead_us(struct iproc_reqctx_s *rctx, int is_esn)
{
    struct sk_buff    *pSkb;
    struct net_device *skb_dev;
    unsigned char     *pdata;
    struct iphdr      *iph;
    struct ip_esp_hdr *esph;
    struct blog_t     *blog_p;
    BlogFcArgs_t       fcArgs={};
    unsigned char      nexthdr;
    int                esph_offset;
    int                esp_mode = BLOG_ESP_MODE_TUNNEL;
    uint32_t           esp_spi, esp_over_udp = 0;
    struct aead_request *req;
    struct crypto_aead *aead;
    struct xfrm_state *xfrm;

    flow_log("%s\n", __func__);

    pSkb = rctx->parent->data;
    rctx->pNBuf = SKBUFF_2_PNBUFF(pSkb);
    xfrm = skb_dst(pSkb)->xfrm;

    if ( pSkb->protocol != htons(ETH_P_IP) ||
         !(rctx->ctx->cipher.mode == CIPHER_MODE_CBC ||
           rctx->ctx->cipher.mode == CIPHER_MODE_GCM ||
           rctx->ctx->cipher.mode == CIPHER_MODE_NONE))
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return 0;
    }

    /* we will not be able to accelerate ESN for GCM flows */
    if (rctx->ctx->cipher.mode == CIPHER_MODE_GCM && 
        xfrm->props.flags & XFRM_STATE_ESN) {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return 0;
    }

    /* ignore packets needing more than one transform */
    if ( !xfrm ||
         xfrm_dst_child(skb_dst(pSkb))->xfrm )
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return 0;
    }

    if ( xfrm->props.mode == XFRM_MODE_TRANSPORT )
        esp_mode = BLOG_ESP_MODE_TRANSPORT;
    else if ( xfrm->props.mode != XFRM_MODE_TUNNEL )
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return 0;
    }

    if ( xfrm->encap )
    {
        esp_over_udp = 1;
        flow_log("US: esp_over_udp<%d>\n", esp_over_udp);
    }

    /* pSkb->data should point to the outer IP header */
    pdata = pSkb->data;
    nexthdr = pdata[pSkb->len - rctx->ctx->digestsize - 1];
    // TODO: handle ipv6
    flow_log("*pdata 0x%x transport_mode %d nexthdr %d blog %px",
        *pdata, esp_mode, nexthdr, blog_ptr(pSkb));
    if ( *pdata != 0x45 || (esp_mode != BLOG_ESP_MODE_TRANSPORT && nexthdr != IPPROTO_IPIP) )
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return 0;
    }

    /* need to clone dst so that it can be used for packets that cannot be
       accelerated after encryption - dst should always be referenced */
    blog_p = blog_ptr(pSkb);
    if ( (blog_p != NULL) )
    {
        blog_p->esptx.dst_p = dst_clone(skb_dst(pSkb));
        if ( 0 == (skb_dst(pSkb)->xfrm->props.flags & XFRM_STATE_NOPMTUDISC) )
        {
            blog_p->esptx.pmtudiscen = 1;
        }
        blog_p->tx.info.bmap.ESP = 1;
        blog_p->esp_mode = esp_mode;
    }
#if !defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    /* if HW offload is enabled, packets without blog pointer
     * may still be processed by HW for sequence number handling */
    else
    {
        return 0;
    }
#endif

    iph = (struct iphdr *)pdata;

    if (iph->protocol == BLOG_IPPROTO_UDP)
        esph_offset = BLOG_IPV4_HDR_LEN + BLOG_UDP_HDR_LEN;
    else
        esph_offset = BLOG_IPV4_HDR_LEN;

    esph = (struct ip_esp_hdr *)(pdata + esph_offset);

    esp_spi = _read32_align16((uint16_t *)&esph->spi);

    if (esp_over_udp &&
        (esp_spi == 0 || esp_spi == htonl(0xFF00000)))
    {
        flow_log("skip learning on ISAKMP / NAT_keepalive packets (esp_spi = 0x%x)\n",
                 esp_spi);
        return 0;
    }

    req = container_of((void *)rctx, struct aead_request, __ctx);
    if (rctx->ctx->cipher.mode == CIPHER_MODE_GCM)
    {
        struct aead_request *req = container_of((void *)rctx, struct aead_request, __ctx);
        uint32_t seqno = ntohl(_read32_align16((uint16_t *)&esph->seq_no));

        memcpy(rctx->ctx->iv, req->iv, 8);
        *(__be64 *)(rctx->ctx->iv) ^= cpu_to_be64(seqno);
    }

    aead = crypto_aead_reqtfm(req);

    iph->tot_len = htons(pSkb->len);
    iph->check   = 0;
    iph->check   = ip_fast_csum((unsigned char *)iph, iph->ihl);

    /* fill the dst IP address needed by blog_emit_args */
    if (is_esn)
        memcpy((uint8_t *)esph - BLOG_ESP_SEQNUM_HI_LEN, ESP_SKB_CB(pSkb)->tmp, BLOG_ESP_SEQNUM_HI_LEN);

    if (esp_spi != rctx->ctx->spi)
    {
        rctx->ctx->ipdaddr = _read32_align16((uint16_t *)&iph->daddr);
        rctx->ctx->ipsaddr = _read32_align16((uint16_t *)&iph->saddr);
        rctx->ctx->spi     = esp_spi;
        rctx->ctx->stream = SPU_STREAM_US;
        rctx->ctx->esp_over_udp = esp_over_udp;
        rctx->ctx->esp_mode = esp_mode;
        rctx->ctx->iv_size = rctx->iv_ctr_len - rctx->ctx->salt_len;
        spu_blog_ctx_add(rctx->ctx);
        flow_log("upstream ips %x, ipd %x, spi %x, seq %x, nexthdr %d, flags %x esp_over_udp<%d>\n",
                 ntohl(iph->saddr), ntohl(iph->daddr), ntohl(esph->spi),
                 ntohl(esph->seq_no), nexthdr, rctx->parent->flags, rctx->ctx->esp_over_udp );
    }
    /* skip learning the flow when no valid channel is allocated */
    if (rctx->ctx->blog_chan_id == BLOG_CHAN_IDX_UNDEF)
        goto spu_blog_done_emit_us;

    /* insert spu_dev_us as transmit device */
    skb_dev = pSkb->dev;
    pSkb->dev = iproc_priv.spu_dev_us;
    if (blog_p)
    {
        fcArgs.esp_ivsize = rctx->ctx->iv_size;
        fcArgs.esp_icvsize = rctx->ctx->digestsize;
        fcArgs.esp_blksize = ALIGN(crypto_aead_blocksize(aead), 4);
        if (esp_over_udp)
        {
            fcArgs.esp_over_udp = esp_over_udp;
            fcArgs.esp_spi = rctx->ctx->spi;
        }
        blog_p->use_xmit_args = 1;

        fcArgs.dev_xmit = (unsigned long)spu_blog_us_dev_xmit_args;
        blog_emit_args(rctx->pNBuf, iproc_priv.spu_dev_us, TYPE_IP, rctx->ctx->blog_chan_id, BLOG_SPU_US, &fcArgs);
    }
    pSkb->dev = skb_dev;

#if defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    if (esp_mode != BLOG_ESP_MODE_TRANSPORT)
    {
        int session_id = spu_offload_get_us_id(rctx->ctx, _read32_align16((uint16_t *)&esph->spi), blog_p);
        if (session_id > 0)
        {
            uint32_t payloadlen;
            if (spu_offload_insert_us_pkt(rctx->pNBuf, session_id, rctx->ctx->digestsize, &payloadlen) == 0)
            {
                /* packet arriving here has memory allocated from ESP framework as CB memory
                 * packet inserted to the offload datapath will not be returned
                 * to the ESP framework, free CB memory now */
                void *tmp = ESP_SKB_CB(pSkb)->tmp;
                if (req->src != req->dst)
                    pr_err("%s src_sg != dst_sg\n", __func__);
                kfree(tmp);
                spin_lock_bh(&xfrm->lock);
                xfrm->curlft.bytes -= payloadlen;
                xfrm->curlft.packets--;
                spin_unlock_bh(&xfrm->lock);
                return 1;
            }
        }
    }
#endif
spu_blog_done_emit_us:

    /* blog did not take care of the packet
     * put back the ESP EPI in the dst IP address location for crypto/esp framework */
    if (is_esn)
        memcpy((uint8_t *)esph - BLOG_ESP_SEQNUM_HI_LEN, &esp_spi, BLOG_ESP_SEQNUM_HI_LEN);

    return 0;
}

void spu_blog_emit_aead_ds(struct iproc_reqctx_s *rctx)
{
    struct sk_buff    *pSkb;
    struct net_device *skb_dev;
    unsigned char     *pdata;
    struct iphdr      *iph;
    struct ip_esp_hdr *esph;
    BlogFcArgs_t       fcArgs={};
    struct xfrm_state *xfrm;
    unsigned char      nexthdr;
    uint32_t           esp_spi, esp_over_udp = 0, skip_blog = 0;
    int                esph_offset;
    int                esp_mode = BLOG_ESP_MODE_TUNNEL;
    struct sec_path    *sp;
    BlogEsp_t          *esprx;


    flow_log("%s\n", __func__);

    pSkb = rctx->parent->data;
    rctx->pNBuf = SKBUFF_2_PNBUFF(pSkb);

    if (pSkb->dev && !is_netdev_wan(pSkb->dev)) { /* ignore LAN side IPSec connections */
        /* This use case is for IPsec tunnel started by the LAN device and terminated on the GW */
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    if ( pSkb->protocol != htons(ETH_P_IP) ||
         !(rctx->ctx->cipher.mode == CIPHER_MODE_CBC ||
           rctx->ctx->cipher.mode == CIPHER_MODE_GCM ||
           rctx->ctx->cipher.mode == CIPHER_MODE_NONE))
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    /* ignore packets requiring more than one transform */
    sp = skb_sec_path(pSkb);
    if ( !sp || (sp->len != 1) )
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    xfrm = sp->xvec[0];
    if (NULL == xfrm)
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    if ( xfrm->props.mode == XFRM_MODE_TRANSPORT )
    {
        flow_log("transport mode");
        esp_mode = BLOG_ESP_MODE_TRANSPORT;
    }
    else if ( xfrm->props.mode != XFRM_MODE_TUNNEL )
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    if ((xfrm->props.flags & XFRM_STATE_ESN) && 
        (rctx->ctx->cipher.mode == CIPHER_MODE_GCM))
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    if (xfrm->encap)
    {
        esp_over_udp = 1;
        flow_log("esp_over_udp<%d>\n", esp_over_udp);
    }

    // TODO: handle ipv6
    if (esp_over_udp)
        esph_offset = BLOG_IPV4_HDR_LEN + BLOG_UDP_HDR_LEN;
    else
        esph_offset = BLOG_IPV4_HDR_LEN;

    /* skb->data will point to the ESP header, need to push data back
       to the outer IP header */
    if (xfrm->props.flags & XFRM_STATE_ESN)
	    esph_offset -= BLOG_ESP_SEQNUM_HI_LEN;

    pdata = skb_push(pSkb, esph_offset);
    nexthdr = pdata[pSkb->len - rctx->ctx->digestsize - 1];

    flow_log("*pdata 0x%x transport_mode %d nexthdr %d blog %px esph_offset %d",
        *pdata, esp_mode, nexthdr, blog_ptr(pSkb), esph_offset);
    if (*pdata != 0x45 || (esp_mode != BLOG_ESP_MODE_TRANSPORT && nexthdr != IPPROTO_IPIP))
    {
        skb_pull(pSkb, esph_offset);
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    /* save the secpath so that it can be used to pass packets to the kernel
       when they cannot be accelerated after decryption */
    if ( blog_ptr(pSkb) == NULL )
    {
        skb_pull(pSkb, esph_offset);
        return;
    }

    esprx = &blog_ptr(pSkb)->esprx;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    esprx->secPath_p = secpath_get(pSkb->sp);
#else
    if (xfrm != esprx->xfrm_st)
    {
        flow_log("%s():%d blog_p %p xfrm old %p cur %p\n",
            __func__, __LINE__, blog_ptr(pSkb), esprx->xfrm_st, xfrm);
        if (esprx->xfrm_st)
	        xfrm_state_put(esprx->xfrm_st);
        xfrm_state_hold(xfrm);
        flow_log("%s:%d held xfrm=%p refcnt=%d\n",
                __func__, __LINE__, xfrm, refcount_read(&xfrm->refcnt));
        esprx->xfrm_st = xfrm;
    }
#endif
    blog_ptr(pSkb)->rx.info.bmap.ESP = 1;
    blog_ptr(pSkb)->esp_mode = esp_mode;

    /* insert spu_dev_ds as transmit device */
    skb_dev = pSkb->dev;
    pSkb->dev = iproc_priv.spu_dev_ds;

    iph = (struct iphdr *)pdata;

    if (xfrm->props.flags & XFRM_STATE_ESN)
        memcpy(&iph->daddr, ESP_SKB_CB(pSkb)->tmp, BLOG_ESP_SEQNUM_HI_LEN);

    if (esp_over_udp)
        esph = (struct ip_esp_hdr *)(pdata + esph_offset);
    else
        esph = (struct ip_esp_hdr *)(pdata + BLOG_IPV4_HDR_LEN);

    esp_spi = _read32_align16((uint16_t *)&esph->spi);

    if (esp_over_udp && (esp_spi == 0 || esp_spi == htonl(0xFF00000)))
    {
        flow_log("skip learning on ISAKMP / NAT_keepalive packets (esp_spi = 0x%x)\n",
                  esp_spi);
        skip_blog = 1;
    }

    flow_log("skip_blog %d esp_spi 0x%xi ctx spi 0x%x", skip_blog, esp_spi, rctx->ctx->spi);
    if (skip_blog == 0)
    {
        if (esp_spi != rctx->ctx->spi)
        {
            rctx->ctx->ipdaddr = _read32_align16((uint16_t *)&iph->daddr);
            rctx->ctx->ipsaddr = _read32_align16((uint16_t *)&iph->saddr);
            rctx->ctx->spi     = _read32_align16((uint16_t *)&esph->spi);
            rctx->ctx->stream = SPU_STREAM_DS;
            rctx->ctx->iv_size = rctx->iv_ctr_len - rctx->ctx->salt_len;
            spu_blog_ctx_add(rctx->ctx);
            flow_log("downstream ips %x ipd %x spi %x seq %x nexthdr %d "
                     "flags %x esp_over_udp %d transport_mode %d",
                     ntohl(iph->saddr), ntohl(iph->daddr), ntohl(esph->spi),
                     ntohl(esph->seq_no), nexthdr, rctx->parent->flags,
                     rctx->ctx->esp_over_udp, esp_mode);
        }
    
        flow_log("blog_chan_id %d", rctx->ctx->blog_chan_id);
        /* learn the flow only if a valid channel is allocated */
        if (rctx->ctx->blog_chan_id < BLOG_CHAN_IDX_UNDEF)
        {
            if (esp_over_udp)
            {
                fcArgs.esp_over_udp = esp_over_udp;
                fcArgs.esp_spi = esp_spi;
            }
            blog_ptr(pSkb)->use_xmit_args = 1;
            fcArgs.dev_xmit = (unsigned long)spu_blog_ds_dev_xmit_args;
            fcArgs.esp_ivsize = rctx->ctx->iv_size;
            fcArgs.esp_icvsize = rctx->ctx->digestsize;

            blog_emit_args(rctx->pNBuf, iproc_priv.spu_dev_ds, TYPE_IP, rctx->ctx->blog_chan_id, BLOG_SPU_DS, &fcArgs);
        }
    }

    skb_pull(pSkb, esph_offset);
    pSkb->dev = skb_dev;
}

static void spu_blog_fc_crypt_done_us(struct bcmspu_message *mssg)
{
    struct iproc_reqctx_s *rctx = container_of(mssg, struct iproc_reqctx_s, mssg);
    struct device *dev = &iproc_priv.pdev->dev;
    struct spu_hw *spu = &iproc_priv.spu;
    int                    err = 0;
    struct sk_buff        *skb;
    BlogAction_t           blogAction;
    unsigned int           pktlen;
    int                    headroom;
    unsigned char         *pData;
    BlogFcArgs_t           fcArgs={};

    flow_log("%s start\n", __func__);

    dma_unmap_sg(dev, mssg->src, sg_nents(mssg->src), DMA_FROM_DEVICE);
    dma_unmap_sg(dev, mssg->dst, sg_nents(mssg->dst), DMA_FROM_DEVICE);
    if ( mssg->src != rctx->src_sg )
    {
        kfree(mssg->src);
    }

    if ( mssg->dst != rctx->dst_sg )
    {
        kfree(mssg->dst);
    }

    /* process the SPU status */
    err = spu->spu_status_process(rctx->msg_buf.rx_stat);
    if (err != 0) {
        if (err == SPU_INVALID_ICV)
            atomic_inc(&iproc_priv.bad_icv);
        nbuff_free(rctx->pNBuf);
        dst_release(rctx->dst);
        kfree(rctx);
        flow_log("SPU process error\n");
        return;
    }

    /* Process the SPU response message */
    handle_aead_resp(rctx);

    /* need to save some things here as nbuf may be consumed */
    fcArgs.esp_ivsize = rctx->iv_ctr_len;
    fcArgs.esp_icvsize = rctx->ctx->digestsize;
    fcArgs.esp_over_udp = rctx->ctx->esp_over_udp;
    fcArgs.esp_mode = rctx->ctx->esp_mode;
    fcArgs.esp_spi = rctx->ctx->spi;

    if ( IS_SKBUFF_PTR(rctx->pNBuf) )
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(rctx->pNBuf);
        pData = skb->data;
        pktlen = skb->len;
        headroom = 0;
        blogAction = blog_sinit(skb, iproc_priv.spu_dev_us,
                                TYPE_IP, 0, BLOG_SPU_US, &fcArgs);
    }
    else
    {
        FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_FKBUFF(rctx->pNBuf);
        headroom = (int)(fkb->data - PFKBUFF_TO_PHEAD(fkb));
        pData = fkb->data;
        pktlen = fkb->len;
        skb = NULL;
        blogAction = blog_finit(fkb, iproc_priv.spu_dev_us,
                                TYPE_IP, rctx->ctx->blog_chan_id, BLOG_SPU_US, &fcArgs);
    }

    if (blogAction == PKT_DROP)
    {
        flow_log("%s:%d - blogAction == PKT_DROP\n", __func__, __LINE__);
        if (rctx->pNBuf)
            nbuff_free(rctx->pNBuf);
        dst_release(rctx->dst);
        kfree(rctx);
        return;
    }

    if ((blogAction == PKT_NORM) || (blogAction == PKT_BLOG))
    {
        flow_log("%s:%d - not blogged\n", __func__, __LINE__);
        /* if nbuf was an fkb then translate to skb */
        if ( NULL == skb )
        {
            /* for PKT_NORM, fkb was released so initialize it again
               in order for xlate to work */
            if ( blogAction == PKT_NORM )
            {
                fkb_init(pData, headroom, pData, pktlen);
            }
            skb = nbuff_xlate(rctx->pNBuf);
        }

        /* dst already ref counted */
        skb_dst_set(skb, dst_clone(rctx->dst));
        skb_reset_network_header(skb);
        skb->dev = iproc_priv.spu_dev_us;
        xfrm_output_resume(skb, 0);
    }
    else
    {
        atomic_inc(&iproc_priv.blogged[SPU_STREAM_US]);
        flow_log("%s:%d - PKT_DONE\n", __func__, __LINE__);
    }
    dst_release(rctx->dst);
    kfree(rctx);

} /* spu_blog_fc_crypt_done_us */

static void spu_blog_fc_crypt_done_ds(struct bcmspu_message *mssg)
{
    struct iproc_reqctx_s *rctx = container_of(mssg, struct iproc_reqctx_s, mssg);
    struct device *dev = &iproc_priv.pdev->dev;
    struct spu_hw *spu = &iproc_priv.spu;
    int err = 0;
    struct sk_buff    *skb;
    FkBuff_t          *fkb;
    struct xfrm_state *xfrm;
    BlogAction_t       blogAction;
    unsigned int       hlen, tlen;
    unsigned int       pktlen;
    unsigned char      padlen;
    unsigned char      nexthdr;
    unsigned int       seqno;
    int                headroom;
    unsigned char     *pData;
    BlogFcArgs_t       fcArgs={};
    int                esph_offset;
    struct iphdr      *iph;

    dma_unmap_sg(dev, mssg->src, sg_nents(mssg->src), DMA_FROM_DEVICE);
    dma_unmap_sg(dev, mssg->dst, sg_nents(mssg->dst), DMA_FROM_DEVICE);
    if ( mssg->src != rctx->src_sg )
    {
        kfree(mssg->src);
    }

    if ( mssg->dst != rctx->dst_sg )
    {
        kfree(mssg->dst);
    }

    flow_log("%s\n", __func__);
    /* process the SPU status */
    err = spu->spu_status_process(rctx->msg_buf.rx_stat);
    if (err != 0) {
        if (err == SPU_INVALID_ICV)
            atomic_inc(&iproc_priv.bad_icv);
        flow_log("SPU process error\n");
        goto exit;
    }

    /* Process the SPU response message */
    handle_aead_resp(rctx);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    xfrm = rctx->sp->xvec[rctx->sp->len-1];
#else
    xfrm = rctx->xfrm_st;
#endif
    if ( NULL == xfrm )
    {
        flow_log("spu_blog_fc_crypt_done_ds: xfrm is NULL\n");
        goto exit;
    }

    /* need to save some things here as nbuf may be consumed */
    /* specify this flow is for inner IP packet only */
    fcArgs.esp_inner_pkt = 1;
    fcArgs.esp_spi = 0;
    fcArgs.esp_over_udp = rctx->ctx->esp_over_udp;
    if (xfrm->props.mode == XFRM_MODE_TRANSPORT)
        fcArgs.esp_mode = BLOG_ESP_MODE_TRANSPORT;

    if ( IS_SKBUFF_PTR(rctx->pNBuf) )
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(rctx->pNBuf);
        pktlen = skb->len;
        pData = skb->data;
    }
    else
    {
        fkb = (FkBuff_t *)PNBUFF_2_PBUF(rctx->pNBuf);
        pktlen = fkb->len;
        pData = fkb->data;
    }

    // TODO: handle ipv6
    iph = (struct iphdr *)pData;
    if (iph->protocol == BLOG_IPPROTO_UDP)
        esph_offset = BLOG_IPV4_HDR_LEN + BLOG_UDP_HDR_LEN;
    else
        esph_offset = BLOG_IPV4_HDR_LEN;

    hlen = esph_offset + BLOG_ESP_SPI_LEN;
    seqno = _read32_align16((uint16_t *)&pData[hlen]);
    hlen += BLOG_ESP_SEQNUM_LEN + rctx->iv_ctr_len - rctx->ctx->salt_len;
    padlen  = pData[pktlen - rctx->ctx->digestsize - 2];
    nexthdr = pData[pktlen - rctx->ctx->digestsize - 1];
    tlen = rctx->ctx->digestsize + padlen + 2;

    if ( IS_SKBUFF_PTR(rctx->pNBuf) )
    {
        headroom = 0;

        /* trim padding and icv */
        __skb_trim(skb, skb->len - tlen);

        /* pull to inner header */
        __skb_pull(skb, hlen);
        if (xfrm->props.mode == XFRM_MODE_TRANSPORT)
        {
            short tot_len = ntohs(iph->tot_len);

            flow_log("%s:%d transport mode, skb %px data %px len %d ip tot_len %d proto %d esph_offset %d nexthdr %d",
                __func__, __LINE__, skb, skb->data, skb->len, tot_len, iph->protocol, esph_offset, nexthdr);

            __skb_push(skb, BLOG_IPV4_HDR_LEN);
            tot_len += (BLOG_IPV4_HDR_LEN - hlen - tlen);
            iph->tot_len = htons(tot_len);
            iph->protocol = nexthdr;
            ip_send_check(iph);
            memmove(skb->data, iph, BLOG_IPV4_HDR_LEN);
        }
        skb_reset_network_header(skb);
        skb_reset_mac_header(skb);
        skb_set_transport_header(skb, 0);

        pktlen = skb->len;
        pData = skb->data;

        blogAction = blog_sinit(skb, iproc_priv.spu_dev_ds,
                                TYPE_IP, 0, BLOG_SPU_DS, &fcArgs);
    }
    else
    {
        fkb->len -= tlen;
        _fkb_pull(fkb, hlen);
        if (xfrm->props.mode == XFRM_MODE_TRANSPORT)
        {
            short tot_len = ntohs(iph->tot_len);

            flow_log("%s:%d transport mode, fkb %px data %px len %d ip tot_len %d proto %d esph_offset %d nexthdr %d",
                __func__, __LINE__, fkb, fkb->data, fkb->len, tot_len, iph->protocol, esph_offset, nexthdr);

            _fkb_push(fkb, BLOG_IPV4_HDR_LEN);
            tot_len += (BLOG_IPV4_HDR_LEN - hlen - tlen);
            iph->tot_len = htons(tot_len);
            iph->protocol = nexthdr;
            ip_send_check(iph);
            memmove(fkb->data, iph, BLOG_IPV4_HDR_LEN);
        }
        headroom = (int)(fkb->data - PFKBUFF_TO_PHEAD(fkb));

        pktlen = fkb->len;
        pData = fkb->data;

        skb = NULL;
        blogAction = blog_finit(fkb, iproc_priv.spu_dev_ds,
                                TYPE_IP, rctx->ctx->blog_chan_id, BLOG_SPU_DS, &fcArgs);
    }

    if (blogAction == PKT_DROP)
    {
        flow_log("%s:%d - blogAction == PKT_DROP\n", __func__, __LINE__);
        goto exit;
    }

    if ((blogAction == PKT_NORM) || (blogAction == PKT_BLOG))
    {
        int seq_hi;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0))
        struct sec_path   *sp;
#endif
        flow_log("%s:%d - Fwd to Linux blogAction<%s>\n", __func__, __LINE__, blogAction==PKT_NORM?"NORM":"BLOG");

        /* nbuf is an fkb - translate to skb */
        if ( skb == NULL )
        {
            /* for PKT_NORM, fkb was released so initialize it again
            in order for xlate to work */
            if ( blogAction == PKT_NORM )
            {
                fkb_init(pData, headroom, pData, pktlen);
            }
            skb = nbuff_xlate(rctx->pNBuf);
        }

        skb->dev = iproc_priv.spu_dev_ds;
        seq_hi = htonl(xfrm_replay_seqhi(xfrm, seqno));
        XFRM_SKB_CB(skb)->seq.input.low = seqno;
        XFRM_SKB_CB(skb)->seq.input.hi = seq_hi;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
        skb->sp = secpath_get(rctx->sp);
#else
        sp = secpath_set(skb);
        if (!sp)
        {
            flow_log("%s:%d - failed to set secpath for skb %p\n",
                __func__, __LINE__, skb);
            goto exit;
        }
        xfrm_state_hold(xfrm);
        flow_log("%s:%d held xfrm=%p refcnt=%d\n",
                __func__, __LINE__, xfrm, refcount_read(&xfrm->refcnt));
	    sp->xvec[sp->len++] = xfrm;
#endif
        dev_hold(skb->dev);
        if (xfrm->props.mode == XFRM_MODE_TRANSPORT)
        {
            skb_reset_network_header(skb);
            skb_reset_transport_header(skb);
            __skb_pull(skb, BLOG_IPV4_HDR_LEN);      
        }
        xfrm_input_resume(skb, nexthdr);
        rctx->pNBuf = NULL;
    }
    else
    {
        flow_log("%s:%d - Accelerated blogAction == PKT_DONE\n", __func__, __LINE__);
        atomic_inc(&iproc_priv.blogged[SPU_STREAM_DS]);
        spin_lock_bh(&xfrm->lock);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
        xfrm->repl->advance(xfrm, seqno);
#else
        xfrm_replay_advance(xfrm, seqno);
#endif
        xfrm->curlft.bytes += pktlen;
        xfrm->curlft.packets++;
        spin_unlock_bh(&xfrm->lock); 
        rctx->pNBuf = NULL;
    }
exit:

    /* free the request context */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    secpath_put(rctx->sp);
#else
    xfrm_state_put(rctx->xfrm_st);
#endif
    if (rctx->pNBuf)
        nbuff_free(rctx->pNBuf);
    kfree(rctx);

}  /* spu_blog_fc_crypt_done_ds */

static int spu_blog_xmit_us(pNBuff_t pNBuf, struct net_device *dev)
{
    flow_log("spu_blog_xmit_us slow path\n");

    nbuff_free(pNBuf);
    return 0;
}

static int spu_blog_us_dev_xmit_args (pNBuff_t pNBuf, struct net_device *dev, BlogFcArgs_t *args)
{
    struct iproc_reqctx_s *rctx;
    struct iproc_ctx_s    *ctx;
    uint32_t               spi;
    int                    ret = 0;
    struct xfrm_state     *xfrm;
    struct net            *net;
    uint32_t               seqno, seqhi = 0;
    unsigned char         *pdata;
    struct sk_buff        *skb;
    int                    nbuflen;
    void                  *ctx_buf;
    struct dst_entry      *dst;
    struct scatterlist    *sg;
    int                    datalen;
    int                    padlen;
    struct device *spu_dev = &iproc_priv.pdev->dev;
    int                    esph_offset;
    struct iphdr          *iph;
    int                   free_buf = 1;

    flow_log("%s\n", __func__);

    if (args->esptx_dst_p == NULL)
    {
        flow_log("esptx_dst_p cannot be NULL\n");
        nbuff_free(pNBuf);
        return -1;
    }

    if (IS_SKBUFF_PTR(pNBuf))
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);
        /* blog is owned by fc so void reference */
        skb->blog_p = NULL;
        pdata = skb->data;
        nbuflen = skb->len;
        /* make sure dirty_p is NULL */
        skb_shinfo(skb)->dirty_p = NULL;
    }
    else
    {
        FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pNBuf);
        fkb = fkb_unshare(fkb);
        /* blog is owned by fc so void reference */
        fkb->blog_p = NULL;
        pdata = fkb->data;
        nbuflen = fkb->len;
        skb = NULL;
    }

    // TODO: handle ipv6
    iph = (struct iphdr *)pdata;
    if (iph->protocol == BLOG_IPPROTO_UDP)
        esph_offset = BLOG_IPV4_HDR_LEN + BLOG_UDP_HDR_LEN;
    else
        esph_offset = BLOG_IPV4_HDR_LEN;

    spi = _read32_align16((uint16_t *)&pdata[esph_offset]);

    ctx = spu_blog_lookup_ctx(spi, SPU_STREAM_US);
    if ( NULL == ctx )
    {
        flow_log("spu_blog_lookup_ctx failed: spi 0x%x\n", htonl(spi));
        ret = -100;
        goto exit;
    }

#if defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    {
        int session_id = spu_offload_get_us_id(ctx, spi, NULL);
        if (session_id > 0)
        {
            uint32_t payloadlen;
            /* packet inserted to offload path, exit here */
            if (spu_offload_insert_us_pkt(pNBuf, session_id, ctx->digestsize, &payloadlen) == 0) {
                free_buf = 0;
                goto exit;
            }
        }
    }
#endif

    dst = args->esptx_dst_p;
    xfrm = dst->xfrm;
    if ( NULL == xfrm )
    {
        flow_log("Packet discarded! blog's xfrm state for this flow doesn't exist.\n");
        goto exit;
    }
    net = xs_net(xfrm);

    spin_lock_bh(&xfrm->lock);
    do
    {
        /* A new key has genid = 0. When the key is renewed, genid is incremented.
        * We want to evict the flow from fcache when the key has been renewed
        * or expired.
        */
        if (xfrm->genid)
        {
            spu_blog_evict(ctx);
        }

        ret = xfrm_state_check_expire(xfrm);
        if (ret)
        {
            XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEEXPIRED);
            break;
        }

        /* pass NULL skb since we have an fkb
           if there is an error then we will create an skb and call
           again so that the error is reported */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
        ret = xfrm->repl->overflow(xfrm, NULL);
#else
        ret = xfrm_replay_overflow(xfrm, NULL);
#endif
        if (ret)
        {
            if ( NULL == skb )
            {
            /* nbuf is an fkb - translate to skb
            call replay function again for auditing */
                skb = nbuff_xlate(pNBuf);
            }
            skb_reset_network_header(skb);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
            xfrm->repl->overflow(xfrm, skb);
#else
            xfrm_replay_overflow(xfrm, skb);
#endif
            XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
            kfree_skb(skb);
            free_buf = 0;
            break;
        }
        if ( xfrm->replay_esn )
        {
            seqno = xfrm->replay_esn->oseq;
            seqhi = htonl(xfrm->replay_esn->oseq_hi);
        }
        else
        {
            seqno = xfrm->replay.oseq;
        }

        /* only count inner ip header and data */
        padlen  = pdata[nbuflen - ctx->digestsize - 2];
        datalen = nbuflen - esph_offset - padlen - ctx->iv_size -
        BLOG_ESP_SPI_LEN - BLOG_ESP_SEQNUM_LEN - BLOG_ESP_PADLEN_LEN -
        BLOG_ESP_NEXT_PROTO_LEN - ctx->digestsize;
        xfrm->curlft.bytes += datalen;
        xfrm->curlft.packets++;
    } while (0);
    spin_unlock_bh(&xfrm->lock);
    if ( ret != 0 )
    {
        /* an error occur, evict the current flow */
        spu_blog_evict(ctx);
        goto exit;
    }

    _write32_align16((uint16_t *)&pdata[esph_offset+BLOG_ESP_SPI_LEN], htonl(seqno));

    /* allocate a transfer request - data is zeroed */
    ctx_buf = kmalloc(sizeof(struct iproc_reqctx_s) +
                      sizeof(struct scatterlist), GFP_ATOMIC);
    if (NULL == ctx_buf)
    {
        flow_log("Packet discarded! Failed to allocate memory for iproc request.\n");
        ret = -1;
        goto exit;
    }
    rctx = (struct iproc_reqctx_s *)ctx_buf;

    rctx->pNBuf          = pNBuf;
    rctx->iv_ctr_len     = ctx->iv_size + ctx->salt_len;
    rctx->dst            = dst;
    rctx->sp             = NULL;

    rctx->gfp = GFP_ATOMIC;
    rctx->is_encrypt = true;
    rctx->bd_suppress = false;
    rctx->total_todo = (datalen + padlen +
                            BLOG_ESP_PADLEN_LEN + BLOG_ESP_NEXT_PROTO_LEN);
    rctx->src_sent = 0;
    rctx->total_sent = 0;
    rctx->total_received = 0;
    rctx->is_sw_hmac = false;
    rctx->ctx = ctx;
    memset(&rctx->mssg, 0, sizeof(struct bcmspu_message));

    rctx->mssg.rx_callback = spu_blog_fc_crypt_done_us;

    /* Initialize current position in src and dst scatterlists */
    sg = (struct scatterlist *)(ctx_buf + sizeof(struct iproc_reqctx_s));
    rctx->src_sg    = &sg[0];
    rctx->dst_sg    = &sg[0];
    rctx->assoc     = &sg[0];
    rctx->src_nents = 0;
    rctx->dst_nents = 0;
    if (ctx->cipher.mode == CIPHER_MODE_CBC)
    {
        rctx->assoc_len = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN;
        rctx->src_skip  = rctx->assoc_len + rctx->iv_ctr_len;
        rctx->dst_skip  = rctx->assoc_len;
    }
    else
    {
        rctx->assoc_len = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + ctx->iv_size;
        rctx->src_skip  = rctx->assoc_len;
        rctx->dst_skip  = rctx->assoc_len;
    }

    sg_init_one(rctx->src_sg,
                &pdata[esph_offset],
		(rctx->total_todo + rctx->assoc_len + ctx->iv_size + ctx->digestsize));

    if (rctx->iv_ctr_len)
    {
        if (ctx->salt_len)
            memcpy(rctx->msg_buf.iv_ctr + ctx->salt_offset,
                   ctx->salt, ctx->salt_len);
        memcpy(rctx->msg_buf.iv_ctr + ctx->salt_offset + ctx->salt_len,
               ctx->iv,
               rctx->iv_ctr_len - ctx->salt_len - ctx->salt_offset);

	if (ctx->cipher.mode == CIPHER_MODE_CBC)
        {
            *(__be64 *) rctx->msg_buf.iv_ctr ^= cpu_to_be64(seqno);
            memset(&pdata[esph_offset + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN], 0, rctx->iv_ctr_len - ctx->salt_len);
        }
        else if (ctx->cipher.mode == CIPHER_MODE_GCM)
        {
            *(__be64 *)(rctx->msg_buf.iv_ctr + ctx->salt_len) ^= cpu_to_be64(seqno);
            memcpy(&pdata[esph_offset + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN],
                   rctx->msg_buf.iv_ctr + ctx->salt_offset + ctx->salt_len,
                   rctx->iv_ctr_len - ctx->salt_len);
        }
        else
            flow_log("non CBC / GCM context.\n");
    }

    rctx->hash_carry_len = 0;
    rctx->esn_spi = false;
    rctx->esn_hdr = false;

    if (xfrm->props.flags & XFRM_STATE_ESN) {
        rctx->esn_hdr = true;
        rctx->seq_hi = seqhi;
    }

    ret = handle_aead_req(rctx);
    if (ret == -EINPROGRESS)
        return 0;

    /* if (ret != -EINPROGRESS) */
    {
        if (rctx->mssg.src)
        {
            dma_unmap_sg(spu_dev, rctx->mssg.src, sg_nents(rctx->mssg.src), DMA_FROM_DEVICE);
            kfree(rctx->mssg.src);
        }
        if (rctx->mssg.dst != rctx->mssg.src)
        {
            dma_unmap_sg(spu_dev, rctx->mssg.dst, sg_nents(rctx->mssg.dst), DMA_FROM_DEVICE);
            kfree(rctx->mssg.dst);
        }
        flow_log("Packet discarded, failed to queue packet to interface.\n");
        kfree(rctx);
    }

exit:
    if (free_buf)
        nbuff_free(pNBuf);

    dst_release(args->esptx_dst_p);

    return ret;
} /* spu_blog_us_dev_xmit_args */

static int spu_blog_xmit_ds(pNBuff_t pNBuf, struct net_device *dev)
{
    flow_log("spu_blog_xmit_ds slow path\n");

    nbuff_free(pNBuf);
    return 0;
}

static int spu_blog_ds_dev_xmit_args (pNBuff_t pNBuf, struct net_device *dev, BlogFcArgs_t *args)
{
    struct sk_buff        *skb;
    unsigned char         *pdata;
    uint32_t               spi;
    struct iproc_ctx_s    *ctx;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    struct sec_path       *secpath = NULL;
#endif
    struct xfrm_state     *xfrm = NULL;
    struct net            *net;
    unsigned int           hlen;
    uint32_t               seqno;
    struct iproc_reqctx_s *rctx;
    unsigned int           nbuflen;
    void                  *ctx_buf;
    struct scatterlist    *sg;
    int                    ret = -1;
    int                    free_buf = 1;
    struct device *spu_dev = &iproc_priv.pdev->dev;
    int                    esph_offset;
    struct iphdr          *iph;

    flow_log("%s\n", __func__);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    if (args->esprx_secPath_p == NULL)
#else
    if (args->esprx_xfrm_st == NULL)
#endif
    {
        nbuff_free(pNBuf);
        return -1;
    }

    if (IS_SKBUFF_PTR(pNBuf))
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);
        /* blog is owned by fc so void reference */
        skb->blog_p = NULL;
        pdata = skb->data;
        nbuflen = skb->len;
        /* make sure dirty_p is NULL */
        skb_shinfo(skb)->dirty_p = NULL;
    }
    else
    {
        FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_FKBUFF(pNBuf);
        fkb = fkb_unshare(fkb);
        /* blog is owned by fc so void reference */
        fkb->blog_p = NULL;
        pdata = fkb->data;
        nbuflen = fkb->len;
        skb = NULL;
    }

    // TODO: handle ipv6
    iph = (struct iphdr *)pdata;
    if (iph->protocol == BLOG_IPPROTO_UDP)
        esph_offset = BLOG_IPV4_HDR_LEN + BLOG_UDP_HDR_LEN;
    else
        esph_offset = BLOG_IPV4_HDR_LEN;

    /* look up if the session is blogged */
    spi = _read32_align16((uint16_t *)&pdata[esph_offset]);
    ctx = spu_blog_lookup_ctx(spi, SPU_STREAM_DS);
    if (ctx == NULL)
    {
        flow_log("spu_blog_lookup_ctx failed: spi %x\n", htonl(spi));
        ret = -EINVAL;
        goto exit;
    }

    /* validate sequence number and xfrm state */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    secpath = args->esprx_secPath_p;
    if (secpath && (secpath->len == 1))
        xfrm = secpath->xvec[secpath->len-1];
#else
    xfrm = args->esprx_xfrm_st;
#endif
    if (NULL == xfrm)
    {
        flow_log("Packet discarded! blog's xfrm state for this flow doesn't exist.\n");
        ret = -1;
        goto exit;
    }

    net = xs_net(xfrm);

    /* this logic assumes that it is a ESP flow */
    hlen = esph_offset + BLOG_ESP_SPI_LEN;
    seqno = _read32_align16((uint16_t *)&pdata[hlen]);

#if defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    spu_update_xfrm_offload_stats((void *)xfrm);
#endif
    spin_lock_bh(&xfrm->lock);
    do
    {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
        ret = xfrm->repl->check(xfrm, NULL, seqno);
#else
        ret = xfrm_replay_check(xfrm, NULL, seqno);
#endif
        if (ret)
        {
            if ( NULL == skb )
            {
                /* nbuff is an FKB - translate to skb*/
                skb = nbuff_xlate(pNBuf);
            }
            skb_reset_network_header(skb);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
            xfrm->repl->check(xfrm, skb, seqno);
#else
            xfrm_replay_check(xfrm, skb, seqno);
#endif
            XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
            flow_log("Packet discarded! %s detected esp flow sequence error.\n", __func__);
            kfree_skb(skb);
            free_buf = 0;
            break;
        }

        ret = xfrm_state_check_expire(xfrm);
        if (ret)
        {
            XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEEXPIRED);
            flow_log("Packet discarded! %s detected expired esp flow.\n", __func__);
            break;
        }
    } while (0);
    spin_unlock_bh(&xfrm->lock);
    if ( ret )
    {
        ret = -1;
        goto exit;
    }

    ctx_buf = kmalloc(sizeof(struct iproc_reqctx_s) +
                      (sizeof(struct scatterlist)), GFP_ATOMIC);
    if (NULL == ctx_buf)
    {
        flow_log("Packet discarded! Failed to allocate memory for iproc request.\n");
        goto exit;
    }

    rctx = (struct iproc_reqctx_s *)ctx_buf;
    memset(&rctx->mssg, 0, sizeof(struct bcmspu_message));

    rctx->mssg.rx_callback    = spu_blog_fc_crypt_done_ds;
    rctx->ctx                 = ctx;
    rctx->iv_ctr_len          = ctx->iv_size + ctx->salt_len; //ctx->iv_ctr_len;
    rctx->pNBuf               = pNBuf;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    rctx->sp                  = secpath;
#else
    rctx->xfrm_st             = xfrm;
#endif

    rctx->gfp            = GFP_ATOMIC;
    rctx->is_encrypt     = false;
    rctx->bd_suppress    = false;
    rctx->is_sw_hmac     = false;
    rctx->assoc_len      = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + ctx->iv_size;
    rctx->total_todo     = nbuflen - esph_offset - rctx->assoc_len;
    rctx->total_sent     = 0;
    rctx->total_received = 0;
    rctx->hash_carry_len = 0;

    if (ctx->salt_len) {
        memcpy(rctx->msg_buf.iv_ctr + ctx->salt_offset,
               ctx->salt, ctx->salt_len);
    }
    memcpy(rctx->msg_buf.iv_ctr + ctx->salt_offset + ctx->salt_len, 
           &pdata[esph_offset + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN],
           ctx->iv_size);

    /* Initialize current position in src and dst scatterlists */
    sg = (struct scatterlist *)(ctx_buf + sizeof(struct iproc_reqctx_s));
    rctx->src_sg    = &sg[0];
    rctx->dst_sg    = &sg[0];
    rctx->assoc     = &sg[0];
    rctx->src_nents = 0;
    rctx->dst_nents = 0;
    rctx->src_skip  = rctx->assoc_len;
    rctx->dst_skip  = rctx->assoc_len;
    sg_init_one(rctx->src_sg,
                &pdata[esph_offset],
                nbuflen - esph_offset);

    rctx->esn_spi = false;
    rctx->esn_hdr = false;
    if (xfrm->props.flags & XFRM_STATE_ESN) {
        rctx->esn_hdr = true;
        rctx->seq_hi = htonl(xfrm_replay_seqhi(xfrm, seqno));
    }

    ret = handle_aead_req(rctx);

    if (ret == -EINPROGRESS)
        return 0;

    /* if (ret != -EINPROGRESS) */
    {
        flow_log("Packet discarded, failed to queue packet to interface.\n");
        if (rctx->mssg.src)
        {
            dma_unmap_sg(spu_dev, rctx->mssg.src, sg_nents(rctx->mssg.src), DMA_FROM_DEVICE);
            kfree(rctx->mssg.src);
        }
        if (rctx->mssg.dst != rctx->mssg.src)
        {
            dma_unmap_sg(spu_dev, rctx->mssg.dst, sg_nents(rctx->mssg.dst), DMA_FROM_DEVICE);
            kfree(rctx->mssg.dst);
        }
        kfree(rctx);
    }
    ret = 0;

exit:
    if (free_buf)
        nbuff_free(pNBuf);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    secpath_put(args->esprx_secPath_p);
#else
    xfrm_state_put(args->esprx_xfrm_st);
#endif

    return ret;
}  /* spu_blog_xmit_ds */

static struct net_device *spu_blog_create_device(char *name, uint32_t dir)
{
    int ret = 0;
    struct net_device *dev;

    flow_log("%s\n", __func__);

    dev = alloc_netdev(0, name, NET_NAME_UNKNOWN, ether_setup);
    if (dev != NULL)
    {
#if defined(CONFIG_BCM_KF_NETDEV_EXT)
#if defined(CONFIG_BCM_SPU_HW_OFFLOAD) && (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE))
	/* Initialize the bcm_netdevice callback function */
	bcm_netdev_ext_field_set(dev, bcm_netdev_cb_fn, spu_offload_priv_info_get);
#endif
        /* Mark device as BCM */
        netdev_bcm_dev_set(dev);
        netdev_dummy_dev_set(dev);
#endif
        dev_alloc_name(dev, dev->name);

        if (dir == 1)
        {
            dev->netdev_ops = &iproc_priv.spu_dev_ops_us;
            bcm_netdev_ext_field_set(dev, dev_xmit_args, spu_blog_us_dev_xmit_args);
        }
        else
        {
            dev->netdev_ops = &iproc_priv.spu_dev_ops_ds;
            bcm_netdev_ext_field_set(dev, dev_xmit_args, spu_blog_ds_dev_xmit_args);
        }

        dev->header_ops = &iproc_priv.spu_dev_header_ops;
        dev->type = ARPHRD_NONE;
        dev->tx_queue_len = 100;
        dev->flags = IFF_NOARP;

        /* Set mtu to a huge value so that fcache will not fragment packets
         * destined to SPU device.
         */
        dev->mtu = BCM_MAX_MTU_PAYLOAD_SIZE;

        ret = register_netdev(dev);
        if (ret)
        {
            flow_log("spu_blog_create_device: register_netdev failed\n");
            free_netdev(dev);
        }
    }
    else
    {
        flow_log("spu_blog_create_device: alloc_netdev failed\n");
        ret = -ENOMEM;
    }

    if ( ret < 0 )
    {
        return ERR_PTR(-ENOMEM);
    }

    /* bring device up */
    rtnl_lock();
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
    dev_open(dev);
#else
    dev_open(dev, NULL);
#endif
    rtnl_unlock();

    return dev;
}  /* spu_blog_create_device() */

int spu_blog_register(void)
{
    int i;
    flow_log("%s\n", __func__);

    for (i = 0; i < SPU_STREAM_MAX; i++)
    {
        iproc_priv.ctxListLock[i] = __RW_LOCK_UNLOCKED(iproc_priv.ctxListLock[i]);
        INIT_LIST_HEAD(&iproc_priv.ctxList[i]);
    }
#if defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    if (spu_offload_init())
    {
        flow_log("spu_blog_register: failed to start hw accelerator\n");
        return -1;
    }
#endif

    iproc_priv.spu_dev_ops_us.ndo_start_xmit = (HardStartXmitFuncP)spu_blog_xmit_us;
    iproc_priv.spu_dev_us = spu_blog_create_device("spu_us_dummy", 1);
    if ( IS_ERR(iproc_priv.spu_dev_us) )
    {
        flow_log("spu_blog_register: failed to create upstream device\n");
        return -1;
    }

    iproc_priv.spu_dev_ops_ds.ndo_start_xmit = (HardStartXmitFuncP)spu_blog_xmit_ds;
    iproc_priv.spu_dev_ds = spu_blog_create_device("spu_ds_dummy", 0);
    if ( IS_ERR(iproc_priv.spu_dev_ds) )
    {
        free_netdev(iproc_priv.spu_dev_us);
        flow_log("spu_blog_register: failed to create downstream device\n");
        return -1;
    }
#if defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    if ( spu_offload_postinit() )
    {
        flow_log("spu_blog_register: failed on hw accelerator postinit\n");
        return -1;
    }
    spu_blog_offload_register();
#endif

    return 0;
} /* spu_blog_register */

void spu_blog_unregister(void)
{
    flow_log("%s\n", __func__);

#if defined (CONFIG_BCM_SPU_HW_OFFLOAD)
    spu_blog_offload_deregister();
    spu_offload_deinit();
#endif
    if (iproc_priv.spu_dev_us)
    {
        unregister_netdev(iproc_priv.spu_dev_us);
        free_netdev(iproc_priv.spu_dev_us);
    }
    if (iproc_priv.spu_dev_ds)
    {
        unregister_netdev(iproc_priv.spu_dev_ds);
        free_netdev(iproc_priv.spu_dev_ds);
    }
} /* spu_blog_unregister */

