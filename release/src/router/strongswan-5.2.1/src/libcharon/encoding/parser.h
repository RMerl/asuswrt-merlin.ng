/*
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
 * @defgroup parser parser
 * @{ @ingroup encoding
 */

#ifndef PARSER_H_
#define PARSER_H_

typedef struct parser_t parser_t;

#include <library.h>
#include <encoding/payloads/encodings.h>
#include <encoding/payloads/payload.h>

/**
 * A parser_t class to parse IKEv2 payloads.
 *
 * A parser is used for parsing one chunk of data. Multiple
 * payloads can be parsed out of the chunk using parse_payload.
 * The parser remains the state until destroyed.
 */
struct parser_t {

	/**
	 * Parses the next payload.
	 *
	 * @warning Caller is responsible for freeing allocated payload.
	 *
	 * Rules for parsing are described in the payload definition.
	 *
	 * @param payload_type	payload type to parse
	 * @param payload		pointer where parsed payload was allocated
	 * @return
	 * 						- SUCCESSFUL if succeeded,
	 * 						- PARSE_ERROR if corrupted/invalid data found
	 */
	status_t (*parse_payload) (parser_t *this, payload_type_t payload_type, payload_t **payload);

	/**
	 * Gets the remaining byte count which is not currently parsed.
	 */
	int (*get_remaining_byte_count) (parser_t *this);

	/**
	 * Resets the current parser context.
	 */
	void (*reset_context) (parser_t *this);

	/**
	 * Destroys a parser_t object.
	 */
	void (*destroy) (parser_t *this);
};

/**
 * Constructor to create a parser_t object.
 *
 * @param data		chunk of data to parse with this parser_t object
 * @return 			parser_t object
 */
parser_t *parser_create(chunk_t data);

#endif /** PARSER_H_ @}*/
