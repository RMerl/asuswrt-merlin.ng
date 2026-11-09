/*
 * Copyright (C) 2024 Tobias Brunner
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
 * @defgroup ml_utils ml_utils
 * @{ @ingroup ml_p
 */

#ifndef ML_UTILS_H_
#define ML_UTILS_H_

#include "ml_params.h"

/**
 * Returns a mod q for a in [0,2*q) in constant time.
 *
 * @param a			value to reduce mod q
 * @return			reduced value
 */
static inline uint16_t ml_reduce_modq(uint16_t a)
{
	const uint16_t diff = a - ML_KEM_Q;
	uint16_t mask = 0 - (diff >> 15);

	return (mask & a) | (~mask & diff);
}

/**
 * Used to assign the given value based on a condition in constant time and
 * without branching.
 *
 * @param dst		the value to set
 * @param val		value to set if condition is 1
 * @param cond		1 to set the value, 0 to leave as is
 */
void ml_assign_cond_int16(int16_t *dst, int16_t val, uint16_t cond);

/**
 * Read up to four bytes in little-endian order from the given buffer.
 *
 * @param buf		byte buffer to read from
 * @param len		length between 0 and 4
 * @return			read value
 */
uint32_t ml_read_bytes_le(uint8_t *buf, size_t len);

/**
 * Write up to four bytes of the given value in little-endian order to a buffer.
 *
 * @param buf		byte buffer to write to
 * @param len		length between 0 and 4
 * @param val		value to write
 */
void ml_write_bytes_le(uint8_t *buf, size_t len, uint32_t val);

#endif /** ML_UTILS_H_ @}*/
