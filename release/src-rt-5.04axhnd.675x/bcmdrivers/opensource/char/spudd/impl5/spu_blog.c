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
#include <crypto/sha.h>
#include <crypto/md5.h>
#include <crypto/scatterwalk.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <linux/ip.h>
#include <linux/if_arp.h>
#include <net/xfrm.h>
#include <net/dst.h>
#include <bcmspudrv.h>
#include <pktHdr.h>
#include <linux/platform_device.h>
#include "spu.h"
#include "cipher.h"
#include "util.h"
#include "spu_blog.h"

void spu_blog_ctx_add(struct iproc_ctx_s *ctx)
{
    flow_log("%s: ctx %p\n", __func__, ctx);
    if (ctx->cipher.mode == CIPHER_MODE_CBC ||
        ctx->cipher.mode == CIPHER_MODE_NONE) 
    {
        write_lock(&iproc_priv.ctxListLock[ctx->stream]);
        list_add(&ctx->entry, &iproc_priv.ctxList[ctx->stream]);
        write_unlock(&iproc_priv.ctxListLock[ctx->stream]);
    }
    flow_log("%s - ctx added \n", __func__);
}

void spu_blog_ctx_del(struct iproc_ctx_s *ctx)
{
   struct iproc_ctx_s *sctx;
   struct iproc_ctx_s *pos;

   flow_log("%s: ctx %p\n", __func__, ctx);
   write_lock(&iproc_priv.ctxListLock[ctx->stream]);
   /* if a ctx does not get used it won't be in the list */
   list_for_each_entry_safe(sctx, pos, &iproc_priv.ctxList[ctx->stream], entry) 
   {
      if (sctx == ctx )
      {
         list_del(&ctx->entry);
         break;
      }
   }
   write_unlock(&iproc_priv.ctxListLock[ctx->stream]);
}

static inline struct iproc_ctx_s *spu_blog_lookup_ctx(uint32_t saddr, uint32_t daddr, uint32_t spi, enum spu_stream_type stream)
{
    struct iproc_ctx_s *ctx;
    struct iproc_ctx_s *rctx = NULL;

    flow_log("%s\n", __func__);

    read_lock(&iproc_priv.ctxListLock[stream]);
    list_for_each_entry(ctx, &iproc_priv.ctxList[stream], entry) 
    {
        if ( (ctx->ipdaddr == daddr) &&
             (ctx->ipsaddr == saddr) &&
             (ctx->spi     == spi) )
        {
            rctx = ctx;
            break;
        }
    }
    read_unlock(&iproc_priv.ctxListLock[stream]);
    return rctx;
}  /* spu_blog_lookup_ctx() */

void spu_blog_emit_aead(struct iproc_reqctx_s *rctx)
{
    struct sk_buff    *pSkb = PNBUFF_2_SKBUFF(rctx->pNBuf);
    struct net_device *skb_dev;
    unsigned char     *pdata;
    struct iphdr      *iph;
    struct ip_esp_hdr *esph;
    BlogFcArgs_t       fcArgs;

    flow_log("%s\n", __func__);

    if ( pSkb->protocol != htons(ETH_P_IP) || 
         !(rctx->ctx->cipher.mode == CIPHER_MODE_CBC ||
           rctx->ctx->cipher.mode == CIPHER_MODE_NONE) )
    {
        blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }

    if (rctx->is_encrypt) /* encryption */
    {
        /* ignore udp encapsulated packets and packets needing
           more than one transform */
        if ( !skb_dst(pSkb)->xfrm ||
             xfrm_dst_child(skb_dst(pSkb))->xfrm ||
             skb_dst(pSkb)->xfrm->encap )
        {
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        if ( skb_dst(pSkb)->xfrm->props.mode != XFRM_MODE_TUNNEL )
        {
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        /* pSkb->data should point to the outer IP header */
        pdata = pSkb->data;
        if ( *pdata != 0x45 )
        {
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        /* need to clone dst so that it can be used for packets that cannot be
           accelerated after encryption - dst should always be referenced */
        if ( (blog_ptr(pSkb) != NULL) )
        {
            blog_ptr(pSkb)->esptx.dst_p = dst_clone(skb_dst(pSkb));
            if ( 0 == (skb_dst(pSkb)->xfrm->props.flags & XFRM_STATE_NOPMTUDISC) )
            {
                blog_ptr(pSkb)->esptx.pmtudiscen = 1;
            }
            blog_ptr(pSkb)->tx.info.bmap.ESP = 1;
        }
        else
        {
            return;
        }

        iph = (struct iphdr *)pdata;
        iph->tot_len = htons(pSkb->len);
        iph->check   = 0;
        iph->check   = ip_fast_csum((unsigned char *)iph, iph->ihl);

        /* insert spu_dev_us as transmit device */
        skb_dev = pSkb->dev;
        pSkb->dev = iproc_priv.spu_dev_us;
        fcArgs.esp_ivsize = rctx->iv_ctr_len;
        fcArgs.esp_icvsize = rctx->ctx->digestsize;
        blog_emit_args(rctx->pNBuf, iproc_priv.spu_dev_us, TYPE_IP, 0, BLOG_SPU_US, &fcArgs);
        pSkb->dev = skb_dev;

        esph = (struct ip_esp_hdr *)(pdata + BLOG_IPV4_HDR_LEN);
        if ( _read32_align16((uint16_t *)&esph->spi) != rctx->ctx->spi )
        {
            rctx->ctx->ipdaddr = _read32_align16((uint16_t *)&iph->daddr);
            rctx->ctx->ipsaddr = _read32_align16((uint16_t *)&iph->saddr);
            rctx->ctx->spi     = _read32_align16((uint16_t *)&esph->spi);
            rctx->ctx->stream = SPU_STREAM_US;
            spu_blog_ctx_add(rctx->ctx);
            flow_log("upstream ips %x, ipd %x, spi %x, seq %d\n",ntohl(iph->saddr), ntohl(iph->daddr),ntohl(esph->spi),ntohl(esph->seq_no));
        }
    }
    else
    {
        struct xfrm_state *xfrm;

        if (pSkb->dev && !is_netdev_wan(pSkb->dev)) { /* ignore LAN side IPSec connections */
            /* This use case is for IPsec tunnel started by the LAN device and terminated on the GW */
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        /* ignore udp encapsualted packets and packets requiring
           more than one transform */
        if ( !secpath_exists(pSkb) ||
            (pSkb->sp->len != 1) )
        {
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        xfrm = pSkb->sp->xvec[pSkb->sp->len-1];
        if ( (NULL == xfrm) ||
            (xfrm->encap) )
        {
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        if ( xfrm->props.mode != XFRM_MODE_TUNNEL)
        {
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        /* skb->data will point to the ESP header, need to push data back
           to the outer IP header */
        pdata = skb_push(pSkb, BLOG_IPV4_HDR_LEN);
        if ( *pdata != 0x45 )
        {
            skb_pull(pSkb, BLOG_IPV4_HDR_LEN);
            blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
            return;
        }

        /* save the secpath so that it can be used to pass packets to the kernel
           when they cannot be accelerated after decryption */
        if ( blog_ptr(pSkb) == NULL )
        {
            skb_pull(pSkb, BLOG_IPV4_HDR_LEN);
            return;
        }

        blog_ptr(pSkb)->esprx.secPath_p = secpath_get(pSkb->sp);
        blog_ptr(pSkb)->rx.info.bmap.ESP = 1;

        /* insert spu_dev_ds as transmit device */
        skb_dev = pSkb->dev;
        pSkb->dev = iproc_priv.spu_dev_ds;
        fcArgs.esp_ivsize = rctx->iv_ctr_len;
        fcArgs.esp_icvsize = rctx->ctx->digestsize;

        iph = (struct iphdr *)pdata;
        esph = (struct ip_esp_hdr *)(pdata + BLOG_IPV4_HDR_LEN);
        if ( _read32_align16((uint16_t *)&esph->spi) != rctx->ctx->spi )
        {
            rctx->ctx->ipdaddr = _read32_align16((uint16_t *)&iph->daddr);
            rctx->ctx->ipsaddr = _read32_align16((uint16_t *)&iph->saddr);
            rctx->ctx->spi     = _read32_align16((uint16_t *)&esph->spi);
            rctx->ctx->stream = SPU_STREAM_DS;
            spu_blog_ctx_add(rctx->ctx);
            flow_log("dowstream ips %x, ipd %x, spi %x, seq %x\n", ntohl(iph->saddr), ntohl(iph->daddr), ntohl(esph->spi), ntohl(esph->seq_no));
        }
        blog_emit_args(rctx->pNBuf, iproc_priv.spu_dev_ds, TYPE_IP, 0, BLOG_SPU_DS, &fcArgs);
        skb_pull(pSkb, BLOG_IPV4_HDR_LEN);
        pSkb->dev = skb_dev;
    }
} /* spu_blog_emit_aead */

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
    BlogFcArgs_t           fcArgs;

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
        secpath_put(rctx->sp);
        kfree(rctx);
        flow_log("SPU process error\n");
        return;
    }

    /* Process the SPU response message */
    handle_aead_resp(rctx);

    /* need to save some things here as nbuf may be consumed */
    fcArgs.esp_ivsize = rctx->iv_ctr_len;
    fcArgs.esp_icvsize = rctx->ctx->digestsize;
    if ( IS_SKBUFF_PTR(rctx->pNBuf) )
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(rctx->pNBuf);
        pData = skb->data;
        pktlen = skb->len;
        headroom = 0;
        skb->fc_ctxt = 0;
        blogAction = blog_sinit_args(skb, iproc_priv.spu_dev_us,
                                     TYPE_IP, 0, BLOG_SPU_US, &fcArgs);
    }
    else
    {
        FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_FKBUFF(rctx->pNBuf);
        headroom = (int)(fkb->data - PFKBUFF_TO_PHEAD(fkb));
        pData = fkb->data;
        pktlen = fkb->len;
        skb = NULL;
        fkb->fc_ctxt = 0;
        blogAction = blog_finit_args(fkb, iproc_priv.spu_dev_us,
                                     TYPE_IP, 0, BLOG_SPU_US, &fcArgs);
    }

    if (blogAction == PKT_DROP)
    {
        flow_log("%s:%d - blogAction == PKT_DROP\n", __func__, __LINE__);
        if (rctx->pNBuf)
            nbuff_free(rctx->pNBuf);
        dst_release(rctx->dst);
        secpath_put(rctx->sp);
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
        flow_log("%s:%d - blogged\n", __func__, __LINE__);
    }
    dst_release(rctx->dst);
    secpath_put(rctx->sp);
    kfree(rctx);

} /* spu_blog_fc_crypt_done_us */

static void spu_blog_fc_crypt_done_ds(struct bcmspu_message *mssg)
{
    struct iproc_reqctx_s *rctx = container_of(mssg, struct iproc_reqctx_s, mssg);
    struct device *dev = &iproc_priv.pdev->dev;
    struct spu_hw *spu = &iproc_priv.spu;
    int err = 0;
    struct sk_buff    *skb;
    struct xfrm_state *xfrm;
    BlogAction_t       blogAction;
    unsigned int       hlen;
    unsigned int       pktlen;
    unsigned char      padlen;
    unsigned char      nexthdr;
    unsigned int       seqno;
    int                headroom;
    unsigned char     *pData;
    BlogFcArgs_t       fcArgs;

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
        nbuff_free(rctx->pNBuf);
        secpath_put(rctx->sp);
        flow_log("SPU process error\n");
        kfree(rctx);
        return;
    }

    /* Process the SPU response message */
    handle_aead_resp(rctx);

    xfrm = rctx->sp->xvec[rctx->sp->len-1];
    if ( NULL == xfrm )
    {
        flow_log("spu_blog_fc_crypt_done_ds: xfrm is NULL\n");
        nbuff_free(rctx->pNBuf);
        secpath_put(rctx->sp);
        kfree(rctx);
        return;
    }

    /* need to save some things here as nbuf may be consumed */
    fcArgs.esp_ivsize = rctx->iv_ctr_len;
    fcArgs.esp_icvsize = rctx->ctx->digestsize;
    if ( IS_SKBUFF_PTR(rctx->pNBuf) )
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(rctx->pNBuf);
        pktlen = skb->len;
        pData = skb->data;
        hlen = BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN;
        seqno = _read32_align16((uint16_t *)&pData[hlen]);
        padlen  = pData[pktlen - rctx->ctx->digestsize - 2];
        nexthdr = pData[pktlen - rctx->ctx->digestsize - 1];
        headroom = 0;
        skb->fc_ctxt = 0;
        blogAction = blog_sinit_args(skb, iproc_priv.spu_dev_ds,
                                     TYPE_IP, 0, BLOG_SPU_DS, &fcArgs);
    }
    else
    {
        FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(rctx->pNBuf);
        pktlen = fkb->len;
        pData = fkb->data;
        hlen = BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN;
        seqno = _read32_align16((uint16_t *)&pData[hlen]);
        padlen  = pData[pktlen - rctx->ctx->digestsize - 2];
        nexthdr = pData[pktlen - rctx->ctx->digestsize - 1];
        headroom = (int)(pData - PFKBUFF_TO_PHEAD(fkb));
        skb = NULL;
        fkb->fc_ctxt = 0;
        blogAction = blog_finit_args(fkb, iproc_priv.spu_dev_ds,
                                     TYPE_IP, 0, BLOG_SPU_DS, &fcArgs);
    }

    if (blogAction == PKT_DROP)
    {
        flow_log("%s:%d - blogAction == PKT_DROP\n", __func__, __LINE__);
        if (rctx->pNBuf)
            nbuff_free(rctx->pNBuf);
        secpath_put(rctx->sp);
        kfree(rctx);
        return;
    }

    if ((blogAction == PKT_NORM) || (blogAction == PKT_BLOG))
    {
        int seq_hi;
        flow_log("%s:%d - not blogged\n", __func__, __LINE__);

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
        hlen += BLOG_ESP_SEQNUM_LEN + rctx->iv_ctr_len;
        XFRM_SKB_CB(skb)->seq.input.low = seqno;
        XFRM_SKB_CB(skb)->seq.input.hi = seq_hi;

        /* trim padding and icv */
        __skb_trim(skb, skb->len - rctx->ctx->digestsize - padlen - 2);

        /* pull to inner IP header */
        __skb_pull(skb, hlen);
        skb_reset_network_header(skb);
        skb_reset_mac_header(skb);
        skb_set_transport_header(skb, 0);

        skb->sp = secpath_get(rctx->sp);
        dev_hold(skb->dev);
        xfrm_input_resume(skb, nexthdr);
    }
    else
    {
        flow_log("%s:%d - blogged\n", __func__, __LINE__);
        atomic_inc(&iproc_priv.blogged[SPU_STREAM_DS]);
        hlen += BLOG_ESP_SEQNUM_LEN+rctx->iv_ctr_len;
        pktlen = pktlen - hlen - rctx->ctx->digestsize - padlen - 2;
        spin_lock(&xfrm->lock);
        xfrm->repl->advance(xfrm, seqno);
        xfrm->curlft.bytes += pktlen;
        xfrm->curlft.packets++;
        spin_unlock(&xfrm->lock); 
    }
    /* free the request context */
    secpath_put(rctx->sp);
    kfree(rctx);
}  /* spu_blog_fc_crypt_done_ds */

static int spu_blog_xmit_us(pNBuff_t pNBuf, struct net_device *dev)
{
    struct iproc_reqctx_s *rctx;
    struct iproc_ctx_s    *ctx;
    struct blog_t         *blog_p;
    struct iphdr          *iph;
    uint32_t               ipsa;
    uint32_t               ipda;
    uint32_t               spi;
    int                    ret;
    struct xfrm_state     *xfrm;
    struct net            *net;
    int                    seqno;
    unsigned char         *pdata;
    struct sk_buff        *skb;
    int                    nbuflen;
    void                  *ctx_buf;
    struct dst_entry      *dst;
    struct scatterlist    *sg;
    int                    datalen;
    int                    padlen;
    struct device *spu_dev = &iproc_priv.pdev->dev;

    flow_log("%s\n", __func__);

    if (IS_SKBUFF_PTR(pNBuf))
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);
        blog_p = skb->blog_p;
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
        blog_p = fkb->blog_p;
        /* blog is owned by fc so void reference */
        fkb->blog_p = NULL;
        pdata = fkb->data;
        nbuflen = fkb->len;
        skb = NULL;
    }

    iph = (struct iphdr *)(pdata);
    ipsa = _read32_align16((uint16_t *)&iph->saddr);
    ipda = _read32_align16((uint16_t *)&iph->daddr);
    spi = _read32_align16((uint16_t *)&pdata[BLOG_IPV4_HDR_LEN]);

    ctx = spu_blog_lookup_ctx(ipsa, ipda, spi, SPU_STREAM_US);
    if ( NULL == ctx )
    {
        flow_log("spu_blog_lookup_ctx failed: ipsa %x, ipda %x, spi %x\n",
            htonl(ipsa), htonl(ipda), htonl(spi));
        nbuff_free(pNBuf);
        return 0;
    }

    dst = blog_p->esptx.dst_p;
    xfrm = dst->xfrm;
    if ( NULL == xfrm )
    {
        flow_log("Packet discarded! blog's xfrm state for this flow doesn't exist.\n");
        nbuff_free(pNBuf);
        return 0;
    }
    net = xs_net(xfrm);

    spin_lock(&xfrm->lock);
    do
    {
        /* A new key has genid = 0. When the key is renewed, genid is incremented.
        * We want to evict the flow from fcache when the key has been renewed
        * or expired.
        */
        if (xfrm->genid)
        {
            /* Set blog tuple_offset to an invalid value so that
             * flow cache will evict the flow on the next packet.
             */
            blog_p->tuple_offset = 0xff;
        }

        ret = xfrm_state_check_expire(xfrm);
        if (ret)
        {
            XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEEXPIRED);
            nbuff_free(pNBuf);
            break;
        }

        /* pass NULL skb since we have an fkb
           if there is an error then we will create an skb and call
           again so that the error is reported */
        ret = xfrm->repl->overflow(xfrm, NULL);
        if (ret)
        {
            if ( NULL == skb )
            {
            /* nbuf is an fkb - translate to skb
            call replay function again for auditing */
                skb = nbuff_xlate(pNBuf);
            }
            skb_reset_network_header(skb);
            xfrm->repl->overflow(xfrm, skb);
            XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
            kfree_skb(skb);
            break;
        }
        if ( xfrm->replay_esn )
        {
            seqno = xfrm->replay_esn->oseq;
        }
        else
        {
            seqno = xfrm->replay.oseq;
        }

        /* only count inner ip header and data */
        padlen  = pdata[nbuflen - ctx->digestsize - 2];
        datalen = nbuflen - BLOG_IPV4_HDR_LEN - padlen - blog_p->esptx.ivsize -
        BLOG_ESP_SPI_LEN - BLOG_ESP_SEQNUM_LEN - BLOG_ESP_PADLEN_LEN -
        BLOG_ESP_NEXT_PROTO_LEN - ctx->digestsize;
        xfrm->curlft.bytes = datalen;
        xfrm->curlft.packets++;
    } while (0);
    spin_unlock(&xfrm->lock);
    if ( ret != 0 )
    {
        blog_p->tuple_offset = 0xff;
        return ret;
    }

    _write32_align16((uint16_t *)&pdata[BLOG_IPV4_HDR_LEN+BLOG_ESP_SPI_LEN], htonl(seqno));

    /* allocate a transfer request - data is zeroed */
    ctx_buf = kmalloc(sizeof(struct iproc_reqctx_s) + 
                      sizeof(struct scatterlist), GFP_ATOMIC);
    if (NULL == ctx_buf)
    {
        flow_log("Packet discarded! Failed to allocate memory for iproc request.\n");
        nbuff_free(pNBuf);
        return -1;
    }
    rctx = (struct iproc_reqctx_s *)ctx_buf;

    rctx->pNBuf          = pNBuf;
    rctx->iv_ctr_len     = blog_p->esptx.ivsize;
    rctx->dst            = dst_clone(blog_p->esptx.dst_p);
    rctx->sp             = NULL;

    rctx->gfp = GFP_ATOMIC;
    rctx->is_encrypt = true;
    rctx->bd_suppress = false;
    rctx->total_todo = (datalen + padlen + blog_p->esptx.ivsize +
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
    rctx->assoc_len = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN;
    rctx->src_skip  = rctx->assoc_len;
    rctx->dst_skip  = rctx->assoc_len;

    sg_init_one(rctx->src_sg, 
                &pdata[BLOG_IPV4_HDR_LEN], (rctx->total_todo + rctx->assoc_len + ctx->digestsize));

    if (rctx->iv_ctr_len)
    {
        memcpy(rctx->msg_buf.iv_ctr, ctx->iv, rctx->iv_ctr_len);
        *(__be64 *) rctx->msg_buf.iv_ctr ^= cpu_to_be64(seqno);
        memset(&pdata[BLOG_IPV4_HDR_LEN + rctx->assoc_len], 0, rctx->iv_ctr_len);
    }
    rctx->hash_carry_len = 0;

    ret = handle_aead_req(rctx);
    if (ret != -EINPROGRESS)
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
        nbuff_free(pNBuf);
        dst_release(rctx->dst);
        kfree(rctx);
    }

    return 0;
} /* spu_blog_xmit_us */

static int spu_blog_xmit_ds(pNBuff_t pNBuf, struct net_device *dev)
{
    struct sk_buff        *skb;
    struct blog_t         *blog_p;
    unsigned char         *pdata;
    struct iphdr          *iph;
    uint32_t               ipsa;
    uint32_t               ipda;
    uint32_t               spi;
    struct iproc_ctx_s    *ctx;
    struct sec_path       *secpath;
    struct xfrm_state     *xfrm = NULL;
    struct net            *net;
    unsigned int           hlen;
    unsigned int           seqno;
    struct iproc_reqctx_s *rctx;
    unsigned int           nbuflen;
    void                  *ctx_buf;
    struct scatterlist    *sg;
    int                    ret;
    struct device *spu_dev = &iproc_priv.pdev->dev;

    flow_log("%s - start\n", __func__);

    if (IS_SKBUFF_PTR(pNBuf))
    {
        skb = (struct sk_buff *)PNBUFF_2_PBUF(pNBuf);
        blog_p = skb->blog_p;
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
        blog_p = fkb->blog_p;
        /* blog is owned by fc so void reference */
        fkb->blog_p = NULL;
        pdata = fkb->data;
        nbuflen = fkb->len;
        skb = NULL;
    }

    /* look up if the session is blogged */
    iph = (struct iphdr *)(pdata);
    ipsa = _read32_align16((uint16_t *)&iph->saddr);
    ipda = _read32_align16((uint16_t *)&iph->daddr);
    spi = _read32_align16((uint16_t *)&pdata[BLOG_IPV4_HDR_LEN]);

    ctx = spu_blog_lookup_ctx(ipsa, ipda, spi, SPU_STREAM_DS);
    if (ctx == NULL)
    {
        flow_log("spu_blog_lookup_ctx failed: ipsa %x, ipda %x, spi %x\n",
                 htonl(ipsa), htonl(ipda), htonl(spi));
        nbuff_free(pNBuf);
        return -EINVAL;
    }

    /* validate sequence number and xfrm state */
    secpath = secpath_get(blog_p->esprx.secPath_p);
    if (secpath && (secpath->len == 1))
        xfrm = secpath->xvec[secpath->len-1];
    if (NULL == xfrm)
    {
        secpath_put(secpath);
        flow_log("Packet discarded! blog's xfrm state for this flow doesn't exist.\n");
        nbuff_free(pNBuf);
        return -1;
    }
    net = xs_net(xfrm);

    /* this logic assumes that it is a ESP flow */
    hlen = BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN;
    seqno = _read32_align16((uint16_t *)&pdata[hlen]);

    spin_lock(&xfrm->lock);
    do
    {
        ret = xfrm->repl->check(xfrm, NULL, seqno);
        if (ret)
        {
            if ( NULL == skb )
            {
                /* nbuff is an FKB - translate to skb*/
                skb = nbuff_xlate(pNBuf);
            }
            skb_reset_network_header(skb);
            xfrm->repl->check(xfrm, skb, seqno);
            XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
            flow_log("Packet discarded! %s detected esp flow sequence error.\n", __func__);
            kfree_skb(skb);
            break;
        }

        ret = xfrm_state_check_expire(xfrm);
        if (ret)
        {
            XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEEXPIRED);
            nbuff_free(pNBuf);
            flow_log("Packet discarded! %s detected expired esp flow.\n", __func__);
            break;
        }
    } while (0);
    spin_unlock(&xfrm->lock);
    if ( ret )
    {
        secpath_put(secpath);
        return -1;
    }

    ctx_buf = kmalloc(sizeof(struct iproc_reqctx_s) + 
                      (sizeof(struct scatterlist)), GFP_ATOMIC);
    if (NULL == ctx_buf)
    {
        flow_log("Packet discarded! Failed to allocate memory for iproc request.\n");
        secpath_put(secpath);
        nbuff_free(pNBuf);
        return -1;        
    }

    rctx = (struct iproc_reqctx_s *)ctx_buf;
    memset(&rctx->mssg, 0, sizeof(struct bcmspu_message));

    rctx->mssg.rx_callback    = spu_blog_fc_crypt_done_ds;
    rctx->ctx                 = ctx;
    rctx->iv_ctr_len          = blog_p->esptx.ivsize; //ctx->iv_ctr_len;
    rctx->pNBuf               = pNBuf;
    rctx->sp                  = secpath;

    rctx->gfp            = GFP_ATOMIC;
    rctx->is_encrypt     = false;
    rctx->bd_suppress    = false;
    rctx->is_sw_hmac     = false;
    rctx->assoc_len      = BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN + rctx->iv_ctr_len;
    rctx->total_todo     = nbuflen - BLOG_IPV4_HDR_LEN - rctx->assoc_len;
    rctx->total_sent     = 0;
    rctx->total_received = 0;
    rctx->hash_carry_len = 0;
    memcpy(rctx->msg_buf.iv_ctr, &pdata[BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN + BLOG_ESP_SEQNUM_LEN],
           rctx->iv_ctr_len);

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
                &pdata[BLOG_IPV4_HDR_LEN],
                nbuflen - BLOG_IPV4_HDR_LEN);

    ret = handle_aead_req(rctx);
    if (ret != -EINPROGRESS)
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
        secpath_put(secpath);
        nbuff_free(pNBuf);
        kfree(rctx);
    }

    return 0;
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
        /* Mark device as BCM */
        netdev_bcm_dev_set(dev);
#endif
        dev_alloc_name(dev, dev->name);

        if (dir == 1)
        {
            dev->netdev_ops = &iproc_priv.spu_dev_ops_us;
        }
        else
        {
            dev->netdev_ops = &iproc_priv.spu_dev_ops_ds;
            netdev_wan_set(dev);
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
    dev_open(dev);
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

    return 0;
} /* spu_blog_register */

void spu_blog_unregister(void)
{
    flow_log("%s\n", __func__);

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

