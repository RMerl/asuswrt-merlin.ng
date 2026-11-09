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

#include "ml_utils.h"

/*
 * Described in header
 */
void ml_assign_cond_int16(int16_t *dst, int16_t val, uint16_t cond)
{
	cond = -cond;
	*dst ^= cond & (val ^ *dst);
}

/*
 * Described in header
 */
uint32_t ml_read_bytes_le(uint8_t *buf, size_t len)
{
	uint32_t x = 0;
	int i;

	for (i = 0; i < len; i++)
	{
		x |= (uint32_t)buf[i] << (8 * i);
	}
	return x;
}

/*
 * Described in header
 */
void ml_write_bytes_le(uint8_t *buf, size_t len, uint32_t val)
{
	int i;

	for (i = 0; i < len; i++)
	{
		buf[i] = val;
		val >>= 8;
	}
}
