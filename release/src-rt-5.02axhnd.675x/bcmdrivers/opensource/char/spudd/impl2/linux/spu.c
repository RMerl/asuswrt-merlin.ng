/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
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
#include <linux/mod_devicetable.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/crypto.h>
#include <linux/hw_random.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <crypto/algapi.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/skcipher.h>
#include <crypto/scatterwalk.h>
#include "../bcmipsec.h"
#include "../bcmsad.h"
#include "../spu.h"
#include "bcm_map_part.h"
#include "bcm_intr.h"
#include "bcmspudrv.h"
#include "../spudrv.h"

extern int spu_hash_register(void);
extern int spu_hash_unregister(void);
extern pspu_dev_ctrl_t pdev_ctrl;
extern void spu_dump_array(char *msg, unsigned char *buf, uint16 len);

extern int spu_process_ipsec(struct spu_trans_req *trans_req, BCM_CRYPTOOP *op);

struct spu_alg_template
{
    struct crypto_alg alg;
    __be32 desc_hdr_template;
};

struct spu_crypto_alg
{
    struct list_head entry;
    __be32 desc_hdr_template;
    struct crypto_alg crypto_alg;
};

int spu_get_framgments(struct scatterlist *scatter_list, int nbytes)
{
    struct scatterlist *sg = scatter_list;
    int fragments = 0;

    while (nbytes > 0) {
	    fragments++;
    	    nbytes -= sg->length;
	    sg = sg_next(sg);

    }

    return fragments;
} /* spu_is_framgmented */

static int aead_setauthsize (struct crypto_aead *authenc, 
                             unsigned int authsize)
{
    struct spu_ctx *ctx = crypto_aead_ctx (authenc);

    ctx->authsize = authsize;

    return 0;
} /* aead_setauthsize */

static int aead_setkey (struct crypto_aead *authenc, 
                        const u8 * key, 
                        unsigned int keylen)
{
    struct spu_ctx *ctx = crypto_aead_ctx (authenc);
    struct rtattr *rta = (void *) key;
    struct crypto_authenc_key_param *param;
    unsigned int authkeylen;
    unsigned int enckeylen;

    if (!RTA_OK (rta, keylen))
	    goto badkey;

    if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
	    goto badkey;

    if (RTA_PAYLOAD (rta) < sizeof (*param))
	    goto badkey;

    param = RTA_DATA (rta);
    enckeylen = be32_to_cpu (param->enckeylen);

    key += RTA_ALIGN (rta->rta_len);
    keylen -= RTA_ALIGN (rta->rta_len);

    if (keylen < enckeylen)
	    goto badkey;

    authkeylen = keylen - enckeylen;

    if (keylen > SPU_MAX_KEY_SIZE)
	    goto badkey;

    memcpy (ctx->auth_key, key, authkeylen);
    memcpy (ctx->crypt_key, (key + authkeylen), enckeylen);

    ctx->cryptkeylen = enckeylen;
    ctx->authkeylen = authkeylen;

    return 0;

badkey:
    crypto_aead_set_flags (authenc, CRYPTO_TFM_RES_BAD_KEY_LEN);

    return -EINVAL;
} /* aead_setkey */

void spu_release_frag_list(struct spu_pkt_frag *pkt_frag)
{
    struct spu_pkt_frag *pkt_frag_tmp = NULL;

    while(pkt_frag)
    {
	    pkt_frag_tmp = pkt_frag;
	    pkt_frag = pkt_frag->next;
	    kfree(pkt_frag_tmp);
    }
    
    return;
} /* spu_release_frag_list */

/*
 * copy buffer to scatterlist
 */
static int linearize_buf_to_sc_list(struct scatterlist *sg_list, 
                                    int nbytes, 
                                    int encDataOffset,
                                    void *buf,
                                    int len)
{
    struct scatterlist *sg = sg_list;
    void *ptr;
    void *tmp;
    int ret = BCM_STATUS_OK;

    tmp = buf;
    tmp += (RX_MSG_HDR_SIZE + encDataOffset);
    while((nbytes > 0) && (sg))
    {
	    ptr = page_address (sg_page (sg)) + sg->offset;

	    memcpy(ptr, tmp, sg->length);
	    nbytes -= sg->length;
	    tmp += sg->length;
	    SPU_DATA_DUMP("linearize_buf_to_sc_list", (ptr-36), sg->length);
	    sg = sg_next(sg);
    }

    if(0 != nbytes)
	    ret = BCM_STATUS_INVALID_INPUT;

    return ret;
} /* linearize_buf_to_sc_list */

/*
 * assign data back to sc_list
 */
static int assign_data_to_sc_list(struct spu_trans_req *trans_req,
                                  struct scatterlist *sgs,
                                  struct scatterlist *sgd,
                                  int cryptlen,
                                  int authsize,
                                  int ivsize)
{
    int ret = BCM_STATUS_OK;
    int nbytes;

    nbytes = cryptlen + authsize;

    if(1 == trans_req->alloc_buff_spu)
    {
	    ret = linearize_buf_to_sc_list(sgd, 
                                       nbytes,
                                       trans_req->headerLen,
                                       trans_req->dbuf,
                                       trans_req->dlen);

	    if(ret != BCM_STATUS_OK)
	    {
	        SPU_TRACE(("Can't move the data to sc_list\n"));
	    }
    }

    return ret;
} /* assign_data_to_sc_list */

/*
 * process ipsec esp response from spu
 */
static int process_ipsec_esp_response(struct spu_trans_req *trans_req)
{
    struct aead_request *areq = trans_req->context;
    int err = trans_req->err;
    struct scatterlist *sgs = areq->src;
    struct scatterlist *sgd = areq->dst;
    struct crypto_aead *aead = NULL;
    struct spu_ctx *ctx = NULL;
    int cryptlen;
    int authsize;
    int ivsize;
    int nbytes;

    if (0 == trans_req->err)
    {
        aead = crypto_aead_reqtfm (areq);
        ctx = crypto_aead_ctx (aead);
        authsize = ctx->authsize;
        ivsize = crypto_aead_ivsize (aead);
        cryptlen = areq->cryptlen;
        nbytes = cryptlen + authsize;
    
        SPU_DATA_DUMP("process_ipsec_esp_response",trans_req->dfrags_list->buf, 
                                                    trans_req->dfrags_list->len);
    
        err = assign_data_to_sc_list(trans_req, sgs, sgd, cryptlen, authsize, ivsize);
        if(err != BCM_STATUS_OK)
        {
            err = -EINVAL;
        }
    }
    else
    {
        err = trans_req->err;
    }

    if((trans_req->dbuf))
    {
        kfree(trans_req->dbuf);
    }
    spu_release_frag_list(trans_req->sfrags_list);
    spu_release_frag_list(trans_req->dfrags_list);
    kfree(trans_req);

    return err;

} /* process_ipsec_esp_response */

/*
 * ipsec_esp descriptor callbacks
 */
static void ipsec_esp_encrypt_done (struct spu_trans_req *trans_req)
{
    struct aead_request *areq = trans_req->context;
    int err = trans_req->err;

    err = process_ipsec_esp_response(trans_req);

    aead_request_complete (areq, err);

    return;
} /* ipsec_esp_encrypt_done */

struct spu_trans_req *spu_alloc_trans_req(uint32 cryptoflags)
{
    struct spu_trans_req *trans_req = NULL;
    gfp_t flags = cryptoflags & CRYPTO_TFM_REQ_MAY_SLEEP ? GFP_KERNEL : 
                                                               GFP_ATOMIC;
    trans_req = kmalloc(sizeof(struct spu_trans_req), flags);
    if(!trans_req)
    {
	    printk("Memory allocation failure\n");
	    return ERR_PTR(-ENOMEM);
    }
    
    memset(trans_req, 0, sizeof(struct spu_trans_req));

    return trans_req;
} /* spu_alloc_trans_req */

/*
 * Initialize ipsec request operation */
static int init_ipsec_request_op(struct spu_trans_req *trans_req, 
                                 struct spu_ctx *ctx,
                                 BCM_CRYPTOOP *op)
{
    int ret = BCM_STATUS_OK;

    memset (op, 0, sizeof (BCM_CRYPTOOP));

    op->encrKey = &ctx->crypt_key[0];
    op->authKey = &ctx->auth_key[0];

    op->spi = 1;			/* FIXME x->id.spi; */
    op->flags.ipv4 = 1;
    op->flags.dir = trans_req->dir;
    op->flags.proto = BCMSAD_PROTOCOL_ESP;
    op->callback = NULL;
    op->flags.mode = BCMSAD_MODE_TUNNEL;

    if(ctx->desc_hdr_template & BCM_DESC_ENCR_ALG_NONE)
	    op->encrAlg = BCMSAD_ENCR_ALG_NONE;
    if(ctx->desc_hdr_template & BCM_DESC_ENCR_ALG_NULL)
	    op->encrAlg = BCMSAD_ENCR_ALG_NULL;
    else if(ctx->desc_hdr_template & BCM_DESC_ENCR_ALG_DES)
	    op->encrAlg = BCMSAD_ENCR_ALG_DES;
    else if(ctx->desc_hdr_template & BCM_DESC_ENCR_ALG_3DES)
	    op->encrAlg = BCMSAD_ENCR_ALG_3DES;
    else if(ctx->desc_hdr_template & BCM_DESC_ENCR_ALG_AES)
	    op->encrAlg = BCMSAD_ENCR_ALG_AES;
    else if(ctx->desc_hdr_template & BCM_DESC_ENCR_ALG_AES_CTR)
	    op->encrAlg = BCMSAD_ENCR_ALG_AES_CTR;

    if(ctx->desc_hdr_template & BCM_DESC_AUTH_ALG_NONE)
	    op->authAlg = BCMSAD_AUTH_ALG_NONE;
    else if(ctx->desc_hdr_template & BCM_DESC_AUTH_ALG_SHA1)
	    op->authAlg = BCMSAD_AUTH_ALG_SHA1;
    else if(ctx->desc_hdr_template & BCM_DESC_AUTH_ALG_MD5)
	    op->authAlg = BCMSAD_AUTH_ALG_MD5;

    op->encrKeyLen = ctx->cryptkeylen;
    op->authKeyLen = ctx->authkeylen;

    return ret;
} /* init_ipsec_request_op */

/*
 * copy buffer from scatterlist
 */
static int sc_list_to_dma_in_list(struct spu_trans_req *trans_req,
				 struct scatterlist *sg_list, 
                                 int nbytes, 
                                 int cryptlen,
                                 int authsize,
                                 int ivsize)
{
    struct scatterlist *sg = sg_list;
    int buffer_len;
    void *ptr;
    int ret = BCM_STATUS_OK;
    struct spu_pkt_frag *pkt_frag = NULL;
    struct spu_pkt_frag *pkt_frag_prev = NULL;

    buffer_len = 0;
    while((nbytes > 0) && (sg))
    {
        ptr = page_address (sg_page (sg)) + sg->offset;
        /* first entry */
        if(0 == buffer_len)
        {
            if ((trans_req->sfrags == 1) &&
                (trans_req->dir == BCMSAD_DIR_OUTBOUND))
            {
                buffer_len = cryptlen + trans_req->headerLen;
            }
            else
            {
                 buffer_len = sg->length + trans_req->headerLen;
            }
            ptr = ptr - trans_req->headerLen;
	    }
	    else
	    {
	        buffer_len = sg->length;
	    }

	    pkt_frag = kmalloc(sizeof(struct spu_pkt_frag), GFP_ATOMIC);
	    if(NULL == pkt_frag)
	    {
	        SPU_TRACE(("No memory for fragmented buffer\n"));
	        return (-ENOMEM);
	    }

	    if(!pkt_frag_prev)
	        trans_req->sfrags_list = pkt_frag;
	    else
	        pkt_frag_prev->next = pkt_frag;

	    pkt_frag->buf = ptr;
	    pkt_frag->len = buffer_len;
	    pkt_frag->next= NULL;
	    pkt_frag_prev = pkt_frag;
	    nbytes -= sg->length;
	    sg = sg_next(sg);
    }

    if((trans_req->sfrags > 1) &&
       (trans_req->dir == BCMSAD_DIR_OUTBOUND) &&
       (pkt_frag))
    {
	    pkt_frag->len = pkt_frag->len - authsize;
    }

    if(0 != nbytes)
	    ret = BCM_STATUS_INVALID_INPUT;

    return ret;
} /* sc_list_to_dma_in_list */

static int sc_list_to_dma_out_list(struct spu_trans_req *trans_req,
				 struct scatterlist *sgd, 
                                 int nbytes, 
                                 int ivsize)
{
    void *ptr;
    int ret = BCM_STATUS_OK;
    struct spu_pkt_frag *pkt_frag = NULL;
    struct spu_pkt_frag *pkt_frag_prev = NULL;
    int nloops = 0;
    int out_dma_len;

    trans_req->dlen = (nbytes + trans_req->headerLen + RX_MSG_HDR_SIZE + RX_STS_SIZE);
    if((trans_req->dfrags == 1) && (!trans_req->alloc_buff_spu))
    {
        ptr = page_address(sg_page (sgd)) + sgd->offset;
        ptr = ptr - (trans_req->headerLen + RX_MSG_HDR_SIZE);
        trans_req->dStatus = ptr + trans_req->dlen - RX_STS_SIZE;

        pkt_frag = kmalloc(sizeof(struct spu_pkt_frag), GFP_ATOMIC);
        if(NULL == pkt_frag)
        {
            SPU_TRACE(("No memory for fragmented buffer\n"));
            return  (-ENOMEM);
        }
        pkt_frag->buf = ptr;
        pkt_frag->len = trans_req->dlen;
        pkt_frag->next = NULL;
        trans_req->dfrags_list = pkt_frag;
    }
    else
    {
        /* allocate memory */
        trans_req->alloc_buff_spu = 1;
        trans_req->dbuf = kmalloc(trans_req->dlen, GFP_ATOMIC);
        trans_req->dStatus = trans_req->dbuf + trans_req->dlen - RX_STS_SIZE;
        if(NULL == trans_req->dbuf)
        {
            SPU_TRACE(("No memory for fragmented buffer\n"));
            return  (-ENOMEM);
        }

        out_dma_len = trans_req->dlen;
        nloops = out_dma_len/SPU_CHAIN_SIZE;
        nloops++;
        while(nloops)
        {
            pkt_frag = kmalloc(sizeof(struct spu_pkt_frag), GFP_ATOMIC);
            if(NULL == pkt_frag)
            {
                SPU_TRACE(("No memory for fragmented buffer\n"));
                return (-ENOMEM);
            }

            if(!pkt_frag_prev)
            {
                trans_req->dfrags_list = pkt_frag;
                pkt_frag->buf = trans_req->dbuf;
            }
            else
            {
                pkt_frag_prev->next = pkt_frag;
                pkt_frag->buf = pkt_frag_prev->buf + pkt_frag_prev->len;
            }

            if(out_dma_len >= SPU_CHAIN_SIZE)
                pkt_frag->len = SPU_CHAIN_SIZE;
            else
                pkt_frag->len = out_dma_len;

            pkt_frag->next= NULL;
            pkt_frag_prev = pkt_frag;
            out_dma_len = out_dma_len - pkt_frag->len;
            nloops--;
            if(out_dma_len <= 0)
                break;
        }
    }

    return ret;
} /* sc_list_to_dma_out_list */
	    
/*
 * Assign buffers for spu engine
 */
static int assign_buffer_for_spu(struct spu_trans_req *trans_req,
                                 struct scatterlist *sgs,
                                 struct scatterlist *sgd,
                                 int cryptlen,
                                 int authsize,
                                 int ivsize)
{
    int nbytes = 0;
    int ret = BCM_STATUS_OK;

    nbytes = cryptlen + authsize;
    trans_req->sfrags = spu_get_framgments(sgs, nbytes);
    trans_req->dfrags = spu_get_framgments(sgd, nbytes);

    if (trans_req->dir == BCMSAD_DIR_OUTBOUND)
    {
        trans_req->slen = cryptlen + trans_req->headerLen;
    }
    else
    {
        trans_req->slen = cryptlen + trans_req->headerLen + authsize;
    }

    ret = sc_list_to_dma_in_list(trans_req,
                               sgs, 
                               nbytes, 
                               cryptlen,
                               authsize,
                               ivsize);
    if(ret == BCM_STATUS_OK)
    {
        ret = sc_list_to_dma_out_list(trans_req,
                               sgd, 
                               nbytes, 
                               ivsize);
    }
    return ret;
} /* assign_buffer_for_spu */


/*
 * fill in and submit ipsec_esp descriptor
 */
static int process_ipsec_esp(struct spu_trans_req *trans_req)
{
    struct aead_request *areq = (struct aead_request *)trans_req->context;
    struct crypto_aead *aead = crypto_aead_reqtfm (areq);
    struct spu_ctx *ctx = crypto_aead_ctx (aead);
    int cryptlen = areq->cryptlen;
    int authsize = ctx->authsize;
    int ivsize = crypto_aead_ivsize (aead);
    struct scatterlist *sgs = areq->src;
    struct scatterlist *sgd = areq->dst;
    int ret;
    BCM_CRYPTOOP op;

    SPU_TRACE(("cryptlen %d authsize %d ivsize %d sgslen %d \n",
              cryptlen, authsize, ivsize, sgs->length));

    init_ipsec_request_op(trans_req, ctx, &op); 

    ret = assign_buffer_for_spu(trans_req, 
                                sgs,
                                sgd,
                                cryptlen,
                                authsize,
                                ivsize);

    if(ret != BCM_STATUS_OK)
    {
        goto bcm_error;
    }

    ret = spu_process_ipsec(trans_req, &op);

    if(ret != BCM_STATUS_OK)
    {
        ret = -EINVAL;
    }
    else
    {
        ret = -EINPROGRESS;
    }

bcm_error:
    if (ret != -EINPROGRESS)
    {
        if((trans_req->dbuf))
        {
            kfree(trans_req->dbuf);
        }
        spu_release_frag_list(trans_req->sfrags_list);
        spu_release_frag_list(trans_req->dfrags_list);
        kfree(trans_req);
    }
    return ret;
} /* process_ipsec_esp */

static int aead_encrypt (struct aead_request *areq)
{
    struct spu_trans_req *trans_req = NULL;

    if (NULL == pdev_ctrl)
        return -ENODEV;

    trans_req = spu_alloc_trans_req(areq->base.flags);
    if (IS_ERR(trans_req))
        return PTR_ERR(trans_req);
    
    trans_req->type = SPU_TRANS_TYPE_IPSEC;
    trans_req->dir = BCMSAD_DIR_OUTBOUND;
    trans_req->callback = ipsec_esp_encrypt_done;
    trans_req->context = areq;
    trans_req->alloc_buff_spu = areq->alloc_buff_spu;
    trans_req->headerLen = areq->headerLen;

    return process_ipsec_esp (trans_req);
} /* aead_encrypt */

static void ipsec_esp_decrypt_swauth_done(struct spu_trans_req *trans_req)
{
    struct aead_request *req = (struct aead_request *)trans_req->context;
    int err = trans_req->err;

    err = process_ipsec_esp_response(trans_req);

    aead_request_complete (req, err);

    return;
} /* ipsec_esp_decrypt_swauth_done */

static int aead_decrypt (struct aead_request *areq)
{
    struct crypto_aead *authenc = crypto_aead_reqtfm (areq);
    struct spu_ctx *ctx = crypto_aead_ctx (authenc);
    unsigned int authsize = ctx->authsize;
    struct spu_trans_req *trans_req = NULL;

    if (NULL == pdev_ctrl)
        return -ENODEV;

    areq->cryptlen -= authsize;

    trans_req = spu_alloc_trans_req(areq->base.flags);
    if (IS_ERR(trans_req))
        return PTR_ERR(trans_req);
    
    trans_req->type = SPU_TRANS_TYPE_IPSEC;
    trans_req->dir = BCMSAD_DIR_INBOUND;
    trans_req->callback = ipsec_esp_decrypt_swauth_done;
    trans_req->context = areq;
    trans_req->alloc_buff_spu = areq->alloc_buff_spu;
    trans_req->headerLen = areq->headerLen;

    return process_ipsec_esp(trans_req);
} /* aead_decrypt */


static int aead_givencrypt (struct aead_givcrypt_request *req)
{
    struct aead_request *areq = &req->areq;
    struct crypto_aead *authenc = crypto_aead_reqtfm (areq);
    struct spu_ctx *ctx = crypto_aead_ctx (authenc);
    struct spu_trans_req *trans_req = NULL;

    if (NULL == pdev_ctrl)
        return -ENODEV;

    memcpy (req->giv, ctx->iv, crypto_aead_ivsize (authenc));
    /* make sure IV changes */
    if ( crypto_aead_ivsize(authenc) >= sizeof(__be64))
    {
        *(__be64 *)req->giv ^= cpu_to_be64(req->seq);
    }

    trans_req = spu_alloc_trans_req(areq->base.flags);
    if (IS_ERR(trans_req))
        return PTR_ERR(trans_req);
    
    trans_req->type = SPU_TRANS_TYPE_IPSEC;
    trans_req->dir = BCMSAD_DIR_OUTBOUND;
    trans_req->callback = ipsec_esp_encrypt_done;
    trans_req->context = areq;
    trans_req->alloc_buff_spu = areq->alloc_buff_spu;
    trans_req->headerLen = areq->headerLen;

    return process_ipsec_esp(trans_req);
} /* aead_givencrypt */


static struct spu_alg_template spu_algs[] = {
  {
   .alg = {
	   .cra_name = "authenc(hmac(sha1),cbc(aes))",
	   .cra_driver_name = "authenc(hmac(sha1-spu),cbc(aes-spu))",
	   .cra_blocksize = AES_BLOCK_SIZE,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_AES |
                                             BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
	   .cra_name = "authenc(hmac(sha1),cbc(des3_ede))",
	   .cra_driver_name = "authenc(hmac(sha1-spu),cbc(des3_ede-spu))",
	   .cra_blocksize = DES3_EDE_BLOCK_SIZE,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_3DES | 
                                             BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
	   .cra_name = "authenc(hmac(sha1),cbc(des))",
	   .cra_driver_name = "authenc(hmac(sha1-spu),cbc(des-spu))",
	   .cra_blocksize = DES_BLOCK_SIZE,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = SHA1_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_DES | 
                                             BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
	   .cra_name = "authenc(hmac(md5),cbc(aes))",
	   .cra_driver_name = "authenc(hmac(md5-spu),cbc(aes-spu))",
	   .cra_blocksize = AES_BLOCK_SIZE,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = AES_BLOCK_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_AES | 
                                             BCM_DESC_AUTH_ALG_MD5,
  },
  {
   .alg = {
	   .cra_name = "authenc(hmac(md5),cbc(des3_ede))",
	   .cra_driver_name = "authenc(hmac(md5-spu),cbc(des3_ede-spu))",
	   .cra_blocksize = DES3_EDE_BLOCK_SIZE,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES3_EDE_BLOCK_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_3DES | 
                                             BCM_DESC_AUTH_ALG_MD5,
  },
  {
   .alg = {
	   .cra_name = "authenc(hmac(md5),cbc(des))",
	   .cra_driver_name = "authenc(hmac(md5-spu),cbc(des-spu))",
	   .cra_blocksize = DES_BLOCK_SIZE,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = DES_BLOCK_SIZE,
			.maxauthsize = MD5_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_DES | 
                                             BCM_DESC_AUTH_ALG_MD5,
  },
  {
   .alg = {
	   .cra_name = "authenc(hmac(sha1),ecb(cipher_null))",
	   .cra_driver_name = "authenc(hmac(sha1-spu),ecb(cipher_null-spu))",
	   .cra_blocksize = 1,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = 0,
			.maxauthsize = SHA1_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_NULL | 
                                             BCM_DESC_AUTH_ALG_SHA1,
  },
  {
   .alg = {
	   .cra_name = "authenc(hmac(md5),ecb(cipher_null))",
	   .cra_driver_name = "authenc(hmac(md5-spu),ecb(cipher_null-spu))",
	   .cra_blocksize = 1,
	   .cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
	   .cra_type = &crypto_aead_type,
	   .cra_aead = {
			.setkey = aead_setkey,
			.setauthsize = aead_setauthsize,
			.encrypt = aead_encrypt,
			.decrypt = aead_decrypt,
			.givencrypt = aead_givencrypt,
			.geniv = "<built-in>",
			.ivsize = 0,
			.maxauthsize = MD5_DIGEST_SIZE,
			}
	   },
			.desc_hdr_template = BCM_DESC_IPSEC_ESP |
                                             BCM_DESC_ENCR_ALG_NULL | 
                                             BCM_DESC_AUTH_ALG_MD5,
  }
};





static int spu_cra_init (struct crypto_tfm *tfm)
{
    struct crypto_alg *alg = tfm->__crt_alg;
    struct spu_crypto_alg *spu_alg;
    struct spu_ctx *ctx = crypto_tfm_ctx (tfm);

    spu_alg = container_of (alg, struct spu_crypto_alg, crypto_alg);

    /* copy descriptor header template value */
    ctx->desc_hdr_template = spu_alg->desc_hdr_template;

    /* random first IV */
    get_random_bytes (ctx->iv, SPU_MAX_IV_LENGTH);

    return 0;
} /* spu_cra_init */

static struct spu_crypto_alg * spu_alg_alloc (struct spu_alg_template *template)
{
    struct spu_crypto_alg *spu_alg;
    struct crypto_alg *alg;

    spu_alg = kzalloc (sizeof (struct spu_crypto_alg), GFP_KERNEL);

    if (!spu_alg)
	    return ERR_PTR (-ENOMEM);

    alg = &spu_alg->crypto_alg;
    *alg = template->alg;

    alg->cra_module = THIS_MODULE;
    alg->cra_init = spu_cra_init;
    alg->cra_priority = SPU_CRA_PRIORITY;
    alg->cra_alignmask = 0;
    alg->cra_ctxsize = sizeof (struct spu_ctx);

    spu_alg->desc_hdr_template = template->desc_hdr_template;

    return spu_alg;
} /* spu_alg_alloc */


int bcm_crypto_dev_disable(void)
{
    struct spu_info *spu = pdev_ctrl->spu_linux;
    struct spu_crypto_alg *spu_alg, *temp;
    int incomplete = 0;

    list_for_each_entry(spu_alg, &spu->alg_list, entry)
    {
        if ( atomic_read(&spu_alg->crypto_alg.cra_refcnt) != 1 )
        {
           printk("reference to %s still held\n", spu_alg->crypto_alg.cra_driver_name);
           incomplete = 1;
           break;
        }
    }
    /* some algorithms are still referenced - unable to shutdown so return error */
    if ( 1 == incomplete )
    {
        return -1;
    }

    if ( spu_hash_unregister() )
    {
        return -1;
    }

    list_for_each_entry_safe(spu_alg, temp, &spu->alg_list, entry) {
        int ret = crypto_unregister_alg(&spu_alg->crypto_alg);
        if ( ret )
        {
            printk(KERN_ERR "Unregister failed for %s with error %d\n", spu_alg->crypto_alg.cra_driver_name, ret);
        }
        list_del(&spu_alg->entry);
        kfree(spu_alg);
    }

    kfree(spu);
    pdev_ctrl->spu_linux = NULL;

    return 0;
} /* spu_linux_cleanup */

int bcm_crypto_dev_init (void)
{
    int i;
    int err;
    struct spu_info *spu;
    struct spu_crypto_alg *spu_alg;

    if ( pdev_ctrl->spu_linux )
        return 0;

    spu = kzalloc(sizeof(struct spu_info), GFP_KERNEL);
    if (NULL == spu)
        return -ENOMEM;

    pdev_ctrl->spu_linux = spu;
    INIT_LIST_HEAD (&spu->alg_list);

    /* register crypto algorithms the device supports */
    for (i = 0; i < ARRAY_SIZE (spu_algs); i++)
    {
        spu_alg = spu_alg_alloc(&spu_algs[i]);
        if (IS_ERR(spu_alg))
        {
            err = PTR_ERR(spu_alg);
            goto err_out;
        }

        err = crypto_register_alg (&spu_alg->crypto_alg);
        if (err)
        {
            printk(KERN_ERR "alg %s registration failed\n",
                           spu_alg->crypto_alg.cra_driver_name);
            kfree (spu_alg);
        }
        else
        {
            list_add_tail (&spu_alg->entry, &spu->alg_list);

            SPU_TRACE(("Registration: %s\n", spu_alg->crypto_alg.cra_driver_name));
        }
    }

    spu_hash_register();

err_out:
    return 0;
} /* bcm_crypto_dev_init */

MODULE_LICENSE("GPL");
