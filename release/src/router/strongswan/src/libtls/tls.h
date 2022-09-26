/*
 * Copyright (C) 2021 Tobias Brunner
 * Copyright (C) 2020-2021 Pascal Knecht
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup libtls libtls
 *
 * @addtogroup libtls
 * TLS implementation on top of libstrongswan
 *
 * @defgroup tls tls
 * @{ @ingroup libtls
 */

#ifndef TLS_H_
#define TLS_H_

/**
 * Maximum size of a TLS fragment
 * as defined by section 6.2.1. "Fragmentation" of RFC 5246 TLS 1.2
 */
#define TLS_MAX_FRAGMENT_LEN	16384

typedef enum tls_version_t tls_version_t;
typedef enum tls_content_type_t tls_content_type_t;
typedef enum tls_handshake_type_t tls_handshake_type_t;
typedef enum tls_purpose_t tls_purpose_t;
typedef enum tls_flag_t tls_flag_t;
typedef struct tls_t tls_t;

#include <library.h>

#include "tls_application.h"
#include "tls_cache.h"

/**
 * TLS/SSL version numbers
 */
enum tls_version_t {
	TLS_UNSPEC = 0,
	SSL_2_0 = 0x0200,
	SSL_3_0 = 0x0300,
	TLS_1_0 = 0x0301,
	TLS_SUPPORTED_MIN = TLS_1_0,
	TLS_1_1 = 0x0302,
	TLS_1_2 = 0x0303,
	TLS_1_3 = 0x0304,
	TLS_SUPPORTED_MAX = TLS_1_3,
};

/**
 * Enum names for tls_version_t
 */
extern enum_name_t *tls_version_names;

/**
 * Simple, numeric enum names for tls_version_t (only supported versions)
 */
extern enum_name_t *tls_numeric_version_names;

/**
 * TLS higher level content type
 */
enum tls_content_type_t {
	TLS_CHANGE_CIPHER_SPEC = 20,
	TLS_ALERT = 21,
	TLS_HANDSHAKE = 22,
	TLS_APPLICATION_DATA = 23,
};

/**
 * Enum names for tls_content_type_t
 */
extern enum_name_t *tls_content_type_names;

/**
 * TLS handshake subtype
 */
enum tls_handshake_type_t {
	TLS_HELLO_REQUEST = 0,
	TLS_CLIENT_HELLO = 1,
	TLS_SERVER_HELLO = 2,
	TLS_HELLO_VERIFY_REQUEST = 3,
	TLS_NEW_SESSION_TICKET = 4,
	TLS_END_OF_EARLY_DATA = 5,
	TLS_HELLO_RETRY_REQUEST = 6,
	TLS_ENCRYPTED_EXTENSIONS = 8,
	TLS_CERTIFICATE = 11,
	TLS_SERVER_KEY_EXCHANGE = 12,
	TLS_CERTIFICATE_REQUEST = 13,
	TLS_SERVER_HELLO_DONE = 14,
	TLS_CERTIFICATE_VERIFY = 15,
	TLS_CLIENT_KEY_EXCHANGE = 16,
	TLS_FINISHED = 20,
	TLS_CERTIFICATE_URL = 21,
	TLS_CERTIFICATE_STATUS = 22,
	TLS_SUPPLEMENTAL_DATA = 23,
	TLS_KEY_UPDATE = 24,
	TLS_MESSAGE_HASH = 254,
};

/**
 * Enum names for tls_handshake_type_t
 */
extern enum_name_t *tls_handshake_type_names;

/**
 * Purpose the TLS stack is initiated for.
 */
enum tls_purpose_t {
	/** authentication in EAP-TLS */
	TLS_PURPOSE_EAP_TLS,
	/** outer authentication and protection in EAP-TTLS */
	TLS_PURPOSE_EAP_TTLS,
	/** outer authentication and protection in EAP-PEAP */
	TLS_PURPOSE_EAP_PEAP,
	/** non-EAP TLS */
	TLS_PURPOSE_GENERIC,
	/** EAP binding for TNC */
	TLS_PURPOSE_EAP_TNC
};

/**
 * TLS Handshake extension types.
 */
enum tls_extension_t {
	/** Server name the client wants to talk to */
	TLS_EXT_SERVER_NAME = 0,
	/** request a maximum fragment size */
	TLS_EXT_MAX_FRAGMENT_LENGTH = 1,
	/** indicate client certificate URL support */
	TLS_EXT_CLIENT_CERTIFICATE_URL = 2,
	/** list of CA the client trusts */
	TLS_EXT_TRUSTED_CA_KEYS = 3,
	/** request MAC truncation to 80-bit */
	TLS_EXT_TRUNCATED_HMAC = 4,
	/** list of OCSP responders the client trusts */
	TLS_EXT_STATUS_REQUEST = 5,
	/** list of supported groups, in legacy tls: elliptic curves */
	TLS_EXT_SUPPORTED_GROUPS = 10,
	/** supported point formats */
	TLS_EXT_EC_POINT_FORMATS = 11,
	/** list supported signature algorithms */
	TLS_EXT_SIGNATURE_ALGORITHMS = 13,
	/** indicate usage of Datagram Transport Layer Security (DTLS) */
	TLS_EXT_USE_SRTP = 14,
	/** indicate usage of heartbeat */
	TLS_EXT_HEARTBEAT = 15,
	/** indicate usage of application-layer protocol negotiation */
	TLS_EXT_APPLICATION_LAYER_PROTOCOL_NEGOTIATION = 16,
	/** exchange raw public key, client side*/
	TLS_CLIENT_CERTIFICATE_TYPE = 19,
	/** exchange raw public key, server side*/
	TLS_SERVER_CERTIFICATE_TYPE = 20,
	/** use encrypt-then-MAC security mechanism RFC 7366 */
	TLS_EXT_ENCRYPT_THEN_MAC = 22,
	/** bind master secret to handshake data RFC 7627 */
	TLS_EXT_EXTENDED_MASTER_SECRET = 23,
	/** session resumption without server-side state RFC 5077 */
	TLS_EXT_SESSION_TICKET = 35,
	/** negotiate identity of the psk **/
	TLS_EXT_PRE_SHARED_KEY = 41,
	/** send data in 0-RTT when psk is used and early data is allowed **/
	TLS_EXT_EARLY_DATA = 42,
	/** negotiate supported tls versions **/
	TLS_EXT_SUPPORTED_VERSIONS = 43,
	/** identify client **/
	TLS_EXT_COOKIE = 44,
	/** psk modes supported by the client **/
	TLS_EXT_PSK_KEY_EXCHANGE_MODES = 45,
	/** indicate supported ca's by endpoint **/
	TLS_EXT_CERTIFICATE_AUTHORITIES = 47,
	/** provide oid/value pairs to match client's certificate **/
	TLS_EXT_OID_FILTERS = 48,
	/** willing to perform post-handshake authentication **/
	TLS_EXT_POST_HANDSHAKE_AUTH = 49,
	/** list supported signature algorithms to verify certificates **/
	TLS_EXT_SIGNATURE_ALGORITHMS_CERT = 50,
	/** list endpoint's cryptographic parameters **/
	TLS_EXT_KEY_SHARE = 51,
	/** cryptographic binding for RFC 5746 renegotiation indication */
	TLS_EXT_RENEGOTIATION_INFO = 65281,
};

enum tls_name_type_t {
	TLS_NAME_TYPE_HOST_NAME = 0,
};

/**
 * Flags that control the behavior of the stack
 */
enum tls_flag_t {
	/** set if cipher suites with null encryption are acceptable */
	TLS_FLAG_ENCRYPTION_OPTIONAL = 1,
	/** set if client authentication is optional even if cert req sent */
	TLS_FLAG_CLIENT_AUTH_OPTIONAL = 2,
};

/**
 * Enum names for tls_extension_t
 */
extern enum_name_t *tls_extension_names;

/**
 * Magic value (SHA-256 of "HelloRetryRequest") for Random to differentiate
 * ServerHello from HelloRetryRequest.
 */
extern chunk_t tls_hello_retry_request_magic;

/**
 * Magic values for downgrade protection (see RFC 8446, section 4.1.3)
 */
extern chunk_t tls_downgrade_protection_tls11;
extern chunk_t tls_downgrade_protection_tls12;

/**
 * A bottom-up driven TLS stack, suitable for EAP implementations.
 */
struct tls_t {

	/**
	 * Process one or more TLS records, pass it to upper layers.
	 *
	 * @param buf		TLS record data, including headers
	 * @param buflen	number of bytes in buf to process
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- NEED_MORE if more invocations to process/build needed
	 */
	status_t (*process)(tls_t *this, void *buf, size_t buflen);

	/**
	 * Query upper layer for one or more TLS records, build fragments.
	 *
	 * The TLS stack automatically fragments the records to the given buffer
	 * size. Fragmentation is indicated by the reclen output parameter and
	 * the return value. For the first fragment of a TLS record, a non-zero
	 * record length is returned in reclen. If more fragments follow, NEED_MORE
	 * is returned. A return value of ALREADY_DONE indicates that the final
	 * fragment has been returned.
	 *
	 * @param buf		buffer to write TLS record fragments to
	 * @param buflen	size of buffer, receives bytes written
	 * @param msglen	receives size of all TLS fragments
	 * @return
	 *					- SUCCESS if TLS negotiation complete
	 *					- FAILED if TLS handshake failed
	 *					- INVALID_STATE if more input data required
	 *					- NEED_MORE if more fragments available
	 *					- ALREADY_DONE if the last available fragment returned
	 */
	status_t (*build)(tls_t *this, void *buf, size_t *buflen, size_t *msglen);

	/**
	 * Check if TLS stack is acting as a server.
	 *
	 * @return			TRUE if server, FALSE if peer
	 */
	bool (*is_server)(tls_t *this);

	/**
	 * Return the server identity.
	 *
	 * @return			server identity
	 */
	identification_t* (*get_server_id)(tls_t *this);

	/**
	 * Set the peer identity.
	 *
	 * @param id		peer identity
	 */
	void (*set_peer_id)(tls_t *this, identification_t *id);

	/**
	 * Return the peer identity.
	 *
	 * @return			peer identity
	 */
	identification_t* (*get_peer_id)(tls_t *this);

	/**
	 * Get the maximum and negotiated TLS version.
	 *
	 * @return			max and negotiated TLS version
	 */
	tls_version_t (*get_version_max)(tls_t *this);

	/**
	* Get the minimum TLS version.
	*
	* @return			min TLS version
	*/
	tls_version_t (*get_version_min)(tls_t *this);

	/**
	 * Set the initial minimum/maximum TLS version, or set both to the same
	 * value once negotiated.
	 *
	 * @param min_version	minimum (or negotiated) TLS version
	 * @param max_version	maximum (or negotiated) TLS version
	 * @return				TRUE if version(s) acceptable
	 */
	bool (*set_version)(tls_t *this, tls_version_t min_version,
						tls_version_t max_version);

	/**
	 * Get the purpose of this TLS stack instance.
	 *
	 * @return			purpose given during construction
	 */
	tls_purpose_t (*get_purpose)(tls_t *this);

	/**
	 * Get the flags controlling this TLS stack instance.
	 *
	 * @return			flags given during construction
	 */
	tls_flag_t (*get_flags)(tls_t *this);

	/**
	 * Check if TLS negotiation completed successfully.
	 *
	 * @return			TRUE if TLS negotiation and authentication complete
	 */
	bool (*is_complete)(tls_t *this);

	/**
	 * Get the MSK for EAP-TLS.
	 *
	 * @return			MSK, internal data
	 */
	chunk_t (*get_eap_msk)(tls_t *this);

	/**
	 * Get the authentication details after completing the handshake.
	 *
	 * @return			authentication details, internal data
	 */
	auth_cfg_t* (*get_auth)(tls_t *this);

	/**
	 * Destroy a tls_t.
	 */
	void (*destroy)(tls_t *this);
};

/**
 * Dummy libtls initialization function needed for integrity test
 */
void libtls_init(void);

/**
 * Create a tls instance.
 *
 * @param is_server			TRUE to act as server, FALSE for client
 * @param server			server identity
 * @param peer				peer identity, NULL for no client authentication
 * @param purpose			purpose this TLS stack instance is used for
 * @param application		higher layer application or NULL if none
 * @param cache				session cache to use, or NULL
 * @param flags				flags that control the behavior of the TLS stack
 * @return					TLS stack
 */
tls_t *tls_create(bool is_server, identification_t *server,
				  identification_t *peer, tls_purpose_t purpose,
				  tls_application_t *application, tls_cache_t *cache,
				  tls_flag_t flags);

#endif /** TLS_H_ @}*/
