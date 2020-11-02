/*
 * aesgcm.c
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
 * $Id: aesgcm.c 417337 2013-08-08 22:38:08Z $
 */

/* XXX: Define bcm_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <bcm_cfg.h>
#include <bcmcrypto/aesgcm.h>
#include <typedefs.h>

#ifdef BCMDRIVER
#include <osl.h>
#else
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define ASSERT assert
#endif  /* BCMDRIVER */

#ifdef FLOOR
#undef FLOOR
#endif // endif
#define FLOOR(x, y) ((x) / (y)) * (y)

static void
aes_gcm_encr_cb(void *ctx_in, gcm_block_t in, gcm_block_t out)
{
	aes_gcm_ctx_t *ctx = (aes_gcm_ctx_t*)ctx_in;
	rijndaelEncrypt(ctx->rkey, ctx->nrounds, in, out);
}

void
aes_gcm_init(aes_gcm_ctx_t *ctx, gcm_op_type_t op_type,
	const uint8 *key, size_t key_len,
	const uint8 *nonce, size_t nonce_len,
	const uint8 *aad, size_t aad_len)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->nrounds = (int)AES_ROUNDS(key_len);
	rijndaelKeySetupEnc(ctx->rkey, key, (int)AES_KEY_BITLEN(key_len));
	gcm_init(&ctx->gcm, op_type, aes_gcm_encr_cb, (void*)ctx,
		nonce, nonce_len, aad, aad_len);
}

void
aes_gcm_update(aes_gcm_ctx_t *ctx, uint8 *data, size_t data_len)
{
	gcm_update(&ctx->gcm, data, data_len);
}

void
aes_gcm_final(aes_gcm_ctx_t *ctx, uint8 *data, size_t data_len,
	uint8 *mac, size_t mac_len)
{
	gcm_final(&ctx->gcm, data, data_len, mac, mac_len);
}

static void
aes_gcm_op(gcm_op_type_t op_type, const uint8 *key, size_t key_len,
    const uint8 *nonce, size_t nonce_len, const uint8 *aad, size_t aad_len,
    uint8 *data, size_t data_len, uint8 *mac, size_t mac_len)
{
	aes_gcm_ctx_t ctx;
	size_t len;

	if (!data)
		data_len = 0;

	aes_gcm_init(&ctx, op_type, key, key_len, nonce, nonce_len,
		aad, aad_len);

	len = FLOOR(data_len, GCM_BLOCK_SZ);
	if (len)
		aes_gcm_update(&ctx, data, len);
	data += len;
	data_len -= len;

	aes_gcm_final(&ctx, data, data_len, mac, mac_len);
}

void
aes_gcm_encrypt(const uint8 *key, size_t key_len,
    const uint8 *nonce, size_t nonce_len, const uint8 *aad, size_t aad_len,
    uint8 *data, size_t data_len, uint8 *mac, size_t mac_len)
{
	aes_gcm_op(GCM_ENCRYPT, key, key_len, nonce, nonce_len, aad, aad_len,
		data, data_len, mac, mac_len);
}

void
aes_gcm_mac(const uint8 *key, size_t key_len,
    const uint8 *nonce, size_t nonce_len, const uint8 *aad, size_t aad_len,
    uint8 *data, size_t data_len, uint8 *mac, size_t mac_len)
{
	aes_gcm_op(GCM_MAC, key, key_len, nonce, nonce_len, aad, aad_len,
		data, data_len, mac, mac_len);
}

int
aes_gcm_decrypt(const uint8 *key, size_t key_len,
    const uint8 *nonce, size_t nonce_len, const uint8 *aad, size_t aad_len,
    uint8 *data, size_t data_len, const uint8 *expected_mac, size_t mac_len)
{
	gcm_block_t computed_mac;
	ASSERT(mac_len <= GCM_BLOCK_SZ);
	aes_gcm_op(GCM_DECRYPT, key, key_len, nonce, nonce_len, aad, aad_len,
		data, data_len, computed_mac, mac_len);
	return !memcmp(expected_mac, computed_mac, mac_len);
}

int
aes_gcm_verify(const uint8 *key, size_t key_len,
    const uint8 *nonce, size_t nonce_len, const uint8 *aad, size_t aad_len,
    /* const */ uint8 *data, size_t data_len,
	const uint8 *expected_mac, size_t mac_len)
{
	gcm_block_t computed_mac;
	ASSERT(mac_len <= GCM_BLOCK_SZ);
	aes_gcm_op(GCM_MAC, key, key_len, nonce, nonce_len, aad, aad_len,
		data, data_len, computed_mac, mac_len);
	return !memcmp(expected_mac, computed_mac, mac_len);
}
