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

#include "callback_cred.h"

typedef struct private_callback_cred_t private_callback_cred_t;

/**
 * Private data of an callback_cred_t object.
 */
struct private_callback_cred_t {

	/**
	 * Public callback_cred_t interface.
	 */
	callback_cred_t public;

	/**
	 * Callback of this set, for all types, and generic
	 */
	union {
		void *generic;
		callback_cred_shared_cb_t shared;
	} cb;

	/**
	 * Data to pass to callback
	 */
	void *data;
};

/**
 * Shared key enumerator on callbacks
 */
typedef struct {
	/* implements enumerator_t */
	enumerator_t public;
	/* backref to this */
	private_callback_cred_t *this;
	/* type if requested key */
	shared_key_type_t type;
	/* own identity to match */
	identification_t *me;
	/* other identity to match */
	identification_t *other;
	/* current shared key */
	shared_key_t *current;
} shared_enumerator_t;

METHOD(enumerator_t, shared_enumerate, bool,
	shared_enumerator_t *this, va_list args)
{
	shared_key_t **out;
	id_match_t *match_me, *match_other;

	VA_ARGS_VGET(args, out, match_me, match_other);
	DESTROY_IF(this->current);
	this->current = this->this->cb.shared(this->this->data, this->type,
								this->me, this->other, match_me, match_other);
	if (this->current)
	{
		*out = this->current;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, shared_destroy, void,
	shared_enumerator_t *this)
{
	DESTROY_IF(this->current);
	free(this);
}

METHOD(credential_set_t, create_shared_enumerator, enumerator_t*,
	private_callback_cred_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	shared_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _shared_enumerate,
			.destroy = _shared_destroy,
		},
		.this = this,
		.type = type,
		.me = me,
		.other = other,
	);
	return &enumerator->public;
}

METHOD(callback_cred_t, destroy, void,
	private_callback_cred_t *this)
{
	free(this);
}

/**
 * Create a generic callback credential set
 */
static private_callback_cred_t* create_generic(void *cb, void *data)
{
	private_callback_cred_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_shared_enumerator = (void*)return_null,
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = (void*)return_null,
				.create_cdp_enumerator  = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
		.cb.generic = cb,
		.data = data,
	);
	return this;
}

/**
 * See header
 */
callback_cred_t *callback_cred_create_shared(callback_cred_shared_cb_t cb,
											 void *data)
{
	private_callback_cred_t *this = create_generic(cb, data);

	this->public.set.create_shared_enumerator = _create_shared_enumerator;

	return &this->public;
}
