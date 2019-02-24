/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "fragment_payload.h"

#include <encoding/payloads/encodings.h>

/** Flag that is set in case the given fragment is the last for the message */
#define LAST_FRAGMENT 0x01

typedef struct private_fragment_payload_t private_fragment_payload_t;

/**
 * Private data of an fragment_payload_t object.
 */
struct private_fragment_payload_t {

	/**
	 * Public fragment_payload_t interface.
	 */
	fragment_payload_t public;

	/**
	 * Next payload type.
	 */
	uint8_t next_payload;

	/**
	 * Reserved byte
	 */
	uint8_t reserved;

	/**
	 * Length of this payload.
	 */
	uint16_t payload_length;

	/**
	 * Fragment ID.
	 */
	uint16_t fragment_id;

	/**
	 * Fragment number.
	 */
	uint8_t fragment_number;

	/**
	 * Flags
	 */
	uint8_t flags;

	/**
	 * The contained fragment data.
	 */
	chunk_t data;
};

/**
 * Encoding rules for an IKEv1 fragment payload
 */
static encoding_rule_t encodings[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_fragment_payload_t, next_payload)		},
	{ RESERVED_BYTE,	offsetof(private_fragment_payload_t, reserved)			},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_fragment_payload_t, payload_length)	},
	{ U_INT_16,			offsetof(private_fragment_payload_t, fragment_id)		},
	{ U_INT_8,			offsetof(private_fragment_payload_t, fragment_number)	},
	{ U_INT_8,			offsetof(private_fragment_payload_t, flags)				},
	/* Fragment data is of variable size */
	{ CHUNK_DATA,		offsetof(private_fragment_payload_t, data)				},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !    RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !         Fragment ID           !  Fragment Num !     Flags     !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                       Fragment Data                           ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_fragment_payload_t *this)
{
	if (this->fragment_number == 0)
	{
		return FAILED;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_fragment_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_fragment_payload_t *this)
{
	return 8;
}

METHOD(payload_t, get_type, payload_type_t,
	private_fragment_payload_t *this)
{
	return PLV1_FRAGMENT;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_fragment_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_fragment_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_fragment_payload_t *this)
{
	return this->payload_length;
}

METHOD(fragment_payload_t, get_id, uint16_t,
	private_fragment_payload_t *this)
{
	return this->fragment_id;
}

METHOD(fragment_payload_t, get_number, uint8_t,
	private_fragment_payload_t *this)
{
	return this->fragment_number;
}

METHOD(fragment_payload_t, is_last, bool,
	private_fragment_payload_t *this)
{
	return (this->flags & LAST_FRAGMENT) == LAST_FRAGMENT;
}

METHOD(fragment_payload_t, get_data, chunk_t,
	private_fragment_payload_t *this)
{
	return this->data;
}

METHOD2(payload_t, fragment_payload_t, destroy, void,
	private_fragment_payload_t *this)
{
	free(this->data.ptr);
	free(this);
}

/*
 * Described in header
 */
fragment_payload_t *fragment_payload_create()
{
	private_fragment_payload_t *this;

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
			.get_id = _get_id,
			.get_number = _get_number,
			.is_last = _is_last,
			.get_data = _get_data,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
	);
	this->payload_length = get_header_length(this);
	return &this->public;
}

/*
 * Described in header
 */
fragment_payload_t *fragment_payload_create_from_data(uint8_t num, bool last,
													  chunk_t data)
{
	private_fragment_payload_t *this;

	this = (private_fragment_payload_t*)fragment_payload_create();
	this->fragment_id = 1;
	this->fragment_number = num;
	this->flags |= (last ? LAST_FRAGMENT : 0);
	this->data = chunk_clone(data);
	this->payload_length = get_header_length(this) + data.len;
	return &this->public;
}
