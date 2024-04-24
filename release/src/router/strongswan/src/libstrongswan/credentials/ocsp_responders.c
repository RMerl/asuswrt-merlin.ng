/*
 * Copyright (C) 2023 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "ocsp_responders.h"

#include <collections/linked_list.h>
#include <threading/rwlock.h>

typedef struct private_ocsp_responders_t private_ocsp_responders_t;

/**
 * Private data
 */
struct private_ocsp_responders_t {

	/**
	 * Public interface
	 */
	ocsp_responders_t public;

	/**
	 * List of registered OCSP responders
	 */
	linked_list_t *responders;

	/**
	 * Lock to access responder list
	 */
	rwlock_t *lock;
};

METHOD(ocsp_responders_t, get_status, cert_validation_t,
	private_ocsp_responders_t *this, certificate_t *cacert,
	chunk_t serial_number, time_t *revocation_time,
	crl_reason_t *revocation_reason)
{
	enumerator_t *enumerator;
	ocsp_responder_t *current;
	cert_validation_t validation = VALIDATION_SKIPPED;

	this->lock->read_lock(this->lock);
	enumerator = this->responders->create_enumerator(this->responders);
	while (enumerator->enumerate(enumerator, &current))
	{
		validation = current->get_status(current, cacert, serial_number,
										 revocation_time, revocation_reason);
		if (validation != VALIDATION_SKIPPED &&
			validation != VALIDATION_FAILED)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	if (validation == VALIDATION_SKIPPED)
	{
		validation = VALIDATION_FAILED;
	}
	return validation;
}

METHOD(ocsp_responders_t, add_responder, void,
	private_ocsp_responders_t *this, ocsp_responder_t *responder)
{
	this->lock->write_lock(this->lock);
	this->responders->insert_last(this->responders, responder);
	this->lock->unlock(this->lock);
}

METHOD(ocsp_responders_t, remove_responder, void,
	private_ocsp_responders_t *this, ocsp_responder_t *responder)
{
	this->lock->write_lock(this->lock);
	this->responders->remove(this->responders, responder, NULL);
	this->lock->unlock(this->lock);
}

METHOD(ocsp_responders_t, destroy, void,
	private_ocsp_responders_t *this)
{
	this->responders->destroy(this->responders);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * Described in header
 */
ocsp_responders_t *ocsp_responders_create()
{
	private_ocsp_responders_t *this;

	INIT(this,
		.public = {
			.get_status = _get_status,
			.add_responder = _add_responder,
			.remove_responder = _remove_responder,
			.destroy = _destroy,
		},
		.responders = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
