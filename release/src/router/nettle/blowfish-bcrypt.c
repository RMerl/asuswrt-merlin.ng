/* blowfish-bcrypt.c

   The blowfish bcrypt implementation.

   Copyright (c) 2020 Stephen R. van den Berg

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "blowfish.h"
#include "blowfish-internal.h"
#include "base64.h"

#include "macros.h"

#define CRYPTPLEN 7
#define SALTLEN ((BLOWFISH_BCRYPT_BINSALT_SIZE*8+5) / 6)

#define HASHOFFSET (CRYPTPLEN + SALTLEN)

static const signed char radix64_decode_table[0x100] = {
  /* White space is HT, VT, FF, CR, LF and SPC */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -2, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  1,
  54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1, -1, -1, -3, -1, -1,
  -1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, -1, -1, -1, -1, -1,
  -1, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
  43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static const char radix64_encode_table[64] =
  "./ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";

int
blowfish_bcrypt_verify(size_t lenkey, const uint8_t *key,
                       size_t lenhashed, const uint8_t *hashed)
{
  uint8_t newhash[BLOWFISH_BCRYPT_HASH_SIZE];

  return blowfish_bcrypt_hash(newhash,
                              lenkey, key, lenhashed, hashed,
                              -1, (void*)0)
   && !strcmp((const char*)newhash, (const char*)hashed);
}

static char *encode_radix64(char *dst, size_t len, const uint8_t *src)
{
  struct base64_encode_ctx ctx;
  base64_encode_init(&ctx);
  ctx.alphabet = radix64_encode_table;
  dst += base64_encode_update(&ctx, dst, len, src);
  dst += base64_encode_final(&ctx, dst);
  *--dst = '\0';	    /* Strip the trailing = */
  return dst;
}

/*
 * Large parts of the code below are based on public domain sources.
 * The comments and copyright notices have been preserved.
 * Any code added or modified by me is licensed under the
 * licenses listed above.  --  Stephen R. van den Berg
 */

/*
 * This code comes from John the Ripper password cracker, with reentrant
 * and crypt(3) interfaces added, but optimizations specific to password
 * cracking removed.
 *
 * Written by Solar Designer <solar at openwall.com> in 1998-2015.
 * No copyright is claimed, and the software is hereby placed in the public
 * domain. In case this attempt to disclaim copyright and place the software
 * in the public domain is deemed null and void, then the software is
 * Copyright (c) 1998-2015 Solar Designer and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * It is my intent that you should be able to use this on your system,
 * as part of a software package, or anywhere else to improve security,
 * ensure compatibility, or for any other purpose. I would appreciate
 * it if you give credit where it is due and keep your modifications in
 * the public domain as well, but I don't require that in order to let
 * you place this code and any modifications you make under a license
 * of your choice.
 *
 * This implementation is fully compatible with OpenBSD's bcrypt.c for prefix
 * "$2b$", originally by Niels Provos <provos at citi.umich.edu>, and it uses
 * some of his ideas. The password hashing algorithm was designed by David
 * Mazieres <dm at lcs.mit.edu>. For information on the level of
 * compatibility for bcrypt hash prefixes other than "$2b$", please refer to
 * the comments in set_key() below and to the included crypt(3) man page.
 */

typedef uint32_t bf_key[_BLOWFISH_ROUNDS + 2];

/*
 * Magic IV for 64 Blowfish encryptions that we do at the end.
 * The string is "OrpheanBeholderScryDoubt" on big-endian.
 */
static uint32_t magic_w[6] = {
  0x4F727068, 0x65616E42, 0x65686F6C,
  0x64657253, 0x63727944, 0x6F756274
};

static void swap32(uint32_t *x, int count)
{
#if !WORDS_BIGENDIAN
  do {
    uint32_t tmp = *x;
    tmp = (tmp << 16) | (tmp >> 16);
    *x++ = ((tmp & 0x00FF00FF) << 8) | ((tmp >> 8) & 0x00FF00FF);
  } while (--count);
#endif
}

static void set_xkey(size_t lenkey, const uint8_t *key,
                     bf_key expanded, bf_key initial,
		     unsigned bug, uint32_t safety)
{
  const uint8_t *ptr = key;
  size_t n = lenkey;
  unsigned i, j;
  uint32_t sign, diff, tmp[2];

/*
 * There was a sign extension bug in older revisions of this function. While
 * we would have liked to simply fix the bug and move on, we have to provide
 * a backwards compatibility feature (essentially the bug) for some systems and
 * a safety measure for some others. The latter is needed because for certain
 * multiple inputs to the buggy algorithm there exist easily found inputs to
 * the correct algorithm that produce the same hash. Thus, we optionally
 * deviate from the correct algorithm just enough to avoid such collisions.
 * While the bug itself affected the majority of passwords containing
 * characters with the 8th bit set (although only a percentage of those in a
 * collision-producing way), the anti-collision safety measure affects
 * only a subset of passwords containing the '\xff' character (not even all of
 * those passwords, just some of them). This character is not found in valid
 * UTF-8 sequences and is rarely used in popular 8-bit character encodings.
 * Thus, the safety measure is unlikely to cause much annoyance, and is a
 * reasonable tradeoff to use when authenticating against existing hashes that
 * are not reliably known to have been computed with the correct algorithm.
 *
 * We use an approach that tries to minimize side-channel leaks of password
 * information - that is, we mostly use fixed-cost bitwise operations instead
 * of branches or table lookups. (One conditional branch based on password
 * length remains. It is not part of the bug aftermath, though, and is
 * difficult and possibly unreasonable to avoid given the use of C strings by
 * the caller, which results in similar timing leaks anyway.)
 *
 * For actual implementation, we set an array index in the variable "bug"
 * (0 means no bug, 1 means sign extension bug emulation) and a flag in the
 * variable "safety" (bit 16 is set when the safety measure is requested).
 * Valid combinations of settings are:
 *
 * Prefix "$2a$": bug = 0, safety = 0x10000
 * Prefix "$2b$": bug = 0, safety = 0
 * Prefix "$2x$": bug = 1, safety = 0
 * Prefix "$2y$": bug = 0, safety = 0
 */

  sign = diff = 0;

  for (i = 0; i < _BLOWFISH_ROUNDS + 2; i++) {
    tmp[0] = tmp[1] = 0;
    for (j = 0; j < 4; j++) {
      tmp[0] <<= 8;
      tmp[0] |= (unsigned char)*ptr; /* correct */
      tmp[1] <<= 8;
      tmp[1] |= (signed char)*ptr; /* bug */
/*
 * Sign extension in the first char has no effect - nothing to overwrite yet,
 * and those extra 24 bits will be fully shifted out of the 32-bit word. For
 * chars 2, 3, 4 in each four-char block, we set bit 7 of "sign" if sign
 * extension in tmp[1] occurs. Once this flag is set, it remains set.
 */
      if (j)
        sign |= tmp[1] & 0x80;
      if (n--)
        ptr++;
      else
        ptr = key, n = lenkey;
    }
    diff |= tmp[0] ^ tmp[1]; /* Non-zero on any differences */

    expanded[i] = tmp[bug];
    initial[i] = _nettle_blowfish_initial_ctx.p[i] ^ tmp[bug];
  }

/*
 * At this point, "diff" is zero if the correct and buggy algorithms produced
 * exactly the same result. If so and if "sign" is non-zero, which indicates
 * that there was a non-benign sign extension, this means that we have a
 * collision between the correctly computed hash for this password and a set of
 * passwords that could be supplied to the buggy algorithm. Our safety measure
 * is meant to protect from such many-buggy to one-correct collisions, by
 * deviating from the correct algorithm in such cases. Let's check for this.
 */
  diff |= diff >> 16; /* still zero if exact match */
  diff &= 0xffff; /* ditto */
  diff += 0xffff; /* bit 16 set if "diff" was non-zero (on non-match) */
  sign <<= 9; /* move the non-benign sign extension flag to bit 16 */
  sign &= ~diff & safety; /* action needed? */

/*
 * If we have determined that we need to deviate from the correct algorithm,
 * flip bit 16 in initial expanded key. (The choice of 16 is arbitrary, but
 * let's stick to it now. It came out of the approach we used above, and it's
 * not any worse than any other choice we could make.)
 *
 * It is crucial that we don't do the same to the expanded key used in the main
 * Eksblowfish loop. By doing it to only one of these two, we deviate from a
 * state that could be directly specified by a password to the buggy algorithm
 * (and to the fully correct one as well, but that's a side-effect).
 */
  initial[0] ^= sign;
}

static int ibcrypt(uint8_t *dst,
                   size_t lenkey, const uint8_t *key,
		   size_t lenscheme, const uint8_t *scheme,
		   int minlog2rounds,
		   int log2rounds, const uint8_t *salt)
{
  struct {
    struct blowfish_ctx ctx;
    bf_key expanded_key;
    union {
      uint32_t salt[4];
      uint32_t output[6];
    } binary;
  } data;
  uint8_t psalt[BLOWFISH_BCRYPT_BINSALT_SIZE];
  uint32_t L, R;
  uint32_t *ptr;
  uint32_t count;
  int i;
  unsigned cscheme;
  unsigned bug = 0;
  uint32_t safety = 0;
  if (lenscheme < 2)
    return 0;

  if (lenscheme >= 3 && *scheme++ != '$')
    return 0;
  if (*scheme++ != '2')
    return 0;

  switch (cscheme = *scheme++) {
    default:
      return 0;
    case 'a': safety = 0x10000;
      break;
    case 'x': bug = 1;
      break;
    case 'b': case 'y':
      break;
  }

  if (lenscheme >= 4) {
    if (*scheme++ != '$')
      return 0;
    if (lenscheme >= 6) {
      if (log2rounds < 0) {
        unsigned c = *scheme++ - '0';
	if (c > 9)
	  return 0;
	log2rounds = c * 10;
        c = *scheme++ - '0';
	if (c > 9)
	  return 0;
	log2rounds += c;
      } else
        scheme += 2;
      if (lenscheme >= CRYPTPLEN && *scheme++ != '$')
	return 0;
      if (lenscheme >= HASHOFFSET && !salt) {
        struct base64_decode_ctx ctx;
        size_t saltlen = BLOWFISH_BCRYPT_BINSALT_SIZE;

        base64_decode_init(&ctx);
        ctx.table = radix64_decode_table;

        if (!base64_decode_update(&ctx, &saltlen, (uint8_t *) data.binary.salt,
                                  SALTLEN, (const char*) scheme)
         || saltlen != BLOWFISH_BCRYPT_BINSALT_SIZE)
          return 0;
      }
    }
  }

  if (salt)
    memcpy(data.binary.salt, salt, BLOWFISH_BCRYPT_BINSALT_SIZE);
  else if (lenscheme < HASHOFFSET)
    return 0;
  memcpy(psalt, data.binary.salt, BLOWFISH_BCRYPT_BINSALT_SIZE);
  swap32(data.binary.salt, 4);

  if (log2rounds < minlog2rounds || log2rounds > 31)
    return 0;
  count = (uint32_t)1 << log2rounds;

  set_xkey(lenkey, key, data.expanded_key, data.ctx.p, bug, safety);
  memcpy(data.ctx.s, _nettle_blowfish_initial_ctx.s, sizeof(data.ctx.s));

  L = R = 0;
  for (i = 0; i < _BLOWFISH_ROUNDS + 2; i += 2) {
    L ^= data.binary.salt[i & 2];
    R ^= data.binary.salt[(i & 2) + 1];
    _nettle_blowfish_encround(&data.ctx, &L, &R);
    data.ctx.p[i] = L;
    data.ctx.p[i + 1] = R;
  }

  ptr = data.ctx.s[0];
  do {
    ptr += 4;
    L ^= data.binary.salt[(_BLOWFISH_ROUNDS + 2) & 3];
    R ^= data.binary.salt[(_BLOWFISH_ROUNDS + 3) & 3];
    _nettle_blowfish_encround(&data.ctx, &L, &R);
    *(ptr - 4) = L;
    *(ptr - 3) = R;

    L ^= data.binary.salt[(_BLOWFISH_ROUNDS + 4) & 3];
    R ^= data.binary.salt[(_BLOWFISH_ROUNDS + 5) & 3];
    _nettle_blowfish_encround(&data.ctx, &L, &R);
    *(ptr - 2) = L;
    *(ptr - 1) = R;
  } while (ptr < &data.ctx.s[3][0xFF]);

  do {
    int done;

    for (i = 0; i < _BLOWFISH_ROUNDS + 2; i += 2) {
      data.ctx.p[i] ^= data.expanded_key[i];
      data.ctx.p[i + 1] ^= data.expanded_key[i + 1];
    }

    done = 0;
    do {
      uint32_t tmp1, tmp2, tmp3, tmp4;

      L = R = 0;
      ptr = data.ctx.p;
      do {
        ptr += 2;
        _nettle_blowfish_encround(&data.ctx, &L, &R);
        *(ptr - 2) = L;
        *(ptr - 1) = R;
      } while (ptr < &data.ctx.p[_BLOWFISH_ROUNDS + 2]);

      ptr = data.ctx.s[0];
      do {
        ptr += 2;
        _nettle_blowfish_encround(&data.ctx, &L, &R);
        *(ptr - 2) = L;
        *(ptr - 1) = R;
      } while (ptr < &data.ctx.s[3][0xFF]);

      if (done)
        break;
      done = 1;

      tmp1 = data.binary.salt[0];
      tmp2 = data.binary.salt[1];
      tmp3 = data.binary.salt[2];
      tmp4 = data.binary.salt[3];
      for (i = 0; i < _BLOWFISH_ROUNDS; i += 4) {
        data.ctx.p[i] ^= tmp1;
        data.ctx.p[i + 1] ^= tmp2;
        data.ctx.p[i + 2] ^= tmp3;
        data.ctx.p[i + 3] ^= tmp4;
      }
      data.ctx.p[16] ^= tmp1;
      data.ctx.p[17] ^= tmp2;
    } while (1);
  } while (--count);

  for (i = 0; i < 6; i += 2) {
    L = magic_w[i];
    R = magic_w[i + 1];

    count = 64;
    do
      _nettle_blowfish_encround(&data.ctx, &L, &R);
    while (--count);

    data.binary.output[i] = L;
    data.binary.output[i + 1] = R;
  }

  *dst++ = '$';
  *dst++ = '2';
  *dst++ = cscheme;
  *dst++ = '$';
  *dst++ = '0' + log2rounds / 10;
  *dst++ = '0' + log2rounds % 10;
  *dst++ = '$';
  dst = (uint8_t*)
        encode_radix64((char*) dst, BLOWFISH_BCRYPT_BINSALT_SIZE, psalt) - 1;

  swap32(data.binary.output, 6);
/* This has to be bug-compatible with the original implementation, so
   only encode 23 of the 24 bytes. */
  encode_radix64((char*) dst, 23, (uint8_t *) data.binary.output);
  return cscheme;
}

/*
 * Please preserve the runtime self-test. It serves two purposes at once:
 *
 * 1. We really can't afford the risk of producing incompatible hashes e.g.
 * when there's something like gcc bug 26587 again, whereas an application or
 * library integrating this code might not also integrate our external tests or
 * it might not run them after every build. Even if it does, the miscompile
 * might only occur on the production build, but not on a testing build (such
 * as because of different optimization settings). It is painful to recover
 * from incorrectly-computed hashes - merely fixing whatever broke is not
 * enough. Thus, a proactive measure like this self-test is needed.
 *
 * 2. We don't want to leave sensitive data from our actual password hash
 * computation on the stack or in registers. Previous revisions of the code
 * would do explicit cleanups, but simply running the self-test after hash
 * computation is more reliable.
 *
 * The performance cost of this quick self-test is around 0.6% at the "$2a$08"
 * setting.
 */
int blowfish_bcrypt_hash(uint8_t *dst,
                         size_t lenkey, const uint8_t *key,
			 size_t lenscheme, const uint8_t *scheme,
			 int log2rounds, const uint8_t *salt)
{
  const uint8_t test_pw[] = "8b \xd0\xc1\xd2\xcf\xcc\xd8";
  const uint8_t test_scheme[] = "$2a$00$abcdefghijklmnopqrstuu";
  static const char * const test_hashes[2] =
    {"i1D709vfamulimlGcq0qq3UvuUasvEa\0\x55",  /* 'a', 'b', 'y' */
     "VUrPmXD6q/nVSSp7pNDhCR9071IfIRe\0\x55"}; /* 'x' */
  const char *test_hash = test_hashes[0];
  int cscheme;
  int ok;
  uint8_t bufs[sizeof(test_scheme) - 1];
  uint8_t bufo[BLOWFISH_BCRYPT_HASH_SIZE];

  *dst = '\0';
/* Hash the supplied password */
  cscheme = ibcrypt(dst, lenkey, key, lenscheme, scheme, 4, log2rounds, salt);

/*
 * Do a quick self-test. It is important that we make both calls to ibcrypt()
 * from the same scope such that they likely use the same stack locations,
 * which makes the second call overwrite the first call's sensitive data on the
 * stack and makes it more likely that any alignment related issues would be
 * detected by the self-test.
 */
  memcpy(bufs, test_scheme, sizeof(test_scheme) - 1);

  if (cscheme)
    test_hash = test_hashes[(bufs[2] = cscheme) == 'x'];

  *bufo = 0;
  ok = ibcrypt(bufo, sizeof(test_pw) - 1, test_pw,
               sizeof(bufs), bufs, 0, -1, (void*)0);

  ok = (ok &&
      !memcmp(bufo, bufs, sizeof(bufs)) &&
      !memcmp(bufo + HASHOFFSET, test_hash, sizeof(test_hash) - 1));

  {
    const uint8_t k[] = "\xff\xa3" "34" "\xff\xff\xff\xa3" "345";
    bf_key ae, ai, ye, yi;
    set_xkey(sizeof(k) - 1, k, ae, ai, 0, 0x10000); /* $2a$ */
    set_xkey(sizeof(k) - 1, k, ye, yi, 0, 0); /* $2y$ */
    ai[0] ^= 0x10000; /* undo the safety (for comparison) */
    ok = ok && ai[0] == 0xdb9c59bc && ye[17] == 0x33343500 &&
        !memcmp(ae, ye, sizeof(ae)) &&
        !memcmp(ai, yi, sizeof(ai));
  }

  return ok && !!cscheme;
}
