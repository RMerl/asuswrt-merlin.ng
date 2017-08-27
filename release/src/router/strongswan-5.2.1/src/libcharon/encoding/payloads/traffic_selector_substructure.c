/*
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2005 Jan Hutter
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

#include "traffic_selector_substructure.h"

#include <encoding/payloads/encodings.h>
#include <collections/linked_list.h>

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
	u_int8_t ts_type;

	/**
	 * IP Protocol ID.
	 */
	u_int8_t ip_protocol_id;

	/**
	 * Length of this payload.
	 */
	u_int16_t payload_length;

	/**
	 * Start port number.
	 */
	u_int16_t start_port;

	/**
	 * End port number.
	 */
	u_int16_t end_port;

	/**
	 * Starting address.
	 */
	chunk_t starting_address;

	/**
	 * Ending address.
	 */
	chunk_t ending_address;
};

/**
 * Encoding rules to parse or generate a TS payload
 *
 * The defined offsets are the positions in a object of type
 * private_traffic_selector_substructure_t.
 */
static encoding_rule_t encodings[] = {
	/* 1 Byte next ts type*/
	{ TS_TYPE,		offsetof(private_traffic_selector_substructure_t, ts_type) 			},
	/* 1 Byte IP protocol id*/
	{ U_INT_8,		offsetof(private_traffic_selector_substructure_t, ip_protocol_id) 	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,offsetof(private_traffic_selector_substructure_t, payload_length)	},
	/* 2 Byte start port*/
	{ U_INT_16,		offsetof(private_traffic_selector_substructure_t, start_port)		},
	/* 2 Byte end port*/
	{ U_INT_16,		offsetof(private_traffic_selector_substructure_t, end_port)			},
	/* starting address is either 4 or 16 byte */
	{ ADDRESS,		offsetof(private_traffic_selector_substructure_t, starting_address)	},
	/* ending address is either 4 or 16 byte */
	{ ADDRESS,		offsetof(private_traffic_selector_substructure_t, ending_address)	}
};

/*
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
*/

METHOD(payload_t, verify, status_t,
	private_traffic_selector_substructure_t *this)
{
	if (this->start_port > this->end_port)
	{
		/* OPAQUE ports are the only exception */
		if (this->start_port != 0xffff && this->end_port != 0)
		{
			return FAILED;
		}
	}
	switch (this->ts_type)
	{
		case TS_IPV4_ADDR_RANGE:
		{
			if ((this->starting_address.len != 4) ||
				(this->ending_address.len != 4))
			{
				/* ipv4 address must be 4 bytes long */
				return FAILED;
			}
			break;
		}
		case TS_IPV6_ADDR_RANGE:
		{
			if ((this->starting_address.len != 16) ||
				(this->ending_address.len != 16))
			{
				/* ipv6 address must be 16 bytes long */
				return FAILED;
			}
			break;
		}
		default:
		{
			/* not supported ts type */
			return FAILED;
		}
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
	return 8;
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
	return traffic_selector_create_from_bytes(
									this->ip_protocol_id, this->ts_type,
									this->starting_address, this->start_port,
									this->ending_address, this->end_port);
}

METHOD2(payload_t, traffic_selector_substructure_t, destroy, void,
	private_traffic_selector_substructure_t *this)
{
	free(this->starting_address.ptr);
	free(this->ending_address.ptr);
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

	this = (private_traffic_selector_substructure_t*)traffic_selector_substructure_create();
	this->ts_type = ts->get_type(ts);
	this->ip_protocol_id = ts->get_protocol(ts);
	this->start_port = ts->get_from_port(ts);
	this->end_port = ts->get_to_port(ts);
	this->starting_address = chunk_clone(ts->get_from_address(ts));
	this->ending_address = chunk_clone(ts->get_to_address(ts));
	this->payload_length = get_header_length(this) +
						this->ending_address.len + this->starting_address.len;

	return &this->public;
}
