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

#include <linux/err.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/crypto.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/rtnetlink.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/dma-direction.h>

#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/md5.h>
#include <crypto/authenc.h>
#include <crypto/skcipher.h>
#include <crypto/hash.h>
#include <crypto/scatterwalk.h>
#include <crypto/aes.h>
#include <linux/nbuff.h>

#include "util.h"
#include "spu.h"
#include "cipher.h"
#include "spu_blog.h"

int pdc_send_data(int chan, struct brcm_message *msg);
int pdc_select_channel(void);

/* ================= Device Structure ================== */

struct spu_private iproc_priv;

/* ==================== Parameters ===================== */

int flow_debug_logging = 0;
int packet_debug_logging = 0;
int debug_logging_sleep = 0;
int givencrypt_test_mode = 0;

module_param(flow_debug_logging, int, 0644);
MODULE_PARM_DESC(flow_debug_logging, "Enable Flow Debug Logging");

module_param(packet_debug_logging, int, 0644);
MODULE_PARM_DESC(packet_debug_logging, "Enable Packet Debug Logging");

module_param(debug_logging_sleep, int, 0644);
MODULE_PARM_DESC(debug_logging_sleep, "Packet Debug Logging Sleep");

module_param(givencrypt_test_mode, int, 0644);
MODULE_PARM_DESC(givencrypt_test_mode, "Turn off givencrypt rnd IV generation");

#if defined(CONFIG_BLOG)
static void aead_configure_blog( int enable);

int blog_enable = 1;  /* Enable blogging for SPU by default
                         set to 0 to disable or at runtime via 
                         spu_blog_enable script */

static int spu_blog_enable_set(const char *val, const struct kernel_param *kp)
{
	int ret;

	/* sets mtu_max directly. no need to restore it in case of
	 * illegal value since we assume this will fail insmod
	 */
	ret = param_set_int(val, kp);
	if (ret)
	{
		return ret;
	}

	aead_configure_blog(blog_enable);

	return ret;
}

static struct kernel_param_ops blog_enable_ops = {
	.set = spu_blog_enable_set,
	.get = param_get_int,
};

module_param_cb(blog_enable, &blog_enable_ops, &blog_enable, 0644);
MODULE_PARM_DESC(blog_enable, "Enable/Disable SPU blogging");
#endif

/* ==================== Queue Tasks and Helpers ==================== */

static int handle_ablkcipher_req(struct iproc_reqctx_s *rctx);
static void handle_ablkcipher_resp(struct iproc_reqctx_s *rctx);
static int handle_ahash_req(struct iproc_reqctx_s *rctx);
static void handle_ahash_resp(struct iproc_reqctx_s *rctx);
static int ahash_req_done(struct iproc_reqctx_s *rctx);
static void spu_rx_callback(struct brcm_message *msg);

/* finish_req() is used to notify that the current request has been completed */
static void finish_req(struct iproc_reqctx_s *rctx, int err);


/* Build up the scatterlist of buffers used to receive a SPU response message
 * for an ablkcipher request. Includes buffers to catch SPU message headers
 * and the response data.
 * Inputs:
 *   mssg - message containing the receive sg
 *   rctx - crypto request context
 *   rx_frag_num - number of scatterlist elements required to hold the
 *                 SPU response message
 *   chunksize - Number of bytes of response data expected
 *   stat_pad_len - Number of bytes required to pad the STAT field to
 *		    a 4-byte boundary
 * Returns:
 *   0 if successful
 *   < 0 if an error
 */
static int
spu_ablkcipher_rx_sg_create(struct brcm_message *mssg,
			    struct iproc_reqctx_s *rctx,
			    u8 rx_frag_num,
			    unsigned chunksize,
			    u32 stat_pad_len)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct scatterlist *sg;     /* used to build sgs in message */
	struct iproc_ctx_s *ctx = rctx->ctx;
	u32 datalen;           /* Number of bytes of response data expected */
	u16 nent;              /* number of sg entries in DMA mapping */

	if (rx_frag_num > MAX_STATIC_FRAGS) {
		mssg->dst = kcalloc(rx_frag_num, sizeof(struct scatterlist),
		                       GFP_KERNEL);
		if (unlikely(mssg->dst == NULL)) {
			dev_err(dev, "%s(): Failed to allocate dst sg", __func__);
			return -ENOMEM;
		}
	}
	else
	{
		mssg->dst = rctx->msg_dst;
	}

	sg = mssg->dst;
	sg_init_table(sg, rx_frag_num);
	/* Space for SPU message header */
	sg_set_buf(sg++, rctx->pctx.spu_resp_hdr, SPU_RESP_HDR_LEN);

	/* If XTS tweak in payload, add buffer to receive encrypted tweak */
	if (ctx->cipher.mode == CIPHER_MODE_XTS) {
		sg_set_buf(sg++, rctx->pctx.supdt, SPU_XTS_TWEAK_SIZE);
	}

	/* Copy in each dst sg entry from request, up to chunksize */
	datalen = spu_msg_sg_add(&sg, &rctx->dst_sg, &rctx->dst_skip,
				 rctx->dst_nents, chunksize);
	if (datalen < chunksize) {
		dev_err(dev,
			"%s(): failed to copy dst sg to msg. chunksize %u, datalen %u",
			__func__, chunksize, datalen);
		return -EFAULT;
	}

	if (ctx->cipher.alg == CIPHER_ALG_RC4)
		/* Add buffer to catch 260-byte SUPDT field for RC4 */
		sg_set_buf(sg++, rctx->pctx.supdt, SPU_SUPDT_LEN);

	if (stat_pad_len)
		sg_set_buf(sg++, rctx->pctx.rx_stat_pad, stat_pad_len);

	sg_set_buf(sg, rctx->pctx.rx_stat, SPU_STATUS_LEN);

	nent = dma_map_sg(dev, mssg->dst, sg_nents(mssg->dst), DMA_BIDIRECTIONAL);
	if (nent == 0) {
		dev_err(dev, "%s(): Failed to DMA map dst sg with %u frags",
			__func__, rx_frag_num);
		return -EIO;
	}
	return 0;
}

/* Build up the scatterlist of buffers used to send a SPU request message
 * for an ablkcipher request. Includes SPU message headers and the request
 * data.
 *
 * Inputs:
 *   mssg - message containing the transmit sg
 *   rctx - crypto request context
 *   tx_frag_num - number of scatterlist elements required to construct the
 *                 SPU request message
 *   chunksize - Number of bytes of request data
 *   pad_len - Number of pad bytes
 * Returns:
 *   0 if successful
 *   < 0 if an error
 */
static int
spu_ablkcipher_tx_sg_create(struct brcm_message *mssg,
			    struct iproc_reqctx_s *rctx,
			    u8 tx_frag_num,
			    unsigned chunksize,
			    u32 pad_len)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct scatterlist *sg;     /* used to build sgs in message */
	u32 datalen;           /* Number of bytes of response data expected */
	u16 nent;              /* number of sg entries in DMA mapping */
	u32 msg_len = 0;       /* length of SPU request in bytes  */

	if (tx_frag_num > MAX_STATIC_FRAGS) {
		mssg->src = kcalloc(tx_frag_num, sizeof(struct scatterlist),
		                       GFP_KERNEL);
		if (unlikely(mssg->src == NULL)) {
			dev_err(dev, "%s(): Failed to allocate src sg", __func__);
			return -ENOMEM;
		}
	}
	else
	{
		mssg->src = rctx->msg_src;
	}

	sg = mssg->src;
	sg_init_table(sg, tx_frag_num);

	sg_set_buf(sg++, rctx->pctx.bcm_spu_req_hdr, rctx->pctx.spu_req_hdr_len);
	msg_len += rctx->pctx.spu_req_hdr_len;

	/* if XTS tweak in payload, copy from IV (where crypto API puts it) */
	if (rctx->ctx->cipher.mode == CIPHER_MODE_XTS) {
		sg_set_buf(sg++, rctx->iv_ctr, SPU_XTS_TWEAK_SIZE);
	}

	/* Copy in each src sg entry from request, up to chunksize */
	datalen = spu_msg_sg_add(&sg, &rctx->src_sg, &rctx->src_skip,
				 rctx->src_nents, chunksize);
	if (unlikely(datalen < chunksize)) {
		dev_err(dev, "%s(): failed to copy src sg to msg",
			__func__);
		return -EFAULT;
	}
	msg_len += datalen;

	if (pad_len)
	{
		sg_set_buf(sg++, rctx->pctx.spu_req_pad, pad_len);
		msg_len += pad_len;
	}

	*rctx->pctx.tx_stat = 0;
	sg_set_buf(sg, rctx->pctx.tx_stat, SPU_STATUS_LEN);
	msg_len += SPU_STATUS_LEN;

	if (unlikely(msg_len >= MAX_SPU_PKT_SIZE)) {
		dev_err(dev,
			"SPU message for block cipher too big. Length %u. Max %u.\n",
			msg_len, MAX_SPU_PKT_SIZE);
		return -EFAULT;
	}

	nent = dma_map_sg(dev, mssg->src, sg_nents(mssg->src), DMA_BIDIRECTIONAL);
	if (unlikely(nent == 0)) {
		dev_err(dev, "%s(): Failed to DMA map dst sg with %u frags",
			__func__, tx_frag_num);
		return -EIO;
	}
	return 0;
}

/*
 * Submit as much of a block cipher request as fits in a single SPU request
 * message, starting at the current position in the request data. This may
 * be called on the crypto API thread, or, when a request is so large it
 * must be broken into multiple SPU messages, on the thread used to invoke
 * the response callback. When requests are broken into multiple SPU
 * messages, we assume subsequent messages depend on previous results, and
 * thus always wait for previous results before submitting the next message.
 * Because requests are submitted in lock step like this, there is no need
 * to synchronize access to request data structures.
 */
static int
handle_ablkcipher_req(struct iproc_reqctx_s *rctx)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct crypto_async_request *areq = rctx->parent;
	struct ablkcipher_request *req =
		container_of(areq, struct ablkcipher_request, base);
	struct iproc_ctx_s *ctx = rctx->ctx;
	int err = 0;
	unsigned chunksize;   /* Number of bytes of request to submit */
	int remaining;        /* Bytes of request still to process */
	int chunk_start;      /* Beginning of data for current SPU msg */
	u32 db_size;
	u32 stat_pad_len;         /* num bytes to align status field */
	bool update_key = false;
	u8 *enckey = ctx->enckey;
	enum spu_cipher_type cipher_type = ctx->cipher_type;
	u8 * iv_ptr;
	u32 data_size;

	/* number of entries in src and dst sg. Always includes SPU msg header
	 * and STAT.
	 */
	u8 rx_frag_num = 2;
	u8 tx_frag_num = 2;

	flow_log("%s\n", __func__);

	chunk_start = rctx->total_sent;
	remaining = rctx->total_todo - chunk_start;

	/* determine the chunk we are breaking off and update the indicies */
	chunksize = (remaining > ctx->max_payload) ? ctx->max_payload : remaining;
	rctx->total_sent += chunksize;

	/* Count number of sg entries to be included in this request */
	rctx->src_nents = spu_sg_count(rctx->src_sg, chunksize, rctx->src_skip);
	rctx->dst_nents = spu_sg_count(rctx->dst_sg, chunksize, rctx->dst_skip);

	if ((ctx->cipher.mode == CIPHER_MODE_CBC) &&
	    rctx->is_encrypt && chunk_start) {
		/* Encrypting non-first first chunk. Copy last block of
		 * previous result to IV for this chunk.
		 */
		sg_copy_part_to_buf(req->dst, rctx->iv_ctr,
		                    rctx->iv_ctr_len,
		                    rctx->dst_skip - rctx->iv_ctr_len);
	}

	/* Use SUPDT field as key. Key field in finish() call is only used
	 * when update_key has been set. Will be ignored in all other cases.
	 */
	if (ctx->cipher.alg == CIPHER_ALG_RC4) {
		rx_frag_num++;
		if (chunk_start) {
			/* for non-first RC4 chunks, use SUPDT from previous
			 * response as key for this chunk.
			 */
			enckey = rctx->pctx.supdt;
			update_key = true;
			cipher_type = CIPHER_TYPE_UPDT;
		} else if (!rctx->is_encrypt) {
			/* First RC4 chunk. For decrypt, key in pre-built msg
			 * header may have been changed if encrypt required
			 * multiple chunks. So revert the key to the
			 * ctx->enckey value.
			 */
			update_key = true;
			cipher_type = CIPHER_TYPE_INIT;
		}
	}

	flow_log("%s()-send req:%p rctx:%p ctx:%p\n", __func__, req, rctx, ctx);
	flow_log("max_payload:%u sent:%u start:%u remains:%u size:%u\n",
		 ctx->max_payload, rctx->total_sent, chunk_start, remaining,
		 chunksize);

	/* In XTS mode, API puts "i" parameter (block tweak) in IV. This 
	 * should be at the start of the BD. The 'j' parameter (block ctr
	 * within larger unit) should be passed as the iv. This can be 0 
	 * since an entire block is handled in one transaction */
	if (ctx->cipher.mode == CIPHER_MODE_XTS) {
		data_size = chunksize + rctx->iv_ctr_len;
		memset(rctx->pctx.supdt, 0, rctx->iv_ctr_len);
		iv_ptr = rctx->pctx.supdt;
		/* extra sg for tx and rx for xts tweak */
		rx_frag_num++;
		tx_frag_num++;
	}
	else {
		data_size = chunksize;
		iv_ptr = rctx->iv_ctr;
	}
	spu_cipher_req_finish(rctx->pctx.bcm_spu_req_hdr,
			      rctx->pctx.spu_req_hdr_len,
			      !(rctx->is_encrypt),
			      ctx->cipher.alg, cipher_type,
			      enckey, ctx->enckeylen, update_key,
			      iv_ptr, rctx->iv_ctr_len,
			      data_size);

	/* iv has been copied to header, update it for the next request
	   if there is still data to send */
	if (rctx->total_todo - rctx->total_sent)
	{
		if ((ctx->cipher.mode == CIPHER_MODE_CBC) &&
		    (!rctx->is_encrypt))
		{
			/* CBC Decrypt: next IV is the last ciphertext block in
			 * this chunk */
			sg_copy_part_to_buf(req->src, rctx->iv_ctr, rctx->iv_ctr_len,
			                    rctx->src_skip + chunksize - rctx->iv_ctr_len);
			packet_dump("    CBC IV next: ", rctx->iv_ctr, rctx->iv_ctr_len);
		}
		else if (ctx->cipher.mode == CIPHER_MODE_CTR)
		{
			/* CTR mode, increment counter for next block. Assumes 16-byte
			 * block (AES).  SPU does not support CTR mode for DES/3DES.
			 */
			add_to_ctr(rctx->iv_ctr, chunksize >> 4 );
			packet_dump("    CTR IV next: ", rctx->iv_ctr, rctx->iv_ctr_len);
		}
	}

	atomic_long_add(chunksize, &iproc_priv.bytes_out);

	db_size = spu_real_db_size(0, 0, 0, chunksize, 0, 0, 0);
	stat_pad_len = spu_wordalign_padlen(db_size);
	if (stat_pad_len) {
		rx_frag_num++;
		tx_frag_num++;
		spu_request_pad(rctx->pctx.spu_req_pad, 0, 0,
				ctx->auth.alg, rctx->total_sent, stat_pad_len);
	}

	dump_spu_msg_hdr(rctx->pctx.bcm_spu_req_hdr,
			 rctx->pctx.spu_req_hdr_len);
	packet_log("BD:\n");
	if (rctx->ctx->cipher.mode == CIPHER_MODE_XTS)
	{
		packet_dump("  sg:", rctx->iv_ctr, rctx->iv_ctr_len);
	}
	else
	{
		packet_log("BD:\n");
	}
	dump_sg(rctx->src_sg, rctx->src_skip, chunksize);
	packet_dump("   pad: ", rctx->pctx.spu_req_pad, stat_pad_len);

	INIT_LIST_HEAD(&rctx->mb_mssg.list);
	rctx->mb_mssg.error = 0;
	/* Create rx scatterlist to catch result */
	rx_frag_num += rctx->dst_nents;
	err = spu_ablkcipher_rx_sg_create(&rctx->mb_mssg, rctx, rx_frag_num, chunksize,
					  stat_pad_len);
	if (err)
		return err;

	/* Create tx scatterlist containing SPU request message */
	tx_frag_num += rctx->src_nents;
	err = spu_ablkcipher_tx_sg_create(&rctx->mb_mssg, rctx, tx_frag_num, chunksize,
					  stat_pad_len);
	if (err)
		return err;

	err = pdc_send_data(ctx->chan_idx, &rctx->mb_mssg);
	if (err < 0) {
		atomic_inc(&iproc_priv.mb_send_fail);
		dev_dbg(dev, "%s(): Failed to send message. err %d.",
			__func__, err);
		return err;
	}

	return -EINPROGRESS;
}

/*
 * Process a block cipher SPU response. Unmaps the scatterlists in the
 * message used to submit the request. Updates the total received count for
 * the request and updates global stats.
 */
static void handle_ablkcipher_resp(struct iproc_reqctx_s *rctx)
{
#ifdef DEBUG
	struct crypto_async_request *areq = rctx->parent;
	struct ablkcipher_request *req = ablkcipher_request_cast(areq);
#endif
	struct iproc_ctx_s *ctx = rctx->ctx;
	struct BD_HEADER *bd_hdr;
	u16 bd_len;

	/* skip MH, EMH headers */
	bd_hdr = (struct BD_HEADER *) (rctx->pctx.spu_resp_hdr + 8);
	bd_len = ntohs(bd_hdr->size);

	/* In XTS mode, the first SPU_XTS_TWEAK_SIZE bytes may be the
	 * encrypted tweak ("i") value; we don't count that.
	 */
	if ((ctx->cipher.mode == CIPHER_MODE_XTS) && 
	    (bd_len >= SPU_XTS_TWEAK_SIZE))
	{
		bd_len -= SPU_XTS_TWEAK_SIZE;
	}

	atomic_long_add(bd_len, &iproc_priv.bytes_in);

	flow_log("%s() rctx:%p  offset: %u, bd_len: %u BD:\n",
		 __func__, rctx, rctx->total_received, bd_len);

	dump_sg(req->dst, rctx->total_received, bd_len);
	if (ctx->cipher.alg == CIPHER_ALG_RC4)
		packet_dump("  supdt ", rctx->pctx.supdt, SPU_SUPDT_LEN);

	rctx->total_received += bd_len;
#ifdef SPU_DEBUG_STATS 
	if (rctx->total_received == rctx->total_todo)
		atomic_inc(&iproc_priv.op_counts[SPU_OP_CIPHER]);
#endif
}

/* Build up the scatterlist of buffers used to receive a SPU response message
 * for an ahash request.
 * Inputs:
 *   mssg - message containing the receive sg
 *   rctx - crypto request context
 *   rx_frag_num - number of scatterlist elements required to hold the
 *                 SPU response message
 *   stat_pad_len - Number of bytes required to pad the STAT field to
 *		    a 4-byte boundary
 * Returns:
 *   0 if successful
 *   < 0 if an error
 */
static int
spu_ahash_rx_sg_create(struct brcm_message *mssg,
		       struct iproc_reqctx_s *rctx,
		       u8 rx_frag_num,
		       u32 stat_pad_len,
		       u32 digestsize)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct scatterlist *sg;     /* used to build sgs in message */
	u16 nent;              /* number of sg entries in DMA mapping */

	if (rx_frag_num > MAX_STATIC_FRAGS) {
		mssg->dst = kcalloc(rx_frag_num, sizeof(struct scatterlist),
		                       GFP_KERNEL);
		if (unlikely(mssg->dst == NULL)) {
			dev_err(dev, "%s(): Failed to allocate dst sg", __func__);
			return -ENOMEM;
		}
	}
	else
	{
		mssg->dst = rctx->msg_dst;
	}

	sg = mssg->dst;
	sg_init_table(sg, rx_frag_num);
	/* Space for SPU message header */
	sg_set_buf(sg++, rctx->pctx.spu_resp_hdr, SPU_HASH_RESP_HDR_LEN);

	sg_set_buf(sg++, rctx->pctx.digest, digestsize);

	if (stat_pad_len)
		sg_set_buf(sg++, rctx->pctx.rx_stat_pad, stat_pad_len);

	sg_set_buf(sg, rctx->pctx.rx_stat, SPU_STATUS_LEN);

	nent = dma_map_sg(dev, mssg->dst, sg_nents(mssg->dst), DMA_BIDIRECTIONAL);
	if (nent == 0) {
		dev_err(dev, "%s(): Failed to DMA map dst sg with %u frags",
			__func__, rx_frag_num);
		return -EIO;
	}
	return 0;
}

/* Build up the scatterlist of buffers used to send a SPU request message
 * for an ahash request. Includes SPU message headers and the request
 * data.
 *
 * Inputs:
 *   mssg - message containing the transmit sg
 *   rctx - crypto request context
 *   tx_frag_num - number of scatterlist elements required to construct the
 *		   SPU request message
 *   spu_hdr_len - length in bytes of SPU message header
 *   hash_carry_len - Number of bytes of data carried over from previous req
 *   new_data_len - Number of bytes of new request data
 *   pad_len - Number of pad bytes
 * Returns:
 *   0 if successful
 *   < 0 if an error
 */
static int
spu_ahash_tx_sg_create(struct brcm_message *mssg,
		       struct iproc_reqctx_s *rctx,
		       u8 tx_frag_num,
		       u32 spu_hdr_len,
		       unsigned hash_carry_len,
		       unsigned new_data_len,
		       u32 pad_len)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct scatterlist *sg;     /* used to build sgs in message */
	u32 datalen;           /* Number of bytes of response data expected */
	u16 nent;              /* number of sg entries in DMA mapping */
	u32 msg_len = 0;       /* length of SPU request in bytes  */

	if (tx_frag_num > MAX_STATIC_FRAGS) {
		mssg->src = kcalloc(tx_frag_num, sizeof(struct scatterlist),
		                       GFP_KERNEL);
		if (unlikely(mssg->src == NULL)) {
			dev_err(dev, "%s(): Failed to allocate src sg", __func__);
			return -ENOMEM;
		}
		sg = mssg->src;
	}
	else
	{
		mssg->src = rctx->msg_src;
	}

	sg = mssg->src;
	sg_init_table(sg, tx_frag_num);

	sg_set_buf(sg++, rctx->pctx.bcm_spu_req_hdr, spu_hdr_len);
	msg_len += spu_hdr_len;

	if (hash_carry_len) {
		sg_set_buf(sg++, rctx->hash_carry, hash_carry_len);
		msg_len += hash_carry_len;
	}

	if (new_data_len) {
		/* Copy in each src sg entry from request, up to chunksize */
		datalen = spu_msg_sg_add(&sg, &rctx->src_sg, &rctx->src_skip,
					 rctx->src_nents,
					 new_data_len);
		if (datalen < new_data_len) {
			dev_err(dev,
				"%s(): failed to copy src sg to msg",
				__func__);
			return -EFAULT;
		}
		msg_len += datalen;
	}

	if (pad_len) {
		sg_set_buf(sg++, rctx->pctx.spu_req_pad, pad_len);
		msg_len += pad_len;
	}

	*rctx->pctx.tx_stat = 0;
	sg_set_buf(sg, rctx->pctx.tx_stat, SPU_STATUS_LEN);
	msg_len += SPU_STATUS_LEN;

	if (unlikely(msg_len >= MAX_SPU_PKT_SIZE)) {
		dev_err(dev,
			"SPU message for ahash too big. Length %u. Max %u.\n",
			msg_len, MAX_SPU_PKT_SIZE);
		return -EFAULT;
	}

	nent = dma_map_sg(dev, mssg->src, sg_nents(mssg->src), DMA_BIDIRECTIONAL);
	if (nent == 0) {
		dev_err(dev, "%s(): Failed to DMA map dst sg with %u frags",
			__func__, tx_frag_num);
		return -EIO;
	}
	return 0;
}

/* Process an asynchronous hash request from the crypto API. Builds a SPU
 * request message and submits on the selected channel. The message is
 * constructed as a scatterlist, including entries from the crypto API's
 * src scatterlist to avoid copying the data to be hashed. This function is
 * called either on the thread from the crypto API, or, in the case that the
 * crypto API request is too large to fit in a single SPU request message,
 * on the thread that invokes the receive callback with a response message.
 * Because some operations require the response from one chunk before the next
 * chunk can be submitted, we always wait for the response for the previous
 * chunk before submitting the next chunk. Because requests are submitted in
 * lock step like this, there is no need to synchronize access to request data
 * structures.
 * Returns:
 *   -EINPROGRESS: request has been submitted to SPU and response will be
 *		   returned asynchronously
 *   -EAGAIN:      non-final request included a small amount of data, which for
 *		   efficiency we did not submit to the SPU, but instead stored
 *		   to be submitted to the SPU with the next part of the request
 *   other:        an error code
 */
static int
handle_ahash_req(struct iproc_reqctx_s *rctx)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct crypto_async_request *areq = rctx->parent;
	struct ahash_request *req = ahash_request_cast(areq);
	struct iproc_ctx_s *ctx = rctx->ctx;
	u8 *authkey = (u8 *) ctx->authkey;
	u32 authkeylen = ctx->authkeylen;
	u32 digestsize;
	u32 statesize;
	u32 nbytes_to_hash;
	enum hash_type hash_type;
	int err;
	u32 chunksize;      /* length of hash carry + new data */
	u32 db_size;        /* Length of data field */
	u32 stat_pad_len;   /* length of padding to align STATUS word */
	u32 local_nbuf;
	u8 rx_frag_num = 3;
	u8 tx_frag_num = 2;
	u32 prevBlocks;

	flow_log("total_todo %u, total_sent %u\n",
		 rctx->total_todo, rctx->total_sent);

	/* compute the amount remaining to hash. This may include data
	 * carried over from previous requests.
	 */
	nbytes_to_hash = rctx->total_todo - rctx->total_sent;
	chunksize = min(nbytes_to_hash, ctx->max_payload);

	if ( !rctx->is_final )
	{
		/* don't carry over more than max_payload bytes */
		u32 max_carry = (HASH_CARRY_MAX > ctx->max_payload) ?
		                 ctx->max_payload : HASH_CARRY_MAX;
		if (nbytes_to_hash < max_carry)
		{
			/* this is not the final hash and there is less data than
			   we want to submit so carry it over */
			sg_copy_part_to_buf(req->src,
			                    rctx->hash_carry + rctx->hash_carry_len,
			                    nbytes_to_hash - rctx->hash_carry_len,
			                    rctx->src_skip);
			rctx->hash_carry_len = nbytes_to_hash;

			flow_log("  Exiting with stored remnant. hash_carry_len: %u\n",
			         rctx->hash_carry_len);
			packet_dump("  buf: ", rctx->hash_carry, rctx->hash_carry_len);
			return -EAGAIN;
		}
		else
		{
			/* there is more data than we can carry over
			   adjust chunksize to block size and hash */
			chunksize &= ~(ctx->blocksize - 1);
		}
	}

	/* if we have hash carry, then prefix it to the data in this request */
	local_nbuf = rctx->hash_carry_len;
	rctx->hash_carry_len = 0;
	if (local_nbuf)
		tx_frag_num++;

	/* Count number of sg entries to be used in this request */
	rctx->src_nents = spu_sg_count(rctx->src_sg, chunksize - local_nbuf,
	                               rctx->src_skip);

	/* determine hash type */
	prevBlocks = 0;
	hash_type = (rctx->total_sent) ? HASH_TYPE_UPDT : HASH_TYPE_INIT;
	if ((rctx->total_sent + chunksize) == rctx->total_todo) {
		/* last chunk */
		if (rctx->is_final)
		{
			if ( rctx->total_sent )
			{
				hash_type = HASH_TYPE_FIN;
				prevBlocks = rctx->total_sent / ctx->blocksize;
			}
			else
			{
				hash_type = HASH_TYPE_FULL;
			}
		}
	}
	rctx->total_sent += chunksize;

	if (ctx->auth.alg == HASH_ALG_SHA224)
	{
		/* state size is SHA256_DIGEST_SIZE */
		statesize = SHA256_DIGEST_SIZE;
	}
	else if (ctx->auth.alg == HASH_ALG_SHA384)
	{
		/* state size is SHA512_DIGEST_SIZE */
		statesize = SHA512_DIGEST_SIZE;
	}
	else
	{
		statesize = ctx->digestsize;
	}

	if ( (hash_type == HASH_TYPE_INIT) ||
	     (hash_type == HASH_TYPE_UPDT) )
	{
		/* UPDT and INIT provide state, not digest */
		digestsize = statesize;
	}
	else
	{
		digestsize = ctx->digestsize;
	}

	if ((hash_type == HASH_TYPE_UPDT) || (hash_type == HASH_TYPE_FIN)) {
		authkey = rctx->pctx.digest;
		authkeylen = statesize;
	}

	atomic_long_add(chunksize, &iproc_priv.bytes_out);

	flow_log("%s() final: %u max_payload: %u nbuf: %u chunk_size: %u\n",
		 __func__, rctx->is_final, ctx->max_payload, local_nbuf,
		chunksize);

	/* Determine total length of padding required. Put all padding in one
	 * buffer.
	 */
	db_size = spu_real_db_size(0, 0, local_nbuf, chunksize - local_nbuf,
				   0, 0, 0);
	stat_pad_len = spu_wordalign_padlen(db_size);
	if (stat_pad_len)
	{
		rx_frag_num++;
		tx_frag_num++;
		spu_request_pad(rctx->pctx.spu_req_pad, 0, 0,
				ctx->auth.alg, rctx->total_sent,
				stat_pad_len);
	}

	rctx->pctx.spu_req_hdr_len = spu_hash_req_create(
	                                  rctx->pctx.bcm_spu_req_hdr,
	                                  ctx->auth.alg,
	                                  ctx->auth.mode,
	                                  hash_type,
	                                  digestsize,
	                                  authkey,
	                                  authkeylen,
	                                  db_size,
	                                  prevBlocks,
	                                  chunksize + stat_pad_len);

	if (rctx->pctx.spu_req_hdr_len == 0) {
		pr_err("Failed to create SPU request header\n");
		return -EFAULT;
	}

	dump_spu_msg_hdr(rctx->pctx.bcm_spu_req_hdr, rctx->pctx.spu_req_hdr_len);
	packet_dump("  prebuf: ", rctx->hash_carry, local_nbuf);
	flow_log("Data: chunksize %d, local_nbuf %d\n", chunksize, local_nbuf);
	dump_sg(rctx->src_sg, rctx->src_skip, chunksize - local_nbuf);
	packet_dump("  pad: ", rctx->pctx.spu_req_pad, stat_pad_len);

	INIT_LIST_HEAD(&rctx->mb_mssg.list);
	rctx->mb_mssg.error = 0;
	/* Create rx scatterlist to catch result */
	err = spu_ahash_rx_sg_create(&rctx->mb_mssg, rctx, rx_frag_num, stat_pad_len,
	                             digestsize);
	if (err)
		return err;

	/* Create tx scatterlist containing SPU request message */
	tx_frag_num += rctx->src_nents;
	err = spu_ahash_tx_sg_create(&rctx->mb_mssg, rctx, tx_frag_num,
	                             rctx->pctx.spu_req_hdr_len,
	                             local_nbuf, chunksize - local_nbuf,
	                             stat_pad_len);
	if (err)
		return err;

	err = pdc_send_data(ctx->chan_idx, &rctx->mb_mssg);
	if (err < 0) {
		atomic_inc(&iproc_priv.mb_send_fail);
		dev_dbg(dev, "%s(): Failed to send message. err %d.",
			__func__, err);
		return err;
	}

	return -EINPROGRESS;
}

/* Process a SPU response message for a hash request. Unmaps the scatterlists
 * in the message used to submit the request. Checks if the entire
 * crypto API request has been processed, and if so, invokes post processing
 * on the result.
 * Returns:
 *   0 if successful
 *   < 0 otherwise
 */
static void handle_ahash_resp(struct iproc_reqctx_s *rctx)
{
	struct iproc_ctx_s *ctx = rctx->ctx;
#ifdef DEBUG
	struct crypto_async_request *areq = rctx->parent;
	struct ahash_request *req = ahash_request_cast(areq);
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	unsigned blocksize = crypto_tfm_alg_blocksize(crypto_ahash_tfm(ahash));
#endif

	flow_log("%s() req:%p blocksize:%u digestsize:%u\n",
		 __func__, req, blocksize, ctx->digestsize);

	atomic_long_add(ctx->digestsize, &iproc_priv.bytes_in);

	/* if this is the final hash and we have processed all the data then
	   finish the request */
	if (rctx->is_final && (rctx->total_sent == rctx->total_todo))
		ahash_req_done(rctx);
	else
		flow_dump("  state ", rctx->pctx.digest, ctx->digestsize);
}

/* Do whatever processing is required after the entire hash request
 * has been processed through the SPU hardware.
 */
static int ahash_req_done(struct iproc_reqctx_s *rctx)
{
	struct crypto_async_request *areq = rctx->parent;
	struct ahash_request *req = ahash_request_cast(areq);
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	struct iproc_ctx_s *ctx = crypto_ahash_ctx(ahash);

	if (NULL == req->result)
	{
		pr_err("%s() Error: no result buffer provided\n", __func__);
		return -EINVAL;
	}

	/* tcrypt ahash speed tests provide a stack-allocated result buffer
	 * that causes the DMA mapping to crash. So we have to copy the
	 * result here.
	 */
	memcpy(req->result, rctx->pctx.digest, ctx->digestsize);

	flow_dump("  digest ", req->result, ctx->digestsize);

	/* if this an HMAC then do the outer hash */
	if (rctx->is_sw_hmac) {
		switch (ctx->auth.alg) {
		case HASH_ALG_MD5:
			do_shash("md5", req->result, ctx->opad, ctx->blocksize,
				 req->result, ctx->digestsize);
			break;
		case HASH_ALG_SHA1:
			do_shash("sha1", req->result, ctx->opad, ctx->blocksize,
				 req->result, ctx->digestsize);
			break;
		case HASH_ALG_SHA224:
			do_shash("sha224", req->result, ctx->opad, ctx->blocksize,
				 req->result, ctx->digestsize);
			break;
		case HASH_ALG_SHA256:
			do_shash("sha256", req->result, ctx->opad, ctx->blocksize,
				 req->result, ctx->digestsize);
			break;
		case HASH_ALG_SHA384:
			do_shash("sha384", req->result, ctx->opad, ctx->blocksize,
					 req->result, ctx->digestsize);
			break;
		case HASH_ALG_SHA512:
			do_shash("sha512", req->result, ctx->opad, ctx->blocksize,
					 req->result, ctx->digestsize);
			break;
		default:
			pr_err("%s() Error : unknown hmac type\n",
			       __func__);
			return -EINVAL;
		}

		flow_dump("  hmac: ", req->result, ctx->digestsize);
	}

#ifdef SPU_DEBUG_STATS 
	if (rctx->is_sw_hmac)
		atomic_inc(&iproc_priv.op_counts[SPU_OP_HMAC]);
	else
		atomic_inc(&iproc_priv.op_counts[SPU_OP_HASH]);
#endif
	return 0;
}

static unsigned spu_dtls_hmac_offset(struct iproc_reqctx_s *rctx,
                                     unsigned int chunksize, 
                                     unsigned int blocksize)
{
	struct iproc_ctx_s *ctx = rctx->ctx;

	unsigned hmac_offset;
	u16 swap_hmac_offset = 0;

	if (!rctx->is_encrypt) {
		char iv_buf[MAX_IV_SIZE];
		char src_buf[MAX_IV_SIZE];
		char dest_buf[MAX_IV_SIZE];
		unsigned i;
		char *alg_name = spu_alg_name(ctx->cipher.alg,
					      ctx->cipher.mode);

		switch (ctx->cipher.mode) {
		case CIPHER_MODE_CBC:
			sg_copy_part_to_buf(rctx->src_sg, iv_buf, rctx->iv_ctr_len,
					    chunksize - (rctx->iv_ctr_len * 2));
			break;

		case CIPHER_MODE_ECB:
			break;

		case CIPHER_MODE_CTR:
			memcpy(iv_buf, rctx->iv_ctr, rctx->iv_ctr_len);

			for (i = ((chunksize / rctx->iv_ctr_len) - 1); i > 0; i--)
				crypto_inc(iv_buf, rctx->iv_ctr_len);
			break;

		case CIPHER_MODE_GCM:
			memcpy(iv_buf, rctx->iv_ctr, rctx->iv_ctr_len);

			for (i = (chunksize / rctx->iv_ctr_len); i > 0; i--)
				crypto_inc(iv_buf, rctx->iv_ctr_len);
			break;

		case CIPHER_MODE_CCM:
			memcpy(iv_buf, rctx->iv_ctr, blocksize);

			for (i = (chunksize / blocksize); i > 0; i--)
				crypto_inc(iv_buf, blocksize);
			break;

		default:
			break;
		}

		sg_copy_part_to_buf(rctx->src_sg, src_buf, rctx->iv_ctr_len,
				    chunksize - (rctx->iv_ctr_len));
		do_decrypt(alg_name, ctx->enckey, ctx->enckeylen,
			   iv_buf, src_buf, dest_buf, rctx->iv_ctr_len);

		hmac_offset = chunksize - 1 - dest_buf[rctx->iv_ctr_len - 1] -
			ctx->digestsize;
	} else {
		char src_buf[8];
		sg_copy_part_to_buf(rctx->src_sg, src_buf, 8, chunksize - 8);
		hmac_offset = chunksize - 1 - src_buf[7] - ctx->digestsize;
	}

	/* Update length field in the DTLS Authen header (assoc data) */
	swap_hmac_offset = ntohs(hmac_offset & 0xffff);
	sg_copy_part_from_buf(rctx->assoc_sg, (u8 *) &swap_hmac_offset, 2,
			      rctx->assoc_len - 2);

	return hmac_offset;
}

/* Build up the scatterlist of buffers used to receive a SPU response message
 * for an AEAD request. Includes buffers to catch SPU message headers
 * and the response data.
 * Inputs:
 *   mssg - message containing the receive sg
 *   rctx - crypto request context
 *   rx_frag_num - number of scatterlist elements required to hold the
 *		   SPU response message
 *   assoc_len - Length of associated data included in the crypto request
 *   resp_len - Number of bytes of response data expected to be written to
 *              dst buffer from crypto API
 *   stat_pad_len - Number of bytes required to pad the STAT field to
 *		    a 4-byte boundary
 * Returns:
 *   0 if successful
 *   < 0 if an error
 */
static int
spu_aead_rx_sg_create(struct brcm_message *mssg,
		      struct iproc_reqctx_s *rctx,
		      u8 rx_frag_num,
		      u32 assoc_len,
		      u32 resp_len)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct scatterlist *sg;     /* used to build sgs in message */
	struct scatterlist *dsg;
	struct iproc_ctx_s *ctx = rctx->ctx;
	u32 datalen;           /* Number of bytes of response data expected */
	u16 nent;              /* number of sg entries in DMA mapping */
	u32 assoc_buf_len;
	u32 data_padlen = 0;
	u32 stat_pad_len = 0;
	u32 db_size = 0;
	u32 incl_icv = false;

	assoc_buf_len = spu_assoc_resp_len(ctx->cipher.mode,
	                                   ctx->alg->dtls_hmac, assoc_len,
	                                   rctx->iv_ctr_len);
	if (assoc_buf_len) {
		rx_frag_num++;
	}
	if (ctx->is_rfc4543) {
		/* RFC4543: only pad after data, not after AAD - IV included in data */
		data_padlen = spu_gcm_ccm_padlen(ctx->cipher.mode,
		                                 assoc_len + resp_len + 
		                                 rctx->aead_iv_len);
		assoc_buf_len = assoc_len;
		/* include IV after AAD */
		rx_frag_num++;
	}
	else if ((ctx->cipher.mode == CIPHER_MODE_GCM) ||
	         (ctx->cipher.mode == CIPHER_MODE_CCM)) {
		data_padlen = spu_gcm_ccm_padlen(ctx->cipher.mode, resp_len);
		if (ctx->cipher.mode == CIPHER_MODE_CCM) {
			/* ICV (after data) must be in the next 32-bit word for CCM */
			data_padlen += spu_wordalign_padlen(assoc_buf_len + resp_len + 
			                                    data_padlen);
		}
	}

	if ((ctx->cipher.mode == CIPHER_MODE_GCM) ||
	    (ctx->cipher.mode == CIPHER_MODE_CCM)) {
		if (data_padlen) {
			/* have to catch gcm/ccm pad in separate buffer */
			rx_frag_num++;
		}

		/* For GCM/CCM, always catch ICV in separate buffer */
		incl_icv = true;
		db_size += ctx->digestsize;
	}
	else if (0 == rctx->is_encrypt)
	{
		/* for decrypt, catch ICV in a separate buffer */
		incl_icv = true;
		db_size += ctx->digestsize;
	}

	db_size += assoc_buf_len + resp_len + data_padlen;
	stat_pad_len = spu_wordalign_padlen(db_size);
	if (stat_pad_len) {
			rx_frag_num++;
	}

	if (rx_frag_num > MAX_STATIC_FRAGS) {
		mssg->dst = kcalloc(rx_frag_num, sizeof(struct scatterlist),
		                       GFP_KERNEL);
		if (unlikely(mssg->dst == NULL)) {
			dev_err(dev, "%s(): Failed to allocate dst sg", __func__);
			return -ENOMEM;
		}
	}
	else
	{
		mssg->dst = rctx->msg_dst;
	}

	sg = mssg->dst;
	sg_init_table(sg, rx_frag_num);

	/* Space for SPU message header */
	sg_set_buf(sg++, rctx->pctx.spu_resp_hdr, SPU_RESP_HDR_LEN);

	if (assoc_buf_len) {
		sg_set_buf(sg++, rctx->pctx.spu_resp_assoc_data, assoc_buf_len);
	}

	if (ctx->is_rfc4543) {
		sg_set_buf(sg++, rctx->aead_iv, rctx->aead_iv_len);
	}

	/* dst sg catches both the data and, if present, the digest 
	   Copy in each dst sg entry from request, up to chunksize
	   the entire aead request must fit into one SPU request 
	   so restore dst_sg to its initial value after assigning
	   the data to the dma so that dst_sq can be used in
 	   handle_aead_resp. */
	dsg = rctx->dst_sg;
	datalen = spu_msg_sg_add(&sg, &rctx->dst_sg, &rctx->dst_skip,
	                         rctx->dst_nents, resp_len);
	rctx->dst_sg = dsg;
	if (datalen < (resp_len)) {
		dev_err(dev,
			"%s(): failed to copy dst sg to msg. expected len %u, datalen %u",
			__func__, resp_len, datalen);
		return -EFAULT;
	}

	/* If GCM/CCM data is padded, catch padding in separate buffer */
	if (data_padlen) {
		sg_set_buf(sg++, rctx->pctx.datapad, data_padlen);
	}

	/* this is set when we want to catch the ICV in a separate buffer */
	if (incl_icv) {
		sg_set_buf(sg++, rctx->pctx.digest, ctx->digestsize);
	}

	if (stat_pad_len) {
		sg_set_buf(sg++, rctx->pctx.rx_stat_pad, stat_pad_len);
	}

	sg_set_buf(sg, rctx->pctx.rx_stat, SPU_STATUS_LEN);

	nent = dma_map_sg(dev, mssg->dst, sg_nents(mssg->dst), DMA_BIDIRECTIONAL);
	if (nent == 0) {
		dev_err(dev, "%s(): Failed to DMA map dst sg with %u frags",
			__func__, rx_frag_num);
		return -EIO;
	}
	return 0;
}

/* Build up the scatterlist of buffers used to send a SPU request message
 * for an AEAD request. Includes SPU message headers and the request
 * data.
 *
 * Inputs:
 *   mssg - message containing the transmit sg
 *   rctx - crypto request context
 *   tx_frag_num - number of scatterlist elements required to construct the
 *		   SPU request message
 *   spu_hdr_len - length of SPU message header in bytes
 *   assoc - crypto API associated data scatterlist
 *   assoc_len - length of associated data
 *   assoc_nents - number of scatterlist entries containing assoc data
 *   aead_iv_len - length of AEAD IV, if included
 *   chunksize - Number of bytes of request data
 *   aad_pad_len - Number of bytes of padding at end of AAD. For GCM.
 *   pad_len - Number of pad bytes
 *   incl_icv - If true, write separate ICV buffer after data and
 *              any padding
 * Returns:
 *   0 if successful
 *   < 0 if an error
 */
static int
spu_aead_tx_sg_create(struct brcm_message *mssg,
		      struct iproc_reqctx_s *rctx,
		      u8 tx_frag_num,
		      u32 spu_hdr_len,
		      struct scatterlist *assoc,
		      unsigned assoc_len,
		      int assoc_nents,
		      u8 *aead_iv_buf,
		      unsigned aead_iv_len,
		      unsigned chunksize,
		      u32 aad_pad_len,
		      u32 pad_len,
		      bool incl_icv)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct scatterlist *sg;     /* used to build sgs in message */
	struct scatterlist *assoc_sg = assoc;
	struct iproc_ctx_s *ctx = rctx->ctx;
	u32 datalen;           /* Number of bytes of data to write */
	u32 written;           /* Number of bytes of data written */
	u16 nent;              /* number of sg entries in DMA mapping */
	u32 msg_len = 0;       /* length of SPU request in bytes  */
	u32 assoc_offset = 0;

	if (tx_frag_num > MAX_STATIC_FRAGS) {
		mssg->src = kcalloc(tx_frag_num, sizeof(struct scatterlist),
		                       GFP_KERNEL);
		if (unlikely(mssg->src == NULL)) {
			dev_err(dev, "%s(): Failed to allocate src sg", __func__);
			return -ENOMEM;
		}
	}
	else
	{
		mssg->src = rctx->msg_src;
	}

	sg = mssg->src;
	sg_init_table(sg, tx_frag_num);

	sg_set_buf(sg++, rctx->pctx.bcm_spu_req_hdr, spu_hdr_len);
	msg_len += spu_hdr_len;

	if (assoc_len) {
		/* Copy in each associated data sg entry from request */
		written = spu_msg_sg_add(&sg, &assoc_sg, &assoc_offset,
					 assoc_nents, assoc_len);
		if (written < assoc_len) {
			dev_err(dev,
				"%s(): failed to copy assoc sg to msg",
				__func__);
			return -EFAULT;
		}
		msg_len += assoc_len;
	}

	if (aead_iv_len) {
		sg_set_buf(sg++, aead_iv_buf, aead_iv_len);
		msg_len += aead_iv_len;
	}

	if (aad_pad_len) {
		sg_set_buf(sg++, rctx->pctx.spu_req_aad_pad, aad_pad_len);
		msg_len += pad_len;
	}

	datalen = chunksize;
	if (incl_icv) {
		datalen -= ctx->digestsize;
	}
	if (datalen) {
		/* For aead, a single msg should consume the entire src sg */
		written = spu_msg_sg_add(&sg, &rctx->src_sg, &rctx->src_skip,
					 rctx->src_nents, datalen);
		if (written < datalen) {
			dev_err(dev, "%s(): failed to copy src sg to msg",
				__func__);
			return -EFAULT;
		}
		msg_len += written;
	}

	if (pad_len) {
		sg_set_buf(sg++, rctx->pctx.spu_req_pad, pad_len);
		msg_len += pad_len;
	}

	if (incl_icv) {
		sg_set_buf(sg++, rctx->pctx.digest, ctx->digestsize);
		msg_len += ctx->digestsize;
	}

	*rctx->pctx.tx_stat = 0;
	sg_set_buf(sg, rctx->pctx.tx_stat, SPU_STATUS_LEN);
	msg_len += SPU_STATUS_LEN;

	if (unlikely(msg_len >= MAX_SPU_PKT_SIZE)) {
		pr_err("SPU message for AEAD request too big. Length %u. Max %u.\n",
		       msg_len, MAX_SPU_PKT_SIZE);
		return -EFAULT;
	}

	nent = dma_map_sg(dev, mssg->src, sg_nents(mssg->src), DMA_BIDIRECTIONAL);
	if (unlikely(nent == 0)) {
		dev_err(dev, "%s(): Failed to DMA map dst sg with %u frags",
			__func__, tx_frag_num);
		return -EIO;
	}

	return 0;
}

/* Submit a SPU request message for the next chunk of the current AEAD request.
 * Unlike other operation types, we assume the length of the request fits in
 * a single SPU request message. aead_enqueue() makes sure this is true.
 * Comments for other op types regarding threads applies here as well.
 */
int handle_aead_req(struct iproc_reqctx_s *rctx)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct iproc_ctx_s *ctx = rctx->ctx;
	u8 *authkey = (u8 *) ctx->authkey;
	int err;
	u32 chunksize;
	u32 resp_len;
	u32 hmac_offset = 0;
	enum hash_type hash_type = HASH_TYPE_NONE;
	u32 aead_iv_len;
	u8 *aead_iv_buf;
	u32 spu_hdr_len;
	u32 aad_pad_len;  /* For AES GCM/CCM, length of padding after AAD */
	u32 data_pad_len;
	u32 db_size;
	u32 stat_pad_len;
	u32 pad_len;
	int assoc_nents;
	bool incl_icv = false;

	/* number of entries in src and dst sg. Always includes SPU msg header
	 * and STAT.
	 */
	u8 rx_frag_num = 2;
	u8 tx_frag_num = 2;

	/* doing the whole thing at once */
	chunksize = rctx->total_todo;

	flow_log("%s: chunksize %u\n", __func__, chunksize);

	if (ctx->alg->dtls_hmac) {
		hmac_offset = spu_dtls_hmac_offset(rctx, chunksize, ctx->blocksize);
	}

	rctx->total_sent = chunksize;

	if ( ctx->is_rfc4543 )
	{
		aead_iv_len = rctx->aead_iv_len;
		aead_iv_buf = rctx->aead_iv;
	}
	else if ((ctx->cipher.mode != CIPHER_MODE_GCM)  && 
	         (ctx->cipher.mode != CIPHER_MODE_CCM)  &&
	         !ctx->alg->dtls_hmac)
	{
		aead_iv_len = rctx->iv_ctr_len;
		aead_iv_buf = rctx->iv_ctr;
	}
	else
	{
		aead_iv_len = 0;
		aead_iv_buf = NULL;
	}

	if (ctx->auth.alg == HASH_ALG_AES) {
		hash_type = ctx->cipher_type;
	}

	if (ctx->cipher.mode == CIPHER_MODE_CCM) {
		/* for CCM, AAD len + 2 (rather than AAD len) needs to be
		 * 128-bit aligned
		 */
		aad_pad_len = spu_gcm_ccm_padlen(ctx->cipher.mode, rctx->assoc_len + 2);

		/* And when decrypting CCM, need to pad without including
		 * size of ICV which is tacked on to end of chunk
		 */
		if (rctx->is_encrypt)
		{
			data_pad_len = spu_gcm_ccm_padlen(ctx->cipher.mode, chunksize);
		}
		else
		{
			data_pad_len = spu_gcm_ccm_padlen(ctx->cipher.mode, 
			                                  chunksize - ctx->digestsize);
		}

		/* CCM also requires software to rewrite portions of IV: */
		spu_ccm_update_iv(ctx->digestsize, rctx->iv_ctr_len, rctx->iv_ctr, 
		                  rctx->assoc_len, chunksize, rctx->is_encrypt, ctx->is_esp);
	}
	else if ( ctx->is_rfc4543 )
	{
		/* RFC4543: data is included in AAD, so don't pad after AAD
		 * and pad data based on both AAD + data size
		 */
		aad_pad_len = 0;
		if (0 == rctx->is_encrypt) {
			data_pad_len = spu_gcm_ccm_padlen(ctx->cipher.mode,
			                                  rctx->assoc_len + chunksize +
			                                  aead_iv_len - ctx->digestsize);
		}
		else {
			data_pad_len = spu_gcm_ccm_padlen(ctx->cipher.mode,
			                                  rctx->assoc_len + chunksize +
			                                  aead_iv_len);
		}
	}
	else {
		aad_pad_len = spu_gcm_ccm_padlen(ctx->cipher.mode, rctx->assoc_len);
		data_pad_len = spu_gcm_ccm_padlen(ctx->cipher.mode, chunksize);
	}
	flow_log("AAD padding: %u bytes, data padding %u bytes\n", aad_pad_len, data_pad_len);

	incl_icv = spu_req_incl_icv(ctx->cipher.mode, rctx->is_encrypt);
	if (incl_icv) {
		tx_frag_num++;
		/* Copy ICV from end of src scatterlist to digest buf */
		sg_copy_part_to_buf(rctx->src_sg, rctx->pctx.digest,
				    ctx->digestsize,
				    chunksize - ctx->digestsize);
	}

	atomic_long_add(chunksize, &iproc_priv.bytes_out);

	flow_log("%s()-sent rctx:%p chunksize:%u hmac_offset:%u\n",
		 __func__, rctx, chunksize, hmac_offset);

	spu_hdr_len = spu_aead_req_create(rctx->pctx.bcm_spu_req_hdr,
	                                  !(rctx->is_encrypt), ctx->auth_first,
	                                  ctx->cipher.alg, ctx->cipher.mode,
	                                  ctx->cipher_type,
	                                  ctx->enckey, ctx->enckeylen,
	                                  rctx->iv_ctr, rctx->iv_ctr_len,
	                                  ctx->auth.alg, ctx->auth.mode,
	                                  hash_type,
	                                  ctx->digestsize,
	                                  authkey, ctx->authkeylen,
	                                  rctx->assoc_len,
	                                  chunksize,
	                                  ctx->alg->dtls_hmac, hmac_offset,
	                                  aead_iv_len, aad_pad_len,
	                                  data_pad_len, ctx->is_rfc4543);

	/* Determine total length of padding required. Put all padding in one
	 * buffer.
	 */
	db_size = spu_real_db_size(rctx->assoc_len, aead_iv_len, 0,
	                           chunksize, aad_pad_len, data_pad_len, 0);
	stat_pad_len = spu_wordalign_padlen(db_size);
	pad_len = data_pad_len + stat_pad_len;
	if (pad_len) {
		tx_frag_num++;
		spu_request_pad(rctx->pctx.spu_req_pad, data_pad_len, 0,
		                ctx->auth.alg, rctx->total_sent, stat_pad_len);
	}

	dump_spu_msg_hdr(rctx->pctx.bcm_spu_req_hdr, spu_hdr_len);
	dump_sg(rctx->assoc_sg, 0, rctx->assoc_len);
	packet_dump("    aead iv: ", aead_iv_buf, aead_iv_len);
	packet_log("BD:\n");
	dump_sg(rctx->src_sg, 0, chunksize);
	packet_dump("   pad: ", rctx->pctx.spu_req_pad, pad_len);

	/* Build message containing SPU request msg and rx buffers
	 * to catch response message
	 */
	rctx->mb_mssg.error = 0;

	/* Create rx scatterlist to catch result */
	resp_len = chunksize;
	if ((ctx->cipher.mode == CIPHER_MODE_GCM) || 
	    (ctx->cipher.mode == CIPHER_MODE_CCM)) {
		/* Always catch GCM/CCM ICV in separate buffer */
		rx_frag_num++;
		if (0 == rctx->is_encrypt) {
			/* Input is ciphertxt plus ICV, but ICV not incl
			 * in output.
			 */
			resp_len -= ctx->digestsize;
		}
	}
	else if (rctx->is_encrypt) {
		/* For other modes, include digest in response buf
		 * if encrypting
		 */
		resp_len += ctx->digestsize;
	}
	else
	{
		/* for decrypt, always catch ICV in a separate buffer, as
		   as dst sg may not include memory for it */
		if ( chunksize < ctx->digestsize ) {
			return -EINVAL;
		}
		resp_len = chunksize - ctx->digestsize;
		rx_frag_num++;
	}
	rctx->dst_nents = spu_sg_count(rctx->dst_sg, resp_len, rctx->dst_skip);
	rx_frag_num += rctx->dst_nents;

	INIT_LIST_HEAD(&rctx->mb_mssg.list);
	rctx->mb_mssg.error = 0;
	err = spu_aead_rx_sg_create(&rctx->mb_mssg, rctx, rx_frag_num, rctx->assoc_len,
	                            resp_len);
	if (err) {
		return err;
	}

	/* Create tx scatterlist containing SPU request message */
	/* Count number of sg entries from the crypto API request that are to
	 * be included in this message */
	if (incl_icv)
	{
		rctx->src_nents = spu_sg_count(rctx->src_sg, chunksize - ctx->digestsize, rctx->src_skip);
	}
	else
	{
		rctx->src_nents = spu_sg_count(rctx->src_sg, chunksize, rctx->src_skip);
	}
	assoc_nents = spu_sg_count(rctx->assoc_sg, rctx->assoc_len, 0);
	tx_frag_num += rctx->src_nents;
	tx_frag_num += assoc_nents;
	if (aad_pad_len) {
		tx_frag_num++;
	}
	if (aead_iv_len) {
		tx_frag_num++;
	}
	err = spu_aead_tx_sg_create(&rctx->mb_mssg, rctx, tx_frag_num, spu_hdr_len,
	                            rctx->assoc_sg, rctx->assoc_len, assoc_nents,
	                            aead_iv_buf, aead_iv_len, chunksize, aad_pad_len,
	                            pad_len, incl_icv);
	if (err) {
		return err;
	}

	err = pdc_send_data(ctx->chan_idx, &rctx->mb_mssg);
	if (err < 0) {
		atomic_inc(&iproc_priv.mb_send_fail);
		dev_dbg(dev, "%s(): Failed to send message. err %d.",
			__func__, err);
		return err;
	}

	return -EINPROGRESS;
}

/* Process a SPU response message for an AEAD request. Unmaps the scatterlists
 * in the message and updates stats.
 */
void handle_aead_resp(struct iproc_reqctx_s *rctx)
{
	struct iproc_ctx_s *ctx = rctx->ctx;
	u32 payload_len;
	u16 icv_offset;

	flow_log("%s()\n", __func__);

	/* See how much data was returned */
	payload_len = spu_payload_length(rctx->pctx.spu_resp_hdr);
	flow_log("payload_len %u\n", payload_len);

	/* skip MH, EMH headers */
	atomic_long_add(payload_len, &iproc_priv.bytes_in);

	packet_dump("  assoc_data ", rctx->pctx.spu_resp_assoc_data,
		    rctx->assoc_len);

	/* If AES GCM, then we need to copy the ICV back to the destination
	 * buffer. In decrypt case, SPU gives us back the digest, but crypto
	 * API doesn't expect ICV in dst buffer.
	 */
	if (((ctx->cipher.mode == CIPHER_MODE_GCM) || 
	     (ctx->cipher.mode == CIPHER_MODE_CCM)) &&
	     rctx->is_encrypt) {
		icv_offset = rctx->total_sent;
		packet_dump("  ICV: ", rctx->pctx.digest, ctx->digestsize);
		flow_log("copying ICV to dst sg at offset %u\n", icv_offset);
		sg_copy_part_from_buf(rctx->dst_sg, rctx->pctx.digest,
		                      ctx->digestsize, icv_offset);
	}

	dump_sg(rctx->dst_sg, 0, rctx->total_todo);
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.op_counts[SPU_OP_AEAD]);
#endif
}

/* finish_req() is used to notify that the current request has been completed */
static void finish_req(struct iproc_reqctx_s *rctx, int err)
{
	struct crypto_async_request *areq = rctx->parent;

	flow_log("%s() rctx:%p err:%d\n\n", __func__, rctx, err);

	if (areq)
		areq->complete(areq, err);
}

/* Callback from message framework */
static void spu_rx_callback(struct brcm_message *msg)
{
	struct device *dev = &iproc_priv.pdev->dev;
	struct iproc_reqctx_s *rctx;
	struct iproc_ctx_s *ctx;
	int err = 0;

	dma_unmap_sg(dev, msg->src, sg_nents(msg->src), DMA_FROM_DEVICE);
	dma_unmap_sg(dev, msg->dst, sg_nents(msg->dst), DMA_FROM_DEVICE);

	rctx = container_of(msg, struct iproc_reqctx_s, mb_mssg);
	if (unlikely(rctx == NULL)) {
		/* This is fatal */
		dev_err(dev, "%s(): no request context", __func__);
		err = -EFAULT;
		goto cb_finish;
	}
	ctx = rctx->ctx;

	if ( msg->src != rctx->msg_src )
	{
		kfree(msg->src);
	}

	if ( msg->dst != rctx->msg_dst )
	{
		kfree(msg->dst);
	}

	/* process the SPU status */
	err = spu_status_process(rctx->pctx.rx_stat);
	if (err != 0) {
		if (err == SPU_INVALID_ICV)
			atomic_inc(&iproc_priv.bad_icv);
		err = -EBADMSG;
		goto cb_finish;
	}

	/* Process the SPU response message */
	switch (rctx->ctx->alg->type) {
	case CRYPTO_ALG_TYPE_ABLKCIPHER:
		handle_ablkcipher_resp(rctx);
		break;
	case CRYPTO_ALG_TYPE_AHASH:
		handle_ahash_resp(rctx);
		break;
	case CRYPTO_ALG_TYPE_AEAD:
		handle_aead_resp(rctx);
		break;
	default:
		err = -EINVAL;
		goto cb_finish;
	}

	/* If this response does not complete the request, then send the next
	 * request chunk.
	 */
	if (rctx->total_sent < rctx->total_todo) {
		switch (rctx->ctx->alg->type) {
		case CRYPTO_ALG_TYPE_ABLKCIPHER:
			err = handle_ablkcipher_req(rctx);
			break;
		case CRYPTO_ALG_TYPE_AHASH:
			err = handle_ahash_req(rctx);
			if (err == -EAGAIN)
				/* we saved data in hash carry, but tell crypto API
				 * we successfully completed request.
				 */
				err = 0;
			break;
		case CRYPTO_ALG_TYPE_AEAD:
		default:
			err = -EINVAL;
		}

		if (err == -EINPROGRESS)
			/* Successfully submitted request for next chunk */
			return;
	}

cb_finish:
	/* AEAD path can call netif_rx which requires bh disable */
	if (CRYPTO_ALG_TYPE_AEAD == rctx->ctx->alg->type)
	{
		local_bh_disable();
		finish_req(rctx, err);
		local_bh_enable();
	}
	else
	{
		finish_req(rctx, err);
	}
}

/* ==================== Kernel Cryptographic API ==================== */

/* ablkcipher helpers */

static int ablkcipher_enqueue(struct ablkcipher_request *req, bool encrypt)
{
	struct iproc_reqctx_s *rctx = ablkcipher_request_ctx(req);
	struct crypto_ablkcipher *reqtfm = crypto_ablkcipher_reqtfm(req);
	struct iproc_ctx_s *ctx = crypto_ablkcipher_ctx(reqtfm);
	int err;

	spu_dma_bufs_alloc(rctx);
	memcpy(rctx->pctx.bcm_spu_req_hdr, &ctx->bcm_spu_req_hdr[0],
	        ctx->spu_req_hdr_len);
	rctx->pctx.spu_req_hdr_len = ctx->spu_req_hdr_len;

	flow_log("%s() req:%p enc:%u rctx:%p ctx:%p\n", __func__, req,
		 encrypt, rctx, ctx);

	rctx->parent = &req->base;
	rctx->is_encrypt = encrypt;
	rctx->bd_suppress = false;
	rctx->total_todo = req->nbytes;
	rctx->total_sent = 0;
	rctx->total_received = 0;
	rctx->ctx = ctx;
	rctx->mb_mssg.rx_callback = spu_rx_callback;

	/* Initialize current position in src and dst scatterlists */
	rctx->src_sg = req->src;
	rctx->src_nents = 0;
	rctx->src_skip = 0;
	rctx->dst_sg = req->dst;
	rctx->dst_nents = 0;
	rctx->dst_skip = 0;

	if (ctx->cipher.mode == CIPHER_MODE_CBC ||
	    ctx->cipher.mode == CIPHER_MODE_CTR ||
	    ctx->cipher.mode == CIPHER_MODE_OFB ||
	    ctx->cipher.mode == CIPHER_MODE_XTS) {
		rctx->iv_ctr_len = crypto_ablkcipher_ivsize(reqtfm);
		if (rctx->iv_ctr_len > MAX_IV_SIZE) {
			pr_err("%s() Error: ivsize too long. (%u > %u bytes)\n",
			       __func__, rctx->iv_ctr_len, MAX_IV_SIZE);
			return -EINVAL;
		}
		memcpy(rctx->iv_ctr, req->info, rctx->iv_ctr_len);
	}
	else {
		rctx->iv_ctr_len = 0;
	}

	err = handle_ablkcipher_req(rctx);
	if (err != -EINPROGRESS)
		finish_req(rctx, err);

	return -EINPROGRESS;
}

static int des_setkey(struct crypto_ablkcipher *cipher, const u8 *key,
		      unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_ablkcipher_ctx(cipher);
	u32 tmp[DES_EXPKEY_WORDS];

	if (keylen == DES_KEY_SIZE) {
		if (des_ekey(tmp, key) == 0) {
			if (crypto_ablkcipher_get_flags(cipher) &
			    CRYPTO_TFM_REQ_WEAK_KEY) {
				crypto_ablkcipher_set_flags(cipher,
				CRYPTO_TFM_RES_WEAK_KEY);
				return -EINVAL;
			}
		}

		ctx->cipher_type = CIPHER_TYPE_DES;
	} else {
		crypto_ablkcipher_set_flags(cipher,
					    CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	return 0;
}

static int threedes_setkey(struct crypto_ablkcipher *cipher, const u8 *key,
			   unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_ablkcipher_ctx(cipher);

	if (keylen == (DES_KEY_SIZE * 3)) {
		const u32 *K = (const u32 *)key;
		if (!((K[0] ^ K[2]) | (K[1] ^ K[3])) ||
		    !((K[2] ^ K[4]) | (K[3] ^ K[5]))) {
			crypto_ablkcipher_set_flags(cipher,
			CRYPTO_TFM_RES_BAD_KEY_SCHED);
			return -EINVAL;
		}

		ctx->cipher_type = CIPHER_TYPE_3DES;
	} else {
		crypto_ablkcipher_set_flags(cipher,
					    CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	return 0;
}

static int aes_setkey(struct crypto_ablkcipher *cipher, const u8 *key,
		      unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_ablkcipher_ctx(cipher);

	if (ctx->cipher.mode == CIPHER_MODE_XTS) {
		/* XTS includes two keys of equal length */
		keylen = keylen / 2;
	}

	switch (keylen) {
	case AES_KEYSIZE_128:
		ctx->cipher_type = CIPHER_TYPE_AES128;
		break;
	case AES_KEYSIZE_192:
		ctx->cipher_type = CIPHER_TYPE_AES192;
		break;
	case AES_KEYSIZE_256:
		ctx->cipher_type = CIPHER_TYPE_AES256;
		break;
	default:
		crypto_ablkcipher_set_flags(cipher,
					    CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	BUG_ON((ctx->max_payload % AES_BLOCK_SIZE != 0));
	return 0;
}

static int rc4_setkey(struct crypto_ablkcipher *cipher, const u8 *key,
		      unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_ablkcipher_ctx(cipher);
	int i;

	ctx->enckeylen = ARC4_MAX_KEY_SIZE + ARC4_STATE_SIZE;

	ctx->enckey[0] = 0x00;	/* 0x00 */
	ctx->enckey[1] = 0x00;	/* i    */
	ctx->enckey[2] = 0x00;	/* 0x00 */
	ctx->enckey[3] = 0x00;	/* j    */
	for (i = 0; i < ARC4_MAX_KEY_SIZE; i++)
		ctx->enckey[i + ARC4_STATE_SIZE] = key[i % keylen];

	ctx->cipher_type = CIPHER_TYPE_INIT;

	return 0;
}

static int ablkcipher_setkey(struct crypto_ablkcipher *cipher, const u8 *key,
			     unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_ablkcipher_ctx(cipher);
	u32 iv_len;
	int err;

	flow_log("ablkcipher_setkey() keylen: %d\n", keylen);
	flow_dump("  key: ", key, keylen);

	switch (ctx->cipher.alg) {
	case CIPHER_ALG_DES:
		err = des_setkey(cipher, key, keylen);
		break;
	case CIPHER_ALG_3DES:
		err = threedes_setkey(cipher, key, keylen);
		break;
	case CIPHER_ALG_AES:
		err = aes_setkey(cipher, key, keylen);
		break;
	case CIPHER_ALG_RC4:
		err = rc4_setkey(cipher, key, keylen);
		break;
	default:
		pr_err("%s() Error: unknown cipher alg\n", __func__);
		err = -EINVAL;
	}
	if (err)
		return err;

	/* RC4 already populated ctx->enkey */
	if (ctx->cipher.alg != CIPHER_ALG_RC4) {
		memcpy(ctx->enckey, key, keylen);
		ctx->enckeylen = keylen;
	}

	/* SPU needs XTS keys in the reverse order the crypto API presents */
	if ((ctx->cipher.alg == CIPHER_ALG_AES) &&
	    (ctx->cipher.mode == CIPHER_MODE_XTS)) {
		unsigned xts_keylen = keylen / 2;

		memcpy(ctx->enckey, key + xts_keylen, xts_keylen);
		memcpy(ctx->enckey + xts_keylen, key, xts_keylen);
	}

	iv_len = crypto_ablkcipher_ivsize(cipher);
	flow_log("%s: iv_len %u\n", __func__, iv_len);
	ctx->spu_req_hdr_len =
		spu_cipher_req_init(ctx->bcm_spu_req_hdr,
				    ctx->cipher.alg,
				    ctx->cipher.mode,
				    ctx->cipher_type,
				    ctx->enckey,
				    ctx->enckeylen,
				    iv_len);
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.setkey_cnt[SPU_OP_CIPHER]);
#endif
	return 0;
}

static int ablkcipher_encrypt(struct ablkcipher_request *req)
{
	flow_log("ablkcipher_encrypt() alkb_req:%p nbytes:%u\n", req,
		 req->nbytes);
	flow_log("sg src %p, sg dst %p\n", req->src, req->dst);
	flow_log("Number of sg entries in src: %u\n", sg_nents(req->src));
	flow_log("Number of sg entries in dst: %u\n", sg_nents(req->dst));

	return ablkcipher_enqueue(req, true);
}

static int ablkcipher_decrypt(struct ablkcipher_request *req)
{
	flow_log("ablkcipher_decrypt() alkb_req:%p nbytes:%u\n", req,
		 req->nbytes);
	return ablkcipher_enqueue(req, false);
}

/* ahash helpers */

static int ahash_enqueue(struct ahash_request *req)
{
	struct iproc_reqctx_s *rctx = ahash_request_ctx(req);
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct iproc_ctx_s *ctx = crypto_ahash_ctx(tfm);
	int i;
	struct scatterlist *sg;
	int err;

	flow_log("ahash_enqueue() req:%p base:%p nbytes:%u\n", req, &req->base,
		 req->nbytes);
	flow_log("  sg src %p\n", req->src);
	flow_log("  Number of sg entries in src: %u\n", sg_nents(req->src));
	for_each_sg(req->src, sg, sg_nents(req->src), i) {
		flow_log("  Length of entry %d: %u\n", i, sg->length);
		flow_log("  page_link: %lu\n", sg->page_link);
		flow_log("  offset: %u\n", sg->offset);
	}

	rctx->parent = &req->base;
	rctx->ctx = ctx;
	rctx->src_sg = req->src;
	rctx->src_skip = 0;
	rctx->src_nents = 0;
	rctx->mb_mssg.rx_callback = spu_rx_callback;

	err = handle_ahash_req(rctx);
	if (err != -EINPROGRESS) {
		if (err == -EAGAIN)
			/* we saved data in hash carry, but tell crypto API
			 * we successfully completed request.
			 */
			err = 0;
		finish_req(rctx, err);
	}

	return -EINPROGRESS;
}

static int ahash_init(struct ahash_request *req)
{
	struct iproc_reqctx_s *rctx = ahash_request_ctx(req);
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct iproc_ctx_s *ctx = crypto_ahash_ctx(tfm);

	spu_dma_bufs_alloc(rctx);

	flow_log("ahash_init() req:%p\n", req);

	/* Initialize the context */
	rctx->hash_carry_len = 0;
	rctx->is_final = 0;
	rctx->total_todo = 0;
	rctx->total_sent = 0;
	rctx->total_received = 0;
	rctx->iv_ctr_len = 0;
	rctx->bd_suppress = true;
	rctx->dst_sg = NULL;
	rctx->dst_skip = 0;
	rctx->dst_nents = 0;

	ctx->digestsize = crypto_ahash_digestsize(tfm);
	ctx->blocksize = crypto_tfm_alg_blocksize(crypto_ahash_tfm(tfm));
	/* If we add a hash whose digest is larger, catch it here. */
	BUG_ON(ctx->digestsize > MAX_DIGEST_SIZE);

	rctx->is_sw_hmac = false;

	return 0;
}

static int ahash_update(struct ahash_request *req)
{
	struct iproc_reqctx_s *rctx = ahash_request_ctx(req);

	flow_log("ahash_update() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	if (!req->nbytes)
		return 0;
	rctx->total_todo += req->nbytes;

	return ahash_enqueue(req);
}

static int ahash_final(struct ahash_request *req)
{
	struct iproc_reqctx_s *rctx = ahash_request_ctx(req);

	flow_log("ahash_final() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	rctx->is_final = 1;

	return ahash_enqueue(req);
}

static int ahash_finup(struct ahash_request *req)
{
	struct iproc_reqctx_s *rctx = ahash_request_ctx(req);

	flow_log("ahash_finup() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	rctx->total_todo += req->nbytes;
	rctx->is_final = 1;

	return ahash_enqueue(req);
}

static int ahash_digest(struct ahash_request *req)
{
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	int err = 0;

	flow_log("ahash_digest() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	/* whole thing at once */
	err = ahash->init(req);
	if (!err)
		err = ahash->finup(req);

	return err;
}

/*  HMAC ahash functions */

static int ahash_hmac_setkey(struct crypto_ahash *ahash, const u8 *key,
			     unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_ahash_ctx(ahash);
	unsigned blocksize = crypto_tfm_alg_blocksize(crypto_ahash_tfm(ahash));
	unsigned digestsize = crypto_ahash_digestsize(ahash);
	unsigned index;
	const unsigned char *pkey;

	flow_log("%s() ahash:%p key:%p keylen:%u blksz:%u digestsz:%u\n",
		 __func__, ahash, key, keylen, blocksize, digestsize);
	flow_dump("  key: ", key, keylen);

	if (keylen > blocksize) {
		switch (ctx->auth.alg) {
		case HASH_ALG_MD5:
			do_shash("md5", &ctx->authkey[0], key, keylen, NULL, 0);
			break;
		case HASH_ALG_SHA1:
			do_shash("sha1", &ctx->authkey[0], key, keylen, NULL, 0);
			break;
		case HASH_ALG_SHA224:
			do_shash("sha224", &ctx->authkey[0], key, keylen, NULL, 0);
			break;
		case HASH_ALG_SHA256:
			do_shash("sha256", &ctx->authkey[0], key, keylen, NULL, 0);
			break;
		case HASH_ALG_SHA384:
			do_shash("sha384", &ctx->authkey[0], key, keylen, NULL, 0);
			break;
		case HASH_ALG_SHA512:
			do_shash("sha512", &ctx->authkey[0], key, keylen, NULL, 0);
			break;
		default:
			pr_err("%s() Error: unknown hash alg\n",
			       __func__);
			return -EINVAL;
		}

		keylen = digestsize;
		pkey = &ctx->authkey[0];
		flow_log("  keylen > digestsize... hashed\n");
		flow_dump("  newkey: ", pkey, keylen);
	}
	else {
		pkey = key;
		flow_log("  keylen < blocksize\n");
	}

	for (index = 0; index < blocksize; index++) {
		if ( index < keylen )
		{
			ctx->ipad[index] = pkey[index] ^ 0x36;
			ctx->opad[index] = pkey[index] ^ 0x5c;
		}
		else
		{
			ctx->ipad[index] = 0x36;
			ctx->opad[index] = 0x5c;
		}
	}
	ctx->authkeylen = 0;

#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.setkey_cnt[SPU_OP_HMAC]);
#endif
	flow_dump("  ipad: ", ctx->ipad, blocksize);
	flow_dump("  opad: ", ctx->opad, blocksize);

	return 0;
}

static int ahash_hmac_init(struct ahash_request *req)
{
	struct iproc_reqctx_s *rctx = ahash_request_ctx(req);
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct iproc_ctx_s *ctx = crypto_ahash_ctx(tfm);

	flow_log("ahash_hmac_init() req:%p\n", req);

	spu_dma_bufs_alloc(rctx);

	/* Initialize the context */
	rctx->is_final = 0;
	rctx->total_todo = 0;
	rctx->total_sent = 0;
	rctx->total_received = 0;
	rctx->iv_ctr_len = 0;
	rctx->bd_suppress = true;
	rctx->dst_sg = NULL;
	rctx->dst_skip = 0;
	rctx->dst_nents = 0;

	ctx->digestsize = crypto_ahash_digestsize(tfm);
	ctx->blocksize = crypto_tfm_alg_blocksize(crypto_ahash_tfm(tfm));
	/* If we add a hash whose digest is larger, catch it here. */
	BUG_ON(ctx->digestsize > MAX_DIGEST_SIZE);

	rctx->is_sw_hmac = true;
	memcpy(rctx->hash_carry, ctx->ipad, ctx->blocksize);
	rctx->hash_carry_len = ctx->blocksize;
	rctx->total_todo += ctx->blocksize;
	ctx->auth.mode = HASH_MODE_HASH;
	return 0;
}

static int ahash_hmac_update(struct ahash_request *req)
{
	flow_log("ahash_hmac_update() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	if (!req->nbytes)
		return 0;

	return ahash_update(req);
}

static int ahash_hmac_final(struct ahash_request *req)
{
	flow_log("ahash_hmac_final() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	return ahash_final(req);
}

static int ahash_hmac_finup(struct ahash_request *req)
{
	flow_log("ahash_hmac_finup() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	return ahash_finup(req);
}

static int ahash_hmac_digest(struct ahash_request *req)
{
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	int err = 0;

	flow_log("ahash_hmac_digest() req:%p nbytes:%u\n", req, req->nbytes);
	/* dump_sg(req->src, req->nbytes); */

	/* whole thing at once */
	err = ahash->init(req);
	if (!err)
		err = ahash->finup(req);

	return err;
}

/* aead helpers */

static int aead_need_fallback(struct aead_request *req)
{
	struct iproc_reqctx_s *rctx = aead_request_ctx(req);
	struct iproc_ctx_s *ctx = crypto_aead_ctx(crypto_aead_reqtfm(req));
	unsigned packetlen;
	u32 digestsize;

	/* SPU cannot handle the trivial AES-GCM case where Plaintext and AAD
	 * are both 0 bytes long
	 */
	if ((ctx->cipher.mode == CIPHER_MODE_GCM) ||
	    (ctx->cipher.mode == CIPHER_MODE_CCM)) {
		u32 cryptlen;
		if ( 0 == rctx->is_encrypt ) {
			cryptlen = req->cryptlen - ctx->digestsize;
		}
		else {
			cryptlen = req->cryptlen;
		}
		if ((cryptlen + req->assoclen) == 0) {
			flow_log("%s() GCM - no associated data or text\n", __func__);
			return 1;
		}
	}

	/* Only support CCM digest size of 8, 12, or 16 bytes */
	if ((ctx->cipher.mode == CIPHER_MODE_CCM) &&
	    (ctx->digestsize != 8) && (ctx->digestsize != 12) &&
	    (ctx->digestsize != 16)) {
		flow_log("%s() AES CCM needs fallbck for digest size %d\n",
				__func__, ctx->digestsize);
		return 1;
	}

	switch (ctx->auth.alg) {
		case HASH_ALG_MD5:
			digestsize = MD5_DIGEST_SIZE;
			break;
		case HASH_ALG_SHA1:
			digestsize = SHA1_DIGEST_SIZE;
			break;
		case HASH_ALG_SHA224:
			digestsize = SHA224_DIGEST_SIZE;
			break;
		case HASH_ALG_SHA256:
			digestsize = SHA256_DIGEST_SIZE;
			break;
		case HASH_ALG_SHA384:
			digestsize = SHA384_DIGEST_SIZE;
			break;
		case HASH_ALG_SHA512:
			digestsize = SHA512_DIGEST_SIZE;
			break;
		case HASH_ALG_AES:
			digestsize = ctx->authkeylen;
			break;
		default:
			pr_err("%s() Error: unknown hash alg %d\n",
			       __func__, ctx->auth.alg);
			return 1;
	}
	if (ctx->authkeylen != digestsize)
	{
		return 1;
	}

	packetlen = (ctx->authkeylen + ctx->enckeylen + rctx->iv_ctr_len +
	             req->assoclen + rctx->iv_ctr_len + (req->cryptlen & 0xffff) + 40);
	flow_log("%s() packetlen:%u\n", __func__, packetlen);

	return (packetlen > ctx->max_payload);
}

static void aead_complete(struct crypto_async_request *areq, int err)
{
	struct aead_request *req;
	struct iproc_reqctx_s *rctx;

	req = container_of(areq, struct aead_request, base);
	rctx = container_of(req, struct iproc_reqctx_s, subreq);
	areq = rctx->parent;
	areq->complete(areq, err);
}

static int aead_do_fallback(struct aead_request *req, bool is_encrypt)
{
	struct iproc_reqctx_s *rctx = aead_request_ctx(req);
	struct crypto_tfm *tfm = crypto_aead_tfm(crypto_aead_reqtfm(req));
	struct iproc_ctx_s *ctx = crypto_tfm_ctx(tfm);
	int err;

	if (ctx->fallback_cipher) {
		struct aead_request *subreq = &rctx->subreq;

		atomic_inc(&iproc_priv.aead_fallback_cnt);

		aead_request_set_tfm(subreq, ctx->fallback_cipher);
		aead_request_set_callback(subreq, req->base.flags,
		                          aead_complete, req->base.data);
		aead_request_set_crypt(subreq, req->src, req->dst,
		                       req->cryptlen, req->iv);
		aead_request_set_assoc(subreq, req->assoc, req->assoclen);
		err = is_encrypt ? crypto_aead_encrypt(subreq) :
		                   crypto_aead_decrypt(subreq);
	}
	else
	{
		err = -EINVAL;
	}

	return err;
}

static int aead_enqueue(struct aead_request *req, bool is_encrypt)
{
	struct iproc_reqctx_s *rctx = aead_request_ctx(req);
	struct crypto_aead *caead = crypto_aead_reqtfm(req);
	struct iproc_ctx_s *ctx = crypto_aead_ctx(caead);
	int err;
	int iv_len = crypto_aead_ivsize(caead);

	spu_dma_bufs_alloc(rctx);

	flow_log("%s() req:%p enc:%u\n", __func__, req, is_encrypt);

	if (req->assoclen > MAX_ASSOC_SIZE) {
		pr_err("%s() Error: associated data too long. (%u > %u bytes)\n",
		       __func__, req->assoclen, MAX_ASSOC_SIZE);
		return -EINVAL;
	}

	rctx->parent = &req->base;
	rctx->is_encrypt = is_encrypt;
	rctx->bd_suppress = false;
	rctx->total_todo = req->cryptlen;
	rctx->total_sent = 0;
	rctx->total_received = 0;
	rctx->is_sw_hmac = false;
	rctx->ctx = ctx;
	rctx->mb_mssg.rx_callback = spu_rx_callback;

	/* Initialize current position in src and dst scatterlists */
	rctx->src_sg = req->src;
	rctx->src_nents = 0;
	rctx->src_skip = 0;
	rctx->dst_sg = req->dst;
	rctx->dst_nents = 0;
	rctx->dst_skip = 0;
	rctx->assoc_sg = req->assoc;
	rctx->assoc_len = req->assoclen;

	if (ctx->cipher.mode == CIPHER_MODE_CBC ||
	    ctx->cipher.mode == CIPHER_MODE_CTR ||
	    ctx->cipher.mode == CIPHER_MODE_OFB ||
	    ctx->cipher.mode == CIPHER_MODE_GCM) {
		rctx->iv_ctr_len = ctx->salt_len + iv_len;
		if (rctx->iv_ctr_len > MAX_IV_SIZE) {
			pr_err("%s() Error: ivsize too long. (%u > %u bytes)\n",
			       __func__, rctx->iv_ctr_len, MAX_IV_SIZE);
			return -EINVAL;
		}
	}
	else if (ctx->cipher.mode == CIPHER_MODE_CCM) {
		rctx->iv_ctr_len = CCM_AES_IV_SIZE;
	}
	else {
		rctx->iv_ctr_len = 0;
	}

	if (rctx->iv_ctr_len) {
		memset(rctx->iv_ctr, 0, rctx->iv_ctr_len);
		if (ctx->salt_len) {
			memcpy(rctx->iv_ctr + ctx->salt_offset, ctx->salt, ctx->salt_len);
		}
		memcpy(rctx->iv_ctr + ctx->salt_offset + ctx->salt_len, req->iv,
		       rctx->iv_ctr_len - ctx->salt_len - ctx->salt_offset);
	}

	/* iv is part of aad */
	if ( ctx->is_rfc4543 )
	{
		rctx->aead_iv_len = iv_len;
		memcpy(rctx->aead_iv, req->iv, iv_len);
	}

	rctx->hash_carry_len = 0;

	flow_log("  iv_ctr_len:%u\n", rctx->iv_ctr_len);
	flow_dump("  iv: ", rctx->iv_ctr, rctx->iv_ctr_len);
	flow_log("  authkeylen:%u\n", ctx->authkeylen);
	flow_log("  ctx:%p\n", ctx);

	/* If we need authenc.c to handle the request then do it... */
	if (unlikely(aead_need_fallback(req)))
		return aead_do_fallback(req, is_encrypt);

#if defined(CONFIG_BLOG)
	if ( blog_enable && (crypto_aead_get_flags(caead) & CRYPTO_TFM_REQ_MAY_BLOG) )
	{
		rctx->pNBuf = SKBUFF_2_PNBUFF(req->base.data);
		spu_blog_emit_aead(rctx);
	}
#endif

	err = handle_aead_req(rctx);
	if (err != -EINPROGRESS)
		finish_req(rctx, err);

	return -EINPROGRESS;
}

static int aead_authenc_setkey(struct crypto_aead *cipher,
			       const u8 *key, unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_aead_ctx(cipher);
	struct rtattr *rta = (void *)key;
	struct crypto_authenc_key_param *param;
	const u8 *origkey = key;
	const unsigned origkeylen = keylen;
	int ret = 0;
	u32 blocksize;

	flow_log("%s() aead:%p key:%p keylen:%u\n", __func__, cipher, key,
		 keylen);
	flow_dump("  key: ", key, keylen);

	if (!RTA_OK(rta, keylen))
		goto badkey;
	if (rta->rta_type != CRYPTO_AUTHENC_KEYA_PARAM)
		goto badkey;
	if (RTA_PAYLOAD(rta) < sizeof(*param))
		goto badkey;

	param = RTA_DATA(rta);
	ctx->enckeylen = be32_to_cpu(param->enckeylen);

	key += RTA_ALIGN(rta->rta_len);
	keylen -= RTA_ALIGN(rta->rta_len);

	if (keylen < ctx->enckeylen)
		goto badkey;

	if (ctx->enckeylen > MAX_KEY_SIZE)
		goto badkey;

	ctx->authkeylen = keylen - ctx->enckeylen;
	if ( ctx->auth.alg >= HASH_ALG_SHA384 )
	{
		blocksize = SHA512_BLOCK_SIZE;
	}
	else
	{
		blocksize = SHA1_BLOCK_SIZE;
	}

	if (ctx->authkeylen > blocksize) {
		u32 digestsize = 0;
		switch (ctx->auth.alg) {
			case HASH_ALG_MD5:
				digestsize = MD5_DIGEST_SIZE;
				do_shash("md5", &ctx->authkey[0], key, ctx->authkeylen, NULL, 0);
				break;
			case HASH_ALG_SHA1:
				digestsize = SHA1_DIGEST_SIZE;
				do_shash("sha1", &ctx->authkey[0], key, ctx->authkeylen, NULL, 0);
				break;
			case HASH_ALG_SHA224:
				digestsize = SHA224_DIGEST_SIZE;
				do_shash("sha224", &ctx->authkey[0], key, ctx->authkeylen, NULL, 0);
				break;
			case HASH_ALG_SHA256:
				digestsize = SHA256_DIGEST_SIZE;
				do_shash("sha256", &ctx->authkey[0], key, ctx->authkeylen, NULL, 0);
				break;
			case HASH_ALG_SHA384:
				digestsize = SHA384_DIGEST_SIZE;
				do_shash("sha384", &ctx->authkey[0], key, ctx->authkeylen, NULL, 0);
				break;
			case HASH_ALG_SHA512:
				digestsize = SHA512_DIGEST_SIZE;
				do_shash("sha512", &ctx->authkey[0], key, ctx->authkeylen, NULL, 0);
				break;
			default:
				pr_err("%s() Error: unknown hash alg\n",
				       __func__);
				return -EINVAL;
		}
		ctx->authkeylen = digestsize;
		flow_log("  keylen > digestsize... hashed\n");
		flow_dump("  newkey: ", &ctx->authkey[0], ctx->authkeylen);
	}
	else
	{
		memcpy(ctx->authkey, key, ctx->authkeylen);
	}

	memcpy(ctx->enckey, key + ctx->authkeylen, ctx->enckeylen);
	switch (ctx->alg->cipher_info.alg) {
	case CIPHER_ALG_DES:
		if (ctx->enckeylen == DES_KEY_SIZE) {
			u32 tmp[DES_EXPKEY_WORDS];
			if (des_ekey(tmp, ctx->enckey) == 0) {
				if (crypto_aead_get_flags(cipher) &
				    CRYPTO_TFM_REQ_WEAK_KEY) {
					crypto_aead_set_flags(cipher,
					CRYPTO_TFM_RES_WEAK_KEY);
					return -EINVAL;
				}
			}

			ctx->cipher_type = CIPHER_TYPE_DES;
		} else {
			goto badkey;
		}
		break;
	case CIPHER_ALG_3DES:
		if (ctx->enckeylen == (DES_KEY_SIZE * 3)) {
			const u32 *K = (const u32 *)ctx->enckey;
			if (!((K[0] ^ K[2]) | (K[1] ^ K[3])) ||
			    !((K[2] ^ K[4]) | (K[3] ^ K[5]))) {
				crypto_aead_set_flags(cipher,
				CRYPTO_TFM_RES_BAD_KEY_SCHED);
				return -EINVAL;
			}

			ctx->cipher_type = CIPHER_TYPE_3DES;
		} else {
			crypto_aead_set_flags(cipher,
					      CRYPTO_TFM_RES_BAD_KEY_LEN);
			return -EINVAL;
		}
		break;
	case CIPHER_ALG_AES:
		switch (ctx->enckeylen) {
		case AES_KEYSIZE_128:
			ctx->cipher_type = CIPHER_TYPE_AES128;
			break;
		case AES_KEYSIZE_192:
			ctx->cipher_type = CIPHER_TYPE_AES192;
			break;
		case AES_KEYSIZE_256:
			ctx->cipher_type = CIPHER_TYPE_AES256;
			break;
		default:
			goto badkey;
		}
		break;
	case CIPHER_ALG_RC4:
		ctx->cipher_type = CIPHER_TYPE_INIT;
		break;
	case CIPHER_ALG_NONE:
		ctx->cipher_type = CIPHER_TYPE_NONE;
		break;
	default:
		pr_err("%s() Error: Unknown cipher alg\n", __func__);
		return -EINVAL;
	}

	flow_log("  enckeylen:%u authkeylen:%u\n", ctx->enckeylen,
		 ctx->authkeylen);
	flow_dump("  enc: ", ctx->enckey, ctx->enckeylen);
	flow_dump("  auth: ", ctx->authkey, ctx->authkeylen);

	/* setkey the fallback just in case we needto use it */
	if (ctx->fallback_cipher) {
		flow_log("  running fallback setkey()\n");

		crypto_aead_clear_flags(ctx->fallback_cipher, CRYPTO_TFM_REQ_MASK);
		crypto_aead_set_flags(ctx->fallback_cipher, crypto_aead_get_flags(cipher) &
		                      CRYPTO_TFM_REQ_MASK);
		ret = crypto_aead_setkey(ctx->fallback_cipher, origkey, origkeylen);
		if (ret) {
			flow_log("  fallback setkey() returned:%d\n", ret);
			crypto_aead_set_flags(cipher, crypto_aead_get_flags(ctx->fallback_cipher) &
			                      CRYPTO_TFM_RES_MASK);
		}
	}
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.setkey_cnt[SPU_OP_AEAD]);
#endif
	return ret;

badkey:
	ctx->enckeylen = 0;
	ctx->authkeylen = 0;
	ctx->digestsize = 0;

	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static int aead_gcm_ccm_setkey(struct crypto_aead *cipher,
                               const u8 *key, unsigned keylen)
{
	struct iproc_ctx_s *ctx = crypto_aead_ctx(cipher);
	const u8 *origkey = key;
	const unsigned origkeylen = keylen;

	int ret = 0;

	flow_log("%s() aead:%p key:%p keylen:%u\n", __func__, cipher, key,
		 keylen);
	flow_dump("  key: ", key, keylen);

	ctx->enckeylen = keylen;
	ctx->digestsize = keylen;
	ctx->authkeylen = 0;

	memcpy(ctx->enckey, key, ctx->enckeylen);

	switch (ctx->enckeylen) {
	case AES_KEYSIZE_128:
		ctx->cipher_type = CIPHER_TYPE_AES128;
		break;
	case AES_KEYSIZE_192:
		ctx->cipher_type = CIPHER_TYPE_AES192;
		break;
	case AES_KEYSIZE_256:
		ctx->cipher_type = CIPHER_TYPE_AES256;
		break;
	default:
		goto badkey;
	}

	flow_log("  enckeylen:%u authkeylen:%u\n", ctx->enckeylen,
		 ctx->authkeylen);
	flow_dump("  enc: ", ctx->enckey, ctx->enckeylen);
	flow_dump("  auth: ", ctx->authkey, ctx->authkeylen);

	/* setkey the fallback just in case we need to use it */
	if (ctx->fallback_cipher) {
		flow_log("  running fallback setkey()\n");

		crypto_aead_clear_flags(ctx->fallback_cipher, CRYPTO_TFM_REQ_MASK);
		crypto_aead_set_flags(ctx->fallback_cipher, crypto_aead_get_flags(cipher) &
		                      CRYPTO_TFM_REQ_MASK);
		ret = crypto_aead_setkey(ctx->fallback_cipher, origkey, origkeylen);
		if (ret) {
			flow_log("  fallback setkey() returned:%d\n", ret);
			crypto_aead_set_flags(cipher, crypto_aead_get_flags(ctx->fallback_cipher) &
			                      CRYPTO_TFM_RES_MASK);
		}
	}
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.setkey_cnt[SPU_OP_AEAD]);
#endif
	return ret;

badkey:
	ctx->enckeylen = 0;
	ctx->authkeylen = 0;
	ctx->digestsize = 0;

	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}


static int aead_gcm_esp_setkey(struct crypto_aead *cipher,
			       const u8 *key, unsigned int keylen)
{
	struct iproc_ctx_s *ctx = crypto_aead_ctx(cipher);
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	int ret = 0;

	flow_log("%s\n", __func__);
	ctx->salt_len = GCM_ESP_SALT_SIZE;
	ctx->salt_offset = GCM_ESP_SALT_OFFSET;
	memcpy(ctx->salt, key + keylen - GCM_ESP_SALT_SIZE, GCM_ESP_SALT_SIZE);
	keylen -= GCM_ESP_SALT_SIZE;
	ctx->digestsize = GCM_ESP_DIGESTSIZE;
	ctx->is_esp = true;
	flow_dump("salt: ", ctx->salt, GCM_ESP_SALT_SIZE);

	flow_log("%s() keylen:%u\n", __func__, keylen);
	flow_dump("  key: ", key, keylen);

	ctx->enckeylen = keylen;
	ctx->authkeylen = 0;
	memcpy(ctx->enckey, key, ctx->enckeylen);

	switch (ctx->enckeylen) {
	case AES_KEYSIZE_128:
		ctx->cipher_type = CIPHER_TYPE_AES128;
		break;
	case AES_KEYSIZE_192:
		ctx->cipher_type = CIPHER_TYPE_AES192;
		break;
	case AES_KEYSIZE_256:
		ctx->cipher_type = CIPHER_TYPE_AES256;
		break;
	default:
		goto badkey;
	}

	flow_log("  enckeylen:%u authkeylen:%u\n", ctx->enckeylen,
		 ctx->authkeylen);
	flow_dump("  enc: ", ctx->enckey, ctx->enckeylen);
	flow_dump("  auth: ", ctx->authkey, ctx->authkeylen);

	/* setkey the fallback just in case we need to use it */
	if (ctx->fallback_cipher) {
		flow_log("  running fallback setkey()\n");

		ctx->fallback_cipher->base.crt_flags &= ~CRYPTO_TFM_REQ_MASK;
		ctx->fallback_cipher->base.crt_flags |=
		    tfm->crt_flags & CRYPTO_TFM_REQ_MASK;
		ret = crypto_aead_setkey(ctx->fallback_cipher, key,
					 keylen + ctx->salt_len);
		if (ret) {
			flow_log("  fallback setkey() returned:%d\n", ret);
			tfm->crt_flags &= ~CRYPTO_TFM_RES_MASK;
			tfm->crt_flags |=
			    (ctx->fallback_cipher->
			     base.crt_flags & CRYPTO_TFM_RES_MASK);
		}
	}
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.setkey_cnt[SPU_OP_AEAD]);
#endif
	return ret;

badkey:
	ctx->enckeylen = 0;
	ctx->authkeylen = 0;
	ctx->digestsize = 0;

	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static int aead_ccm_esp_setkey(struct crypto_aead *cipher,
			       const u8 *key, unsigned int keylen)
{
	struct iproc_ctx_s *ctx = crypto_aead_ctx(cipher);
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	int ret = 0;

	flow_log("%s\n", __func__);
	ctx->salt_len = CCM_ESP_SALT_SIZE;
	ctx->salt_offset = CCM_ESP_SALT_OFFSET;
	memcpy(ctx->salt, key + keylen - CCM_ESP_SALT_SIZE, CCM_ESP_SALT_SIZE);
	keylen -= CCM_ESP_SALT_SIZE;
	ctx->is_esp = true;
	flow_dump("salt: ", ctx->salt, CCM_ESP_SALT_SIZE);

	flow_log("%s() keylen:%u\n", __func__, keylen);
	flow_dump("  key: ", key, keylen);

	ctx->enckeylen = keylen;
	ctx->authkeylen = 0;
	memcpy(ctx->enckey, key, ctx->enckeylen);

	switch (ctx->enckeylen) {
	case AES_KEYSIZE_128:
		ctx->cipher_type = CIPHER_TYPE_AES128;
		break;
	case AES_KEYSIZE_192:
		ctx->cipher_type = CIPHER_TYPE_AES192;
		break;
	case AES_KEYSIZE_256:
		ctx->cipher_type = CIPHER_TYPE_AES256;
		break;
	default:
		goto badkey;
	}

	flow_log("  enckeylen:%u authkeylen:%u\n", ctx->enckeylen,
		 ctx->authkeylen);
	flow_dump("  enc: ", ctx->enckey, ctx->enckeylen);
	flow_dump("  auth: ", ctx->authkey, ctx->authkeylen);

	/* setkey the fallback just in case we need to use it */
	if (ctx->fallback_cipher) {
		flow_log("  running fallback setkey()\n");

		ctx->fallback_cipher->base.crt_flags &= ~CRYPTO_TFM_REQ_MASK;
		ctx->fallback_cipher->base.crt_flags |=
		    tfm->crt_flags & CRYPTO_TFM_REQ_MASK;
		ret = crypto_aead_setkey(ctx->fallback_cipher, key,
					 keylen + ctx->salt_len);
		if (ret) {
			flow_log("  fallback setkey() returned:%d\n", ret);
			tfm->crt_flags &= ~CRYPTO_TFM_RES_MASK;
			tfm->crt_flags |=
			    (ctx->fallback_cipher->
			     base.crt_flags & CRYPTO_TFM_RES_MASK);
		}
	}
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.setkey_cnt[SPU_OP_AEAD]);
#endif
	return ret;

badkey:
	ctx->enckeylen = 0;
	ctx->authkeylen = 0;
	ctx->digestsize = 0;

	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static int aead_gcm_rfc4543_esp_setkey(struct crypto_aead *cipher,
                                       const u8 *key, unsigned int keylen)
{
	struct iproc_ctx_s *ctx = crypto_aead_ctx(cipher);
	struct crypto_tfm *tfm = crypto_aead_tfm(cipher);
	int ret = 0;

	flow_log("%s\n", __func__);
	ctx->salt_len = GCM_ESP_SALT_SIZE;
	ctx->salt_offset = GCM_ESP_SALT_OFFSET;
	memcpy(ctx->salt, key + keylen - GCM_ESP_SALT_SIZE, GCM_ESP_SALT_SIZE);
	keylen -= GCM_ESP_SALT_SIZE;
	ctx->digestsize = GCM_ESP_DIGESTSIZE;
	ctx->is_esp = true;
	ctx->is_rfc4543 = true;
	flow_dump("salt: ", ctx->salt, GCM_ESP_SALT_SIZE);

	flow_log("%s() keylen:%u\n", __func__, keylen);
	flow_dump("  key: ", key, keylen);

	ctx->enckeylen = keylen;
	ctx->authkeylen = 0;
	memcpy(ctx->enckey, key, ctx->enckeylen);

	switch (ctx->enckeylen) {
	case AES_KEYSIZE_128:
		ctx->cipher_type = CIPHER_TYPE_AES128;
		break;
	case AES_KEYSIZE_192:
		ctx->cipher_type = CIPHER_TYPE_AES192;
		break;
	case AES_KEYSIZE_256:
		ctx->cipher_type = CIPHER_TYPE_AES256;
		break;
	default:
		goto badkey;
	}

	flow_log("  enckeylen:%u authkeylen:%u\n", ctx->enckeylen,
		 ctx->authkeylen);
	flow_dump("  enc: ", ctx->enckey, ctx->enckeylen);
	flow_dump("  auth: ", ctx->authkey, ctx->authkeylen);

	/* setkey the fallback just in case we need to use it */
	if (ctx->fallback_cipher) {
		flow_log("  running fallback setkey()\n");

		ctx->fallback_cipher->base.crt_flags &= ~CRYPTO_TFM_REQ_MASK;
		ctx->fallback_cipher->base.crt_flags |=
		    tfm->crt_flags & CRYPTO_TFM_REQ_MASK;
		ret = crypto_aead_setkey(ctx->fallback_cipher, key,
					 keylen + ctx->salt_len);
		if (ret) {
			flow_log("  fallback setkey() returned:%d\n", ret);
			tfm->crt_flags &= ~CRYPTO_TFM_RES_MASK;
			tfm->crt_flags |=
			    (ctx->fallback_cipher->
			     base.crt_flags & CRYPTO_TFM_RES_MASK);
		}
	}
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.setkey_cnt[SPU_OP_AEAD]);
#endif
	return ret;

badkey:
	ctx->enckeylen = 0;
	ctx->authkeylen = 0;
	ctx->digestsize = 0;

	crypto_aead_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}

static int aead_setauthsize(struct crypto_aead *cipher, unsigned authsize)
{
	struct iproc_ctx_s *ctx = crypto_aead_ctx(cipher);
	int ret = 0;

	flow_log("%s() aead:%p authkeylen:%u authsize:%u\n",
		 __func__, cipher, ctx->authkeylen, authsize);

	ctx->digestsize = authsize;

	/* setkey the fallback just in case we need to use it */
	if (ctx->fallback_cipher) {
		flow_log("  running fallback setauth()\n");

		ret = crypto_aead_setauthsize(ctx->fallback_cipher, authsize);
		if (ret)
			flow_log("  fallback setauth() returned:%d\n", ret);
	}

	return ret;
}

static int aead_encrypt(struct aead_request *req)
{
	flow_log("%s() aead_req:%p cryptlen:%u %08x\n", __func__, req,
		 req->cryptlen, req->cryptlen);
	dump_sg(req->src, 0, req->cryptlen);
	flow_log("  assoc_len:%u\n", req->assoclen);
	dump_sg(req->assoc, 0, req->assoclen);

	return aead_enqueue(req, true);
}

static int aead_decrypt(struct aead_request *req)
{
	flow_log("%s() aead_req:%p cryptlen:%u\n", __func__, req,
		 req->cryptlen);
	dump_sg(req->src, 0, req->cryptlen);
	flow_log("  assoc_len:%u\n", req->assoclen);
	dump_sg(req->assoc, 0, req->assoclen);

	return aead_enqueue(req, false);
}

static int aead_givencrypt(struct aead_givcrypt_request *greq)
{
	struct crypto_aead *authenc = crypto_aead_reqtfm(&greq->areq);
	struct iproc_ctx_s *ctx = crypto_aead_ctx(authenc);

	unsigned iv_size = crypto_aead_ivsize(authenc);

	if (iv_size)
	{
		flow_log("%s() greq:%p seq:%016llx giv:%p iv_size:%u\n", __func__,
		         greq, greq->seq, greq->giv, iv_size);

		/* if we are testing givencrypt then use
		   zero for the IV base each time */
		if (givencrypt_test_mode)
			memset(greq->giv, 0, iv_size);
		else
			memcpy(greq->giv, ctx->iv, iv_size);

		/* avoid consecutive packets going out with same IV */
		*(__be64 *) greq->giv ^= cpu_to_be64(greq->seq);
		memcpy(greq->areq.iv, greq->giv, iv_size);

		flow_dump("  giv: ", greq->giv, iv_size);
	}

	return aead_encrypt(&greq->areq);
}

/* ==================== Supported Cipher Algorithms ==================== */

static struct iproc_alg_s driver_algs[] = {
/* AEAD algorithms. */
	/* AES-GCM */
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "gcm(aes)",
			.cra_driver_name = "gcm-aes-iproc",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = GCM_AES_IV_SIZE,
				     .maxauthsize = AES_BLOCK_SIZE,
				     .setkey = aead_gcm_ccm_setkey,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_GCM,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_AES,
		       .mode = HASH_MODE_GCM,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
		.type = CRYPTO_ALG_TYPE_AEAD,
		.alg.crypto = {
			.cra_name = "ccm(aes)",
			.cra_driver_name = "ccm-aes-iproc",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = CCM_AES_IV_SIZE,
				     .maxauthsize = AES_BLOCK_SIZE,
				     .setkey = aead_gcm_ccm_setkey,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
		},
		.cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CCM,
		},
		.auth_info = {
		       .alg = HASH_ALG_AES,
		       .mode = HASH_MODE_CCM,
		},
		.auth_first = 0,
		.dtls_hmac = 0,
	},

/* IPSEC AEAD algorithms. */
	/* enc -> hash - aes */
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "rfc4106(gcm(aes))",
			.cra_driver_name = "gcm-aes-esp-iproc",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = GCM_ESP_IV_SIZE,
				     .maxauthsize = AES_BLOCK_SIZE,
				     .setkey = aead_gcm_esp_setkey,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK
		},
		.cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_GCM,
		},
		.auth_info = {
		       .alg = HASH_ALG_AES,
		       .mode = HASH_MODE_GCM,
		},
		.auth_first = 0,
		.dtls_hmac = 0,
	},
	{
		.type = CRYPTO_ALG_TYPE_AEAD,
		.alg.crypto = {
			.cra_name = "rfc4309(ccm(aes))",
			.cra_driver_name = "ccm-aes-esp-iproc",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = CCM_AES_IV_SIZE,
				     .maxauthsize = AES_BLOCK_SIZE,
				     .setkey = aead_ccm_esp_setkey,
			},
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK
		},
		.cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CCM,
		},
		.auth_info = {
		       .alg = HASH_ALG_AES,
		       .mode = HASH_MODE_CCM,
		},
		.auth_first = 0,
		.dtls_hmac = 0,
	},  
	{
		.type = CRYPTO_ALG_TYPE_AEAD,
		.alg.crypto = {
			.cra_name = "rfc4543(gcm(aes))",
			.cra_driver_name = "gmac-aes-esp-iproc",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = GCM_ESP_IV_SIZE,
				     .maxauthsize = AES_BLOCK_SIZE,
				     .setkey = aead_gcm_rfc4543_esp_setkey,
			},
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK
		},
		.cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_GCM,
		},
		.auth_info = {
		       .alg = HASH_ALG_AES,
		       .mode = CIPHER_MODE_GCM,
		},
		.auth_first = 0,
		.dtls_hmac = 0,
	}, 
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(md5),ecb(cipher_null))",
			.cra_driver_name = "authenc(hmac(md5-iproc),ecb(cipher_null-iproc))",
			.cra_blocksize = 1,
			.cra_aead = {
				     .ivsize = 0,
				     .maxauthsize = MD5_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_MD5,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha1),ecb(cipher_null))",
			.cra_driver_name = "authenc(hmac(sha1-iproc),ecb(cipher_null-iproc))",
			.cra_blocksize = 1,
			.cra_aead = {
				     .ivsize = 0,
				     .maxauthsize = SHA1_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA1,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha224),ecb(cipher_null))",
			.cra_driver_name = "authenc(hmac(sha224-iproc),ecb(cipher_null-iproc))",
			.cra_blocksize = 1,
			.cra_aead = {
				     .ivsize = 0,
				     .maxauthsize = SHA224_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA224,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha256),ecb(cipher_null))",
			.cra_driver_name = "authenc(hmac(sha256-iproc),ecb(cipher_null-iproc))",
			.cra_blocksize = 1,
			.cra_aead = {
				     .ivsize = 0,
				     .maxauthsize = SHA256_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA256,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha384),ecb(cipher_null))",
			.cra_driver_name = "authenc(hmac(sha384-iproc),ecb(cipher_null-iproc))",
			.cra_blocksize = 1,
			.cra_aead = {
				     .ivsize = 0,
				     .maxauthsize = SHA384_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA384,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha512),ecb(cipher_null))",
			.cra_driver_name = "authenc(hmac(sha512-iproc),ecb(cipher_null-iproc))",
			.cra_blocksize = 1,
			.cra_aead = {
				     .ivsize = 0,
				     .maxauthsize = SHA512_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA512,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(md5),cbc(aes))",
			.cra_driver_name = "authenc(hmac(md5-iproc),cbc(aes-iproc))",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = AES_BLOCK_SIZE,
				     .maxauthsize = MD5_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_MD5,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha1),cbc(aes))",
			.cra_driver_name = "authenc(hmac(sha1-iproc),cbc(aes-iproc))",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = AES_BLOCK_SIZE,
				     .maxauthsize = SHA1_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA1,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha224),cbc(aes))",
			.cra_driver_name = "authenc(hmac(sha224-iproc),cbc(aes-iproc))",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = AES_BLOCK_SIZE,
				     .maxauthsize = SHA224_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA224,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha256),cbc(aes))",
			.cra_driver_name = "authenc(hmac(sha256-iproc),cbc(aes-iproc))",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = AES_BLOCK_SIZE,
				     .maxauthsize = SHA256_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA256,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha384),cbc(aes))",
			.cra_driver_name = "authenc(hmac(sha384-iproc),cbc(aes-iproc))",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = AES_BLOCK_SIZE,
				     .maxauthsize = SHA384_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA384,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha512),cbc(aes))",
			.cra_driver_name = "authenc(hmac(sha512-iproc),cbc(aes-iproc))",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = AES_BLOCK_SIZE,
				     .maxauthsize = SHA512_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA512,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	/* enc -> hash - des */
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(md5),cbc(des))",
			.cra_driver_name = "authenc(hmac(md5-iproc),cbc(des-iproc))",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES_BLOCK_SIZE,
				     .maxauthsize = MD5_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_MD5,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha1),cbc(des))",
			.cra_driver_name = "authenc(hmac(sha1-iproc),cbc(des-iproc))",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES_BLOCK_SIZE,
				     .maxauthsize = SHA1_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA1,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha224),cbc(des))",
			.cra_driver_name = "authenc(hmac(sha224-iproc),cbc(des-iproc))",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES_BLOCK_SIZE,
				     .maxauthsize = SHA224_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA224,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha256),cbc(des))",
			.cra_driver_name = "authenc(hmac(sha256-iproc),cbc(des-iproc))",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES_BLOCK_SIZE,
				     .maxauthsize = SHA256_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA256,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha384),cbc(des))",
			.cra_driver_name = "authenc(hmac(sha384-iproc),cbc(des-iproc))",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES_BLOCK_SIZE,
				     .maxauthsize = SHA384_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA384,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha512),cbc(des))",
			.cra_driver_name = "authenc(hmac(sha512-iproc),cbc(des-iproc))",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES_BLOCK_SIZE,
				     .maxauthsize = SHA512_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA512,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	/* enc -> hash - 3des */
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(md5),cbc(des3_ede))",
			.cra_driver_name = "authenc(hmac(md5-iproc),cbc(des3_ede-iproc))",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES3_EDE_BLOCK_SIZE,
				     .maxauthsize = MD5_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_MD5,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha1),cbc(des3_ede))",
			.cra_driver_name = "authenc(hmac(sha1-iproc),cbc(des3_ede-iproc))",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES3_EDE_BLOCK_SIZE,
				     .maxauthsize = SHA1_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA1,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha224),cbc(des3_ede))",
			.cra_driver_name = "authenc(hmac(sha224-iproc),cbc(des3_ede-iproc))",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES3_EDE_BLOCK_SIZE,
				     .maxauthsize = SHA224_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA224,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha256),cbc(des3_ede))",
			.cra_driver_name = "authenc(hmac(sha256-iproc),cbc(des3_ede-iproc))",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES3_EDE_BLOCK_SIZE,
				     .maxauthsize = SHA256_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA256,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha384),cbc(des3_ede))",
			.cra_driver_name = "authenc(hmac(sha384-iproc),cbc(des3_ede-iproc))",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES3_EDE_BLOCK_SIZE,
				     .maxauthsize = SHA384_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA384,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AEAD,
	 .alg.crypto = {
			.cra_name = "authenc(hmac(sha512),cbc(des3_ede))",
			.cra_driver_name = "authenc(hmac(sha512-iproc),cbc(des3_ede-iproc))",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_aead = {
				     .ivsize = DES3_EDE_BLOCK_SIZE,
				     .maxauthsize = SHA512_DIGEST_SIZE,
				     },
			.cra_flags = CRYPTO_ALG_NEED_FALLBACK,
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA512,
		       .mode = HASH_MODE_HMAC,
		       },
	 .auth_first = 0,
	 .dtls_hmac = 0,
	 },

/* ABLKCIPHER algorithms. */
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ecb(arc4)",
			.cra_driver_name = "ecb(arc4-iproc)",
			.cra_blocksize = ARC4_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = ARC4_MIN_KEY_SIZE,
					   .max_keysize = ARC4_MAX_KEY_SIZE,
					   .ivsize = 0,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_RC4,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ofb(des)",
			.cra_driver_name = "ofb(des-iproc)",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = DES_KEY_SIZE,
					   .max_keysize = DES_KEY_SIZE,
					   .ivsize = DES_BLOCK_SIZE,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_OFB,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "cbc(des)",
			.cra_driver_name = "cbc(des-iproc)",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = DES_KEY_SIZE,
					   .max_keysize = DES_KEY_SIZE,
					   .ivsize = DES_BLOCK_SIZE,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ecb(des)",
			.cra_driver_name = "ecb(des-iproc)",
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = DES_KEY_SIZE,
					   .max_keysize = DES_KEY_SIZE,
					   .ivsize = 0,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_DES,
			 .mode = CIPHER_MODE_ECB,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ofb(des3_ede)",
			.cra_driver_name = "ofb(des3_ede-iproc)",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = DES3_EDE_KEY_SIZE,
					   .max_keysize = DES3_EDE_KEY_SIZE,
					   .ivsize = DES3_EDE_BLOCK_SIZE,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_OFB,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "cbc(des3_ede)",
			.cra_driver_name = "cbc(des3_ede-iproc)",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = DES3_EDE_KEY_SIZE,
					   .max_keysize = DES3_EDE_KEY_SIZE,
					   .ivsize = DES3_EDE_BLOCK_SIZE,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ecb(des3_ede)",
			.cra_driver_name = "ecb(des3_ede-iproc)",
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = DES3_EDE_KEY_SIZE,
					   .max_keysize = DES3_EDE_KEY_SIZE,
					   .ivsize = 0,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_3DES,
			 .mode = CIPHER_MODE_ECB,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ofb(aes)",
			.cra_driver_name = "ofb(aes-iproc)",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = AES_MIN_KEY_SIZE,
					   .max_keysize = AES_MAX_KEY_SIZE,
					   .ivsize = AES_BLOCK_SIZE,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_OFB,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "cbc(aes)",
			.cra_driver_name = "cbc(aes-iproc)",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = AES_MIN_KEY_SIZE,
					   .max_keysize = AES_MAX_KEY_SIZE,
					   .ivsize = AES_BLOCK_SIZE,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CBC,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ecb(aes)",
			.cra_driver_name = "ecb(aes-iproc)",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = AES_MIN_KEY_SIZE,
					   .max_keysize = AES_MAX_KEY_SIZE,
					   .ivsize = 0,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_ECB,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	 .alg.crypto = {
			.cra_name = "ctr(aes)",
			.cra_driver_name = "ctr(aes-iproc)",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   /* .geniv = "chainiv", */
					   .min_keysize = AES_MIN_KEY_SIZE,
					   .max_keysize = AES_MAX_KEY_SIZE,
					   .ivsize = AES_BLOCK_SIZE,
					   }
			},
	 .cipher_info = {
			 .alg = CIPHER_ALG_AES,
			 .mode = CIPHER_MODE_CTR,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	 },
	{
	.type = CRYPTO_ALG_TYPE_ABLKCIPHER,
	.alg.crypto = {
			.cra_name = "xts(aes)",
			.cra_driver_name = "xts(aes-iproc)",
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ablkcipher = {
					   .min_keysize = 2 * AES_MIN_KEY_SIZE,
					   .max_keysize = 2 * AES_MAX_KEY_SIZE,
					   .ivsize = AES_BLOCK_SIZE,
					   }
			},
	.cipher_info = {
			.alg = CIPHER_ALG_AES,
			.mode = CIPHER_MODE_XTS,
			},
	.auth_info = {
		       .alg = HASH_ALG_NONE,
		       .mode = HASH_MODE_NONE,
		       },
	},

/* AHASH algorithms. */
	{
	 .type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = MD5_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "md5",
				    .cra_driver_name = "md5-iproc",
				    .cra_blocksize = MD5_BLOCK_WORDS * 4,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_MD5,
		       .mode = HASH_MODE_HASH,
		       },
	 },
	{
	 .type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = MD5_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "hmac(md5)",
				    .cra_driver_name = "hmac(md5-iproc)",
				    .cra_blocksize = MD5_BLOCK_WORDS * 4,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_MD5,
		       .mode = HASH_MODE_HMAC,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA1_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "sha1",
				    .cra_driver_name = "sha1-iproc",
				    .cra_blocksize = SHA1_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA1,
		       .mode = HASH_MODE_HASH,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA1_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "hmac(sha1)",
				    .cra_driver_name = "hmac(sha1-iproc)",
				    .cra_blocksize = SHA1_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA1,
		       .mode = HASH_MODE_HMAC,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA224_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "sha224",
				    .cra_driver_name = "sha224-iproc",
				    .cra_blocksize = SHA224_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA224,
		       .mode = HASH_MODE_HASH,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA224_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "hmac(sha224)",
				    .cra_driver_name = "hmac(sha224-iproc)",
				    .cra_blocksize = SHA224_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA224,
		       .mode = HASH_MODE_HMAC,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA256_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "sha256",
				    .cra_driver_name = "sha256-iproc",
				    .cra_blocksize = SHA256_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA256,
		       .mode = HASH_MODE_HASH,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA256_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "hmac(sha256)",
				    .cra_driver_name = "hmac(sha256-iproc)",
				    .cra_blocksize = SHA256_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA256,
		       .mode = HASH_MODE_HMAC,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA384_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "sha384",
				    .cra_driver_name = "sha384-iproc",
				    .cra_blocksize = SHA384_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA384,
		       .mode = HASH_MODE_HASH,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA384_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "hmac(sha384)",
				    .cra_driver_name = "hmac(sha384-iproc)",
				    .cra_blocksize = SHA384_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA384,
		       .mode = HASH_MODE_HMAC,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA512_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "sha512",
				    .cra_driver_name = "sha512-iproc",
				    .cra_blocksize = SHA512_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA512,
		       .mode = HASH_MODE_HASH,
		       },
	 },
	{.type = CRYPTO_ALG_TYPE_AHASH,
	 .alg.hash = {
		      .halg.digestsize = SHA512_DIGEST_SIZE,
		      /* import/export not supported so statesize is not required
		         However, it needs to be set to pass ahash_prepare_alg */
		      .halg.statesize = 1,
		      .halg.base = {
				    .cra_name = "hmac(sha512)",
				    .cra_driver_name = "hmac(sha512-iproc)",
				    .cra_blocksize = SHA512_BLOCK_SIZE,
				    }
		      },
	 .cipher_info = {
			 .alg = CIPHER_ALG_NONE,
			 .mode = CIPHER_MODE_NONE,
			 },
	 .auth_info = {
		       .alg = HASH_ALG_SHA512,
		       .mode = HASH_MODE_HMAC,
		       },
	 },
};

static int generic_cra_init(struct crypto_tfm *tfm)
{
	struct crypto_alg *alg = tfm->__crt_alg;
	struct iproc_alg_s *cipher_alg;
	struct iproc_ctx_s *ctx = crypto_tfm_ctx(tfm);

	flow_log("%s() name:%s\n", __func__, &alg->cra_driver_name[0]);

	if ((alg->cra_flags & CRYPTO_ALG_TYPE_MASK) == CRYPTO_ALG_TYPE_AHASH)
		cipher_alg =
		    container_of(__crypto_ahash_alg(alg), struct iproc_alg_s,
				 alg.hash);
	else
		cipher_alg = container_of(alg, struct iproc_alg_s, alg.crypto);

	ctx->alg = cipher_alg;
	ctx->cipher = cipher_alg->cipher_info;
	ctx->auth = cipher_alg->auth_info;
	ctx->auth_first = cipher_alg->auth_first;
	ctx->max_payload = cipher_alg->max_payload;

	ctx->fallback_cipher = NULL;

	ctx->enckeylen = 0;
	ctx->authkeylen = 0;

	ctx->chan_idx = pdc_select_channel();
#ifdef SPU_DEBUG_STATS 
	atomic_inc(&iproc_priv.stream_count);
	atomic_inc(&iproc_priv.session_count);
#endif
	return 0;
}

static int ablkcipher_cra_init(struct crypto_tfm *tfm)
{
	flow_log("%s() tfm:%p\n", __func__, tfm);

	tfm->crt_ablkcipher.reqsize = sizeof(struct iproc_reqctx_s);

	return generic_cra_init(tfm);
}

static int ahash_cra_init(struct crypto_tfm *tfm)
{
	int err = generic_cra_init(tfm);

	flow_log("%s() tfm:%p\n", __func__, tfm);

	crypto_ahash_set_reqsize(__crypto_ahash_cast(tfm),
				 sizeof(struct iproc_reqctx_s));

	return err;
}

static int aead_cra_init(struct crypto_tfm *tfm)
{
	struct iproc_ctx_s *ctx = crypto_tfm_ctx(tfm);
	struct crypto_alg *alg = tfm->__crt_alg;

	int err = generic_cra_init(tfm);

	flow_log("%s() tfm:%p\n", __func__, tfm);

	/* random first IV */
	get_random_bytes(ctx->iv, MAX_IV_SIZE);
	flow_dump("  rnd ctx iv: ", ctx->iv, MAX_IV_SIZE-MAX_SALT_SIZE);

	if (!err) {
		if (alg->cra_flags & CRYPTO_ALG_NEED_FALLBACK) {
			flow_log("%s() creating fallback cipher\n", __func__);

			ctx->fallback_cipher =
			    crypto_alloc_aead(alg->cra_name, 0,
					      CRYPTO_ALG_ASYNC |
					      CRYPTO_ALG_NEED_FALLBACK);
			if (IS_ERR(ctx->fallback_cipher)) {
				pr_err(
				       "%s() Error: failed to allocate fallback for %s\n",
				       __func__, alg->cra_name);
				return PTR_ERR(ctx->fallback_cipher);
			}
		}
	}

	/* when fallback is set, the request size must be the sum of our request
	   size and the fallback request size */
	if ( ctx->fallback_cipher)
	{
		tfm->crt_aead.reqsize = crypto_aead_reqsize(ctx->fallback_cipher) +
		                        sizeof(struct iproc_reqctx_s);
	}
	else
	{
		tfm->crt_aead.reqsize = sizeof(struct iproc_reqctx_s);
	}

	ctx->salt_len = 0;
	ctx->salt_offset = 0;
	ctx->is_esp = false;
	ctx->is_rfc4543 = false;

	return err;
}

static void generic_cra_exit(struct crypto_tfm *tfm)
{
#ifdef SPU_DEBUG_STATS 
	atomic_dec(&iproc_priv.session_count);
#endif
}

static void aead_cra_exit(struct crypto_tfm *tfm)
{
	struct iproc_ctx_s *ctx = crypto_tfm_ctx(tfm);

	generic_cra_exit(tfm);

	if (ctx->fallback_cipher) {
		crypto_free_aead(ctx->fallback_cipher);
		ctx->fallback_cipher = NULL;
	}

#if defined(CONFIG_BLOG)
	spu_blog_ctx_del(ctx);
#endif
}

#if defined (CONFIG_BLOG)
/* disable blog */
static void aead_configure_blog( int enable )
{
	int i;
	for (i = 0; i < ARRAY_SIZE(driver_algs); i++)
	{
		if (CRYPTO_ALG_TYPE_AEAD == driver_algs[i].type)
		{
			if ( enable ) 
			{ 
				driver_algs[i].alg.crypto.cra_flags |= CRYPTO_ALG_BLOG;
			}
			else
			{
				driver_algs[i].alg.crypto.cra_flags &= ~CRYPTO_ALG_BLOG;
			}
		}
	}
}
#endif

/* ==================== Kernel Platform API ==================== */

static int spu_dt_read(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *dn = pdev->dev.of_node;
	struct resource *spu_ctrl_regs;
	const void *dt_val_ptr;
	int i;

	if (!of_device_is_available(dn)) {
		dev_crit(dev, "SPU device not available");
		return -ENODEV;
	}

	/* Get number of SPUs from device tree */
	dt_val_ptr = of_get_property(dn, "brcm,num_spu", NULL);
	if (!dt_val_ptr) {
		dev_err(dev,
			"%s failed to get num_spu from device tree",
			__func__);
	} else {
		iproc_priv.num_spu = be32_to_cpup(dt_val_ptr);
		dev_dbg(dev, "Device has %d SPUs",
			 iproc_priv.num_spu);
	}

	for (i = 0; i < iproc_priv.num_spu; i++) {
		spu_ctrl_regs = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (spu_ctrl_regs == NULL)
			return -ENODEV;

		dev_dbg(dev,
			 "SPU %d control register region res.start = %#x, res.end = %#x",
			 i,
			 (unsigned int) spu_ctrl_regs->start,
			 (unsigned int) spu_ctrl_regs->end);
		/* Not currently using the SPU ctrl registers. So for now, no
		 * need to map them.
		 */
	}
	return 0;
}

int iproc_crypto_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int err = 0, i, j;

	flow_log("iproc-crypto: %s()\n", __func__);

	memset(&iproc_priv, 0, sizeof(iproc_priv));
	for (i = 0; i < SPU_STREAM_MAX; i++)
		iproc_priv.ctxListLock[i] = __RW_LOCK_UNLOCKED(iproc_priv.ctxListLock[i]);
	iproc_priv.pdev = pdev;
	platform_set_drvdata(iproc_priv.pdev, &iproc_priv);

#ifdef CONFIG_BCM_GLB_COHERENCY
	dma_set_coherent_mask(dev, DMA_BIT_MASK(32));
	dev->archdata.dma_coherent = 1;
#endif

	err = spu_dt_read(pdev);
	if (err)
		goto failure;

	atomic_long_set(&iproc_priv.bytes_in, 0);
	atomic_long_set(&iproc_priv.bytes_out, 0);
	atomic_set(&iproc_priv.mb_send_fail, 0);
	atomic_set(&iproc_priv.bad_icv, 0);
	atomic_set(&iproc_priv.aead_fallback_cnt, 0);
#ifdef SPU_DEBUG_STATS 
	atomic_set(&iproc_priv.session_count, 0);
	atomic_set(&iproc_priv.stream_count, 0);
	for (i = 0; i < SPU_OP_NUM; i++) {
		atomic_set(&iproc_priv.op_counts[i], 0);
		atomic_set(&iproc_priv.setkey_cnt[i], 0);
	}
#endif
	spu_setup_debugfs(&iproc_priv);

#if defined(CONFIG_BLOG)
	err = spu_blog_register();
	if ( err != 0 )
	{
		goto failure;
	}
#endif

	/* register crypto algorithms the device supports */
	for (i = 0; i < ARRAY_SIZE(driver_algs); i++) {
		char *name = NULL;

		driver_algs[i].max_payload = MAX_SPU_PKT_SIZE -
		                             (SPU_HEADER_ALLOC_LEN + SPU_STATUS_LEN);
		driver_algs[i].max_payload &= 0xFFFFFFC0;

		switch (driver_algs[i].type) {
		case CRYPTO_ALG_TYPE_ABLKCIPHER:
			driver_algs[i].alg.crypto.cra_module = THIS_MODULE;
			/* Higher value is higher priority. The newer software
			 * implementation for CONFIG_ARM64_CRYPTO registers
			 * with priority 300. So use priority 400 to prefer hw
			 * to sw.
			 */
			driver_algs[i].alg.crypto.cra_priority = 400;
			driver_algs[i].alg.crypto.cra_alignmask = 0;
			driver_algs[i].alg.crypto.cra_ctxsize =
			    sizeof(struct iproc_ctx_s);
			INIT_LIST_HEAD(&driver_algs[i].alg.crypto.cra_list);

			driver_algs[i].alg.crypto.cra_init =
			    ablkcipher_cra_init;
			driver_algs[i].alg.crypto.cra_exit = generic_cra_exit;
			driver_algs[i].alg.crypto.cra_type =
			    &crypto_ablkcipher_type;
			driver_algs[i].alg.crypto.cra_flags =
			    CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC |
			    CRYPTO_ALG_KERN_DRIVER_ONLY;

			driver_algs[i].alg.crypto.cra_ablkcipher.setkey =
			    ablkcipher_setkey;
			driver_algs[i].alg.crypto.cra_ablkcipher.encrypt =
			    ablkcipher_encrypt;
			driver_algs[i].alg.crypto.cra_ablkcipher.decrypt =
			    ablkcipher_decrypt;

			err = crypto_register_alg(&driver_algs[i].alg.crypto);
			name = driver_algs[i].alg.crypto.cra_driver_name;
			//pr_info("  registered ablkcipher %s\n", name);
			break;
		case CRYPTO_ALG_TYPE_AHASH:
			driver_algs[i].alg.hash.halg.base.cra_module =
			    THIS_MODULE;
			driver_algs[i].alg.hash.halg.base.cra_priority = 400;
			driver_algs[i].alg.hash.halg.base.cra_alignmask = 0;
			driver_algs[i].alg.hash.halg.base.cra_ctxsize =
			    sizeof(struct iproc_ctx_s);

			driver_algs[i].alg.hash.halg.base.cra_init =
			    ahash_cra_init;
			driver_algs[i].alg.hash.halg.base.cra_exit =
			    generic_cra_exit;
			driver_algs[i].alg.hash.halg.base.cra_type =
			    &crypto_ahash_type;
			driver_algs[i].alg.hash.halg.base.cra_flags =
			    CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC;

			if (driver_algs[i].auth_info.mode != HASH_MODE_HMAC) {
				driver_algs[i].alg.hash.init = ahash_init;
				driver_algs[i].alg.hash.update = ahash_update;
				driver_algs[i].alg.hash.final = ahash_final;
				driver_algs[i].alg.hash.finup = ahash_finup;
				driver_algs[i].alg.hash.digest = ahash_digest;
			} else {
				driver_algs[i].alg.hash.setkey =
				    ahash_hmac_setkey;
				driver_algs[i].alg.hash.init = ahash_hmac_init;
				driver_algs[i].alg.hash.update =
				    ahash_hmac_update;
				driver_algs[i].alg.hash.final =
				    ahash_hmac_final;
				driver_algs[i].alg.hash.finup =
				    ahash_hmac_finup;
				driver_algs[i].alg.hash.digest =
				    ahash_hmac_digest;
			}

			err = crypto_register_ahash(&driver_algs[i].alg.hash);
			name =
			    driver_algs[i].alg.hash.halg.base.cra_driver_name;
			//pr_info("  registered ahash %s\n", name);
			break;
		case CRYPTO_ALG_TYPE_AEAD:
			driver_algs[i].alg.crypto.cra_module = THIS_MODULE;
			driver_algs[i].alg.crypto.cra_priority = 1500;
			driver_algs[i].alg.crypto.cra_alignmask = 0;
			driver_algs[i].alg.crypto.cra_ctxsize =
			    sizeof(struct iproc_ctx_s);
			INIT_LIST_HEAD(&driver_algs[i].alg.crypto.cra_list);

			driver_algs[i].alg.crypto.cra_init = aead_cra_init;
			driver_algs[i].alg.crypto.cra_exit = aead_cra_exit;
			driver_algs[i].alg.crypto.cra_type = &crypto_aead_type;
			driver_algs[i].alg.crypto.cra_flags |= CRYPTO_ALG_TYPE_AEAD | 
			                                       CRYPTO_ALG_ASYNC;
#if defined(CONFIG_BLOG)
			if ( blog_enable )
			{
				driver_algs[i].alg.crypto.cra_flags |= CRYPTO_ALG_BLOG;
			}
#endif
			if ((driver_algs[i].cipher_info.mode != CIPHER_MODE_GCM) &&
			    (driver_algs[i].cipher_info.mode != CIPHER_MODE_CCM)) {
				driver_algs[i].alg.crypto.cra_aead.setkey =
				    aead_authenc_setkey;
			}
			driver_algs[i].alg.crypto.cra_aead.setauthsize =
			    aead_setauthsize;
			driver_algs[i].alg.crypto.cra_aead.encrypt =
			    aead_encrypt;
			driver_algs[i].alg.crypto.cra_aead.decrypt =
			    aead_decrypt;
			driver_algs[i].alg.crypto.cra_aead.givencrypt =
			    aead_givencrypt;

			err = crypto_register_alg(&driver_algs[i].alg.crypto);
			name = driver_algs[i].alg.crypto.cra_driver_name;
			//pr_info("  registered aead %s\n", name);
			break;
		default:
			dev_err(dev,
				"iproc-crypto: unknown alg name: %s type: %d\n",
				name, driver_algs[i].type);
			err = -EINVAL;
		}

		if (err) {
			dev_err(dev, "%s alg registration failed\n", name);
			goto err_algs;
		}
	}

	return 0;

err_algs:
	for (j = 0; j < i; j++) {
		switch (driver_algs[j].type) {
		case CRYPTO_ALG_TYPE_ABLKCIPHER:
		case CRYPTO_ALG_TYPE_AEAD:
			crypto_unregister_alg(&driver_algs[j].alg.crypto);
			break;
		case CRYPTO_ALG_TYPE_AHASH:
			crypto_unregister_ahash(&driver_algs[j].alg.hash);
			break;
		}
	}
failure:
	dev_err(dev, "%s failed with error %d.\n", __func__, err);

	return err;
}

int iproc_crypto_remove(struct platform_device *pdev)
{
	int i;

	pr_info("iproc-crypto: remove()\n");

	for (i = 0; i < ARRAY_SIZE(driver_algs); i++) {
		switch (driver_algs[i].type) {
		case CRYPTO_ALG_TYPE_ABLKCIPHER:
		case CRYPTO_ALG_TYPE_AEAD:
			crypto_unregister_alg(&driver_algs[i].alg.crypto);
			pr_info("  unregistered cipher %s\n",
				driver_algs[i].alg.crypto.cra_driver_name);
			break;
		case CRYPTO_ALG_TYPE_AHASH:
			crypto_unregister_ahash(&driver_algs[i].alg.hash);
			pr_info("  unregistered hash %s\n",
				driver_algs[i].alg.hash.halg.base.
				cra_driver_name);
			break;
		}
	}

#if defined(CONFIG_BLOG)
	spu_blog_unregister();
#endif

	spu_free_debugfs_stats(&iproc_priv);
	spu_free_debugfs(&iproc_priv);

	pr_info("iproc-crypto: remove() done\n");

	return 0;
}

/* ===== Kernel Module API ===== */

static const struct of_device_id bcm_iproc_dt_ids[] = {
	{ .compatible = "brcm,spu-crypto"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, bcm_iproc_dt_ids);

static struct platform_driver crypto_pdriver = {
	.driver = {
		   .name = "bcmspu",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(bcm_iproc_dt_ids),
		   },
	.probe = iproc_crypto_probe,
	.remove = iproc_crypto_remove,
};

static int __init iproc_crypto_init(void)
{
	int rc;

	pr_info("bcmspu: loading driver\n");

	rc = platform_driver_register(&crypto_pdriver);

	if (rc < 0) {
		pr_err("%s: Driver registration failed, error %d\n",
		       __func__, rc);
		return rc;
	}
	return 0;
}

static void __exit iproc_crypto_exit(void)
{
	platform_driver_unregister(&crypto_pdriver);
}

module_init(iproc_crypto_init);
module_exit(iproc_crypto_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
