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

#include <linux/debugfs.h>

#include "cipher.h"
#include "util.h"

/* Copy sg data, from skip, length len, to dest */
void sg_copy_part_to_buf(struct scatterlist *src, u8 *dest,
			 unsigned int len, unsigned skip)
{
	unsigned index = 0;
	unsigned next_index;
	unsigned copied_len = 0;
	unsigned end = skip + len;

	next_index = src->length;

	while (src && next_index < skip) {
		src = sg_next(src);
		index = next_index;

		if (!src)
			break;
		next_index += src->length;
	}

	while (src && index < end) {
		unsigned offset_from_page_start = skip - index;
		unsigned page_copy_len =
		    min(end - index, src->length) - offset_from_page_start;

		memcpy(dest + copied_len,
		       (u8 *) sg_virt(src) + offset_from_page_start,
		       page_copy_len);

		copied_len += page_copy_len;
		src = sg_next(src);
		skip = index = next_index;

		if (!src)
			break;
		next_index += src->length;
	}
}

/*
 * Copy data into a scatterlist starting at a specified offset in the
 * scatterlist. Specifically, copy len bytes of data in the buffer src
 * into the scatterlist dest, starting skip bytes into the scatterlist.
 */
void sg_copy_part_from_buf(struct scatterlist *dest, u8 *src,
			   unsigned len, unsigned skip)
{
	/* byte index at the end of the previous scatterlist entry */
	unsigned index = 0;

	/* byte index at the end of the current scatterlist entry */
	unsigned next_index;

	/* byte index in scatterlist at end of copy */
	unsigned end = skip + len;
	unsigned copied_len = 0;    /* number of bytes copied so far */

	next_index = dest->length;

	/* Find the scatterlist entry at skip bytes from the start of
	 * the scatter table
	 */
	while (dest && next_index < skip) {
		dest = sg_next(dest);
		index = next_index;

		if (!dest)
			break;
		next_index += dest->length;
	}

	while (dest && index < end) {
		unsigned offset_from_page_start = skip - index;
		unsigned page_copy_len =
		    min(end - index, dest->length) - offset_from_page_start;

		memcpy((u8 *) sg_virt(dest) + offset_from_page_start,
		       src + copied_len, page_copy_len);

		copied_len += page_copy_len;
		dest = sg_next(dest);
		skip = index = next_index;

		if (!dest)
			break;
		next_index += dest->length;
	}
}

/*
 * Determine number of elements in scatterlist to provide a specified
 * number of bytes.
 * Inputs:
 *   sg_list - scatterlist to examine
 *   nbytes  - consider elements of scatterlist until reaching this
 *             number of bytes
 *
 * Returns the number of entries contributing to nbytes of data
 */
int spu_sg_count(struct scatterlist *sg_list, int nbytes, int skip)
{
	struct scatterlist *sg = sg_list;
	int sg_nents = 0;

	while (sg && (nbytes > 0)) {
		sg_nents++;
		/* account for the data being skipped in the first sg */
		nbytes -= (sg->length - skip);
		skip = 0;
		sg = sg_next(sg);
	}

	return sg_nents;
}

/* Copy scatterlist entries from one sg to another, up to a given length.
 * Copies the entries themselves, not the data in the entries. Assumes
 * to_sg has enough entries. Does not limit the size of an individual buffer
 * in to_sg.
 *
 * Inputs:
 *   to_sg       - scatterlist to copy to
 *   from_sg     - scatterlist to copy from
 *   from_skip   - number of bytes to skip in from_sg. Non-zero when previous
 *                 request included part of the buffer in entry in from_sg.
 *   from_nents  - number of entries in from_sg
 *   length      - number of bytes of buffers to copy. may reach this limit
 *                 before exhausting from_sg.
 *
 *
 * Outputs:
 *   to_sg, from_sg, skip are all updated to end of copy
 *
 * Returns: Number of bytes copied
 */
u32 spu_msg_sg_add(struct scatterlist **to_sg,
		   struct scatterlist **from_sg, u32 *from_skip,
		   u8 from_nents, u32 length)
{
	struct scatterlist *sg;  /* an entry in from_sg */
	struct scatterlist *to = *to_sg;
	struct scatterlist *from = *from_sg;
	u32 skip = *from_skip;
	u32 offset;
	int i;
	u32 entry_len = 0;
	u32 frag_len = 0;  /* length of entry added to to_sg */
	u32 copied = 0;    /* number of bytes copied so far */

	if (length == 0)
		return 0;

	for_each_sg(from, sg, from_nents, i) {
		/* number of bytes in this from entry not yet used */
		entry_len = sg->length - skip;
		frag_len = min(entry_len, length - copied);
		offset = sg->offset + skip;
		if (frag_len)
			sg_set_page(to++, sg_page(sg), frag_len, offset);
		copied += frag_len;
		if (copied == entry_len) {
			/* used up all of from entry */
			skip = 0;  /* start at beginning of next entry */
		}
		if (copied == length)
			break;
	}
	*to_sg = to;
	*from_sg = sg;
	if (frag_len < entry_len)
		*from_skip = skip + frag_len;
	else
		*from_skip = 0;

	return copied;
}

void add_to_ctr(uint8_t *ctr_pos, unsigned increment)
{
	__be64 *high_be = (__be64 *) ctr_pos;
	__be64 *low_be = high_be + 1;
	uint64_t orig_low = __be64_to_cpu(*low_be);
	uint64_t new_low = orig_low + (uint64_t) increment;

	*low_be = __cpu_to_be64(new_low);
	if (new_low < orig_low)
		/* there was a carry from the low 8 bytes */
		*high_be = __cpu_to_be64(__be64_to_cpu(*high_be) + 1);
}

struct sdesc {
	struct shash_desc shash;
	char ctx[];
};

/* do a synchronous decrypt operation */
int do_decrypt(char *alg_name,
	       void *key_ptr, unsigned key_len,
	       void *iv_ptr, void *src_ptr, void *dst_ptr, unsigned block_len)
{
	struct scatterlist sg_in[1], sg_out[1];
	struct crypto_blkcipher *tfm =
	    crypto_alloc_blkcipher(alg_name, 0, CRYPTO_ALG_ASYNC);
	struct blkcipher_desc desc = {.tfm = tfm, .flags = 0 };
	int ret = 0;
	void *iv;
	int ivsize;

	flow_log("%s() name:%s block_len:%u\n", __func__, alg_name,
		 block_len);

	if (IS_ERR(tfm))
		return PTR_ERR(tfm);

	crypto_blkcipher_setkey((void *)tfm, key_ptr, key_len);

	sg_init_table(sg_in, 1);
	sg_set_buf(sg_in, src_ptr, block_len);

	sg_init_table(sg_out, 1);
	sg_set_buf(sg_out, dst_ptr, block_len);

	iv = crypto_blkcipher_crt(tfm)->iv;
	ivsize = crypto_blkcipher_ivsize(tfm);
	memcpy(iv, iv_ptr, ivsize);

	ret = crypto_blkcipher_decrypt(&desc, sg_out, sg_in, block_len);
	crypto_free_blkcipher(tfm);

	if (ret < 0)
		pr_err("aes_decrypt failed %d\n", ret);

	return ret;
}

/* produce a message digest from data of length n bytes */
int do_shash(unsigned char *name, unsigned char *result,
	     const uint8_t *data1, unsigned data1_len,
	     const uint8_t *data2, unsigned data2_len)
{
	int rc;
	unsigned size;
	struct crypto_shash *hash;
	struct sdesc *sdesc;

	hash = crypto_alloc_shash(name, 0, 0);
	if (IS_ERR(hash)) {
		rc = PTR_ERR(hash);
		pr_err("%s: Crypto %s allocation error %d", __func__,
		       name, rc);
		return rc;
	}

	size = sizeof(struct shash_desc) + crypto_shash_descsize(hash);
	sdesc = kmalloc(size, GFP_KERNEL);
	if (!sdesc) {
		rc = -ENOMEM;
		pr_err("%s: Memory allocation failure", __func__);
		goto do_shash_err;
	}
	sdesc->shash.tfm = hash;
	sdesc->shash.flags = 0x0;

	rc = crypto_shash_init(&sdesc->shash);
	if (rc) {
		pr_err("%s: Could not init %s shash", __func__, name);
		goto do_shash_err;
	}
	rc = crypto_shash_update(&sdesc->shash, data1, data1_len);
	if (rc) {
		pr_err("%s: Could not update1", __func__);
		goto do_shash_err;
	}
	if (data2 && data2_len) {
		rc = crypto_shash_update(&sdesc->shash, data2, data2_len);
		if (rc) {
			pr_err("%s: Could not update2", __func__);
			goto do_shash_err;
		}
	}
	rc = crypto_shash_final(&sdesc->shash, result);
	if (rc)
		pr_err("%s: Could not genereate %s hash", __func__,
		       name);

do_shash_err:
	crypto_free_shash(hash);
	kfree(sdesc);

	return rc;
}

/* Dump len bytes of a scatterlist starting at skip bytes into the sg */
void __dump_sg(struct scatterlist *sg, unsigned skip, unsigned len)
{
	uint8_t dbuf[16];
	unsigned idx = skip;
	unsigned num_out = 0;  /* number of bytes dumped so far */
	unsigned count;

	if (packet_debug_logging) {
		while (num_out < len) {
			count = (len - num_out > 16) ? 16 : len - num_out;
			sg_copy_part_to_buf(sg, dbuf, count, idx);
			num_out += count;
			print_hex_dump(KERN_ALERT, "  sg: ", DUMP_PREFIX_NONE,
				       4, 1, dbuf, count, false);
			idx += 16;
		}
	}
	if (debug_logging_sleep)
		msleep(debug_logging_sleep);
}

/* Returns the name for a given cipher alg/mode */
char *spu_alg_name(enum spu_cipher_alg alg, enum spu_cipher_mode mode)
{
	switch (alg) {
	case CIPHER_ALG_AES:
		switch (mode) {
		case CIPHER_MODE_CBC:
			return "cbc(aes)";
		case CIPHER_MODE_ECB:
			return "ecb(aes)";
		case CIPHER_MODE_CTR:
			return "ctr(aes)";
		case CIPHER_MODE_XTS:
			return "xts(aes)";
		case CIPHER_MODE_GCM:
			return "gcm(aes)";
		case CIPHER_MODE_CCM:
			return "ccm(aes)";
		default:
			return "";
		}
		break;
	case CIPHER_ALG_DES:
		switch (mode) {
		case CIPHER_MODE_CBC:
			return "cbc(des)";
		case CIPHER_MODE_ECB:
			return "ecb(des)";
		case CIPHER_MODE_CTR:
			return "ctr(des)";
		default:
			return "";
		}
		break;
	case CIPHER_ALG_3DES:
		switch (mode) {
		case CIPHER_MODE_CBC:
			return "cbc(des3_ede)";
		case CIPHER_MODE_ECB:
			return "ecb(des3_ede)";
		case CIPHER_MODE_CTR:
			return "ctr(des3_ede)";
		default:
			return "";
		}
		break;
	default:
		return "";
	}
}

static ssize_t spu_debugfs_read(struct file *filp, char __user *ubuf,
				size_t count, loff_t *offp)
{
	struct spu_private *ipriv;
	char *buf;
	ssize_t ret, out_offset, out_count;

	out_count = 1024;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ipriv = filp->private_data;
	out_offset = 0;
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Number of SPUs.........%u\n",
			       ipriv->num_spu);
#ifdef SPU_DEBUG_STATS
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Current sessions.......%u\n",
			       atomic_read(&ipriv->session_count));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Session count..........%u\n",
			       atomic_read(&ipriv->stream_count));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Cipher setkey..........%u\n",
			       atomic_read(&ipriv->setkey_cnt[SPU_OP_CIPHER]));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "HMAC setkey............%u\n",
			       atomic_read(&ipriv->setkey_cnt[SPU_OP_HMAC]));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "AEAD setkey............%u\n",
			       atomic_read(&ipriv->setkey_cnt[SPU_OP_AEAD]));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Cipher Ops.............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_CIPHER]));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Hash Ops...............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_HASH]));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "HMAC Ops...............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_HMAC]));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "AEAD Ops...............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_AEAD]));
#endif
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Bytes of req data......%lu\n",
			       atomic_long_read(&ipriv->bytes_out));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Bytes of resp data.....%lu\n",
			       atomic_long_read(&ipriv->bytes_in));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Message send failures..%u\n",
			       atomic_read(&ipriv->mb_send_fail));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "Check ICV errors.......%u\n",
			       atomic_read(&ipriv->bad_icv));
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			       "AEAD fallback Ops......%u\n",
			       atomic_read(&ipriv->aead_fallback_cnt));

	if (out_offset > out_count)
		out_offset = out_count;

	ret = simple_read_from_buffer(ubuf, count, offp, buf, out_offset);
	kfree(buf);
	return ret;
}

/*
 * Create the debug FS directories. If the top-level directory has not yet
 * been created, create it now. Create a stats file in this directory for
 * a SPU.
 */
void spu_setup_debugfs(struct spu_private *iproc_priv)
{
	if (!debugfs_initialized())
		return;

	if (!iproc_priv->debugfs_dir)
	{
		iproc_priv->debugfs_dir = debugfs_create_dir(KBUILD_MODNAME, NULL);
	}

	iproc_priv->debugfs_fo.owner = THIS_MODULE;
	iproc_priv->debugfs_fo.open = simple_open;
	iproc_priv->debugfs_fo.read = spu_debugfs_read;
	if (!iproc_priv->debugfs_stats)
	{
		debugfs_create_file("stats", S_IRUSR, iproc_priv->debugfs_dir,
				    iproc_priv, &iproc_priv->debugfs_fo);
	}
}

void spu_free_debugfs(struct spu_private *iproc_priv)
{
	if (iproc_priv->debugfs_dir && simple_empty(iproc_priv->debugfs_dir)) {
		debugfs_remove(iproc_priv->debugfs_dir);
		iproc_priv->debugfs_dir = NULL;
	}
}

void spu_free_debugfs_stats(struct spu_private *iproc_priv)
{
	debugfs_remove(iproc_priv->debugfs_stats);
	iproc_priv->debugfs_stats = NULL;
}

/*
 * Allocate DMA buffers for various SPU message fragments.
 * Returns 0 if all allocations are successful.
 */
int spu_dma_bufs_alloc(struct iproc_reqctx_s *rctx)
{
	memset(&rctx->pctx_data, 0, sizeof(rctx->pctx_data));

	rctx->pctx.bcm_spu_req_hdr = &rctx->pctx_data[0];
	rctx->pctx.spu_resp_hdr = rctx->pctx.bcm_spu_req_hdr + ALIGN(SPU_HEADER_ALLOC_LEN, 4);
	rctx->pctx.spu_resp_assoc_data = rctx->pctx.spu_resp_hdr + ALIGN(SPU_RESP_HDR_LEN, 4);
	rctx->pctx.datapad = rctx->pctx.spu_resp_assoc_data + MAX_ASSOC_SIZE + MAX_IV_SIZE;
	rctx->pctx.digest = rctx->pctx.datapad + AES_BLOCK_SIZE;
	rctx->pctx.supdt = rctx->pctx.digest + MAX_DIGEST_SIZE;
	rctx->pctx.spu_req_aad_pad = rctx->pctx.supdt + SPU_SUPDT_LEN;
	rctx->pctx.spu_req_pad = rctx->pctx.spu_req_aad_pad + AES_BLOCK_SIZE;
	rctx->pctx.tx_stat = rctx->pctx.spu_req_pad + SPU_PAD_LEN_MAX;
	rctx->pctx.rx_stat_pad = rctx->pctx.tx_stat + SPU_STATUS_LEN;
	rctx->pctx.rx_stat = rctx->pctx.rx_stat_pad + SPU_STAT_PAD_MAX;
	return 0;
}

/**
 * format_value_ccm() - Format a value into a buffer, using a specified number
 *			of bytes (i.e. maybe writing value X into a 4 byte
 *			buffer, or maybe into a 12 byte buffer), as per the
 *			SPU CCM spec.
 *
 * @val:		value to write (up to max of unsigned int)
 * @buf:		(pointer to) buffer to write the value
 * @len:		number of bytes to use (0 to 255)
 *
 */
void format_value_ccm(unsigned int val, u8 *buf, u8 len)
{
	int i;

	if ( len > sizeof (val) )
	{
		memset(buf, 0, len);
		len = sizeof(val);
	}
	/* Then, starting from right side, fill in with data */
	for (i = 0; i < len; i++) {
		buf[len - i - 1] = (val >> (8 * i)) & 0xff;
	}
}
