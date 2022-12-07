/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2016  Intel Corporation. All rights reserved.
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

#ifndef __ELL_KEY_H
#define __ELL_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include <ell/checksum.h>

struct l_key;
struct l_keyring;

enum l_key_feature {
	L_KEY_FEATURE_DH	= 1 << 0,
	L_KEY_FEATURE_RESTRICT	= 1 << 1,
	L_KEY_FEATURE_CRYPTO	= 1 << 2,
};

enum l_key_type {
	L_KEY_RAW = 0,
	L_KEY_RSA,
};

enum l_keyring_restriction {
	L_KEYRING_RESTRICT_ASYM = 0,
	L_KEYRING_RESTRICT_ASYM_CHAIN,
};

enum l_key_cipher_type {
	L_KEY_RSA_PKCS1_V1_5,
	L_KEY_RSA_RAW,
};

struct l_key *l_key_new(enum l_key_type type, const void *payload,
			size_t payload_length);

void l_key_free(struct l_key *key);
void l_key_free_norevoke(struct l_key *key);

bool l_key_update(struct l_key *key, const void *payload, size_t len);

bool l_key_extract(struct l_key *key, void *payload, size_t *len);

ssize_t l_key_get_payload_size(struct l_key *key);

bool l_key_get_info(struct l_key *key, enum l_key_cipher_type cipher,
			enum l_checksum_type checksum, size_t *bits,
			bool *out_public);

struct l_key *l_key_generate_dh_private(const void *prime_buf,
					size_t prime_len);

bool l_key_compute_dh_public(struct l_key *generator, struct l_key *private_key,
				struct l_key *prime,
				void *payload, size_t *len);

bool l_key_compute_dh_secret(struct l_key *other_public, struct l_key *private_key,
				struct l_key *prime,
				void *payload, size_t *len);

bool l_key_validate_dh_payload(const void *payload, size_t len,
				const void *prime_buf, size_t prime_len);

ssize_t l_key_encrypt(struct l_key *key, enum l_key_cipher_type cipher,
			enum l_checksum_type checksum, const void *in,
			void *out, size_t len_in, size_t len_out);

ssize_t l_key_decrypt(struct l_key *key, enum l_key_cipher_type cipher,
			enum l_checksum_type checksum, const void *in,
			void *out, size_t len_in, size_t len_out);

ssize_t l_key_sign(struct l_key *key, enum l_key_cipher_type cipher,
			enum l_checksum_type checksum, const void *in,
			void *out, size_t len_in, size_t len_out);

bool l_key_verify(struct l_key *key, enum l_key_cipher_type cipher,
			enum l_checksum_type checksum, const void *data,
			const void *sig, size_t len_data, size_t len_sig);

struct l_keyring *l_keyring_new(void);

bool l_keyring_restrict(struct l_keyring *keyring, enum l_keyring_restriction res,
			const struct l_keyring *trust);

void l_keyring_free(struct l_keyring *keyring);
void l_keyring_free_norevoke(struct l_keyring *keyring);

bool l_keyring_link(struct l_keyring *keyring, const struct l_key *key);

bool l_keyring_unlink(struct l_keyring *keyring, const struct l_key *key);

bool l_keyring_link_nested(struct l_keyring *keyring,
				const struct l_keyring *nested);
bool l_keyring_unlink_nested(struct l_keyring *keyring,
				const struct l_keyring *nested);

bool l_key_is_supported(uint32_t features);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_KEY_H */
