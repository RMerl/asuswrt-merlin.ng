/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom 
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
#include "spu.h"
#include "spu_runner.h"

void spu_blog_ctx_add(struct spu_ctx *ctx)
{
    spin_lock_bh(&spuinfo->spuListLock);
    list_add_tail(&ctx->entry, &spuinfo->ctxList);
    spin_unlock_bh(&spuinfo->spuListLock);
}

void spu_blog_ctx_del(struct spu_ctx *ctx)
{
    spin_lock_bh(&spuinfo->spuListLock);
    list_del(&ctx->entry);
    spin_unlock_bh(&spuinfo->spuListLock);
}

static struct spu_ctx *spu_blog_lookup_ctx(uint32_t saddr, uint32_t daddr, uint32_t spi)
{
   struct spu_ctx *ctx;
   struct spu_ctx *rctx = NULL;

   spin_lock_bh(&spuinfo->spuListLock);
   list_for_each_entry(ctx, &spuinfo->ctxList, entry) 
   {
      if ( (ctx->ipdaddr == daddr) &&
           (ctx->ipsaddr == saddr) &&
           (ctx->spi     == spi) )
      {
         rctx = ctx;
         break;
      }
   }
   spin_unlock_bh(&spuinfo->spuListLock);
   return rctx;
}  /* spu_blog_lookup_ctx() */

void spu_blog_emit(struct spu_trans_req *pTransReq)
{
    struct sk_buff    *pSkb = PNBUFF_2_SKBUFF(pTransReq->pNBuf);
    struct net_device *skb_dev;
    unsigned char     *pdata;
    struct iphdr      *iph;
    struct ip_esp_hdr *esph;
    
    if ( (pSkb->protocol != htons(ETH_P_IP)) || (pTransReq->pAllocSkb != NULL) )
    {
		blog_skip(pSkb, blog_skip_reason_spudd_check_failure);
        return;
    }
   
    if (SPU_DIRECTION_US == pTransReq->pSpuCtx->direction)
    {
        /* ignore udp encapsulated packets and packets needing
           more than one transform */
        if ( !skb_dst(pSkb)->xfrm ||
              skb_dst(pSkb)->child->xfrm ||
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
        pSkb->dev = spuinfo->spu_dev_us;
        blog_emit(pTransReq->pNBuf, spuinfo->spu_dev_us, TYPE_IP, pTransReq->ivsize, BLOG_SPU_US);
        pSkb->dev = skb_dev;

        if ( 0 == pTransReq->pSpuCtx->ipdaddr )
        {
            int seqno;
            esph = (struct ip_esp_hdr *)(pdata + BLOG_IPV4_HDR_LEN);
            pTransReq->pSpuCtx->ipdaddr = _read32_align16((uint16_t *)&iph->daddr);
            pTransReq->pSpuCtx->ipsaddr = _read32_align16((uint16_t *)&iph->saddr);
            pTransReq->pSpuCtx->spi     = _read32_align16((uint16_t *)&esph->spi);
            seqno = _read32_align16((uint16_t *)&esph->seq_no);
            SPU_TRACE(("upstream ips %x, ipd %x, spi %x, seq %d\n", ntohl(iph->saddr), ntohl(iph->daddr), ntohl(esph->spi), ntohl(esph->seq_no)));
        }
    }
    else /* SPU_DIRECTION_DS */
    {
        struct xfrm_state *xfrm;

        /* ignore udp encapsualted packets and packets requiring
           more than one transform */
        if ( !secpath_exists(pSkb) ||
             (pSkb->sp->len > 1) )
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

        blog_ptr(pSkb)->esptx.secPath_p = secpath_get(pSkb->sp);
        blog_ptr(pSkb)->tx.info.bmap.ESP = 1;

        /* insert spu_dev_ds as transmit device */
        skb_dev = pSkb->dev;
        pSkb->dev = spuinfo->spu_dev_ds;
        
        if ( 0 == pTransReq->pSpuCtx->ipdaddr )
        {
            iph = (struct iphdr *)pdata;
            esph = (struct ip_esp_hdr *)(pdata + BLOG_IPV4_HDR_LEN);
            pTransReq->pSpuCtx->ipdaddr = _read32_align16((uint16_t *)&iph->daddr);
            pTransReq->pSpuCtx->ipsaddr = _read32_align16((uint16_t *)&iph->saddr);
            pTransReq->pSpuCtx->spi     = _read32_align16((uint16_t *)&esph->spi);
            SPU_TRACE(("dowstream ips %x, ipd %x, spi %x, seq %x\n",
                       ntohl(iph->saddr), ntohl(iph->daddr), ntohl(esph->spi), ntohl(esph->seq_no)));

        }
        blog_emit(pTransReq->pNBuf, spuinfo->spu_dev_ds, TYPE_IP, pTransReq->ivsize, BLOG_SPU_DS);
        skb_pull(pSkb, BLOG_IPV4_HDR_LEN);
        pSkb->dev = skb_dev;
    }
}

static void spu_blog_fc_crypt_done_us(struct spu_trans_req *pTransReq)
{
   if ( pTransReq->err != 0 )
   {
      /* there was an error
         no auditing in xfrm_output_resume so just
         increment xfrm stat */
      struct xfrm_state *xfrm = pTransReq->dst->xfrm;
      if ( xfrm )
      {
         XFRM_INC_STATS(xs_net(xfrm), LINUX_MIB_XFRMOUTSTATEPROTOERROR);
      }
      nbuff_free(pTransReq->pNBuf);
   }
   else
   {
      struct sk_buff *skb;
      BlogAction_t    blogAction;
      int             headroom;
      uint8_t        *pData;
      uint32_t        len;;

      if ( IS_SKBUFF_PTR(pTransReq->pNBuf) )
      {
         skb = (struct sk_buff *)PNBUFF_2_PBUF(pTransReq->pNBuf);
         pData = skb->data;
         len = skb->len;
         headroom = 0;
         skb->fc_ctxt = 0;
         blogAction = blog_sinit(skb, spuinfo->spu_dev_us,
                                 TYPE_IP, pTransReq->ivsize, BLOG_SPU_US);
      }
      else
      {
         FkBuff_t *fkb = PNBUFF_2_FKBUFF(pTransReq->pNBuf);
         headroom = (int)(fkb->data - PFKBUFF_TO_PHEAD(fkb));
         pData = fkb->data;
         len = fkb->len;
         skb = NULL;
         fkb->fc_ctxt = 0;
         blogAction = blog_finit(fkb, spuinfo->spu_dev_us,
                                 TYPE_IP, pTransReq->ivsize, BLOG_SPU_US);
      }

      if (blogAction == PKT_DROP)
      {
         SPU_TRACE(("spu_blog_fc_crypt_done_us: blog_finit action drop\n"));
         spuinfo->stats.encDrops++;
         cache_invalidate_len(pData, len);
         nbuff_free(pTransReq->pNBuf);
         return;
      }

      /* safe to count as egress now */
      spuinfo->stats.encSpuEgress++;

      if ((blogAction == PKT_NORM) || (blogAction == PKT_BLOG))
      {
          /* if nbuf was an fkb then translate to skb */
          if ( NULL == skb )
          {
             /* for PKT_NORM, fkb was released so initialize it again
                in order for xlate to work */
             if ( blogAction == PKT_NORM )
             {
                fkb_init(pData, headroom, pData, len);
             }
             skb = nbuff_xlate(pTransReq->pNBuf);
          }

          /* dst already ref counted */
          skb_dst_set(skb, dst_clone(pTransReq->dst));
          skb_reset_network_header(skb);
          skb->dev = spuinfo->spu_dev_us;
          xfrm_output_resume(skb, 0);
      }
      /* else PKT_DONE - packet was consumed */
   }
} /* spu_blog_fc_crype_done_us */

static void spu_blog_fc_crypt_done_ds(struct spu_trans_req *pTransReq)
{
   struct sk_buff *skb;
   if ( pTransReq->err != 0 )
   {
      /* there was an error
         in case of icv failure pass packet to kernel
         for auditing */
      if ( IS_SKBUFF_PTR(pTransReq->pNBuf) )
      {
         skb = (struct sk_buff *)PNBUFF_2_PBUF(pTransReq->pNBuf);
      }
      else
      {
         skb = nbuff_xlate(pTransReq->pNBuf);
         skb->dev = pTransReq->dev;
      }

      skb->sp = secpath_get(pTransReq->sp);
      skb_reset_network_header(skb);
      /* pull data to the ESP header */
      skb_pull(skb, BLOG_IPV4_HDR_LEN);
      dev_hold(skb->dev);
      xfrm_input_resume(skb, pTransReq->err);
   }
   else
   {
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

      xfrm = pTransReq->sp->xvec[pTransReq->sp->len-1];
      if ( NULL == xfrm )
      {
         spuinfo->stats.decErrors++;
         nbuff_free(pTransReq->pNBuf);
         return;
      }

      /* need to save some things here as nbuf may be consumed */
      if ( IS_SKBUFF_PTR(pTransReq->pNBuf) )
      {
         skb = (struct sk_buff *)PNBUFF_2_PBUF(pTransReq->pNBuf);
         pktlen = skb->len;
         pData = skb->data;
         hlen = BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN;
         seqno = _read32_align16((uint16_t *)&pData[hlen]);
         padlen  = pData[pktlen - SPU_MAX_ICV_LENGTH - 2];
         nexthdr = pData[pktlen - SPU_MAX_ICV_LENGTH - 1];
         headroom = 0;
         skb->fc_ctxt = 0;
         blogAction = blog_sinit(skb, spuinfo->spu_dev_ds,
                                 TYPE_IP, pTransReq->ivsize, BLOG_SPU_DS);
      }
      else
      {
         FkBuff_t *fkb = (FkBuff_t *)PNBUFF_2_PBUF(pTransReq->pNBuf);
         pktlen = fkb->len;
         pData = fkb->data;
         hlen = BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN;
         seqno = _read32_align16((uint16_t *)&pData[hlen]);
         padlen  = pData[pktlen - SPU_MAX_ICV_LENGTH - 2];
         nexthdr = pData[pktlen - SPU_MAX_ICV_LENGTH - 1];
         headroom = (int)(pData - PFKBUFF_TO_PHEAD(fkb));
         skb = NULL;
         fkb->fc_ctxt = 0;
         blogAction = blog_finit(fkb, spuinfo->spu_dev_ds,
                                 TYPE_IP, pTransReq->ivsize, BLOG_SPU_DS);
      }

      if (blogAction == PKT_DROP)
      {
         SPU_TRACE(("spu_blog_fc_crypt_done_ds: blog_finit action drop\n"));
         spuinfo->stats.decDrops++;
         cache_invalidate_len(pData, pktlen);
         nbuff_free(pTransReq->pNBuf);
         return;
      }

      /* safe to count as egress now */
      spuinfo->stats.decSpuEgress++;

      if ((blogAction == PKT_NORM) || (blogAction == PKT_BLOG))
      {
         int             seq_hi;

         /* nbuf is an fkb - translate to skb */
         if ( skb == NULL )
         {
            /* for PKT_NORM, fkb was released so initialize it again
               in order for xlate to work */
            if ( blogAction == PKT_NORM )
            {
                fkb_init(pData, headroom, pData, pktlen);
            }
            skb = nbuff_xlate(pTransReq->pNBuf);
         }

         skb->dev = spuinfo->spu_dev_ds;
         seq_hi = htonl(xfrm_replay_seqhi(xfrm, seqno));
         hlen += BLOG_ESP_SEQNUM_LEN + pTransReq->ivsize;
         XFRM_SKB_CB(skb)->seq.input.low = seqno;
         XFRM_SKB_CB(skb)->seq.input.hi = seq_hi;

         /* trim padding and icv */
         __skb_trim(skb, skb->len - SPU_MAX_ICV_LENGTH - padlen - 2);

         /* pull to inner IP header */
         __skb_pull(skb, hlen);
         skb_reset_network_header(skb);
         skb_reset_mac_header(skb);
         skb_set_transport_header(skb, 0);

         skb->sp = secpath_get(pTransReq->sp);
         dev_hold(skb->dev);
         xfrm_input_resume(skb, nexthdr);
      }
      else
      {
         hlen += BLOG_ESP_SEQNUM_LEN+pTransReq->ivsize;
         pktlen = pktlen - hlen - SPU_MAX_ICV_LENGTH - padlen - 2;
         spin_lock_bh(&xfrm->lock);
         xfrm->repl->advance(xfrm, seqno);
         xfrm->curlft.bytes += pktlen;
         xfrm->curlft.packets++;
         spin_unlock_bh(&xfrm->lock); 
      }
   }
}  /* spu_blog_fc_crypt_done_ds */

static int spu_blog_xmit_us(pNBuff_t pNBuf, struct net_device *dev)
{
   struct spu_trans_req *pTransReq;
   struct spu_ctx       *pCtx;
   struct blog_t        *blog_p;
   struct iphdr         *iph;
   uint32_t              ipsa;
   uint32_t              ipda;
   uint32_t              spi;
   int                   ret;
   struct xfrm_state    *xfrm;
   struct net           *net;
   int                   seqno;
   unsigned char        *pdata;
   struct sk_buff       *skb;
   int                   nbuflen;

   spuinfo->stats.encIngress++;
   if ( IS_SKBUFF_PTR(pNBuf) )
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

   pCtx = spu_blog_lookup_ctx(ipsa, ipda, spi);
   if ( NULL == pCtx )
   {
      SPU_TRACE(("spu_blog_lookup_ctx failed: ipsa %x, ipda %x, spi %x\n", htonl(ipsa), htonl(ipda), htonl(spi)));
      spuinfo->stats.encErrors++;
      cache_invalidate_len(pdata, nbuflen);
      nbuff_free(pNBuf);
      return 0;
   }

   /* allocate a transfer request - data is zeroed */
   pTransReq = spu_alloc_trans_req(0);
   if (IS_ERR(pTransReq))
   {
      SPU_TRACE(("spu_blog_lookup_ctx failed: ipsa %x, ipda %x, spi %x\n", htonl(ipsa), htonl(ipda), htonl(spi)));
      spuinfo->stats.encDrops++;
      cache_invalidate_len(pdata, nbuflen);
      nbuff_free(pNBuf);
      return -1;
   }
   pTransReq->callback   = spu_blog_fc_crypt_done_us;
   pTransReq->pNBuf      = pNBuf;
   pTransReq->ivsize     = blog_p->esptx.ivsize;
   pTransReq->dst        = blog_p->esptx.dst_p;
   pTransReq->pSpuCtx    = pCtx;

   xfrm = pTransReq->dst->xfrm;
   if ( NULL == xfrm )
   {
      spuinfo->stats.encErrors++;
      spu_free_trans_req(pTransReq);
      cache_invalidate_len(pdata, nbuflen);
      nbuff_free(pTransReq->pNBuf);
      return 0;
   }
   net = xs_net(xfrm);

   spin_lock_bh(&xfrm->lock);
   do
   {
      int padlen;
      int datalen;

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
         cache_invalidate_len(pdata, nbuflen);
         nbuff_free(pTransReq->pNBuf);
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
            skb = nbuff_xlate(pTransReq->pNBuf);
         }
         skb_reset_network_header(skb);
         xfrm->repl->overflow(xfrm, skb);
         XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATESEQERROR);
         cache_invalidate_len(pdata, nbuflen);
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
      padlen  = pdata[nbuflen - SPU_MAX_ICV_LENGTH - 2];
      datalen = nbuflen - BLOG_IPV4_HDR_LEN - padlen - pTransReq->ivsize -
                BLOG_ESP_SPI_LEN - BLOG_ESP_SEQNUM_LEN - BLOG_ESP_PADLEN_LEN -
                BLOG_ESP_NEXT_PROTO_LEN - SPU_MAX_ICV_LENGTH;
      xfrm->curlft.bytes = datalen;
      xfrm->curlft.packets++;
   } while (0);
   spin_unlock_bh(&xfrm->lock);
   if ( ret != 0 )
   {
      spu_free_trans_req(pTransReq);
      blog_p->tuple_offset = 0xff;
      return ret;
   }

   _write32_align16((uint16_t *)&pdata[BLOG_IPV4_HDR_LEN+BLOG_ESP_SPI_LEN], htonl(seqno));

   /* flush all data */
   cache_flush_len(pdata, nbuflen);
   ret = spu_runner_process_ipsec(pTransReq, pNBuf, BLOG_IPV4_HDR_LEN);
   if (ret != 0)
   {
      spu_free_trans_req(pTransReq);
      SPU_TRACE(("spu_blog_xmit_us: spu_runner_process_ipsec returned error %d\n", ret));
      spuinfo->stats.encDrops++;
      cache_invalidate_len(pdata, nbuflen);
      nbuff_free(pNBuf);
   }   
   
   return 0;
} /* spu_blog_xmit_us */

static int spu_blog_xmit_ds(pNBuff_t pNBuf, struct net_device *dev)
{
   struct spu_trans_req *pTransReq;
   struct spu_ctx       *pCtx;
   struct blog_t        *blog_p;
   struct iphdr         *iph;
   uint32_t              ipsa;
   uint32_t              ipda;
   uint32_t              spi;
   int                   ret;
   struct xfrm_state    *xfrm;
   struct net           *net;
   unsigned int          hlen;
   unsigned int          seqno;
   struct sec_path      *secpath;
   unsigned char        *pdata;
   struct sk_buff       *skb;
   int                   nbuflen;

   spuinfo->stats.decIngress++;
   if ( IS_SKBUFF_PTR(pNBuf) )
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

   pCtx = spu_blog_lookup_ctx(ipsa, ipda, spi);
   if ( pCtx == NULL )
   {
      SPU_TRACE(("spu_blog_lookup_ctx failed: ipsa %x, ipda %x, spi %x\n", htonl(ipsa), htonl(ipda), htonl(spi)));
      spuinfo->stats.decErrors++;
      nbuff_free(pNBuf);
      return -EINVAL;
   }

   /* validate sequence number and xfrm state */
   secpath = blog_p->esptx.secPath_p;
   xfrm = secpath->xvec[secpath->len-1];
   if ( NULL == xfrm )
   {
      spuinfo->stats.decErrors++;
      cache_invalidate_len(pdata, nbuflen);
      nbuff_free(pNBuf);
      return -1;
   }
   net = xs_net(xfrm);

   hlen = BLOG_IPV4_HDR_LEN + BLOG_ESP_SPI_LEN;
   seqno = _read32_align16((uint16_t *)&pdata[hlen]);

   spin_lock_bh(&xfrm->lock);
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
         kfree_skb(skb);
         spuinfo->stats.decErrors++;
         break;
      }

      ret = xfrm_state_check_expire(xfrm);
      if (ret)
      {
         XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEEXPIRED);
         nbuff_free(pNBuf);
         spuinfo->stats.decErrors++;
         break;
      }
   } while (0);
   spin_unlock_bh(&xfrm->lock);
   if ( ret )
   {
      return -1;
   }

   /* allocate a transfer request - data is zeroed */
   pTransReq = spu_alloc_trans_req(0);
   if (IS_ERR(pTransReq))
   {
      SPU_TRACE(("spu_blog_xmit_ds failed to alloacte transfer request\n"));
      spuinfo->stats.decDrops++;
      cache_invalidate_len(pdata, nbuflen);
      nbuff_free(pNBuf);
      return -1;
   }

   pTransReq->callback   = spu_blog_fc_crypt_done_ds;
   pTransReq->pSpuCtx    = pCtx;
   pTransReq->ivsize     = blog_p->esptx.ivsize;
   pTransReq->pNBuf      = pNBuf;
   /* save the blog secpath in pTransReq */
   pTransReq->sp         = secpath;
   /* save the rx dev info as the info is needed by the kernel in case when
      something went wrong and the packet has to be sent back to kernel */
   pTransReq->dev        = blog_p->rx_dev_p;

   cache_flush_len(pdata, nbuflen);
   ret = spu_runner_process_ipsec(pTransReq, pNBuf, BLOG_IPV4_HDR_LEN);
   if (ret != 0)
   {
      spu_free_trans_req(pTransReq);
      SPU_TRACE(("spu_blog_xmit_ds: spu_runner_process_ipsec returned error %d\n", ret));
      spuinfo->stats.decDrops++;
      cache_invalidate_len(pdata, nbuflen);
      nbuff_free(pNBuf);
   } 
   
   return 0;
}  /* spu_blog_xmit_ds */

static struct net_device *spu_blog_create_device(char *name, uint32_t dir)
{
   int ret = 0;
   struct net_device *dev;
   
   dev = alloc_netdev(0, name, NET_NAME_UNKNOWN, ether_setup);
   if (dev != NULL)
   {
      dev_alloc_name(dev, dev->name);

      if (dir == 1)
      {
         dev->netdev_ops = &spuinfo->spu_dev_ops_us;
      }
      else
      {
         dev->netdev_ops = &spuinfo->spu_dev_ops_ds;
         dev->priv_flags |= IFF_WANDEV;
      }

      dev->header_ops = &spuinfo->spu_dev_header_ops;
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
         printk("spu_blog_create_device: register_netdev failed\n");
         free_netdev(dev);
      }
   }
   else
   {
      printk("spu_blog_create_device: alloc_netdev failed\n");
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
    INIT_LIST_HEAD(&spuinfo->ctxList);
    spuinfo->spu_dev_ops_us.ndo_start_xmit = (HardStartXmitFuncP)spu_blog_xmit_us;
    spuinfo->spu_dev_us = spu_blog_create_device("spu_us_dummy", 1);
    if ( IS_ERR(spuinfo->spu_dev_us) )
    {
        printk("spu_blog_init: failed to create upstream device\n");
        return -1;
    }

    spuinfo->spu_dev_ops_ds.ndo_start_xmit = (HardStartXmitFuncP)spu_blog_xmit_ds;
    spuinfo->spu_dev_ds = spu_blog_create_device("spu_ds_dummy", 0);
    if ( IS_ERR(spuinfo->spu_dev_ds) )
    {
        free_netdev(spuinfo->spu_dev_us);
        printk("spu_blog_init: failed to create downstream device\n");
        return -1;
    }

    return 0;
}

void spu_blog_unregister(void)
{
   if (spuinfo->spu_dev_us)
   {
      unregister_netdev(spuinfo->spu_dev_us);
      free_netdev(spuinfo->spu_dev_us);
   }
   if (spuinfo->spu_dev_ds)
   {
      unregister_netdev(spuinfo->spu_dev_ds);
      free_netdev(spuinfo->spu_dev_ds);
   }
}

