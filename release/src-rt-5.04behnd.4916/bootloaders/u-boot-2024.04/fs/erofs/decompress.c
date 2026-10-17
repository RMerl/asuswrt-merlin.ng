// SPDX-License-Identifier: GPL-2.0+
#include "decompress.h"

#if IS_ENABLED(CONFIG_LZ4)
#include <u-boot/lz4.h>
static int z_erofs_decompress_lz4(struct z_erofs_decompress_req *rq)
{
	int ret = 0;
	char *dest = rq->out;
	char *src = rq->in;
	char *buff = NULL;
	bool support_0padding = false;
	unsigned int inputmargin = 0;

	if (erofs_sb_has_lz4_0padding()) {
		support_0padding = true;

		while (!src[inputmargin & (erofs_blksiz() - 1)])
			if (!(++inputmargin & (erofs_blksiz() - 1)))
				break;

		if (inputmargin >= rq->inputsize)
			return -EIO;
	}

	if (rq->decodedskip) {
		buff = malloc(rq->decodedlength);
		if (!buff)
			return -ENOMEM;
		dest = buff;
	}

	if (rq->partial_decoding || !support_0padding)
		ret = LZ4_decompress_safe_partial(src + inputmargin, dest,
						  rq->inputsize - inputmargin,
						  rq->decodedlength, rq->decodedlength);
	else
		ret = LZ4_decompress_safe(src + inputmargin, dest,
					  rq->inputsize - inputmargin,
					  rq->decodedlength);

	if (ret != (int)rq->decodedlength) {
		erofs_err("failed to %s decompress %d in[%u, %u] out[%u]",
			  rq->partial_decoding ? "partial" : "full",
			  ret, rq->inputsize, inputmargin, rq->decodedlength);
		ret = -EIO;
		goto out;
	}

	if (rq->decodedskip)
		memcpy(rq->out, dest + rq->decodedskip,
		       rq->decodedlength - rq->decodedskip);

out:
	if (buff)
		free(buff);

	return ret;
}
#endif

int z_erofs_decompress(struct z_erofs_decompress_req *rq)
{
	if (rq->alg == Z_EROFS_COMPRESSION_INTERLACED) {
		unsigned int count, rightpart, skip;

		/* XXX: should support inputsize >= erofs_blksiz() later */
		if (rq->inputsize > erofs_blksiz())
			return -EFSCORRUPTED;

		if (rq->decodedlength > erofs_blksiz())
			return -EFSCORRUPTED;

		if (rq->decodedlength < rq->decodedskip)
			return -EFSCORRUPTED;

		count = rq->decodedlength - rq->decodedskip;
		skip = erofs_blkoff(rq->interlaced_offset + rq->decodedskip);
		rightpart = min(erofs_blksiz() - skip, count);
		memcpy(rq->out, rq->in + skip, rightpart);
		memcpy(rq->out + rightpart, rq->in, count - rightpart);
		return 0;
	} else if (rq->alg == Z_EROFS_COMPRESSION_SHIFTED) {
		if (rq->decodedlength > rq->inputsize)
			return -EFSCORRUPTED;

		DBG_BUGON(rq->decodedlength < rq->decodedskip);
		memcpy(rq->out, rq->in + rq->decodedskip,
		       rq->decodedlength - rq->decodedskip);
		return 0;
	}

#if IS_ENABLED(CONFIG_LZ4)
	if (rq->alg == Z_EROFS_COMPRESSION_LZ4)
		return z_erofs_decompress_lz4(rq);
#endif
	return -EOPNOTSUPP;
}
