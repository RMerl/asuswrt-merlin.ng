/*
 * hmac.c	For the sake of illustration we provide the following
 *		sample code for the implementation of HMAC-MD5 as well
 *		as some corresponding test vectors (the code is based
 *		on MD5 code as described in [MD5]).
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 */

/*
** Function: fr_hmac_md5
*/

RCSID("$Id$")

#include <freeradius-devel/libradius.h>
#include <freeradius-devel/md5.h>

/** Calculate HMAC using MD5
 *
 * @param text Pointer to data stream.
 * @param text_len length of data stream.
 * @param key Pointer to authentication key.
 * @param key_len Length of authentication key.
 * @param digest Caller digest to be filled in.
 */
void fr_hmac_md5(uint8_t const *text, size_t text_len, uint8_t const *key, size_t key_len, uint8_t *digest)
{
	FR_MD5_CTX context;
	uint8_t k_ipad[65];    /* inner padding - key XORd with ipad */
	uint8_t k_opad[65];    /* outer padding - key XORd with opad */
	uint8_t tk[16];
	int i;

	/* if key is longer than 64 bytes reset it to key=MD5(key) */
	if (key_len > 64) {
		FR_MD5_CTX tctx;

		fr_MD5Init(&tctx);
		fr_MD5Update(&tctx, key, key_len);
		fr_MD5Final(tk, &tctx);

		key = tk;
		key_len = 16;
	}

	/*
	 * the HMAC_MD5 transform looks like:
	 *
	 * MD5(K XOR opad, MD5(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times

	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected
	 */

	/* start out by storing key in pads */
	memset( k_ipad, 0, sizeof(k_ipad));
	memset( k_opad, 0, sizeof(k_opad));
	memcpy( k_ipad, key, key_len);
	memcpy( k_opad, key, key_len);

	/* XOR key with ipad and opad values */
	for (i = 0; i < 64; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}
	/*
	 * perform inner MD5
	 */
	fr_MD5Init(&context);		   /* init context for 1st
					      * pass */
	fr_MD5Update(&context, k_ipad, 64);      /* start with inner pad */
	fr_MD5Update(&context, text, text_len); /* then text of datagram */
	fr_MD5Final(digest, &context);	  /* finish up 1st pass */
	/*
	 * perform outer MD5
	 */
	fr_MD5Init(&context);		   /* init context for 2nd
					      * pass */
	fr_MD5Update(&context, k_opad, 64);     /* start with outer pad */
	fr_MD5Update(&context, digest, 16);     /* then results of 1st
					      * hash */
	fr_MD5Final(digest, &context);	  /* finish up 2nd pass */
}

/*
Test Vectors (Trailing '\0' of a character string not included in test):

  key =	 0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b
  key_len =     16 bytes
  data =	"Hi There"
  data_len =    8  bytes
  digest =      0x9294727a3638bb1c13f48ef8158bfc9d

  key =	 "Jefe"
  data =	"what do ya want for nothing?"
  data_len =    28 bytes
  digest =      0x750c783e6ab0b503eaa86e310a5db738

  key =	 0xAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

  key_len       16 bytes
  data =	0xDDDDDDDDDDDDDDDDDDDD...
		..DDDDDDDDDDDDDDDDDDDD...
		..DDDDDDDDDDDDDDDDDDDD...
		..DDDDDDDDDDDDDDDDDDDD...
		..DDDDDDDDDDDDDDDDDDDD
  data_len =    50 bytes
  digest =      0x56be34521d144c88dbb8c733f0e8b3f6
*/

#ifdef TESTING
/*
 *  cc -DTESTING -I ../include/ hmac.c md5.c -o hmac
 *
 *  ./hmac Jefe "what do ya want for nothing?"
 */
int main(int argc, char **argv)
{
  uint8_t digest[16];
  char *key;
  int key_len;
  char *text;
  int text_len;
  int i;

  key = argv[1];
  key_len = strlen(key);

  text = argv[2];
  text_len = strlen(text);

  fr_hmac_md5(text, text_len, key, key_len, digest);

  for (i = 0; i < 16; i++) {
    printf("%02x", digest[i]);
  }
  printf("\n");

  exit(0);
  return 0;
}

#endif
