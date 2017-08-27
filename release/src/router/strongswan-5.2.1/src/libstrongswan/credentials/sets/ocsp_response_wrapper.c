/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "ocsp_response_wrapper.h"

typedef struct private_ocsp_response_wrapper_t private_ocsp_response_wrapper_t;

/**
 * private data of ocsp_response_wrapper
 */
struct private_ocsp_response_wrapper_t {

	/**
	 * public functions
	 */
	ocsp_response_wrapper_t public;

	/**
	 * wrapped OCSP response
	 */
	ocsp_response_t *response;
};

/**
 * enumerator for ocsp_response_wrapper_t.create_cert_enumerator()
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** enumerator over ocsp response */
	enumerator_t *inner;
	/** type of cert */
	certificate_type_t cert;
	/** type of key */
	key_type_t key;
	/** filtering identity */
	identification_t *id;
} wrapper_enumerator_t;

/**
 * enumerate function wrapper_enumerator_t
 */
static bool enumerate(wrapper_enumerator_t *this, certificate_t **cert)
{
	certificate_t *current;
	public_key_t *public;

	while (this->inner->enumerate(this->inner, &current))
	{
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

/**
 * destroy function for wrapper_enumerator_t
 */
static void enumerator_destroy(wrapper_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(credential_set_t, create_enumerator, enumerator_t*,
	private_ocsp_response_wrapper_t *this,certificate_type_t cert,
	key_type_t key, identification_t *id, bool trusted)
{
	wrapper_enumerator_t *enumerator;

	if (trusted)
	{
		return NULL;
	}

	enumerator = malloc_thing(wrapper_enumerator_t);
	enumerator->cert = cert;
	enumerator->key = key;
	enumerator->id = id;
	enumerator->inner = this->response->create_cert_enumerator(this->response);
	enumerator->public.enumerate = (void*)enumerate;
	enumerator->public.destroy = (void*)enumerator_destroy;
	return &enumerator->public;
}

METHOD(ocsp_response_wrapper_t, destroy, void,
	private_ocsp_response_wrapper_t *this)
{
	free(this);
}

/*
 * see header file
 */
ocsp_response_wrapper_t *ocsp_response_wrapper_create(ocsp_response_t *response)
{
	private_ocsp_response_wrapper_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_cert_enumerator = _create_enumerator,
				.create_private_enumerator = (void*)return_null,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
		.response = response,
	);

	return &this->public;
}
