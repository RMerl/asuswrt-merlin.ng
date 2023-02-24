/*
 * Copyright (C) 2020 Tobias Brunner
 * Copyright (C) 2015 Andreas Steffen
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

/**
 * @defgroup vici_authority vici_authority
 * @{ @ingroup vici
 */

#ifndef VICI_AUTHORITY_H_
#define VICI_AUTHORITY_H_

#include "vici_dispatcher.h"

typedef struct vici_authority_t vici_authority_t;

/**
 * In-memory certification authority backend, managed by VICI.
 */
struct vici_authority_t {

	/**
	 * Implements credential_set_t
	 */
	credential_set_t set;

	/**
	 * Add a CA certificate and return a reference if it is already stored,
	 * otherwise returns the same certificate.
	 *
	 * @param cert		certificate to check
	 * @return			reference to stored CA certificate, or original
	 */
	certificate_t *(*add_ca_cert)(vici_authority_t *this, certificate_t *cert);

	/**
	 * Remove CA certificates added via add_ca_cert().
	 */
	void (*clear_ca_certs)(vici_authority_t *this);

	/**
	 * Destroy a vici_authority_t.
	 */
	void (*destroy)(vici_authority_t *this);
};

/**
 * Create a vici_authority instance.
 *
 * @param dispatcher		dispatcher to receive requests from
 * @return					authority backend
 */
vici_authority_t *vici_authority_create(vici_dispatcher_t *dispatcher);

#endif /** VICI_AUTHORITY_H_ @}*/
