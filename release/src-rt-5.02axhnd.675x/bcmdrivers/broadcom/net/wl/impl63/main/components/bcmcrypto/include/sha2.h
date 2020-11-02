/*
 * sha2.h
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *
 *  <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

/*
 * This header defines the API for SHA1/SHA2 FIPS 180-2/180-4
 */

#ifndef _SHA2_H_
#define _SHA2_H_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>
#endif // endif
#include <bcmtlv.h>

#define SHA2_MAX_BLOCK_SZ 128
#define SHA2_MAX_DIGEST_LEN 64		/* SHA384..SHA512 */
#define SHA2_MAX_CONTEXT_SZ 224		/* ROUNDUP(sizeof(sha2x_context_t), 8) */

/* digest length constants */
#define SHA2_SHA1_DIGEST_LEN 20
#define SHA2_SHA256_DIGEST_LEN 32
#define SHA2_SHA384_DIGEST_LEN 48

typedef enum {
	HASH_NONE		= 0,
	HASH_SHA1       = 1,
	HASH_SHA256     = 2,
	HASH_SHA384     = 3,
	HASH_SHA512     = 4,
	HASH_SHA512_224 = 5,
	HASH_SHA512_256 = 6,
	HASH_MD5        = 7,	/* not really based, for uniformity and hmac */
	HASH_SHA2_MAX           /* place holder, not used */
} sha2_hash_type_t;

struct sha2_ctx {
	sha2_hash_type_t hash_type;
	const struct sha2_hash_impl_entry *impl;
	void *impl_ctx;
	uint64 buf[SHA2_MAX_CONTEXT_SZ >> 3];
};
typedef struct sha2_ctx sha2_ctx_t;

/* initialize a context, return BCME_ status */
int  sha2_init(sha2_ctx_t *ctx, sha2_hash_type_t hash_type);

/* update the context w/ data */
void sha2_update(sha2_ctx_t *ctx, const uint8 *data, int data_len);

/* finalize; digest_len is in/out, if specified output is limited
 * to the specified value, and returns length written
 */
void sha2_final(sha2_ctx_t *ctx, uint8* digest, int digest_len);

/* one shot compute sha2 */
int sha2(sha2_hash_type_t hash_type,
	const bcm_const_xlvp_t *prefixes, int num_prefixes,
	const uint8 *data, int data_len, uint8 *digest, int digest_len);

/* return digest length of the hash function */
int sha2_digest_len(sha2_hash_type_t hash_type);

/* internal block size used by the hash function */
int sha2_block_size(sha2_hash_type_t hash_type);

/* hmac support */
int hmac_sha2(sha2_hash_type_t hash_type, const uint8 *key, int key_len,
	const bcm_const_xlvp_t *prefixes, int num_prefixes,
	const uint8 *data, int data_len, uint8 *digest, int digest_len);

struct hmac_sha2_ctx {
	uint8 opad[SHA2_MAX_BLOCK_SZ];
	sha2_ctx_t sha2_ctx;
};

typedef struct hmac_sha2_ctx hmac_sha2_ctx_t;

/* initialize hmac context */
int hmac_sha2_init(hmac_sha2_ctx_t *ctx, sha2_hash_type_t hash_type,
	const uint8 *key, int key_len);

/* update hmac context with data */
void hmac_sha2_update(hmac_sha2_ctx_t *ctx, const uint8 *data, int data_len);

/* finalize hmac context, digest_len is in/out to allow output trunication */
void hmac_sha2_final(hmac_sha2_ctx_t *ctx, uint8 *digest, int digest_len);

/* arbitrary length hash using prefixes */
int hmac_sha2_n(sha2_hash_type_t hash_type, const uint8 *key, int key_len,
	const bcm_const_xlvp_t *prefixes, int num_prefixes,
    const uint8 *data, int data_len, uint8 *out, int out_len);

/* rfc 5295 version of kdf, also supports HKDF-Expand from rfc 5869 */
int kdf5295(sha2_hash_type_t hash_type,
	const uint8 *key, int key_len, const char *label,
    const uint8 *info, int info_len, uint8 *out, int out_len);

#endif /* _SHA2_H_ */
