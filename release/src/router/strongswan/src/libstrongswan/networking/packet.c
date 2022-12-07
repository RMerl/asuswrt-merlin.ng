/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "packet.h"

#include <metadata/metadata_set.h>

typedef struct private_packet_t private_packet_t;

/**
 * Private data of an packet_t object.
 */
struct private_packet_t {

	/**
	 * Public part of a packet_t object.
	 */
	packet_t public;

	/**
	 * source address
	 */
	host_t *source;

	/**
	 * destination address
	 */
	host_t *destination;

	/**
	 * DSCP value on packet
	 */
	uint8_t dscp;

	 /**
	  * message data
	  */
	chunk_t data;

	/**
	 * actual chunk returned from get_data, adjusted when skip_bytes is called
	 */
	chunk_t adjusted_data;

	/**
	 * Set of metadata objects, if any
	 */
	metadata_set_t *metadata;
};

METHOD(packet_t, set_source, void,
	private_packet_t *this, host_t *source)
{
	DESTROY_IF(this->source);
	this->source = source;
}

METHOD(packet_t, set_destination, void,
	private_packet_t *this, host_t *destination)
{
	DESTROY_IF(this->destination);
	this->destination = destination;
}

METHOD(packet_t, get_source, host_t*,
	private_packet_t *this)
{
	return this->source;
}

METHOD(packet_t, get_destination, host_t*,
	private_packet_t *this)
{
	return this->destination;
}

METHOD(packet_t, get_data, chunk_t,
	private_packet_t *this)
{
	return this->adjusted_data;
}

METHOD(packet_t, set_data, void,
	private_packet_t *this, chunk_t data)
{
	free(this->data.ptr);
	this->adjusted_data = this->data = data;
}

METHOD(packet_t, get_dscp, uint8_t,
	private_packet_t *this)
{
	return this->dscp;
}
METHOD(packet_t, set_dscp, void,
	private_packet_t *this, uint8_t value)
{
	this->dscp = value;
}

METHOD(packet_t, skip_bytes, void,
	private_packet_t *this, size_t bytes)
{
	this->adjusted_data = chunk_skip(this->adjusted_data, bytes);
}

METHOD(packet_t, get_metadata, metadata_t*,
	private_packet_t *this, const char *key)
{
	return metadata_set_get(this->metadata, key);
}

METHOD(packet_t, set_metadata, void,
	private_packet_t *this, const char *key, metadata_t *data)
{
	if (!this->metadata && data)
	{
		this->metadata = metadata_set_create();
	}
	metadata_set_put(this->metadata, key, data);
}

METHOD(packet_t, destroy, void,
	private_packet_t *this)
{
	DESTROY_IF(this->source);
	DESTROY_IF(this->destination);
	metadata_set_destroy(this->metadata);
	free(this->data.ptr);
	free(this);
}

/**
 * Clone the packet with or without data
 */
static packet_t *clone_packet(private_packet_t *this, bool skip_data)
{
	private_packet_t *other;

	other = (private_packet_t*)packet_create();
	if (this->destination)
	{
		set_destination(other, this->destination->clone(this->destination));
	}
	if (this->source)
	{
		set_source(other, this->source->clone(this->source));
	}
	other->metadata = metadata_set_clone(this->metadata);
	set_dscp(other, this->dscp);

	if (!skip_data && this->data.ptr)
	{
		set_data(other, chunk_clone(this->adjusted_data));
	}
	return &other->public;
}

METHOD(packet_t, clone_, packet_t*,
	private_packet_t *this)
{
	return clone_packet(this, FALSE);
}

/*
 * Described in header
 */
packet_t *packet_clone_no_data(packet_t *packet)
{
	return clone_packet((private_packet_t*)packet, TRUE);
}

/**
 * Described in header.
 */
packet_t *packet_create_from_data(host_t *src, host_t *dst, chunk_t data)
{
	private_packet_t *this;

	INIT(this,
		.public = {
			.set_data = _set_data,
			.get_data = _get_data,
			.set_source = _set_source,
			.get_source = _get_source,
			.set_destination = _set_destination,
			.get_destination = _get_destination,
			.get_dscp = _get_dscp,
			.set_dscp = _set_dscp,
			.get_metadata = _get_metadata,
			.set_metadata = _set_metadata,
			.skip_bytes = _skip_bytes,
			.clone = _clone_,
			.destroy = _destroy,
		},
		.source = src,
		.destination = dst,
		.adjusted_data = data,
		.data = data,
	);

	return &this->public;
}

/*
 * Described in header.
 */
packet_t *packet_create()
{
	return packet_create_from_data(NULL, NULL, chunk_empty);
}
