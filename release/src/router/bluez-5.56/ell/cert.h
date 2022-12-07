/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
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

#ifndef __ELL_CERT_H
#define __ELL_CERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct l_queue;
struct l_cert;
struct l_certchain;

enum l_cert_key_type {
	L_CERT_KEY_RSA,
	L_CERT_KEY_UNKNOWN,
};

typedef bool (*l_cert_walk_cb_t)(struct l_cert *cert, void *user_data);

struct l_cert *l_cert_new_from_der(const uint8_t *buf, size_t buf_len);
void l_cert_free(struct l_cert *cert);

const uint8_t *l_cert_get_der_data(struct l_cert *cert, size_t *out_len);
const uint8_t *l_cert_get_dn(struct l_cert *cert, size_t *out_len);
enum l_cert_key_type l_cert_get_pubkey_type(struct l_cert *cert);
struct l_key *l_cert_get_pubkey(struct l_cert *cert);

void l_certchain_free(struct l_certchain *chain);

struct l_cert *l_certchain_get_leaf(struct l_certchain *chain);
void l_certchain_walk_from_leaf(struct l_certchain *chain,
				l_cert_walk_cb_t cb, void *user_data);
void l_certchain_walk_from_ca(struct l_certchain *chain,
				l_cert_walk_cb_t cb, void *user_data);

bool l_certchain_verify(struct l_certchain *chain, struct l_queue *ca_certs,
			const char **error);

bool l_cert_load_container_file(const char *filename, const char *password,
				struct l_certchain **out_certchain,
				struct l_key **out_privkey,
				bool *out_encrypted);

bool l_cert_pkcs5_pbkdf1(enum l_checksum_type type, const char *password,
				const uint8_t *salt, size_t salt_len,
				unsigned int iter_count,
				uint8_t *out_dk, size_t dk_len);
bool l_cert_pkcs5_pbkdf2(enum l_checksum_type type, const char *password,
				const uint8_t *salt, size_t salt_len,
				unsigned int iter_count,
				uint8_t *out_dk, size_t dk_len);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_CERT_H */
