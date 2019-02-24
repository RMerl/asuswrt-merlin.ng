/*
 * Copyright (C) 2007 Tobias Brunner
 * Copyright (C) 2005-2011 Martin Willi
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
 * @defgroup ike_header ike_header
 * @{ @ingroup payloads
 */

#ifndef IKE_HEADER_H_
#define IKE_HEADER_H_

typedef enum exchange_type_t exchange_type_t;
typedef struct ike_header_t ike_header_t;

#include <library.h>
#include <encoding/payloads/payload.h>

/**
 * Major Version of IKEv1 we implement.
 */
#define IKEV1_MAJOR_VERSION 1

/**
 * Minor Version of IKEv1 we implement.
 */
#define IKEV1_MINOR_VERSION 0

/**
 * Major Version of IKEv2 we implement.
 */
#define IKEV2_MAJOR_VERSION 2

/**
 * Minor Version of IKEv2 we implement.
 */
#define IKEV2_MINOR_VERSION 0

/**
 * Length of IKE Header in Bytes.
 */
#define IKE_HEADER_LENGTH 28

/**
 * Different types of IKE-Exchanges.
 *
 * See RFC for different types.
 */
enum exchange_type_t{

	/**
	 * Identity Protection (Main mode).
	 */
	ID_PROT = 2,

	/**
	 * Authentication Only.
	 */
	AUTH_ONLY = 3,

	/**
	 * Aggressive (Aggressive mode)
	 */
	AGGRESSIVE = 4,

	/**
	 * Informational in IKEv1
	 */
	INFORMATIONAL_V1 = 5,

	/**
	 * Transaction (ISAKMP Cfg Mode "draft-ietf-ipsec-isakmp-mode-cfg-05")
	 */
	TRANSACTION = 6,

	/**
	 * Quick Mode
	 */
	QUICK_MODE = 32,

	/**
	 * New Group Mode
	 */
	NEW_GROUP_MODE = 33,

	/**
	 * IKE_SA_INIT.
	 */
	IKE_SA_INIT = 34,

	/**
	 * IKE_AUTH.
	 */
	IKE_AUTH = 35,

	/**
	 * CREATE_CHILD_SA.
	 */
	CREATE_CHILD_SA = 36,

	/**
	 * INFORMATIONAL in IKEv2.
	 */
	INFORMATIONAL = 37,

	/**
	 * IKE_SESSION_RESUME (RFC 5723).
	 */
	IKE_SESSION_RESUME = 38,

#ifdef ME
	/**
	 * ME_CONNECT
	 */
	ME_CONNECT = 240,
#endif /* ME */

	/**
	 * Undefined exchange type, in private space.
	 */
	EXCHANGE_TYPE_UNDEFINED = 255,
};

/**
 * enum name for exchange_type_t
 */
extern enum_name_t *exchange_type_names;

/**
 * An object of this type represents an IKE header of either IKEv1 or IKEv2.
 */
struct ike_header_t {
	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the initiator spi.
	 *
	 * @return				initiator_spi
	 */
	uint64_t (*get_initiator_spi) (ike_header_t *this);

	/**
	 * Set the initiator spi.
	 *
	 * @param initiator_spi	initiator_spi
	 */
	void (*set_initiator_spi) (ike_header_t *this, uint64_t initiator_spi);

	/**
	 * Get the responder spi.
	 *
	 * @return				responder_spi
	 */
	uint64_t (*get_responder_spi) (ike_header_t *this);

	/**
	 * Set the responder spi.
	 *
	 * @param responder_spi	responder_spi
	 */
	void (*set_responder_spi) (ike_header_t *this, uint64_t responder_spi);

	/**
	 * Get the major version.
	 *
	 * @return				major version
	 */
	uint8_t (*get_maj_version) (ike_header_t *this);

	/**
	 * Set the major version.
	 *
	 * @param major			major version
	 */
	void (*set_maj_version) (ike_header_t *this, uint8_t major);

	/**
	 * Get the minor version.
	 *
	 * @return				minor version
	 */
	uint8_t (*get_min_version) (ike_header_t *this);

	/**
	 * Set the minor version.
	 *
	 * @param minor			minor version
	 */
	void (*set_min_version) (ike_header_t *this, uint8_t minor);

	/**
	 * Get the response flag.
	 *
	 * @return				response flag
	 */
	bool (*get_response_flag) (ike_header_t *this);

	/**
	 * Set the response flag-
	 *
	 * @param response		response flag
	 */
	void (*set_response_flag) (ike_header_t *this, bool response);

	/**
	 * Get "higher version supported"-flag.
	 *
	 * @return				version flag
	 */
	bool (*get_version_flag) (ike_header_t *this);

	/**
	 * Set the "higher version supported"-flag.
	 *
	 * @param version		flag value
	 */
	void (*set_version_flag)(ike_header_t *this, bool version);

	/**
	 * Get the initiator flag.
	 *
	 * @return				initiator flag
	 */
	bool (*get_initiator_flag) (ike_header_t *this);

	/**
	 * Set the initiator flag.
	 *
	 * @param initiator		initiator flag
	 */
	void (*set_initiator_flag) (ike_header_t *this, bool initiator);

	/**
	 * Get the encryption flag.
	 *
	 * @return				encryption flag
	 */
	bool (*get_encryption_flag) (ike_header_t *this);

	/**
	 * Set the encryption flag.
	 *
	 * @param encryption		encryption flag
	 */
	void (*set_encryption_flag) (ike_header_t *this, bool encryption);

	/**
	 * Get the commit flag.
	 *
	 * @return				commit flag
	 */
	bool (*get_commit_flag) (ike_header_t *this);

	/**
	 * Set the commit flag.
	 *
	 * @param commit		commit flag
	 */
	void (*set_commit_flag) (ike_header_t *this, bool commit);

	/**
	 * Get the authentication only flag.
	 *
	 * @return				authonly flag
	 */
	bool (*get_authonly_flag) (ike_header_t *this);

	/**
	 * Set the authentication only flag.
	 *
	 * @param authonly		authonly flag
	 */
	void (*set_authonly_flag) (ike_header_t *this, bool authonly);

	/**
	 * Get the exchange type.
	 *
	 * @return				exchange type
	 */
	uint8_t (*get_exchange_type) (ike_header_t *this);

	/**
	 * Set the  exchange type.
	 *
	 * @param exchange_type	exchange type
	 */
	void (*set_exchange_type) (ike_header_t *this, uint8_t exchange_type);

	/**
	 * Get the message id.
	 *
	 * @return				message id
	 */
	uint32_t (*get_message_id) (ike_header_t *this);

	/**
	 * Set the message id.
	 *
	 * @param initiator_spi	message id
	 */
	void (*set_message_id) (ike_header_t *this, uint32_t message_id);

	/**
	 * Destroys a ike_header_t object.
	 */
	void (*destroy) (ike_header_t *this);
};

/**
 * Create an empty ike_header_t object.
 *
 * @return ike_header_t object
 */
ike_header_t *ike_header_create(void);

/**
 * Create an ike_header_t object for a specific major/minor version
 *
 * @return ike_header_t object
 */
ike_header_t *ike_header_create_version(int major, int minor);

#endif /** IKE_HEADER_H_ @}*/
