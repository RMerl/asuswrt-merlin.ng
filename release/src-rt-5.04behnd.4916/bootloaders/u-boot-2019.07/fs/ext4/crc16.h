/*
 * crc16.h - CRC-16 routine
 * Implements the standard CRC-16:
 *  Width 16
 *  Poly  0x8005 (x16 + x15 + x2 + 1)
 *  Init  0
 *
 * Copyright (c) 2005 Ben Gardner <bgardner@wabtec.com>
 * This source code is licensed under the GNU General Public License,
 * Version 2. See the file COPYING for more details.
 */
#ifndef __CRC16_H
#define __CRC16_H
extern unsigned int ext2fs_crc16(unsigned int crc,
	const void *buffer, unsigned int len);
#endif
