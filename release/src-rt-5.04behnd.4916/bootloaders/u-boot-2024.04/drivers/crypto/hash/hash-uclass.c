// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 ASPEED Technology Inc.
 * Author: ChiaWei Wang <chiawei_wang@aspeedtech.com>
 */

#define LOG_CATEGORY UCLASS_HASH

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <u-boot/hash.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/list.h>

struct hash_info {
	char *name;
	uint32_t digest_size;
};

static const struct hash_info hash_info[HASH_ALGO_NUM] = {
	[HASH_ALGO_CRC16_CCITT] = { "crc16-ccitt", 2 },
	[HASH_ALGO_CRC32] = { "crc32", 4 },
	[HASH_ALGO_MD5] = { "md5", 16 },
	[HASH_ALGO_SHA1] = { "sha1", 20 },
	[HASH_ALGO_SHA256] = { "sha256", 32 },
	[HASH_ALGO_SHA384] = { "sha384", 48 },
	[HASH_ALGO_SHA512] = { "sha512", 64},
};

enum HASH_ALGO hash_algo_lookup_by_name(const char *name)
{
	int i;

	if (!name)
		return HASH_ALGO_INVALID;

	for (i = 0; i < HASH_ALGO_NUM; ++i)
		if (!strcmp(name, hash_info[i].name))
			return i;

	return HASH_ALGO_INVALID;
}

ssize_t hash_algo_digest_size(enum HASH_ALGO algo)
{
	if (algo >= HASH_ALGO_NUM)
		return -EINVAL;

	return hash_info[algo].digest_size;
}

const char *hash_algo_name(enum HASH_ALGO algo)
{
	if (algo >= HASH_ALGO_NUM)
		return NULL;

	return hash_info[algo].name;
}

int hash_digest(struct udevice *dev, enum HASH_ALGO algo,
		const void *ibuf, const uint32_t ilen,
		void *obuf)
{
	struct hash_ops *ops = (struct hash_ops *)device_get_ops(dev);

	if (!ops->hash_digest)
		return -ENOSYS;

	return ops->hash_digest(dev, algo, ibuf, ilen, obuf);
}

int hash_digest_wd(struct udevice *dev, enum HASH_ALGO algo,
		   const void *ibuf, const uint32_t ilen,
		   void *obuf, uint32_t chunk_sz)
{
	struct hash_ops *ops = (struct hash_ops *)device_get_ops(dev);

	if (!ops->hash_digest_wd)
		return -ENOSYS;

	return ops->hash_digest_wd(dev, algo, ibuf, ilen, obuf, chunk_sz);
}

int hash_init(struct udevice *dev, enum HASH_ALGO algo, void **ctxp)
{
	struct hash_ops *ops = (struct hash_ops *)device_get_ops(dev);

	if (!ops->hash_init)
		return -ENOSYS;

	return ops->hash_init(dev, algo, ctxp);
}

int hash_update(struct udevice *dev, void *ctx, const void *ibuf, const uint32_t ilen)
{
	struct hash_ops *ops = (struct hash_ops *)device_get_ops(dev);

	if (!ops->hash_update)
		return -ENOSYS;

	return ops->hash_update(dev, ctx, ibuf, ilen);
}

int hash_finish(struct udevice *dev, void *ctx, void *obuf)
{
	struct hash_ops *ops = (struct hash_ops *)device_get_ops(dev);

	if (!ops->hash_finish)
		return -ENOSYS;

	return ops->hash_finish(dev, ctx, obuf);
}

UCLASS_DRIVER(hash) = {
	.id	= UCLASS_HASH,
	.name	= "hash",
};
