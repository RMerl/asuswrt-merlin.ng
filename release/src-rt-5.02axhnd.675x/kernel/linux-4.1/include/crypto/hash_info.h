/*
 * Hash Info: Hash algorithms information
 *
 * Copyright (c) 2013 Dmitry Kasatkin <d.kasatkin@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#ifndef _CRYPTO_HASH_INFO_H
#define _CRYPTO_HASH_INFO_H

#include <crypto/sha.h>
#include <crypto/md5.h>

#include <uapi/linux/hash_info.h>

/* not defined in include/crypto/ */
#define RMD128_DIGEST_SIZE      16
#define RMD160_DIGEST_SIZE	20
#define RMD256_DIGEST_SIZE      32
#define RMD320_DIGEST_SIZE      40

/* not defined in include/crypto/ */
#define WP512_DIGEST_SIZE	64
#define WP384_DIGEST_SIZE	48
#define WP256_DIGEST_SIZE	32

/* not defined in include/crypto/ */
#define TGR128_DIGEST_SIZE 16
#define TGR160_DIGEST_SIZE 20
#define TGR192_DIGEST_SIZE 24

extern const char *const hash_algo_name[HASH_ALGO__LAST];
extern const int hash_digest_size[HASH_ALGO__LAST];

#endif /* _CRYPTO_HASH_INFO_H */
