/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <openssl/opensslv.h>
#include <openssl/opensslconf.h>

#if OPENSSL_VERSION_NUMBER >= 0x0090807fL
#ifndef OPENSSL_NO_CMS

#include "openssl_pkcs7.h"
#include "openssl_util.h"

#include <library.h>
#include <utils/debug.h>
#include <asn1/oid.h>
#include <credentials/sets/mem_cred.h>

#include <openssl/cms.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define X509_ATTRIBUTE_get0_object(attr) ({ (attr)->object; })
#endif

typedef struct private_openssl_pkcs7_t private_openssl_pkcs7_t;

/**
 * Private data of an openssl_pkcs7_t object.
 */
struct private_openssl_pkcs7_t {

	/**
	 * Public pkcs7_t interface.
	 */
	pkcs7_t public;

	/**
	 * Type of this container
	 */
	container_type_t type;

	/**
	 * OpenSSL CMS structure
	 */
	CMS_ContentInfo *cms;
};

/**
 * OpenSSL does not allow us to read the signature to verify it with our own
 * crypto API. We define the internal CMS_SignerInfo structure here to get it.
 */
struct CMS_SignerInfo_st {
	long version;
	void *sid;
	X509_ALGOR *digestAlgorithm;
	STACK_OF(X509_ATTRIBUTE) *signedAttrs;
	X509_ALGOR *signatureAlgorithm;
	ASN1_OCTET_STRING *signature;
	/* and more... */
};

/**
 * And we also need access to the wrappend CMS_KeyTransRecipientInfo to
 * read the encrypted key
 */
struct CMS_KeyTransRecipientInfo_st {
	long version;
	void *rid;
	X509_ALGOR *keyEncryptionAlgorithm;
	ASN1_OCTET_STRING *encryptedKey;
};

struct CMS_RecipientInfo_st {
	int type;
	struct CMS_KeyTransRecipientInfo_st *ktri;
	/* and more in union... */
};

struct CMS_EncryptedContentInfo_st {
	ASN1_OBJECT *contentType;
	X509_ALGOR *contentEncryptionAlgorithm;
	ASN1_OCTET_STRING *encryptedContent;
	/* and more... */
};

struct CMS_EnvelopedData_st {
	long version;
	void *originatorInfo;
	STACK_OF(CMS_RecipientInfo) *recipientInfos;
	struct CMS_EncryptedContentInfo_st *encryptedContentInfo;
	/* and more... */
};

struct CMS_ContentInfo_st {
	ASN1_OBJECT *contentType;
	struct CMS_EnvelopedData_st *envelopedData;
	/* and more in union... */
};

/**
 * We can't include asn1.h, declare function prototypes directly
 */
chunk_t asn1_wrap(int, const char *mode, ...);
int asn1_unwrap(chunk_t*, chunk_t*);

/**
 * Enumerator over certificates
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** Stack of X509 certificates */
	STACK_OF(X509) *certs;
	/** current enumerator position in certificates */
	int i;
	/** currently enumerating certificate_t */
	certificate_t *cert;
} cert_enumerator_t;

METHOD(enumerator_t, cert_destroy, void,
	cert_enumerator_t *this)
{
	DESTROY_IF(this->cert);
	free(this);
}

METHOD(enumerator_t, cert_enumerate, bool,
	cert_enumerator_t *this, va_list args)
{
	certificate_t **out;

	VA_ARGS_VGET(args, out);

	if (!this->certs)
	{
		return FALSE;
	}
	while (this->i < sk_X509_num(this->certs))
	{
		chunk_t encoding;
		X509 *x509;

		/* clean up previous round */
		DESTROY_IF(this->cert);
		this->cert = NULL;

		x509 = sk_X509_value(this->certs, this->i++);
		encoding = openssl_i2chunk(X509, x509);
		this->cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
										BUILD_BLOB_ASN1_DER, encoding,
										BUILD_END);
		free(encoding.ptr);
		if (!this->cert)
		{
			continue;
		}
		*out = this->cert;
		return TRUE;
	}
	return FALSE;
}

METHOD(pkcs7_t, create_cert_enumerator, enumerator_t*,
	private_openssl_pkcs7_t *this)
{
	cert_enumerator_t *enumerator;

	if (this->type == CONTAINER_PKCS7_SIGNED_DATA)
	{
		INIT(enumerator,
			.public = {
				.enumerate = enumerator_enumerate_default,
				.venumerate = _cert_enumerate,
				.destroy = _cert_destroy,
			},
			.certs = CMS_get1_certs(this->cms),
		);
		return &enumerator->public;
	}
	return enumerator_create_empty();
}

/**
 * Enumerator for signatures
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** Stack of signerinfos */
	STACK_OF(CMS_SignerInfo) *signers;
	/** current enumerator position in signers */
	int i;
	/** currently enumerating auth config */
	auth_cfg_t *auth;
	/** full CMS */
	CMS_ContentInfo *cms;
	/** credential set containing wrapped certificates */
	mem_cred_t *creds;
} signature_enumerator_t;

/**
 * Verify signerInfo signature
 */
static auth_cfg_t *verify_signature(CMS_SignerInfo *si, int hash_oid)
{
	enumerator_t *enumerator;
	public_key_t *key;
	certificate_t *cert;
	auth_cfg_t *auth, *found = NULL;
	identification_t *issuer, *serial;
	chunk_t attrs = chunk_empty, sig, attr;
	X509_NAME *name;
	ASN1_INTEGER *snr;
	int i;

	if (CMS_SignerInfo_get0_signer_id(si, NULL, &name, &snr) != 1)
	{
		return NULL;
	}
	issuer = openssl_x509_name2id(name);
	if (!issuer)
	{
		return NULL;
	}
	serial = identification_create_from_encoding(
									ID_KEY_ID, openssl_asn1_str2chunk(snr));

	/* reconstruct DER encoded attributes to verify signature */
	for (i = 0; i < CMS_signed_get_attr_count(si); i++)
	{
		attr = openssl_i2chunk(X509_ATTRIBUTE, CMS_signed_get_attr(si, i));
		attrs = chunk_cat("mm", attrs, attr);
	}
	/* wrap in a ASN1_SET */
	attrs = asn1_wrap(0x31, "m", attrs);

	/* TODO: find a better way to access and verify the signature */
	sig = openssl_asn1_str2chunk(si->signature);
	enumerator = lib->credmgr->create_trusted_enumerator(lib->credmgr,
														KEY_RSA, serial, FALSE);
	while (enumerator->enumerate(enumerator, &cert, &auth))
	{
		if (issuer->equals(issuer, cert->get_issuer(cert)))
		{
			key = cert->get_public_key(cert);
			if (key)
			{
				if (key->verify(key, signature_scheme_from_oid(hash_oid), NULL,
								attrs, sig))
				{
					found = auth->clone(auth);
					key->destroy(key);
					break;
				}
				key->destroy(key);
			}
		}
	}
	enumerator->destroy(enumerator);
	issuer->destroy(issuer);
	serial->destroy(serial);
	free(attrs.ptr);

	return found;
}

/**
 * Verify the message digest in the signerInfo attributes
 */
static bool verify_digest(CMS_ContentInfo *cms, CMS_SignerInfo *si, int hash_oid)
{
	ASN1_OCTET_STRING *os, **osp;
	hash_algorithm_t hash_alg;
	chunk_t digest, content, hash;
	hasher_t *hasher;

	os = CMS_signed_get0_data_by_OBJ(si,
				OBJ_nid2obj(NID_pkcs9_messageDigest), -3, V_ASN1_OCTET_STRING);
	if (!os)
	{
		return FALSE;
	}
	digest = openssl_asn1_str2chunk(os);
	osp = CMS_get0_content(cms);
	if (!osp)
	{
		return FALSE;
	}
	content = openssl_asn1_str2chunk(*osp);

	hash_alg = hasher_algorithm_from_oid(hash_oid);
	hasher = lib->crypto->create_hasher(lib->crypto, hash_alg);
	if (!hasher)
	{
		DBG1(DBG_LIB, "hash algorithm %N not supported",
			 hash_algorithm_names, hash_alg);
		return FALSE;
	}
	if (!hasher->allocate_hash(hasher, content, &hash))
	{
		hasher->destroy(hasher);
		return FALSE;
	}
	hasher->destroy(hasher);

	if (!chunk_equals_const(digest, hash))
	{
		free(hash.ptr);
		DBG1(DBG_LIB, "invalid messageDigest");
		return FALSE;
	}
	free(hash.ptr);
	return TRUE;
}

METHOD(enumerator_t, signature_enumerate, bool,
	signature_enumerator_t *this, va_list args)
{
	auth_cfg_t **out;

	VA_ARGS_VGET(args, out);

	if (!this->signers)
	{
		return FALSE;
	}
	while (this->i < sk_CMS_SignerInfo_num(this->signers))
	{
		CMS_SignerInfo *si;
		X509_ALGOR *digest, *sig;
		int hash_oid;

		/* clean up previous round */
		DESTROY_IF(this->auth);
		this->auth = NULL;

		si = sk_CMS_SignerInfo_value(this->signers, this->i++);

		CMS_SignerInfo_get0_algs(si, NULL, NULL, &digest, &sig);
		hash_oid = openssl_asn1_known_oid(digest->algorithm);
		if (openssl_asn1_known_oid(sig->algorithm) != OID_RSA_ENCRYPTION)
		{
			DBG1(DBG_LIB, "only RSA digest encryption supported");
			continue;
		}
		this->auth = verify_signature(si, hash_oid);
		if (!this->auth)
		{
			DBG1(DBG_LIB, "unable to verify pkcs7 attributes signature");
			continue;
		}
		if (!verify_digest(this->cms, si, hash_oid))
		{
			continue;
		}
		*out = this->auth;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, signature_destroy, void,
	signature_enumerator_t *this)
{
	lib->credmgr->remove_local_set(lib->credmgr, &this->creds->set);
	this->creds->destroy(this->creds);
	DESTROY_IF(this->auth);
	free(this);
}

METHOD(container_t, create_signature_enumerator, enumerator_t*,
	private_openssl_pkcs7_t *this)
{
	signature_enumerator_t *enumerator;

	if (this->type == CONTAINER_PKCS7_SIGNED_DATA)
	{
		enumerator_t *certs;
		certificate_t *cert;

		INIT(enumerator,
			.public = {
				.enumerate = enumerator_enumerate_default,
				.venumerate = _signature_enumerate,
				.destroy = _signature_destroy,
			},
			.cms = this->cms,
			.signers = CMS_get0_SignerInfos(this->cms),
			.creds = mem_cred_create(),
		);

		/* make available wrapped certs during signature checking */
		certs = create_cert_enumerator(this);
		while (certs->enumerate(certs, &cert))
		{
			enumerator->creds->add_cert(enumerator->creds, FALSE,
										cert->get_ref(cert));
		}
		certs->destroy(certs);

		lib->credmgr->add_local_set(lib->credmgr, &enumerator->creds->set,
									FALSE);

		return &enumerator->public;
	}
	return enumerator_create_empty();
}


METHOD(container_t, get_type, container_type_t,
	private_openssl_pkcs7_t *this)
{
	return this->type;
}

METHOD(pkcs7_t, get_attribute, bool,
	private_openssl_pkcs7_t *this, int oid,
	enumerator_t *enumerator, chunk_t *value)
{
	signature_enumerator_t *e;
	CMS_SignerInfo *si;
	X509_ATTRIBUTE *attr;
	ASN1_TYPE *type;
	chunk_t chunk, wrapped;
	int i;

	e = (signature_enumerator_t*)enumerator;
	if (e->i <= 0)
	{
		return FALSE;
	}

	/* "i" gets incremeneted after enumerate(), hence read from previous */
	si = sk_CMS_SignerInfo_value(e->signers, e->i - 1);
	for (i = 0; i < CMS_signed_get_attr_count(si); i++)
	{
		attr = CMS_signed_get_attr(si, i);
		if (X509_ATTRIBUTE_count(attr) == 1 &&
			openssl_asn1_known_oid(X509_ATTRIBUTE_get0_object(attr)) == oid)
		{
			/* get first value in SET */
			type = X509_ATTRIBUTE_get0_type(attr, 0);
			chunk = wrapped = openssl_i2chunk(ASN1_TYPE, type);
			if (asn1_unwrap(&chunk, &chunk) != 0x100 /* ASN1_INVALID */)
			{
				*value = chunk_clone(chunk);
				free(wrapped.ptr);
				return TRUE;
			}
			free(wrapped.ptr);
		}
	}
	return FALSE;
}

/**
 * Find a private key for issuerAndSerialNumber
 */
static private_key_t *find_private(identification_t *issuer,
								   identification_t *serial)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	public_key_t *public;
	private_key_t *private = NULL;
	identification_t *id;
	chunk_t fp;

	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
											CERT_X509, KEY_RSA, serial, FALSE);
	while (enumerator->enumerate(enumerator, &cert))
	{
		if (issuer->equals(issuer, cert->get_issuer(cert)))
		{
			public = cert->get_public_key(cert);
			if (public)
			{
				if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &fp))
				{
					id = identification_create_from_encoding(ID_KEY_ID, fp);
					private = lib->credmgr->get_private(lib->credmgr,
														KEY_ANY, id, NULL);
					id->destroy(id);
				}
				public->destroy(public);
			}
		}
		if (private)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return private;
}

/**
 * Decrypt enveloped-data with a decrypted symmetric key
 */
static bool decrypt_symmetric(private_openssl_pkcs7_t *this, chunk_t key,
							  chunk_t encrypted, chunk_t *plain)
{
	encryption_algorithm_t encr;
	X509_ALGOR *alg;
	crypter_t *crypter;
	chunk_t iv;
	size_t key_size;

	/* read encryption algorithm from internal structures; TODO fixup */
	alg = this->cms->envelopedData->encryptedContentInfo->
												contentEncryptionAlgorithm;
	encr = encryption_algorithm_from_oid(openssl_asn1_known_oid(alg->algorithm),
										 &key_size);
	if (alg->parameter->type != V_ASN1_OCTET_STRING)
	{
		return FALSE;
	}
	iv = openssl_asn1_str2chunk(alg->parameter->value.octet_string);

	crypter = lib->crypto->create_crypter(lib->crypto, encr, key_size / 8);
	if (!crypter)
	{
		DBG1(DBG_LIB, "crypter %N-%d not available",
			 encryption_algorithm_names, alg, key_size);
		return FALSE;
	}
	if (key.len != crypter->get_key_size(crypter))
	{
		DBG1(DBG_LIB, "symmetric key length is wrong");
		crypter->destroy(crypter);
		return FALSE;
	}
	if (iv.len != crypter->get_iv_size(crypter))
	{
		DBG1(DBG_LIB, "IV length is wrong");
		crypter->destroy(crypter);
		return FALSE;
	}
	if (!crypter->set_key(crypter, key) ||
		!crypter->decrypt(crypter, encrypted, iv, plain))
	{
		crypter->destroy(crypter);
		return FALSE;
	}
	crypter->destroy(crypter);
	return TRUE;
}

/**
 * Remove enveloped-data PKCS#7 padding from plain data
 */
static bool remove_padding(chunk_t *data)
{
	u_char *pos;
	u_char pattern;
	size_t padding;

	if (!data->len)
	{
		return FALSE;
	}
	pos = data->ptr + data->len - 1;
	padding = pattern = *pos;

	if (padding > data->len)
	{
		DBG1(DBG_LIB, "padding greater than data length");
		return FALSE;
	}
	data->len -= padding;

	while (padding-- > 0)
	{
		if (*pos-- != pattern)
		{
			DBG1(DBG_LIB, "wrong padding pattern");
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Decrypt PKCS#7 enveloped-data
 */
static bool decrypt(private_openssl_pkcs7_t *this,
					chunk_t encrypted, chunk_t *plain)
{
	STACK_OF(CMS_RecipientInfo) *ris;
	CMS_RecipientInfo *ri;
	chunk_t chunk, key = chunk_empty;
	int i;

	ris = CMS_get0_RecipientInfos(this->cms);
	for (i = 0; i < sk_CMS_RecipientInfo_num(ris); i++)
	{
		ri = sk_CMS_RecipientInfo_value(ris, i);
		if (CMS_RecipientInfo_type(ri) == CMS_RECIPINFO_TRANS)
		{
			identification_t *serial, *issuer;
			private_key_t *private;
			X509_ALGOR *alg;
			X509_NAME *name;
			ASN1_INTEGER *sn;
			u_char zero = 0;
			int oid;

			if (CMS_RecipientInfo_ktri_get0_algs(ri, NULL, NULL, &alg) == 1 &&
				CMS_RecipientInfo_ktri_get0_signer_id(ri, NULL, &name, &sn) == 1)
			{
				oid = openssl_asn1_known_oid(alg->algorithm);
				if (oid != OID_RSA_ENCRYPTION)
				{
					DBG1(DBG_LIB, "only RSA encryption supported in PKCS#7");
					continue;
				}
				issuer = openssl_x509_name2id(name);
				if (!issuer)
				{
					continue;
				}
				chunk = openssl_asn1_str2chunk(sn);
				if (chunk.len && chunk.ptr[0] & 0x80)
				{	/* if MSB is set, append a zero to make it non-negative */
					chunk = chunk_cata("cc", chunk_from_thing(zero), chunk);
				}
				serial = identification_create_from_encoding(ID_KEY_ID, chunk);
				private = find_private(issuer, serial);
				issuer->destroy(issuer);
				serial->destroy(serial);

				if (private)
				{
					/* get encryptedKey from internal structure; TODO fixup */
					chunk = openssl_asn1_str2chunk(ri->ktri->encryptedKey);
					if (private->decrypt(private, ENCRYPT_RSA_PKCS1,
										 chunk, &key))
					{
						private->destroy(private);
						break;
					}
					private->destroy(private);
				}
			}
		}
	}
	if (!key.len)
	{
		DBG1(DBG_LIB, "no private key found to decrypt PKCS#7");
		return FALSE;
	}
	if (!decrypt_symmetric(this, key, encrypted, plain))
	{
		chunk_clear(&key);
		return FALSE;
	}
	chunk_clear(&key);
	if (!remove_padding(plain))
	{
		free(plain->ptr);
		return FALSE;
	}
	return TRUE;
}

METHOD(container_t, get_data, bool,
	private_openssl_pkcs7_t *this, chunk_t *data)
{
	ASN1_OCTET_STRING **os;
	chunk_t chunk;

	os = CMS_get0_content(this->cms);
	if (os)
	{
		chunk = openssl_asn1_str2chunk(*os);
		switch (this->type)
		{
			case CONTAINER_PKCS7_DATA:
			case CONTAINER_PKCS7_SIGNED_DATA:
				*data = chunk_clone(chunk);
				return TRUE;
			case CONTAINER_PKCS7_ENVELOPED_DATA:
				return decrypt(this, chunk, data);
			default:
				break;
		}
	}
	return FALSE;
}

METHOD(container_t, get_encoding, bool,
	private_openssl_pkcs7_t *this, chunk_t *data)
{
	return FALSE;
}

METHOD(container_t, destroy, void,
	private_openssl_pkcs7_t *this)
{
	CMS_ContentInfo_free(this->cms);
	free(this);
}

/**
 * Generic constructor
 */
static private_openssl_pkcs7_t* create_empty()
{
	private_openssl_pkcs7_t *this;

	INIT(this,
		.public = {
			.container = {
				.get_type = _get_type,
				.create_signature_enumerator = _create_signature_enumerator,
				.get_data = _get_data,
				.get_encoding = _get_encoding,
				.destroy = _destroy,
			},
			.get_attribute = _get_attribute,
			.create_cert_enumerator = _create_cert_enumerator,
		},
	);

	return this;
}

/**
 * Parse a PKCS#7 container
 */
static bool parse(private_openssl_pkcs7_t *this, chunk_t blob)
{
	BIO *bio;

	bio = BIO_new_mem_buf(blob.ptr, blob.len);
	this->cms = d2i_CMS_bio(bio, NULL);
	BIO_free(bio);

	if (!this->cms)
	{
		return FALSE;
	}
	switch (openssl_asn1_known_oid((ASN1_OBJECT*)CMS_get0_type(this->cms)))
	{
		case OID_PKCS7_DATA:
			this->type = CONTAINER_PKCS7_DATA;
			break;
		case OID_PKCS7_SIGNED_DATA:
			this->type = CONTAINER_PKCS7_SIGNED_DATA;
			break;
		case OID_PKCS7_ENVELOPED_DATA:
			this->type = CONTAINER_PKCS7_ENVELOPED_DATA;
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

/**
 * See header
 */
pkcs7_t *openssl_pkcs7_load(container_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;
	private_openssl_pkcs7_t *this;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (blob.len)
	{
		this = create_empty();
		if (parse(this, blob))
		{
			return &this->public;
		}
		destroy(this);
	}
	return NULL;
}

#endif /* OPENSSL_NO_CMS */
#endif /* OPENSSL_VERSION_NUMBER */
