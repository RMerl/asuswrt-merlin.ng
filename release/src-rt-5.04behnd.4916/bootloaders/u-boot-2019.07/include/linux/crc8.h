/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 */


#ifndef __linux_crc8_h
#define __linux_crc8_h

/**
 * crc8() - Calculate and return CRC-8 of the data
 *
 * This uses an x^8 + x^2 + x + 1 polynomial.  A table-based algorithm would
 * be faster, but for only a few bytes it isn't worth the code size
 *
 * @crc_start: CRC8 start value
 * @vptr: Buffer to checksum
 * @len: Length of buffer in bytes
 * @return CRC8 checksum
 */
unsigned int crc8(unsigned int crc_start, const unsigned char *vptr, int len);

#endif
