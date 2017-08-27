/*
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2006-2008 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
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

#include "notify_payload.h"

#include <daemon.h>
#include <encoding/payloads/encodings.h>
#include <crypto/hashers/hasher.h>

ENUM_BEGIN(notify_type_names, UNSUPPORTED_CRITICAL_PAYLOAD, UNSUPPORTED_CRITICAL_PAYLOAD,
	"UNSUPPORTED_CRITICAL_PAYLOAD");
ENUM_NEXT(notify_type_names, INVALID_IKE_SPI, INVALID_MAJOR_VERSION, UNSUPPORTED_CRITICAL_PAYLOAD,
	"INVALID_IKE_SPI",
	"INVALID_MAJOR_VERSION");
ENUM_NEXT(notify_type_names, INVALID_SYNTAX, INVALID_SYNTAX, INVALID_MAJOR_VERSION,
	"INVALID_SYNTAX");
ENUM_NEXT(notify_type_names, INVALID_MESSAGE_ID, INVALID_MESSAGE_ID, INVALID_SYNTAX,
	"INVALID_MESSAGE_ID");
ENUM_NEXT(notify_type_names, INVALID_SPI, INVALID_SPI, INVALID_MESSAGE_ID,
	"INVALID_SPI");
ENUM_NEXT(notify_type_names, ATTRIBUTES_NOT_SUPPORTED, NO_PROPOSAL_CHOSEN, INVALID_SPI,
	"ATTRIBUTES_NOT_SUPPORTED",
	"NO_PROPOSAL_CHOSEN");
ENUM_NEXT(notify_type_names, PAYLOAD_MALFORMED, AUTHENTICATION_FAILED, NO_PROPOSAL_CHOSEN,
	"PAYLOAD_MALFORMED",
	"INVALID_KE_PAYLOAD",
	"INVALID_ID_INFORMATION",
	"INVALID_CERT_ENCODING",
	"INVALID_CERTIFICATE",
	"CERT_TYPE_UNSUPPORTED",
	"INVALID_CERT_AUTHORITY",
	"INVALID_HASH_INFORMATION",
	"AUTHENTICATION_FAILED");
ENUM_NEXT(notify_type_names, SINGLE_PAIR_REQUIRED, CHILD_SA_NOT_FOUND, AUTHENTICATION_FAILED,
	"SINGLE_PAIR_REQUIRED",
	"NO_ADDITIONAL_SAS",
	"INTERNAL_ADDRESS_FAILURE",
	"FAILED_CP_REQUIRED",
	"TS_UNACCEPTABLE",
	"INVALID_SELECTORS",
	"UNACCEPTABLE_ADDRESSES",
	"UNEXPECTED_NAT_DETECTED",
	"USE_ASSIGNED_HoA",
	"TEMPORARY_FAILURE",
	"CHILD_SA_NOT_FOUND");
ENUM_NEXT(notify_type_names, ME_CONNECT_FAILED, ME_CONNECT_FAILED, CHILD_SA_NOT_FOUND,
	"ME_CONNECT_FAILED");
ENUM_NEXT(notify_type_names, MS_NOTIFY_STATUS, MS_NOTIFY_STATUS, ME_CONNECT_FAILED,
	"MS_NOTIFY_STATUS");
ENUM_NEXT(notify_type_names, INITIAL_CONTACT, FRAGMENTATION_SUPPORTED, MS_NOTIFY_STATUS,
	"INITIAL_CONTACT",
	"SET_WINDOW_SIZE",
	"ADDITIONAL_TS_POSSIBLE",
	"IPCOMP_SUPPORTED",
	"NAT_DETECTION_SOURCE_IP",
	"NAT_DETECTION_DESTINATION_IP",
	"COOKIE",
	"USE_TRANSPORT_MODE",
	"HTTP_CERT_LOOKUP_SUPPORTED",
	"REKEY_SA",
	"ESP_TFC_PADDING_NOT_SUPPORTED",
	"NON_FIRST_FRAGMENTS_ALSO",
	"MOBIKE_SUPPORTED",
	"ADDITIONAL_IP4_ADDRESS",
	"ADDITIONAL_IP6_ADDRESS",
	"NO_ADDITIONAL_ADDRESSES",
	"UPDATE_SA_ADDRESSES",
	"COOKIE2",
	"NO_NATS_ALLOWED",
	"AUTH_LIFETIME",
	"MULTIPLE_AUTH_SUPPORTED",
	"ANOTHER_AUTH_FOLLOWS",
	"REDIRECT_SUPPORTED",
	"REDIRECT",
	"REDIRECTED_FROM",
	"TICKET_LT_OPAQUE",
	"TICKET_REQUEST",
	"TICKET_ACK",
	"TICKET_NACK",
	"TICKET_OPAQUE",
	"LINK_ID",
	"USE_WESP_MODE",
	"ROHC_SUPPORTED",
	"EAP_ONLY_AUTHENTICATION",
	"CHILDLESS_IKEV2_SUPPORTED",
	"QUICK_CRASH_DETECTION",
	"IKEV2_MESSAGE_ID_SYNC_SUPPORTED",
	"IKEV2_REPLAY_COUNTER_SYNC_SUPPORTED",
	"IKEV2_MESSAGE_ID_SYNC",
	"IPSEC_REPLAY_COUNTER_SYNC",
	"SECURE PASSWORD_METHOD",
	"PSK_PERSIST",
	"PSK_CONFIRM",
	"ERX_SUPPORTED",
	"IFOM_CAPABILITY",
	"SENDER_REQUEST_ID",
	"FRAGMENTATION_SUPPORTED");
ENUM_NEXT(notify_type_names, INITIAL_CONTACT_IKEV1, INITIAL_CONTACT_IKEV1, FRAGMENTATION_SUPPORTED,
	"INITIAL_CONTACT");
ENUM_NEXT(notify_type_names, DPD_R_U_THERE, DPD_R_U_THERE_ACK, INITIAL_CONTACT_IKEV1,
	"DPD_R_U_THERE",
	"DPD_R_U_THERE_ACK");
ENUM_NEXT(notify_type_names, UNITY_LOAD_BALANCE, UNITY_LOAD_BALANCE, DPD_R_U_THERE_ACK,
	"UNITY_LOAD_BALANCE");
ENUM_NEXT(notify_type_names, USE_BEET_MODE, USE_BEET_MODE, UNITY_LOAD_BALANCE,
	"USE_BEET_MODE");
ENUM_NEXT(notify_type_names, ME_MEDIATION, RADIUS_ATTRIBUTE, USE_BEET_MODE,
	"ME_MEDIATION",
	"ME_ENDPOINT",
	"ME_CALLBACK",
	"ME_CONNECTID",
	"ME_CONNECTKEY",
	"ME_CONNECTAUTH",
	"ME_RESPONSE",
	"RADIUS_ATTRIBUTE");
ENUM_END(notify_type_names, RADIUS_ATTRIBUTE);


ENUM_BEGIN(notify_type_short_names, UNSUPPORTED_CRITICAL_PAYLOAD, UNSUPPORTED_CRITICAL_PAYLOAD,
	"CRIT");
ENUM_NEXT(notify_type_short_names, INVALID_IKE_SPI, INVALID_MAJOR_VERSION, UNSUPPORTED_CRITICAL_PAYLOAD,
	"INVAL_IKE_SPI",
	"INVAL_MAJOR");
ENUM_NEXT(notify_type_short_names, INVALID_SYNTAX, INVALID_SYNTAX, INVALID_MAJOR_VERSION,
	"INVAL_SYN");
ENUM_NEXT(notify_type_short_names, INVALID_MESSAGE_ID, INVALID_MESSAGE_ID, INVALID_SYNTAX,
	"INVAL_MID");
ENUM_NEXT(notify_type_short_names, INVALID_SPI, INVALID_SPI, INVALID_MESSAGE_ID,
	"INVAL_SPI");
ENUM_NEXT(notify_type_short_names, ATTRIBUTES_NOT_SUPPORTED, NO_PROPOSAL_CHOSEN, INVALID_SPI,
	"ATTR_UNSUP",
	"NO_PROP");
ENUM_NEXT(notify_type_short_names, PAYLOAD_MALFORMED, AUTHENTICATION_FAILED, NO_PROPOSAL_CHOSEN,
	"PLD_MAL",
	"INVAL_KE",
	"INVAL_ID",
	"INVAL_CERTEN",
	"INVAL_CERT",
	"CERT_UNSUP",
	"INVAL_CA",
	"INVAL_HASH",
	"AUTH_FAILED");
ENUM_NEXT(notify_type_short_names, SINGLE_PAIR_REQUIRED, CHILD_SA_NOT_FOUND, AUTHENTICATION_FAILED,
	"SINGLE_PAIR",
	"NO_ADD_SAS",
	"INT_ADDR_FAIL",
	"FAIL_CP_REQ",
	"TS_UNACCEPT",
	"INVAL_SEL",
	"UNACCEPT_ADDR",
	"UNEXPECT_NAT",
	"ASSIGNED_HoA",
	"TEMP_FAIL",
	"NO_CHILD_SA");
ENUM_NEXT(notify_type_short_names, ME_CONNECT_FAILED, ME_CONNECT_FAILED, CHILD_SA_NOT_FOUND,
	"ME_CONN_FAIL");
ENUM_NEXT(notify_type_short_names, MS_NOTIFY_STATUS, MS_NOTIFY_STATUS, ME_CONNECT_FAILED,
	"MS_STATUS");
ENUM_NEXT(notify_type_short_names, INITIAL_CONTACT, FRAGMENTATION_SUPPORTED, MS_NOTIFY_STATUS,
	"INIT_CONTACT",
	"SET_WINSIZE",
	"ADD_TS_POSS",
	"IPCOMP_SUP",
	"NATD_S_IP",
	"NATD_D_IP",
	"COOKIE",
	"USE_TRANSP",
	"HTTP_CERT_LOOK",
	"REKEY_SA",
	"ESP_TFC_PAD_N",
	"NON_FIRST_FRAG",
	"MOBIKE_SUP",
	"ADD_4_ADDR",
	"ADD_6_ADDR",
	"NO_ADD_ADDR",
	"UPD_SA_ADDR",
	"COOKIE2",
	"NO_NATS",
	"AUTH_LFT",
	"MULT_AUTH",
	"AUTH_FOLLOWS",
	"REDIR_SUP",
	"REDIR",
	"REDIR_FROM",
	"TKT_LT_OPAK",
	"TKT_REQ",
	"TKT_ACK",
	"TKT_NACK",
	"TKT_OPAK",
	"LINK_ID",
	"WESP_MODE",
	"ROHC_SUP",
	"EAP_ONLY",
	"CHDLESS_SUP",
	"CRASH_DET",
	"MSG_ID_SYN_SUP",
	"RPL_CTR_SYN_SUP",
	"MSG_ID_SYN",
	"RPL_CTR_SYN",
	"SEC_PASSWD",
	"PSK_PST",
	"PSK_CFM",
	"ERX_SUP",
	"IFOM_CAP",
	"SENDER_REQ_ID",
	"FRAG_SUP");
ENUM_NEXT(notify_type_short_names, INITIAL_CONTACT_IKEV1, INITIAL_CONTACT_IKEV1, FRAGMENTATION_SUPPORTED,
	"INITIAL_CONTACT");
ENUM_NEXT(notify_type_short_names, DPD_R_U_THERE, DPD_R_U_THERE_ACK, INITIAL_CONTACT_IKEV1,
	"DPD",
	"DPD_ACK");
ENUM_NEXT(notify_type_short_names, UNITY_LOAD_BALANCE, UNITY_LOAD_BALANCE, DPD_R_U_THERE_ACK,
	"UNITY_LB");
ENUM_NEXT(notify_type_short_names, USE_BEET_MODE, USE_BEET_MODE, UNITY_LOAD_BALANCE,
	"BEET_MODE");
ENUM_NEXT(notify_type_short_names, ME_MEDIATION, RADIUS_ATTRIBUTE, USE_BEET_MODE,
	"ME_MED",
	"ME_EP",
	"ME_CB",
	"ME_CID",
	"ME_CKEY",
	"ME_CAUTH",
	"ME_R",
	"RADIUS");
ENUM_END(notify_type_short_names, RADIUS_ATTRIBUTE);


typedef struct private_notify_payload_t private_notify_payload_t;

/**
 * Private data of an notify_payload_t object.
 */
struct private_notify_payload_t {

	/**
	 * Public notify_payload_t interface.
	 */
	notify_payload_t public;

	/**
	 * Next payload type.
	 */
	u_int8_t  next_payload;

	/**
	 * Critical flag.
	 */
	bool critical;

	/**
	 * reserved bits
	 */
	bool reserved[8];

	/**
	 * Length of this payload.
	 */
	u_int16_t payload_length;

	/**
	 * Domain of interpretation, IKEv1 only.
	 */
	u_int32_t doi;

	/**
	 * Protocol id.
	 */
	u_int8_t protocol_id;

	/**
	 * Spi size.
	 */
	u_int8_t spi_size;

	/**
	 * Notify message type.
	 */
	u_int16_t notify_type;

	/**
	 * Security parameter index (spi).
	 */
	chunk_t spi;

	/**
	 * Notification data.
	 */
	chunk_t notify_data;

	/**
	 * Type of payload, PLV2_NOTIFY or PLV1_NOTIFY
	 */
	payload_type_t type;
};

/**
 * Encoding rules for an IKEv2 notification payload
 */
static encoding_rule_t encodings_v2[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_notify_payload_t, next_payload)	},
	/* the critical bit */
	{ FLAG,				offsetof(private_notify_payload_t, critical)		},
	/* 7 Bit reserved bits, nowhere stored */
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[0])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[1])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[2])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[3])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[4])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[5])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[6])		},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_notify_payload_t, payload_length)	},
	/* Protocol ID as 8 bit field*/
	{ U_INT_8,			offsetof(private_notify_payload_t, protocol_id)		},
	/* SPI Size as 8 bit field*/
	{ SPI_SIZE,			offsetof(private_notify_payload_t, spi_size)		},
	/* Notify message type as 16 bit field*/
	{ U_INT_16,			offsetof(private_notify_payload_t, notify_type)		},
	/* SPI as variable length field*/
	{ SPI,				offsetof(private_notify_payload_t, spi)				},
	/* Key Exchange Data is from variable size */
	{ CHUNK_DATA,		offsetof(private_notify_payload_t, notify_data)		},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !  Protocol ID  !   SPI Size    !      Notify Message Type      !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                Security Parameter Index (SPI)                 ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                       Notification Data                       ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
/**
 * Encoding rules for an IKEv1 notification payload
 */
static encoding_rule_t encodings_v1[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_notify_payload_t, next_payload)	},
	/* 8 reserved bits */
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[0])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[1])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[2])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[3])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[4])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[5])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[6])		},
	{ RESERVED_BIT,		offsetof(private_notify_payload_t, reserved[7])		},
	/* Length of the whole payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_notify_payload_t, payload_length)	},
	/* DOI as  32 bit field*/
	{ U_INT_32,			offsetof(private_notify_payload_t, doi)				},
	/* Protocol ID as 8 bit field*/
	{ U_INT_8,			offsetof(private_notify_payload_t, protocol_id)		},
	/* SPI Size as 8 bit field*/
	{ SPI_SIZE,			offsetof(private_notify_payload_t, spi_size)		},
	/* Notify message type as 16 bit field*/
	{ U_INT_16,			offsetof(private_notify_payload_t, notify_type)		},
	/* SPI as variable length field*/
	{ SPI,				offsetof(private_notify_payload_t, spi)				},
	/* Key Exchange Data is from variable size */
	{ CHUNK_DATA,		offsetof(private_notify_payload_t, notify_data)		},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !    RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                             DOI                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !  Protocol ID  !   SPI Size    !      Notify Message Type      !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                Security Parameter Index (SPI)                 ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                                                               !
      ~                       Notification Data                       ~
      !                                                               !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/


METHOD(payload_t, verify, status_t,
	private_notify_payload_t *this)
{
	bool bad_length = FALSE;

	switch (this->protocol_id)
	{
		case PROTO_NONE:
		case PROTO_IKE:
		case PROTO_AH:
		case PROTO_ESP:
			break;
		default:
			DBG1(DBG_ENC, "Unknown protocol (%d)", this->protocol_id);
			return FAILED;
	}

	switch (this->notify_type)
	{
		case INVALID_KE_PAYLOAD:
		{
			if (this->type == PLV2_NOTIFY && this->notify_data.len != 2)
			{
				bad_length = TRUE;
			}
			break;
		}
		case NAT_DETECTION_SOURCE_IP:
		case NAT_DETECTION_DESTINATION_IP:
		case ME_CONNECTAUTH:
		{
			if (this->notify_data.len != HASH_SIZE_SHA1)
			{
				bad_length = TRUE;
			}
			break;
		}
		case INVALID_SYNTAX:
		case INVALID_MAJOR_VERSION:
		case NO_PROPOSAL_CHOSEN:
		{
			if (this->type == PLV2_NOTIFY && this->notify_data.len != 0)
			{
				bad_length = TRUE;
			}
			break;
		}
		case ADDITIONAL_IP4_ADDRESS:
		{
			if (this->notify_data.len != 4)
			{
				bad_length = TRUE;
			}
			break;
		}
		case ADDITIONAL_IP6_ADDRESS:
		{
			if (this->notify_data.len != 16)
			{
				bad_length = TRUE;
			}
			break;
		}
		case AUTH_LIFETIME:
		{
			if (this->notify_data.len != 4)
			{
				bad_length = TRUE;
			}
			break;
		}
		case IPCOMP_SUPPORTED:
		{
			if (this->notify_data.len != 3)
			{
				bad_length = TRUE;
			}
			break;
		}
		case ME_ENDPOINT:
			if (this->notify_data.len != 8 &&
				this->notify_data.len != 12 &&
				this->notify_data.len != 24)
			{
				bad_length = TRUE;
			}
			break;
		case ME_CONNECTID:
			if (this->notify_data.len < 4 ||
				this->notify_data.len > 16)
			{
				bad_length = TRUE;
			}
			break;
		case ME_CONNECTKEY:
			if (this->notify_data.len < 16 ||
				this->notify_data.len > 32)
			{
				bad_length = TRUE;
			}
			break;
		case DPD_R_U_THERE:
		case DPD_R_U_THERE_ACK:
			if (this->notify_data.len != 4)
			{
				bad_length = TRUE;
			}
			break;
		default:
			/* TODO: verify */
			break;
	}
	if (bad_length)
	{
		DBG1(DBG_ENC, "invalid notify data length for %N (%d)",
			 notify_type_names, this->notify_type,
			 this->notify_data.len);
		return FAILED;
	}
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_notify_payload_t *this, encoding_rule_t **rules)
{
	if (this->type == PLV2_NOTIFY)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_notify_payload_t *this)
{
	if (this->type == PLV2_NOTIFY)
	{
		return 8 + this->spi_size;
	}
	return 12 + this->spi_size;
}

METHOD(payload_t, get_type, payload_type_t,
	private_notify_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_notify_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_notify_payload_t *this, payload_type_t type)
{
	this->next_payload = type;
}

/**
 * recompute the payloads length.
 */
static void compute_length(private_notify_payload_t *this)
{
	this->payload_length = get_header_length(this) + this->notify_data.len;
}

METHOD(payload_t, get_length, size_t,
	private_notify_payload_t *this)
{
	return this->payload_length;
}

METHOD(notify_payload_t, get_protocol_id, u_int8_t,
	private_notify_payload_t *this)
{
	return this->protocol_id;
}

METHOD(notify_payload_t, set_protocol_id, void,
	private_notify_payload_t *this, u_int8_t protocol_id)
{
	this->protocol_id = protocol_id;
}

METHOD(notify_payload_t, get_notify_type, notify_type_t,
	private_notify_payload_t *this)
{
	return this->notify_type;
}

METHOD(notify_payload_t, set_notify_type, void,
	private_notify_payload_t *this, notify_type_t notify_type)
{
	this->notify_type = notify_type;
}

METHOD(notify_payload_t, get_spi, u_int32_t,
	private_notify_payload_t *this)
{
	switch (this->protocol_id)
	{
		case PROTO_AH:
		case PROTO_ESP:
			if (this->spi.len == 4)
			{
				return *((u_int32_t*)this->spi.ptr);
			}
		default:
			break;
	}
	return 0;
}

METHOD(notify_payload_t, set_spi, void,
	private_notify_payload_t *this, u_int32_t spi)
{
	chunk_free(&this->spi);
	switch (this->protocol_id)
	{
		case PROTO_AH:
		case PROTO_ESP:
			this->spi = chunk_alloc(4);
			*((u_int32_t*)this->spi.ptr) = spi;
			break;
		default:
			break;
	}
	this->spi_size = this->spi.len;
	compute_length(this);
}

METHOD(notify_payload_t, get_spi_data, chunk_t,
	private_notify_payload_t *this)
{
	switch (this->protocol_id)
	{
		case PROTO_IKE:
			if (this->spi.len == 16)
			{
				return this->spi;
			}
		default:
			break;
	}
	return chunk_empty;
}

METHOD(notify_payload_t, set_spi_data, void,
	private_notify_payload_t *this, chunk_t spi)
{
	chunk_free(&this->spi);
	switch (this->protocol_id)
	{
		case PROTO_IKE:
			this->spi = chunk_clone(spi);
		default:
			break;
	}
	this->spi_size = this->spi.len;
	compute_length(this);
}

METHOD(notify_payload_t, get_notification_data, chunk_t,
	private_notify_payload_t *this)
{
	return this->notify_data;
}

METHOD(notify_payload_t, set_notification_data, void,
	private_notify_payload_t *this, chunk_t data)
{
	free(this->notify_data.ptr);
	this->notify_data = chunk_clone(data);
	compute_length(this);
}

METHOD2(payload_t, notify_payload_t, destroy, void,
	private_notify_payload_t *this)
{
	free(this->notify_data.ptr);
	free(this->spi.ptr);
	free(this);
}

/*
 * Described in header
 */
notify_payload_t *notify_payload_create(payload_type_t type)
{
	private_notify_payload_t *this;

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
			.get_protocol_id = _get_protocol_id,
			.set_protocol_id  = _set_protocol_id,
			.get_notify_type = _get_notify_type,
			.set_notify_type = _set_notify_type,
			.get_spi = _get_spi,
			.set_spi = _set_spi,
			.get_spi_data = _get_spi_data,
			.set_spi_data = _set_spi_data,
			.get_notification_data = _get_notification_data,
			.set_notification_data = _set_notification_data,
			.destroy = _destroy,
		},
		.doi = IKEV1_DOI_IPSEC,
		.next_payload = PL_NONE,
		.type = type,
	);
	compute_length(this);
	return &this->public;
}

/*
 * Described in header.
 */
notify_payload_t *notify_payload_create_from_protocol_and_type(
			payload_type_t type, protocol_id_t protocol, notify_type_t notify)
{
	notify_payload_t *this = notify_payload_create(type);

	this->set_notify_type(this, notify);
	this->set_protocol_id(this, protocol);

	return this;
}
