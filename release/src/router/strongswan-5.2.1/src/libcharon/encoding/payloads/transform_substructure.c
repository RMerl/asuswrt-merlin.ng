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

#include "transform_substructure.h"

#include <encoding/payloads/transform_attribute.h>
#include <encoding/payloads/encodings.h>
#include <library.h>
#include <collections/linked_list.h>
#include <daemon.h>

typedef struct private_transform_substructure_t private_transform_substructure_t;

/**
 * Private data of an transform_substructure_t object.
 */
struct private_transform_substructure_t {

	/**
	 * Public transform_substructure_t interface.
	 */
	transform_substructure_t public;

	/**
	 * Next payload type.
	 */
	u_int8_t  next_payload;

	/**
	 * Reserved byte
	 */
	u_int8_t reserved[3];

	/**
	 * Length of this payload.
	 */
	u_int16_t transform_length;

	/**
	 * Type or number, Type of the transform in IKEv2, number in IKEv2.
	 */
	u_int8_t transform_ton;

	/**
	 * Transform ID, as encoded in IKEv1.
	 */
	u_int8_t transform_id_v1;

	/**
	 * Transform ID, as encoded in IKEv2.
	 */
	u_int16_t transform_id_v2;

	/**
	 * Transforms Attributes are stored in a linked_list_t.
	 */
	linked_list_t *attributes;

	/**
	 * Payload type, PLV2_TRANSFORM_SUBSTRUCTURE or PLV1_TRANSFORM_SUBSTRUCTURE
	 */
	payload_type_t type;
};

/**
 * Encoding rules for PLV2_TRANSFORM_SUBSTRUCTURE
 */
static encoding_rule_t encodings_v2[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_transform_substructure_t, next_payload)	},
	/* 1 Reserved Byte */
	{ RESERVED_BYTE,	offsetof(private_transform_substructure_t, reserved[0])		},
	/* Length of the whole transform substructure*/
	{ PAYLOAD_LENGTH,	offsetof(private_transform_substructure_t, transform_length)},
	/* transform type */
	{ U_INT_8,			offsetof(private_transform_substructure_t, transform_ton)	},
	/* transform identifier, as used by IKEv1 */
	{ RESERVED_BYTE,	offsetof(private_transform_substructure_t, reserved[1])		},
	/* transform identifier, as used by IKEv2 */
	{ U_INT_16,			offsetof(private_transform_substructure_t, transform_id_v2)	},
	/* Attributes in a transform attribute list */
	{ PAYLOAD_LIST + PLV2_TRANSFORM_ATTRIBUTE,
						offsetof(private_transform_substructure_t, attributes)		}
};

/**
 * Encoding rules for PLV1_TRANSFORM_SUBSTRUCTURE
 */
static encoding_rule_t encodings_v1[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_transform_substructure_t, next_payload)	},
	/* 1 Reserved Byte */
	{ RESERVED_BYTE,	offsetof(private_transform_substructure_t, reserved[0])		},
	/* Length of the whole transform substructure*/
	{ PAYLOAD_LENGTH,	offsetof(private_transform_substructure_t, transform_length)},
	/* transform number */
	{ U_INT_8,			offsetof(private_transform_substructure_t, transform_ton)},
	/* transform identifier, as used by IKEv1 */
	{ U_INT_8,			offsetof(private_transform_substructure_t, transform_id_v1)	},
	/* transform identifier, as used by IKEv2 */
	{ RESERVED_BYTE,	offsetof(private_transform_substructure_t, reserved[1])		},
	{ RESERVED_BYTE,	offsetof(private_transform_substructure_t, reserved[2])		},
	/* Attributes in a transform attribute list */
	{ PAYLOAD_LIST + PLV1_TRANSFORM_ATTRIBUTE,
						offsetof(private_transform_substructure_t, attributes)		}
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! 0 (last) or 3 !   RESERVED    !        Transform Length       !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Tfrm Typ or # ! Tfrm ID IKEv1 !        Transform ID IKEv2     !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                      Transform Attributes                     ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_transform_substructure_t *this)
{
	status_t status = SUCCESS;
	enumerator_t *enumerator;
	payload_t *attribute;

	if (this->next_payload != PL_NONE && this->next_payload != 3)
	{
		DBG1(DBG_ENC, "inconsistent next payload");
		return FAILED;
	}

	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &attribute))
	{
		status = attribute->verify(attribute);
		if (status != SUCCESS)
		{
			DBG1(DBG_ENC, "TRANSFORM_ATTRIBUTE verification failed");
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* proposal number is checked in SA payload */
	return status;
}

METHOD(payload_t, get_encoding_rules, int,
	private_transform_substructure_t *this, encoding_rule_t **rules)
{
	if (this->type == PLV2_TRANSFORM_SUBSTRUCTURE)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_transform_substructure_t *this)
{
	return 8;
}

METHOD(payload_t, get_type, payload_type_t,
	private_transform_substructure_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_transform_substructure_t *this)
{
	return this->next_payload;
}

/**
 * recompute the length of the payload.
 */
static void compute_length(private_transform_substructure_t *this)
{
	enumerator_t *enumerator;
	payload_t *attribute;

	this->transform_length = get_header_length(this);
	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &attribute))
	{
		this->transform_length += attribute->get_length(attribute);
	}
	enumerator->destroy(enumerator);
}

METHOD(payload_t, get_length, size_t,
	private_transform_substructure_t *this)
{
	return this->transform_length;
}

METHOD(transform_substructure_t, add_transform_attribute, void,
	private_transform_substructure_t *this, transform_attribute_t *attribute)
{
	this->attributes->insert_last(this->attributes, attribute);
	compute_length(this);
}

METHOD(transform_substructure_t, set_is_last_transform, void,
	private_transform_substructure_t *this, bool is_last)
{
	this->next_payload = is_last ? 0: TRANSFORM_TYPE_VALUE;
}

METHOD(payload_t, set_next_type, void,
	private_transform_substructure_t *this,payload_type_t type)
{
}

METHOD(transform_substructure_t, get_transform_type_or_number, u_int8_t,
	private_transform_substructure_t *this)
{
	return this->transform_ton;
}

METHOD(transform_substructure_t, get_transform_id, u_int16_t,
	private_transform_substructure_t *this)
{
	if (this->type == PLV2_TRANSFORM_SUBSTRUCTURE)
	{
		return this->transform_id_v2;
	}
	return this->transform_id_v1;
}

METHOD(transform_substructure_t, create_attribute_enumerator, enumerator_t*,
	private_transform_substructure_t *this)
{
	return this->attributes->create_enumerator(this->attributes);
}

METHOD2(payload_t, transform_substructure_t, destroy, void,
	private_transform_substructure_t *this)
{
	this->attributes->destroy_offset(this->attributes,
									 offsetof(payload_t, destroy));
	free(this);
}

/*
 * Described in header.
 */
transform_substructure_t *transform_substructure_create(payload_type_t type)
{
	private_transform_substructure_t *this;

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
			.add_transform_attribute = _add_transform_attribute,
			.set_is_last_transform = _set_is_last_transform,
			.get_transform_type_or_number = _get_transform_type_or_number,
			.get_transform_id = _get_transform_id,
			.create_attribute_enumerator = _create_attribute_enumerator,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.transform_length = get_header_length(this),
		.attributes = linked_list_create(),
		.type = type,
	);
	return &this->public;
}

/*
 * Described in header
 */
transform_substructure_t *transform_substructure_create_type(payload_type_t type,
				u_int8_t type_or_number, u_int16_t id)
{
	private_transform_substructure_t *this;

	this = (private_transform_substructure_t*)transform_substructure_create(type);

	this->transform_ton = type_or_number;
	if (type == PLV2_TRANSFORM_SUBSTRUCTURE)
	{
		this->transform_id_v2 = id;
	}
	else
	{
		this->transform_id_v1 = id;
	}
	return &this->public;
}

