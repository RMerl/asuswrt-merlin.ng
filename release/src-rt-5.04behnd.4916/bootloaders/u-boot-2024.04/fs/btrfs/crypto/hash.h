#ifndef CRYPTO_HASH_H
#define CRYPTO_HASH_H

#include <linux/types.h>

#define CRYPTO_HASH_SIZE_MAX	32

void btrfs_hash_init(void);
int hash_crc32c(const u8 *buf, size_t length, u8 *out);
int hash_xxhash(const u8 *buf, size_t length, u8 *out);
int hash_sha256(const u8 *buf, size_t length, u8 *out);
int hash_blake2(const u8 *buf, size_t length, u8 *out);

u32 crc32c(u32 seed, const void * data, size_t len);

/* Blake2B is not yet supported due to lack of library */

#endif
