/*
 * Copyright (C) 2006-2009 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
	ENCRYPTION_ALGORITHM = 1,
	PSEUDO_RANDOM_FUNCTION = 2,
	INTEGRITY_ALGORITHM = 3,
	KEY_EXCHANGE_METHOD = 4,
	EXTENDED_SEQUENCE_NUMBERS = 5,
	HASH_ALGORITHM = 256,
	RANDOM_NUMBER_GENERATOR = 257,
	AEAD_ALGORITHM = 258,
	COMPRESSION_ALGORITHM = 259,
	EXTENDED_OUTPUT_FUNCTION = 260,
	DETERMINISTIC_RANDOM_BIT_GENERATOR = 261,
	KEY_DERIVATION_FUNCTION = 262,
};

/**
 * enum names for transform_type_t.
 */
extern enum_name_t *transform_type_names;

/**
 * Get the enum names for a specific transform type.
 *
 * @param type		type of transform to get enum names for
 * @return			enum names
 */
enum_name_t *transform_get_enum_names(transform_type_t type);

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
