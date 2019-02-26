/*
 * Copyright (C) 2011-2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
/*
 * Copyright (C) 2016 EDF S.A.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "pkcs11_private_key.h"

#include "pkcs11_library.h"
#include "pkcs11_manager.h"
#include "pkcs11_public_key.h"

#include <utils/debug.h>
#include <asn1/asn1.h>

typedef struct private_pkcs11_private_key_t private_pkcs11_private_key_t;

/**
 * Private data of an pkcs11_private_key_t object.
 */
struct private_pkcs11_private_key_t {

	/**
	 * Public pkcs11_private_key_t interface.
	 */
	pkcs11_private_key_t public;

	/**
	 * PKCS#11 module
	 */
	pkcs11_library_t *lib;

	/**
	 * Slot the token is in
	 */
	CK_SLOT_ID slot;

	/**
	 * Token session
	 */
	CK_SESSION_HANDLE session;

	/**
	 * Key object on the token
	 */
	CK_OBJECT_HANDLE object;

	/**
	 * Key requires reauthentication for each signature/decryption
	 */
	CK_BBOOL reauth;

	/**
	 * Keyid of the key we use
	 */
	identification_t *keyid;

	/**
	 * Associated public key
	 */
	public_key_t *pubkey;

	/**
	 * References to this key
	 */
	refcount_t ref;

	/**
	 * Type of this private key
	 */
	key_type_t type;
};


METHOD(private_key_t, get_type, key_type_t,
	private_pkcs11_private_key_t *this)
{
	return this->type;
}

METHOD(private_key_t, get_keysize, int,
	private_pkcs11_private_key_t *this)
{
	return this->pubkey->get_keysize(this->pubkey);
}

/**
 * See header.
 */
CK_MECHANISM_PTR pkcs11_signature_scheme_to_mech(signature_scheme_t scheme,
												 key_type_t type, size_t keylen,
												 hash_algorithm_t *hash)
{
	static struct {
		signature_scheme_t scheme;
		CK_MECHANISM mechanism;
		key_type_t type;
		size_t keylen;
		hash_algorithm_t hash;
	} mappings[] = {
		{SIGN_RSA_EMSA_PKCS1_NULL,		{CKM_RSA_PKCS,			NULL, 0},
		 KEY_RSA, 0,									   HASH_UNKNOWN},
		{SIGN_RSA_EMSA_PKCS1_SHA2_256,	{CKM_SHA256_RSA_PKCS,	NULL, 0},
		 KEY_RSA, 0,									   HASH_UNKNOWN},
		{SIGN_RSA_EMSA_PKCS1_SHA2_384,	{CKM_SHA384_RSA_PKCS,	NULL, 0},
		 KEY_RSA, 0,									   HASH_UNKNOWN},
		{SIGN_RSA_EMSA_PKCS1_SHA2_512,	{CKM_SHA512_RSA_PKCS,	NULL, 0},
		 KEY_RSA, 0,									   HASH_UNKNOWN},
		{SIGN_RSA_EMSA_PKCS1_SHA1,		{CKM_SHA1_RSA_PKCS,		NULL, 0},
		 KEY_RSA, 0,									   HASH_UNKNOWN},
		{SIGN_RSA_EMSA_PKCS1_MD5,		{CKM_MD5_RSA_PKCS,		NULL, 0},
		 KEY_RSA, 0,									   HASH_UNKNOWN},
		{SIGN_ECDSA_WITH_NULL,			{CKM_ECDSA,				NULL, 0},
		 KEY_ECDSA, 0,									   HASH_UNKNOWN},
		{SIGN_ECDSA_WITH_SHA1_DER,		{CKM_ECDSA_SHA1,		NULL, 0},
		 KEY_ECDSA, 0,									   HASH_UNKNOWN},
		{SIGN_ECDSA_WITH_SHA256_DER,	{CKM_ECDSA,				NULL, 0},
		 KEY_ECDSA, 0,										HASH_SHA256},
		{SIGN_ECDSA_WITH_SHA384_DER,	{CKM_ECDSA,				NULL, 0},
		 KEY_ECDSA, 0,										HASH_SHA384},
		{SIGN_ECDSA_WITH_SHA512_DER,	{CKM_ECDSA,				NULL, 0},
		 KEY_ECDSA, 0,										HASH_SHA512},
		{SIGN_ECDSA_256,				{CKM_ECDSA,				NULL, 0},
		 KEY_ECDSA, 256,									HASH_SHA256},
		{SIGN_ECDSA_384,				{CKM_ECDSA,				NULL, 0},
		 KEY_ECDSA, 384,									HASH_SHA384},
		{SIGN_ECDSA_521,				{CKM_ECDSA,				NULL, 0},
		 KEY_ECDSA, 521,									HASH_SHA512},
	};
	int i;

	for (i = 0; i < countof(mappings); i++)
	{
		if (mappings[i].scheme == scheme)
		{
			size_t len = mappings[i].keylen;
			if (mappings[i].type != type || (len && keylen != len))
			{
				return NULL;
			}
			if (hash)
			{
				*hash = mappings[i].hash;
			}
			return &mappings[i].mechanism;
		}
	}
	return NULL;
}

/**
 * See header.
 */
CK_MECHANISM_PTR pkcs11_encryption_scheme_to_mech(encryption_scheme_t scheme)
{
	static struct {
		encryption_scheme_t scheme;
		CK_MECHANISM mechanism;
	} mappings[] = {
		{ENCRYPT_RSA_PKCS1,			{CKM_RSA_PKCS,				NULL, 0}},
		{ENCRYPT_RSA_OAEP_SHA1,		{CKM_RSA_PKCS_OAEP,			NULL, 0}},
	};
	int i;

	for (i = 0; i < countof(mappings); i++)
	{
		if (mappings[i].scheme == scheme)
		{
			return &mappings[i].mechanism;
		}
	}
	return NULL;
}

/**
 * Reauthenticate to do a signature
 */
static bool reauth(private_pkcs11_private_key_t *this,
				   CK_SESSION_HANDLE session)
{
	enumerator_t *enumerator;
	shared_key_t *shared;
	chunk_t pin;
	CK_RV rv;
	bool found = FALSE, success = FALSE;

	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
												SHARED_PIN, this->keyid, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		found = TRUE;
		pin = shared->get_key(shared);
		rv = this->lib->f->C_Login(session, CKU_CONTEXT_SPECIFIC,
								   pin.ptr, pin.len);
		if (rv == CKR_OK)
		{
			success = TRUE;
			break;
		}
		DBG1(DBG_CFG, "reauthentication login failed: %N", ck_rv_names, rv);
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		DBG1(DBG_CFG, "private key requires reauthentication, but no PIN found");
		return FALSE;
	}
	return success;
}

METHOD(private_key_t, sign, bool,
	private_pkcs11_private_key_t *this, signature_scheme_t scheme, void *params,
	chunk_t data, chunk_t *signature)
{
	CK_MECHANISM_PTR mechanism;
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR buf;
	CK_ULONG len;
	CK_RV rv;
	hash_algorithm_t hash_alg;
	chunk_t hash = chunk_empty;

	mechanism = pkcs11_signature_scheme_to_mech(scheme, this->type,
												get_keysize(this), &hash_alg);
	if (!mechanism)
	{
		DBG1(DBG_LIB, "signature scheme %N not supported",
			 signature_scheme_names, scheme);
		return FALSE;
	}
	rv = this->lib->f->C_OpenSession(this->slot, CKF_SERIAL_SESSION, NULL, NULL,
									 &session);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "opening PKCS#11 session failed: %N", ck_rv_names, rv);
		return FALSE;
	}
	rv = this->lib->f->C_SignInit(session, mechanism, this->object);
	if (this->reauth && !reauth(this, session))
	{
		this->lib->f->C_CloseSession(session);
		return FALSE;
	}
	if (rv != CKR_OK)
	{
		this->lib->f->C_CloseSession(session);
		DBG1(DBG_LIB, "C_SignInit() failed: %N", ck_rv_names, rv);
		return FALSE;
	}
	if (hash_alg != HASH_UNKNOWN)
	{
		hasher_t *hasher;

		hasher = lib->crypto->create_hasher(lib->crypto, hash_alg);
		if (!hasher || !hasher->allocate_hash(hasher, data, &hash))
		{
			DESTROY_IF(hasher);
			this->lib->f->C_CloseSession(session);
			return FALSE;
		}
		hasher->destroy(hasher);
		data = hash;
	}
	len = (get_keysize(this) + 7) / 8;
	if (this->type == KEY_ECDSA)
	{	/* signature is twice the length of the base point order */
		len *= 2;
	}
	buf = malloc(len);
	rv = this->lib->f->C_Sign(session, data.ptr, data.len, buf, &len);
	this->lib->f->C_CloseSession(session);
	chunk_free(&hash);
	if (rv != CKR_OK)
	{
		DBG1(DBG_LIB, "C_Sign() failed: %N", ck_rv_names, rv);
		free(buf);
		return FALSE;
	}
	switch (scheme)
	{
		case SIGN_ECDSA_WITH_SHA1_DER:
		case SIGN_ECDSA_WITH_SHA256_DER:
		case SIGN_ECDSA_WITH_SHA384_DER:
		case SIGN_ECDSA_WITH_SHA512_DER:
		{
			chunk_t r, s;

			/* return an ASN.1 encoded sequence of integers r and s, removing
			 * any zero-padding */
			len /= 2;
			r = chunk_skip_zero(chunk_create(buf, len));
			s = chunk_skip_zero(chunk_create(buf+len, len));
			*signature = asn1_wrap(ASN1_SEQUENCE, "mm",
								   asn1_integer("c", r), asn1_integer("c", s));
			free(buf);
			break;
		}
		default:
			*signature = chunk_create(buf, len);
			break;
	}
	return TRUE;
}

METHOD(private_key_t, decrypt, bool,
	private_pkcs11_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypt, chunk_t *plain)
{
	CK_MECHANISM_PTR mechanism;
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR buf;
	CK_ULONG len;
	CK_RV rv;

	mechanism = pkcs11_encryption_scheme_to_mech(scheme);
	if (!mechanism)
	{
		DBG1(DBG_LIB, "encryption scheme %N not supported",
			 encryption_scheme_names, scheme);
		return FALSE;
	}
	rv = this->lib->f->C_OpenSession(this->slot, CKF_SERIAL_SESSION, NULL, NULL,
									 &session);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "opening PKCS#11 session failed: %N", ck_rv_names, rv);
		return FALSE;
	}
	rv = this->lib->f->C_DecryptInit(session, mechanism, this->object);
	if (this->reauth && !reauth(this, session))
	{
		this->lib->f->C_CloseSession(session);
		return FALSE;
	}
	if (rv != CKR_OK)
	{
		this->lib->f->C_CloseSession(session);
		DBG1(DBG_LIB, "C_DecryptInit() failed: %N", ck_rv_names, rv);
		return FALSE;
	}
	len = (get_keysize(this) + 7) / 8;
	buf = malloc(len);
	rv = this->lib->f->C_Decrypt(session, crypt.ptr, crypt.len, buf, &len);
	this->lib->f->C_CloseSession(session);
	if (rv != CKR_OK)
	{
		DBG1(DBG_LIB, "C_Decrypt() failed: %N", ck_rv_names, rv);
		free(buf);
		return FALSE;
	}
	*plain = chunk_create(buf, len);
	return TRUE;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_pkcs11_private_key_t *this)
{
	return this->pubkey->get_ref(this->pubkey);
}

METHOD(private_key_t, get_fingerprint, bool,
	private_pkcs11_private_key_t *this, cred_encoding_type_t type,
	chunk_t *fingerprint)
{
	return this->pubkey->get_fingerprint(this->pubkey, type, fingerprint);
}

METHOD(private_key_t, get_encoding, bool,
	private_pkcs11_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	return FALSE;
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_pkcs11_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_pkcs11_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		if (this->pubkey)
		{
			this->pubkey->destroy(this->pubkey);
		}
		this->keyid->destroy(this->keyid);
		this->lib->f->C_CloseSession(this->session);
		free(this);
	}
}

/**
 * Find the PKCS#11 library by its friendly name
 */
static pkcs11_library_t* find_lib(char *module)
{
	pkcs11_manager_t *manager;
	enumerator_t *enumerator;
	pkcs11_library_t *p11, *found = NULL;
	CK_SLOT_ID slot;

	manager = lib->get(lib, "pkcs11-manager");
	if (!manager)
	{
		return NULL;
	}
	enumerator = manager->create_token_enumerator(manager);
	while (enumerator->enumerate(enumerator, &p11, &slot))
	{
		if (streq(module, p11->get_name(p11)))
		{
			found = p11;
			break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Find the PKCS#11 lib having a keyid, and optionally a slot
 */
static pkcs11_library_t* find_lib_by_keyid(chunk_t keyid, int *slot,
										   CK_OBJECT_CLASS class)
{
	pkcs11_manager_t *manager;
	enumerator_t *enumerator;
	pkcs11_library_t *p11, *found = NULL;
	CK_SLOT_ID current;

	manager = lib->get(lib, "pkcs11-manager");
	if (!manager)
	{
		return NULL;
	}
	enumerator = manager->create_token_enumerator(manager);
	while (enumerator->enumerate(enumerator, &p11, &current))
	{
		if (*slot == -1 || *slot == current)
		{
			/* look for a pubkey/cert, it is usually readable without login */
			CK_ATTRIBUTE tmpl[] = {
				{CKA_CLASS, &class, sizeof(class)},
				{CKA_ID, keyid.ptr, keyid.len},
			};
			CK_OBJECT_HANDLE object;
			CK_SESSION_HANDLE session;
			CK_RV rv;
			enumerator_t *keys;

			rv = p11->f->C_OpenSession(current, CKF_SERIAL_SESSION, NULL, NULL,
									   &session);
			if (rv != CKR_OK)
			{
				DBG1(DBG_CFG, "opening PKCS#11 session failed: %N",
					 ck_rv_names, rv);
				continue;
			}
			keys = p11->create_object_enumerator(p11, session,
												 tmpl, countof(tmpl), NULL, 0);
			if (keys->enumerate(keys, &object))
			{
				DBG1(DBG_CFG, "found key on PKCS#11 token '%s':%d",
					 p11->get_name(p11), current);
				found = p11;
				*slot = current;
			}
			keys->destroy(keys);
			p11->f->C_CloseSession(session);
			if (found)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Find the PKCS#11 lib and CKA_ID of the certificate object of a given
 * subjectKeyIdentifier and optional slot
 */
static pkcs11_library_t* find_lib_and_keyid_by_skid(chunk_t keyid_chunk,
													chunk_t *ckaid, int *slot)
{
	CK_OBJECT_CLASS class = CKO_CERTIFICATE;
	CK_CERTIFICATE_TYPE type = CKC_X_509;
	CK_ATTRIBUTE tmpl[] = {
		{CKA_CLASS, &class, sizeof(class)},
		{CKA_CERTIFICATE_TYPE, &type, sizeof(type)},
	};
	CK_ATTRIBUTE attr[] = {
		{CKA_VALUE, NULL, 0},
		{CKA_ID, NULL, 0},
	};
	CK_OBJECT_HANDLE object;
	CK_SESSION_HANDLE session;
	CK_RV rv;
	pkcs11_manager_t *manager;
	enumerator_t *enumerator, *certs;
	identification_t *keyid;
	pkcs11_library_t *p11, *found = NULL;
	CK_SLOT_ID current;
	linked_list_t *raw;
	certificate_t *cert;
	struct {
		chunk_t value;
		chunk_t ckaid;
	} *entry;

	manager = lib->get(lib, "pkcs11-manager");
	if (!manager)
	{
		return NULL;
	}

	keyid = identification_create_from_encoding(ID_KEY_ID, keyid_chunk);
	/* store result in a temporary list, avoid recursive operation */
	raw = linked_list_create();

	enumerator = manager->create_token_enumerator(manager);
	while (enumerator->enumerate(enumerator, &p11, &current))
	{
		if (*slot != -1 && *slot != current)
		{
			continue;
		}
		rv = p11->f->C_OpenSession(current, CKF_SERIAL_SESSION, NULL, NULL,
								   &session);
		if (rv != CKR_OK)
		{
			DBG1(DBG_CFG, "opening PKCS#11 session failed: %N",
				 ck_rv_names, rv);
			continue;
		}
		certs = p11->create_object_enumerator(p11, session, tmpl, countof(tmpl),
											  attr, countof(attr));
		while (certs->enumerate(certs, &object))
		{
			INIT(entry,
				.value = chunk_clone(
							chunk_create(attr[0].pValue, attr[0].ulValueLen)),
				.ckaid = chunk_clone(
							chunk_create(attr[1].pValue, attr[1].ulValueLen)),
			);
			raw->insert_last(raw, entry);
		}
		certs->destroy(certs);

		while (raw->remove_first(raw, (void**)&entry) == SUCCESS)
		{
			if (!found)
			{
				cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
										  CERT_X509, BUILD_BLOB_ASN1_DER,
										  entry->value, BUILD_END);
				if (cert)
				{
					if (cert->has_subject(cert, keyid))
					{
						DBG1(DBG_CFG, "found cert with keyid '%#B' on PKCS#11 "
							 "token '%s':%d", &keyid_chunk, p11->get_name(p11),
							 current);
						found = p11;
						*ckaid = chunk_clone(entry->ckaid);
						*slot = current;
					}
					cert->destroy(cert);
				}
				else
				{
					DBG1(DBG_CFG, "parsing cert with CKA_ID '%#B' on PKCS#11 "
						 "token '%s':%d failed", &entry->ckaid,
						 p11->get_name(p11), current);
				}
			}
			chunk_free(&entry->value);
			chunk_free(&entry->ckaid);
			free(entry);
		}
		p11->f->C_CloseSession(session);
		if (found)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	keyid->destroy(keyid);
	raw->destroy(raw);
	return found;
}

/**
 * Find the key on the token
 */
static bool find_key(private_pkcs11_private_key_t *this, chunk_t keyid)
{
	CK_OBJECT_CLASS class = CKO_PRIVATE_KEY;
	CK_ATTRIBUTE tmpl[] = {
		{CKA_CLASS, &class, sizeof(class)},
		{CKA_ID, keyid.ptr, keyid.len},
	};
	CK_OBJECT_HANDLE object;
	CK_KEY_TYPE type;
	CK_BBOOL reauth = FALSE;
	CK_ATTRIBUTE attr[] = {
		{CKA_KEY_TYPE, &type, sizeof(type)},
		{CKA_ALWAYS_AUTHENTICATE, &reauth, sizeof(reauth)},
	};
	enumerator_t *enumerator;
	int count = countof(attr);
	bool found = FALSE;

	/* do not use CKA_ALWAYS_AUTHENTICATE if not supported */
	if (!(this->lib->get_features(this->lib) & PKCS11_ALWAYS_AUTH_KEYS))
	{
		count--;
	}
	enumerator = this->lib->create_object_enumerator(this->lib,
							this->session, tmpl, countof(tmpl), attr, count);
	if (enumerator->enumerate(enumerator, &object))
	{
		this->type = KEY_RSA;
		switch (type)
		{
			case CKK_ECDSA:
				this->type = KEY_ECDSA;
				/* fall-through */
			case CKK_RSA:
				this->reauth = reauth;
				this->object = object;
				found = TRUE;
				break;
			default:
				DBG1(DBG_CFG, "PKCS#11 key type %d not supported", type);
				break;
		}
	}
	enumerator->destroy(enumerator);
	return found;
}

/**
 * Find a PIN and try to log in
 */
static bool login(private_pkcs11_private_key_t *this, int slot)
{
	enumerator_t *enumerator;
	shared_key_t *shared;
	chunk_t pin;
	CK_RV rv;
	CK_SESSION_INFO info;
	bool found = FALSE, success = FALSE;

	rv = this->lib->f->C_GetSessionInfo(this->session, &info);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "C_GetSessionInfo failed: %N", ck_rv_names, rv);
		return FALSE;
	}
	if (info.state != CKS_RO_PUBLIC_SESSION &&
		info.state != CKS_RW_PUBLIC_SESSION)
	{	/* already logged in with another session, skip */
		return TRUE;
	}

	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
												SHARED_PIN, this->keyid, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		found = TRUE;
		pin = shared->get_key(shared);
		rv = this->lib->f->C_Login(this->session, CKU_USER, pin.ptr, pin.len);
		if (rv == CKR_OK)
		{
			success = TRUE;
			break;
		}
		DBG1(DBG_CFG, "login to '%s':%d failed: %N",
			 this->lib->get_name(this->lib), slot, ck_rv_names, rv);
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		DBG1(DBG_CFG, "no PIN found for PKCS#11 key %Y", this->keyid);
		return FALSE;
	}
	return success;
}

/**
 * Get a public key from a certificate with a given key ID.
 */
static public_key_t* find_pubkey_in_certs(private_pkcs11_private_key_t *this,
										  chunk_t keyid)
{
	CK_OBJECT_CLASS class = CKO_CERTIFICATE;
	CK_CERTIFICATE_TYPE type = CKC_X_509;
	CK_ATTRIBUTE tmpl[] = {
		{CKA_CLASS, &class, sizeof(class)},
		{CKA_CERTIFICATE_TYPE, &type, sizeof(type)},
		{CKA_ID, keyid.ptr, keyid.len},
	};
	CK_OBJECT_HANDLE object;
	CK_ATTRIBUTE attr[] = {
		{CKA_VALUE, NULL, 0},
	};
	enumerator_t *enumerator;
	chunk_t data = chunk_empty;
	public_key_t *key = NULL;
	certificate_t *cert;

	enumerator = this->lib->create_object_enumerator(this->lib, this->session,
									tmpl, countof(tmpl), attr, countof(attr));
	if (enumerator->enumerate(enumerator, &object))
	{
		data = chunk_clone(chunk_create(attr[0].pValue, attr[0].ulValueLen));
	}
	enumerator->destroy(enumerator);

	if (data.ptr)
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_BLOB_ASN1_DER, data, BUILD_END);
		free(data.ptr);
		if (cert)
		{
			key = cert->get_public_key(cert);
			cert->destroy(cert);
		}
	}
	return key;
}

/**
 * See header.
 */
pkcs11_private_key_t *pkcs11_private_key_connect(key_type_t type, va_list args)
{
	private_pkcs11_private_key_t *this;
	char *module = NULL;
	chunk_t keyid = chunk_empty, ckaid = chunk_empty;
	int slot = -1;
	CK_RV rv;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_PKCS11_KEYID:
				keyid = va_arg(args, chunk_t);
				continue;
			case BUILD_PKCS11_SLOT:
				slot = va_arg(args, int);
				continue;
			case BUILD_PKCS11_MODULE:
				module = va_arg(args, char*);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!keyid.len)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.sign = _sign,
				.decrypt = _decrypt,
				.get_keysize = _get_keysize,
				.get_public_key = _get_public_key,
				.equals = private_key_equals,
				.belongs_to = private_key_belongs_to,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = private_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.ref = 1,
	);

	if (module && slot != -1)
	{
		this->lib = find_lib(module);
		if (!this->lib)
		{
			DBG1(DBG_CFG, "PKCS#11 module '%s' not found", module);
			free(this);
			return NULL;
		}
	}
	else
	{
		this->lib = find_lib_by_keyid(keyid, &slot, CKO_PUBLIC_KEY);
		if (!this->lib)
		{
			this->lib = find_lib_by_keyid(keyid, &slot, CKO_CERTIFICATE);
		}
		if (!this->lib)
		{
			this->lib = find_lib_and_keyid_by_skid(keyid, &ckaid, &slot);
		}
		if (!this->lib)
		{
			DBG1(DBG_CFG, "no PKCS#11 module found having a keyid %#B", &keyid);
			free(this);
			return NULL;
		}
	}

	rv = this->lib->f->C_OpenSession(slot, CKF_SERIAL_SESSION,
									 NULL, NULL, &this->session);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "opening private key session on '%s':%d failed: %N",
			 module, slot, ck_rv_names, rv);
		free(this);
		return NULL;
	}

	this->slot = slot;
	this->keyid = identification_create_from_encoding(ID_KEY_ID, keyid);

	if (!login(this, slot))
	{
		destroy(this);
		return NULL;
	}

	if (ckaid.ptr)
	{
		DBG1(DBG_CFG, "using CKA_ID '%#B' for key with keyid '%#B'",
			 &ckaid, &keyid);
		keyid = ckaid;
	}

	if (!find_key(this, keyid))
	{
		DBG1(DBG_CFG, "did not find the key with %s '%#B'",
			 ckaid.ptr ? "CKA_ID" : "keyid", &keyid);
		destroy(this);
		return NULL;
	}

	this->pubkey = pkcs11_public_key_connect(this->lib, slot, this->type, keyid);
	if (!this->pubkey)
	{
		this->pubkey = find_pubkey_in_certs(this, keyid);
		if (!this->pubkey)
		{
			DBG1(DBG_CFG, "no public key or certificate found for private key "
				 "(%s '%#B') on '%s':%d", ckaid.ptr ? "CKA_ID" : "keyid",
				 &keyid, module, slot);
			destroy(this);
			return NULL;
		}
	}
	return &this->public;
}
