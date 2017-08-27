/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "ietf_attr_port_filter.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <collections/linked_list.h>
#include <utils/debug.h>


typedef struct private_ietf_attr_port_filter_t private_ietf_attr_port_filter_t;
typedef struct port_entry_t port_entry_t;

/**
 * Port Filter entry
 */
struct port_entry_t {
	bool      blocked;
	u_int8_t  protocol;
	u_int16_t port;
};

/**
 * PA-TNC Port Filter Type  (see section 4.2.6 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Reserved  |B|    Protocol   |         Port Number           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Reserved  |B|    Protocol   |         Port Number           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PORT_FILTER_ENTRY_SIZE	4

/**
 * Private data of an ietf_attr_port_filter_t object.
 */
struct private_ietf_attr_port_filter_t {

	/**
	 * Public members of ietf_attr_port_filter_t
	 */
	ietf_attr_port_filter_t public;

	/**
	 * Vendor-specific attribute type
	 */
	pen_type_t type;

	/**
	 * Length of attribute value
	 */
	size_t length;

	/**
	 * Attribute value or segment
	 */
	chunk_t value;

	/**
	 * Noskip flag
	 */
	bool noskip_flag;

	/**
	 * List of Port Filter entries
	 */
	linked_list_t *ports;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_port_filter_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_port_filter_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_port_filter_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_port_filter_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_port_filter_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	port_entry_t *entry;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(this->ports->get_count(this->ports) *
							   PORT_FILTER_ENTRY_SIZE);

	enumerator = this->ports->create_enumerator(this->ports);
	while (enumerator->enumerate(enumerator, &entry))
	{
		writer->write_uint8 (writer, entry->blocked ? 0x01 : 0x00);
		writer->write_uint8 (writer, entry->protocol);
		writer->write_uint16(writer, entry->port);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_port_filter_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	port_entry_t *entry;
	u_int8_t blocked;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len % PORT_FILTER_ENTRY_SIZE)
	{
		DBG1(DBG_TNC, "ietf port filter attribute value is not a multiple of %d",
			 PORT_FILTER_ENTRY_SIZE);
		return FAILED;
	}
	reader = bio_reader_create(this->value);

	while (reader->remaining(reader))
	{
		entry = malloc_thing(port_entry_t);
		reader->read_uint8 (reader, &blocked);
		entry->blocked = blocked & 0x01;
		reader->read_uint8 (reader, &entry->protocol);
		reader->read_uint16(reader, &entry->port);
		this->ports->insert_last(this->ports, entry);
	}
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_port_filter_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_port_filter_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_port_filter_t *this)
{
	if (ref_put(&this->ref))
	{
		this->ports->destroy_function(this->ports, free);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ietf_attr_port_filter_t, add_port, void,
	private_ietf_attr_port_filter_t *this, bool blocked, u_int8_t protocol,
	u_int16_t port)
{
	port_entry_t *entry;

	entry = malloc_thing(port_entry_t);
	entry->blocked = blocked;
	entry->protocol = protocol;
	entry->port = port;
	this->ports->insert_last(this->ports, entry);
}

/**
 * Enumerate port filter entries
 */
static bool port_filter(void *null, port_entry_t **entry,
						bool *blocked, void *i2, u_int8_t *protocol, void *i3,
						u_int16_t *port)
{
	*blocked = (*entry)->blocked;
	*protocol = (*entry)->protocol;
	*port = (*entry)->port;
	return TRUE;
}

METHOD(ietf_attr_port_filter_t, create_port_enumerator, enumerator_t*,
	private_ietf_attr_port_filter_t *this)
{
	return enumerator_create_filter(this->ports->create_enumerator(this->ports),
					(void*)port_filter, NULL, NULL);
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_port_filter_create(void)
{
	private_ietf_attr_port_filter_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.add_port = _add_port,
			.create_port_enumerator = _create_port_enumerator,
		},
		.type = { PEN_IETF, IETF_ATTR_PORT_FILTER },
		.ports = linked_list_create(),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_port_filter_create_from_data(size_t length,
													  chunk_t data)
{
	private_ietf_attr_port_filter_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.add_port = _add_port,
			.create_port_enumerator = _create_port_enumerator,
		},
		.type = {PEN_IETF, IETF_ATTR_PORT_FILTER },
		.length = length,
		.value = chunk_clone(data),
		.ports = linked_list_create(),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


