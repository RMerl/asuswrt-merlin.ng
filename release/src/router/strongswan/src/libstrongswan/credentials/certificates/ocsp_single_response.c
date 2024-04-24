/*
 * Copyright (C) 2023 Andreas Steffen, strongSec GmbH
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

#include "ocsp_single_response.h"

typedef struct private_ocsp_single_response_t private_ocsp_single_response_t;

/**
 * Private data of an ocsp_single_response object.
 */
struct private_ocsp_single_response_t {

	/**
	 * Public interface for this ocsp_single_response object.
	 */
	ocsp_single_response_t public;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

METHOD(ocsp_single_response_t, get_ref, ocsp_single_response_t*,
	private_ocsp_single_response_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(ocsp_single_response_t, destroy, void,
	private_ocsp_single_response_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->public.issuerNameHash.ptr);
		free(this->public.issuerKeyHash.ptr);
		free(this->public.serialNumber.ptr);
		free(this);
	}
}

/**
 * See header.
 */
ocsp_single_response_t *ocsp_single_response_create()
{
	private_ocsp_single_response_t *this;

	INIT(this,
		.public = {
			.hashAlgorithm = HASH_UNKNOWN,
			.status = VALIDATION_FAILED,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.ref = 1,
	);

	return &this->public;
}
