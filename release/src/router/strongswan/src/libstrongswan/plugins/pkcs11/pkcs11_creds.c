/*
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

#include "pkcs11_creds.h"
#include "pkcs11_manager.h"

#include <utils/debug.h>
#include <collections/linked_list.h>

typedef struct private_pkcs11_creds_t private_pkcs11_creds_t;

/**
 * Private data of an pkcs11_creds_t object.
 */
struct private_pkcs11_creds_t {

	/**
	 * Public pkcs11_creds_t interface.
	 */
	pkcs11_creds_t public;

	/**
	 * PKCS# library
	 */
	pkcs11_library_t *lib;

	/**
	 * Token slot
	 */
	CK_SLOT_ID slot;

	/**
	 * List of trusted certificates
	 */
	linked_list_t *trusted;

	/**
	 * List of untrusted certificates
	 */
	linked_list_t *untrusted;
};

/**
 * Find certificates, optionally trusted
 */
static void find_certificates(private_pkcs11_creds_t *this,
							  CK_SESSION_HANDLE session)
{
	CK_OBJECT_CLASS class = CKO_CERTIFICATE;
	CK_CERTIFICATE_TYPE type = CKC_X_509;
	CK_BBOOL trusted = TRUE;
	CK_ATTRIBUTE tmpl[] = {
		{CKA_CLASS, &class, sizeof(class)},
		{CKA_CERTIFICATE_TYPE, &type, sizeof(type)},
	};
	CK_OBJECT_HANDLE object;
	CK_ATTRIBUTE attr[] = {
		{CKA_VALUE, NULL, 0},
		{CKA_LABEL, NULL, 0},
		{CKA_TRUSTED, &trusted, sizeof(trusted)}
	};
	enumerator_t *enumerator;
	linked_list_t *raw;
	certificate_t *cert;
	struct {
		chunk_t value;
		chunk_t label;
		bool trusted;
	} *entry;
	int count = countof(attr);

	/* store result in a temporary list, avoid recursive operation */
	raw = linked_list_create();
	/* do not use trusted argument if not supported */
	if (!(this->lib->get_features(this->lib) & PKCS11_TRUSTED_CERTS))
	{
		count--;
	}
	enumerator = this->lib->create_object_enumerator(this->lib,
									session, tmpl, countof(tmpl), attr, count);
	while (enumerator->enumerate(enumerator, &object))
	{
		entry = malloc(sizeof(*entry));
		entry->value = chunk_clone(
							chunk_create(attr[0].pValue, attr[0].ulValueLen));
		entry->label = chunk_clone(
							chunk_create(attr[1].pValue, attr[1].ulValueLen));
		entry->trusted = trusted;
		raw->insert_last(raw, entry);
	}
	enumerator->destroy(enumerator);

	while (raw->remove_first(raw, (void**)&entry) == SUCCESS)
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							BUILD_BLOB_ASN1_DER, entry->value,
							BUILD_END);
		if (cert)
		{
			DBG1(DBG_CFG, "    loaded %strusted cert '%.*s'",
				 entry->trusted ? "" : "un", (int)entry->label.len,
				 entry->label.ptr);
			/* trusted certificates are also returned as untrusted */
			this->untrusted->insert_last(this->untrusted, cert);
			if (entry->trusted)
			{
				this->trusted->insert_last(this->trusted, cert->get_ref(cert));
			}
		}
		else
		{
			DBG1(DBG_CFG, "    loading cert '%.*s' failed",
				(int)entry->label.len, entry->label.ptr);
		}
		free(entry->value.ptr);
		free(entry->label.ptr);
		free(entry);
	}
	raw->destroy(raw);
}

/**
 * Load in the certificates from the token
 */
static bool load_certificates(private_pkcs11_creds_t *this)
{
	CK_SESSION_HANDLE session;
	CK_RV rv;

	rv = this->lib->f->C_OpenSession(this->slot, CKF_SERIAL_SESSION,
									 NULL, NULL, &session);
	if (rv != CKR_OK)
	{
		DBG1(DBG_CFG, "opening session failed: %N", ck_rv_names, rv);
		return FALSE;
	}

	find_certificates(this, session);

	this->lib->f->C_CloseSession(session);
	return TRUE;
}

CALLBACK(certs_filter, bool,
	identification_t *id, enumerator_t *orig, va_list args)
{
	public_key_t *public;
	certificate_t *cert, **out;

	VA_ARGS_VGET(args, out);

	while (orig->enumerate(orig, &cert))
	{
		if (id == NULL || cert->has_subject(cert, id))
		{
			*out = cert;
			return TRUE;
		}
		public = cert->get_public_key(cert);
		if (public)
		{
			if (public->has_fingerprint(public, id->get_encoding(id)))
			{
				public->destroy(public);
				*out = cert;
				return TRUE;
			}
			public->destroy(public);
		}
	}
	return FALSE;
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_pkcs11_creds_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	enumerator_t *inner;

	if (cert != CERT_X509 && cert != CERT_ANY)
	{
		return NULL;
	}
	if (trusted)
	{
		inner = this->trusted->create_enumerator(this->trusted);
	}
	else
	{
		inner = this->untrusted->create_enumerator(this->untrusted);
	}
	return enumerator_create_filter(inner, certs_filter, id, NULL);
}

METHOD(pkcs11_creds_t, get_library, pkcs11_library_t*,
	private_pkcs11_creds_t *this)
{
	return this->lib;
}

METHOD(pkcs11_creds_t, get_slot, CK_SLOT_ID,
	private_pkcs11_creds_t *this)
{
	return this->slot;
}

METHOD(pkcs11_creds_t, destroy, void,
	private_pkcs11_creds_t *this)
{
	this->trusted->destroy_offset(this->trusted,
								offsetof(certificate_t, destroy));
	this->untrusted->destroy_offset(this->untrusted,
								offsetof(certificate_t, destroy));
	free(this);
}

/**
 * See header
 */
pkcs11_creds_t *pkcs11_creds_create(pkcs11_library_t *p11, CK_SLOT_ID slot)
{
	private_pkcs11_creds_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_shared_enumerator = (void*)enumerator_create_empty,
				.create_private_enumerator = (void*)enumerator_create_empty,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_cdp_enumerator  = (void*)enumerator_create_empty,
				.cache_cert = (void*)nop,
			},
			.get_library = _get_library,
			.get_slot = _get_slot,
			.destroy = _destroy,
		},
		.lib = p11,
		.slot = slot,
		.trusted = linked_list_create(),
		.untrusted = linked_list_create(),
	);

	if (!load_certificates(this))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}

/**
 * See header.
 */
certificate_t *pkcs11_creds_load(certificate_type_t type, va_list args)
{
	chunk_t keyid = chunk_empty, data = chunk_empty;
	enumerator_t *enumerator, *certs;
	pkcs11_manager_t *manager;
	pkcs11_library_t *p11;
	certificate_t *cert = NULL;
	CK_SLOT_ID current, slot = -1;
	char *module = NULL;

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

	manager = lib->get(lib, "pkcs11-manager");
	if (!manager)
	{
		return NULL;
	}
	enumerator = manager->create_token_enumerator(manager);
	while (enumerator->enumerate(enumerator, &p11, &current))
	{
		CK_OBJECT_CLASS class = CKO_CERTIFICATE;
		CK_CERTIFICATE_TYPE type = CKC_X_509;
		CK_ATTRIBUTE tmpl[] = {
			{CKA_CLASS, &class, sizeof(class)},
			{CKA_CERTIFICATE_TYPE, &type, sizeof(type)},
			{CKA_ID, keyid.ptr, keyid.len},
		};
		CK_ATTRIBUTE attr[] = {
			{CKA_VALUE, NULL, 0},
		};
		CK_OBJECT_HANDLE object;
		CK_SESSION_HANDLE session;
		CK_RV rv;

		if (slot != -1 && slot != current)
		{
			continue;
		}
		if (module && !streq(module, p11->get_name(p11)))
		{
			continue;
		}

		rv = p11->f->C_OpenSession(current, CKF_SERIAL_SESSION, NULL, NULL,
								   &session);
		if (rv != CKR_OK)
		{
			DBG1(DBG_CFG, "opening PKCS#11 session failed: %N", ck_rv_names, rv);
			continue;
		}
		certs = p11->create_object_enumerator(p11, session,
									tmpl, countof(tmpl), attr, countof(attr));
		if (certs->enumerate(certs, &object))
		{
			data = chunk_clone(chunk_create(attr[0].pValue, attr[0].ulValueLen));
		}
		certs->destroy(certs);
		p11->f->C_CloseSession(session);

		if (data.ptr)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (data.ptr)
	{
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_BLOB_ASN1_DER, data, BUILD_END);
		free(data.ptr);
		if (!cert)
		{
			DBG1(DBG_CFG, "parsing PKCS#11 certificate %#B failed", &keyid);
		}
	}
	else
	{
		DBG1(DBG_CFG, "PKCS#11 certificate %#B not found", &keyid);
	}
	return cert;
}
