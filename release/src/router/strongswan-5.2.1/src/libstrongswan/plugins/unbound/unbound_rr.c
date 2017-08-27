/*
 * Copyright (C) 2012 Reto Guadagnini
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

#include <resolver/rr.h>

#include <library.h>
#include <utils/debug.h>

#include <stdlib.h>
#include <string.h>

#include "unbound_rr.h"

typedef struct private_unbound_rr_t private_unbound_rr_t;

/**
 * private data of an unbound_rr_t object.
 */
struct private_unbound_rr_t {

	/**
	 * Public data
	 */
	unbound_rr_t public;

	/**
	 * Owner name
	 */
	char* name;

	/**
	 * Type
	 */
	rr_type_t type;

	/**
	 * Class
	 */
	rr_class_t class;

	/**
	 * TTL
	 */
	uint32_t ttl;

	/**
	 * Size of the rdata field in octets
	 */
	uint16_t size;

	/**
	 * RDATA field (array of bytes in network order)
	 */
	u_char *rdata;
};

METHOD(rr_t, get_name, char *,
	private_unbound_rr_t *this)
{
	return this->name;
}

METHOD(rr_t, get_type, rr_type_t,
	private_unbound_rr_t *this)
{
	return this->type;
}

METHOD(rr_t, get_class, rr_class_t,
	private_unbound_rr_t *this)
{
	return this->class;
}

METHOD(rr_t, get_ttl, uint32_t,
	private_unbound_rr_t *this)
{
	return this->ttl;
}

METHOD(rr_t, get_rdata, chunk_t,
	private_unbound_rr_t *this)
{
	return chunk_create(this->rdata, this->size);
}

METHOD(rr_t, destroy, void,
	private_unbound_rr_t *this)
{
	free(this->name);
	free(this->rdata);
	free(this);
}

/*
 * Described in header.
 */
unbound_rr_t *unbound_rr_create_frm_ldns_rr(ldns_rr *rr)
{
	private_unbound_rr_t *this;
	ldns_status status;
	ldns_buffer *buf;
	int i;

	INIT(this,
		.public = {
			.interface = {
				.get_name = _get_name,
				.get_type = _get_type,
				.get_class = _get_class,
				.get_ttl = _get_ttl,
				.get_rdata = _get_rdata,
				.destroy = _destroy,
			},
		},
	);

	this->name = ldns_rdf2str(ldns_rr_owner(rr));
	if (!this->name)
	{
		DBG1(DBG_LIB, "failed to parse the owner name of a DNS RR");
		_destroy(this);
		return NULL;
	}

	this->type = (rr_type_t)ldns_rr_get_type(rr);
	this->class = (rr_class_t)ldns_rr_get_class(rr);
	this->ttl = ldns_rr_ttl(rr);
	for(i = 0; i < ldns_rr_rd_count(rr); i++)
	{
		this->size += ldns_rdf_size(ldns_rr_rdf(rr, i));
	}

	/**
	 * The ldns library splits the RDATA field of a RR in various rdf.
	 * Here we reassemble these rdf to get the RDATA field of the RR.
	 */
	buf = ldns_buffer_new(LDNS_MIN_BUFLEN);
	/* The buffer will be resized automatically by ldns_rr_rdata2buffer_wire() */
	status = ldns_rr_rdata2buffer_wire(buf, rr);

	if (status != LDNS_STATUS_OK)
	{
		DBG1(DBG_LIB, "failed to get the RDATA field of a DNS RR");
		_destroy(this);
		return NULL;
	}

	this->rdata = ldns_buffer_export(buf);

	return &this->public;
}
