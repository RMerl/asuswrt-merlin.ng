/* scrypt.c - Scrypt password-based key derivation function.
 * Copyright (C) 2012 Simon Josefsson
 * Copyright (C) 2013 Christian Grothoff
 * Copyright (C) 2013 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* Adapted from the nettle, low-level cryptographics library for
 * libgcrypt by Christian Grothoff; original license:
 *
 * Copyright (C) 2012 Simon Josefsson
 *
 * The nettle library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The nettle library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the nettle library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02111-1301, USA.
 */

#include <config.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "kdf-internal.h"
#include "bufhelp.h"

/* We really need a 64 bit type for this code.  */
#define SALSA20_INPUT_LENGTH 16

#define ROTL32(n,x) (((x)<<(n)) | ((x)>>(32-(n))))


/* Reads a 64-bit integer, in network, big-endian, byte order */
#define READ_UINT64(p) buf_get_be64(p)


/* And the other, little-endian, byteorder */
#define LE_READ_UINT64(p) buf_get_le64(p)

#define LE_SWAP32(v) le_bswap32(v)


#define QROUND(x0, x1, x2, x3) do { \
  x1 ^= ROTL32(7, x0 + x3);	    \
  x2 ^= ROTL32(9, x1 + x0);	    \
  x3 ^= ROTL32(13, x2 + x1);	    \
  x0 ^= ROTL32(18, x3 + x2);	    \
  } while(0)


static void
salsa20_core (u32 *dst, const u32 *src, unsigned int rounds)
{
  u32 x[SALSA20_INPUT_LENGTH];
  unsigned i;

  assert ( (rounds & 1) == 0);

  for (i = 0; i < SALSA20_INPUT_LENGTH; i++)
    x[i] = LE_SWAP32(src[i]);

  for (i = 0; i < rounds;i += 2)
    {
      QROUND(x[0], x[4], x[8], x[12]);
      QROUND(x[5], x[9], x[13], x[1]);
      QROUND(x[10], x[14], x[2], x[6]);
      QROUND(x[15], x[3], x[7], x[11]);

      QROUND(x[0], x[1], x[2], x[3]);
      QROUND(x[5], x[6], x[7], x[4]);
      QROUND(x[10], x[11], x[8], x[9]);
      QROUND(x[15], x[12], x[13], x[14]);
    }

  for (i = 0; i < SALSA20_INPUT_LENGTH; i++)
    {
      u32 t = x[i] + LE_SWAP32(src[i]);
      dst[i] = LE_SWAP32(t);
    }
}


static void
scrypt_block_mix (u32 r, unsigned char *B, unsigned char *tmp2)
{
  u64 i;
  unsigned char *X = tmp2;
  unsigned char *Y = tmp2 + 64;

#if 0
  if (r == 1)
    {
      for (i = 0; i < 2 * r; i++)
        {
          size_t j;
          printf ("B[%d] = ", (int)i);
          for (j = 0; j < 64; j++)
            {
              if (j && !(j % 16))
                printf ("\n       ");
              printf (" %02x", B[i * 64 + j]);
            }
          putchar ('\n');
        }
    }
#endif

  /* X = B[2 * r - 1] */
  memcpy (X, &B[(2 * r - 1) * 64], 64);

  /* for i = 0 to 2 * r - 1 do */
  for (i = 0; i <= 2 * r - 1; i++)
    {
      /* T = X xor B[i] */
      buf_xor(X, X, &B[i * 64], 64);

      /* X = Salsa (T) */
      salsa20_core ((u32*)(void*)X, (u32*)(void*)X, 8);

      /* Y[i] = X */
      memcpy (&Y[i * 64], X, 64);
    }

  for (i = 0; i < r; i++)
    {
      memcpy (&B[i * 64], &Y[2 * i * 64], 64);
      memcpy (&B[(r + i) * 64], &Y[(2 * i + 1) * 64], 64);
    }

#if 0
  if (r==1)
    {
      for (i = 0; i < 2 * r; i++)
        {
          size_t j;
          printf ("B'[%d] =", (int)i);
          for (j = 0; j < 64; j++)
            {
              if (j && !(j % 16))
                printf ("\n       ");
              printf (" %02x", B[i * 64 + j]);
            }
          putchar ('\n');
        }
    }
#endif
}


static void
scrypt_ro_mix (u32 r, unsigned char *B, u64 N,
	      unsigned char *tmp1, unsigned char *tmp2)
{
  unsigned char *X = B, *T = B;
  u64 i;

#if 0
  if (r == 1)
    {
      printf ("B = ");
      for (i = 0; i < 128 * r; i++)
        {
          if (i && !(i % 16))
            printf ("\n    ");
          printf (" %02x", B[i]);
        }
      putchar ('\n');
    }
#endif

  /* for i = 0 to N - 1 do */
  for (i = 0; i <= N - 1; i++)
    {
      /* V[i] = X */
      memcpy (&tmp1[i * 128 * r], X, 128 * r);

      /* X =  ScryptBlockMix (X) */
      scrypt_block_mix (r, X, tmp2);
    }

  /* for i = 0 to N - 1 do */
  for (i = 0; i <= N - 1; i++)
    {
      u64 j;

      /* j = Integerify (X) mod N */
      j = LE_READ_UINT64 (&X[128 * r - 64]) % N;

      /* T = X xor V[j] */
      buf_xor (T, T, &tmp1[j * 128 * r], 128 * r);

      /* X = scryptBlockMix (T) */
      scrypt_block_mix (r, T, tmp2);
    }

#if 0
  if (r == 1)
    {
      printf ("B' =");
      for (i = 0; i < 128 * r; i++)
        {
          if (i && !(i % 16))
            printf ("\n    ");
          printf (" %02x", B[i]);
        }
      putchar ('\n');
    }
#endif
}


/*
 *
 */
gcry_err_code_t
_gcry_kdf_scrypt (const unsigned char *passwd, size_t passwdlen,
                  int algo, int subalgo,
                  const unsigned char *salt, size_t saltlen,
                  unsigned long iterations,
                  size_t dkLen, unsigned char *DK)
{
  u64 N = subalgo;    /* CPU/memory cost parameter.  */
  u32 r;              /* Block size.  */
  u32 p = iterations; /* Parallelization parameter.  */

  gpg_err_code_t ec;
  u32 i;
  unsigned char *B = NULL;
  unsigned char *tmp1 = NULL;
  unsigned char *tmp2 = NULL;
  size_t r128;
  size_t nbytes;

  if (subalgo < 1 || !iterations)
    return GPG_ERR_INV_VALUE;

  if (algo == GCRY_KDF_SCRYPT)
    r = 8;
  else if (algo == 41) /* Hack to allow the use of all test vectors.  */
    r = 1;
  else
    return GPG_ERR_UNKNOWN_ALGORITHM;

  r128 = r * 128;
  if (r128 / 128 != r)
    return GPG_ERR_ENOMEM;

  nbytes = p * r128;
  if (r128 && nbytes / r128 != p)
    return GPG_ERR_ENOMEM;

  nbytes = N * r128;
  if (r128 && nbytes / r128 != N)
    return GPG_ERR_ENOMEM;

  nbytes = 64 + r128;
  if (nbytes < r128)
    return GPG_ERR_ENOMEM;

  B = xtrymalloc (p * r128);
  if (!B)
    {
      ec = gpg_err_code_from_syserror ();
      goto leave;
    }

  tmp1 = xtrymalloc (N * r128);
  if (!tmp1)
    {
      ec = gpg_err_code_from_syserror ();
      goto leave;
    }

  tmp2 = xtrymalloc (64 + r128);
  if (!tmp2)
    {
      ec = gpg_err_code_from_syserror ();
      goto leave;
    }

  ec = _gcry_kdf_pkdf2 (passwd, passwdlen, GCRY_MD_SHA256, salt, saltlen,
                        1 /* iterations */, p * r128, B);

  for (i = 0; !ec && i < p; i++)
    scrypt_ro_mix (r, &B[i * r128], N, tmp1, tmp2);

  if (!ec)
    ec = _gcry_kdf_pkdf2 (passwd, passwdlen, GCRY_MD_SHA256, B, p * r128,
                          1 /* iterations */, dkLen, DK);

 leave:
  xfree (tmp2);
  xfree (tmp1);
  xfree (B);

  return ec;
}
