// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2014 Rockchip Electronics
 *
 * Rivest Cipher 4 (RC4) implementation
 */

#ifndef USE_HOSTCC
#include <common.h>
#endif
#include <rc4.h>

void rc4_encode(unsigned char *buf, unsigned int len, unsigned char key[16])
{
	unsigned char s[256], k[256], temp;
	unsigned short i, j, t;
	int ptr;

	j = 0;
	for (i = 0; i < 256; i++) {
		s[i] = (unsigned char)i;
		j &= 0x0f;
		k[i] = key[j];
		j++;
	}

	j = 0;
	for (i = 0; i < 256; i++) {
		j = (j + s[i] + k[i]) % 256;
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
	}

	i = 0;
	j = 0;
	for (ptr = 0; ptr < len; ptr++) {
		i = (i + 1) % 256;
		j = (j + s[i]) % 256;
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
		t = (s[i] + (s[j] % 256)) % 256;
		buf[ptr] = buf[ptr] ^ s[t];
	}
}
