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

#include <linux/debugfs.h>

#include "cipher.h"
#include "util.h"

/* offset of SPU_OFIFO_CTRL register */
#define SPU_OFIFO_CTRL      0x40
#define SPU_FIFO_WATERMARK  0x1FF

/**
 * spu_sg_at_offset() - Find the scatterlist entry at a given distance from the
 * start of a scatterlist.
 * @sg:         [in]  Start of a scatterlist
 * @skip:       [in]  Distance from the start of the scatterlist, in bytes
 * @sge:        [out] Scatterlist entry at skip bytes from start
 * @sge_offset: [out] Number of bytes from start of sge buffer to get to
 *                    requested distance.
 *
 * Return: 0 if entry found at requested distance
 *         < 0 otherwise
 */
int spu_sg_at_offset(struct scatterlist *sg, unsigned int skip,
		     struct scatterlist **sge, unsigned int *sge_offset)
{
	/* byte index from start of sg to the end of the previous entry */
	unsigned int index = 0;
	/* byte index from start of sg to the end of the current entry */
	unsigned int next_index;

	next_index = sg->length;
	while (next_index <= skip) {
		sg = sg_next(sg);
		index = next_index;
		if (!sg)
			return -EINVAL;
		next_index += sg->length;
	}

	*sge_offset = skip - index;
	*sge = sg;
	return 0;
}

/* Copy len bytes of sg data, starting at offset skip, to a dest buffer */
void sg_copy_part_to_buf(struct scatterlist *src, u8 *dest,
			 unsigned int len, unsigned int skip)
{
	size_t copied;
	unsigned int nents = sg_nents(src);

	copied = sg_pcopy_to_buffer(src, nents, dest, len, skip);
	if (copied != len) {
		flow_log("%s copied %u bytes of %u requested. ",
			 __func__, (u32)copied, len);
		flow_log("sg with %u entries and skip %u\n", nents, skip);
	}
}

/*
 * Copy data into a scatterlist starting at a specified offset in the
 * scatterlist. Specifically, copy len bytes of data in the buffer src
 * into the scatterlist dest, starting skip bytes into the scatterlist.
 */
void sg_copy_part_from_buf(struct scatterlist *dest, u8 *src,
			   unsigned int len, unsigned int skip)
{
	size_t copied;
	unsigned int nents = sg_nents(dest);

	copied = sg_pcopy_from_buffer(dest, nents, src, len, skip);
	if (copied != len) {
		flow_log("%s copied %u bytes of %u requested. ",
			 __func__, (u32)copied, len);
		flow_log("sg with %u entries and skip %u\n", nents, skip);
	}
}

/**
 * spu_sg_count() - Determine number of elements in scatterlist to provide a
 * specified number of bytes.
 * @sg_list:  scatterlist to examine
 * @skip:     index of starting point
 * @nbytes:   consider elements of scatterlist until reaching this number of
 *	      bytes
 *
 * Return: the number of sg entries contributing to nbytes of data
 */
int spu_sg_count(struct scatterlist *sg_list, unsigned int skip, int nbytes)
{
	struct scatterlist *sg;
	int sg_nents = 0;
	unsigned int offset;

	if (!sg_list)
		return 0;

	if (spu_sg_at_offset(sg_list, skip, &sg, &offset) < 0)
		return 0;

	while (sg && (nbytes > 0)) {
		sg_nents++;
		nbytes -= (sg->length - offset);
		offset = 0;
		sg = sg_next(sg);
	}
	return sg_nents;
}

/**
 * spu_msg_sg_add() - Copy scatterlist entries from one sg to another, up to a
 * given length.
 * @to_sg:       scatterlist to copy to
 * @from_sg:     scatterlist to copy from
 * @from_skip:   number of bytes to skip in from_sg. Non-zero when previous
 *		 request included part of the buffer in entry in from_sg.
 *		 Assumes from_skip < from_sg->length.
 * @from_nents   number of entries in from_sg
 * @length       number of bytes to copy. may reach this limit before exhausting
 *		 from_sg.
 *
 * Copies the entries themselves, not the data in the entries. Assumes to_sg has
 * enough entries. Does not limit the size of an individual buffer in to_sg.
 *
 * to_sg, from_sg, skip are all updated to end of copy
 *
 * Return: Number of bytes copied
 */
u32 spu_msg_sg_add(struct scatterlist **to_sg,
		   struct scatterlist **from_sg, u32 *from_skip,
		   u8 from_nents, u32 length)
{
	struct scatterlist *sg;	/* an entry in from_sg */
	struct scatterlist *to = *to_sg;
	struct scatterlist *from = *from_sg;
	u32 skip = *from_skip;
	u32 offset;
	int i;
	u32 entry_len = 0;
	u32 frag_len = 0;	/* length of entry added to to_sg */
	u32 copied = 0;		/* number of bytes copied so far */

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
			skip = 0;	/* start at beginning of next entry */
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

void add_to_ctr(u8 *ctr_pos, unsigned int increment)
{
	__be64 *high_be = (__be64 *)ctr_pos;
	__be64 *low_be = high_be + 1;
	u64 orig_low = __be64_to_cpu(*low_be);
	u64 new_low = orig_low + (u64)increment;

	*low_be = __cpu_to_be64(new_low);
	if (new_low < orig_low)
		/* there was a carry from the low 8 bytes */
		*high_be = __cpu_to_be64(__be64_to_cpu(*high_be) + 1);
}

struct sdesc {
	struct shash_desc shash;
	char ctx[];
};

/**
 * do_shash() - Do a synchronous hash operation in software
 * @name:       The name of the hash algorithm
 * @result:     Buffer where digest is to be written
 * @data1:      First part of data to hash. May be NULL.
 * @data1_len:  Length of data1, in bytes
 * @data2:      Second part of data to hash. May be NULL.
 * @data2_len:  Length of data2, in bytes
 * @key:	Key (if keyed hash)
 * @key_len:	Length of key, in bytes (or 0 if non-keyed hash)
 *
 * Note that the crypto API will not select this driver's own transform because
 * this driver only registers asynchronous algos.
 *
 * Return: 0 if hash successfully stored in result
 *         < 0 otherwise
 */
int do_shash(unsigned char *name, unsigned char *result,
	     const u8 *data1, unsigned int data1_len,
	     const u8 *data2, unsigned int data2_len,
	     const u8 *key, unsigned int key_len)
{
	int rc;
	unsigned int size;
	struct crypto_shash *hash;
	struct sdesc *sdesc;

	hash = crypto_alloc_shash(name, 0, 0);
	if (IS_ERR(hash)) {
		rc = PTR_ERR(hash);
		pr_err("%s: Crypto %s allocation error %d\n", __func__, name, rc);
		return rc;
	}

	size = sizeof(struct shash_desc) + crypto_shash_descsize(hash);
	sdesc = kmalloc(size, GFP_KERNEL);
	if (!sdesc) {
		rc = -ENOMEM;
		goto do_shash_err;
	}
	sdesc->shash.tfm = hash;

	if (key_len > 0) {
		rc = crypto_shash_setkey(hash, key, key_len);
		if (rc) {
			pr_err("%s: Could not setkey %s shash\n", __func__, name);
			goto do_shash_err;
		}
	}

	rc = crypto_shash_init(&sdesc->shash);
	if (rc) {
		pr_err("%s: Could not init %s shash\n", __func__, name);
		goto do_shash_err;
	}
	rc = crypto_shash_update(&sdesc->shash, data1, data1_len);
	if (rc) {
		pr_err("%s: Could not update1\n", __func__);
		goto do_shash_err;
	}
	if (data2 && data2_len) {
		rc = crypto_shash_update(&sdesc->shash, data2, data2_len);
		if (rc) {
			pr_err("%s: Could not update2\n", __func__);
			goto do_shash_err;
		}
	}
	rc = crypto_shash_final(&sdesc->shash, result);
	if (rc)
		pr_err("%s: Could not generate %s hash\n", __func__, name);

do_shash_err:
	crypto_free_shash(hash);
	kfree(sdesc);

	return rc;
}

/* Dump len bytes of a scatterlist starting at skip bytes into the sg */
void __dump_sg(struct scatterlist *sg, unsigned int skip, unsigned int len)
{
	u8 dbuf[16];
	unsigned int idx = skip;
	unsigned int num_out = 0;	/* number of bytes dumped so far */
	unsigned int count;

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
	case CIPHER_ALG_RC4:
		return "rc4";
	case CIPHER_ALG_AES:
		switch (mode) {
		case CIPHER_MODE_CBC:
			return "cbc(aes)";
		case CIPHER_MODE_ECB:
			return "ecb(aes)";
		case CIPHER_MODE_OFB:
			return "ofb(aes)";
		case CIPHER_MODE_CFB:
			return "cfb(aes)";
		case CIPHER_MODE_CTR:
			return "ctr(aes)";
		case CIPHER_MODE_XTS:
			return "xts(aes)";
		case CIPHER_MODE_GCM:
			return "gcm(aes)";
		default:
			return "aes";
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
			return "des";
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
			return "3des";
		}
		break;
	default:
		return "other";
	}
}

static ssize_t spu_debugfs_read(struct file *file, char __user *ubuf,
				size_t count, loff_t *offp)
{
	struct device_private *ipriv;
	char *buf;
	ssize_t ret, out_offset, out_count;
	int i;
	u32 fifo_len;
	u32 spu_ofifo_ctrl;
	u32 alg;
	u32 mode;
	u32 op_cnt;

	out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ipriv = file->private_data;
	out_offset = 0;
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Number of SPUs.........%u\n",
			       ipriv->spu.num_spu);
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Current sessions.......%u\n",
			       atomic_read(&ipriv->session_count));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Session count..........%u\n",
			       atomic_read(&ipriv->stream_count));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Cipher setkey..........%u\n",
			       atomic_read(&ipriv->setkey_cnt[SPU_OP_CIPHER]));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Cipher Ops.............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_CIPHER]));
	for (alg = 0; alg < CIPHER_ALG_LAST; alg++) {
		for (mode = 0; mode < CIPHER_MODE_LAST; mode++) {
			op_cnt = atomic_read(&ipriv->cipher_cnt[alg][mode]);
			if (op_cnt) {
				out_offset += scnprintf(buf + out_offset,
						       out_count - out_offset,
			       "  %-21s%-11u\n",
			       spu_alg_name(alg, mode), op_cnt);
			}
		}
	}
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Hash Ops...............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_HASH]));
	for (alg = 0; alg < HASH_ALG_LAST; alg++) {
		op_cnt = atomic_read(&ipriv->hash_cnt[alg]);
		if (op_cnt) {
			out_offset += scnprintf(buf + out_offset,
					       out_count - out_offset,
		       "  %-21s%-11u\n",
		       hash_alg_name[alg], op_cnt);
		}
	}
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "HMAC setkey............%u\n",
			       atomic_read(&ipriv->setkey_cnt[SPU_OP_HMAC]));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "HMAC Ops...............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_HMAC]));
	for (alg = 0; alg < HASH_ALG_LAST; alg++) {
		op_cnt = atomic_read(&ipriv->hmac_cnt[alg]);
		if (op_cnt) {
			out_offset += scnprintf(buf + out_offset,
					       out_count - out_offset,
		       "  %-21s%-11u\n",
		       hash_alg_name[alg], op_cnt);
		}
	}
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "AEAD setkey............%u\n",
			       atomic_read(&ipriv->setkey_cnt[SPU_OP_AEAD]));

	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "AEAD Ops...............%u\n",
			       atomic_read(&ipriv->op_counts[SPU_OP_AEAD]));
	for (alg = 0; alg < AEAD_TYPE_LAST; alg++) {
		op_cnt = atomic_read(&ipriv->aead_cnt[alg]);
		if (op_cnt) {
			out_offset += scnprintf(buf + out_offset,
					       out_count - out_offset,
		       "  %-21s%-11u\n",
		       aead_alg_name[alg], op_cnt);
		}
	}
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Bytes of req data......%llu\n",
			       (u64)atomic64_read(&ipriv->bytes_out));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Bytes of resp data.....%llu\n",
			       (u64)atomic64_read(&ipriv->bytes_in));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Channel full...........%u\n",
			       atomic_read(&ipriv->chan_no_spc));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Channel send failures..%u\n",
			       atomic_read(&ipriv->chan_send_fail));
	out_offset += scnprintf(buf + out_offset, out_count - out_offset,
			       "Check ICV errors.......%u\n",
			       atomic_read(&ipriv->bad_icv));
#if defined(CONFIG_BLOG)
		out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				       "Packets blogged (us)...%u\n",
				       atomic_read(&ipriv->blogged[SPU_STREAM_US]));
		out_offset += scnprintf(buf + out_offset, out_count - out_offset,
				       "                (ds)...%u\n",
				       atomic_read(&ipriv->blogged[SPU_STREAM_DS]));
#endif
	if (ipriv->spu.spu_type == SPU_TYPE_SPUM)
		for (i = 0; i < ipriv->spu.num_spu; i++) {
			spu_ofifo_ctrl = ioread32(ipriv->spu.reg_vbase[i] +
						  SPU_OFIFO_CTRL);
			fifo_len = spu_ofifo_ctrl & SPU_FIFO_WATERMARK;
			out_offset += scnprintf(buf + out_offset,
					       out_count - out_offset,
				       "SPU %d output FIFO high water.....%u\n",
				       i, fifo_len);
		}

	if (out_offset > out_count)
		out_offset = out_count;

	ret = simple_read_from_buffer(ubuf, count, offp, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_stats = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = spu_debugfs_read,
};

#if defined(SPU_TEST_RAW_PERF)
/*
 * "test_size" specifies how many crypto requests will be simulated
 * in each test run. If test_size is less than or equal to
 * SPU2_TEST_BATCHSIZE, crypto results are compared with expected
 * results, regardless of what value "spu_test_check_output" is set to.
 */
static long long test_size = 0;
static ssize_t spu_debugfs_test_start_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct device_private *ipriv = file->private_data;
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	test_size = simple_strtoll(buf, NULL, 10);
	ipriv->spu.spu_test_raw_perf(test_size);

	return count;
}

static const struct file_operations spu_debugfs_test_start = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_start_write,
};

int spu_test_preinit = 0;
static ssize_t spu_debugfs_test_preinit_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_preinit = simple_strtol(buf, NULL, 10);
	return count;
}

static ssize_t spu_debugfs_test_preinit_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t ret, out_offset = 0, out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	out_offset += snprintf(buf + out_offset, out_count - out_offset,
		"%d\n", spu_test_preinit);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}
static const struct file_operations spu_debugfs_test_preinit = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_preinit_write,
	.read = spu_debugfs_test_preinit_read,
};

int spu_test_dedicated_pool = 1;
static ssize_t spu_debugfs_test_dedicated_pool_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_dedicated_pool = simple_strtol(buf, NULL, 10);
	return count;
}

static ssize_t spu_debugfs_test_dedicated_pool_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t ret, out_offset = 0, out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	out_offset += snprintf(buf + out_offset, out_count - out_offset,
		"%d\n", spu_test_dedicated_pool);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_test_dedicated_pool = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_dedicated_pool_write,
	.read = spu_debugfs_test_dedicated_pool_read,
};

int spu_test_verbose = 1;
static ssize_t spu_debugfs_test_verbose_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_verbose = simple_strtol(buf, NULL, 10);
	return count;
}

static ssize_t spu_debugfs_test_verbose_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t ret, out_offset = 0, out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	out_offset += snprintf(buf + out_offset, out_count - out_offset,
		"%d\n", spu_test_verbose);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_test_verbose = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_verbose_write,
	.read = spu_debugfs_test_verbose_read,
};

unsigned long spu_test_in_transit_limit = ULONG_MAX;
static ssize_t spu_debugfs_test_in_transit_limit_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_in_transit_limit = simple_strtoul(buf, NULL, 10);
	if (spu_test_in_transit_limit == 0)
		spu_test_in_transit_limit = ULONG_MAX;
	return count;
}

static ssize_t spu_debugfs_test_in_transit_limit_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t ret, out_offset = 0, out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (spu_test_in_transit_limit == ULONG_MAX)
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
			"0 = None\n");
	else
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
			"%ld\n", spu_test_in_transit_limit);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_test_in_transit_limit = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_in_transit_limit_write,
	.read = spu_debugfs_test_in_transit_limit_read,
};

int spu_test_vector_id = 1;
static ssize_t spu_debugfs_test_vector_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_vector_id = simple_strtol(buf, NULL, 10);
	return count;
}


static ssize_t spu_debugfs_test_vector_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	struct device_private *ipriv = file->private_data;
	char *buf;
	int i, max;
	ssize_t ret, out_offset = 0, out_count = 2048;
	struct spu_test_vector_t *test_vector;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	max = ipriv->spu.spu_get_test_vectors(&test_vector);
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
			"Test Vector Options:\n");
	for (i = 0; i < max; i++) {
		out_offset += snprintf(buf + out_offset, out_count - out_offset,
			" %s Vector #%d: %s\n", (i == spu_test_vector_id)? "*" : " ",
			i, test_vector[i].name);
	}
	out_offset += snprintf(buf + out_offset, out_count - out_offset,
		" %s Vector #%d: Randomly selected from the list above for every request\n",
		((spu_test_vector_id >= max) || (spu_test_vector_id < 0))? "*" : " ", max);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_test_vector = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_vector_write,
	.read = spu_debugfs_test_vector_read,
};

int spu_test_src_frag = 1;
static ssize_t spu_debugfs_test_src_frag_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_src_frag = simple_strtol(buf, NULL, 10);
	return count;
}

static ssize_t spu_debugfs_test_src_frag_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t ret, out_offset = 0, out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	out_offset += snprintf(buf + out_offset, out_count - out_offset,
		"%d\n", spu_test_src_frag);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_test_src_frag = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_src_frag_write,
	.read = spu_debugfs_test_src_frag_read,
};

int spu_test_check_output = 1;
static ssize_t spu_debugfs_test_check_output_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_check_output = simple_strtol(buf, NULL, 10);
	return count;
}

static ssize_t spu_debugfs_test_check_output_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t ret, out_offset = 0, out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	out_offset += snprintf(buf + out_offset, out_count - out_offset,
		"%d\n", spu_test_check_output);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_test_check_output = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_check_output_write,
	.read = spu_debugfs_test_check_output_read,
};

int spu_test_same_src_buffer = 0;
static ssize_t spu_debugfs_test_same_src_buffer_write (struct file *file,
			    const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t buf_size;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
	    return -EFAULT;

	buf[buf_size] = '\0';
	spu_test_same_src_buffer = simple_strtol(buf, NULL, 10);
	return count;
}

static ssize_t spu_debugfs_test_same_src_buffer_read (struct file *file,
			    char __user *user_buf, size_t count, loff_t *ppos)
{
	char *buf;
	ssize_t ret, out_offset = 0, out_count = 2048;

	buf = kmalloc(out_count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	out_offset += snprintf(buf + out_offset, out_count - out_offset,
		"%d\n", spu_test_same_src_buffer);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, out_offset);
	kfree(buf);
	return ret;
}

static const struct file_operations spu_debugfs_test_same_src_buffer = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.write = spu_debugfs_test_same_src_buffer_write,
	.read = spu_debugfs_test_same_src_buffer_read,
};
#endif

/*
 * Create the debug FS directories. If the top-level directory has not yet
 * been created, create it now. Create a stats file in this directory for
 * a SPU.
 */
void spu_setup_debugfs(void)
{
	if (!debugfs_initialized())
		return;

	if (!iproc_priv.debugfs_dir)
		iproc_priv.debugfs_dir = debugfs_create_dir(KBUILD_MODNAME,
							    NULL);

	if (!iproc_priv.debugfs_stats)
		/* Create file with permissions S_IRUSR */
		debugfs_create_file("stats", 0400, iproc_priv.debugfs_dir,
				    &iproc_priv, &spu_debugfs_stats);

#if defined(SPU_TEST_RAW_PERF)
	if (iproc_priv.spu.spu_test_raw_perf)
	{
		if (!iproc_priv.debugfs_test_dir)
			iproc_priv.debugfs_test_dir = debugfs_create_dir("test",
								    						 iproc_priv.debugfs_dir);

		if (!iproc_priv.debugfs_test_start)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("start", 0200, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_start);

		if (!iproc_priv.debugfs_test_vector)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("vector", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_vector);

		if (!iproc_priv.debugfs_test_preinit)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("preinit", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_preinit);

		if (!iproc_priv.debugfs_test_dedicated_pool)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("dedicated_pool", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_dedicated_pool);

		if (!iproc_priv.debugfs_test_verbose)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("verbose", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_verbose);

		if (!iproc_priv.debugfs_test_in_transit_limit)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("in_transit_limit", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_in_transit_limit);

		if (!iproc_priv.debugfs_test_src_frag)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("src_frag", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_src_frag);

		if (!iproc_priv.debugfs_test_check_output)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("check_output", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_check_output);

		if (!iproc_priv.debugfs_test_same_src_buffer)
			/* Create file with permissions S_IRUSR */
			debugfs_create_file("same_src_buffer", 0600, iproc_priv.debugfs_test_dir,
					    &iproc_priv, &spu_debugfs_test_same_src_buffer);

	}
#endif
}

void spu_free_debugfs(void)
{
	debugfs_remove_recursive(iproc_priv.debugfs_dir);
	iproc_priv.debugfs_dir = NULL;
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

	/* First clear full output buffer */
	memset(buf, 0, len);

	/* Then, starting from right side, fill in with data */
	for (i = 0; i < len; i++) {
		buf[len - i - 1] = (val >> (8 * i)) & 0xff;
		if (i >= 3)
			break;  /* Only handle up to 32 bits of 'val' */
	}
}
