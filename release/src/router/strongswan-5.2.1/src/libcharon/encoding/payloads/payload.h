/*
 * Copyright (C) 2007 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
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

/**
 * @defgroup payload payload
 * @{ @ingroup payloads
 */

#ifndef PAYLOAD_H_
#define PAYLOAD_H_

typedef enum payload_type_t payload_type_t;
typedef struct payload_t payload_t;

#include <library.h>
#include <encoding/payloads/encodings.h>

/**
 * Domain of interpretation used by IPsec/IKEv1
 */
#define IKEV1_DOI_IPSEC 1

/**
 * Payload-Types of an IKE message.
 *
 * Header and substructures are also defined as
 * payload types with values from PRIVATE USE space.
 */
enum payload_type_t {

	/**
	 * End of payload list in next_payload
	 */
	PL_NONE = 0,

	/**
	 * The security association (SA) payload containing proposals.
	 */
	PLV1_SECURITY_ASSOCIATION = 1,

	/**
	 * The proposal payload, containing transforms.
	 */
	PLV1_PROPOSAL = 2,

	/**
	 * The transform payload.
	 */
	PLV1_TRANSFORM = 3,

	/**
	 * The key exchange (KE) payload containing diffie-hellman values.
	 */
	PLV1_KEY_EXCHANGE = 4,

	/**
	 * ID payload.
	 */
	PLV1_ID = 5,

	/**
	 * Certificate payload with certificates (CERT).
	 */
	PLV1_CERTIFICATE = 6,

	/**
	 * Certificate request payload.
	 */
	PLV1_CERTREQ = 7,

	/**
	 * Hash payload.
	 */
	PLV1_HASH = 8,

	/**
	 * Signature payload
	 */
	PLV1_SIGNATURE = 9,

	/**
	 * Nonce payload.
	 */
	PLV1_NONCE = 10,

	/**
	 * Notification payload.
	 */
	PLV1_NOTIFY = 11,

	/**
	 * Delete payload.
	 */
	PLV1_DELETE = 12,

	/**
	 * Vendor id payload.
	 */
	PLV1_VENDOR_ID = 13,

	/**
	 * Attribute payload (ISAKMP Mode Config, aka configuration payload.
	 */
	PLV1_CONFIGURATION = 14,

	/**
	 * NAT discovery payload (NAT-D).
	 */
	PLV1_NAT_D = 20,

	/**
	 * NAT original address payload (NAT-OA).
	 */
	PLV1_NAT_OA = 21,

	/**
	 * The security association (SA) payload containing proposals.
	 */
	PLV2_SECURITY_ASSOCIATION = 33,

	/**
	 * The key exchange (KE) payload containing diffie-hellman values.
	 */
	PLV2_KEY_EXCHANGE = 34,

	/**
	 * Identification for the original initiator (IDi).
	 */
	PLV2_ID_INITIATOR = 35,

	/**
	 * Identification for the original responder (IDr).
	 */
	PLV2_ID_RESPONDER = 36,

	/**
	 * Certificate payload with certificates (CERT).
	 */
	PLV2_CERTIFICATE = 37,

	/**
	 * Certificate request payload (CERTREQ).
	 */
	PLV2_CERTREQ = 38,

	/**
	 * Authentication payload contains auth data (AUTH).
	 */
	PLV2_AUTH = 39,

	/**
	 * Nonces, for initiator and responder (Ni, Nr, N)
	 */
	PLV2_NONCE = 40,

	/**
	 * Notify paylaod (N).
	 */
	PLV2_NOTIFY = 41,

	/**
	 * Delete payload (D)
	 */
	PLV2_DELETE = 42,

	/**
	 * Vendor id paylpoad (V).
	 */
	PLV2_VENDOR_ID = 43,

	/**
	 * Traffic selector for the original initiator (TSi).
	 */
	PLV2_TS_INITIATOR = 44,

	/**
	 * Traffic selector for the original responser (TSr).
	 */
	PLV2_TS_RESPONDER = 45,

	/**
	 * Encrypted payload, contains other payloads (E).
	 */
	PLV2_ENCRYPTED = 46,

	/**
	 * Configuration payload (CP).
	 */
	PLV2_CONFIGURATION = 47,

	/**
	 * Extensible authentication payload (EAP).
	 */
	PLV2_EAP = 48,

	/**
	 * Generic Secure Password Method (GSPM).
	 */
	PLV2_GSPM = 49,

	/**
	 * Group Identification (draft-yeung-g-ikev2)
	 */
	PLV2_IDG = 50,

	/**
	 * Group Security Association (draft-yeung-g-ikev2)
	 */
	PLV2_GSA = 51,

	/**
	 * Key Download (draft-yeung-g-ikev2)
	 */
	PLV2_KD = 52,

	/**
	 * Encrypted fragment payload (SKF), RFC 7383
	 */
	PLV2_FRAGMENT = 53,

#ifdef ME
	/**
	 * Identification payload for peers has a value from
	 * the PRIVATE USE space.
	 */
	PLV2_ID_PEER = 128,
#endif /* ME */

	/**
	 * NAT discovery payload (NAT-D) (drafts).
	 */
	PLV1_NAT_D_DRAFT_00_03 = 130,

	/**
	 * NAT original address payload (NAT-OA) (drafts).
	 */
	PLV1_NAT_OA_DRAFT_00_03 = 131,

	/**
	 * IKEv1 fragment (proprietary IKEv1 extension)
	 */
	PLV1_FRAGMENT = 132,

	/**
	 * Header has a value of PRIVATE USE space.
	 *
	 * This type and all the following are never sent over wire and are
	 * used internally only.
	 */
	PL_HEADER = 256,

	/**
	 * PLV2_PROPOSAL_SUBSTRUCTURE, IKEv2 proposals in a SA payload.
	 */
	PLV2_PROPOSAL_SUBSTRUCTURE,

	/**
	 * PLV1_PROPOSAL_SUBSTRUCTURE, IKEv1 proposals in a SA payload.
	 */
	PLV1_PROPOSAL_SUBSTRUCTURE,

	/**
	 * PLV2_TRANSFORM_SUBSTRUCTURE, IKEv2 transforms in a proposal substructure.
	 */
	PLV2_TRANSFORM_SUBSTRUCTURE,

	/**
	 * PLV1_TRANSFORM_SUBSTRUCTURE, IKEv1 transforms in a proposal substructure.
	 */
	PLV1_TRANSFORM_SUBSTRUCTURE,

	/**
	 * PLV2_TRANSFORM_ATTRIBUTE, IKEv2 attribute in a transform.
	 */
	PLV2_TRANSFORM_ATTRIBUTE,

	/**
	 * PLV1_TRANSFORM_ATTRIBUTE, IKEv1 attribute in a transform.
	 */
	PLV1_TRANSFORM_ATTRIBUTE,

	/**
	 * PLV2_TRAFFIC_SELECTOR_SUBSTRUCTURE, traffic selector in a TS payload.
	 */
	PLV2_TRAFFIC_SELECTOR_SUBSTRUCTURE,

	/**
	 * PLV2_CONFIGURATION_ATTRIBUTE, IKEv2 attribute in a configuration payload.
	 */
	PLV2_CONFIGURATION_ATTRIBUTE,

	/**
	 * PLV1_CONFIGURATION_ATTRIBUTE, IKEv1 attribute in a configuration payload.
	 */
	PLV1_CONFIGURATION_ATTRIBUTE,

	/**
	 * This is not really a payload, but rather the complete IKEv1 message.
	 */
	PLV1_ENCRYPTED,
};

/**
 * enum names for payload_type_t.
 */
extern enum_name_t *payload_type_names;

/**
 * enum names for payload_type_t in a short form.
 */
extern enum_name_t *payload_type_short_names;

/**
 * Generic interface for all payload types (incl.header and substructures).
 *
 * To handle all kinds of payloads on a generic way, this interface must
 * be implemented by every payload. This allows parser_t/generator_t a simple
 * handling of all payloads.
 */
struct payload_t {

	/**
	 * Get encoding rules for this payload.
	 *
	 * @param rules			location to store pointer to rules
	 * @return				number of rules
	 */
	int (*get_encoding_rules) (payload_t *this, encoding_rule_t **rules);

	/**
	 * Get non-variable header length for a variable length payload.
	 *
	 * @return				fixed length of the payload
	 */
	int (*get_header_length)(payload_t *this);

	/**
	 * Get type of payload.
	 *
	 * @return				type of this payload
	 */
	payload_type_t (*get_type) (payload_t *this);

	/**
	 * Get type of next payload or PL_NONE (0) if this is the last one.
	 *
	 * @return				type of next payload
	 */
	payload_type_t (*get_next_type) (payload_t *this);

	/**
	 * Set type of next payload.
	 *
	 * @param type			type of next payload
	 */
	void (*set_next_type) (payload_t *this,payload_type_t type);

	/**
	 * Get length of payload.
	 *
	 * @return				length of this payload
	 */
	size_t (*get_length) (payload_t *this);

	/**
	 * Verifies payload structure and makes consistence check.
	 *
	 * @return				SUCCESS,  FAILED if consistence not given
	 */
	status_t (*verify) (payload_t *this);

	/**
	 * Destroys a payload and all included substructures.
	 */
	void (*destroy) (payload_t *this);
};

/**
 * Create an empty payload.
 *
 * Useful for the parser, who wants a generic constructor for all payloads.
 * It supports all payload_t methods. If a payload type is not known,
 * an unknwon_paylod is created with the chunk of data in it.
 *
 * @param type		type of the payload to create
 * @return			payload_t object
 */
payload_t *payload_create(payload_type_t type);

/**
 * Check if a specific payload is implemented, or handled as unknown payload.
 *
 * @param type		type of the payload to check
 * @return			FALSE if payload type handled as unknown payload
 */
bool payload_is_known(payload_type_t type);

/**
 * Get the value field in a payload using encoding rules.
 *
 * @param payload	payload to look up a field
 * @param type		encoding rule type to look up
 * @param skip		number rules of type to skip, 0 to get first
 * @return			type specific value pointer, NULL if not found
 */
void* payload_get_field(payload_t *payload, encoding_type_t type, u_int skip);

#endif /** PAYLOAD_H_ @}*/
