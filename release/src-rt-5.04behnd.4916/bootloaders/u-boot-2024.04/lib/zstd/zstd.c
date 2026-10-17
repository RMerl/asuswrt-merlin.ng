// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <abuf.h>
#include <log.h>
#include <malloc.h>
#include <linux/errno.h>
#include <linux/zstd.h>

int zstd_decompress(struct abuf *in, struct abuf *out)
{
	zstd_dctx *ctx;
	size_t wsize, len;
	void *workspace;
	int ret;

	wsize = zstd_dctx_workspace_bound();
	workspace = malloc(wsize);
	if (!workspace) {
		debug("%s: cannot allocate workspace of size %zu\n", __func__,
			wsize);
		return -ENOMEM;
	}

	ctx = zstd_init_dctx(workspace, wsize);
	if (!ctx) {
		log_err("%s: zstd_init_dctx() failed\n", __func__);
		ret = -EPERM;
		goto do_free;
	}

	/*
	 * Find out how large the frame actually is, there may be junk at
	 * the end of the frame that zstd_decompress_dctx() can't handle.
	 */
	len = zstd_find_frame_compressed_size(abuf_data(in), abuf_size(in));
	if (zstd_is_error(len)) {
		log_err("%s: failed to detect compressed size: %d\n", __func__,
			zstd_get_error_code(len));
		ret = -EINVAL;
		goto do_free;
	}

	len = zstd_decompress_dctx(ctx, abuf_data(out), abuf_size(out),
				   abuf_data(in), len);
	if (zstd_is_error(len)) {
		log_err("%s: failed to decompress: %d\n", __func__,
			zstd_get_error_code(len));
		ret = -EINVAL;
		goto do_free;
	}

	ret = len;
do_free:
	free(workspace);
	return ret;
}
