// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 ASPEED Technology Inc.
 * Author: ChiaWei Wang <chiawei_wang@aspeedtech.com>
 */
#include <config.h>
#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <watchdog.h>
#include <u-boot/hash.h>
#include <u-boot/crc.h>
#include <u-boot/md5.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>

/* CRC16-CCITT */
static void hash_init_crc16_ccitt(void *ctx)
{
	*((uint16_t *)ctx) = 0;
}

static void hash_update_crc16_ccitt(void *ctx, const void *ibuf, uint32_t ilen)
{
	*((uint16_t *)ctx) = crc16_ccitt(*((uint16_t *)ctx), ibuf, ilen);
}

static void hash_finish_crc16_ccitt(void *ctx, void *obuf)
{
	*((uint16_t *)obuf) = *((uint16_t *)ctx);
}

/* CRC32 */
static void hash_init_crc32(void *ctx)
{
	*((uint32_t *)ctx) = 0;
}

static void hash_update_crc32(void *ctx, const void *ibuf, uint32_t ilen)
{
	*((uint32_t *)ctx) = crc32(*((uint32_t *)ctx), ibuf, ilen);
}

static void hash_finish_crc32(void *ctx, void *obuf)
{
	*((uint32_t *)obuf) = *((uint32_t *)ctx);
}

/* MD5 */
static void hash_init_md5(void *ctx)
{
	MD5Init((struct MD5Context *)ctx);
}

static void hash_update_md5(void *ctx, const void *ibuf, uint32_t ilen)
{
	MD5Update((struct MD5Context *)ctx, ibuf, ilen);
}

static void hash_finish_md5(void *ctx, void *obuf)
{
	MD5Final(obuf, (struct MD5Context *)ctx);
}

/* SHA1 */
static void hash_init_sha1(void *ctx)
{
	sha1_starts((sha1_context *)ctx);
}

static void hash_update_sha1(void *ctx, const void *ibuf, uint32_t ilen)
{
	sha1_update((sha1_context *)ctx, ibuf, ilen);
}

static void hash_finish_sha1(void *ctx, void *obuf)
{
	sha1_finish((sha1_context *)ctx, obuf);
}

/* SHA256 */
static void hash_init_sha256(void *ctx)
{
	sha256_starts((sha256_context *)ctx);
}

static void hash_update_sha256(void *ctx, const void *ibuf, uint32_t ilen)
{
	sha256_update((sha256_context *)ctx, ibuf, ilen);
}

static void hash_finish_sha256(void *ctx, void *obuf)
{
	sha256_finish((sha256_context *)ctx, obuf);
}

/* SHA384 */
static void hash_init_sha384(void *ctx)
{
	sha384_starts((sha512_context *)ctx);
}

static void hash_update_sha384(void *ctx, const void *ibuf, uint32_t ilen)
{
	sha384_update((sha512_context *)ctx, ibuf, ilen);
}

static void hash_finish_sha384(void *ctx, void *obuf)
{
	sha384_finish((sha512_context *)ctx, obuf);
}

/* SHA512 */
static void hash_init_sha512(void *ctx)
{
	sha512_starts((sha512_context *)ctx);
}

static void hash_update_sha512(void *ctx, const void *ibuf, uint32_t ilen)
{
	sha512_update((sha512_context *)ctx, ibuf, ilen);
}

static void hash_finish_sha512(void *ctx, void *obuf)
{
	sha512_finish((sha512_context *)ctx, obuf);
}

struct sw_hash_ctx {
	enum HASH_ALGO algo;
	uint8_t algo_ctx[];
};

struct sw_hash_impl {
	void (*init)(void *ctx);
	void (*update)(void *ctx, const void *ibuf, uint32_t ilen);
	void (*finish)(void *ctx, void *obuf);
	uint32_t ctx_alloc_sz;
};

static struct sw_hash_impl sw_hash_impl[HASH_ALGO_NUM] = {
	[HASH_ALGO_CRC16_CCITT] = {
		.init = hash_init_crc16_ccitt,
		.update = hash_update_crc16_ccitt,
		.finish = hash_finish_crc16_ccitt,
		.ctx_alloc_sz = sizeof(uint16_t),
	},

	[HASH_ALGO_CRC32] = {
		.init = hash_init_crc32,
		.update = hash_update_crc32,
		.finish = hash_finish_crc32,
		.ctx_alloc_sz = sizeof(uint32_t),
	},

	[HASH_ALGO_MD5] = {
		.init = hash_init_md5,
		.update = hash_update_md5,
		.finish = hash_finish_md5,
		.ctx_alloc_sz = sizeof(struct MD5Context),
	},

	[HASH_ALGO_SHA1] = {
		.init = hash_init_sha1,
		.update = hash_update_sha1,
		.finish = hash_finish_sha1,
		.ctx_alloc_sz = sizeof(sha1_context),
	},

	[HASH_ALGO_SHA256] = {
		.init = hash_init_sha256,
		.update = hash_update_sha256,
		.finish = hash_finish_sha256,
		.ctx_alloc_sz = sizeof(sha256_context),
	},

	[HASH_ALGO_SHA384] = {
		.init = hash_init_sha384,
		.update = hash_update_sha384,
		.finish = hash_finish_sha384,
		.ctx_alloc_sz = sizeof(sha512_context),
	},

	[HASH_ALGO_SHA512] = {
		.init = hash_init_sha512,
		.update = hash_update_sha512,
		.finish = hash_finish_sha512,
		.ctx_alloc_sz = sizeof(sha512_context),
	},
};

static int sw_hash_init(struct udevice *dev, enum HASH_ALGO algo, void **ctxp)
{
	struct sw_hash_ctx *hash_ctx;
	struct sw_hash_impl *hash_impl = &sw_hash_impl[algo];

	hash_ctx = malloc(sizeof(hash_ctx->algo) + hash_impl->ctx_alloc_sz);
	if (!hash_ctx)
		return -ENOMEM;

	hash_ctx->algo = algo;

	hash_impl->init(hash_ctx->algo_ctx);

	*ctxp = hash_ctx;

	return 0;
}

static int sw_hash_update(struct udevice *dev, void *ctx, const void *ibuf, uint32_t ilen)
{
	struct sw_hash_ctx *hash_ctx = ctx;
	struct sw_hash_impl *hash_impl = &sw_hash_impl[hash_ctx->algo];

	hash_impl->update(hash_ctx->algo_ctx, ibuf, ilen);

	return 0;
}

static int sw_hash_finish(struct udevice *dev, void *ctx, void *obuf)
{
	struct sw_hash_ctx *hash_ctx = ctx;
	struct sw_hash_impl *hash_impl = &sw_hash_impl[hash_ctx->algo];

	hash_impl->finish(hash_ctx->algo_ctx, obuf);

	free(ctx);

	return 0;
}

static int sw_hash_digest_wd(struct udevice *dev, enum HASH_ALGO algo,
			     const void *ibuf, const uint32_t ilen,
			     void *obuf, uint32_t chunk_sz)
{
	int rc;
	void *ctx;
	const void *cur, *end;
	uint32_t chunk;

	rc = sw_hash_init(dev, algo, &ctx);
	if (rc)
		return rc;

	if (IS_ENABLED(CONFIG_HW_WATCHDOG) || CONFIG_IS_ENABLED(WATCHDOG)) {
		cur = ibuf;
		end = ibuf + ilen;

		while (cur < end) {
			chunk = end - cur;
			if (chunk > chunk_sz)
				chunk = chunk_sz;

			rc = sw_hash_update(dev, ctx, cur, chunk);
			if (rc)
				return rc;

			cur += chunk;
			schedule();
		}
	} else {
		rc = sw_hash_update(dev, ctx, ibuf, ilen);
		if (rc)
			return rc;
	}

	rc = sw_hash_finish(dev, ctx, obuf);
	if (rc)
		return rc;

	return 0;
}

static int sw_hash_digest(struct udevice *dev, enum HASH_ALGO algo,
			  const void *ibuf, const uint32_t ilen,
			  void *obuf)
{
	/* re-use the watchdog version with input length as the chunk_sz */
	return sw_hash_digest_wd(dev, algo, ibuf, ilen, obuf, ilen);
}

static const struct hash_ops hash_ops_sw = {
	.hash_init = sw_hash_init,
	.hash_update = sw_hash_update,
	.hash_finish = sw_hash_finish,
	.hash_digest_wd = sw_hash_digest_wd,
	.hash_digest = sw_hash_digest,
};

U_BOOT_DRIVER(hash_sw) = {
	.name = "hash_sw",
	.id = UCLASS_HASH,
	.ops = &hash_ops_sw,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(hash_sw) = {
	.name = "hash_sw",
};
