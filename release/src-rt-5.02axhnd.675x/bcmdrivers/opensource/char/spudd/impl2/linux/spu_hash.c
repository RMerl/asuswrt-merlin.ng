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

#include <net/ah.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>

extern pspu_dev_ctrl_t pdev_ctrl;

extern void spu_dump_array(char *msg, unsigned char *buf, uint16 len);
extern struct spu_trans_req *spu_alloc_trans_req(uint32 cryptoflags);
extern int spu_get_framgments(struct scatterlist *scatter_list, int nbytes);
extern void spu_release_frag_list(struct spu_pkt_frag *pkt_frag);

extern int spu_process_ipsec(struct spu_trans_req *trans_req, BCM_CRYPTOOP *op);


#ifndef MAX_AH_AUTH_LEN 
#define MAX_AH_AUTH_LEN 64
#endif

static void spu_ah_digest_done (struct spu_trans_req *trans_req)
{
    struct ahash_request *req = trans_req->context;
    int err = trans_req->err;
    unsigned char *hmac;
    int buffer_offset = 0;
    int dstLen;


    if(req->nbytes % ALIGN_SIZE)
        buffer_offset = ALIGN_SIZE - (req->nbytes % ALIGN_SIZE);
    buffer_offset = req->nbytes + buffer_offset;

    if( 0 == trans_req->alloc_buff_spu)
    {
        unsigned char *buf;
        buf = page_address (sg_page (req->src)) + req->src->offset;
        hmac = buf + buffer_offset;
    }
    else
    {
        hmac = trans_req->dbuf + RX_MSG_HDR_SIZE + buffer_offset;
    }

    dstLen = crypto_ahash_digestsize(crypto_ahash_reqtfm(req));
    if ( dstLen > MAX_AH_AUTH_LEN )
    {
        dstLen = MAX_AH_AUTH_LEN;
    }
    memcpy(req->result, hmac, dstLen);

    if(err != BCM_STATUS_OK)
    {
        err = -EINVAL;
    }

    if((trans_req->dbuf))
    {
        kfree(trans_req->dbuf);
    }
    spu_release_frag_list(trans_req->sfrags_list);
    spu_release_frag_list(trans_req->dfrags_list);
    kfree(trans_req);

    req->base.complete(&req->base, err);

} /* spu_ah_digest_done */

int spu_ahash_md5_init(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_md5_init\n"));

    return 0;
} /* spu_ahash_md5_init */

int spu_ahash_md5_update(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_md5_update\n"));

    return 0;
} /* spu_ahash_md5_update */

int spu_ahash_md5_final(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_md5_final\n"));

    return 0;
} /* spu_ahash_md5_final */

int spu_ahash_md5_finup(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_md5_finup\n"));

    return 0;
} /* spu_ahash_md5_finup */

/*
 * copy buffer from scatterlist
 */
static int ah_sc_list_to_dma_in_list(struct spu_trans_req *trans_req,
                                     struct scatterlist *sg_list, 
                                     int nbytes)
{
    struct scatterlist *sg = sg_list;
    void *ptr;
    int ret = BCM_STATUS_OK;
    struct spu_pkt_frag *pkt_frag = NULL;
    struct spu_pkt_frag *pkt_frag_prev = NULL;

    while((nbytes > 0) && (sg))
    {
        ptr = page_address (sg_page (sg)) + sg->offset;
        SPU_DATA_DUMP("ah_sc_list_to_dma_in_list", ptr, sg->length);

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
        pkt_frag->len = sg->length;
        pkt_frag->next= NULL;

        pkt_frag_prev = pkt_frag;

        nbytes -= sg->length;
        sg = sg_next(sg);
    }

    return ret;
} /* ah_sc_list_to_dma_in_list */

static int ah_sc_list_to_dma_out_list(struct spu_trans_req *trans_req,
                                      struct scatterlist *sgd, 
                                      int nbytes)
{
    void *ptr;
    int ret = BCM_STATUS_OK;
    struct spu_pkt_frag *pkt_frag = NULL;
    struct spu_pkt_frag *pkt_frag_prev = NULL;
    int nloops = 0;
    int out_dma_len;

    trans_req->dlen = nbytes + MAX_AH_AUTH_LEN + RX_MSG_HDR_SIZE + RX_STS_SIZE;
    trans_req->dlen = (trans_req->dlen + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
    if((trans_req->dfrags == 1) &&
       (!trans_req->alloc_buff_spu))
    {
        ptr = page_address (sg_page (sgd)) + sgd->offset;
        trans_req->dbuf = NULL;
        trans_req->dStatus = NULL;
        pkt_frag = kmalloc(sizeof(struct spu_pkt_frag), GFP_ATOMIC);
        if(NULL == pkt_frag)
        {
            SPU_TRACE(("No memory for fragmented buffer\n"));
            return (-ENOMEM);
        }
        ptr = ptr - RX_MSG_HDR_SIZE;
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
        trans_req->dStatus = NULL;
        if(NULL == trans_req->dbuf)
        {
            SPU_TRACE(("No memory for fragmented buffer\n"));
            return (-ENOMEM);
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

int spu_ahash_md5_digest(struct ahash_request *req)
{
    struct spu_ctx *ctx = NULL;
    int ret = BCM_STATUS_OK;
    struct spu_trans_req *trans_req = NULL;
    BCM_CRYPTOOP op;

    if (NULL == pdev_ctrl)
        return -ENODEV;

    if(!(req && req->base.tfm))
        return -EINVAL;

    ctx = crypto_tfm_ctx(req->base.tfm);

    trans_req = spu_alloc_trans_req(req->base.flags);

    if (IS_ERR(trans_req))
        return PTR_ERR(trans_req);

    trans_req->sfrags = spu_get_framgments(req->src, req->nbytes);
    trans_req->dfrags = trans_req->sfrags;
    trans_req->alloc_buff_spu = req->alloc_buff_spu;
    trans_req->headerLen = req->headerLen;

    ret = ah_sc_list_to_dma_in_list(trans_req, req->src, req->nbytes); 
    if(ret == BCM_STATUS_OK)
        ret = ah_sc_list_to_dma_out_list(trans_req, req->src, req->nbytes); 

    if(ret == BCM_STATUS_OK)
    {
        trans_req->type = SPU_TRANS_TYPE_IPSEC;
        trans_req->callback = spu_ah_digest_done;
        trans_req->context = req;

        memset (&op, 0, sizeof (BCM_CRYPTOOP));
        op.authKey = &ctx->auth_key[0];
        op.authKeyLen = ctx->authkeylen;
        op.encrAlg = BCMSAD_ENCR_ALG_NONE;
        op.authAlg = BCMSAD_AUTH_ALG_MD5;
        op.spi = 1;            /* FIXME x->id.spi; */
        op.flags.ipv4 = 1;
        op.flags.dir = BCMSAD_DIR_OUTBOUND;
        op.flags.proto = BCMSAD_PROTOCOL_AH;
        op.callback = NULL;
        op.flags.mode = BCMSAD_MODE_TUNNEL;

        trans_req->slen = req->nbytes;

        ret = spu_process_ipsec(trans_req, &op);
    }

    if(ret != BCM_STATUS_OK)
    {
        ret = -EINVAL;
    }
    else
    {
        ret = -EINPROGRESS;
    }

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

} /* spu_ahash_md5_digest */

int spu_ahash_md5_export(struct ahash_request *req, void *out)
{
    SPU_TRACE(("spu_ahash_md5_export\n"));

    return 0;
} /* spu_ahash_md5_export */

int spu_ahash_md5_import(struct ahash_request *req, const void *in)
{
    SPU_TRACE(("spu_ahash_md5_import\n"));

    return 0;
} /* spu_ahash_md5_import */

static int spu_ahash_md5_setkey(struct crypto_ahash *tfm, 
                                const u8 *key, 
                                unsigned int keylen)
{
    struct spu_ctx *ctx = crypto_ahash_ctx (tfm);

    ctx->authkeylen = keylen;
    memcpy(ctx->auth_key, key, ctx->authkeylen);

    return 0;
} /* spu_ahash_md5_setkey */

static int spu_ahash_sha1_init(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_sha1_init\n"));

    return 0;
} /* spu_ahash_sha1_init */

static int spu_ahash_sha1_update(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_sha1_update\n"));

    return 0;
} /* spu_ahash_sha1_update */


static int spu_ahash_sha1_final(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_sha1_final\n"));

    return 0;
} /* spu_ahash_sha1_final */

static int spu_ahash_sha1_finup(struct ahash_request *req)
{
    SPU_TRACE(("spu_ahash_sha1_finup\n"));

    return 0;
} /* spu_ahash_sha1_finup */

static int spu_ahash_sha1_digest(struct ahash_request *req)
{
    struct spu_ctx *ctx = NULL;
    int ret = BCM_STATUS_OK;
    struct spu_trans_req *trans_req = NULL;
    BCM_CRYPTOOP op;

    if (NULL == pdev_ctrl)
        return -ENODEV;

    if(!(req && req->base.tfm))
	    return -EINVAL;

    ctx = crypto_tfm_ctx(req->base.tfm);

    trans_req = spu_alloc_trans_req(req->base.flags);

    if (IS_ERR(trans_req))
	    return PTR_ERR(trans_req);

    trans_req->sfrags = spu_get_framgments(req->src, req->nbytes);
    trans_req->dfrags = trans_req->sfrags;
    trans_req->alloc_buff_spu = req->alloc_buff_spu;
    trans_req->headerLen = req->headerLen;

    ret = ah_sc_list_to_dma_in_list(trans_req, req->src, req->nbytes); 
    if(ret == BCM_STATUS_OK)
        ret = ah_sc_list_to_dma_out_list(trans_req, req->src, req->nbytes); 

    if(ret == BCM_STATUS_OK)
    {
        trans_req->type = SPU_TRANS_TYPE_IPSEC;
        trans_req->callback = spu_ah_digest_done;
        trans_req->context = req;

        memset (&op, 0, sizeof (BCM_CRYPTOOP));
        op.authKey = &ctx->auth_key[0];
        op.authKeyLen = ctx->authkeylen;
        op.encrAlg = BCMSAD_ENCR_ALG_NONE;
        op.authAlg = BCMSAD_AUTH_ALG_SHA1;
        op.spi = 1;			/* FIXME x->id.spi; */
        op.flags.ipv4 = 1;
        op.flags.dir = BCMSAD_DIR_OUTBOUND;
        op.flags.proto = BCMSAD_PROTOCOL_AH;
        op.callback = NULL;
        op.flags.mode = BCMSAD_MODE_TUNNEL;

        trans_req->slen = req->nbytes;

        ret = spu_process_ipsec(trans_req, &op);
    }

    if(ret != BCM_STATUS_OK)
    {
	    ret = -EINVAL;
    }
    else
    {
	    ret = -EINPROGRESS;
    }

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
} /* spu_ahash_sha1_digest */

static int spu_ahash_sha1_export(struct ahash_request *req, void *out)
{

    return 0;
} /* spu_ahash_sha1_export */

static int spu_ahash_sha1_import(struct ahash_request *req, const void *in)
{
    return 0;
} /* spu_ahash_sha1_import */

static int spu_ahash_sha1_setkey(struct crypto_ahash *tfm, 
                                 const u8 *key,
                                 unsigned int keylen)
{
    struct spu_ctx *ctx = crypto_ahash_ctx (tfm);

    ctx->authkeylen = keylen;
    memcpy(ctx->auth_key, key, ctx->authkeylen);

    return 0;
} /* spu_ahash_sha1_setkey */

static int spu_ahash_cra_init(struct crypto_tfm *tfm)
{
    return 0;
} /* spu_ahash_cra_init */

static void spu_ahash_cra_exit(struct crypto_tfm *tfm)
{
    return;
} /* spu_ahash_cra_exit */

struct ahash_alg spu_md5_alg = {
    .init       = 	spu_ahash_md5_init,
    .update 	=	spu_ahash_md5_update,
    .final  	=	spu_ahash_md5_final,
    .finup  	=	spu_ahash_md5_finup,
    .digest     =	spu_ahash_md5_digest,
    .export     =	spu_ahash_md5_export,
    .import     =	spu_ahash_md5_import,
    .setkey     =	spu_ahash_md5_setkey,
    .halg = {
	.digestsize	=	MD5_DIGEST_SIZE,
	/* import/export not supported so statesize is not required
	   However, it needs to be set to pass ahash_prepare_alg */
	.statesize = 1,
	.base = {
	    .cra_name		=	"hmac(md5)",
	    .cra_driver_name	=	"hmac-md5-spu",
	    .cra_priority       =	SPU_CRA_PRIORITY,
	    .cra_flags		=	CRYPTO_ALG_TYPE_AHASH |
                                        CRYPTO_ALG_ASYNC,
	    .cra_blocksize      =	SPU_MD5_HAMC_BLOCK_SIZE,
	    .cra_ctxsize        =	sizeof(struct spu_ctx),
	    .cra_module         =	THIS_MODULE,
	    .cra_init           =	spu_ahash_cra_init,
	    .cra_exit           =	spu_ahash_cra_exit,
	}
    }
};

struct ahash_alg spu_sha1_alg = {
    .init           = 	spu_ahash_sha1_init,
    .update         =	spu_ahash_sha1_update,
    .final          =	spu_ahash_sha1_final,
    .finup          =	spu_ahash_sha1_finup,
    .digest         =	spu_ahash_sha1_digest,
    .export         =	spu_ahash_sha1_export,
    .import         =	spu_ahash_sha1_import,
    .setkey         =	spu_ahash_sha1_setkey,
    .halg           =	{
	.digestsize	=	SHA1_DIGEST_SIZE,
	/* import/export not supported so statesize is not required
	   However, it needs to be set to pass ahash_prepare_alg */
	.statesize	=	1,
	.base  = {
	    .cra_name           =	"hmac(sha1)",
	    .cra_driver_name    =	"hamc-sha1-spu",
	    .cra_priority       =	SPU_CRA_PRIORITY,
	    .cra_flags		=	CRYPTO_ALG_TYPE_AHASH | 
                                        CRYPTO_ALG_ASYNC,
	    .cra_blocksize      =	SHA1_BLOCK_SIZE,
	    .cra_ctxsize        =	sizeof(struct spu_ctx),
	    .cra_module         =	THIS_MODULE,
	    .cra_init           =	spu_ahash_cra_init,
	    .cra_exit           =	spu_ahash_cra_exit,
	}
    }
};

int spu_hash_register(void)
{
    int rc;
    struct spu_info *spuinfo = pdev_ctrl->spu_linux;

    /* flags are modified when algs are unregistered */
    spu_md5_alg.halg.base.cra_flags = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC;
    rc = crypto_register_ahash(&spu_md5_alg);
    if(rc)
    {
        printk(KERN_ERR "%s registration failed with error %d\n", spu_md5_alg.halg.base.cra_driver_name, rc);
    }
    else
    {
        spuinfo->md5_registered = 1;
    }
    
    /* flags are modified when algs are unregistered */
    spu_sha1_alg.halg.base.cra_flags = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC;
    rc = crypto_register_ahash(&spu_sha1_alg);
    if(rc)
    {
        printk(KERN_ERR "%s registration failed with error %d\n", spu_sha1_alg.halg.base.cra_driver_name, rc);
    }
    else
    {
        spuinfo->sha1_registered = 1;
    }

    return rc;
} /* spu_hash_register */

int spu_hash_unregister(void)
{
    struct spu_info *spuinfo = pdev_ctrl->spu_linux;
    if ( (spuinfo->md5_registered) && atomic_read(&spu_md5_alg.halg.base.cra_refcnt) != 1 )
    {
       printk("reference to %s still held\n", spu_md5_alg.halg.base.cra_driver_name);
       return -1;
    }

    if ( (spuinfo->sha1_registered) && atomic_read(&spu_sha1_alg.halg.base.cra_refcnt) != 1 )
    {
       printk("reference to %s still held\n", spu_sha1_alg.halg.base.cra_driver_name);
       return -1;
    }

    if (spuinfo->md5_registered)
    {
        crypto_unregister_ahash(&spu_md5_alg);
        spuinfo->md5_registered = 0;
    }

    if (spuinfo->sha1_registered)
    {
       crypto_unregister_ahash(&spu_sha1_alg);
       spuinfo->sha1_registered = 0;
    }

    return 0;
} /* spu_hash_unregister */
