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
 * @defgroup libsimaka libsimaka
 *
 * @addtogroup libsimaka
 * Library providing functions shared between EAP-SIM and EAP-AKA plugins.
 *
 * @defgroup simaka_message simaka_message
 * @{ @ingroup libsimaka
 */

#ifndef SIMAKA_MESSAGE_H_
#define SIMAKA_MESSAGE_H_

#include <utils/utils.h>
#include <eap/eap.h>

#include "simaka_crypto.h"

typedef enum simaka_attribute_t simaka_attribute_t;
typedef enum simaka_subtype_t simaka_subtype_t;
typedef enum simaka_notification_t simaka_notification_t;
typedef enum simaka_client_error_t simaka_client_error_t;
typedef struct simaka_message_t simaka_message_t;

/**
 * Subtypes of EAP-SIM/AKA messages
 */
enum simaka_subtype_t {
	AKA_CHALLENGE = 1,
	AKA_AUTHENTICATION_REJECT = 2,
	AKA_SYNCHRONIZATION_FAILURE = 4,
	AKA_IDENTITY = 5,
	SIM_START = 10,
	SIM_CHALLENGE = 11,
	SIM_NOTIFICATION = 12,
	AKA_NOTIFICATION = 12,
	SIM_REAUTHENTICATION = 13,
	AKA_REAUTHENTICATION = 13,
	SIM_CLIENT_ERROR = 14,
	AKA_CLIENT_ERROR = 14,
};

/**
 * Enum names for simaka_subtype_t
 */
extern enum_name_t *simaka_subtype_names;

/**
 * Attributes in EAP-SIM/AKA messages
 */
enum simaka_attribute_t {
	AT_RAND = 1,
	AT_AUTN = 2,
	AT_RES = 3,
	AT_AUTS = 4,
	AT_PADDING = 6,
	AT_NONCE_MT = 7,
	AT_PERMANENT_ID_REQ = 10,
	AT_MAC = 11,
	AT_NOTIFICATION = 12,
	AT_ANY_ID_REQ = 13,
	AT_IDENTITY = 14,
	AT_VERSION_LIST = 15,
	AT_SELECTED_VERSION = 16,
	AT_FULLAUTH_ID_REQ = 17,
	AT_COUNTER = 19,
	AT_COUNTER_TOO_SMALL = 20,
	AT_NONCE_S = 21,
	AT_CLIENT_ERROR_CODE = 22,
	AT_IV = 129,
	AT_ENCR_DATA = 130,
	AT_NEXT_PSEUDONYM = 132,
	AT_NEXT_REAUTH_ID = 133,
	AT_CHECKCODE = 134,
	AT_RESULT_IND = 135,
};

/**
 * Enum names for simaka_attribute_t
 */
extern enum_name_t *simaka_attribute_names;

/**
 * Notification codes used within AT_NOTIFICATION attribute.
 */
enum simaka_notification_t {
	/** SIM General failure after authentication. (Implies failure) */
	SIM_GENERAL_FAILURE_AA = 0,
	/** AKA General failure after authentication. (Implies failure) */
	AKA_GENERAL_FAILURE_AA = 0,
	/** SIM General failure. (Implies failure, used before authentication) */
	SIM_GENERAL_FAILURE = 16384,
	/** AKA General failure. (Implies failure, used before authentication) */
	AKA_GENERAL_FAILURE = 16384,
	/** SIM User has been temporarily denied access to the requested service. */
	SIM_TEMP_DENIED = 1026,
	/** AKA User has been temporarily denied access to the requested service. */
	AKA_TEMP_DENIED = 1026,
	/** SIM User has not subscribed to the requested service. */
	SIM_NOT_SUBSCRIBED = 1031,
	/** AKA User has not subscribed to the requested service. */
	AKA_NOT_SUBSCRIBED = 1031,
	/** SIM Success. User has been successfully authenticated. */
	SIM_SUCCESS = 32768,
	/** AKA Success. User has been successfully authenticated. */
	AKA_SUCCESS = 32768,
};

/**
 * Enum names for simaka_notification_t
 */
extern enum_name_t *simaka_notification_names;

/**
 * Error codes sent in AT_CLIENT_ERROR_CODE attribute
 */
enum simaka_client_error_t {
	/** AKA unable to process packet */
	AKA_UNABLE_TO_PROCESS = 0,
	/** SIM unable to process packet */
	SIM_UNABLE_TO_PROCESS = 0,
	/** SIM unsupported version */
	SIM_UNSUPPORTED_VERSION = 1,
	/** SIM insufficient number of challenges */
	SIM_INSUFFICIENT_CHALLENGES = 2,
	/** SIM RANDs are not fresh */
	SIM_RANDS_NOT_FRESH = 3,
};

/**
 * Enum names for simaka_client_error_t
 */
extern enum_name_t *simaka_client_error_names;

/**
 * Check if an EAP-SIM/AKA attribute is "skippable".
 *
 * @param attribute		attribute to check
 * @return				TRUE if attribute skippable, FALSE if non-skippable
 */
bool simaka_attribute_skippable(simaka_attribute_t attribute);

/**
 * EAP-SIM and EAP-AKA message abstraction.
 *
 * Messages for EAP-SIM and EAP-AKA share a common format, this class
 * abstracts such a message and provides encoding/encryption/signing
 * functionality.
 */
struct simaka_message_t {

	/**
	 * Check if the given message is a request or response.
	 *
	 * @return			TRUE if request, FALSE if response
	 */
	bool (*is_request)(simaka_message_t *this);

	/**
	 * Get the EAP message identifier.
	 *
	 * @return			EAP message identifier
	 */
	uint8_t (*get_identifier)(simaka_message_t *this);

	/**
	 * Get the EAP type of the message.
	 *
	 * @return			EAP type: EAP-SIM or EAP-AKA
	 */
	eap_type_t (*get_type)(simaka_message_t *this);

	/**
	 * Get the subtype of an EAP-SIM message.
	 *
	 * @return			subtype of message
	 */
	simaka_subtype_t (*get_subtype)(simaka_message_t *this);

	/**
	 * Create an enumerator over message attributes.
	 *
	 * @return			enumerator over (simaka_attribute_t, chunk_t)
	 */
	enumerator_t* (*create_attribute_enumerator)(simaka_message_t *this);

	/**
	 * Append an attribute to the EAP-SIM message.
	 *
	 * Make sure to pass only data of correct length for the given attribute.
	 *
	 * @param type		type of attribute to add to message
	 * @param data		unpadded attribute data to add
	 */
	void (*add_attribute)(simaka_message_t *this, simaka_attribute_t type,
						  chunk_t data);

	/**
	 * Parse a message, with optional attribute decryption.
	 *
	 * This method does not verify message integrity, as the key is available
	 * only after the payload has been parsed. It might be necessary to call
	 * parse twice, as key derivation data in EAP-SIM/AKA is in the same
	 * packet as encrypted data.
	 *
	 * @param crypto	EAP-SIM/AKA crypto helper
	 * @return			TRUE if message parsed successfully
	 */
	bool (*parse)(simaka_message_t *this);

	/**
	 * Verify the message integrity of a parsed message.
	 *
	 * @param crypto	EAP-SIM/AKA crypto helper
	 * @param sigdata	additional data to include in signature, if any
	 * @return			TRUE if message integrity check successful
	 */
	bool (*verify)(simaka_message_t *this, chunk_t sigdata);

	/**
	 * Generate a message, optionally encrypt attributes and create a MAC.
	 *
	 * @param sigdata	additional data to include in signature, if any
	 * @param gen		allocated generated data, if successful
	 * @return			TRUE if successful
	 */
	bool (*generate)(simaka_message_t *this, chunk_t sigdata, chunk_t *gen);

	/**
	 * Destroy a simaka_message_t.
	 */
	void (*destroy)(simaka_message_t *this);
};

/**
 * Create an empty simaka_message.
 *
 * @param request		TRUE for a request message, FALSE for a response
 * @param identifier	EAP message identifier
 * @param type			EAP type: EAP-SIM or EAP-AKA
 * @param subtype		subtype of the EAP message
 * @param crypto		EAP-SIM/AKA crypto helper
 * @return				empty message of requested kind, NULL on error
 */
simaka_message_t *simaka_message_create(bool request, uint8_t identifier,
									eap_type_t type, simaka_subtype_t subtype,
									simaka_crypto_t *crypto);

/**
 * Create an simaka_message from a chunk of data.
 *
 * @param data			message data to parse
 * @param crypto		EAP-SIM/AKA crypto helper
 * @return				EAP message, NULL on error
 */
simaka_message_t *simaka_message_create_from_payload(chunk_t data,
													 simaka_crypto_t *crypto);

#endif /** SIMAKA_MESSAGE_H_ @}*/
