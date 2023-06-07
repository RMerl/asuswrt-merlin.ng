/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdint.h>

uint32_t crc24_bit_reverse(uint32_t value);

uint32_t crc24_calculate(uint32_t preset, const uint8_t *data, uint8_t len);
uint32_t crc24_reverse(uint32_t crc, const uint8_t *data, uint8_t len);
