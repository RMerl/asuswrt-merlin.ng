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

#include "auth_payload.h"

#include <encoding/payloads/encodings.h>

typedef struct private_auth_payload_t private_auth_payload_t;

/**
 * Private data of an auth_payload_t object.
 *
 */
struct private_auth_payload_t {

	/**
	 * Public auth_payload_t interface.
	 */
	auth_payload_t public;

	/**
	 * Next payload type.
	 */
	u_int8_t  next_payload;

	/**
	 * Critical flag.
	 */
	bool critical;

	/**
	 * Reserved bits
	 */
	bool reserved_bit[7];

	/**
	 * Reserved bytes
	 */
	u_int8_t reserved_byte[3];

	/**
	 * Length of this payload.
	 */
	u_int16_t payload_length;

	/**
	 * Method of the AUTH Data.
	 */
	u_int8_t auth_method;

	/**
	 * The contained auth data value.
	 */
	chunk_t auth_data;
};

/**
 * Encoding rules to parse or generate a AUTH payload
 *
 * The defined offsets are the positions in a object of type
 * private_auth_payload_t.
 */
static encoding_rule_t encodings[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_auth_payload_t, next_payload)		},
	/* the critical bit */
	{ FLAG,				offsetof(private_auth_payload_t, critical)			},
	/* 7 Bit reserved bits */
	{ RESERVED_BIT,		offsetof(private_auth_payload_t, reserved_bit[0])	},
	{ RESERVED_BIT,		offsetof(private_auth_payload_t, reserved_bit[1])	},
	{ RESERVED_BIT,		offsetof(private_auth_payload_t, reserved_bit[2])	},
	{ RESERVED_BIT,		offsetof(private_auth_payload_t, reserved_bit[3])	},
	{ RESERVED_BIT,		offsetof(private_auth_payload_t, reserved_bit[4])	},
	{ RESERVED_BIT,		offsetof(private_auth_payload_t, reserved_bit[5])	},
	{ RESERVED_BIT,		offsetof(private_auth_payload_t, reserved_bit[6])	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_auth_payload_t, payload_length)	},
	/* 1 Byte AUTH type*/
	{ U_INT_8,			offsetof(private_auth_payload_t, auth_method)		},
	/* 3 reserved bytes */
	{ RESERVED_BYTE,	offsetof(private_auth_payload_t, reserved_byte[0])	},
	{ RESERVED_BYTE,	offsetof(private_auth_payload_t, reserved_byte[1])	},
	{ RESERVED_BYTE,	offsetof(private_auth_payload_t, reserved_byte[2])	},
	/* some auth data bytes, length is defined in PAYLOAD_LENGTH */
	{ CHUNK_DATA,		offsetof(private_auth_payload_t, auth_data)	}
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Auth Method   !                RESERVED                       !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                      Authentication Data                      ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_auth_payload_t *this)
{
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_auth_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_auth_payload_t *this)
{
	return 8;
}

METHOD(payload_t, get_type, payload_type_t,
	private_auth_payload_t *this)
{
	return PLV2_AUTH;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_auth_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_auth_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_auth_payload_t *this)
{
	return this->payload_length;
}

METHOD(auth_payload_t, set_auth_method, void,
	private_auth_payload_t *this, auth_method_t method)
{
	this->auth_method = method;
}

METHOD(auth_payload_t, get_auth_method, auth_method_t,
	private_auth_payload_t *this)
{
	return this->auth_method;
}

METHOD(auth_payload_t, set_data, void,
	private_auth_payload_t *this, chunk_t data)
{
	free(this->auth_data.ptr);
	this->auth_data = chunk_clone(data);
	this->payload_length = get_header_length(this) + this->auth_data.len;
}

METHOD(auth_payload_t, get_data, chunk_t,
	private_auth_payload_t *this)
{
	return this->auth_data;
}

METHOD2(payload_t, auth_payload_t, destroy, void,
	private_auth_payload_t *this)
{
	free(this->auth_data.ptr);
	free(this);
}

/*
 * Described in header
 */
auth_payload_t *auth_payload_create()
{
	private_auth_payload_t *this;

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
			.set_auth_method = _set_auth_method,
			.get_auth_method = _get_auth_method,
			.set_data = _set_data,
			.get_data = _get_data,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.payload_length = get_header_length(this),
	);
	return &this->public;
}
