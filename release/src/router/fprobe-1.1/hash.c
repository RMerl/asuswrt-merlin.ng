/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: hash.c,v 1.3.2.3 2005/01/29 19:28:04 sla Exp $
*/

#include <common.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <hash.h>


#ifdef HASH_TYPE_CRC
static uint16_t crc16_poly;
static uint8_t shuffle_table[256];
static uint16_t crc16_table[256];

uint16_t crc16(uint16_t crc, uint8_t val)
{
	int i;

	crc ^= val << 8;
	for (i = 8; i--; )
		crc = crc & 0x8000 ? (crc << 1) ^ crc16_poly : crc << 1;
	return crc;
}
#endif

hash_t hash(void *p, int size)
{
	/*
	?FIXME?
	Check for valid size (> 0)
	*/

	hash_t hash = 0;

#if defined HASH_TYPE_XOR && HASH_BITS == 16
	if (size & 1) hash = *((uint8_t *) p++);
	size >>= 1;
#endif

	for (;size--;) {
#ifdef HASH_TYPE_XOR
		hash ^= *((hash_t *) p++);
# if HASH_BITS == 16
		p++;
# endif
#endif
#ifdef HASH_TYPE_CRC
		hash = crc16_table[shuffle_table[*((uint8_t *) p++)] \
			^ (hash >> 8)] ^ (hash << 8);
#endif
	}
	return hash;
}

void hash_init()
{

#ifdef HASH_TYPE_CRC
	int rnd, i, j, m;
	FILE *rnddev;

	if ((rnddev = fopen(RNDDEV, "r"))) {
		fcntl(fileno(rnddev), F_SETFL, \
			fcntl(fileno(rnddev), F_GETFL) | O_NONBLOCK);
		fread(&rnd, sizeof(rnd), 1, rnddev);
		fclose(rnddev);
	}
	srand(time(NULL) ^ getpid() ^ rnd);
	crc16_poly = rand() | 1;

	for (i = 0; i < 256; i++) {
	    crc16_table[i] = crc16(0, i);
	    shuffle_table[i] = i;
	}

	for (i = 0; i < 256; i++) {
	    j = (int) (256.0 * rand() / (RAND_MAX + 1.0));
	    m = shuffle_table[i];
	    shuffle_table[i] = shuffle_table[j];
	    shuffle_table[j] = m;
	}
#endif
}
