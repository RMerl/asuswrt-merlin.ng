/*
 * Copyright (C) 2007 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
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

/* offsetof macro */
#include <stddef.h>

#include "ike_header.h"

#include <encoding/payloads/encodings.h>


typedef struct private_ike_header_t private_ike_header_t;

/**
 * Private data of an ike_header_t object.
 */
struct private_ike_header_t {
	/**
	 * Public interface.
	 */
	ike_header_t public;

	/**
	 * SPI of the initiator.
	 */
	uint64_t initiator_spi;

	/**
	 * SPI of the responder.
	 */
	uint64_t responder_spi;

	/**
	 * Next payload type.
	 */
	uint8_t  next_payload;
	/**
	 * IKE major version.
	 */
	uint8_t  maj_version;

	/**
	 * IKE minor version.
	 */
	uint8_t  min_version;

	/**
	 * Exchange type .
	 */
	uint8_t  exchange_type;

	/**
	 * Flags of the Message.
	 */
	struct {
		/**
		 * Sender is initiator of the associated IKE_SA_INIT-Exchange.
		 */
		bool initiator;

		/**
		 * Is protocol supporting higher version?
		 */
		bool version;

		/**
		 * TRUE, if this is a response, FALSE if its a Request.
		 */
		bool response;

		/**
		 * TRUE, if the packet is encrypted (IKEv1).
		 */
		bool encryption;

		/**
		 * TRUE, if the commit flag is set (IKEv1).
		 */
		bool commit;

		/**
		 * TRUE, if the auth only flag is set (IKEv1).
		 */
		bool authonly;
	} flags;

	/**
	 * Reserved bits of IKE header
	 */
	bool reserved[2];

	/**
	 * Associated Message-ID.
	 */
	uint32_t message_id;

	/**
	 * Length of the whole IKEv2-Message (header and all payloads).
	 */
	uint32_t length;
};

ENUM_BEGIN(exchange_type_names, ID_PROT, TRANSACTION,
	"ID_PROT",
	"AUTH_ONLY",
	"AGGRESSIVE",
	"INFORMATIONAL_V1",
	"TRANSACTION");
ENUM_NEXT(exchange_type_names, QUICK_MODE, IKE_SESSION_RESUME, TRANSACTION,
	"QUICK_MODE",
	"NEW_GROUP_MODE",
	"IKE_SA_INIT",
	"IKE_AUTH",
	"CREATE_CHILD_SA",
	"INFORMATIONAL",
	"IKE_SESSION_RESUME");
#ifdef ME
ENUM_NEXT(exchange_type_names, ME_CONNECT, ME_CONNECT, IKE_SESSION_RESUME,
	"ME_CONNECT");
ENUM_NEXT(exchange_type_names, EXCHANGE_TYPE_UNDEFINED,
							   EXCHANGE_TYPE_UNDEFINED, ME_CONNECT,
	"EXCHANGE_TYPE_UNDEFINED");
#else
ENUM_NEXT(exchange_type_names, EXCHANGE_TYPE_UNDEFINED,
							   EXCHANGE_TYPE_UNDEFINED, IKE_SESSION_RESUME,
	"EXCHANGE_TYPE_UNDEFINED");
#endif /* ME */
ENUM_END(exchange_type_names, EXCHANGE_TYPE_UNDEFINED);

/**
 * Encoding rules to parse or generate a IKE-Header.
 *
 * The defined offsets are the positions in a object of type
 * ike_header_t.
 */
static encoding_rule_t encodings[] = {
	/* 8 Byte SPI, stored in the field initiator_spi */
	{ IKE_SPI,		offsetof(private_ike_header_t, initiator_spi)	},
	/* 8 Byte SPI, stored in the field responder_spi */
	{ IKE_SPI,		offsetof(private_ike_header_t, responder_spi)	},
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,		offsetof(private_ike_header_t, next_payload)	},
	/* 4 Bit major version, stored in the field maj_version */
	{ U_INT_4,		offsetof(private_ike_header_t, maj_version)		},
	/* 4 Bit minor version, stored in the field min_version */
	{ U_INT_4,		offsetof(private_ike_header_t, min_version)		},
	/* 8 Bit for the exchange type */
	{ U_INT_8,		offsetof(private_ike_header_t, exchange_type)	},
	/* 2 Bit reserved bits */
	{ RESERVED_BIT,	offsetof(private_ike_header_t, reserved[0])		},
	{ RESERVED_BIT,	offsetof(private_ike_header_t, reserved[1])		},
	/* 6 flags  */
	{ FLAG,			offsetof(private_ike_header_t, flags.response)	},
	{ FLAG,			offsetof(private_ike_header_t, flags.version)	},
	{ FLAG,			offsetof(private_ike_header_t, flags.initiator)	},
	{ FLAG,			offsetof(private_ike_header_t, flags.authonly)	},
	{ FLAG,			offsetof(private_ike_header_t, flags.commit)	},
	{ FLAG,			offsetof(private_ike_header_t, flags.encryption)},
	/* 4 Byte message id, stored in the field message_id */
	{ U_INT_32,		offsetof(private_ike_header_t, message_id)		},
	/* 4 Byte length fied, stored in the field length */
	{ HEADER_LENGTH,	offsetof(private_ike_header_t, length)			}
};

/*                         1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                       IKE_SA Initiator's SPI                  !
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                       IKE_SA Responder's SPI                  !
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !  Next Payload ! MjVer ! MnVer ! Exchange Type !     Flags     !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                          Message ID                           !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                            Length                             !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_ike_header_t *this)
{
	switch (this->exchange_type)
	{
		case ID_PROT:
		case AGGRESSIVE:
			if (this->message_id != 0)
			{
				return FAILED;
			}
			/* fall */
		case AUTH_ONLY:
		case INFORMATIONAL_V1:
		case TRANSACTION:
		case QUICK_MODE:
		case NEW_GROUP_MODE:
			if (this->maj_version == IKEV2_MAJOR_VERSION)
			{
				/* IKEv1 exchange type in IKEv2? */
				return FAILED;
			}
			break;
		case IKE_SA_INIT:
		case IKE_AUTH:
		case CREATE_CHILD_SA:
		case INFORMATIONAL:
		case IKE_SESSION_RESUME:
#ifdef ME
		case ME_CONNECT:
#endif /* ME */
			if (this->maj_version == IKEV1_MAJOR_VERSION)
			{
				/* IKEv2 exchange type in IKEv1? */
				return FAILED;
			}
			break;
		default:
			if (this->maj_version == IKEV1_MAJOR_VERSION ||
				this->maj_version == IKEV2_MAJOR_VERSION)
			{
				/* unsupported exchange type for known version */
				return FAILED;
			}
			break;
	}
	if (this->initiator_spi == 0)
	{
#ifdef ME
		if (this->exchange_type != INFORMATIONAL)
			/* we allow zero spi for INFORMATIONAL exchanges,
			 * to allow connectivity checks */
#endif /* ME */
		{
			return FAILED;
		}
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_ike_header_t *this, encoding_rule_t **rules)
{
	*rules = encodings;
	return countof(encodings);
}

METHOD(payload_t, get_header_length, int,
	private_ike_header_t *this)
{
	return IKE_HEADER_LENGTH;
}

METHOD(payload_t, get_type, payload_type_t,
	private_ike_header_t *this)
{
	return PL_HEADER;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_ike_header_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_ike_header_t *this, payload_type_t type)
{
	this->next_payload = type;
}

METHOD(payload_t, get_length, size_t,
	private_ike_header_t *this)
{
	return this->length;
}

METHOD(ike_header_t, get_initiator_spi, uint64_t,
	private_ike_header_t *this)
{
	return this->initiator_spi;
}

METHOD(ike_header_t, set_initiator_spi, void,
	private_ike_header_t *this, uint64_t initiator_spi)
{
	this->initiator_spi = initiator_spi;
}

METHOD(ike_header_t, get_responder_spi, uint64_t,
	private_ike_header_t *this)
{
	return this->responder_spi;
}

METHOD(ike_header_t, set_responder_spi, void,
	private_ike_header_t *this, uint64_t responder_spi)
{
	this->responder_spi = responder_spi;
}

METHOD(ike_header_t, get_maj_version, uint8_t,
	private_ike_header_t *this)
{
	return this->maj_version;
}

METHOD(ike_header_t, set_maj_version, void,
	private_ike_header_t *this, uint8_t major)
{
	this->maj_version = major;
}

METHOD(ike_header_t, get_min_version, uint8_t,
	private_ike_header_t *this)
{
	return this->min_version;
}

METHOD(ike_header_t, set_min_version, void,
	private_ike_header_t *this, uint8_t minor)
{
	this->min_version = minor;
}

METHOD(ike_header_t, get_response_flag, bool,
	private_ike_header_t *this)
{
	return this->flags.response;
}

METHOD(ike_header_t, set_response_flag, void,
	private_ike_header_t *this, bool response)
{
	this->flags.response = response;
}

METHOD(ike_header_t, get_version_flag, bool,
	private_ike_header_t *this)
{
	return this->flags.version;
}

METHOD(ike_header_t, set_version_flag, void,
	private_ike_header_t *this, bool version)
{
	this->flags.version = version;
}

METHOD(ike_header_t, get_initiator_flag, bool,
	private_ike_header_t *this)
{
	return this->flags.initiator;
}

METHOD(ike_header_t, set_initiator_flag, void,
	private_ike_header_t *this, bool initiator)
{
	this->flags.initiator = initiator;
}

METHOD(ike_header_t, get_encryption_flag, bool,
	private_ike_header_t *this)
{
	return this->flags.encryption;
}

METHOD(ike_header_t, set_encryption_flag, void,
	private_ike_header_t *this, bool encryption)
{
	this->flags.encryption = encryption;
}


METHOD(ike_header_t, get_commit_flag, bool,
	private_ike_header_t *this)
{
	return this->flags.commit;
}

METHOD(ike_header_t, set_commit_flag, void,
	private_ike_header_t *this, bool commit)
{
	this->flags.commit = commit;
}

METHOD(ike_header_t, get_authonly_flag, bool,
	private_ike_header_t *this)
{
	return this->flags.authonly;
}

METHOD(ike_header_t, set_authonly_flag, void,
	private_ike_header_t *this, bool authonly)
{
	this->flags.authonly = authonly;
}

METHOD(ike_header_t, get_exchange_type, uint8_t,
	private_ike_header_t *this)
{
	return this->exchange_type;
}

METHOD(ike_header_t, set_exchange_type, void,
	private_ike_header_t *this, uint8_t exchange_type)
{
	this->exchange_type = exchange_type;
}

METHOD(ike_header_t, get_message_id, uint32_t,
	private_ike_header_t *this)
{
	return this->message_id;
}

METHOD(ike_header_t, set_message_id, void,
	private_ike_header_t *this, uint32_t message_id)
{
	this->message_id = message_id;
}

METHOD2(payload_t, ike_header_t, destroy, void,
	private_ike_header_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ike_header_t *ike_header_create()
{
	private_ike_header_t *this;

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
			.get_initiator_spi = _get_initiator_spi,
			.set_initiator_spi = _set_initiator_spi,
			.get_responder_spi = _get_responder_spi,
			.set_responder_spi = _set_responder_spi,
			.get_maj_version = _get_maj_version,
			.set_maj_version = _set_maj_version,
			.get_min_version = _get_min_version,
			.set_min_version = _set_min_version,
			.get_response_flag = _get_response_flag,
			.set_response_flag = _set_response_flag,
			.get_version_flag = _get_version_flag,
			.set_version_flag = _set_version_flag,
			.get_initiator_flag = _get_initiator_flag,
			.set_initiator_flag = _set_initiator_flag,
			.get_encryption_flag = _get_encryption_flag,
			.set_encryption_flag = _set_encryption_flag,
			.get_commit_flag = _get_commit_flag,
			.set_commit_flag = _set_commit_flag,
			.get_authonly_flag = _get_authonly_flag,
			.set_authonly_flag = _set_authonly_flag,
			.get_exchange_type = _get_exchange_type,
			.set_exchange_type = _set_exchange_type,
			.get_message_id = _get_message_id,
			.set_message_id = _set_message_id,
			.destroy = _destroy,
		},
		.length = IKE_HEADER_LENGTH,
		.exchange_type = EXCHANGE_TYPE_UNDEFINED,
	);

	return &this->public;
}

/*
 * Described in header.
 */
ike_header_t *ike_header_create_version(int major, int minor)
{
	ike_header_t *this = ike_header_create();

	this->set_maj_version(this, major);
	this->set_min_version(this, minor);
	if (major == IKEV2_MAJOR_VERSION)
	{
		this->set_initiator_flag(this, TRUE);
	}
	return this;
}
