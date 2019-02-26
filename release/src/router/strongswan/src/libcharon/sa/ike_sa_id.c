/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "ike_sa_id.h"

#include <stdio.h>
#include <encoding/payloads/ike_header.h>

typedef struct private_ike_sa_id_t private_ike_sa_id_t;

/**
 * Private data of an ike_sa_id_t object.
 */
struct private_ike_sa_id_t {
	/**
	 * Public interface of ike_sa_id_t.
	 */
	ike_sa_id_t public;

	/**
	 * Major IKE version of IKE_SA.
	 */
	uint8_t ike_version;

	 /**
	  * SPI of initiator.
	  */
	uint64_t initiator_spi;

	 /**
	  * SPI of responder.
	  */
	uint64_t responder_spi;

	/**
	 * Role for specific IKE_SA.
	 */
	bool is_initiator_flag;
};

METHOD(ike_sa_id_t, get_ike_version, uint8_t,
	private_ike_sa_id_t *this)
{
	return this->ike_version;
}

METHOD(ike_sa_id_t, set_responder_spi, void,
	private_ike_sa_id_t *this, uint64_t responder_spi)
{
	this->responder_spi = responder_spi;
}

METHOD(ike_sa_id_t, set_initiator_spi, void,
	private_ike_sa_id_t *this, uint64_t initiator_spi)
{
	this->initiator_spi = initiator_spi;
}

METHOD(ike_sa_id_t, get_initiator_spi, uint64_t,
	private_ike_sa_id_t *this)
{
	return this->initiator_spi;
}

METHOD(ike_sa_id_t, get_responder_spi, uint64_t,
	private_ike_sa_id_t *this)
{
	return this->responder_spi;
}

METHOD(ike_sa_id_t, equals, bool,
	private_ike_sa_id_t *this, private_ike_sa_id_t *other)
{
	if (other == NULL)
	{
		return FALSE;
	}
	return this->ike_version == other->ike_version &&
		   (this->ike_version == IKEV1_MAJOR_VERSION ||
			this->is_initiator_flag == other->is_initiator_flag) &&
		   this->initiator_spi == other->initiator_spi &&
		   this->responder_spi == other->responder_spi;
}

METHOD(ike_sa_id_t, replace_values, void,
	private_ike_sa_id_t *this, private_ike_sa_id_t *other)
{
	this->ike_version = other->ike_version;
	this->initiator_spi = other->initiator_spi;
	this->responder_spi = other->responder_spi;
	this->is_initiator_flag = other->is_initiator_flag;
}

METHOD(ike_sa_id_t, is_initiator, bool,
	private_ike_sa_id_t *this)
{
	return this->is_initiator_flag;
}

METHOD(ike_sa_id_t, switch_initiator, bool,
	private_ike_sa_id_t *this)
{
	this->is_initiator_flag = !this->is_initiator_flag;
	return this->is_initiator_flag;
}

METHOD(ike_sa_id_t, clone_, ike_sa_id_t*,
	private_ike_sa_id_t *this)
{
	return ike_sa_id_create(this->ike_version, this->initiator_spi,
							this->responder_spi, this->is_initiator_flag);
}

METHOD(ike_sa_id_t, destroy, void,
	private_ike_sa_id_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ike_sa_id_t * ike_sa_id_create(uint8_t ike_version, uint64_t initiator_spi,
							   uint64_t responder_spi, bool is_initiator_flag)
{
	private_ike_sa_id_t *this;

	INIT(this,
		.public = {
			.get_ike_version = _get_ike_version,
			.set_responder_spi = _set_responder_spi,
			.set_initiator_spi = _set_initiator_spi,
			.get_responder_spi = _get_responder_spi,
			.get_initiator_spi = _get_initiator_spi,
			.equals = (void*)_equals,
			.replace_values = (void*)_replace_values,
			.is_initiator = _is_initiator,
			.switch_initiator = _switch_initiator,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.ike_version = ike_version,
		.initiator_spi = initiator_spi,
		.responder_spi = responder_spi,
		.is_initiator_flag = is_initiator_flag,
	);

	return &this->public;
}
