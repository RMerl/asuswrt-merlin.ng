/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 ASPEED Technology Inc.
 */
#ifndef _UBOOT_HASH_H
#define _UBOOT_HASH_H

enum HASH_ALGO {
	HASH_ALGO_CRC16_CCITT,
	HASH_ALGO_CRC32,
	HASH_ALGO_MD5,
	HASH_ALGO_SHA1,
	HASH_ALGO_SHA256,
	HASH_ALGO_SHA384,
	HASH_ALGO_SHA512,

	HASH_ALGO_NUM,

	HASH_ALGO_INVALID = 0xffffffff,
};

/* general APIs for hash algo information */
enum HASH_ALGO hash_algo_lookup_by_name(const char *name);
ssize_t hash_algo_digest_size(enum HASH_ALGO algo);
const char *hash_algo_name(enum HASH_ALGO algo);

/* device-dependent APIs */
int hash_digest(struct udevice *dev, enum HASH_ALGO algo,
		const void *ibuf, const uint32_t ilen,
		void *obuf);
int hash_digest_wd(struct udevice *dev, enum HASH_ALGO algo,
		   const void *ibuf, const uint32_t ilen,
		   void *obuf, uint32_t chunk_sz);
int hash_init(struct udevice *dev, enum HASH_ALGO algo, void **ctxp);
int hash_update(struct udevice *dev, void *ctx, const void *ibuf, const uint32_t ilen);
int hash_finish(struct udevice *dev, void *ctx, void *obuf);

/*
 * struct hash_ops - Driver model for Hash operations
 *
 * The uclass interface is implemented by all hash devices
 * which use driver model.
 */
struct hash_ops {
	/* progressive operations */
	int (*hash_init)(struct udevice *dev, enum HASH_ALGO algo, void **ctxp);
	int (*hash_update)(struct udevice *dev, void *ctx, const void *ibuf, const uint32_t ilen);
	int (*hash_finish)(struct udevice *dev, void *ctx, void *obuf);

	/* all-in-one operation */
	int (*hash_digest)(struct udevice *dev, enum HASH_ALGO algo,
			   const void *ibuf, const uint32_t ilen,
			   void *obuf);

	/* all-in-one operation with watchdog triggering every chunk_sz */
	int (*hash_digest_wd)(struct udevice *dev, enum HASH_ALGO algo,
			      const void *ibuf, const uint32_t ilen,
			      void *obuf, uint32_t chunk_sz);
};

#endif
