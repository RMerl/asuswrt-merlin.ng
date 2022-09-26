/*
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
 * @defgroup tls_alert tls_alert
 * @{ @ingroup libtls
 */

#ifndef TLS_ALERT_H_
#define TLS_ALERT_H_

#include <library.h>

typedef struct tls_alert_t tls_alert_t;
typedef enum tls_alert_level_t tls_alert_level_t;
typedef enum tls_alert_desc_t tls_alert_desc_t;

/**
 * Level of a TLS alert
 */
enum tls_alert_level_t {
	TLS_WARNING = 1,
	TLS_FATAL = 2,
};

/**
 * Description of a TLS alert
 */
enum tls_alert_desc_t {
	TLS_CLOSE_NOTIFY = 0,
	TLS_UNEXPECTED_MESSAGE = 10,
	TLS_BAD_RECORD_MAC = 20,
	TLS_DECRYPTION_FAILED = 21,
	TLS_RECORD_OVERFLOW = 22,
	TLS_DECOMPRESSION_FAILURE = 30,
	TLS_HANDSHAKE_FAILURE = 40,
	TLS_NO_CERTIFICATE = 41,
	TLS_BAD_CERTIFICATE = 42,
	TLS_UNSUPPORTED_CERTIFICATE = 43,
	TLS_CERTIFICATE_REVOKED = 44,
	TLS_CERTIFICATE_EXPIRED = 45,
	TLS_CERTIFICATE_UNKNOWN = 46,
	TLS_ILLEGAL_PARAMETER = 47,
	TLS_UNKNOWN_CA = 48,
	TLS_ACCESS_DENIED = 49,
	TLS_DECODE_ERROR = 50,
	TLS_DECRYPT_ERROR = 51,
	TLS_EXPORT_RESTRICTION = 60,
	TLS_PROTOCOL_VERSION = 70,
	TLS_INSUFFICIENT_SECURITY = 71,
	TLS_INTERNAL_ERROR = 80,
	TLS_INAPPROPRIATE_FALLBACK = 86,
	TLS_USER_CANCELED = 90,
	TLS_NO_RENEGOTIATION = 100,
	TLS_MISSING_EXTENSION = 109,
	TLS_UNSUPPORTED_EXTENSION = 110,
	TLS_CERTIFICATE_UNOBTAINABLE = 111,
	TLS_RECOGNIZED_NAME = 112,
	TLS_BAD_CERTIFICATE_STATUS_RESPONSE = 113,
	TLS_BAD_CERTIFICATE_HASH_VALUE = 114,
	TLS_UNKNOWN_PSK_IDENTITY = 115,
	TLS_CERTIFICATE_REQUIRED = 116,
	TLS_NO_APPLICATION_PROTOCOL = 120,
};

/**
 * Enum names for alert descriptions
 */
extern enum_name_t *tls_alert_desc_names;

/**
 * TLS alert handling.
 */
struct tls_alert_t {

	/**
	 * Add an alert to the TLS alert queue, will be sent.
	 *
	 * @param level			level of TLS alert
	 * @param description	description of alert
	 */
	void (*add)(tls_alert_t *this, tls_alert_level_t level,
				tls_alert_desc_t description);

	/**
	 * Get an alert pushed to the alert queue, to send.
	 *
	 * @param level			receives TLS alert level
	 * @param description	receives TLS alert description
	 * @return				TRUE if returned an alert
	 */
	bool (*get)(tls_alert_t *this, tls_alert_level_t *level,
				tls_alert_desc_t *description);

	/**
	 * Did a fatal alert occur?.
	 *
	 * @return				TRUE if a fatal alert has occurred
	 */
	bool (*fatal)(tls_alert_t *this);

	/**
	 * Process a received TLS alert.
	 *
	 * @param level			level of received alert
	 * @param description	alert description
	 * @return				status to pass down to TLS stack
	 */
	status_t (*process)(tls_alert_t *this, tls_alert_level_t level,
						tls_alert_desc_t description);

	/**
	 * Destroy a tls_alert_t.
	 */
	void (*destroy)(tls_alert_t *this);
};

/**
 * Create a tls_alert instance.
 */
tls_alert_t *tls_alert_create();

#endif /** TLS_ALERT_H_ @}*/
