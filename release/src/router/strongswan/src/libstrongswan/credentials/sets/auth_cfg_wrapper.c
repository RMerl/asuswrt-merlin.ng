/*
 * Copyright (C) 2008-2009 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

#include <library.h>
#include <utils/debug.h>

#include "auth_cfg_wrapper.h"

typedef struct private_auth_cfg_wrapper_t private_auth_cfg_wrapper_t;

/**
 * private data of auth_cfg_wrapper
 */
struct private_auth_cfg_wrapper_t {

	/**
	 * public functions
	 */
	auth_cfg_wrapper_t public;

	/**
	 * wrapped auth info
	 */
	auth_cfg_t *auth;
};

/**
 * enumerator for auth_cfg_wrapper_t.create_cert_enumerator()
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** inner enumerator from auth_cfg */
	enumerator_t *inner;
	/** wrapped auth round */
	auth_cfg_t *auth;
	/** enumerated cert type */
	certificate_type_t cert;
	/** enumerated key type */
	key_type_t key;
	/** enumerated id */
	identification_t *id;
} wrapper_enumerator_t;

/**
 * Tries to fetch a certificate that was supplied as "Hash and URL"
 * (replaces rule type and value in place).
 */
static bool fetch_cert(wrapper_enumerator_t *enumerator,
					   auth_rule_t *rule, void **value)
{
	char *url = (char*)*value;
	if (!url)
	{
		/* fetching the certificate previously failed */
		return FALSE;
	}

	chunk_t data;
	certificate_t *cert;

	DBG1(DBG_CFG, "  fetching certificate from '%s' ...", url);
	if (lib->fetcher->fetch(lib->fetcher, url, &data, FETCH_END) != SUCCESS)
	{
		DBG1(DBG_CFG, "  fetching certificate failed");
		/* we set the item to NULL, so we can skip it */
		enumerator->auth->replace(enumerator->auth, enumerator->inner,
								  *rule, NULL);
		return FALSE;
	}

	cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
							  BUILD_BLOB_ASN1_DER, data, BUILD_END);
	free(data.ptr);

	if (!cert)
	{
		DBG1(DBG_CFG, "  parsing fetched certificate failed");
		/* we set the item to NULL, so we can skip it */
		enumerator->auth->replace(enumerator->auth, enumerator->inner,
								  *rule, NULL);
		return FALSE;
	}

	DBG1(DBG_CFG, "  fetched certificate \"%Y\"", cert->get_subject(cert));
	lib->credmgr->cache_cert(lib->credmgr, cert);

	if (*rule == AUTH_HELPER_IM_HASH_URL)
	{
		*rule = AUTH_HELPER_IM_CERT;
	}
	else
	{
		*rule = AUTH_HELPER_SUBJECT_CERT;
	}
	*value = cert;
	enumerator->auth->replace(enumerator->auth, enumerator->inner,
							  *rule, cert);
	return TRUE;
}

METHOD(enumerator_t, enumerate, bool,
	wrapper_enumerator_t *this, va_list args)
{
	auth_rule_t rule;
	certificate_t *current, **cert;
	public_key_t *public;

	VA_ARGS_VGET(args, cert);

	while (this->inner->enumerate(this->inner, &rule, &current))
	{
		if (rule == AUTH_HELPER_IM_HASH_URL ||
			rule == AUTH_HELPER_SUBJECT_HASH_URL)
		{	/* on-demand fetching of hash and url certificates */
			if (!fetch_cert(this, &rule, (void**)&current))
			{
				continue;
			}
		}
		else if (rule != AUTH_HELPER_SUBJECT_CERT &&
				 rule != AUTH_HELPER_IM_CERT &&
				 rule != AUTH_HELPER_REVOCATION_CERT &&
				 rule != AUTH_HELPER_AC_CERT)
		{	/* handle only HELPER certificates */
			continue;
		}
		if (this->cert != CERT_ANY && this->cert != current->get_type(current))
		{	/* CERT type requested, but does not match */
			continue;
		}
		public = current->get_public_key(current);
		if (this->key != KEY_ANY && !public)
		{	/* key type requested, but no public key */
			DESTROY_IF(public);
			continue;
		}
		if (this->key != KEY_ANY && public && this->key != public->get_type(public))
		{	/* key type requested, but public key has another type */
			DESTROY_IF(public);
			continue;
		}
		DESTROY_IF(public);
		if (this->id && !current->has_subject(current, this->id))
		{	/* subject requested, but does not match */
			continue;
		}
		*cert = current;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, wrapper_enumerator_destroy, void,
	wrapper_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(credential_set_t, create_enumerator, enumerator_t*,
	private_auth_cfg_wrapper_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	wrapper_enumerator_t *enumerator;

	if (trusted)
	{
		return NULL;
	}
	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate,
			.destroy = _wrapper_enumerator_destroy,
		},
		.auth = this->auth,
		.cert = cert,
		.key = key,
		.id = id,
		.inner = this->auth->create_enumerator(this->auth),
	);
	return &enumerator->public;
}

METHOD(auth_cfg_wrapper_t, destroy, void,
	private_auth_cfg_wrapper_t *this)
{
	free(this);
}

/*
 * see header file
 */
auth_cfg_wrapper_t *auth_cfg_wrapper_create(auth_cfg_t *auth)
{
	private_auth_cfg_wrapper_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_cert_enumerator = _create_enumerator,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
		.auth = auth,
	);

	return &this->public;
}
