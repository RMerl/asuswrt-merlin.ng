/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: hash.h,v 1.1.1.1.2.4 2005/01/29 19:28:04 sla Exp $
*/

#ifndef _HASH_H_
#define _HASH_H_

#ifndef HASH_TYPE_XOR
# ifndef HASH_TYPE_CRC
#  error HASH_TYPE_XOR or HASH_TYPE_CRC must be defined
# endif
#else
# ifdef HASH_TYPE_CRC
#  error HASH_TYPE_XOR or HASH_TYPE_CRC must be defined
# endif
#endif

#if HASH_BITS != 16
# ifdef HASH_TYPE_CRC
#  error illegal value in HASH_BITS
# endif
# if HASH_BITS != 8
#  error illegal value in HASH_BITS
# endif
#endif

#include <my_inttypes.h>

#if HASH_BITS == 8
typedef uint8_t hash_t;
#endif

#if HASH_BITS == 16
typedef uint16_t hash_t;
#endif

#define CRC16_POLY  0x8005

#ifndef RNDDEV
# define RNDDEV "/dev/random"
#endif

uint16_t crc16(uint16_t, uint8_t);
hash_t hash(void *, int);
void hash_init();

#endif
