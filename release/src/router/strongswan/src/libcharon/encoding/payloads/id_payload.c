/*
 * Copyright (C) 2005-2011 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2007-2011 Tobias Brunner
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

#include "id_payload.h"

#include <daemon.h>
#include <encoding/payloads/encodings.h>

typedef struct private_id_payload_t private_id_payload_t;

/**
 * Private data of an id_payload_t object.
 */
struct private_id_payload_t {

	/**
	 * Public id_payload_t interface.
	 */
	id_payload_t public;

	/**
	 * Next payload type.
	 */
	uint8_t next_payload;

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
	uint8_t reserved_byte[3];

	/**
	 * Length of this payload.
	 */
	uint16_t payload_length;

	/**
	 * Type of the ID Data.
	 */
	uint8_t id_type;

	/**
	 * The contained id data value.
	 */
	chunk_t id_data;

	/**
	 * Tunneled protocol ID for IKEv1 quick modes.
	 */
	uint8_t protocol_id;

	/**
	 * Tunneled port for IKEv1 quick modes.
	 */
	uint16_t port;

	/**
	 * one of PLV2_ID_INITIATOR, PLV2_ID_RESPONDER, IDv1 and PLV1_NAT_OA
	 */
	payload_type_t type;
};

/**
 * Encoding rules for an IKEv2 ID payload
 */
static encoding_rule_t encodings_v2[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_id_payload_t, next_payload)	},
	/* the critical bit */
	{ FLAG,				offsetof(private_id_payload_t, critical)		},
	/* 7 Bit reserved bits */
	{ RESERVED_BIT,		offsetof(private_id_payload_t, reserved_bit[0])	},
	{ RESERVED_BIT,		offsetof(private_id_payload_t, reserved_bit[1])	},
	{ RESERVED_BIT,		offsetof(private_id_payload_t, reserved_bit[2])	},
	{ RESERVED_BIT,		offsetof(private_id_payload_t, reserved_bit[3])	},
	{ RESERVED_BIT,		offsetof(private_id_payload_t, reserved_bit[4])	},
	{ RESERVED_BIT,		offsetof(private_id_payload_t, reserved_bit[5])	},
	{ RESERVED_BIT,		offsetof(private_id_payload_t, reserved_bit[6])	},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_id_payload_t, payload_length)	},
	/* 1 Byte ID type*/
	{ U_INT_8,			offsetof(private_id_payload_t, id_type)			},
	/* 3 reserved bytes */
	{ RESERVED_BYTE,	offsetof(private_id_payload_t, reserved_byte[0])},
	{ RESERVED_BYTE,	offsetof(private_id_payload_t, reserved_byte[1])},
	{ RESERVED_BYTE,	offsetof(private_id_payload_t, reserved_byte[2])},
	/* some id data bytes, length is defined in PAYLOAD_LENGTH */
	{ CHUNK_DATA,		offsetof(private_id_payload_t, id_data)			},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !   ID Type     !                 RESERVED                      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                   Identification Data                         ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/**
 * Encoding rules for an IKEv1 ID payload
 */
static encoding_rule_t encodings_v1[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_id_payload_t, next_payload)	},
	/* Reserved Byte is skipped */
	{ RESERVED_BYTE,	offsetof(private_id_payload_t, reserved_byte[0])},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_id_payload_t, payload_length)	},
	/* 1 Byte ID type*/
	{ U_INT_8,			offsetof(private_id_payload_t, id_type)			},
	{ U_INT_8,			offsetof(private_id_payload_t, protocol_id)		},
	{ U_INT_16,			offsetof(private_id_payload_t, port)			},
	/* some id data bytes, length is defined in PAYLOAD_LENGTH */
	{ CHUNK_DATA,		offsetof(private_id_payload_t, id_data)			},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !    RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !   ID Type     ! Protocol ID   !           Port                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                   Identification Data                         ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_id_payload_t *this)
{
	bool bad_length = FALSE;

	if ((this->type == PLV1_NAT_OA || this->type == PLV1_NAT_OA_DRAFT_00_03) &&
		this->id_type != ID_IPV4_ADDR && this->id_type != ID_IPV6_ADDR)
	{
		DBG1(DBG_ENC, "invalid ID type %N for %N payload", id_type_names,
			 this->id_type, payload_type_short_names, this->type);
		return FAILED;
	}
	switch (this->id_type)
	{
		case ID_IPV4_ADDR_RANGE:
		case ID_IPV4_ADDR_SUBNET:
			bad_length = this->id_data.len != 8;
			break;
		case ID_IPV6_ADDR_RANGE:
		case ID_IPV6_ADDR_SUBNET:
			bad_length = this->id_data.len != 32;
			break;
	}
	if (bad_length)
	{
		DBG1(DBG_ENC, "invalid %N length (%d bytes)",
			 id_type_names, this->id_type, this->id_data.len);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_id_payload_t *this, encoding_rule_t **rules)
{
	if (this->type == PLV1_ID ||
		this->type == PLV1_NAT_OA || this->type == PLV1_NAT_OA_DRAFT_00_03)
	{
		*rules = encodings_v1;
		return countof(encodings_v1);
	}
	*rules = encodings_v2;
	return countof(encodings_v2);
}

METHOD(payload_t, get_header_length, int,
	private_id_payload_t *this)
{
	return 8;
}

METHOD(payload_t, get_type, payload_type_t,
	private_id_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_id_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_id_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_id_payload_t *this)
{
	return this->payload_length;
}

METHOD(id_payload_t, get_identification, identification_t*,
	private_id_payload_t *this)
{
	return identification_create_from_encoding(this->id_type, this->id_data);
}

/**
 * Create a traffic selector from an range ID
 */
static traffic_selector_t *get_ts_from_range(private_id_payload_t *this,
											 ts_type_t type)
{
	return traffic_selector_create_from_bytes(this->protocol_id, type,
		chunk_create(this->id_data.ptr, this->id_data.len / 2), this->port,
		chunk_skip(this->id_data, this->id_data.len / 2), this->port ?: 65535);
}

/**
 * Create a traffic selector from an subnet ID
 */
static traffic_selector_t *get_ts_from_subnet(private_id_payload_t *this,
											  ts_type_t type)
{
	traffic_selector_t *ts;
	chunk_t net, netmask;
	int i;

	net = chunk_create(this->id_data.ptr, this->id_data.len / 2);
	netmask = chunk_clone(chunk_skip(this->id_data, this->id_data.len / 2));
	for (i = 0; i < net.len; i++)
	{
		netmask.ptr[i] = (netmask.ptr[i] ^ 0xFF) | net.ptr[i];
	}
	ts = traffic_selector_create_from_bytes(this->protocol_id, type,
								net, this->port, netmask, this->port ?: 65535);
	chunk_free(&netmask);
	return ts;
}

/**
 * Create a traffic selector from an IP ID
 */
static traffic_selector_t *get_ts_from_ip(private_id_payload_t *this,
										  ts_type_t type)
{
	return traffic_selector_create_from_bytes(this->protocol_id, type,
				this->id_data, this->port, this->id_data, this->port ?: 65535);
}

METHOD(id_payload_t, get_ts, traffic_selector_t*,
	private_id_payload_t *this)
{
	switch (this->id_type)
	{
		case ID_IPV4_ADDR_SUBNET:
			if (this->id_data.len == 8)
			{
				return get_ts_from_subnet(this, TS_IPV4_ADDR_RANGE);
			}
			break;
		case ID_IPV6_ADDR_SUBNET:
			if (this->id_data.len == 32)
			{
				return get_ts_from_subnet(this, TS_IPV6_ADDR_RANGE);
			}
			break;
		case ID_IPV4_ADDR_RANGE:
			if (this->id_data.len == 8)
			{
				return get_ts_from_range(this, TS_IPV4_ADDR_RANGE);
			}
			break;
		case ID_IPV6_ADDR_RANGE:
			if (this->id_data.len == 32)
			{
				return get_ts_from_range(this, TS_IPV6_ADDR_RANGE);
			}
			break;
		case ID_IPV4_ADDR:
			if (this->id_data.len == 4)
			{
				return get_ts_from_ip(this, TS_IPV4_ADDR_RANGE);
			}
			break;
		case ID_IPV6_ADDR:
			if (this->id_data.len == 16)
			{
				return get_ts_from_ip(this, TS_IPV6_ADDR_RANGE);
			}
			break;
		default:
			break;
	}
	return NULL;
}

METHOD(id_payload_t, get_encoded, chunk_t,
	private_id_payload_t *this)
{
	uint16_t port = htons(this->port);
	return chunk_cat("cccc", chunk_from_thing(this->id_type),
					 chunk_from_thing(this->protocol_id),
					 chunk_from_thing(port), this->id_data);
}

METHOD2(payload_t, id_payload_t, destroy, void,
	private_id_payload_t *this)
{
	free(this->id_data.ptr);
	free(this);
}

/*
 * Described in header.
 */
id_payload_t *id_payload_create(payload_type_t type)
{
	private_id_payload_t *this;

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
			.get_identification = _get_identification,
			.get_encoded = _get_encoded,
			.get_ts = _get_ts,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.payload_length = get_header_length(this),
		.type = type,
	);
	return &this->public;
}

/*
 * Described in header.
 */
id_payload_t *id_payload_create_from_identification(payload_type_t type,
													identification_t *id)
{
	private_id_payload_t *this;

	this = (private_id_payload_t*)id_payload_create(type);
	this->id_data = chunk_clone(id->get_encoding(id));
	this->id_type = id->get_type(id);
	this->payload_length += this->id_data.len;

	return &this->public;
}

/*
 * Described in header.
 */
id_payload_t *id_payload_create_from_ts(traffic_selector_t *ts)
{
	private_id_payload_t *this;
	uint8_t mask;
	host_t *net;

	this = (private_id_payload_t*)id_payload_create(PLV1_ID);

	if (ts->is_host(ts, NULL))
	{
		if (ts->get_type(ts) == TS_IPV4_ADDR_RANGE)
		{
			this->id_type = ID_IPV4_ADDR;
		}
		else
		{
			this->id_type = ID_IPV6_ADDR;
		}
		this->id_data = chunk_clone(ts->get_from_address(ts));
	}
	else if (ts->to_subnet(ts, &net, &mask))
	{
		uint8_t netmask[16], len, byte;

		if (ts->get_type(ts) == TS_IPV4_ADDR_RANGE)
		{
			this->id_type = ID_IPV4_ADDR_SUBNET;
			len = 4;
		}
		else
		{
			this->id_type = ID_IPV6_ADDR_SUBNET;
			len = 16;
		}
		memset(netmask, 0, sizeof(netmask));
		for (byte = 0; byte < sizeof(netmask); byte++)
		{
			if (mask < 8)
			{
				netmask[byte] = 0xFF << (8 - mask);
				break;
			}
			netmask[byte] = 0xFF;
			mask -= 8;
		}
		this->id_data = chunk_cat("cc", net->get_address(net),
								  chunk_create(netmask, len));
		net->destroy(net);
	}
	else
	{
		if (ts->get_type(ts) == TS_IPV4_ADDR_RANGE)
		{
			this->id_type = ID_IPV4_ADDR_RANGE;
		}
		else
		{
			this->id_type = ID_IPV6_ADDR_RANGE;
		}
		this->id_data = chunk_cat("cc",
							ts->get_from_address(ts), ts->get_to_address(ts));
		net->destroy(net);
	}
	this->port = ts->get_from_port(ts);
	this->protocol_id = ts->get_protocol(ts);
	this->payload_length += this->id_data.len;

	return &this->public;
}

