/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup libradius libradius
 *
 * @addtogroup libradius
 * RADIUS protocol support library.
 *
 * @defgroup radius_message radius_message
 * @{ @ingroup libradius
 */

#ifndef RADIUS_MESSAGE_H_
#define RADIUS_MESSAGE_H_

#include <library.h>
#include <pen/pen.h>

#define MAX_RADIUS_ATTRIBUTE_SIZE	253

#define RADIUS_TUNNEL_TYPE_ESP		9

typedef struct radius_message_t radius_message_t;
typedef enum radius_message_code_t radius_message_code_t;
typedef enum radius_attribute_type_t radius_attribute_type_t;

/**
 * RADIUS Message Codes.
 */
enum radius_message_code_t {
	RMC_ACCESS_REQUEST = 1,
	RMC_ACCESS_ACCEPT = 2,
	RMC_ACCESS_REJECT = 3,
	RMC_ACCOUNTING_REQUEST = 4,
	RMC_ACCOUNTING_RESPONSE = 5,
	RMC_ACCESS_CHALLENGE = 11,
	RMC_DISCONNECT_REQUEST = 40,
	RMC_DISCONNECT_ACK = 41,
	RMC_DISCONNECT_NAK = 42,
	RMC_COA_REQUEST = 43,
	RMC_COA_ACK = 44,
	RMC_COA_NAK = 45,
};

/**
 * Enum names for radius_attribute_type_t.
 */
extern enum_name_t *radius_message_code_names;

/**
 * RADIUS Attribute Types.
 */
enum radius_attribute_type_t {
	RAT_USER_NAME = 1,
	RAT_USER_PASSWORD = 2,
	RAT_CHAP_PASSWORD = 3,
	RAT_NAS_IP_ADDRESS = 4,
	RAT_NAS_PORT = 5,
	RAT_SERVICE_TYPE = 6,
	RAT_FRAMED_PROTOCOL = 7,
	RAT_FRAMED_IP_ADDRESS = 8,
	RAT_FRAMED_IP_NETMASK = 9,
	RAT_FRAMED_ROUTING = 10,
	RAT_FILTER_ID = 11,
	RAT_FRAMED_MTU = 12,
	RAT_FRAMED_COMPRESSION = 13,
	RAT_LOGIN_IP_HOST = 14,
	RAT_LOGIN_SERVICE = 15,
	RAT_LOGIN_TCP_PORT = 16,
	RAT_REPLY_MESSAGE = 18,
	RAT_CALLBACK_NUMBER = 19,
	RAT_CALLBACK_ID = 20,
	RAT_FRAMED_ROUTE = 22,
	RAT_FRAMED_IPX_NETWORK = 23,
	RAT_STATE = 24,
	RAT_CLASS = 25,
	RAT_VENDOR_SPECIFIC = 26,
	RAT_SESSION_TIMEOUT = 27,
	RAT_IDLE_TIMEOUT = 28,
	RAT_TERMINATION_ACTION = 29,
	RAT_CALLED_STATION_ID = 30,
	RAT_CALLING_STATION_ID = 31,
	RAT_NAS_IDENTIFIER = 32,
	RAT_PROXY_STATE = 33,
	RAT_LOGIN_LAT_SERVICE = 34,
	RAT_LOGIN_LAT_NODE = 35,
	RAT_LOGIN_LAT_GROUP = 36,
	RAT_FRAMED_APPLETALK_LINK = 37,
	RAT_FRAMED_APPLETALK_NETWORK = 38,
	RAT_FRAMED_APPLETALK_ZONE = 39,
	RAT_ACCT_STATUS_TYPE = 40,
	RAT_ACCT_DELAY_TIME = 41,
	RAT_ACCT_INPUT_OCTETS = 42,
	RAT_ACCT_OUTPUT_OCTETS = 43,
	RAT_ACCT_SESSION_ID = 44,
	RAT_ACCT_AUTHENTIC = 45,
	RAT_ACCT_SESSION_TIME = 46,
	RAT_ACCT_INPUT_PACKETS = 47,
	RAT_ACCT_OUTPUT_PACKETS = 48,
	RAT_ACCT_TERMINATE_CAUSE = 49,
	RAT_ACCT_MULTI_SESSION_ID = 50,
	RAT_ACCT_LINK_COUNT = 51,
	RAT_ACCT_INPUT_GIGAWORDS = 52,
	RAT_ACCT_OUTPUT_GIGAWORDS = 53,
	RAT_EVENT_TIMESTAMP = 55,
	RAT_EGRESS_VLANID = 56,
	RAT_INGRESS_FILTERS = 57,
	RAT_EGRESS_VLAN_NAME = 58,
	RAT_USER_PRIORITY_TABLE = 59,
	RAT_CHAP_CHALLENGE = 60,
	RAT_NAS_PORT_TYPE = 61,
	RAT_PORT_LIMIT = 62,
	RAT_LOGIN_LAT_PORT = 63,
	RAT_TUNNEL_TYPE = 64,
	RAT_TUNNEL_MEDIUM_TYPE = 65,
	RAT_TUNNEL_CLIENT_ENDPOINT = 66,
	RAT_TUNNEL_SERVER_ENDPOINT = 67,
	RAT_ACCT_TUNNEL_CONNECTION = 68,
	RAT_TUNNEL_PASSWORD = 69,
	RAT_ARAP_PASSWORD = 70,
	RAT_ARAP_FEATURES = 71,
	RAT_ARAP_ZONE_ACCESS = 72,
	RAT_ARAP_SECURITY = 73,
	RAT_ARAP_SECURITY_DATA = 74,
	RAT_PASSWORD_RETRY = 75,
	RAT_PROMPT = 76,
	RAT_CONNECT_INFO = 77,
	RAT_CONFIGURATION_TOKEN = 78,
	RAT_EAP_MESSAGE = 79,
	RAT_MESSAGE_AUTHENTICATOR = 80,
	RAT_TUNNEL_PRIVATE_GROUP_ID = 81,
	RAT_TUNNEL_ASSIGNMENT_ID = 82,
	RAT_TUNNEL_PREFERENCE = 83,
	RAT_ARAP_CHALLENGE_RESPONSE = 84,
	RAT_ACCT_INTERIM_INTERVAL = 85,
	RAT_ACCT_TUNNEL_PACKETS_LOST = 86,
	RAT_NAS_PORT_ID = 87,
	RAT_FRAMED_POOL = 88,
	RAT_CUI = 89,
	RAT_TUNNEL_CLIENT_AUTH_ID = 90,
	RAT_TUNNEL_SERVER_AUTH_ID = 91,
	RAT_NAS_FILTER_RULE = 92,
	RAT_UNASSIGNED = 93,
	RAT_ORIGINATING_LINE_INFO = 94,
	RAT_NAS_IPV6_ADDRESS = 95,
	RAT_FRAMED_INTERFACE_ID = 96,
	RAT_FRAMED_IPV6_PREFIX = 97,
	RAT_LOGIN_IPV6_HOST = 98,
	RAT_FRAMED_IPV6_ROUTE = 99,
	RAT_FRAMED_IPV6_POOL = 100,
	RAT_ERROR_CAUSE = 101,
	RAT_EAP_KEY_NAME = 102,
	RAT_DIGEST_RESPONSE = 103,
	RAT_DIGEST_REALM = 104,
	RAT_DIGEST_NONCE = 105,
	RAT_DIGEST_RESPONSE_AUTH = 106,
	RAT_DIGEST_NEXTNONCE = 107,
	RAT_DIGEST_METHOD = 108,
	RAT_DIGEST_URI = 109,
	RAT_DIGEST_QOP = 110,
	RAT_DIGEST_ALGORITHM = 111,
	RAT_DIGEST_ENTITY_BODY_HASH = 112,
	RAT_DIGEST_CNONCE = 113,
	RAT_DIGEST_NONCE_COUNT = 114,
	RAT_DIGEST_USERNAME = 115,
	RAT_DIGEST_OPAQUE = 116,
	RAT_DIGEST_AUTH_PARAM = 117,
	RAT_DIGEST_AKA_AUTS = 118,
	RAT_DIGEST_DOMAIN = 119,
	RAT_DIGEST_STALE = 120,
	RAT_DIGEST_HA1 = 121,
	RAT_SIP_AOR = 122,
	RAT_DELEGATED_IPV6_PREFIX = 123,
	RAT_MIP6_FEATURE_VECTOR = 124,
	RAT_MIP6_HOME_LINK_PREFIX = 125,
	RAT_FRAMED_IPV6_ADDRESS = 168,
	RAT_FRAMED_IPV6_DNS_SERVER = 169,
	RAT_ROUTE_IPV6_INFORMATION = 170,
	RAT_DELEGATED_IPV6_PREFIX_POOL = 171,
	RAT_STATEFUL_IPV6_ADDRESS_POOL = 172,
};

/**
 * Enum names for radius_attribute_type_t.
 */
extern enum_name_t *radius_attribute_type_names;

/**
 * A RADIUS message, contains attributes.
 */
struct radius_message_t {

	/**
	 * Create an enumerator over contained RADIUS attributes.
	 *
	 * @return				enumerator over (int type, chunk_t data)
	 */
	enumerator_t* (*create_enumerator)(radius_message_t *this);

	/**
	 * Create an enumerator over contained RADIUS Vendor-ID attributes.
	 *
	 * This enumerator parses only vendor specific attributes in the format
	 * recommended in RFC2865.
	 *
	 * @return				enumerator over (int vendor, int type, chunk_t data)
	 */
	enumerator_t* (*create_vendor_enumerator)(radius_message_t *this);

	/**
	 * Add a RADIUS attribute to the message.
	 *
	 * @param type			type of attribute to add
	 * @param				attribute data, gets cloned
	 */
	void (*add)(radius_message_t *this, radius_attribute_type_t type,
				chunk_t data);

	/**
	 * Get the message type (code).
	 *
	 * @return				message code
	 */
	radius_message_code_t (*get_code)(radius_message_t *this);

	/**
	 * Get the message identifier.
	 *
	 * @return				message identifier
	 */
	uint8_t (*get_identifier)(radius_message_t *this);

	/**
	 * Set the message identifier.
	 *
	 * @param identifier	message identifier
	 */
	void (*set_identifier)(radius_message_t *this, uint8_t identifier);

	/**
	 * Get the 16 byte authenticator.
	 *
	 * @return				pointer to the Authenticator field
	 */
	uint8_t* (*get_authenticator)(radius_message_t *this);

	/**
	 * Get the RADIUS message in its encoded form.
	 *
	 * @return				chunk pointing to internal RADIUS message.
	 */
	chunk_t (*get_encoding)(radius_message_t *this);

	/**
	 * Calculate and add the Message-Authenticator attribute to the message.
	 *
	 * @param req_auth		16 byte Authenticator of request, or NULL
	 * @param secret		shared RADIUS secret
	 * @param signer		HMAC-MD5 signer with secret set
	 * @param hasher		MD5 hasher
	 * @param rng			RNG to create Request-Authenticator, NULL to omit
	 * @param msg_auth		calculate and add Message-Authenticator
	 * @return				TRUE if signed successfully
	 */
	bool (*sign)(radius_message_t *this, uint8_t *req_auth, chunk_t secret,
				 hasher_t *hasher, signer_t *signer, rng_t *rng, bool msg_auth);

	/**
	 * Verify the integrity of a received RADIUS message.
	 *
	 * @param req_auth		16 byte Authenticator of request, or NULL
	 * @param secret		shared RADIUS secret
	 * @param signer		HMAC-MD5 signer with secret set
	 * @param hasher		MD5 hasher
	 */
	bool (*verify)(radius_message_t *this, uint8_t *req_auth, chunk_t secret,
				   hasher_t *hasher, signer_t *signer);

	/**
	 * Perform RADIUS attribute en-/decryption.
	 *
	 * Performs en-/decryption by XOring the hash-extended secret into data,
	 * as specified in RFC 2865 5.2 and used by RFC 2548.
	 *
	 * @param salt			salt to append to message authenticator, if any
	 * @param in			data to en-/decrypt, multiple of HASH_SIZE_MD5
	 * @param out			en-/decrypted data, length equal to in
	 * @param secret		RADIUS secret
	 * @param hasher		MD5 hasher
	 * @return				TRUE if en-/decryption successful
	 */
	bool (*crypt)(radius_message_t *this, chunk_t salt, chunk_t in, chunk_t out,
				  chunk_t secret, hasher_t *hasher);

	/**
	 * Destroy the message.
	 */
	void (*destroy)(radius_message_t *this);
};

/**
 * Create an empty RADIUS message.
 *
 * @param code			request type
 * @return				radius_message_t object
 */
radius_message_t *radius_message_create(radius_message_code_t code);

/**
 * Parse and verify a received RADIUS message.
 *
 * @param data			received message data
 * @return				radius_message_t object, NULL if length invalid
 */
radius_message_t *radius_message_parse(chunk_t data);

/**
 * @}
 * @addtogroup libradius
 * @{
 *
 * Dummy libradius initialization function needed for integrity test
 */
void libradius_init(void);

#endif /** RADIUS_MESSAGE_H_ @}*/
