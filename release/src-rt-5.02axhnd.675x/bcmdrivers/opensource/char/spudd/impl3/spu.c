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

#include <bcmspudrv.h>
#include <bcm_mm.h>
#include "spu.h"
#include "spu_runner.h"
#include "spu_blog.h"

struct spu_info *spuinfo = NULL;

typedef void (*IPSec_SPU_FN_IOCTL) (unsigned long arg);
static int  spu_open (struct inode *inode, struct file *filp);
static long spu_ioctl_unlocked(struct file *filep, unsigned int cmd, unsigned long arg);
static int  spu_release (struct inode *inode, struct file *filp);

static void spu_dump_data( struct aead_request *areq, char *giv, char *pId, int ingress)
{
#if defined(SPU_PACKET_TRACE)
    struct crypto_aead *aead;
    struct scatterlist *sg;
    int                 i;
    unsigned int        totalSize;
    unsigned int        dispCnt;
    char               *ptr;

    aead = crypto_aead_reqtfm(areq);
    sg   = areq->assoc;
    totalSize = 0;
    while ( sg )
    {
        totalSize += sg->length;
        sg = sg_next(sg);
    }

    totalSize += crypto_aead_ivsize(aead);

    if ( 1 == ingress )
    {
        sg = areq->src;
    }
    else
    {
        sg = areq->dst;
    }
    while ( sg )
    {
        totalSize += sg->length;
        sg = sg_next(sg);
    }

    printk("%s: data length %d (display max is %d)\n", pId, totalSize, (64 + areq->assoclen + crypto_aead_ivsize(aead)));
    dispCnt = 0;
    sg = areq->assoc;
    while ( sg )
    {
        ptr = (char *)page_address(sg_page(sg)) + sg->offset;
        for ( i = 0; i < sg->length; i++)
        {
           printk("%02x ", ptr[i]);
           if ( 0 == ((dispCnt+1) & 0x1F))
           {
              printk("\n");
           }
           dispCnt++;
        }
        sg = sg_next(sg);
    }

    if ( giv )
    {
        for ( i = 0; i < crypto_aead_ivsize(aead); i++)
        {
           printk("%02x ", giv[i]);
           if ( 0 == ((dispCnt+1) & 0x1F))
           {
              printk("\n");
           }
           dispCnt++;
        }
    }
    else
    {
        for ( i = 0; i < crypto_aead_ivsize(aead); i++)
        {
           printk("xx ");
           if ( 0 == ((dispCnt+1) & 0x1F))
           {
              printk("\n");
           }
           dispCnt++;
        }
    }

    if ( 1 == ingress )
    {
        sg = areq->src;
    }
    else
    {
        sg = areq->dst;
    }
    while ( sg )
    {
        ptr = (char *)page_address(sg_page(sg)) + sg->offset;
        for ( i = 0; i < sg->length; i++)
        {
           printk("%02x ", ptr[i]);
           if ( 0 == ((dispCnt+1) & 0x1F))
           {
              printk("\n");
           }
           dispCnt++;
           /* print out max of 64 bytes of data after the assoc data */
           if (dispCnt >= (64 + areq->assoclen + crypto_aead_ivsize(aead))) break;
        }
        sg = sg_next(sg);
    }
    printk("\n");
#endif
}

/* allocate a transfer request and initialize to 0 */
struct spu_trans_req *spu_alloc_trans_req(unsigned int cryptoflags)
{
    struct spu_trans_req *pTransReq = NULL;
    unsigned long         flags;

    spin_lock_irqsave(&spuinfo->spuListLock, flags);

    if ( list_empty(&spuinfo->transReqList) ) {
       SPU_TRACE(("exhausted transfer requests\n"));
       spin_unlock_irqrestore(&spuinfo->spuListLock, flags);
       return ERR_PTR(-EBUSY);
    }

    pTransReq = list_first_entry(&spuinfo->transReqList, struct spu_trans_req, entry);
    list_del(&pTransReq->entry);

    spin_unlock_irqrestore(&spuinfo->spuListLock, flags);
    /* do not initialize index or list */
    memset(&pTransReq->context, 0, sizeof(*pTransReq) - offsetof(struct spu_trans_req, context));
    return pTransReq;
} /* spu_alloc_trans_req */

void spu_free_trans_req(struct spu_trans_req *pTransReq)
{
   unsigned long         flags;
   struct spu_trans_req *pReqFree;

   spin_lock_irqsave(&spuinfo->spuListLock, flags);
   list_for_each_entry(pReqFree, &spuinfo->transReqList, entry)
   {
      if ( pReqFree->index == pTransReq->index )
      {
         /* buffer was freed twice */
         SPU_TRACE(("Transfer request already in free list\n"));
         spin_unlock_irqrestore(&spuinfo->spuListLock, flags);
         return;
      }
   }
   list_add_tail(&pTransReq->entry, &spuinfo->transReqList);
   spin_unlock_irqrestore(&spuinfo->spuListLock, flags);
}

static int spu_aead_need_fallback(struct aead_request *areq, unsigned char *iv, int *pSrcLen)
{
    uint8_t             num_frags;
    struct scatterlist *sg;
    int                 srcLen = 0;
    int                 retVal = SPU_FALLBACK_METHOD_NONE;
    struct sk_buff     *pSkb;
    char               *ptr1;
    char               *ptr2;
    struct crypto_aead *aead = crypto_aead_reqtfm(areq);

    do 
    {
        /* make sure that source and destination are the same */
        if (areq->src != areq->dst) 
        {
            retVal = SPU_FALLBACK_METHOD_SKB;
            break;
        }

        /* if there is more than one fragment for associated data
           then use fallback */
        num_frags = 0;
        sg = areq->assoc;
        while ( sg )
        {
            num_frags++;
            sg = sg_next(sg);
        }
        if ( num_frags > 1 )
        {
            retVal = SPU_FALLBACK_METHOD_SKB;
            break;
        }

        /* if there is more than one fragment for src data
           then use fallback */
        num_frags = 0;
        sg = areq->src;
        while ( sg )
        {
            srcLen += sg->length;
            num_frags++;
            sg = sg_next(sg);
        }
        if ( num_frags > 1 )
        {
            retVal = SPU_FALLBACK_METHOD_SKB;
            break;
        }

        /* make sure that src data (assoc - iv - text) is contiguous */
        sg = areq->assoc;
        ptr1 = page_address(sg_page(sg)) + sg->offset + sg->length;
        ptr2 = iv;
        if (ptr1 != ptr2 )
        {
            retVal = SPU_FALLBACK_METHOD_SKB;
            break;
        }

        ptr2 += crypto_aead_ivsize(aead);
        sg = areq->src;
        ptr1 = page_address(sg_page(sg)) + sg->offset;
        if (ptr1 != ptr2 )
        {
            retVal = SPU_FALLBACK_METHOD_SKB;
            break;
        }
    } while ( 0 );

    pSkb = areq->base.data;
    if (atomic_read(&pSkb->users) != 1)
    {
        SPU_TRACE(("SKB has multiple users\n"));
        retVal = SPU_FALLBACK_METHOD_SKB;
    }

    /* validate length */
    if ( SPU_FALLBACK_METHOD_SKB == retVal )
    {
       int len;
       len = areq->assoclen + srcLen;
       len += crypto_aead_ivsize(crypto_aead_reqtfm(areq));

       if ( len > SPU_MAX_PAYLOAD )
       {
           retVal = SPU_FALLBACK_METHOD_CIPHER;
       }
    }
    else if ( pSkb->len > SPU_MAX_PAYLOAD )
    {
        retVal = SPU_FALLBACK_METHOD_CIPHER;
    }

    if ( pSrcLen != NULL ) *pSrcLen = srcLen;
    return retVal;
}

static struct sk_buff *spu_skb_alloc_and_copy(struct aead_request *areq, unsigned char *giv, int srcLen)
{
    struct crypto_aead *aead;
    struct sk_buff     *pSkb;
    struct scatterlist *sg;
    char               *pData;
    void               *ptr;
    int                 len;
 
    /* alocate data for assoc data and IV, IV may not need to be copied */
    aead   = crypto_aead_reqtfm(areq);
    len    = srcLen + areq->assoclen + crypto_aead_ivsize(aead);
    pSkb   = alloc_skb(len, GFP_ATOMIC);
    if ( NULL == pSkb ) {
       return ERR_PTR(-ENOMEM);
    }

    /* call to skb_reserve not requried - do not need any headroom */
    pData = skb_put(pSkb, len);
    sg = areq->assoc;
    while(sg)
    {
        if ( len < sg->length )
        {
           SPU_TRACE(("Unable to copy associataed data: required %d, available %d\n", sg->length, len));
           kfree_skb(pSkb);
           return ERR_PTR(-EINVAL);
        }
        ptr = page_address(sg_page(sg)) + sg->offset;
        memcpy(pData, ptr, sg->length);
        len   -= sg->length;
        pData += sg->length;
        sg = sg_next(sg);
    }

    /* copy iv if pointer is set, otherwise just leave room for it */
    if ( giv )
    {
        if ( len < crypto_aead_ivsize(aead) )
        {
           SPU_TRACE(("Unable to copy src iv: required %d, available %d\n", crypto_aead_ivsize(aead), len));
           kfree_skb(pSkb);
           return ERR_PTR(-EINVAL);
        }
        /* copy the IV to the buffer */
        memcpy(pData, giv, crypto_aead_ivsize(aead));
    }
    len -= crypto_aead_ivsize(aead);
    pData += crypto_aead_ivsize(aead);

    sg = areq->src;
    while(sg)
    {
        if ( len < sg->length )
        {
           SPU_TRACE(("Unable to copy src data: required %d, available %d\n", sg->length, len));
           kfree_skb(pSkb);
           return ERR_PTR(-EINVAL);
        }
        ptr = page_address(sg_page(sg)) + sg->offset;
        memcpy(pData, ptr, sg->length);
        len   -= sg->length;
        pData += sg->length;
        sg = sg_next(sg);
    }

    return pSkb;
}

static void spu_skb_copy_and_free(struct spu_trans_req *pTransReq)
{
    struct aead_request *areq = pTransReq->context;
    struct crypto_aead  *aead;
    struct scatterlist  *sg;
    char                *pData;
    void                *ptr;
    int                  srcLen;
    int                  ivlen;

    aead   = crypto_aead_reqtfm(areq);
    pData  = pTransReq->pAllocSkb->data + areq->assoclen;
    srcLen = pTransReq->pAllocSkb->len - areq->assoclen;
    do
    {
        ivlen = crypto_aead_ivsize(aead);
        if ( pTransReq->giv )
        {
            if ( srcLen < ivlen )
            {
               SPU_TRACE(("Unable to copy dst iv: required %d, available %d\n", ivlen, srcLen));
               break;
            }
            memcpy(pTransReq->giv, pData, ivlen);
        }
        srcLen -= ivlen;
        pData += ivlen;

        sg = areq->dst;
        while(sg)
        {
            if ( srcLen < sg->length )
            {
               SPU_TRACE(("Unable to copy dst data: required %d, available %d\n", ivlen, srcLen));
               break;
            }
            ptr = page_address(sg_page(sg)) + sg->offset;
            memcpy(ptr, pData, sg->length);
            srcLen -= sg->length;
            pData  += sg->length;
            sg = sg_next(sg);
        }
    } while(0);

    kfree_skb(pTransReq->pAllocSkb);
    pTransReq->pAllocSkb = NULL;

    return;
}


static void spu_aead_fallback_cipher_comp_enc(struct crypto_async_request *areq, int err)
{
    struct aead_request *req;
    struct aead_givcrypt_request *subgreq;

    spuinfo->stats.encSpuEgress++;
    /* free sub request and complete original request */
    req = container_of(areq, struct aead_request, base);
    subgreq = container_of(req, struct aead_givcrypt_request, areq);
    req = areq->data;
    kfree(subgreq);
    req->base.complete(&req->base, err);
}

static void spu_aead_fallback_cipher_comp_dec(struct crypto_async_request *areq, int err)
{
    struct aead_request *subreq;
    struct aead_request *req;

    spuinfo->stats.decSpuEgress++;
    /* free sub request and complete original request */
    subreq = container_of(areq, struct aead_request, base);
    req = areq->data;
    kfree(subreq);
    req->base.complete(&req->base, err);
}

static int spu_aead_do_fallback_cipher(struct crypto_aead *aead,
                                       void *org_request,
                                       bool is_encrypt)
{
    int err;
    int reqlen;

    if (aead) {
        if ( is_encrypt )
        {
            struct aead_givcrypt_request *greq = (struct aead_givcrypt_request *)org_request;
            struct aead_request *req = &greq->areq;
            struct aead_givcrypt_request *subreq;

            reqlen = sizeof(struct aead_givcrypt_request) + crypto_aead_reqsize(aead);
            subreq = kmalloc(reqlen, GFP_ATOMIC);
            if ( NULL == subreq )
            {
               return -ENOMEM;
            }

            aead_givcrypt_set_tfm(subreq, aead);
            aead_givcrypt_set_callback(subreq, aead_request_flags(req),
                                       spu_aead_fallback_cipher_comp_enc,
                                       org_request);
            aead_givcrypt_set_crypt(subreq, req->src, req->dst, 
                                    req->cryptlen, req->iv);
            aead_givcrypt_set_assoc(subreq, req->assoc, req->assoclen);
            aead_givcrypt_set_giv(subreq, greq->giv, greq->seq);
            err = crypto_aead_givencrypt(subreq);
            if ( err != -EINPROGRESS )
            {
                kfree(subreq);
            }
        }
        else
        {
            struct aead_request *req = (struct aead_request *)org_request;
            struct aead_request *subreq;

            reqlen = sizeof(struct aead_request) + crypto_aead_reqsize(aead);
            subreq = kmalloc(reqlen, GFP_ATOMIC);
            if ( NULL == subreq )
            {
               return -ENOMEM;
            }

            aead_request_set_tfm(subreq, aead);
            aead_request_set_callback(subreq, aead_request_flags(req), 
                                      spu_aead_fallback_cipher_comp_dec, 
                                      org_request);
            aead_request_set_crypt(subreq, req->src, req->dst, 
                                   req->cryptlen, req->iv);
            aead_request_set_assoc(subreq, req->assoc, req->assoclen);
            err = crypto_aead_decrypt(subreq);
            if ( err != -EINPROGRESS )
            {
                kfree(subreq);
            }
        }
    }
    else
    {
        err = -EINVAL;
    }
    return err;
}


static void spu_esp_crypt_done (struct spu_trans_req *transReq)
{
    struct aead_request *areq = transReq->context;

    if ( transReq->pAllocSkb )
    {
        /* copy data and free alloacted skb even if there 
           was an error */
        spu_skb_copy_and_free(transReq);
    }

    if ( 0 == transReq->err )
    {
       if ( SPU_DIRECTION_US == transReq->pSpuCtx->direction )
       {
           spuinfo->stats.encSpuEgress++;
       }
       else
       {
           spuinfo->stats.decSpuEgress++;
       }
    }

    spu_dump_data(areq, transReq->giv ? transReq->giv : areq->iv, "spu_esp_crypt_done", 0);

    aead_request_complete (areq, transReq->err);

    return;
} /* spu_esp_crypt_done */

static void spu_compute_aes_dec_key(struct spu_ctx *ctx, unsigned char *dest)
{
    struct crypto_aes_ctx gen_aes_key;
    int                   key_pos;
    int                   key_len;

    memset(&gen_aes_key, 0, sizeof(struct crypto_aes_ctx));

    key_len = (ctx->descAlg >> BCM_DESC_ENCR_KEYLEN_SHIFT) & BCM_DESC_ENCR_KEYLEN_MASK;
    crypto_aes_expand_key(&gen_aes_key, &ctx->encrypt_key[0], key_len);
    key_pos = key_len + 24;
    memcpy(dest, &gen_aes_key.key_enc[key_pos], 16);
    switch (key_len) {
        case AES_KEYSIZE_256:
            key_pos -= 2;
            /* fall */
        case AES_KEYSIZE_192:
            key_pos -= 2;
            memcpy(&dest[16], &gen_aes_key.key_enc[key_pos], 16);
            break;
    }
    
#if defined(SPU_KEY_TRACE)
{
    int i;
    printk("compute_aes_dec_key: len %d, ", key_len);
    for(i = 0; i < key_len; i++)
    {
      printk("%02x", dest[i]);
    }
    printk("\n");
}
#endif
}

static int spu_aead_setauthsize (struct crypto_aead *aead, 
                                 unsigned int authsize)
{
    struct spu_ctx *ctx = crypto_aead_ctx(aead);

    if (authsize != SPU_MAX_ICV_LENGTH)
    {
        return -EINVAL;
    }

    /* setkey the fallback just in case we need to use it */
    if (ctx->fallback_cipher) {
        return crypto_aead_setauthsize(ctx->fallback_cipher, authsize);
    }

    return 0;
} /* spu_aead_setauthsize */

static int spu_aead_setkey (struct crypto_aead *aead, 
                            const u8           *key, 
                            unsigned int        keylen)
{
    struct spu_ctx *ctx = crypto_aead_ctx (aead);
    struct rtattr *rta;
    struct crypto_authenc_key_param *param;
    unsigned int authkeylen;
    unsigned int enckeylen;
    const uint8_t *pKey = key;
    unsigned int length = keylen;
    
    rta = (struct rtattr *)pKey;
    if (!RTA_OK (rta, length))
        goto badkey;

    if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
        goto badkey;

    if (RTA_PAYLOAD (rta) < sizeof (*param))
        goto badkey;

    param = RTA_DATA (rta);
    enckeylen = be32_to_cpu (param->enckeylen);

    pKey += RTA_ALIGN (rta->rta_len);
    length -= RTA_ALIGN (rta->rta_len);
    if (length < enckeylen)
    {
        goto badkey;
    }
    authkeylen = length - enckeylen;

    if ( spu_runner_descr_key_validate(enckeylen, authkeylen) < 0 )
    {
        goto badkey;
    }

    ctx->update = 1;
    memcpy (ctx->auth_key, pKey, authkeylen);
    memcpy (ctx->encrypt_key, (pKey + authkeylen), enckeylen);
    ctx->descAlg |= (authkeylen & BCM_DESC_AUTH_KEYLEN_MASK) << BCM_DESC_AUTH_KEYLEN_SHIFT;
    ctx->descAlg |= (enckeylen & BCM_DESC_ENCR_KEYLEN_MASK) << BCM_DESC_ENCR_KEYLEN_SHIFT;
    if (ctx->descAlg & BCM_DESC_ENCR_ALG_AES)
    {
        spu_compute_aes_dec_key(ctx, &ctx->decrypt_key[0]);
    }

    /* this is unlikely but check anyway */
    if (ctx->direction != SPU_DIRECTION_INVALID)
    {
       spu_runner_descr_config(ctx);
    }

    /* setkey the fallback just in case we needto use it */
    if (ctx->fallback_cipher)
    {
        int ret;

        crypto_aead_clear_flags(ctx->fallback_cipher, CRYPTO_TFM_REQ_MASK);
        crypto_aead_set_flags(ctx->fallback_cipher, crypto_aead_get_flags(aead) &
                              CRYPTO_TFM_REQ_MASK);
        ret = crypto_aead_setkey(ctx->fallback_cipher, key, keylen);
        if (ret)
        {
            crypto_aead_set_flags(aead, crypto_aead_get_flags(ctx->fallback_cipher) &
                                  CRYPTO_TFM_RES_MASK);
        }
    }

#if defined(SPU_KEY_TRACE)
{
    int i;

    printk("spu_aead_setkey - akey len %d, ", authkeylen);
    for(i = 0; i < authkeylen; i++)
    {
        printk("%02x", ctx->auth_key[i]);
    }
    printk("\n");

    printk("spu_aead_setkey - ekey len %d, ", enckeylen);
    for(i = 0; i < enckeylen; i++)
    {
        printk("%02x", ctx->encrypt_key[i]);
    }
    printk("\n");
}
#endif
    return 0;

badkey:
    SPU_TRACE(("spu_aead_setkey: Invlaid key\n"));
    crypto_aead_set_flags (aead, CRYPTO_TFM_RES_BAD_KEY_LEN);

    return -EINVAL;
} /* spu_aead_setkey */

static int spu_aead_givencrypt(struct aead_givcrypt_request *req)
{
    struct aead_request      *areq = &req->areq;
    struct crypto_aead       *aead;
    struct spu_trans_req     *pTransReq;
    int                       ret;
    struct spu_ctx           *pCtx;
    int                       srcLen = 0;
    struct sk_buff           *pSkb = NULL;
    unsigned int              offset;
    int                       fbMethod;

    spu_dump_data(areq, NULL, "spu_aead_givencrypt", 1);

    spuinfo->stats.encIngress++;
    fbMethod = spu_aead_need_fallback(areq, req->giv, &srcLen);
    if (SPU_FALLBACK_METHOD_SKB == fbMethod)
    {
        pSkb = spu_skb_alloc_and_copy(areq, NULL, srcLen);
        /* if we could not allocate an skb then use
           SPU_FALLBACK_METHOD_CIPHER */
        if ( IS_ERR(pSkb) )
        {
            int err = PTR_ERR(pSkb);
            pSkb = NULL;
            if ( -ENOMEM == err )
            {
                fbMethod = SPU_FALLBACK_METHOD_CIPHER;
            }
            else
            {
                spuinfo->stats.encDrops++;
                return err;
            }
        }
        else
        {
            spuinfo->stats.encFallback++;
        }
    }

    aead = crypto_aead_reqtfm(areq);
    pCtx = crypto_aead_ctx(aead);
    if (SPU_FALLBACK_METHOD_CIPHER == fbMethod)
    {
        int err;

        err = spu_aead_do_fallback_cipher(pCtx->fallback_cipher, req, 1);
        if ( 0 == err )
        {
            spuinfo->stats.encFallback++;
            spuinfo->stats.encSpuEgress++;
        }
        else if ( -EINPROGRESS == err )
        {
            spuinfo->stats.encFallback++;
        }
        else
        {
            spuinfo->stats.encDrops++;
        }
        return err;
    }

    pTransReq = spu_alloc_trans_req(areq->base.flags);
    if (IS_ERR(pTransReq))
    {
        kfree_skb(pSkb);
        spuinfo->stats.encDrops++;
        return PTR_ERR(pTransReq);
    }
    if ( pCtx->direction == SPU_DIRECTION_INVALID )
    {
       pCtx->next_hdr  = areq->next_hdr;
       pCtx->direction = SPU_DIRECTION_US;
       spu_runner_descr_config(pCtx);
    }
    pTransReq->context  = areq;
    pTransReq->callback = spu_esp_crypt_done;
    pTransReq->pSpuCtx  = pCtx;
    if ( pSkb != NULL )
    {
       pTransReq->pAllocSkb = pSkb;
       offset = 0;
    }
    else
    {
       pTransReq->pAllocSkb = NULL;
       pSkb   = areq->base.data;
       offset = areq->data_offset;
    }
    pTransReq->giv    = req->giv;
    pTransReq->pNBuf  = SKBUFF_2_PNBUFF(pSkb);
    pTransReq->ivsize = crypto_aead_ivsize(aead);

#if defined(CONFIG_BLOG)
    if ( crypto_aead_get_flags(aead) & CRYPTO_TFM_REQ_MAY_BLOG )
      spu_blog_emit(pTransReq);
#endif

    /* flush all data */
    cache_flush_len(pSkb->data, pSkb->len);
    ret = spu_runner_process_ipsec(pTransReq, pTransReq->pNBuf, offset);
    if ( 0 == ret )
    {
        ret = -EINPROGRESS;
        pCtx->update = 0;
    }
    else
    {
        SPU_TRACE(("aead_giv_encrypt: spu_runner_process_ipsec returned error %d\n", ret));
        if ( pTransReq->pAllocSkb )
        {
            kfree_skb(pTransReq->pAllocSkb);
            pTransReq->pAllocSkb = NULL;
        }
        spu_free_trans_req(pTransReq);
        ret = -EBUSY;
        spuinfo->stats.encDrops++;
    }
    return ret;
} /* spu_aead_givencrypt */

static int spu_aead_decrypt (struct aead_request *areq)
{
    struct crypto_aead       *aead;
    struct spu_trans_req     *pTransReq;
    int                       ret;
    struct spu_ctx           *pCtx;
    int                       srcLen = 0;
    struct sk_buff           *pSkb = NULL;
    unsigned int              offset;
    int                       fbMethod;

    spu_dump_data(areq, areq->iv, "spu_aead_decrypt - org", 1);

    spuinfo->stats.decIngress++;
    fbMethod = spu_aead_need_fallback(areq, areq->iv, &srcLen);
    if (SPU_FALLBACK_METHOD_SKB == fbMethod)
    {
        pSkb = spu_skb_alloc_and_copy(areq, areq->iv, srcLen);
        /* if we could not allocate an skb then try
           SPU_FALLBACK_METHOD_CIPHER */
        if ( IS_ERR(pSkb) )
        {
            int err = PTR_ERR(pSkb);
            pSkb = NULL;
            if ( -ENOMEM == err )
            {
                fbMethod = SPU_FALLBACK_METHOD_CIPHER;
            }
            else
            {
                spuinfo->stats.decDrops++;
                return err;
            }
        }
        else
        {
            spuinfo->stats.decFallback++;
        }
    }

    aead = crypto_aead_reqtfm(areq);
    pCtx = crypto_aead_ctx(aead);
    if (SPU_FALLBACK_METHOD_CIPHER == fbMethod)
    {
        int err;
        err = spu_aead_do_fallback_cipher(pCtx->fallback_cipher, areq, 0);
        if ( 0 == err )
        {
            spuinfo->stats.decFallback++;
            spuinfo->stats.decSpuEgress++;
        }
        else if ( -EINPROGRESS == err )
        {
            spuinfo->stats.decFallback++;
        }
        else
        {
            spuinfo->stats.decDrops++;
        }
        return err;
    }

    pTransReq = spu_alloc_trans_req(areq->base.flags);
    if (IS_ERR(pTransReq))
    {
        kfree_skb(pSkb);
        spuinfo->stats.decDrops++;
        return PTR_ERR(pTransReq);
    }
    if ( pCtx->direction == SPU_DIRECTION_INVALID )
    {
       pCtx->direction = SPU_DIRECTION_DS;
       pCtx->next_hdr  = 0;
       spu_runner_descr_config(pCtx);
    }
    pTransReq->context  = areq;
    pTransReq->callback = spu_esp_crypt_done;
    pTransReq->err      = 0;
    pTransReq->pSpuCtx  = pCtx;
    if ( pSkb != NULL )
    {
       pTransReq->pAllocSkb = pSkb;
       offset = 0;
    }
    else
    {
       pSkb = areq->base.data;
       pTransReq->pAllocSkb = NULL;
       offset = areq->data_offset;
    }
    pTransReq->giv    = NULL;
    pTransReq->pNBuf  = SKBUFF_2_PNBUFF(pSkb);
    pTransReq->ivsize = crypto_aead_ivsize(aead);

#if defined(CONFIG_BLOG)
    if ( crypto_aead_get_flags(aead) & CRYPTO_TFM_REQ_MAY_BLOG )
      spu_blog_emit(pTransReq);
#endif

    /* flush all data */
    cache_flush_len(pSkb->data, pSkb->len);
    ret = spu_runner_process_ipsec(pTransReq, pTransReq->pNBuf, offset);
    if ( 0 == ret )
    {
        ret = -EINPROGRESS;
        pCtx->update = 0;
    }
    else
    {
        SPU_TRACE(("spu_aead_decrypt spu_runner_process_ipsec returned error %d\n", ret));
        if ( pTransReq->pAllocSkb )
        {
            kfree_skb(pTransReq->pAllocSkb);
            pTransReq->pAllocSkb = NULL;
        }
        spu_free_trans_req(pTransReq);
        ret = -EBUSY;
        spuinfo->stats.decDrops++;
    }        

    return ret;
} /* spu_aead_decrypt */

static void spu_cra_exit (struct crypto_tfm *tfm)
{
    struct spu_ctx *ctx = crypto_tfm_ctx (tfm);

    if (ctx->fallback_cipher) {
        crypto_free_aead(ctx->fallback_cipher);
        ctx->fallback_cipher = NULL;
    }

    spu_runner_descr_put(ctx->pDescr);
#if defined(CONFIG_BLOG)
    spu_blog_ctx_del(ctx);
#endif
} /* spu_cra_exit */

static int spu_cra_init (struct crypto_tfm *tfm)
{
    struct crypto_alg *alg = tfm->__crt_alg;
    struct spu_crypto_alg *spuAlg;
    struct spu_ctx *ctx = crypto_tfm_ctx (tfm);

    spuAlg = container_of(alg, struct spu_crypto_alg, cryptoAlg);
    ctx->direction = SPU_DIRECTION_INVALID;
    ctx->update    = 1;
    ctx->descAlg   = spuAlg->descAlg;
    ctx->pDescr    = spu_runner_descr_get();
    if ( ctx->pDescr == NULL )
    {
        return -ENOBUFS;
    }
#if defined(CONFIG_BLOG)
    spu_blog_ctx_add(ctx);
#endif

    if (alg->cra_flags & CRYPTO_ALG_NEED_FALLBACK)
    {
        ctx->fallback_cipher = crypto_alloc_aead(alg->cra_name, 0,
                                                 CRYPTO_ALG_ASYNC | 
                                                 CRYPTO_ALG_NEED_FALLBACK);
        if (IS_ERR(ctx->fallback_cipher))
        {
            SPU_TRACE(("%s() Error: failed to allocate fallback for %s\n", 
                       __func__, alg->cra_name));
            return PTR_ERR(ctx->fallback_cipher);
        }
    }

    return 0;
} /* spu_cra_init */

static struct spu_crypto_alg *spu_alg_alloc( struct spu_alg_template *algTemplate )
{
    struct spu_crypto_alg *spuAlg;
    struct crypto_alg     *alg;

    spuAlg = kzalloc(sizeof(struct spu_crypto_alg), GFP_KERNEL);
    if (NULL == spuAlg)
    {
        return ERR_PTR(-ENOMEM);
    }

    INIT_LIST_HEAD(&spuAlg->entry);
    spuAlg->descAlg = algTemplate->descAlg;
    alg = &spuAlg->cryptoAlg;
    memcpy(alg, &algTemplate->alg, sizeof(struct crypto_alg));
    alg->cra_module = THIS_MODULE;
    alg->cra_init = spu_cra_init;
    alg->cra_exit = spu_cra_exit;
    alg->cra_priority = SPU_CRA_PRIORITY;
    alg->cra_alignmask = 0;
    alg->cra_ctxsize = sizeof (struct spu_ctx);
    alg->cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | 
                     CRYPTO_ALG_BLOG | CRYPTO_ALG_NEED_FALLBACK;
    alg->cra_type = &crypto_aead_type;

    return spuAlg;
} /* spu_alg_alloc */

static struct spu_alg_template spu_algs[] = {
  {
   .alg = {
       .cra_name = "authenc(hmac(sha1),cbc(aes))",
       .cra_driver_name = "authenc(hmac(sha1-spu),cbc(aes-spu))",
       .cra_blocksize = AES_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = AES_BLOCK_SIZE,
            .maxauthsize = SHA1_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_AES |
              BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(sha1),cbc(des3_ede))",
       .cra_driver_name = "authenc(hmac(sha1-spu),cbc(des3_ede-spu))",
       .cra_blocksize = DES3_EDE_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = DES3_EDE_BLOCK_SIZE,
            .maxauthsize = SHA1_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_3DES | 
              BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(sha1),cbc(des))",
       .cra_driver_name = "authenc(hmac(sha1-spu),cbc(des-spu))",
       .cra_blocksize = DES_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = DES_BLOCK_SIZE,
            .maxauthsize = SHA1_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_DES | 
              BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(md5),cbc(aes))",
       .cra_driver_name = "authenc(hmac(md5-spu),cbc(aes-spu))",
       .cra_blocksize = AES_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = AES_BLOCK_SIZE,
            .maxauthsize = MD5_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_AES | 
              BCM_DESC_AUTH_ALG_MD5,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(md5),cbc(des3_ede))",
       .cra_driver_name = "authenc(hmac(md5-spu),cbc(des3_ede-spu))",
       .cra_blocksize = DES3_EDE_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = DES3_EDE_BLOCK_SIZE,
            .maxauthsize = MD5_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_3DES | 
              BCM_DESC_AUTH_ALG_MD5,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(md5),cbc(des))",
       .cra_driver_name = "authenc(hmac(md5-spu),cbc(des-spu))",
       .cra_blocksize = DES_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = DES_BLOCK_SIZE,
            .maxauthsize = MD5_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_DES | 
              BCM_DESC_AUTH_ALG_MD5,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(sha256),cbc(aes))",
       .cra_driver_name = "authenc(hmac(sha256-spu),cbc(aes-spu))",
       .cra_blocksize = AES_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = AES_BLOCK_SIZE,
            .maxauthsize = SHA256_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_AES |
              BCM_DESC_AUTH_ALG_SHA256,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(sha256),cbc(des3_ede))",
       .cra_driver_name = "authenc(hmac(sha256-spu),cbc(des3_ede-spu))",
       .cra_blocksize = DES3_EDE_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = DES3_EDE_BLOCK_SIZE,
            .maxauthsize = SHA256_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_3DES | 
              BCM_DESC_AUTH_ALG_SHA256,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(sha256),cbc(des))",
       .cra_driver_name = "authenc(hmac(sha256-spu),cbc(des-spu))",
       .cra_blocksize = DES_BLOCK_SIZE,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
             /*.encrypt is not supported as iv is always generated */
            .geniv = "<built-in>",
            .ivsize = DES_BLOCK_SIZE,
            .maxauthsize = SHA256_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_DES | 
              BCM_DESC_AUTH_ALG_SHA256,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(sha1),ecb(cipher_null))",
       .cra_driver_name = "authenc(hmac(sha1-spu),ecb(cipher_null-spu))",
       .cra_blocksize = 1,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
            .geniv = "<built-in>",
            .ivsize = 0,
            .maxauthsize = SHA1_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_NULL | 
              BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(sha256),ecb(cipher_null))",
       .cra_driver_name = "authenc(hmac(sha256-spu),ecb(cipher_null-spu))",
       .cra_blocksize = 1,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
            .geniv = "<built-in>",
            .ivsize = 0,
            .maxauthsize = SHA256_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_NULL | 
              BCM_DESC_AUTH_ALG_SHA256,
  },
  {
   .alg = {
       .cra_name = "authenc(hmac(md5),ecb(cipher_null))",
       .cra_driver_name = "authenc(hmac(md5-spu),ecb(cipher_null-spu))",
       .cra_blocksize = 1,
       .cra_aead = {
            .setkey = spu_aead_setkey,
            .setauthsize = spu_aead_setauthsize,
            .decrypt = spu_aead_decrypt,
            .givencrypt = spu_aead_givencrypt,
            .geniv = "<built-in>",
            .ivsize = 0,
            .maxauthsize = MD5_DIGEST_SIZE,
            }
       },
   .descAlg = BCM_DESC_ENCR_ALG_NULL | 
              BCM_DESC_AUTH_ALG_MD5,
  }
};


static void spu_unregister(unsigned long arg)
{
    struct spu_crypto_alg *spuAlg;
    struct spu_crypto_alg *temp;
    SPUDDDRV_INITIALIZE    KArg;
    int                    err;
    int                    loopCnt;
    int                    reqCnt;
    int                    incomplete = 0;

    do
    {
        KArg.bvStatus = SPUSTS_SUCCESS;
        if ( NULL == spuinfo->transReqBase )
        {
            /* already unregistered - return success */
            break;
        }

        /* release algorithms that are not referenced */
        list_for_each_entry(spuAlg, &spuinfo->algList, entry)
        {
            if ( atomic_read(&spuAlg->cryptoAlg.cra_refcnt) != 1 )
            {
               printk("reference to alg %s still held - spu unregister incomplete\n", spuAlg->cryptoAlg.cra_name);
               incomplete = 1;
               break;
            }
        }
        /* some algorithms are still referenced - unable to shutdown so return error */
        if ( 1 == incomplete )
        {
            KArg.bvStatus = SPUSTS_ERROR;
            break;
        }

        /* release algorithms that are not referenced */
        list_for_each_entry_safe(spuAlg, temp, &spuinfo->algList, entry)
        {
            err = crypto_unregister_alg(&spuAlg->cryptoAlg);
            if ( err )
            {
               printk("unregister alg failed %s, err %d\n", spuAlg->cryptoAlg.cra_name, err);
            }
            list_del(&spuAlg->entry);
        }

        /* wait for all pending requests to complete */
        loopCnt = 0;
        while ( loopCnt < 10 )
        {
             struct spu_trans_req *req;

             reqCnt = 0;
             spin_lock_bh(&spuinfo->spuListLock);
             list_for_each_entry(req, &spuinfo->transReqList, entry) 
             {
                 reqCnt++;
             }
             spin_unlock_bh(&spuinfo->spuListLock);
             if ( reqCnt == SPU_NUM_TRANS_REQ )
             {
                 break;
             }
             loopCnt++;
             msleep(100);
        }
        if (10 == loopCnt)
        {
           SPU_TRACE(("spu_unregister: %d requests still pending\n", (SPU_NUM_TRANS_REQ - reqCnt)));
        }

#if defined(CONFIG_BLOG)
        spu_blog_unregister();
#endif

        if (spuinfo->transReqBase) kfree(spuinfo->transReqBase);
        spuinfo->transReqBase = NULL;
    } while (0);

    if ( arg != 0 )
    {
        put_user(KArg.bvStatus, &((SPUDDDRV_INITIALIZE *)arg)->bvStatus);
    }
    return;
} /* spu_unregister */

static void spu_register (unsigned long arg)
{
    struct spu_crypto_alg   *spuAlg;
    struct spu_alg_template *algTemplate;
    int                      i;
    int                      err;
    struct spu_trans_req    *pTransReq;
    int                      size;
    SPUDDDRV_INITIALIZE      KArg;

    KArg.bvStatus = SPUSTS_SUCCESS;
    do
    {
        if ( spuinfo->transReqBase != NULL )
        {
            /* already registered - return success */
            break;
        }

        memset(&spuinfo->stats, 0, sizeof(SPU_STAT_PARMS));
        INIT_LIST_HEAD(&spuinfo->algList);
        INIT_LIST_HEAD(&spuinfo->transReqList);

        /* allocate transfer requests */
        size = (SPU_NUM_TRANS_REQ * sizeof(struct spu_trans_req));
        spuinfo->transReqBase = kzalloc(size, GFP_KERNEL);
        if (NULL == spuinfo->transReqBase)
        {
            printk("spu_register: Insufficient memory for spu transfer requests\n");
            KArg.bvStatus = SPUSTS_MEMERR;
            break;
        }

        pTransReq = (struct spu_trans_req *)spuinfo->transReqBase;
        for (i = 0; i < SPU_NUM_TRANS_REQ; i++)
        {
            list_add_tail(&pTransReq->entry, &spuinfo->transReqList);
            pTransReq->index = i + SPU_TRANS_REQ_BASE_IDX;
            pTransReq++;
        }

#if defined(CONFIG_BLOG)
        err = spu_blog_register();
        if ( err != 0 )
        {
            printk("spu_blog_register failed with error %d\n", err);
            KArg.bvStatus = err;
            break;
        }
#endif
        /* register crypto algorithms */
        for (i = 0; i < ARRAY_SIZE(spu_algs); i++)
        {
            algTemplate = &spu_algs[i];
            spuAlg = spu_alg_alloc(algTemplate);
            if ( IS_ERR(spuAlg) )
            {
               printk("unable to allocate algorithm\n");
               KArg.bvStatus = SPUSTS_ERROR;
               continue;
            }

            err = crypto_register_alg (&spuAlg->cryptoAlg);
            if (err)
            {
                printk("spu_register: alg %s registration failed, err %d\n", spuAlg->cryptoAlg.cra_driver_name, err);
            }
            else
            {
                list_add_tail(&spuAlg->entry, &spuinfo->algList);
                SPU_TRACE(("spu_register: alg %s registered\n", spuAlg->cryptoAlg.cra_driver_name));
            }
        }
     } while ( 0 );

    if ( SPUSTS_SUCCESS != KArg.bvStatus )
    {
        spu_unregister(0);
    }

    put_user (KArg.bvStatus, &((SPUDDDRV_INITIALIZE *)arg)->bvStatus);

    return;
} /* spu_register */

/***************************************************************************
 * Function Name: do_spu_test
 * Description  : 
 *                
 * Returns      : N/A
 ***************************************************************************/
static void spu_test(unsigned long arg)
{
    SPU_TRACE (("SPU: do_spu_test\n"));

    return;
} /* do_spu_test */

/***************************************************************************
 * Function Name: do_spu_show
 * Description  : show statistics
 *                
 * Returns      : N/A
 ***************************************************************************/
static void spu_show (unsigned long arg)
{
    SPUDDDRV_SPU_SHOW  KArg;

    SPU_TRACE(("IPSEC SPU: do_spu_show\n"));

    if(!spuinfo)
    {
        return;
    }

    KArg.bvStatus = SPUSTS_SUCCESS;
    memcpy(&KArg.stats, &spuinfo->stats, sizeof(SPU_STAT_PARMS));
    if ( 0 != copy_to_user((void *)arg, &KArg, sizeof(SPUDDDRV_SPU_SHOW)) )
    {
        printk("spu_show - error copying data to user\n");
    }
    return;
} /* do_spu_show */

/***************************************************************************
 * Function Name: spu_ioctl
 * Description  : Entry point for application commands.
 * Returns      : 0 - success.
 ***************************************************************************/
static long spu_ioctl_unlocked(struct file *filep, unsigned int cmd, unsigned long arg)
{
    long                rt = 0;
    unsigned int        cmdnr = _IOC_NR(cmd);
    IPSec_SPU_FN_IOCTL  IoctlFuncs[] = { spu_register,
                                         spu_unregister,
                                         spu_show,
                                         spu_test };

    mutex_lock(&spuinfo->spuIoctlMutex);
    if (cmdnr < MAX_SPUDDDRV_IOCTL_COMMANDS)
    {
        SPU_TRACE (("IPSEC SPU: Ioctl cmd %u\n", cmdnr));
        (*IoctlFuncs[cmdnr])(arg);
    }
    else
    {
        rt = -EINVAL;
    }
    mutex_unlock(&spuinfo->spuIoctlMutex);

    return rt;
}

/***************************************************************************
 * Function Name: spu_release
 * Description  : Called when an application closes this device.
 * Returns      : 0 - success.
 ***************************************************************************/
static int spu_release (struct inode *inode, struct file *filp)
{
  return (0);
}


/***************************************************************************
 * Function Name: spu_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int spu_open (struct inode *inode, struct file *filp)
{
    return (0);
}

__exit void bcm_spu_deinit(void)
{
    spu_runner_deinit( );

    if ( spuinfo )
    {
        if ( !IS_ERR(spuinfo->device) )
        {
            device_destroy(spuinfo->spu_class, MKDEV(spuinfo->major, 0));
            cdev_del(&spuinfo->cdev);
        }

        if (!IS_ERR(spuinfo->spu_class))
        {
            class_destroy(spuinfo->spu_class);
        }

        if ( spuinfo->major )
        {
            unregister_chrdev_region(MKDEV(spuinfo->major, 0), SPU_NUM_DEVICES);
        }

        kfree(spuinfo);
        spuinfo = NULL;
    }

    return;
}

__init int bcm_spu_init (void)
{
    int   err;
    dev_t dev = 0;
    dev_t devno;

    spuinfo = kzalloc(sizeof(struct spu_info), GFP_KERNEL);
    if (NULL == spuinfo) {
        printk("bcm_spu_init: Insufficient memory for context data\n");
        err = -ENOMEM;
        goto fail;
    }

    /* Get a range of minor numbers (starting with 0) to work with */
    err = alloc_chrdev_region(&dev, 0, SPU_NUM_DEVICES, SPU_DEVICE_NAME);
    if (err < 0) {
        printk(KERN_WARNING "bcm_spu_init: alloc_chrdev_region() failed\n");
        goto fail;
    }
    spuinfo->major = MAJOR(dev);

    /* create device class */
    spuinfo->spu_class = class_create(THIS_MODULE, SPU_DEVICE_NAME);
    if (IS_ERR(spuinfo->spu_class)) {
        err = PTR_ERR(spuinfo->spu_class);
        goto fail;
    }

    devno = MKDEV(spuinfo->major, 0);
    spin_lock_init(&spuinfo->spuListLock);
    mutex_init(&spuinfo->spuIoctlMutex);
    spuinfo->spu_file_ops.unlocked_ioctl = spu_ioctl_unlocked;
#if defined(CONFIG_COMPAT)
    spuinfo->spu_file_ops.compat_ioctl = spu_ioctl_unlocked,
#endif
    spuinfo->spu_file_ops.open = spu_open;
    spuinfo->spu_file_ops.release = spu_release;

    memset(&spuinfo->cdev, 0, sizeof(spuinfo->cdev));
    cdev_init(&spuinfo->cdev, &spuinfo->spu_file_ops);
    spuinfo->cdev.owner = THIS_MODULE;

    err = cdev_add(&spuinfo->cdev, devno, 1);
    if (err)
    {
        printk(KERN_WARNING "bcm_spu_init: Error %d while trying to add %s%d",
               err, SPU_DEVICE_NAME, 0);
        goto fail;
    }

    spuinfo->device = device_create(spuinfo->spu_class, NULL, 
                                    devno, NULL, SPU_DEVICE_NAME "%d", 0);
    if (IS_ERR(spuinfo->device))
    {
        err = PTR_ERR(spuinfo->device);
        printk(KERN_WARNING "bcm_spu_init: Error %d while trying to create %s%d",
                    err, SPU_DEVICE_NAME, 0);
        cdev_del(&spuinfo->cdev);
        goto fail;
    }

    err = spu_runner_init( );
    if ( err != 0 )
    {
        goto fail;
    }
    return 0;

fail:
    bcm_spu_deinit();
    return err;
}

module_init(bcm_spu_init);
module_exit(bcm_spu_deinit);
MODULE_LICENSE("GPL");
