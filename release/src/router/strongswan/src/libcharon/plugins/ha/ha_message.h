/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup ha_message ha_message
 * @{ @ingroup ha
 */

#ifndef HA_MESSAGE_H_
#define HA_MESSAGE_H_

#include <library.h>
#include <networking/host.h>
#include <utils/identification.h>
#include <sa/ike_sa_id.h>
#include <selectors/traffic_selector.h>

/**
 * Protocol version of this implementation
 */
#define HA_MESSAGE_VERSION 3

typedef struct ha_message_t ha_message_t;
typedef enum ha_message_type_t ha_message_type_t;
typedef enum ha_message_attribute_t ha_message_attribute_t;
typedef union ha_message_value_t ha_message_value_t;

/**
 * Type of a HA message
 */
enum ha_message_type_t {
	/** add a completely new IKE_SA */
	HA_IKE_ADD = 1,
	/** update an existing IKE_SA (identities, address update, ...) */
	HA_IKE_UPDATE,
	/** update initiator message id */
	HA_IKE_MID_INITIATOR,
	/** update responder message id */
	HA_IKE_MID_RESPONDER,
	/** delete an existing IKE_SA */
	HA_IKE_DELETE,
	/** add a new CHILD_SA */
	HA_CHILD_ADD,
	/** delete an existing CHILD_SA */
	HA_CHILD_DELETE,
	/** segments the sending node is giving up */
	HA_SEGMENT_DROP,
	/** segments the sending node is taking over */
	HA_SEGMENT_TAKE,
	/** status with the segments the sending node is currently serving */
	HA_STATUS,
	/** segments the receiving node is requested to resync */
	HA_RESYNC,
	/** IV synchronization for IKEv1 Main/Aggressive mode */
	HA_IKE_IV,
};

/**
 * Enum names for message types
 */
extern enum_name_t *ha_message_type_names;

/**
 * Type of attributes contained in a message
 */
enum ha_message_attribute_t {
	/** ike_sa_id_t*, to identify IKE_SA */
	HA_IKE_ID = 1,
	/** ike_sa_id_t*, identifies IKE_SA which gets rekeyed */
	HA_IKE_REKEY_ID,
	/** identification_t*, local identity */
	HA_LOCAL_ID,
	/** identification_t*, remote identity */
	HA_REMOTE_ID,
	/** identification_t*, remote EAP identity */
	HA_REMOTE_EAP_ID,
	/** host_t*, local address */
	HA_LOCAL_ADDR,
	/** host_t*, remote address */
	HA_REMOTE_ADDR,
	/** char*, name of configuration */
	HA_CONFIG_NAME,
	/** uint32_t, bitset of ike_condition_t */
	HA_CONDITIONS,
	/** uint32_t, bitset of ike_extension_t */
	HA_EXTENSIONS,
	/** host_t*, local virtual IP */
	HA_LOCAL_VIP,
	/** host_t*, remote virtual IP */
	HA_REMOTE_VIP,
	/** host_t*, known peer addresses (used for MOBIKE) */
	HA_PEER_ADDR,
	/** uint8_t, initiator of an exchange, TRUE for local */
	HA_INITIATOR,
	/** chunk_t, initiators nonce */
	HA_NONCE_I,
	/** chunk_t, responders nonce */
	HA_NONCE_R,
	/** chunk_t, diffie hellman shared secret */
	HA_SECRET,
	/** chunk_t, SKd of old SA if rekeying */
	HA_OLD_SKD,
	/** uint16_t, pseudo random function */
	HA_ALG_PRF,
	/** uint16_t, old pseudo random function if rekeying */
	HA_ALG_OLD_PRF,
	/** uint16_t, encryption algorithm */
	HA_ALG_ENCR,
	/** uint16_t, encryption key size in bytes */
	HA_ALG_ENCR_LEN,
	/** uint16_t, integrity protection algorithm */
	HA_ALG_INTEG,
	/** uint16_t, DH group */
	HA_ALG_DH,
	/** uint8_t, IPsec mode, TUNNEL|TRANSPORT|... */
	HA_IPSEC_MODE,
	/** uint8_t, IPComp protocol */
	HA_IPCOMP,
	/** uint32_t, inbound security parameter index */
	HA_INBOUND_SPI,
	/** uint32_t, outbound security parameter index */
	HA_OUTBOUND_SPI,
	/** uint16_t, inbound security parameter index */
	HA_INBOUND_CPI,
	/** uint16_t, outbound security parameter index */
	HA_OUTBOUND_CPI,
	/** traffic_selector_t*, local traffic selector */
	HA_LOCAL_TS,
	/** traffic_selector_t*, remote traffic selector */
	HA_REMOTE_TS,
	/** uint32_t, message ID */
	HA_MID,
	/** uint16_t, HA segment */
	HA_SEGMENT,
	/** uint16_t, Extended Sequence numbers */
	HA_ESN,
	/** uint8_t, IKE version */
	HA_IKE_VERSION,
	/** chunk_t, own DH public value */
	HA_LOCAL_DH,
	/** chunk_t, remote DH public value */
	HA_REMOTE_DH,
	/** chunk_t, shared secret for IKEv1 key derivation */
	HA_PSK,
	/** chunk_t, IV for next IKEv1 message */
	HA_IV,
	/** uint16_t, auth_method_t for IKEv1 key derivation */
	HA_AUTH_METHOD,
};

/**
 * Union to enumerate typed attributes in a message
 */
union ha_message_value_t {
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	char *str;
	chunk_t chunk;
	ike_sa_id_t *ike_sa_id;
	identification_t *id;
	host_t *host;
	traffic_selector_t *ts;
};

/**
 * Abstracted message passed between nodes in a HA cluster.
 */
struct ha_message_t {

	/**
	 * Get the type of the message.
	 *
	 * @return		message type
	 */
	ha_message_type_t (*get_type)(ha_message_t *this);

	/**
	 * Add an attribute to a message.
	 *
	 * @param attribute		attribute type to add
	 * @param ...			attribute specific data
	 */
	void (*add_attribute)(ha_message_t *this,
						  ha_message_attribute_t attribute, ...);

	/**
	 * Create an enumerator over all attributes in a message.
	 *
	 * @return				enumerator over attribute, ha_message_value_t
	 */
	enumerator_t* (*create_attribute_enumerator)(ha_message_t *this);

	/**
	 * Get the message in a encoded form.
	 *
	 * @return				chunk pointing to internal data
	 */
	chunk_t (*get_encoding)(ha_message_t *this);

	/**
	 * Destroy a ha_message_t.
	 */
	void (*destroy)(ha_message_t *this);
};

/**
 * Create a new ha_message instance, ready for adding attributes
 *
 * @param type				type of the message
 */
ha_message_t *ha_message_create(ha_message_type_t type);

/**
 * Create a ha_message from encoded data.
 *
 * @param data				encoded message data
 */
ha_message_t *ha_message_parse(chunk_t data);

#endif /** HA_MESSAGE_ @}*/
