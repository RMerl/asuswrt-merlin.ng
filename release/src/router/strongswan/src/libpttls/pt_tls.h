/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup pt_tls libpttls
 *
 * @addtogroup pt_tls
 * @{
 */

#ifndef PT_TLS_H_
#define PT_TLS_H_

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>
#include <tls_socket.h>

/**
 * PT-TLS version we support
 */
#define PT_TLS_VERSION 1

/**
 * Length of a PT-TLS header
 */
#define PT_TLS_HEADER_LEN 16

/**
 * Maximum size of a PT-TLS message
 */
#define PT_TLS_MAX_MESSAGE_LEN	128 * TLS_MAX_FRAGMENT_LEN - PT_TLS_HEADER_LEN

/**
 * Default PT-TLS port
 */
#define PT_TLS_PORT	 271

typedef enum pt_tls_message_type_t pt_tls_message_type_t;
typedef enum pt_tls_sasl_result_t pt_tls_sasl_result_t;
typedef enum pt_tls_auth_t pt_tls_auth_t;

/**
 * Message types, as defined by NEA PT-TLS
 */
enum pt_tls_message_type_t {
	PT_TLS_EXPERIMENTAL = 0,
	PT_TLS_VERSION_REQUEST = 1,
	PT_TLS_VERSION_RESPONSE = 2,
	PT_TLS_SASL_MECHS = 3,
	PT_TLS_SASL_MECH_SELECTION = 4,
	PT_TLS_SASL_AUTH_DATA = 5,
	PT_TLS_SASL_RESULT = 6,
	PT_TLS_PB_TNC_BATCH = 7,
	PT_TLS_ERROR = 8,
};

extern enum_name_t *pt_tls_message_type_names;

/**
 * Result code for a single SASL mechanism, as sent in PT_TLS_SASL_RESULT
 */
enum pt_tls_sasl_result_t {
	PT_TLS_SASL_RESULT_SUCCESS = 0,
	PT_TLS_SASL_RESULT_FAILURE = 1,
	PT_TLS_SASL_RESULT_ABORT = 2,
	PT_TLS_SASL_RESULT_MECH_FAILURE = 3,
};

extern enum_name_t *pt_tls_sasl_result_names;

/**
 * Client authentication to require as PT-TLS server.
 */
enum pt_tls_auth_t {
	/** don't require TLS client certificate or request SASL authentication */
	PT_TLS_AUTH_NONE,
	/** require TLS certificate authentication, no SASL */
	PT_TLS_AUTH_TLS,
	/** do SASL regardless of TLS certificate authentication */
	PT_TLS_AUTH_SASL,
	/* if client does not authenticate with a TLS certificate, request SASL */
	PT_TLS_AUTH_TLS_OR_SASL,
	/* require both, TLS certificate authentication and SASL */
	PT_TLS_AUTH_TLS_AND_SASL,
};

/**
 * Read a PT-TLS message, create reader over Message Value.
 *
 * @param tls			TLS socket to read from
 * @param vendor		receives Message Type Vendor ID from header
 * @param type			receives Message Type from header
 * @param identifier	receives Message Identifier
 * @return				reader over message value, NULL on error
 */
bio_reader_t* pt_tls_read(tls_socket_t *tls, uint32_t *vendor,
						  uint32_t *type, uint32_t *identifier);

/**
 * Prepend a PT-TLS header to a writer, send data, destroy writer.
 *
 * @param tls			TLS socket to write to
 * @param type			Message Type to write
 * @param identifier	Message Identifier to write
 * @param data			Message value to write
 * @return				TRUE if data written successfully
 */
bool pt_tls_write(tls_socket_t *tls, pt_tls_message_type_t type,
				  uint32_t identifier, chunk_t data);

/**
 * Dummy libpttls initialization function needed for integrity test
 */
void libpttls_init(void);

#endif /** PT_TLS_H_ @}*/
