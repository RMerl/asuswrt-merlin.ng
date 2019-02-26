/*
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include <stddef.h>

#include "ts_payload.h"

#include <encoding/payloads/encodings.h>
#include <collections/linked_list.h>

typedef struct private_ts_payload_t private_ts_payload_t;

/**
 * Private data of an ts_payload_t object.
 */
struct private_ts_payload_t {

	/**
	 * Public ts_payload_t interface.
	 */
	ts_payload_t public;

	/**
	 * TRUE if this TS payload is of type TSi, FALSE for TSr.
	 */
	bool is_initiator;

	/**
	 * Next payload type.
	 */
	uint8_t  next_payload;

	/**
	 * Critical flag.
	 */
	bool critical;

	/**
	 * reserved bits
	 */
	bool reserved_bit[7];

	/**
	 * reserved bytes
	 */
	bool reserved_byte[3];

	/**
	 * Length of this payload.
	 */
	uint16_t payload_length;

	/**
	 * Number of traffic selectors
	 */
	uint8_t ts_num;

	/**
	 * Contains the traffic selectors of type traffic_selector_substructure_t.
	 */
	linked_list_t *substrs;
};

/**
 * Encoding rules to parse or generate a TS payload
 *
 * The defined offsets are the positions in a object of type
 * private_ts_payload_t.
 */
static encoding_rule_t encodings[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_ts_payload_t, next_payload)	},
	/* the critical bit */
	{ FLAG,				offsetof(private_ts_payload_t, critical)		},
	/* 7 Bit reserved bits */
	{ RESERVED_BIT,		offsetof(private_ts_payload_t, reserved_bit[0])	},
	{ RESERVED_BIT,		offsetof(private_ts_payload_t, reserved_bit[1])	},
	{ RESERVED_BIT,		offsetof(private_ts_payload_t, reserved_bit[2])	},
	{ RESERVED_BIT,		offsetof(private_ts_payload_t, reserved_bit[3])	},
	{ RESERVED_BIT,		offsetof(private_ts_payload_t, reserved_bit[4])	},
	{ RESERVED_BIT,		offsetof(private_ts_payload_t, reserved_bit[5])	},
	{ RESERVED_BIT,		offsetof(private_ts_payload_t, reserved_bit[6])	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_ts_payload_t, payload_length)	},
	/* 1 Byte TS type*/
	{ U_INT_8,			offsetof(private_ts_payload_t, ts_num)			},
	/* 3 reserved bytes */
	{ RESERVED_BYTE,	offsetof(private_ts_payload_t, reserved_byte[0])},
	{ RESERVED_BYTE,	offsetof(private_ts_payload_t, reserved_byte[1])},
	{ RESERVED_BYTE,	offsetof(private_ts_payload_t, reserved_byte[2])},
	/* wrapped list of traffic selectors substructures */
	{ PAYLOAD_LIST + PLV2_TRAFFIC_SELECTOR_SUBSTRUCTURE,
						offsetof(private_ts_payload_t, substrs)			},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Number of TSs !                 RESERVED                      !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                       <Traffic Selectors>                     ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_ts_payload_t *this)
{
	enumerator_t *enumerator;
	payload_t *substr;
	status_t status = SUCCESS;

	if (this->ts_num != this->substrs->get_count(this->substrs))
	{
		return FAILED;
	}
	enumerator = this->substrs->create_enumerator(this->substrs);
	while (enumerator->enumerate(enumerator, &substr))
	{
		status = substr->verify(substr);
		if (status != SUCCESS)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);

	return status;
}

METHOD(payload_t, get_encoding_rules, int,
	private_ts_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_ts_payload_t *this)
{
	return 8;
}

METHOD(payload_t, get_type, payload_type_t,
	private_ts_payload_t *this)
{
	if (this->is_initiator)
	{
		return PLV2_TS_INITIATOR;
	}
	return PLV2_TS_RESPONDER;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_ts_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_ts_payload_t *this,payload_type_t type)
{
	this->next_payload = type;
}

/**
 * recompute the length of the payload.
 */
static void compute_length(private_ts_payload_t *this)
{
	enumerator_t *enumerator;
	payload_t *subst;

	this->payload_length = get_header_length(this);
	this->ts_num = 0;
	enumerator = this->substrs->create_enumerator(this->substrs);
	while (enumerator->enumerate(enumerator, &subst))
	{
		this->payload_length += subst->get_length(subst);
		this->ts_num++;
	}
	enumerator->destroy(enumerator);
}

METHOD(payload_t, get_length, size_t,
	private_ts_payload_t *this)
{
	return this->payload_length;
}

METHOD(ts_payload_t, get_initiator, bool,
	private_ts_payload_t *this)
{
	return this->is_initiator;
}

METHOD(ts_payload_t, set_initiator, void,
	private_ts_payload_t *this,bool is_initiator)
{
	this->is_initiator = is_initiator;
}

METHOD(ts_payload_t, get_traffic_selectors, linked_list_t*,
	private_ts_payload_t *this)
{
	traffic_selector_t *ts;
	enumerator_t *enumerator;
	traffic_selector_substructure_t *subst;
	linked_list_t *list;

	list = linked_list_create();
	enumerator = this->substrs->create_enumerator(this->substrs);
	while (enumerator->enumerate(enumerator, &subst))
	{
		ts = subst->get_traffic_selector(subst);
		list->insert_last(list, ts);
	}
	enumerator->destroy(enumerator);

	return list;
}

METHOD2(payload_t, ts_payload_t, destroy, void,
	private_ts_payload_t *this)
{
	this->substrs->destroy_offset(this->substrs, offsetof(payload_t, destroy));
	free(this);
}

/*
 * Described in header
 */
ts_payload_t *ts_payload_create(bool is_initiator)
{
	private_ts_payload_t *this;

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
			.get_initiator = _get_initiator,
			.set_initiator = _set_initiator,
			.get_traffic_selectors = _get_traffic_selectors,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.payload_length = get_header_length(this),
		.is_initiator = is_initiator,
		.substrs = linked_list_create(),
	);
	return &this->public;
}

/*
 * Described in header
 */
ts_payload_t *ts_payload_create_from_traffic_selectors(bool is_initiator,
											linked_list_t *traffic_selectors)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	traffic_selector_substructure_t *subst;
	private_ts_payload_t *this;

	this = (private_ts_payload_t*)ts_payload_create(is_initiator);

	enumerator = traffic_selectors->create_enumerator(traffic_selectors);
	while (enumerator->enumerate(enumerator, &ts))
	{
		subst = traffic_selector_substructure_create_from_traffic_selector(ts);
		this->substrs->insert_last(this->substrs, subst);
	}
	enumerator->destroy(enumerator);
	compute_length(this);

	return &this->public;
}
