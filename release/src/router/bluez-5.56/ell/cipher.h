/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __ELL_CIPHER_H
#define __ELL_CIPHER_H

#ifdef __cplusplus
extern "C" {
#endif

struct l_cipher;

enum l_cipher_type {
	L_CIPHER_AES = 0,
	L_CIPHER_AES_CBC,
	L_CIPHER_AES_CTR,
	L_CIPHER_ARC4,
	L_CIPHER_DES,
	L_CIPHER_DES_CBC,
	L_CIPHER_DES3_EDE_CBC,
	L_CIPHER_RC2_CBC,
};

struct l_cipher *l_cipher_new(enum l_cipher_type type,
				const void *key, size_t key_length);

void l_cipher_free(struct l_cipher *cipher);

bool l_cipher_encrypt(struct l_cipher *cipher,
			const void *in, void *out, size_t len);
bool l_cipher_encryptv(struct l_cipher *cipher,
				const struct iovec *in, size_t in_cnt,
				const struct iovec *out, size_t out_cnt);

bool l_cipher_decrypt(struct l_cipher *cipher,
			const void *in, void *out, size_t len);
bool l_cipher_decryptv(struct l_cipher *cipher,
				const struct iovec *in, size_t in_cnt,
				const struct iovec *out, size_t out_cnt);

bool l_cipher_set_iv(struct l_cipher *cipher, const uint8_t *iv,
			size_t iv_length);

struct l_aead_cipher;

enum l_aead_cipher_type {
	L_AEAD_CIPHER_AES_CCM = 0,
	L_AEAD_CIPHER_AES_GCM,
};

struct l_aead_cipher *l_aead_cipher_new(enum l_aead_cipher_type type,
					const void *key, size_t key_length,
					size_t tag_length);

void l_aead_cipher_free(struct l_aead_cipher *cipher);

bool l_aead_cipher_encrypt(struct l_aead_cipher *cipher,
				const void *in, size_t in_len,
				const void *ad, size_t ad_len,
				const void *nonce, size_t nonce_len,
				void *out, size_t out_len);

bool l_aead_cipher_decrypt(struct l_aead_cipher *cipher,
				const void *in, size_t in_len,
				const void *ad, size_t ad_len,
				const void *nonce, size_t nonce_len,
				void *out, size_t out_len);

bool l_cipher_is_supported(enum l_cipher_type type);
bool l_aead_cipher_is_supported(enum l_aead_cipher_type type);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_CIPHER_H */
