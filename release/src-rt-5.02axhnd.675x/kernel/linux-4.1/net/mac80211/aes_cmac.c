/*
 * AES-128-CMAC with TLen 16 for IEEE 802.11w BIP
 * Copyright 2008, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/crypto.h>
#include <linux/export.h>
#include <linux/err.h>
#include <crypto/aes.h>

#include <net/mac80211.h>
#include "key.h"
#include "aes_cmac.h"

#define CMAC_TLEN 8 /* CMAC TLen = 64 bits (8 octets) */
#define CMAC_TLEN_256 16 /* CMAC TLen = 128 bits (16 octets) */
#define AAD_LEN 20


static void gf_mulx(u8 *pad)
{
	int i, carry;

	carry = pad[0] & 0x80;
	for (i = 0; i < AES_BLOCK_SIZE - 1; i++)
		pad[i] = (pad[i] << 1) | (pad[i + 1] >> 7);
	pad[AES_BLOCK_SIZE - 1] <<= 1;
	if (carry)
		pad[AES_BLOCK_SIZE - 1] ^= 0x87;
}

static void aes_cmac_vector(struct crypto_cipher *tfm, size_t num_elem,
			    const u8 *addr[], const size_t *len, u8 *mac,
			    size_t mac_len)
{
	u8 cbc[AES_BLOCK_SIZE], pad[AES_BLOCK_SIZE];
	const u8 *pos, *end;
	size_t i, e, left, total_len;

	memset(cbc, 0, AES_BLOCK_SIZE);

	total_len = 0;
	for (e = 0; e < num_elem; e++)
		total_len += len[e];
	left = total_len;

	e = 0;
	pos = addr[0];
	end = pos + len[0];

	while (left >= AES_BLOCK_SIZE) {
		for (i = 0; i < AES_BLOCK_SIZE; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		if (left > AES_BLOCK_SIZE)
			crypto_cipher_encrypt_one(tfm, cbc, cbc);
		left -= AES_BLOCK_SIZE;
	}

	memset(pad, 0, AES_BLOCK_SIZE);
	crypto_cipher_encrypt_one(tfm, pad, pad);
	gf_mulx(pad);

	if (left || total_len == 0) {
		for (i = 0; i < left; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		cbc[left] ^= 0x80;
		gf_mulx(pad);
	}

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		pad[i] ^= cbc[i];
	crypto_cipher_encrypt_one(tfm, pad, pad);
	memcpy(mac, pad, mac_len);
}


void ieee80211_aes_cmac(struct crypto_cipher *tfm, const u8 *aad,
			const u8 *data, size_t data_len, u8 *mic)
{
	const u8 *addr[3];
	size_t len[3];
	u8 zero[CMAC_TLEN];

	memset(zero, 0, CMAC_TLEN);
	addr[0] = aad;
	len[0] = AAD_LEN;
	addr[1] = data;
	len[1] = data_len - CMAC_TLEN;
	addr[2] = zero;
	len[2] = CMAC_TLEN;

	aes_cmac_vector(tfm, 3, addr, len, mic, CMAC_TLEN);
}

void ieee80211_aes_cmac_256(struct crypto_cipher *tfm, const u8 *aad,
			    const u8 *data, size_t data_len, u8 *mic)
{
	const u8 *addr[3];
	size_t len[3];
	u8 zero[CMAC_TLEN_256];

	memset(zero, 0, CMAC_TLEN_256);
	addr[0] = aad;
	len[0] = AAD_LEN;
	addr[1] = data;
	len[1] = data_len - CMAC_TLEN_256;
	addr[2] = zero;
	len[2] = CMAC_TLEN_256;

	aes_cmac_vector(tfm, 3, addr, len, mic, CMAC_TLEN_256);
}

struct crypto_cipher *ieee80211_aes_cmac_key_setup(const u8 key[],
						   size_t key_len)
{
	struct crypto_cipher *tfm;

	tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
	if (!IS_ERR(tfm))
		crypto_cipher_setkey(tfm, key, key_len);

	return tfm;
}


void ieee80211_aes_cmac_key_free(struct crypto_cipher *tfm)
{
	crypto_free_cipher(tfm);
}

void ieee80211_aes_cmac_calculate_k1_k2(struct ieee80211_key_conf *keyconf,
					u8 *k1, u8 *k2)
{
	u8 l[AES_BLOCK_SIZE] = {};
	struct ieee80211_key *key =
		container_of(keyconf, struct ieee80211_key, conf);

	crypto_cipher_encrypt_one(key->u.aes_cmac.tfm, l, l);

	memcpy(k1, l, AES_BLOCK_SIZE);
	gf_mulx(k1);

	memcpy(k2, k1, AES_BLOCK_SIZE);
	gf_mulx(k2);
}
EXPORT_SYMBOL(ieee80211_aes_cmac_calculate_k1_k2);
