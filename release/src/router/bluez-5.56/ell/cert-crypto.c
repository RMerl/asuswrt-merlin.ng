/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2020  Intel Corporation. All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>

#include "checksum.h"
#include "cipher.h"
#include "util.h"
#include "utf8.h"
#include "asn1-private.h"
#include "private.h"
#include "missing.h"
#include "cert.h"
#include "cert-private.h"

/* RFC8018 section 5.1 */
LIB_EXPORT bool l_cert_pkcs5_pbkdf1(enum l_checksum_type type,
					const char *password,
					const uint8_t *salt, size_t salt_len,
					unsigned int iter_count,
					uint8_t *out_dk, size_t dk_len)
{
	size_t hash_len, t_len;
	uint8_t t[20 + salt_len + strlen(password)];
	struct l_checksum *checksum;

	switch (type) {
	case L_CHECKSUM_MD5:
		hash_len = 16;
		break;
	case L_CHECKSUM_SHA1:
		hash_len = 20;
		break;
	case L_CHECKSUM_NONE:
	case L_CHECKSUM_MD4:
	case L_CHECKSUM_SHA224:
	case L_CHECKSUM_SHA256:
	case L_CHECKSUM_SHA384:
	case L_CHECKSUM_SHA512:
		return false;
	default:
		return false;
	}

	if (dk_len > hash_len)
		return false;

	checksum = l_checksum_new(type);
	if (!checksum)
		return false;

	memcpy(t, password, strlen(password));
	memcpy(t + strlen(password), salt, salt_len);
	t_len = strlen(password) + salt_len;

	while (iter_count) {
		l_checksum_reset(checksum);

		if (!l_checksum_update(checksum, t, t_len))
			break;

		if (l_checksum_get_digest(checksum, t, hash_len) !=
				(ssize_t) hash_len)
			break;

		t_len = hash_len;
		iter_count--;
	}

	l_checksum_free(checksum);

	if (!iter_count)
		memcpy(out_dk, t, dk_len);

	explicit_bzero(t, sizeof(t));
	return !iter_count;
}

/* RFC8018 section 5.2 */
LIB_EXPORT bool l_cert_pkcs5_pbkdf2(enum l_checksum_type type,
					const char *password,
					const uint8_t *salt, size_t salt_len,
					unsigned int iter_count,
					uint8_t *out_dk, size_t dk_len)
{
	size_t h_len;
	struct l_checksum *checksum;
	unsigned int i;

	switch (type) {
	case L_CHECKSUM_SHA1:
		h_len = 20;
		break;
	case L_CHECKSUM_SHA224:
		h_len = 28;
		break;
	case L_CHECKSUM_SHA256:
		h_len = 32;
		break;
	case L_CHECKSUM_SHA384:
		h_len = 48;
		break;
	case L_CHECKSUM_SHA512:
		h_len = 64;
		break;
	case L_CHECKSUM_NONE:
	case L_CHECKSUM_MD4:
	case L_CHECKSUM_MD5:
		return false;
	default:
		return false;
	}

	checksum = l_checksum_new_hmac(type, password, strlen(password));
	if (!checksum)
		return false;

	for (i = 1; dk_len; i++) {
		unsigned int j, k;
		uint8_t u[salt_len + 64];
		size_t u_len;
		size_t block_len = h_len;

		if (block_len > dk_len)
			block_len = dk_len;

		memset(out_dk, 0, block_len);

		memcpy(u, salt, salt_len);
		l_put_be32(i, u + salt_len);
		u_len = salt_len + 4;

		for (j = 0; j < iter_count; j++) {
			l_checksum_reset(checksum);

			if (!l_checksum_update(checksum, u, u_len))
				break;

			if (l_checksum_get_digest(checksum, u, h_len) !=
					(ssize_t) h_len)
				break;

			u_len = h_len;

			for (k = 0; k < block_len; k++)
				out_dk[k] ^= u[k];
		}

		if (j < iter_count)
			break;

		out_dk += block_len;
		dk_len -= block_len;
	}

	l_checksum_free(checksum);

	return !dk_len;
}

/* RFC7292 Appendix B */
uint8_t *cert_pkcs12_pbkdf(const char *password,
				const struct cert_pkcs12_hash *hash,
				const uint8_t *salt, size_t salt_len,
				unsigned int iterations, uint8_t id,
				size_t key_len)
{
	/* All lengths in bytes instead of bits */
	size_t passwd_len = password ? 2 * strlen(password) + 2 : 0;
	uint8_t *bmpstring;
	/* Documented as v(ceiling(s/v)), usually will just equal v */
	unsigned int s_len = (salt_len + hash->v - 1) & ~(hash->v - 1);
	/* Documented as p(ceiling(s/p)), usually will just equal v */
	unsigned int p_len = password ?
			(passwd_len + hash->v - 1) & ~(hash->v - 1) : 0;
	uint8_t di[hash->v + s_len + p_len];
	uint8_t *ptr;
	unsigned int j;
	uint8_t *key;
	unsigned int bytes;
	struct l_checksum *h = l_checksum_new(hash->alg);

	if (!h)
		return NULL;

	/*
	 * The BMPString encoding, in practice same as UCS-2, can end up
	 * at 2 * strlen(password) + 2 bytes or shorter depending on the
	 * characters used.  Recalculate p_len after we know it.
	 * Important: The password must be valid UTF-8 here.
	 */
	if (password) {
		if (!(bmpstring = l_utf8_to_ucs2be(password, &passwd_len))) {
			l_checksum_free(h);
			return NULL;
		}

		p_len = (passwd_len + hash->v - 1) & ~(hash->v - 1);
	}

	memset(di, id, hash->v);
	ptr = di + hash->v;

	for (j = salt_len; j < s_len; j += salt_len, ptr += salt_len)
		memcpy(ptr, salt, salt_len);

	if (s_len) {
		memcpy(ptr, salt, s_len + salt_len - j);
		ptr += s_len + salt_len - j;
	}

	if (p_len) {
		for (j = passwd_len; j < p_len;
					j += passwd_len, ptr += passwd_len)
			memcpy(ptr, bmpstring, passwd_len);

		memcpy(ptr, bmpstring, p_len + passwd_len - j);

		explicit_bzero(bmpstring, passwd_len);
		l_free(bmpstring);
	}

	key = l_malloc(key_len + hash->len);

	for (bytes = 0; bytes < key_len; bytes += hash->u) {
		uint8_t b[hash->v];
		uint8_t *input = di;
		unsigned int input_len = hash->v + s_len + p_len;

		for (j = 0; j < iterations; j++) {
			if (!l_checksum_update(h, input, input_len) ||
					l_checksum_get_digest(h,
							key + bytes,
							hash->len) <= 0) {
				l_checksum_free(h);
				l_free(key);
				return NULL;
			}

			input = key + bytes;
			input_len = hash->u;
			l_checksum_reset(h);
		}

		if (bytes + hash->u >= key_len)
			break;

		for (j = 0; j < hash->v - hash->u; j += hash->u)
			memcpy(b + j, input, hash->u);

		memcpy(b + j, input, hash->v - j);

		ptr = di + hash->v;
		for (j = 0; j < s_len + p_len; j += hash->v, ptr += hash->v) {
			unsigned int k;
			uint16_t carry = 1;

			/*
			 * Not specified in the RFC7292 but implementations
			 * sum these octet strings as big-endian integers.
			 * We could use 64-bit additions here but the benefit
			 * may not compensate the cost of the byteswapping.
			 */
			for (k = hash->v - 1; k > 0; k--) {
				carry = ptr[k] + b[k] + carry;
				ptr[k] = carry;
				carry >>= 8;
			}

			ptr[k] += b[k] + carry;
			explicit_bzero(&carry, sizeof(carry));
		}

		explicit_bzero(b, sizeof(b));
	}

	explicit_bzero(di, sizeof(di));
	l_checksum_free(h);
	return key;
}

/* RFC7292 Appendix A */
static const struct cert_pkcs12_hash pkcs12_sha1_hash = {
	.alg = L_CHECKSUM_SHA1,
	.len = 20,
	.u   = 20,
	.v   = 64,
	.oid = { 5, { 0x2b, 0x0e, 0x03, 0x02, 0x1a } },
};

/* RFC8018 Section A.2 */
static struct asn1_oid pkcs5_pbkdf2_oid = {
	9, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05, 0x0c }
};

/* RFC8018 Section A.4 */
static struct asn1_oid pkcs5_pbes2_oid = {
	9, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05, 0x0d }
};

/* RFC8018 Section A.3 */
static const struct pkcs5_pbes1_encryption_oid {
	enum l_checksum_type checksum_type;
	enum l_cipher_type cipher_type;
	struct asn1_oid oid;
} pkcs5_pbes1_encryption_oids[] = {
	{ /* pbeWithMD5AndDES-CBC */
		L_CHECKSUM_MD5, L_CIPHER_DES_CBC,
		{ 9, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05, 0x03 } },
	},
	{ /* pbeWithSHA1AndDES-CBC */
		L_CHECKSUM_SHA1, L_CIPHER_DES_CBC,
		{ 9, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05, 0x0a } },
	},
	/* MD2- and RC2-based schemes 1, 4, 6 and 11 not supported */
};

/* RFC7292 Appendix C */
static const struct pkcs12_encryption_oid {
	enum l_cipher_type cipher_type;
	unsigned int key_length;
	unsigned int iv_length;
	bool copy_k1;	/* Expand the 2-Key 3DES key for 3-Key 3DES */
	bool is_block;
	struct asn1_oid oid;
} pkcs12_encryption_oids[] = {
	{ /* pbeWithSHAAnd128BitRC4 */
		.cipher_type = L_CIPHER_ARC4,
		.key_length = 16,
		.oid = { 10, {
			0x2a, 0x86, 0x48, 0x86, 0xf7,
			0x0d, 0x01, 0x0c, 0x01, 0x01,
		} }
	},
	{ /* pbeWithSHAAnd40BitRC4 */
		.cipher_type = L_CIPHER_ARC4,
		.key_length = 5,
		.oid = { 10, {
			0x2a, 0x86, 0x48, 0x86, 0xf7,
			0x0d, 0x01, 0x0c, 0x01, 0x02,
		} }
	},
	{ /* pbeWithSHAAnd3-KeyTripleDES-CBC */
		.cipher_type = L_CIPHER_DES3_EDE_CBC,
		.key_length = 24,
		.iv_length = 8,
		.is_block = true,
		.oid = { 10, {
			0x2a, 0x86, 0x48, 0x86, 0xf7,
			0x0d, 0x01, 0x0c, 0x01, 0x03,
		} }
	},
	{ /* pbeWithSHAAnd2-KeyTripleDES-CBC */
		.cipher_type = L_CIPHER_DES3_EDE_CBC,
		.key_length = 16,
		.iv_length = 8,
		.copy_k1 = true,
		.is_block = true,
		.oid = { 10, {
			0x2a, 0x86, 0x48, 0x86, 0xf7,
			0x0d, 0x01, 0x0c, 0x01, 0x04,
		} }
	},
	{ /* pbeWithSHAAnd128BitRC2-CBC */
		.cipher_type = L_CIPHER_RC2_CBC,
		.key_length = 16,
		.iv_length = 8,
		.is_block = true,
		.oid = { 10, {
			0x2a, 0x86, 0x48, 0x86, 0xf7,
			0x0d, 0x01, 0x0c, 0x01, 0x05,
		} }
	},
	{ /* pbeWithSHAAnd40BitRC2-CBC */
		.cipher_type = L_CIPHER_RC2_CBC,
		.key_length = 5,
		.iv_length = 8,
		.is_block = true,
		.oid = { 10, {
			0x2a, 0x86, 0x48, 0x86, 0xf7,
			0x0d, 0x01, 0x0c, 0x01, 0x06,
		} }
	},
};

static const struct pkcs5_digest_alg_oid {
	enum l_checksum_type type;
	struct asn1_oid oid;
} pkcs5_digest_alg_oids[] = {
	{ /* hmacWithSHA1 */
		L_CHECKSUM_SHA1,
		{ 8, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x07 } },
	},
	{ /* hmacWithSHA224 */
		L_CHECKSUM_SHA224,
		{ 8, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x08 } },
	},
	{ /* hmacWithSHA256 */
		L_CHECKSUM_SHA256,
		{ 8, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x09 } },
	},
	{ /* hmacWithSHA384 */
		L_CHECKSUM_SHA384,
		{ 8, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x0a } },
	},
	{ /* hmacWithSHA512 */
		L_CHECKSUM_SHA512,
		{ 8, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x0b } },
	},
	/* hmacWithSHA512-224 and hmacWithSHA512-256 not supported */
};

static const struct pkcs5_enc_alg_oid {
	enum l_cipher_type cipher_type;
	uint8_t key_size, iv_size;
	struct asn1_oid oid;
} pkcs5_enc_alg_oids[] = {
	{ /* desCBC */
		L_CIPHER_DES_CBC, 8, 8,
		{ 5, { 0x2b, 0x0e, 0x03, 0x02, 0x07 } },
	},
	{ /* des-EDE3-CBC */
		L_CIPHER_DES3_EDE_CBC, 24, 8,
		{ 8, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x03, 0x07 } },
	},
	/* RC2/RC5-based schemes 2 and 9 not supported */
	{ /* aes128-CBC-PAD */
		L_CIPHER_AES_CBC, 16, 16,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x02 } },
	},
	{ /* aes192-CBC-PAD */
		L_CIPHER_AES_CBC, 24, 16,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x16 } },
	},
	{ /* aes256-CBC-PAD */
		L_CIPHER_AES_CBC, 32, 16,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x2a } },
	},
};

static struct l_cipher *cipher_from_pkcs5_pbes2_params(
						const uint8_t *pbes2_params,
						size_t pbes2_params_len,
						const char *password)
{
	uint8_t tag;
	const uint8_t *kdf_sequence, *enc_sequence, *oid, *params,
		*salt, *iter_count_buf, *key_len_buf, *prf_sequence;
	size_t kdf_len, enc_len, params_len, salt_len, key_len, tmp_len;
	unsigned int i, iter_count, pos;
	enum l_checksum_type prf_alg = L_CHECKSUM_NONE;
	const struct pkcs5_enc_alg_oid *enc_scheme = NULL;
	uint8_t derived_key[64];
	struct l_cipher *cipher;

	/* RFC8018 section A.4 */

	kdf_sequence = asn1_der_find_elem(pbes2_params, pbes2_params_len, 0,
						&tag, &kdf_len);
	if (!kdf_sequence || tag != ASN1_ID_SEQUENCE)
		return NULL;

	enc_sequence = asn1_der_find_elem(pbes2_params, pbes2_params_len, 1,
						&tag, &enc_len);
	if (!enc_sequence || tag != ASN1_ID_SEQUENCE)
		return NULL;

	if (asn1_der_find_elem(pbes2_params, pbes2_params_len, 2,
						&tag, &tmp_len))
		return NULL;

	/* RFC8018 section A.2 */

	oid = asn1_der_find_elem(kdf_sequence, kdf_len, 0, &tag, &tmp_len);
	if (!oid || tag != ASN1_ID_OID)
		return NULL;

	if (!asn1_oid_eq(&pkcs5_pbkdf2_oid, tmp_len, oid))
		return NULL;

	params = asn1_der_find_elem(kdf_sequence, kdf_len, 1,
						&tag, &params_len);
	if (!params || tag != ASN1_ID_SEQUENCE)
		return NULL;

	if (asn1_der_find_elem(kdf_sequence, kdf_len, 2, &tag, &tmp_len))
		return NULL;

	salt = asn1_der_find_elem(params, params_len, 0, &tag, &salt_len);
	if (!salt || tag != ASN1_ID_OCTET_STRING ||
			salt_len < 1 || salt_len > 512)
		return NULL;

	iter_count_buf = asn1_der_find_elem(params, params_len, 1,
						&tag, &tmp_len);
	if (!iter_count_buf || tag != ASN1_ID_INTEGER ||
			tmp_len < 1 || tmp_len > 4)
		return NULL;

	iter_count = 0;

	while (tmp_len--)
		iter_count = (iter_count << 8) | *iter_count_buf++;

	pos = 2;
	key_len_buf = asn1_der_find_elem(params, params_len, pos,
						&tag, &tmp_len);
	if (key_len_buf && tag == ASN1_ID_INTEGER) {
		if (tmp_len != 1)
			return NULL;

		pos++;
		key_len = 0;

		while (tmp_len--)
			key_len = (key_len << 8) | *key_len_buf++;
	} else
		key_len = 0;

	prf_sequence = asn1_der_find_elem(params, params_len, pos,
						&tag, &tmp_len);
	if (prf_sequence && tag == ASN1_ID_SEQUENCE) {
		pos++;

		oid = asn1_der_find_elem(prf_sequence, tmp_len, 0,
						&tag, &tmp_len);
		if (!oid || tag != ASN1_ID_OID)
			return NULL;

		for (i = 0; i < L_ARRAY_SIZE(pkcs5_digest_alg_oids); i++)
			if (asn1_oid_eq(&pkcs5_digest_alg_oids[i].oid,
						tmp_len, oid))
				prf_alg = pkcs5_digest_alg_oids[i].type;

		if (prf_alg == L_CHECKSUM_NONE)
			return NULL;
	} else
		prf_alg = L_CHECKSUM_SHA1;

	oid = asn1_der_find_elem(enc_sequence, enc_len, 0, &tag, &tmp_len);
	if (!oid || tag != ASN1_ID_OID)
		return NULL;

	for (i = 0; i < L_ARRAY_SIZE(pkcs5_enc_alg_oids); i++) {
		if (asn1_oid_eq(&pkcs5_enc_alg_oids[i].oid, tmp_len, oid)) {
			enc_scheme = &pkcs5_enc_alg_oids[i];
			break;
		}
	}

	if (!enc_scheme)
		return NULL;

	params = asn1_der_find_elem(enc_sequence, enc_len, 1,
						&tag, &params_len);
	if (!params)
		return NULL;

	/* RFC8018 section B.2 */

	/*
	 * Since we don't support the RC2/RC5 PBES2 ciphers, our parameters
	 * only have an obligatory OCTET STRING IV parameter and a fixed key
	 * length.
	 */
	if (tag != ASN1_ID_OCTET_STRING || params_len != enc_scheme->iv_size)
		return NULL;

	if (key_len && enc_scheme->key_size != key_len)
		return NULL;

	key_len = enc_scheme->key_size;

	if (asn1_der_find_elem(enc_sequence, enc_len, 2, &tag, &tmp_len))
		return NULL;

	/* RFC8018 section 6.2 */

	if (!l_cert_pkcs5_pbkdf2(prf_alg, password, salt, salt_len, iter_count,
					derived_key, key_len))
		return NULL;

	cipher = l_cipher_new(enc_scheme->cipher_type, derived_key, key_len);
	if (cipher && !l_cipher_set_iv(cipher, params, enc_scheme->iv_size)) {
		l_cipher_free(cipher);
		cipher = NULL;
	}

	explicit_bzero(derived_key, 16);
	return cipher;
}

static struct l_cipher *cipher_from_pkcs12_alg_id(
				const struct pkcs12_encryption_oid *scheme,
				const uint8_t *params, size_t params_len,
				const char *password, bool *out_is_block)
{
	uint8_t tag;
	const uint8_t *salt;
	const uint8_t *iterations_data;
	size_t salt_len;
	size_t iterations_len;
	unsigned int iterations;
	uint8_t *key;
	size_t key_len;
	struct l_cipher *cipher;

	/* Same parameters as in PKCS#5 */
	salt = asn1_der_find_elem(params, params_len, 0, &tag, &salt_len);
	if (!salt || tag != ASN1_ID_OCTET_STRING)
		return NULL;

	iterations_data = asn1_der_find_elem(params, params_len, 1,
						&tag, &iterations_len);
	if (!iterations_data || tag != ASN1_ID_INTEGER ||
			iterations_len < 1 || iterations_len > 4)
		return NULL;

	for (iterations = 0; iterations_len; iterations_len--)
		iterations = (iterations << 8) | *iterations_data++;

	if (iterations < 1 || iterations > 8192)
		return NULL;

	if (iterations_data != params + params_len)
		return NULL;

	key_len = scheme->key_length;
	key = cert_pkcs12_pbkdf(password, &pkcs12_sha1_hash, salt, salt_len,
				iterations, 1, key_len);
	if (!key)
		return NULL;

	if (scheme->copy_k1) {
		/*
		 * 2-Key 3DES is like L_CIPHER_DES3_EDE_CBC except the last
		 * of the 3 8-byte keys is not generated using a KDF and
		 * instead is a copy of the first key.  In other words
		 * the first half of the 16-byte key material is appended
		 * at the end to produce the 24 bytes for DES3_EDE_CBC.
		 */
		uint8_t *key2 = l_malloc(24);

		memcpy(key2, key, 16);
		memcpy(key2 + 16, key, 8);
		explicit_bzero(key, key_len);
		l_free(key);
		key = key2;
		key_len = 24;
	}

	cipher = l_cipher_new(scheme->cipher_type, key, key_len);
	explicit_bzero(key, key_len);
	l_free(key);

	if (!cipher)
		return NULL;

	if (scheme->iv_length) {
		uint8_t *iv = cert_pkcs12_pbkdf(password, &pkcs12_sha1_hash,
						salt, salt_len, iterations, 2,
						scheme->iv_length);

		if (!iv || !l_cipher_set_iv(cipher, iv, scheme->iv_length)) {
			l_cipher_free(cipher);
			cipher = NULL;
		}

		if (iv)
			explicit_bzero(iv, scheme->iv_length);

		l_free(iv);
	}

	if (out_is_block)
		*out_is_block = scheme->is_block;

	return cipher;
}

struct l_cipher *cert_cipher_from_pkcs_alg_id(const uint8_t *id_asn1,
						size_t id_asn1_len,
						const char *password,
						bool *out_is_block)
{
	uint8_t tag;
	const uint8_t *oid, *params, *salt, *iter_count_buf;
	size_t oid_len, params_len, tmp_len;
	unsigned int i, iter_count;
	const struct pkcs5_pbes1_encryption_oid *pbes1_scheme = NULL;
	uint8_t derived_key[16];
	struct l_cipher *cipher;

	oid = asn1_der_find_elem(id_asn1, id_asn1_len, 0, &tag, &oid_len);
	if (!oid || tag != ASN1_ID_OID)
		return NULL;

	params = asn1_der_find_elem(id_asn1, id_asn1_len, 1, &tag, &params_len);
	if (!params || tag != ASN1_ID_SEQUENCE)
		return NULL;

	if (asn1_der_find_elem(id_asn1, id_asn1_len, 2, &tag, &tmp_len))
		return NULL;

	if (asn1_oid_eq(&pkcs5_pbes2_oid, oid_len, oid)) {
		if (out_is_block)
			*out_is_block = true;

		return cipher_from_pkcs5_pbes2_params(params, params_len,
							password);
	}

	/* RFC8018 section A.3 */

	for (i = 0; i < L_ARRAY_SIZE(pkcs5_pbes1_encryption_oids); i++) {
		if (asn1_oid_eq(&pkcs5_pbes1_encryption_oids[i].oid,
					oid_len, oid)) {
			pbes1_scheme = &pkcs5_pbes1_encryption_oids[i];
			break;
		}
	}

	/* Check if this is a PKCS#12 OID */
	if (!pbes1_scheme) {
		for (i = 0; i < L_ARRAY_SIZE(pkcs12_encryption_oids); i++)
			if (asn1_oid_eq(&pkcs12_encryption_oids[i].oid,
					oid_len, oid))
				return cipher_from_pkcs12_alg_id(
						&pkcs12_encryption_oids[i],
						params, params_len, password,
						out_is_block);

		return NULL;
	}

	salt = asn1_der_find_elem(params, params_len, 0, &tag, &tmp_len);
	if (!salt || tag != ASN1_ID_OCTET_STRING || tmp_len != 8)
		return NULL;

	iter_count_buf = asn1_der_find_elem(params, params_len, 1,
						&tag, &tmp_len);
	if (!iter_count_buf || tag != ASN1_ID_INTEGER ||
			tmp_len < 1 || tmp_len > 4)
		return NULL;

	iter_count = 0;

	while (tmp_len--)
		iter_count = (iter_count << 8) | *iter_count_buf++;

	if (asn1_der_find_elem(params, params_len, 2, &tag, &tmp_len))
		return NULL;

	/* RFC8018 section 6.1 */

	if (!l_cert_pkcs5_pbkdf1(pbes1_scheme->checksum_type, password,
					salt, 8, iter_count, derived_key, 16))
		return NULL;

	cipher = l_cipher_new(pbes1_scheme->cipher_type, derived_key + 0, 8);
	if (cipher && !l_cipher_set_iv(cipher, derived_key + 8, 8)) {
		l_cipher_free(cipher);
		cipher = NULL;
	}

	explicit_bzero(derived_key, 16);

	if (out_is_block)
		*out_is_block = true;

	return cipher;
}
