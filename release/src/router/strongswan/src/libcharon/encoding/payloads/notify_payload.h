/*
 * Copyright (C) 2006-2018 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
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

/**
 * @defgroup notify_payload notify_payload
 * @{ @ingroup payloads
 */

#ifndef NOTIFY_PAYLOAD_H_
#define NOTIFY_PAYLOAD_H_

typedef enum notify_type_t notify_type_t;
typedef struct notify_payload_t notify_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/proposal_substructure.h>
#include <collections/linked_list.h>

/**
 * Notify message types for IKEv2, and a subset for IKEv1.
 */
enum notify_type_t {
	/* notify error messages */
	UNSUPPORTED_CRITICAL_PAYLOAD = 1,
	/* IKEv1 alias */
	INVALID_PAYLOAD_TYPE = 1,
	INVALID_IKE_SPI = 4,
	INVALID_MAJOR_VERSION = 5,
	INVALID_SYNTAX = 7,
	/* IKEv1 alias */
	INVALID_EXCHANGE_TYPE = 7,
	INVALID_MESSAGE_ID = 9,
	INVALID_SPI = 11,
	/* IKEv1 only */
	ATTRIBUTES_NOT_SUPPORTED = 13,
	/* IKEv1 alias */
	NO_PROPOSAL_CHOSEN = 14,
	/* IKEv1 only */
	PAYLOAD_MALFORMED = 16,
	INVALID_KE_PAYLOAD = 17,
	/* IKEv1 alias */
	INVALID_KEY_INFORMATION = 17,
	/* IKEv1 only */
	INVALID_ID_INFORMATION = 18,
	INVALID_CERT_ENCODING = 19,
	INVALID_CERTIFICATE = 20,
	CERT_TYPE_UNSUPPORTED = 21,
	INVALID_CERT_AUTHORITY = 22,
	INVALID_HASH_INFORMATION = 23,
	AUTHENTICATION_FAILED = 24,
	SINGLE_PAIR_REQUIRED = 34,
	NO_ADDITIONAL_SAS = 35,
	INTERNAL_ADDRESS_FAILURE = 36,
	FAILED_CP_REQUIRED = 37,
	TS_UNACCEPTABLE = 38,
	INVALID_SELECTORS = 39,
	/* mobile extension, RFC 4555 */
	UNACCEPTABLE_ADDRESSES = 40,
	UNEXPECTED_NAT_DETECTED = 41,
	/* mobile IPv6 bootstrapping, RFC 5026 */
	USE_ASSIGNED_HoA = 42,
	/* IKEv2 RFC 5996 */
	TEMPORARY_FAILURE = 43,
	CHILD_SA_NOT_FOUND = 44,

	/* IKE-ME, private use */
	ME_CONNECT_FAILED = 8192,

	/* Windows error code */
	MS_NOTIFY_STATUS = 12345,

	/* notify status messages */
	INITIAL_CONTACT = 16384,
	SET_WINDOW_SIZE = 16385,
	ADDITIONAL_TS_POSSIBLE = 16386,
	IPCOMP_SUPPORTED = 16387,
	NAT_DETECTION_SOURCE_IP = 16388,
	NAT_DETECTION_DESTINATION_IP = 16389,
	COOKIE = 16390,
	USE_TRANSPORT_MODE = 16391,
	HTTP_CERT_LOOKUP_SUPPORTED = 16392,
	REKEY_SA = 16393,
	ESP_TFC_PADDING_NOT_SUPPORTED = 16394,
	NON_FIRST_FRAGMENTS_ALSO = 16395,
	/* mobike extension, RFC4555 */
	MOBIKE_SUPPORTED = 16396,
	ADDITIONAL_IP4_ADDRESS = 16397,
	ADDITIONAL_IP6_ADDRESS = 16398,
	NO_ADDITIONAL_ADDRESSES = 16399,
	UPDATE_SA_ADDRESSES = 16400,
	COOKIE2 = 16401,
	NO_NATS_ALLOWED = 16402,
	/* repeated authentication extension, RFC4478 */
	AUTH_LIFETIME = 16403,
	/* multiple authentication exchanges, RFC 4739 */
	MULTIPLE_AUTH_SUPPORTED = 16404,
	ANOTHER_AUTH_FOLLOWS = 16405,
	/* redirect mechanism, RFC 5685 */
	REDIRECT_SUPPORTED = 16406,
	REDIRECT = 16407,
	REDIRECTED_FROM = 16408,
	/* session resumption, RFC 5723 */
	TICKET_LT_OPAQUE = 16409,
	TICKET_REQUEST = 16410,
	TICKET_ACK = 16411,
	TICKET_NACK = 16412,
	TICKET_OPAQUE = 16413,
	/* IPv6 configuration, RFC 5739 */
	LINK_ID = 16414,
	/* wrapped esp, RFC 5840 */
	USE_WESP_MODE = 16415,
	/* robust header compression, RFC 5857 */
	ROHC_SUPPORTED = 16416,
	/* EAP-only authentication, RFC 5998 */
	EAP_ONLY_AUTHENTICATION = 16417,
	/* Childless initiation of IKEv2 SA, RFC 6023 */
	CHILDLESS_IKEV2_SUPPORTED = 16418,
	/* Quick crash detection for IKE, RFC 6290 */
	QUICK_CRASH_DETECTION = 16419,
	/* High availability of IKEv2/IPsec, RFC 6311 */
	IKEV2_MESSAGE_ID_SYNC_SUPPORTED = 16420,
	IKEV2_REPLAY_COUNTER_SYNC_SUPPORTED = 16421,
	IKEV2_MESSAGE_ID_SYNC = 16422,
	IPSEC_REPLAY_COUNTER_SYNC = 16423,
	/* Secure password methods, RFC 6467 */
	SECURE_PASSWORD_METHOD = 16424,
	/* PACE, RFC 6631 */
	PSK_PERSIST = 16425,
	PSK_CONFIRM = 16426,
	/* EAP Re-authentication Extension, RFC 6867 */
	ERX_SUPPORTED = 16427,
	/* IFOM capability, 3GPP TS 24.303, annex B.2 */
	IFOM_CAPABILITY = 16428,
	/* SENDER_REQUEST_ID (draft-yeung-g-ikev2) */
	SENDER_REQUEST_ID = 16429,
	/* IKEv2 fragmentation supported, RFC 7383 */
	FRAGMENTATION_SUPPORTED = 16430,
	/* Signature Hash Algorithms, RFC 7427 */
	SIGNATURE_HASH_ALGORITHMS = 16431,
	/* Use Postquantum Preshared Key (draft-ietf-ipsecme-qr-ikev2) */
	USE_PPK = 16435,
	/* Postquantum Preshared Key Identity (draft-ietf-ipsecme-qr-ikev2) */
	PPK_IDENTITY = 16436,
	/* No Postquantum Preshared Key Auth (draft-ietf-ipsecme-qr-ikev2) */
	NO_PPK_AUTH = 16437,
	/* IKEv1 initial contact */
	INITIAL_CONTACT_IKEV1 = 24578,
	/* IKEv1 DPD */
	DPD_R_U_THERE = 36136,
	DPD_R_U_THERE_ACK = 36137,
	/* IKEv1 Cisco High Availability */
	UNITY_LOAD_BALANCE = 40501,
	/* BEET mode, not even a draft yet. private use */
	USE_BEET_MODE = 40961,
	/* IKE-ME, private use */
	ME_MEDIATION = 40962,
	ME_ENDPOINT = 40963,
	ME_CALLBACK = 40964,
	ME_CONNECTID = 40965,
	ME_CONNECTKEY = 40966,
	ME_CONNECTAUTH = 40967,
	ME_RESPONSE = 40968,
	/* RADIUS attribute received/to send to a AAA backend */
	RADIUS_ATTRIBUTE = 40969,
};

/**
 * enum name for notify_type_t.
 */
extern enum_name_t *notify_type_names;

/**
 * enum name for notify_type_t (shorter strings).
 */
extern enum_name_t *notify_type_short_names;

/**
 * Class representing an IKEv2-Notify Payload.
 *
 * The Notify Payload format is described in Draft section 3.10.
 */
struct notify_payload_t {
	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Gets the protocol id of this payload.
	 *
	 * @return			protocol id of this payload
	 */
	uint8_t (*get_protocol_id) (notify_payload_t *this);

	/**
	 * Sets the protocol id of this payload.
	 *
	 * @param protocol_id	protocol id to set
	 */
	void (*set_protocol_id) (notify_payload_t *this, uint8_t protocol_id);

	/**
	 * Gets the notify message type of this payload.
	 *
	 * @return			notify message type of this payload
	 */
	notify_type_t (*get_notify_type) (notify_payload_t *this);

	/**
	 * Sets notify message type of this payload.
	 *
	 * @param type		notify message type to set
	 */
	void (*set_notify_type) (notify_payload_t *this, notify_type_t type);

	/**
	 * Returns the currently set spi of this payload.
	 *
	 * This is only valid for notifys with protocol AH|ESP
	 *
	 * @return		SPI value
	 */
	uint32_t (*get_spi) (notify_payload_t *this);

	/**
	 * Sets the spi of this payload.
	 *
	 * This is only valid for notifys with protocol AH|ESP
	 *
	 * @param spi	SPI value
	 */
	void (*set_spi) (notify_payload_t *this, uint32_t spi);

	/**
	 * Returns the currently set spi of this payload.
	 *
	 * This is only valid for notifys with protocol ISAKMP
	 *
	 * @return		SPI value
	 */
	chunk_t (*get_spi_data) (notify_payload_t *this);

	/**
	 * Sets the spi of this payload.
	 *
	 * This is only valid for notifys with protocol ISAKMP
	 *
	 * @param spi	SPI value
	 */
	void (*set_spi_data) (notify_payload_t *this, chunk_t spi);

	/**
	 * Returns the currently set notification data of payload.
	 *
	 * Returned data are not copied.
	 *
	 * @return		chunk_t pointing to the value
	 */
	chunk_t (*get_notification_data) (notify_payload_t *this);

	/**
	 * Sets the notification data of this payload.
	 *
	 * @warning Value is getting copied.
	 *
	 * @param notification_data	chunk_t pointing to the value to set
	 */
	void (*set_notification_data) (notify_payload_t *this,
								   chunk_t notification_data);

	/**
	 * Destroys an notify_payload_t object.
	 */
	void (*destroy) (notify_payload_t *this);
};

/**
 * Creates an empty notify_payload_t object
 *
 * @param type		payload type, PLV2_NOTIFY or PLV1_NOTIFY
 * @return			created notify_payload_t object
 */
notify_payload_t *notify_payload_create(payload_type_t type);

/**
 * Creates an notify_payload_t object of specific type for specific protocol id.
 *
 * @param type					payload type, PLV2_NOTIFY or PLV1_NOTIFY
 * @param protocol				protocol id (IKE, AH or ESP)
 * @param notify				type of notify
 * @return						notify_payload_t object
 */
notify_payload_t *notify_payload_create_from_protocol_and_type(
			payload_type_t type, protocol_id_t protocol, notify_type_t notify);

#endif /** NOTIFY_PAYLOAD_H_ @}*/
