// SPDX-License-Identifier: CC0-1.0
/* Based on libxcrypt v4.4.17-0-g6b110bc */
/* One way encryption based on the SHA512-based Unix crypt implementation.
 *
 * Written by Ulrich Drepper <drepper at redhat.com> in 2007 [1].
 * Modified by Zack Weinberg <zackw at panix.com> in 2017, 2018.
 * Composed by Björn Esser <besser82 at fedoraproject.org> in 2018.
 * Modified by Björn Esser <besser82 at fedoraproject.org> in 2020.
 * Modified by Steffen Jaeckel <jaeckel-floss at eyet-services.de> in 2021
 * for U-Boot, instead of using the global errno to use a static one
 * inside this file.
 * To the extent possible under law, the named authors have waived all
 * copyright and related or neighboring rights to this work.
 *
 * See https://creativecommons.org/publicdomain/zero/1.0/ for further
 * details.
 *
 * This file is a modified except from [2], lines 1403 up to 1676.
 *
 * [1]  https://www.akkadia.org/drepper/sha-crypt.html
 * [2]  https://www.akkadia.org/drepper/SHA-crypt.txt
 */

#include "crypt-port.h"
#include "alg-sha512.h"

#include <linux/errno.h>
#include <stdio.h>
#include <stdlib.h>

#if INCLUDE_sha512crypt

/* Define our magic string to mark salt for SHA512 "encryption"
   replacement.  */
static const char sha512_salt_prefix[] = "$6$";

/* Prefix for optional rounds specification.  */
static const char sha512_rounds_prefix[] = "rounds=";

/* Maximum salt string length.  */
#define SALT_LEN_MAX 16
/* Default number of rounds if not explicitly specified.  */
#define ROUNDS_DEFAULT 5000
/* Minimum number of rounds.  */
#define ROUNDS_MIN 1000
/* Maximum number of rounds.  */
#define ROUNDS_MAX 999999999

/* The maximum possible length of a SHA512-hashed password string,
   including the terminating NUL character.  Prefix (including its NUL)
   + rounds tag ("rounds=$" = "rounds=\0") + strlen(ROUNDS_MAX)
   + salt (up to SALT_LEN_MAX chars) + '$' + hash (86 chars).  */

#define LENGTH_OF_NUMBER(n) (sizeof #n - 1)

#define SHA512_HASH_LENGTH \
  (sizeof (sha512_salt_prefix) + sizeof (sha512_rounds_prefix) + \
   LENGTH_OF_NUMBER (ROUNDS_MAX) + SALT_LEN_MAX + 1 + 86)

static_assert (SHA512_HASH_LENGTH <= CRYPT_OUTPUT_SIZE,
               "CRYPT_OUTPUT_SIZE is too small for SHA512");

/* A sha512_buffer holds all of the sensitive intermediate data.  */
struct sha512_buffer
{
  SHA512_CTX ctx;
  uint8_t result[64];
  uint8_t p_bytes[64];
  uint8_t s_bytes[64];
};

static_assert (sizeof (struct sha512_buffer) <= ALG_SPECIFIC_SIZE,
               "ALG_SPECIFIC_SIZE is too small for SHA512");


/* Use this instead of including errno.h */
static int errno;

void crypt_sha512crypt_rn(const char *phrase, size_t phr_size,
			  const char *setting, size_t ARG_UNUSED(set_size),
			  uint8_t *output, size_t out_size, void *scratch,
			  size_t scr_size);

int crypt_sha512crypt_rn_wrapped(const char *phrase, size_t phr_size,
				 const char *setting, size_t set_size,
				 u8 *output, size_t out_size, void *scratch,
				 size_t scr_size)
{
	errno = 0;
	crypt_sha512crypt_rn(phrase, phr_size, setting, set_size, output,
			     out_size, scratch, scr_size);
	return -errno;
}

/* Subroutine of _xcrypt_crypt_sha512crypt_rn: Feed CTX with LEN bytes of a
   virtual byte sequence consisting of BLOCK repeated over and over
   indefinitely.  */
static void
sha512_process_recycled_bytes (unsigned char block[64], size_t len,
                               SHA512_CTX *ctx)
{
  size_t cnt;
  for (cnt = len; cnt >= 64; cnt -= 64)
    SHA512_Update (ctx, block, 64);
  SHA512_Update (ctx, block, cnt);
}

void
crypt_sha512crypt_rn (const char *phrase, size_t phr_size,
                      const char *setting, size_t ARG_UNUSED (set_size),
                      uint8_t *output, size_t out_size,
                      void *scratch, size_t scr_size)
{
  /* This shouldn't ever happen, but...  */
  if (out_size < SHA512_HASH_LENGTH
      || scr_size < sizeof (struct sha512_buffer))
    {
      errno = ERANGE;
      return;
    }

  struct sha512_buffer *buf = scratch;
  SHA512_CTX *ctx = &buf->ctx;
  uint8_t *result = buf->result;
  uint8_t *p_bytes = buf->p_bytes;
  uint8_t *s_bytes = buf->s_bytes;
  char *cp = (char *)output;
  const char *salt = setting;

  size_t salt_size;
  size_t cnt;
  /* Default number of rounds.  */
  size_t rounds = ROUNDS_DEFAULT;
  bool rounds_custom = false;

  /* Find beginning of salt string.  The prefix should normally always
     be present.  Just in case it is not.  */
  if (strncmp (sha512_salt_prefix, salt, sizeof (sha512_salt_prefix) - 1) == 0)
    /* Skip salt prefix.  */
    salt += sizeof (sha512_salt_prefix) - 1;

  if (strncmp (salt, sha512_rounds_prefix, sizeof (sha512_rounds_prefix) - 1)
      == 0)
    {
      const char *num = salt + sizeof (sha512_rounds_prefix) - 1;
      /* Do not allow an explicit setting of zero rounds, nor of the
         default number of rounds, nor leading zeroes on the rounds.  */
      if (!(*num >= '1' && *num <= '9'))
        {
          errno = EINVAL;
          return;
        }

      errno = 0;
      char *endp;
      rounds = strtoul (num, &endp, 10);
      if (endp == num || *endp != '$'
          || rounds < ROUNDS_MIN
          || rounds > ROUNDS_MAX
          || errno)
        {
          errno = EINVAL;
          return;
        }
      salt = endp + 1;
      rounds_custom = true;
    }

  /* The salt ends at the next '$' or the end of the string.
     Ensure ':' does not appear in the salt (it is used as a separator in /etc/passwd).
     Also check for '\n', as in /etc/passwd the whole parameters of the user data must
     be on a single line. */
  salt_size = strcspn (salt, "$:\n");
  if (!(salt[salt_size] == '$' || !salt[salt_size]))
    {
      errno = EINVAL;
      return;
    }

  /* Ensure we do not use more salt than SALT_LEN_MAX. */
  if (salt_size > SALT_LEN_MAX)
    salt_size = SALT_LEN_MAX;

  /* Compute alternate SHA512 sum with input PHRASE, SALT, and PHRASE.  The
     final result will be added to the first context.  */
  SHA512_Init (ctx);

  /* Add phrase.  */
  SHA512_Update (ctx, phrase, phr_size);

  /* Add salt.  */
  SHA512_Update (ctx, salt, salt_size);

  /* Add phrase again.  */
  SHA512_Update (ctx, phrase, phr_size);

  /* Now get result of this (64 bytes) and add it to the other
     context.  */
  SHA512_Final (result, ctx);

  /* Prepare for the real work.  */
  SHA512_Init (ctx);

  /* Add the phrase string.  */
  SHA512_Update (ctx, phrase, phr_size);

  /* The last part is the salt string.  This must be at most 8
     characters and it ends at the first `$' character (for
     compatibility with existing implementations).  */
  SHA512_Update (ctx, salt, salt_size);

  /* Add for any character in the phrase one byte of the alternate sum.  */
  for (cnt = phr_size; cnt > 64; cnt -= 64)
    SHA512_Update (ctx, result, 64);
  SHA512_Update (ctx, result, cnt);

  /* Take the binary representation of the length of the phrase and for every
     1 add the alternate sum, for every 0 the phrase.  */
  for (cnt = phr_size; cnt > 0; cnt >>= 1)
    if ((cnt & 1) != 0)
      SHA512_Update (ctx, result, 64);
    else
      SHA512_Update (ctx, phrase, phr_size);

  /* Create intermediate result.  */
  SHA512_Final (result, ctx);

  /* Start computation of P byte sequence.  */
  SHA512_Init (ctx);

  /* For every character in the password add the entire password.  */
  for (cnt = 0; cnt < phr_size; ++cnt)
    SHA512_Update (ctx, phrase, phr_size);

  /* Finish the digest.  */
  SHA512_Final (p_bytes, ctx);

  /* Start computation of S byte sequence.  */
  SHA512_Init (ctx);

  /* For every character in the password add the entire password.  */
  for (cnt = 0; cnt < (size_t) 16 + (size_t) result[0]; ++cnt)
    SHA512_Update (ctx, salt, salt_size);

  /* Finish the digest.  */
  SHA512_Final (s_bytes, ctx);

  /* Repeatedly run the collected hash value through SHA512 to burn
     CPU cycles.  */
  for (cnt = 0; cnt < rounds; ++cnt)
    {
      /* New context.  */
      SHA512_Init (ctx);

      /* Add phrase or last result.  */
      if ((cnt & 1) != 0)
        sha512_process_recycled_bytes (p_bytes, phr_size, ctx);
      else
        SHA512_Update (ctx, result, 64);

      /* Add salt for numbers not divisible by 3.  */
      if (cnt % 3 != 0)
        sha512_process_recycled_bytes (s_bytes, salt_size, ctx);

      /* Add phrase for numbers not divisible by 7.  */
      if (cnt % 7 != 0)
        sha512_process_recycled_bytes (p_bytes, phr_size, ctx);

      /* Add phrase or last result.  */
      if ((cnt & 1) != 0)
        SHA512_Update (ctx, result, 64);
      else
        sha512_process_recycled_bytes (p_bytes, phr_size, ctx);

      /* Create intermediate result.  */
      SHA512_Final (result, ctx);
    }

  /* Now we can construct the result string.  It consists of four
     parts, one of which is optional.  We already know that buflen is
     at least sha512_hash_length, therefore none of the string bashing
     below can overflow the buffer. */

  memcpy (cp, sha512_salt_prefix, sizeof (sha512_salt_prefix) - 1);
  cp += sizeof (sha512_salt_prefix) - 1;

  if (rounds_custom)
    {
      int n = snprintf (cp,
                        SHA512_HASH_LENGTH - (sizeof (sha512_salt_prefix) - 1),
                        "%s%zu$", sha512_rounds_prefix, rounds);
      cp += n;
    }

  memcpy (cp, salt, salt_size);
  cp += salt_size;
  *cp++ = '$';

#define b64_from_24bit(B2, B1, B0, N)                   \
  do {                                                  \
    unsigned int w = ((((unsigned int)(B2)) << 16) |    \
                      (((unsigned int)(B1)) << 8) |     \
                      ((unsigned int)(B0)));            \
    int n = (N);                                        \
    while (n-- > 0)                                     \
      {                                                 \
        *cp++ = b64t[w & 0x3f];                         \
        w >>= 6;                                        \
      }                                                 \
  } while (0)

  b64_from_24bit (result[0], result[21], result[42], 4);
  b64_from_24bit (result[22], result[43], result[1], 4);
  b64_from_24bit (result[44], result[2], result[23], 4);
  b64_from_24bit (result[3], result[24], result[45], 4);
  b64_from_24bit (result[25], result[46], result[4], 4);
  b64_from_24bit (result[47], result[5], result[26], 4);
  b64_from_24bit (result[6], result[27], result[48], 4);
  b64_from_24bit (result[28], result[49], result[7], 4);
  b64_from_24bit (result[50], result[8], result[29], 4);
  b64_from_24bit (result[9], result[30], result[51], 4);
  b64_from_24bit (result[31], result[52], result[10], 4);
  b64_from_24bit (result[53], result[11], result[32], 4);
  b64_from_24bit (result[12], result[33], result[54], 4);
  b64_from_24bit (result[34], result[55], result[13], 4);
  b64_from_24bit (result[56], result[14], result[35], 4);
  b64_from_24bit (result[15], result[36], result[57], 4);
  b64_from_24bit (result[37], result[58], result[16], 4);
  b64_from_24bit (result[59], result[17], result[38], 4);
  b64_from_24bit (result[18], result[39], result[60], 4);
  b64_from_24bit (result[40], result[61], result[19], 4);
  b64_from_24bit (result[62], result[20], result[41], 4);
  b64_from_24bit (0, 0, result[63], 2);

  *cp = '\0';
}

#ifndef NO_GENSALT

void
gensalt_sha512crypt_rn (unsigned long count,
                        const uint8_t *rbytes, size_t nrbytes,
                        uint8_t *output, size_t output_size)
{
  gensalt_sha_rn ('6', SALT_LEN_MAX, ROUNDS_DEFAULT, ROUNDS_MIN, ROUNDS_MAX,
                  count, rbytes, nrbytes, output, output_size);
}

#endif

#endif
