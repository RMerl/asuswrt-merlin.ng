/*
 * Copyright (C) 2012-2019 Tobias Brunner
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
	ADDITIONAL_KEY_EXCHANGE_1 = 6,
	ADDITIONAL_KEY_EXCHANGE_2 = 7,
	ADDITIONAL_KEY_EXCHANGE_3 = 8,
	ADDITIONAL_KEY_EXCHANGE_4 = 9,
	ADDITIONAL_KEY_EXCHANGE_5 = 10,
	ADDITIONAL_KEY_EXCHANGE_6 = 11,
	ADDITIONAL_KEY_EXCHANGE_7 = 12,
	HASH_ALGORITHM = 256,
	RANDOM_NUMBER_GENERATOR = 257,
	AEAD_ALGORITHM = 258,
	COMPRESSION_ALGORITHM = 259,
	EXTENDED_OUTPUT_FUNCTION = 260,
	DETERMINISTIC_RANDOM_BIT_GENERATOR = 261,
	KEY_DERIVATION_FUNCTION = 262,
};

/**
 * Maximum number of additional key exchanges.
 */
#define MAX_ADDITIONAL_KEY_EXCHANGES (ADDITIONAL_KEY_EXCHANGE_7 - \
									  ADDITIONAL_KEY_EXCHANGE_1 + 1)

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
 * Check if the given transform type is used to negotiate a key exchange.
 *
 * @param type		type of transform to check
 * @return			TRUE if the transform type negotiates a key exchange
 */
static inline bool is_ke_transform(transform_type_t type)
{
	return type == KEY_EXCHANGE_METHOD || (ADDITIONAL_KEY_EXCHANGE_1 <= type &&
										   type <= ADDITIONAL_KEY_EXCHANGE_7);
}

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
