/*
 * Copyright (C) 2022 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
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

#include "traffic_selector_substructure.h"

#include <encoding/payloads/encodings.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

typedef struct private_traffic_selector_substructure_t private_traffic_selector_substructure_t;

/**
 * Private data of an traffic_selector_substructure_t object.
 */
struct private_traffic_selector_substructure_t {

	/**
	 * Public traffic_selector_substructure_t interface.
	 */
	traffic_selector_substructure_t public;

	/**
	 * Type of traffic selector.
	 */
	uint8_t ts_type;

	/**
	 * IP Protocol ID.
	 */
	uint8_t ip_protocol_id;

	/**
	 * Length of this payload.
	 */
	uint16_t payload_length;

	/**
	 * Port/address range or security label.
	 */
	chunk_t ts_data;
};

/**
 * Encoding rules to parse or generate a TS payload.
 *
 * Due to the generic nature of security labels, the actual structure of regular
 * TS is not parsed with these rules.
 *
 * The defined offsets are the positions in a object of type
 * private_traffic_selector_substructure_t.
 */
static encoding_rule_t encodings[] = {
	/* 1 Byte next ts type*/
	{ U_INT_8,		offsetof(private_traffic_selector_substructure_t, ts_type) 			},
	/* 1 Byte IP protocol id*/
	{ U_INT_8,		offsetof(private_traffic_selector_substructure_t, ip_protocol_id) 	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,offsetof(private_traffic_selector_substructure_t, payload_length)	},
	/* traffic selector data, length is defined in PAYLOAD_LENGTH */
	{ CHUNK_DATA,	offsetof(private_traffic_selector_substructure_t, ts_data)			},
};

/* Regular traffic selectors for address ranges:
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !   TS Type     !IP Protocol ID*|       Selector Length         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Start Port*         |           End Port*           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                         Starting Address*                     ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                         Ending Address*                       ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

 * Security labels:
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +---------------+---------------+-------------------------------+
      |   TS Type     |    Reserved   |       Selector Length         |
      +---------------+---------------+-------------------------------+
      |                                                               |
      ~                         Security Label*                       ~
      |                                                               |
      +---------------------------------------------------------------+
*/

/**
 * Parse the data of a regular address range traffic selector.
 */
static bool parse_ts_data(private_traffic_selector_substructure_t *this,
						  uint16_t *start_port, uint16_t *end_port,
						  chunk_t *start_addr, chunk_t *end_addr)
{
	bio_reader_t *reader;
	int addr_len;

	switch (this->ts_type)
	{
		case TS_IPV4_ADDR_RANGE:
			addr_len = 4;
			break;
		case TS_IPV6_ADDR_RANGE:
			addr_len = 16;
			break;
		default:
			return FALSE;
	}

	reader = bio_reader_create(this->ts_data);
	if (!reader->read_uint16(reader, start_port) ||
		!reader->read_uint16(reader, end_port) ||
		!reader->read_data(reader, addr_len, start_addr) ||
		!reader->read_data(reader, addr_len, end_addr) ||
		reader->remaining(reader) > 0)
	{
		reader->destroy(reader);
		return FALSE;
	}
	reader->destroy(reader);
	return TRUE;
}

METHOD(payload_t, verify, status_t,
	private_traffic_selector_substructure_t *this)
{
	switch (this->ts_type)
	{
		case TS_IPV4_ADDR_RANGE:
		case TS_IPV6_ADDR_RANGE:
		{
			uint16_t start_port, end_port;
			chunk_t start_addr, end_addr;

			if (!parse_ts_data(this, &start_port, &end_port, &start_addr,
							   &end_addr))
			{
				return FAILED;
			}
			if (start_port > end_port)
			{
				/* OPAQUE ports are the only exception */
				if (start_port != 0xffff && end_port != 0)
				{
					return FAILED;
				}
			}
			break;
		}
		case TS_SECLABEL:
			if (!this->ts_data.len)
			{
				return FAILED;
			}
			break;
		default:
			/* unsupported TS type, just ignored later */
			break;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_traffic_selector_substructure_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_traffic_selector_substructure_t *this)
{
	return 4;
}

METHOD(payload_t, get_type, payload_type_t,
	private_traffic_selector_substructure_t *this)
{
	return PLV2_TRAFFIC_SELECTOR_SUBSTRUCTURE;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_traffic_selector_substructure_t *this)
{
	return PL_NONE;
}

METHOD(payload_t, set_next_type, void,
	private_traffic_selector_substructure_t *this,payload_type_t type)
{
}

METHOD(payload_t, get_length, size_t,
	private_traffic_selector_substructure_t *this)
{
	return this->payload_length;
}

METHOD(traffic_selector_substructure_t, get_traffic_selector, traffic_selector_t*,
	private_traffic_selector_substructure_t *this)
{
	uint16_t start_port, end_port;
	chunk_t start_addr, end_addr;

	if (!parse_ts_data(this, &start_port, &end_port, &start_addr, &end_addr))
	{
		return NULL;
	}
	return traffic_selector_create_from_bytes(
									this->ip_protocol_id, this->ts_type,
									start_addr, start_port, end_addr, end_port);
}

METHOD(traffic_selector_substructure_t, get_sec_label, sec_label_t*,
	private_traffic_selector_substructure_t *this)
{
	if (this->ts_type != TS_SECLABEL)
	{
		return NULL;
	}
	return sec_label_from_encoding(this->ts_data);
}

METHOD2(payload_t, traffic_selector_substructure_t, destroy, void,
	private_traffic_selector_substructure_t *this)
{
	free(this->ts_data.ptr);
	free(this);
}

/*
 * Described in header
 */
traffic_selector_substructure_t *traffic_selector_substructure_create()
{
	private_traffic_selector_substructure_t *this;

	INIT(this,
		.public = {
			.payload_interface = {
				.verify = _verify,
				.get_encoding_rules = _get_encoding_rules,
				.get_header_length = _get_header_length,
				.get_length = _get_length,
				.get_next_type = _get_next_type,
				.set_next_type = _set_next_type,
				.get_type = _get_type,
				.destroy = _destroy,
			},
			.get_traffic_selector = _get_traffic_selector,
			.get_sec_label = _get_sec_label,
			.destroy = _destroy,
		},
		.payload_length = get_header_length(this),
		/* must be set to be valid */
		.ts_type = TS_IPV4_ADDR_RANGE,
	);
	return &this->public;
}

/*
 * Described in header
 */
traffic_selector_substructure_t *traffic_selector_substructure_create_from_traffic_selector(
													traffic_selector_t *ts)
{
	private_traffic_selector_substructure_t *this;
	bio_writer_t *writer;

	this = (private_traffic_selector_substructure_t*)traffic_selector_substructure_create();
	this->ts_type = ts->get_type(ts);
	this->ip_protocol_id = ts->get_protocol(ts);

	writer = bio_writer_create(this->ts_type == TS_IPV4_ADDR_RANGE ? 12 : 36);
	writer->write_uint16(writer, ts->get_from_port(ts));
	writer->write_uint16(writer, ts->get_to_port(ts));
	writer->write_data(writer, ts->get_from_address(ts));
	writer->write_data(writer, ts->get_to_address(ts));
	this->ts_data = writer->extract_buf(writer);
	this->payload_length += this->ts_data.len;
	writer->destroy(writer);
	return &this->public;
}

/*
 * Described in header
 */
traffic_selector_substructure_t *traffic_selector_substructure_create_from_sec_label(
													sec_label_t *label)
{
	private_traffic_selector_substructure_t *this;

	this = (private_traffic_selector_substructure_t*)traffic_selector_substructure_create();
	this->ts_type = TS_SECLABEL;
	this->ts_data = chunk_clone(label->get_encoding(label));
	this->payload_length += this->ts_data.len;
	return &this->public;
}
