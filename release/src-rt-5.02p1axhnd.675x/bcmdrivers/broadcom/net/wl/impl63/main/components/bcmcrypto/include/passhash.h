/*
 * passhash.h
 * Perform password to key hash algorithm as defined in WPA and 802.11i
 * specifications.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: passhash.h 691062 2017-03-20 18:53:37Z $
 */

#ifndef _PASSHASH_H_
#define _PASSHASH_H_

#include <typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif // endif

/* passhash: perform passwork to key hash algorithm as defined in WPA and 802.11i
 * specifications.
 *
 *	password is an ascii string of 8 to 63 characters in length
 *	ssid is up to 32 bytes
 *	ssidlen is the length of ssid in bytes
 *	output must be at lest 40 bytes long, and returns a 256 bit key
 *	returns 0 on success, non-zero on failure
 */
int passhash(char *password, int passlen, unsigned char *ssid, int ssidlen,
                              unsigned char *output);

/* init_passhash/do_passhash/get_passhash: perform passwork to key hash algorithm
 * as defined in WPA and 802.11i specifications, and break lengthy calculation into
 * smaller pieces.
 *
 *	password is an ascii string of 8 to 63 characters in length
 *	ssid is up to 32 bytes
 *	ssidlen is the length of ssid in bytes
 *	output must be at lest 40 bytes long, and returns a 256 bit key
 *	returns 0 on success, negative on failure.
 *
 *	Allocate passhash_t and call init_passhash() to initialize it before
 *	calling do_passhash(), and don't release password and ssid until passhash
 *	is done.
 *	Call do_passhash() to request and perform # iterations. do_passhash()
 *	returns positive value to indicate it is in progress, so continue to
 *	call it until it returns 0 which indicates a success.
 *	Call get_passhash() to get the hash value when do_passhash() is done.
 */
#include <sha2.h>

typedef struct {
	unsigned char digest[SHA2_SHA1_DIGEST_LEN];	/* Un-1 */
	int count;				/* Count */
	unsigned char output[2*SHA2_SHA1_DIGEST_LEN];	/* output */
	char *password;
	int passlen;
	unsigned char *ssid;
	int ssidlen;
	int iters;
} passhash_t;

int init_passhash(passhash_t *ph,
                         char *password, int passlen, unsigned char *ssid, int ssidlen);
int do_passhash(passhash_t *ph, int iterations);
int get_passhash(passhash_t *ph, unsigned char *output, int outlen);

/* PBKDF2 - see https://tools.ietf.org/html/rfc8018 */
void PBKDF2i(sha2_hash_type_t hash_type,  const char *pass, int pass_len,
	const uint8 *salt, int salt_len, int niter, uint32 i, uint8 *out, int out_len);

#ifdef __cplusplus
}
#endif // endif
#endif /* _PASSHASH_H_ */
