/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "crc.h"

uint32_t crc24_bit_reverse(uint32_t value)
{
	uint32_t result = 0;
	uint8_t i;

	for (i = 0; i < 24; i++)
		result |= ((value >> i) & 1) << (23 - i);

	return result;
}

uint32_t crc24_calculate(uint32_t preset, const uint8_t *data, uint8_t len)
{
	uint32_t state = preset;
	uint8_t i;

	for (i = 0; i < len; i++) {
		uint8_t n, cur = data[i];

		for (n = 0; n < 8; n++) {
			int next_bit = (state ^ cur) & 1;

			cur >>= 1;
			state >>= 1;
			if (next_bit) {
				state |= 1 << 23;
				state ^= 0x5a6000;
			}
		}
	}

	return state;
}

uint32_t crc24_reverse(uint32_t crc, const uint8_t *data, uint8_t len)
{
	uint32_t state = crc;
	uint8_t i;

	for (i = 0; i < len; i++) {
		uint8_t n, cur = data[len - i - 1];

		for (n = 0; n < 8; n++) {
			int top_bit = state >> 23;

			state = (state << 1) & 0xffffff;
			state |= top_bit ^ ((cur >> (7 - n)) & 1);
			if (top_bit)
				state ^= 0xb4c000;
		}
	}

	return state;
}
