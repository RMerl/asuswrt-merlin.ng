/*
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>

#include "private.h"
#include "key.h"
#include "queue.h"
#include "asn1-private.h"
#include "cipher.h"
#include "pem-private.h"
#include "cert.h"
#include "cert-private.h"
#include "missing.h"

#define X509_CERTIFICATE_POS			0
#define   X509_TBSCERTIFICATE_POS		  0
#define     X509_TBSCERT_VERSION_POS		    ASN1_CONTEXT_EXPLICIT(0)
#define     X509_TBSCERT_SERIAL_POS		    0
#define     X509_TBSCERT_SIGNATURE_POS		    1
#define       X509_ALGORITHM_ID_ALGORITHM_POS	      0
#define       X509_ALGORITHM_ID_PARAMS_POS	      1
#define     X509_TBSCERT_ISSUER_DN_POS		    2
#define     X509_TBSCERT_VALIDITY_POS		    3
#define     X509_TBSCERT_SUBJECT_DN_POS		    4
#define     X509_TBSCERT_SUBJECT_KEY_POS	    5
#define       X509_SUBJECT_KEY_ALGORITHM_POS	      0
#define       X509_SUBJECT_KEY_VALUE_POS	      1
#define     X509_TBSCERT_ISSUER_UID_POS		    ASN1_CONTEXT_IMPLICIT(1)
#define     X509_TBSCERT_SUBJECT_UID_POS	    ASN1_CONTEXT_IMPLICIT(2)
#define     X509_TBSCERT_EXTENSIONS_POS		    ASN1_CONTEXT_EXPLICIT(3)
#define   X509_SIGNATURE_ALGORITHM_POS		  1
#define   X509_SIGNATURE_VALUE_POS		  2

struct l_cert {
	enum l_cert_key_type pubkey_type;
	struct l_cert *issuer;
	struct l_cert *issued;
	size_t asn1_len;
	uint8_t asn1[0];
};

struct l_certchain {
	struct l_cert *leaf;	/* Bottom of the doubly-linked list */
	struct l_cert *ca;	/* Top of the doubly-linked list */
};

static const struct pkcs1_encryption_oid {
	enum l_cert_key_type key_type;
	struct asn1_oid oid;
} pkcs1_encryption_oids[] = {
	{ /* rsaEncryption */
		L_CERT_KEY_RSA,
		{ 9, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01 } },
	},
};

static bool cert_set_pubkey_type(struct l_cert *cert)
{
	const uint8_t *key_type;
	size_t key_type_len;
	int i;

	key_type = asn1_der_find_elem_by_path(cert->asn1, cert->asn1_len,
						ASN1_ID_OID, &key_type_len,
						X509_CERTIFICATE_POS,
						X509_TBSCERTIFICATE_POS,
						X509_TBSCERT_SUBJECT_KEY_POS,
						X509_SUBJECT_KEY_ALGORITHM_POS,
						X509_ALGORITHM_ID_ALGORITHM_POS,
						-1);
	if (!key_type)
		return false;

	for (i = 0; i < (int) L_ARRAY_SIZE(pkcs1_encryption_oids); i++)
		if (asn1_oid_eq(&pkcs1_encryption_oids[i].oid,
					key_type_len, key_type))
			break;

	if (i == L_ARRAY_SIZE(pkcs1_encryption_oids))
		cert->pubkey_type = L_CERT_KEY_UNKNOWN;
	else
		cert->pubkey_type = pkcs1_encryption_oids[i].key_type;

	return true;
}

LIB_EXPORT struct l_cert *l_cert_new_from_der(const uint8_t *buf,
						size_t buf_len)
{
	const uint8_t *seq = buf;
	size_t seq_len = buf_len;
	size_t content_len;
	struct l_cert *cert;

	/* Sanity check: outer element is a SEQUENCE */
	if (seq_len-- < 1 || *seq++ != ASN1_ID_SEQUENCE)
		return NULL;

	/* Sanity check: the SEQUENCE spans the whole buffer */
	content_len = asn1_parse_definite_length(&seq, &seq_len);
	if (content_len < 64 || content_len != seq_len)
		return NULL;

	/*
	 * We could require the signature algorithm and the key algorithm
	 * to be one of our supported types here but instead we only
	 * require that when the user wants to verify this certificate or
	 * get the public key respectively.
	 */

	cert = l_malloc(sizeof(struct l_cert) + buf_len);
	cert->issuer = NULL;
	cert->issued = NULL;
	cert->asn1_len = buf_len;
	memcpy(cert->asn1, buf, buf_len);

	/* Sanity check: structure is correct up to the Public Key Algorithm */
	if (!cert_set_pubkey_type(cert)) {
		l_free(cert);
		return NULL;
	}

	return cert;
}

LIB_EXPORT void l_cert_free(struct l_cert *cert)
{
	l_free(cert);
}

LIB_EXPORT const uint8_t *l_cert_get_der_data(struct l_cert *cert,
						size_t *out_len)
{
	if (unlikely(!cert))
		return NULL;

	*out_len = cert->asn1_len;
	return cert->asn1;
}

LIB_EXPORT const uint8_t *l_cert_get_dn(struct l_cert *cert, size_t *out_len)
{
	if (unlikely(!cert))
		return NULL;

	return asn1_der_find_elem_by_path(cert->asn1, cert->asn1_len,
						ASN1_ID_SEQUENCE, out_len,
						X509_CERTIFICATE_POS,
						X509_TBSCERTIFICATE_POS,
						X509_TBSCERT_SUBJECT_DN_POS,
						-1);
}

const uint8_t *cert_get_extension(struct l_cert *cert,
					const struct asn1_oid *ext_id,
					bool *out_critical, size_t *out_len)
{
	const uint8_t *ext, *end;
	size_t ext_len;

	if (unlikely(!cert))
		return NULL;

	ext = asn1_der_find_elem_by_path(cert->asn1, cert->asn1_len,
						ASN1_ID_SEQUENCE, &ext_len,
						X509_CERTIFICATE_POS,
						X509_TBSCERTIFICATE_POS,
						X509_TBSCERT_EXTENSIONS_POS,
						-1);
	if (unlikely(!ext))
		return NULL;

	end = ext + ext_len;
	while (ext < end) {
		const uint8_t *seq, *oid, *data;
		uint8_t tag;
		size_t len, oid_len, data_len;
		bool critical;

		seq = asn1_der_find_elem(ext, end - ext, 0, &tag, &len);
		if (unlikely(!seq || tag != ASN1_ID_SEQUENCE))
			return false;

		ext = seq + len;

		oid = asn1_der_find_elem(seq, len, 0, &tag, &oid_len);
		if (unlikely(!oid || tag != ASN1_ID_OID))
			return false;

		if (!asn1_oid_eq(ext_id, oid_len, oid))
			continue;

		data = asn1_der_find_elem(seq, len, 1, &tag, &data_len);
		critical = false;

		if (data && tag == ASN1_ID_BOOLEAN) {
			if (data_len != 1)
				return false;

			critical = *data != 0;	/* Tolerate BER booleans */

			data = asn1_der_find_elem(seq, len, 2, &tag, &data_len);
		}

		if (unlikely(!data || tag != ASN1_ID_OCTET_STRING))
			return false;

		if (out_critical)
			*out_critical = critical;

		if (out_len)
			*out_len = data_len;

		return data;
	}

	return NULL;
}

LIB_EXPORT enum l_cert_key_type l_cert_get_pubkey_type(struct l_cert *cert)
{
	if (unlikely(!cert))
		return L_CERT_KEY_UNKNOWN;

	return cert->pubkey_type;
}

/*
 * Note: Returns a new l_key object to be freed by the caller.
 */
LIB_EXPORT struct l_key *l_cert_get_pubkey(struct l_cert *cert)
{
	if (unlikely(!cert))
		return NULL;

	/* Use kernel's ASN.1 certificate parser to find the key data for us */
	if (cert->pubkey_type == L_CERT_KEY_RSA)
		return l_key_new(L_KEY_RSA, cert->asn1, cert->asn1_len);

	return NULL;
}

/*
 * Note: takes ownership of the certificate.  The certificate is
 * assumed to be new and not linked into any certchain object.
 */
struct l_certchain *certchain_new_from_leaf(struct l_cert *leaf)
{
	struct l_certchain *chain;

	chain = l_new(struct l_certchain, 1);
	chain->leaf = leaf;
	chain->ca = leaf;
	return chain;
}

/*
 * Note: takes ownership of the certificate.  The certificate is
 * assumed to be new and not linked into any certchain object.
 */
void certchain_link_issuer(struct l_certchain *chain, struct l_cert *ca)
{
	ca->issued = chain->ca;
	chain->ca->issuer = ca;
	chain->ca = ca;
}

static struct l_cert *certchain_pop_ca(struct l_certchain *chain)
{
	struct l_cert *ca = chain->ca;

	if (!ca)
		return NULL;

	if (ca->issued) {
		chain->ca = ca->issued;
		ca->issued->issuer = NULL;
		ca->issued = NULL;
	} else {
		chain->ca = NULL;
		chain->leaf = NULL;
	}

	return ca;
}

LIB_EXPORT void l_certchain_free(struct l_certchain *chain)
{
	while (chain && chain->ca)
		l_cert_free(certchain_pop_ca(chain));

	l_free(chain);
}

LIB_EXPORT struct l_cert *l_certchain_get_leaf(struct l_certchain *chain)
{
	if (unlikely(!chain))
		return NULL;

	return chain->leaf;
}

/*
 * Call @cb for each certificate in the chain starting from the leaf
 * certificate.  Stop if a call returns @true.
 */
LIB_EXPORT void l_certchain_walk_from_leaf(struct l_certchain *chain,
						l_cert_walk_cb_t cb,
						void *user_data)
{
	struct l_cert *cert;

	if (unlikely(!chain))
		return;

	for (cert = chain->leaf; cert; cert = cert->issuer)
		if (cb(cert, user_data))
			break;
}

/*
 * Call @cb for each certificate in the chain starting from the root
 * certificate.  Stop if a call returns @true.
 */
LIB_EXPORT void l_certchain_walk_from_ca(struct l_certchain *chain,
						l_cert_walk_cb_t cb,
						void *user_data)
{
	struct l_cert *cert;

	if (unlikely(!chain))
		return;

	for (cert = chain->ca; cert; cert = cert->issued)
		if (cb(cert, user_data))
			break;
}

static struct l_keyring *cert_set_to_keyring(struct l_queue *certs, char *error)
{
	struct l_keyring *ring;
	const struct l_queue_entry *entry;
	int i = 1;

	ring = l_keyring_new();
	if (!ring)
		return NULL;

	for (entry = l_queue_get_entries(certs); entry; entry = entry->next) {
		struct l_cert *cert = entry->data;
		struct l_key *key = l_cert_get_pubkey(cert);

		if (!key) {
			sprintf(error, "Can't get public key from certificate "
				"%i / %i in certificate set", i,
				l_queue_length(certs));
			goto cleanup;
		}

		if (!l_keyring_link(ring, key)) {
			l_key_free(key);
			sprintf(error, "Can't link the public key from "
				"certificate %i / %i to target keyring",
				i, l_queue_length(certs));
			goto cleanup;
		}

		l_key_free_norevoke(key);
		i++;
	}

	return ring;

cleanup:
	l_keyring_free(ring);
	return NULL;
}

static bool cert_is_in_set(struct l_cert *cert, struct l_queue *set)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(set); entry; entry = entry->next) {
		struct l_cert *cert2 = entry->data;

		if (cert == cert2)
			return true;

		if (cert->asn1_len == cert2->asn1_len &&
				!memcmp(cert->asn1, cert2->asn1,
					cert->asn1_len))
			return true;
	}

	return false;
}

static struct l_key *cert_try_link(struct l_cert *cert, struct l_keyring *ring)
{
	struct l_key *key;

	key = l_key_new(L_KEY_RSA, cert->asn1, cert->asn1_len);
	if (!key)
		return NULL;

	if (l_keyring_link(ring, key))
		return key;

	l_key_free(key);
	return NULL;
}

static void cert_keyring_cleanup(struct l_keyring **p)
{
	l_keyring_free(*p);
}

#define RETURN_ERROR(msg, args...)	\
	do {	\
		if (error) {	\
			*error = error_buf;	\
			snprintf(error_buf, sizeof(error_buf), msg, ## args); \
		}	\
		return false;	\
	} while (0)

LIB_EXPORT bool l_certchain_verify(struct l_certchain *chain,
					struct l_queue *ca_certs,
					const char **error)
{
	struct l_keyring *ca_ring = NULL;
	L_AUTO_CLEANUP_VAR(struct l_keyring *, verify_ring,
				cert_keyring_cleanup) = NULL;
	struct l_cert *cert;
	struct l_key *prev_key = NULL;
	int verified = 0;
	int ca_match = 0;
	int i = 0;
	static char error_buf[200];

	if (unlikely(!chain || !chain->leaf))
		RETURN_ERROR("Chain empty");

	verify_ring = l_keyring_new();
	if (!verify_ring)
		RETURN_ERROR("Can't create verify keyring");

	for (cert = chain->ca; cert; cert = cert->issued, i++)
		if (cert_is_in_set(cert, ca_certs)) {
			ca_match = i + 1;
			break;
		}

	cert = chain->ca;

	/*
	 * For TLS compatibility the trusted root CA certificate is
	 * optionally present in the chain.
	 *
	 * RFC5246 7.4.2:
	 * "Because certificate validation requires that root keys be
	 * distributed independently, the self-signed certificate that
	 * specifies the root certificate authority MAY be omitted from
	 * the chain, under the assumption that the remote end must
	 * already possess it in order to validate it in any case."
	 *
	 * The following is an optimization to skip verifying the root
	 * cert in the chain if it is bitwise-identical to one of the
	 * trusted CA certificates.  In that case we don't have to load
	 * all of the trusted certificates into the kernel, link them
	 * to @ca_ring or link @ca_ring to @verify_ring, instead we
	 * load the first certificate into @verify_ring before we set
	 * the restric mode on it, same as when no trusted CAs are
	 * provided.
	 *
	 * Note this happens to work around a kernel issue preventing
	 * self-signed certificates missing the optional AKID extension
	 * from being linked to a restricted keyring.  That issue would
	 * have affected us if the trusted CA set included such
	 * certificate and the same certificate was at the root of
	 * the chain.
	 */
	if (ca_certs && !ca_match) {
		ca_ring = cert_set_to_keyring(ca_certs, error_buf);
		if (!ca_ring) {
			if (error)
				*error = error_buf;
			return false;
		}

		if (!l_keyring_link_nested(verify_ring, ca_ring)) {
			l_keyring_free(ca_ring);
			RETURN_ERROR("Can't link CA ring to verify ring");
		}
	} else
		prev_key = cert_try_link(cert, verify_ring);

	/*
	 * The top, unverified certificate(s) are linked to the keyring and
	 * we can now force verification of any new certificates linked.
	 */
	if (!l_keyring_restrict(verify_ring, L_KEYRING_RESTRICT_ASYM_CHAIN,
				NULL)) {
		l_key_free(prev_key);
		l_keyring_free(ca_ring);
		RETURN_ERROR("Can't restrict verify keyring");
	}

	if (ca_ring) {
		/*
		 * Verify the first certificate outside of the loop, then
		 * revoke the trusted CAs' keys so that only the newly
		 * verified cert's public key remains in the ring.
		 */
		prev_key = cert_try_link(cert, verify_ring);
		l_keyring_free(ca_ring);
	}

	cert = cert->issued;

	/* Verify the rest of the chain */
	while (prev_key && cert) {
		struct l_key *new_key = cert_try_link(cert, verify_ring);

		/*
		 * Free and revoke the issuer's public key again leaving only
		 * new_key in verify_ring to ensure the next certificate linked
		 * is signed by the owner of this key.
		 */
		l_key_free(prev_key);
		prev_key = new_key;
		cert = cert->issued;
		verified++;
	}

	if (!prev_key) {
		int total = 0;
		char str[100];

		for (cert = chain->ca; cert; cert = cert->issued, total++);

		if (ca_match)
			snprintf(str, sizeof(str), "%i / %i matched a trusted "
					"certificate, root not verified",
					ca_match, total);
		else
			snprintf(str, sizeof(str), "root %sverified against "
					"trusted CA(s)",
					ca_certs && !ca_match && verified ? "" :
					"not ");

		RETURN_ERROR("Linking certificate %i / %i failed, %s",
				verified + 1, total, str);
	}

	l_key_free(prev_key);
	return true;
}

struct l_key *cert_key_from_pkcs8_private_key_info(const uint8_t *der,
							size_t der_len)
{
	return l_key_new(L_KEY_RSA, der, der_len);
}

/*
 * The passphrase, if given, must have been validated as UTF-8 unless the
 * caller knows that PKCS#12 encryption algorithms are not used.
 * Use l_utf8_validate.
 */
struct l_key *cert_key_from_pkcs8_encrypted_private_key_info(const uint8_t *der,
							size_t der_len,
							const char *passphrase)
{
	const uint8_t *key_info, *alg_id, *data;
	uint8_t tag;
	size_t key_info_len, alg_id_len, data_len, tmp_len;
	struct l_cipher *alg;
	uint8_t *decrypted;
	struct l_key *pkey;
	bool r;
	bool is_block;
	size_t decrypted_len;

	/* Technically this is BER, not limited to DER */
	key_info = asn1_der_find_elem(der, der_len, 0, &tag, &key_info_len);
	if (!key_info || tag != ASN1_ID_SEQUENCE)
		return NULL;

	alg_id = asn1_der_find_elem(key_info, key_info_len, 0, &tag,
					&alg_id_len);
	if (!alg_id || tag != ASN1_ID_SEQUENCE)
		return NULL;

	data = asn1_der_find_elem(key_info, key_info_len, 1, &tag, &data_len);
	if (!data || tag != ASN1_ID_OCTET_STRING || data_len < 8 ||
			(data_len & 7) != 0)
		return NULL;

	if (asn1_der_find_elem(der, der_len, 2, &tag, &tmp_len))
		return NULL;

	alg = cert_cipher_from_pkcs_alg_id(alg_id, alg_id_len, passphrase,
						&is_block);
	if (!alg)
		return NULL;

	decrypted = l_malloc(data_len);

	r = l_cipher_decrypt(alg, data, decrypted, data_len);
	l_cipher_free(alg);

	if (!r) {
		l_free(decrypted);
		return NULL;
	}

	decrypted_len = data_len;

	/*
	 * For block ciphers strip padding as defined in RFC8018
	 * (for PKCS#5 v1) or RFC1423 / RFC5652 (for v2).
	 */
	if (is_block) {
		uint8_t pad = decrypted[data_len - 1];

		pkey = NULL;

		if (pad > data_len || pad > 16 || pad == 0)
			goto cleanup;

		if (!l_secure_memeq(decrypted + data_len - pad, pad - 1U, pad))
			goto cleanup;

		decrypted_len -= pad;
	}

	pkey = cert_key_from_pkcs8_private_key_info(decrypted, decrypted_len);

cleanup:
	explicit_bzero(decrypted, data_len);
	l_free(decrypted);
	return pkey;
}

struct l_key *cert_key_from_pkcs1_rsa_private_key(const uint8_t *der,
							size_t der_len)
{
	const uint8_t *data;
	uint8_t tag;
	size_t data_len;
	const uint8_t *key_data;
	size_t key_data_len;
	int i;
	uint8_t *private_key;
	size_t private_key_len;
	uint8_t *one_asymmetric_key;
	uint8_t *ptr;
	struct l_key *pkey;

	static const uint8_t version0[] = {
		ASN1_ID_INTEGER, 0x01, 0x00
	};
	static const uint8_t pkcs1_rsa_encryption[] = {
		ASN1_ID_SEQUENCE, 0x0d,
		ASN1_ID_OID, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
		0x01, 0x01, 0x01,
		ASN1_ID_NULL, 0x00,
	};

	/*
	 * Sanity check that it's a version 0 or 1 RSAPrivateKey structure
	 * with the 8 integers.
	 */
	key_data = asn1_der_find_elem(der, der_len, 0, &tag, &key_data_len);
	if (!key_data || tag != ASN1_ID_SEQUENCE)
		return NULL;

	data = asn1_der_find_elem(key_data, key_data_len, 0, &tag,
					&data_len);
	if (!data || tag != ASN1_ID_INTEGER || data_len != 1 ||
			(data[0] != 0x00 && data[0] != 0x01))
		return NULL;

	for (i = 1; i < 9; i++) {
		data = asn1_der_find_elem(key_data, key_data_len, i, &tag,
						&data_len);
		if (!data || tag != ASN1_ID_INTEGER || data_len < 1)
			return NULL;
	}

	private_key = l_malloc(10 + der_len);
	ptr = private_key;
	*ptr++ = ASN1_ID_OCTET_STRING;
	asn1_write_definite_length(&ptr, der_len);
	memcpy(ptr, der, der_len);
	ptr += der_len;
	private_key_len = ptr - private_key;

	one_asymmetric_key = l_malloc(32 + private_key_len);
	ptr = one_asymmetric_key;
	*ptr++ = ASN1_ID_SEQUENCE;
	asn1_write_definite_length(&ptr, sizeof(version0) +
					sizeof(pkcs1_rsa_encryption) +
					private_key_len);
	memcpy(ptr, version0, sizeof(version0));
	ptr += sizeof(version0);
	memcpy(ptr, pkcs1_rsa_encryption, sizeof(pkcs1_rsa_encryption));
	ptr += sizeof(pkcs1_rsa_encryption);
	memcpy(ptr, private_key, private_key_len);
	ptr += private_key_len;
	explicit_bzero(private_key, private_key_len);
	l_free(private_key);

	pkey = cert_key_from_pkcs8_private_key_info(one_asymmetric_key,
						ptr - one_asymmetric_key);
	explicit_bzero(one_asymmetric_key, ptr - one_asymmetric_key);
	l_free(one_asymmetric_key);

	return pkey;
}

static const uint8_t *cert_unpack_pkcs7_content_info(const uint8_t *container,
					size_t container_len, int pos,
					const struct asn1_oid *expected_oid,
					struct asn1_oid *out_oid,
					uint8_t *out_tag, size_t *out_len)
{
	const uint8_t *content_info;
	size_t content_info_len;
	const uint8_t *type;
	size_t type_len;
	const uint8_t *ret;
	uint8_t tag;

	if (!(content_info = asn1_der_find_elem(container, container_len, pos,
						&tag, &content_info_len)) ||
			tag != ASN1_ID_SEQUENCE)
		return NULL;

	if (!(type = asn1_der_find_elem(content_info, content_info_len, 0,
					&tag, &type_len)) ||
			tag != ASN1_ID_OID ||
			type_len > sizeof(out_oid->asn1))
		return NULL;

	if (expected_oid && !asn1_oid_eq(expected_oid, type_len, type))
		return NULL;

	if (!(ret = asn1_der_find_elem(content_info, content_info_len,
					ASN1_CONTEXT_EXPLICIT(0),
					out_tag, out_len)) ||
			ret + *out_len != content_info + content_info_len)
		return NULL;

	if (out_oid) {
		out_oid->asn1_len = type_len;
		memcpy(out_oid->asn1, type, type_len);
	}

	return ret;
}

/* RFC5652 Section 8 */
static uint8_t *cert_decrypt_pkcs7_encrypted_data(const uint8_t *data,
						size_t data_len,
						const char *password,
						struct asn1_oid *out_oid,
						size_t *out_len)
{
	const uint8_t *version;
	size_t version_len;
	const uint8_t *encrypted_info;
	size_t encrypted_info_len;
	const uint8_t *type;
	size_t type_len;
	const uint8_t *alg_id;
	size_t alg_id_len;
	const uint8_t *encrypted;
	size_t encrypted_len;
	uint8_t tag;
	struct l_cipher *alg;
	uint8_t *plaintext;
	int i;
	bool ok;
	bool is_block;

	if (!(version = asn1_der_find_elem(data, data_len, 0, &tag,
						&version_len)) ||
			tag != ASN1_ID_INTEGER || version_len != 1 ||
			!L_IN_SET(version[0], 0, 2))
		return NULL;

	if (!(encrypted_info = asn1_der_find_elem(data, data_len, 1, &tag,
							&encrypted_info_len)) ||
			tag != ASN1_ID_SEQUENCE)
		return NULL;

	if (!(type = asn1_der_find_elem(encrypted_info, encrypted_info_len, 0,
					&tag, &type_len)) ||
			tag != ASN1_ID_OID ||
			type_len > sizeof(out_oid->asn1))
		return NULL;

	if (!(alg_id = asn1_der_find_elem(encrypted_info, encrypted_info_len, 1,
					&tag, &alg_id_len)) ||
			tag != ASN1_ID_SEQUENCE)
		return NULL;

	/* Not optional in our case, defined [0] IMPLICIT OCTET STRING */
	if (!(encrypted = asn1_der_find_elem(encrypted_info, encrypted_info_len,
						ASN1_CONTEXT_IMPLICIT(0),
						&tag, &encrypted_len)) ||
			tag != ASN1_ID(ASN1_CLASS_CONTEXT, 0, 0) ||
			encrypted_len < 8)
		return NULL;

	if (!(alg = cert_cipher_from_pkcs_alg_id(alg_id, alg_id_len, password,
							&is_block)))
		return NULL;

	plaintext = l_malloc(encrypted_len);
	ok = l_cipher_decrypt(alg, encrypted, plaintext, encrypted_len);
	l_cipher_free(alg);

	if (!ok) {
		l_free(plaintext);
		return NULL;
	}

	if (is_block) {
		bool ok = true;

		/* Also validate the padding */
		if (encrypted_len < plaintext[encrypted_len - 1] ||
				plaintext[encrypted_len - 1] > 16) {
			plaintext[encrypted_len - 1] = 1;
			ok = false;
		}

		for (i = 1; i < plaintext[encrypted_len - 1]; i++)
			if (plaintext[encrypted_len - 1 - i] !=
					plaintext[encrypted_len - 1])
				ok = false;

		if (!ok) {
			explicit_bzero(plaintext, encrypted_len);
			l_free(plaintext);
			return NULL;
		}

		encrypted_len -= plaintext[encrypted_len - 1];
	}

	if (out_oid) {
		out_oid->asn1_len = type_len;
		memcpy(out_oid->asn1, type, type_len);
	}

	*out_len = encrypted_len;
	return plaintext;
}

/* RFC7292 Appendix A. */
static const struct cert_pkcs12_hash pkcs12_mac_algs[] = {
	{
		L_CHECKSUM_MD5,    16, 16, 64,
		{ 8, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0f, 0x02, 0x05 } }
	},
	{
		L_CHECKSUM_SHA1,   20, 20, 64,
		{ 5, { 0x2b, 0x0e, 0x03, 0x02, 0x1a } }
	},
	{
		L_CHECKSUM_SHA224, 28, 28, 64,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04 } }
	},
	{
		L_CHECKSUM_SHA256, 32, 32, 64,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01 } }
	},
	{
		L_CHECKSUM_SHA384, 48, 48, 128,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02 } }
	},
	{
		L_CHECKSUM_SHA512, 64, 64, 128,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03 } }
	},
	{
		L_CHECKSUM_SHA512, 64, 28, 128,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x05 } }
	},
	{
		L_CHECKSUM_SHA512, 64, 32, 128,
		{ 9, { 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x06 } }
	},
};

static const struct asn1_oid pkcs12_key_bag_oid = {
	11, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x0c, 0x0a, 0x01, 0x01 }
};

static const struct asn1_oid pkcs12_pkcs8_shrouded_key_bag_oid = {
	11, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x0c, 0x0a, 0x01, 0x02 }
};

static const struct asn1_oid pkcs12_cert_bag_oid = {
	11, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x0c, 0x0a, 0x01, 0x03 }
};

static const struct asn1_oid pkcs12_safe_contents_bag_oid = {
	11, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x0c, 0x0a, 0x01, 0x06 }
};

static const struct asn1_oid pkcs9_x509_certificate_oid = {
	10, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x16, 0x01 }
};

/* RFC7292 Section 4.2.3 */
static bool cert_parse_pkcs12_cert_bag(const uint8_t *data, size_t data_len,
					struct l_certchain **out_certchain)
{
	const uint8_t *cert_bag;
	size_t cert_bag_len;
	const uint8_t *cert_id;
	size_t cert_id_len;
	const uint8_t *cert_value;
	size_t cert_value_len;
	uint8_t tag;
	struct l_cert *cert;

	if (!(cert_bag = asn1_der_find_elem(data, data_len, 0,
						&tag, &cert_bag_len)) ||
			tag != ASN1_ID_SEQUENCE)
		return false;

	if (!(cert_id = asn1_der_find_elem(cert_bag, cert_bag_len, 0,
						&tag, &cert_id_len)) ||
			tag != ASN1_ID_OID)
		return false;

	if (!(cert_value = asn1_der_find_elem(cert_bag, cert_bag_len,
						ASN1_CONTEXT_EXPLICIT(0),
						&tag, &cert_value_len)) ||
			tag != ASN1_ID_OCTET_STRING ||
			cert_value + cert_value_len != data + data_len)
		return false;

	/* Skip unsupported certificate types */
	if (!asn1_oid_eq(&pkcs9_x509_certificate_oid, cert_id_len, cert_id))
		return true;

	if (!(cert = l_cert_new_from_der(cert_value, cert_value_len)))
		return false;

	if (!*out_certchain)
		*out_certchain = certchain_new_from_leaf(cert);
	else
		certchain_link_issuer(*out_certchain, cert);

	return true;
}

static bool cert_parse_pkcs12_safe_contents(const uint8_t *data,
					size_t data_len, const char *password,
					struct l_certchain **out_certchain,
					struct l_key **out_privkey)
{
	const uint8_t *safe_contents;
	size_t safe_contents_len;
	uint8_t tag;

	if (!(safe_contents = asn1_der_find_elem(data, data_len, 0, &tag,
							&safe_contents_len)) ||
			tag != ASN1_ID_SEQUENCE ||
			data + data_len != safe_contents + safe_contents_len)
		return false;

	/* RFC7292 Section 4.2 */
	while (safe_contents_len) {
		const uint8_t *safe_bag;
		size_t safe_bag_len;
		const uint8_t *bag_id;
		size_t bag_id_len;
		const uint8_t *bag_value;
		int bag_value_len;

		/* RFC7292 Section 4.2 */
		if (!(safe_bag = asn1_der_find_elem(safe_contents,
							safe_contents_len, 0,
							&tag, &safe_bag_len)) ||
				tag != ASN1_ID_SEQUENCE)
			return false;

		if (!(bag_id = asn1_der_find_elem(safe_bag, safe_bag_len, 0,
							&tag, &bag_id_len)) ||
				tag != ASN1_ID_OID)
			return false;

		/*
		 * The bagValue is EXPLICITly tagged but we don't want to
		 * unpack the inner TLV yet so don't use asn1_der_find_elem.
		 */
		safe_bag_len -= bag_id + bag_id_len - safe_bag;
		safe_bag = bag_id + bag_id_len;

		if (safe_bag_len < 4)
			return false;

		tag = *safe_bag++;
		safe_bag_len--;
		bag_value_len = asn1_parse_definite_length(&safe_bag,
								&safe_bag_len);
		bag_value = safe_bag;

		if (bag_value_len < 0 || bag_value_len > (int) safe_bag_len ||
				tag != ASN1_ID(ASN1_CLASS_CONTEXT, 1, 0))
			return false;

		/* PKCS#9 attributes ignored */

		safe_contents_len -= (safe_bag + safe_bag_len - safe_contents);
		safe_contents = safe_bag + safe_bag_len;

		if (asn1_oid_eq(&pkcs12_key_bag_oid, bag_id_len, bag_id)) {
			if (!out_privkey || *out_privkey)
				continue;

			*out_privkey =
				cert_key_from_pkcs8_private_key_info(bag_value,
								bag_value_len);
			if (!*out_privkey)
				return false;
		} else if (asn1_oid_eq(&pkcs12_pkcs8_shrouded_key_bag_oid,
					bag_id_len, bag_id)) {
			if (!out_privkey || *out_privkey)
				continue;

			*out_privkey =
				cert_key_from_pkcs8_encrypted_private_key_info(
								bag_value,
								bag_value_len,
								password);
			if (!*out_privkey)
				return false;
		} else if (asn1_oid_eq(&pkcs12_cert_bag_oid,
					bag_id_len, bag_id)) {
			if (!out_certchain)
				continue;

			if (!cert_parse_pkcs12_cert_bag(bag_value, bag_value_len,
							out_certchain))
				return false;
		} else if (asn1_oid_eq(&pkcs12_safe_contents_bag_oid,
					bag_id_len, bag_id)) {
			/* TODO: depth check */
			if (!(cert_parse_pkcs12_safe_contents(bag_value,
								bag_value_len,
								password,
								out_certchain,
								out_privkey)))
				return false;
		}
	}

	return true;
}

static bool cert_check_pkcs12_integrity(const uint8_t *mac_data,
					size_t mac_data_len,
					const uint8_t *auth_safe,
					size_t auth_safe_len,
					const char *password)
{
	const uint8_t *mac;
	size_t mac_len;
	const uint8_t *mac_salt;
	size_t mac_salt_len;
	const uint8_t *iterations_data;
	size_t iterations_len;
	unsigned int iterations;
	const uint8_t *digest_alg;
	size_t digest_alg_len;
	const uint8_t *digest;
	size_t digest_len;
	const uint8_t *alg_id;
	size_t alg_id_len;
	const struct cert_pkcs12_hash *mac_hash;
	L_AUTO_FREE_VAR(uint8_t *, key) = NULL;
	struct l_checksum *hmac;
	uint8_t hmac_val[64];
	uint8_t tag;
	bool ok;
	unsigned int i;

	if (!(mac = asn1_der_find_elem(mac_data, mac_data_len, 0, &tag,
					&mac_len)) ||
			tag != ASN1_ID_SEQUENCE)
		return false;

	if (!(mac_salt = asn1_der_find_elem(mac_data, mac_data_len, 1, &tag,
						&mac_salt_len)) ||
			tag != ASN1_ID_OCTET_STRING || mac_salt_len > 1024)
		return false;

	if (!(iterations_data = asn1_der_find_elem(mac_data, mac_data_len, 2,
							&tag,
							&iterations_len)) ||
			tag != ASN1_ID_INTEGER || iterations_len > 4)
		return false;

	for (iterations = 0; iterations_len; iterations_len--)
		iterations = (iterations << 8) | *iterations_data++;

	if (iterations < 1 || iterations > 8192)
		return false;

	/* RFC2315 Section 9.4 */
	if (!(digest_alg = asn1_der_find_elem(mac, mac_len, 0, &tag,
						&digest_alg_len)) ||
			tag != ASN1_ID_SEQUENCE)
		return false;

	if (!(digest = asn1_der_find_elem(mac, mac_len, 1, &tag,
						&digest_len)) ||
			tag != ASN1_ID_OCTET_STRING)
		return false;

	if (!(alg_id = asn1_der_find_elem(digest_alg, digest_alg_len,
						0, &tag, &alg_id_len)) ||
			tag != ASN1_ID_OID)
		return false;

	/* This is going to be used for both the MAC and its key derivation */
	for (i = 0; i < L_ARRAY_SIZE(pkcs12_mac_algs); i++)
		if (asn1_oid_eq(&pkcs12_mac_algs[i].oid, alg_id_len, alg_id)) {
			mac_hash = &pkcs12_mac_algs[i];
			break;
		}

	if (i == L_ARRAY_SIZE(pkcs12_mac_algs) || digest_len != mac_hash->u)
		return false;

	if (!(key = cert_pkcs12_pbkdf(password, mac_hash,
					mac_salt, mac_salt_len,
					iterations, 3, mac_hash->u)))
		return false;

	hmac = l_checksum_new_hmac(mac_hash->alg, key, mac_hash->u);
	explicit_bzero(key, mac_hash->u);

	if (!hmac)
		return false;

	ok = l_checksum_update(hmac, auth_safe, auth_safe_len) &&
		l_checksum_get_digest(hmac, hmac_val, mac_hash->len) > 0;
	l_checksum_free(hmac);

	if (!ok)
		return false;

	/*
	 * SHA-512/224 and SHA-512/256 are not supported.  We can truncate the
	 * output for key derivation but we can't do this inside the HMAC
	 * algorithms based on these hashes.  We skip the MAC verification
	 * if one of these hashes is used (identified by .u != .len)
	 */
	if (mac_hash->u != mac_hash->len)
		return true;

	return l_secure_memcmp(hmac_val, digest, digest_len) == 0;
}

/* RFC5652 Section 4 */
static const struct asn1_oid pkcs7_data_oid = {
	9, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01 }
};

/* RFC5652 Section 8 */
static const struct asn1_oid pkcs7_encrypted_data_oid = {
	9, { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x06 }
};

static bool cert_parse_auth_safe_content(const uint8_t *data, size_t data_len,
					uint8_t tag,
					const struct asn1_oid *data_oid,
					const char *password,
					struct l_certchain **out_certchain,
					struct l_key **out_privkey)
{
	if (asn1_oid_eq(&pkcs7_encrypted_data_oid,
			data_oid->asn1_len, data_oid->asn1)) {
		uint8_t *plaintext;
		size_t plaintext_len;
		struct asn1_oid oid;
		bool ok;

		if (tag != ASN1_ID_SEQUENCE)
			return false;

		/*
		 * This is same as PKCS#7 encryptedData but the ciphers
		 * used are from PKCS#12 (broken but still the default
		 * everywhere) and PKCS#5 (recommended).
		 */
		plaintext = cert_decrypt_pkcs7_encrypted_data(data,
								data_len,
								password, &oid,
								&plaintext_len);
		if (!plaintext)
			return false;

		/*
		 * Since we only support PKCS#7 data and encryptedData
		 * types, and there's no point re-encrypting
		 * encryptedData, the plaintext must be a PKCS#7
		 * "data".
		 */
		ok = asn1_oid_eq(&pkcs7_data_oid,
					oid.asn1_len, oid.asn1) &&
			cert_parse_pkcs12_safe_contents(plaintext,
							plaintext_len,
							password,
							out_certchain,
							out_privkey);
		explicit_bzero(plaintext, plaintext_len);
		l_free(plaintext);

		if (!ok)
			return false;
	} else if (asn1_oid_eq(&pkcs7_data_oid,
				data_oid->asn1_len, data_oid->asn1)) {
		if (tag != ASN1_ID_OCTET_STRING)
			return false;

		if (!cert_parse_pkcs12_safe_contents(data, data_len,
							password,
							out_certchain,
							out_privkey))
			return false;
	}
	/* envelopedData support not needed */

	return true;
}

static bool cert_parse_pkcs12_pfx(const uint8_t *ptr, size_t len,
					const char *password,
					struct l_certchain **out_certchain,
					struct l_key **out_privkey)
{
	const uint8_t *version;
	size_t version_len;
	const uint8_t *auth_safe;
	size_t auth_safe_len;
	const uint8_t *mac_data;
	size_t mac_data_len;
	const uint8_t *auth_safe_seq;
	size_t auth_safe_seq_len;
	uint8_t tag;
	unsigned int i;
	struct l_certchain *certchain = NULL;
	struct l_key *privkey = NULL;

	/* RFC7292 Section 4 */
	if (!(version = asn1_der_find_elem(ptr, len, 0, &tag, &version_len)) ||
			tag != ASN1_ID_INTEGER)
		return false;

	if (version_len != 1 || version[0] != 3)
		return false;

	/*
	 * Since we only support the password-based integrity mode,  the
	 * authSafe must be of PKCS#7 type "data" and not "signedData".
	 */
	if (!(auth_safe = cert_unpack_pkcs7_content_info(ptr, len, 1,
							&pkcs7_data_oid, NULL,
							&tag,
							&auth_safe_len)) ||
			tag != ASN1_ID_OCTET_STRING)
		return false;

	/*
	 * openssl can generate PFX structures without macData not signed
	 * with a public key so handle this case, otherwise the macData
	 * would not be optional.
	 */
	if (auth_safe + auth_safe_len == ptr + len)
		goto integrity_check_done;

	if (!(mac_data = asn1_der_find_elem(ptr, len, 2, &tag,
						&mac_data_len)) ||
			tag != ASN1_ID_SEQUENCE)
		return false;

	if (!cert_check_pkcs12_integrity(mac_data, mac_data_len,
						auth_safe, auth_safe_len,
						password))
		return false;

integrity_check_done:
	if (!(auth_safe_seq = asn1_der_find_elem(auth_safe, auth_safe_len, 0,
						&tag, &auth_safe_seq_len)) ||
			tag != ASN1_ID_SEQUENCE ||
			auth_safe + auth_safe_len !=
			auth_safe_seq + auth_safe_seq_len)
		return false;

	i = 0;
	while (1) {
		struct asn1_oid data_oid;
		const uint8_t *data;
		size_t data_len;

		if (!(data = cert_unpack_pkcs7_content_info(auth_safe_seq,
							auth_safe_seq_len, i++,
							NULL, &data_oid, &tag,
							&data_len)))
			goto error;

		if (!cert_parse_auth_safe_content(data, data_len, tag,
							&data_oid, password,
							out_certchain ?
							&certchain : NULL,
							out_privkey ?
							&privkey : NULL))
			goto error;

		if (data + data_len == auth_safe_seq + auth_safe_seq_len)
			break;
	}

	if (out_certchain)
		*out_certchain = certchain;

	if (out_privkey)
		*out_privkey = privkey;

	return true;

error:
	if (certchain)
		l_certchain_free(certchain);

	if (privkey)
		l_key_free(privkey);

	return false;
}

static int cert_try_load_der_format(const uint8_t *content, size_t content_len,
					const char *password,
					struct l_certchain **out_certchain,
					struct l_key **out_privkey,
					bool *out_encrypted)
{
	const uint8_t *seq;
	size_t seq_len;
	const uint8_t *elem_data;
	size_t elem_len;
	uint8_t tag;

	if (!(seq = asn1_der_find_elem(content, content_len,
					0, &tag, &seq_len)))
		/* May not have been a DER file after all */
		return -ENOMSG;

	/*
	 * See if the first sub-element is another sequence, then, out of
	 * the formats that we currently support this can only be a raw
	 * certificate.  If integer, it's going to be PKCS#12.  If we wish
	 * to add any more formats we'll probably need to start guessing
	 * from the filename suffix.
	 */
	if (!(elem_data = asn1_der_find_elem(seq, seq_len,
						0, &tag, &elem_len)))
		return -ENOMSG;

	if (tag == ASN1_ID_SEQUENCE) {
		if (out_certchain) {
			struct l_cert *cert;

			if (!(cert = l_cert_new_from_der(content, content_len)))
				return -EINVAL;

			*out_certchain = certchain_new_from_leaf(cert);

			if (out_privkey)
				*out_privkey = NULL;

			return 0;
		}

		return -EINVAL;
	}

	if (tag == ASN1_ID_INTEGER) {
		/*
		 * Since we don't support public key-protected PKCS#12
		 * modes, we always require the password at least for the
		 * integrity check.  Strictly speaking encryption may not
		 * actually be in use.  We also don't support files with
		 * different integrity and privacy passwords, they must
		 * be identical if privacy is enabled.
		 */
		if (out_encrypted)
			*out_encrypted = true;

		if (!password) {
			if (!out_encrypted)
				return -EINVAL;

			if (out_certchain)
				*out_certchain = NULL;

			if (out_privkey)
				*out_privkey = NULL;

			return 0;
		}

		if (cert_parse_pkcs12_pfx(seq, seq_len, password,
						out_certchain, out_privkey))
			return 0;
		else
			return -EINVAL;
	}

	return -ENOMSG;
}

static bool cert_try_load_pem_format(const char *content, size_t content_len,
					const char *password,
					struct l_certchain **out_certchain,
					struct l_key **out_privkey,
					bool *out_encrypted)
{
	bool error = false;
	bool done = false;
	struct l_certchain *certchain = NULL;
	struct l_key *privkey = NULL;
	bool encrypted = false;

	while (!done && !error && content_len) {
		uint8_t *der;
		size_t der_len;
		char *type_label;
		char *headers;
		const char *endp;

		if (!(der = pem_load_buffer(content, content_len, &type_label,
						&der_len, &headers, &endp)))
			break;

		content_len -= endp - content;
		content = endp;

		if (out_certchain && L_IN_STRSET(type_label, "CERTIFICATE")) {
			struct l_cert *cert;

			if (!(cert = l_cert_new_from_der(der, der_len))) {
				error = true;
				goto next;
			}

			if (!certchain)
				certchain = certchain_new_from_leaf(cert);
			else
				certchain_link_issuer(certchain, cert);

			goto next;
		}

		/* Only use the first private key found */
		if (out_privkey && !privkey && L_IN_STRSET(type_label,
							"PRIVATE KEY",
							"ENCRYPTED PRIVATE KEY",
							"RSA PRIVATE KEY")) {
			privkey = pem_load_private_key(der, der_len, type_label,
							password, headers,
							&encrypted);
			if (!privkey) {
				if (certchain) {
					l_certchain_free(certchain);
					certchain = NULL;
				}

				if (password)
					error = true;
				else
					error = !encrypted || !out_encrypted;

				done = true;
			}

			continue;
		}

		/* Cisco/gnutls-type PEM-encoded PKCS#12, probably rare */
		if (L_IN_STRSET(type_label, "PKCS12")) {
			encrypted = true;

			if (!password) {
				if (certchain && out_privkey) {
					l_certchain_free(certchain);
					certchain = NULL;
				}

				error = !out_encrypted;
				done = true;
				goto next;
			}

			error = !cert_parse_pkcs12_pfx(der, der_len, password,
							out_certchain ?
							&certchain : NULL,
							out_privkey ?
							&privkey : NULL);
			goto next;
		}

next:
		explicit_bzero(der, der_len);
		l_free(der);
		l_free(type_label);
		l_free(headers);
	}

	if (error) {
		if (certchain)
			l_certchain_free(certchain);

		if (privkey)
			l_key_free(privkey);

		return false;
	}

	if (out_certchain)
		*out_certchain = certchain;

	if (out_privkey)
		*out_privkey = privkey;

	if (out_encrypted)
		*out_encrypted = encrypted;

	return true;
}

/*
 * Look at a file, try to detect which of the few X.509 certificate and/or
 * private key container formats it uses and load any certificates in it as
 * a certificate chain object, and load the first private key as an l_key
 * object.
 *
 * Currently supported are:
 *  PEM X.509 certificates
 *  PEM PKCS#8 encrypted and unencrypted private keys
 *  PEM legacy PKCS#1 encrypted and unencrypted private keys
 *  Raw X.509 certificates (.cer, .der, .crt)
 *  PKCS#12 certificates
 *  PKCS#12 encrypted private keys
 *
 * The raw format contains exactly one certificate, PEM and PKCS#12 files
 * can contain any combination of certificates and private keys.
 *
 * The password must have been validated as UTF-8 (use l_utf8_validate)
 * unless the caller knows that no PKCS#12-defined encryption algorithm
 * or MAC is used.
 *
 * Returns false on "unrecoverable" errors, and *out_certchain,
 * *out_privkey and *out_encrypted (if provided) are not modified.  However
 * when true is returned, *out_certchain and *out_privkey (if provided) may
 * be set to NULL when nothing could be loaded only due to missing password,
 * and *out_encrypted (if provided) will be set accordingly.  It will also
 * be set on success to indicate whether the password was used.
 * *out_certchain and/or *out_privkey will also be NULL if the container
 * was loaded but there were no certificates or private keys in it.
 */
LIB_EXPORT bool l_cert_load_container_file(const char *filename,
					const char *password,
					struct l_certchain **out_certchain,
					struct l_key **out_privkey,
					bool *out_encrypted)
{
	struct pem_file_info file;
	bool error = true;

	if (unlikely(!filename))
		return false;

	if (pem_file_open(&file, filename) < 0)
		return false;

	if (file.st.st_size < 1)
		goto close;

	/* See if we have a DER sequence tag at the start */
	if (file.data[0] == ASN1_ID_SEQUENCE) {
		int err;

		err = cert_try_load_der_format(file.data, file.st.st_size,
						password, out_certchain,
						out_privkey, out_encrypted);
		if (!err) {
			error = false;
			goto close;
		}

		if (err != -ENOMSG)
			goto close;

		/* Try PEM */
	}

	/*
	 * RFC 7486 allows whitespace and possibly other data before the
	 * PEM "encapsulation boundary" so rather than check if the start
	 * of the data looks like PEM, we fall back to this format if the
	 * data didn't look like anything else we knew about.
	 */
	if (cert_try_load_pem_format((const char *) file.data, file.st.st_size,
					password, out_certchain, out_privkey,
					out_encrypted))
		error = false;

close:
	pem_file_close(&file);
	return !error;
}
