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

#include <stddef.h>

#include "configuration_attribute.h"

#include <encoding/payloads/encodings.h>
#include <library.h>
#include <daemon.h>

typedef struct private_configuration_attribute_t private_configuration_attribute_t;

/**
 * Private data of an configuration_attribute_t object.
 */
struct private_configuration_attribute_t {

	/**
	 * Public configuration_attribute_t interface.
	 */
	configuration_attribute_t public;

	/**
	 * Value encoded in length field?
	 */
	bool af_flag;

	/**
	 * Reserved bit (af_flag in IKEv2)
	 */
	bool reserved;

	/**
	 * Type of the attribute.
	 */
	u_int16_t attr_type;

	/**
	 * Length of the attribute, value if af_flag set.
	 */
	u_int16_t length_or_value;

	/**
	 * Attribute value as chunk.
	 */
	chunk_t value;

	/**
	 * Payload type, PLV2_CONFIGURATION_ATTRIBUTE or DATA_ATTRIBUTE_V1
	 */
	payload_type_t type;
};

/**
 * Encoding rules for a IKEv2 configuration attribute / IKEv1 data attribute
 */
static encoding_rule_t encodings_v2[] = {
	/* 1 reserved bit */
	{ RESERVED_BIT,					offsetof(private_configuration_attribute_t, reserved)		},
	/* type of the attribute as 15 bit unsigned integer */
	{ ATTRIBUTE_TYPE,				offsetof(private_configuration_attribute_t, attr_type)		},
	/* Length of attribute value */
	{ ATTRIBUTE_LENGTH,				offsetof(private_configuration_attribute_t, length_or_value)},
	/* Value of attribute if attribute format flag is zero */
	{ ATTRIBUTE_VALUE,				offsetof(private_configuration_attribute_t, value)			},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !R|         Attribute Type      !            Length             |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      ~                             Value                             ~
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/**
 * Encoding rules for a IKEv1 data attribute
 */
static encoding_rule_t encodings_v1[] = {
	/* AF Flag */
	{ ATTRIBUTE_FORMAT,				offsetof(private_configuration_attribute_t, af_flag)		},
	/* type of the attribute as 15 bit unsigned integer */
	{ ATTRIBUTE_TYPE,				offsetof(private_configuration_attribute_t, attr_type)		},
	/* Length of attribute value */
	{ ATTRIBUTE_LENGTH_OR_VALUE,	offsetof(private_configuration_attribute_t, length_or_value)},
	/* Value of attribute if attribute format flag is zero */
	{ ATTRIBUTE_VALUE,				offsetof(private_configuration_attribute_t, value)			},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !F|         Attribute Type      !            Length             |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                               |
      ~                             Value                             ~
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/


METHOD(payload_t, verify, status_t,
	private_configuration_attribute_t *this)
{
	bool failed = FALSE;

	switch (this->attr_type)
	{
		case INTERNAL_IP4_ADDRESS:
		case INTERNAL_IP4_NETMASK:
		case INTERNAL_IP4_DNS:
		case INTERNAL_IP4_NBNS:
		case INTERNAL_ADDRESS_EXPIRY:
		case INTERNAL_IP4_DHCP:
			if (this->length_or_value != 0 && this->length_or_value != 4)
			{
				failed = TRUE;
			}
			break;
		case INTERNAL_IP4_SUBNET:
			if (this->length_or_value != 0 && this->length_or_value != 8)
			{
				failed = TRUE;
			}
			break;
		case INTERNAL_IP6_ADDRESS:
		case INTERNAL_IP6_SUBNET:
			if (this->length_or_value != 0 && this->length_or_value != 17)
			{
				failed = TRUE;
			}
			break;
		case INTERNAL_IP6_DNS:
		case INTERNAL_IP6_NBNS:
		case INTERNAL_IP6_DHCP:
			if (this->length_or_value != 0 && this->length_or_value != 16)
			{
				failed = TRUE;
			}
			break;
		case SUPPORTED_ATTRIBUTES:
			if (this->length_or_value % 2)
			{
				failed = TRUE;
			}
			break;
		case APPLICATION_VERSION:
		case INTERNAL_IP4_SERVER:
		case INTERNAL_IP6_SERVER:
		case XAUTH_TYPE:
		case XAUTH_USER_NAME:
		case XAUTH_USER_PASSWORD:
		case XAUTH_PASSCODE:
		case XAUTH_MESSAGE:
		case XAUTH_CHALLENGE:
		case XAUTH_DOMAIN:
		case XAUTH_STATUS:
		case XAUTH_NEXT_PIN:
		case XAUTH_ANSWER:
		case UNITY_BANNER:
		case UNITY_SAVE_PASSWD:
		case UNITY_DEF_DOMAIN:
		case UNITY_SPLITDNS_NAME:
		case UNITY_SPLIT_INCLUDE:
		case UNITY_NATT_PORT:
		case UNITY_LOCAL_LAN:
		case UNITY_PFS:
		case UNITY_FW_TYPE:
		case UNITY_BACKUP_SERVERS:
		case UNITY_DDNS_HOSTNAME:
			/* any length acceptable */
			break;
		default:
			DBG1(DBG_ENC, "unknown attribute type %N",
				 configuration_attribute_type_names, this->attr_type);
			break;
	}

	if (failed)
	{
		DBG1(DBG_ENC, "invalid attribute length %d for %N",
			 this->length_or_value, configuration_attribute_type_names,
			 this->attr_type);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_configuration_attribute_t *this, encoding_rule_t **rules)
{
	if (this->type == PLV2_CONFIGURATION_ATTRIBUTE)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_configuration_attribute_t *this)
{
	return 4;
}

METHOD(payload_t, get_type, payload_type_t,
	private_configuration_attribute_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_configuration_attribute_t *this)
{
	return PL_NONE;
}

METHOD(payload_t, set_next_type, void,
	private_configuration_attribute_t *this, payload_type_t type)
{
}

METHOD(payload_t, get_length, size_t,
	private_configuration_attribute_t *this)
{
	return get_header_length(this) + this->value.len;
}

METHOD(configuration_attribute_t, get_cattr_type, configuration_attribute_type_t,
	private_configuration_attribute_t *this)
{
	return this->attr_type;
}

METHOD(configuration_attribute_t, get_chunk, chunk_t,
	private_configuration_attribute_t *this)
{
	if (this->af_flag)
	{
		return chunk_from_thing(this->length_or_value);
	}
	return this->value;
}

METHOD(configuration_attribute_t, get_value, u_int16_t,
	private_configuration_attribute_t *this)
{
	if (this->af_flag)
	{
		return this->length_or_value;
	}
	return 0;
}

METHOD2(payload_t, configuration_attribute_t, destroy, void,
	private_configuration_attribute_t *this)
{
	free(this->value.ptr);
	free(this);
}

/*
 * Described in header.
 */
configuration_attribute_t *configuration_attribute_create(payload_type_t type)
{
	private_configuration_attribute_t *this;

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
			.get_chunk = _get_chunk,
			.get_value = _get_value,
			.get_type = _get_cattr_type,
			.destroy = _destroy,
		},
		.type = type
	);
	return &this->public;
}

/*
 * Described in header.
 */
configuration_attribute_t *configuration_attribute_create_chunk(
	payload_type_t type, configuration_attribute_type_t attr_type, chunk_t chunk)
{
	private_configuration_attribute_t *this;

	this = (private_configuration_attribute_t*)
							configuration_attribute_create(type);
	this->attr_type = ((u_int16_t)attr_type) & 0x7FFF;
	this->value = chunk_clone(chunk);
	this->length_or_value = chunk.len;

	return &this->public;
}

/*
 * Described in header.
 */
configuration_attribute_t *configuration_attribute_create_value(
					configuration_attribute_type_t attr_type, u_int16_t value)
{
	private_configuration_attribute_t *this;

	this = (private_configuration_attribute_t*)
					configuration_attribute_create(PLV1_CONFIGURATION_ATTRIBUTE);
	this->attr_type = ((u_int16_t)attr_type) & 0x7FFF;
	this->length_or_value = value;
	this->af_flag = TRUE;

	return &this->public;
}

