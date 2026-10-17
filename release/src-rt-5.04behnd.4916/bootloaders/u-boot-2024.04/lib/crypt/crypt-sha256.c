// SPDX-License-Identifier: CC0-1.0
/* Based on libxcrypt v4.4.17-0-g6b110bc */
/* One way encryption based on the SHA256-based Unix crypt implementation.
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
 * This file is a modified except from [2], lines 648 up to 909.
 *
 * [1]  https://www.akkadia.org/drepper/sha-crypt.html
 * [2]  https://www.akkadia.org/drepper/SHA-crypt.txt
 */

#include "crypt-port.h"
#include "alg-sha256.h"

#include <linux/errno.h>
#include <stdio.h>
#include <stdlib.h>

#if INCLUDE_sha256crypt

/* Define our magic string to mark salt for SHA256 "encryption"
   replacement.  */
static const char sha256_salt_prefix[] = "$5$";

/* Prefix for optional rounds specification.  */
static const char sha256_rounds_prefix[] = "rounds=";

/* Maximum salt string length.  */
#define SALT_LEN_MAX 16
/* Default number of rounds if not explicitly specified.  */
#define ROUNDS_DEFAULT 5000
/* Minimum number of rounds.  */
#define ROUNDS_MIN 1000
/* Maximum number of rounds.  */
#define ROUNDS_MAX 999999999

/* The maximum possible length of a SHA256-hashed password string,
   including the terminating NUL character.  Prefix (including its NUL)
   + rounds tag ("rounds=$" = "rounds=\0") + strlen(ROUNDS_MAX)
   + salt (up to SALT_LEN_MAX chars) + '$' + hash (43 chars).  */

#define LENGTH_OF_NUMBER(n) (sizeof #n - 1)

#define SHA256_HASH_LENGTH \
  (sizeof (sha256_salt_prefix) + sizeof (sha256_rounds_prefix) + \
   LENGTH_OF_NUMBER (ROUNDS_MAX) + SALT_LEN_MAX + 1 + 43)

static_assert (SHA256_HASH_LENGTH <= CRYPT_OUTPUT_SIZE,
               "CRYPT_OUTPUT_SIZE is too small for SHA256");

/* A sha256_buffer holds all of the sensitive intermediate data.  */
struct sha256_buffer
{
  SHA256_CTX ctx;
  uint8_t result[32];
  uint8_t p_bytes[32];
  uint8_t s_bytes[32];
};

static_assert (sizeof (struct sha256_buffer) <= ALG_SPECIFIC_SIZE,
               "ALG_SPECIFIC_SIZE is too small for SHA256");


/* Use this instead of including errno.h */
static int errno;

void crypt_sha256crypt_rn(const char *phrase, size_t phr_size,
			  const char *setting, size_t ARG_UNUSED(set_size),
			  uint8_t *output, size_t out_size, void *scratch,
			  size_t scr_size);

int crypt_sha256crypt_rn_wrapped(const char *phrase, size_t phr_size,
				 const char *setting, size_t set_size,
				 u8 *output, size_t out_size, void *scratch,
				 size_t scr_size)
{
	errno = 0;
	crypt_sha256crypt_rn(phrase, phr_size, setting, set_size, output,
			     out_size, scratch, scr_size);
	return -errno;
}

/* Feed CTX with LEN bytes of a virtual byte sequence consisting of
   BLOCK repeated over and over indefinitely.  */
static void
SHA256_Update_recycled (SHA256_CTX *ctx,
                        unsigned char block[32], size_t len)
{
  size_t cnt;
  for (cnt = len; cnt >= 32; cnt -= 32)
    SHA256_Update (ctx, block, 32);
  SHA256_Update (ctx, block, cnt);
}

void
crypt_sha256crypt_rn (const char *phrase, size_t phr_size,
                      const char *setting, size_t ARG_UNUSED (set_size),
                      uint8_t *output, size_t out_size,
                      void *scratch, size_t scr_size)
{
  /* This shouldn't ever happen, but...  */
  if (out_size < SHA256_HASH_LENGTH
      || scr_size < sizeof (struct sha256_buffer))
    {
      errno = ERANGE;
      return;
    }

  struct sha256_buffer *buf = scratch;
  SHA256_CTX *ctx = &buf->ctx;
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
  if (strncmp (sha256_salt_prefix, salt, sizeof (sha256_salt_prefix) - 1) == 0)
    /* Skip salt prefix.  */
    salt += sizeof (sha256_salt_prefix) - 1;

  if (strncmp (salt, sha256_rounds_prefix, sizeof (sha256_rounds_prefix) - 1)
      == 0)
    {
      const char *num = salt + sizeof (sha256_rounds_prefix) - 1;
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

  /* Compute alternate SHA256 sum with input PHRASE, SALT, and PHRASE.  The
     final result will be added to the first context.  */
  SHA256_Init (ctx);

  /* Add phrase.  */
  SHA256_Update (ctx, phrase, phr_size);

  /* Add salt.  */
  SHA256_Update (ctx, salt, salt_size);

  /* Add phrase again.  */
  SHA256_Update (ctx, phrase, phr_size);

  /* Now get result of this (32 bytes).  */
  SHA256_Final (result, ctx);

  /* Prepare for the real work.  */
  SHA256_Init (ctx);

  /* Add the phrase string.  */
  SHA256_Update (ctx, phrase, phr_size);

  /* The last part is the salt string.  This must be at most 8
     characters and it ends at the first `$' character (for
     compatibility with existing implementations).  */
  SHA256_Update (ctx, salt, salt_size);

  /* Add for any character in the phrase one byte of the alternate sum.  */
  for (cnt = phr_size; cnt > 32; cnt -= 32)
    SHA256_Update (ctx, result, 32);
  SHA256_Update (ctx, result, cnt);

  /* Take the binary representation of the length of the phrase and for every
     1 add the alternate sum, for every 0 the phrase.  */
  for (cnt = phr_size; cnt > 0; cnt >>= 1)
    if ((cnt & 1) != 0)
      SHA256_Update (ctx, result, 32);
    else
      SHA256_Update (ctx, phrase, phr_size);

  /* Create intermediate result.  */
  SHA256_Final (result, ctx);

  /* Start computation of P byte sequence.  */
  SHA256_Init (ctx);

  /* For every character in the password add the entire password.  */
  for (cnt = 0; cnt < phr_size; ++cnt)
    SHA256_Update (ctx, phrase, phr_size);

  /* Finish the digest.  */
  SHA256_Final (p_bytes, ctx);

  /* Start computation of S byte sequence.  */
  SHA256_Init (ctx);

  /* For every character in the password add the entire password.  */
  for (cnt = 0; cnt < (size_t) 16 + (size_t) result[0]; ++cnt)
    SHA256_Update (ctx, salt, salt_size);

  /* Finish the digest.  */
  SHA256_Final (s_bytes, ctx);

  /* Repeatedly run the collected hash value through SHA256 to burn
     CPU cycles.  */
  for (cnt = 0; cnt < rounds; ++cnt)
    {
      /* New context.  */
      SHA256_Init (ctx);

      /* Add phrase or last result.  */
      if ((cnt & 1) != 0)
        SHA256_Update_recycled (ctx, p_bytes, phr_size);
      else
        SHA256_Update (ctx, result, 32);

      /* Add salt for numbers not divisible by 3.  */
      if (cnt % 3 != 0)
        SHA256_Update_recycled (ctx, s_bytes, salt_size);

      /* Add phrase for numbers not divisible by 7.  */
      if (cnt % 7 != 0)
        SHA256_Update_recycled (ctx, p_bytes, phr_size);

      /* Add phrase or last result.  */
      if ((cnt & 1) != 0)
        SHA256_Update (ctx, result, 32);
      else
        SHA256_Update_recycled (ctx, p_bytes, phr_size);

      /* Create intermediate result.  */
      SHA256_Final (result, ctx);
    }

  /* Now we can construct the result string.  It consists of four
     parts, one of which is optional.  We already know that there
     is sufficient space at CP for the longest possible result string.  */
  memcpy (cp, sha256_salt_prefix, sizeof (sha256_salt_prefix) - 1);
  cp += sizeof (sha256_salt_prefix) - 1;

  if (rounds_custom)
    {
      int n = snprintf (cp,
                        SHA256_HASH_LENGTH - (sizeof (sha256_salt_prefix) - 1),
                        "%s%zu$", sha256_rounds_prefix, rounds);
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

  b64_from_24bit (result[0], result[10], result[20], 4);
  b64_from_24bit (result[21], result[1], result[11], 4);
  b64_from_24bit (result[12], result[22], result[2], 4);
  b64_from_24bit (result[3], result[13], result[23], 4);
  b64_from_24bit (result[24], result[4], result[14], 4);
  b64_from_24bit (result[15], result[25], result[5], 4);
  b64_from_24bit (result[6], result[16], result[26], 4);
  b64_from_24bit (result[27], result[7], result[17], 4);
  b64_from_24bit (result[18], result[28], result[8], 4);
  b64_from_24bit (result[9], result[19], result[29], 4);
  b64_from_24bit (0, result[31], result[30], 3);

  *cp = '\0';
}

#ifndef NO_GENSALT

void
gensalt_sha256crypt_rn (unsigned long count,
                        const uint8_t *rbytes, size_t nrbytes,
                        uint8_t *output, size_t output_size)
{
  gensalt_sha_rn ('5', SALT_LEN_MAX, ROUNDS_DEFAULT, ROUNDS_MIN, ROUNDS_MAX,
                  count, rbytes, nrbytes, output, output_size);
}

#endif

#endif
