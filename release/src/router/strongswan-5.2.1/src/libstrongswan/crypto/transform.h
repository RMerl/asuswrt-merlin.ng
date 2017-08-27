/*
 * Copyright (C) 2006-2009 Martin Willi
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
 * @defgroup transforms transforms
 * @{ @ingroup crypto
 */

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

typedef enum transform_type_t transform_type_t;

#include <utils/utils.h>

/**
 * Type of a transform, as in IKEv2 RFC 3.3.2.
 */
enum transform_type_t {
	UNDEFINED_TRANSFORM_TYPE = 241,
	HASH_ALGORITHM = 242,
	RANDOM_NUMBER_GENERATOR = 243,
	AEAD_ALGORITHM = 244,
	COMPRESSION_ALGORITHM = 245,
	ENCRYPTION_ALGORITHM = 1,
	PSEUDO_RANDOM_FUNCTION = 2,
	INTEGRITY_ALGORITHM = 3,
	DIFFIE_HELLMAN_GROUP = 4,
	EXTENDED_SEQUENCE_NUMBERS = 5
};

/**
 * enum names for transform_type_t.
 */
extern enum_name_t *transform_type_names;

/**
 * Extended sequence numbers, as in IKEv2 RFC 3.3.2.
 */
enum extended_sequence_numbers_t {
	NO_EXT_SEQ_NUMBERS = 0,
	EXT_SEQ_NUMBERS = 1
};

/**
 * enum strings for extended_sequence_numbers_t.
 */
extern enum_name_t *extended_sequence_numbers_names;

#endif /** TRANSFORM_H_ @}*/
