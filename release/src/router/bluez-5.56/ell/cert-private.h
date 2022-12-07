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

struct asn1_oid;

struct l_certchain *certchain_new_from_leaf(struct l_cert *leaf);
void certchain_link_issuer(struct l_certchain *chain, struct l_cert *ca);

const uint8_t *cert_get_extension(struct l_cert *cert,
					const struct asn1_oid *ext_id,
					bool *out_critical, size_t *out_len);

struct l_key *cert_key_from_pkcs8_private_key_info(const uint8_t *der,
							size_t der_len);
struct l_key *cert_key_from_pkcs8_encrypted_private_key_info(const uint8_t *der,
							size_t der_len,
							const char *passphrase);
struct l_key *cert_key_from_pkcs1_rsa_private_key(const uint8_t *der,
							size_t der_len);

struct cert_pkcs12_hash {
	enum l_checksum_type alg;
	unsigned int len;
	unsigned int u;
	unsigned int v;
	struct asn1_oid oid;
};

uint8_t *cert_pkcs12_pbkdf(const char *password,
				const struct cert_pkcs12_hash *hash,
				const uint8_t *salt, size_t salt_len,
				unsigned int iterations, uint8_t id,
				size_t key_len);

struct l_cipher *cert_cipher_from_pkcs_alg_id(const uint8_t *id_asn1,
						size_t id_asn1_len,
						const char *password,
						bool *out_is_block);
