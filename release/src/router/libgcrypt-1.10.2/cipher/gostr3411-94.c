/* gostr3411-94.c - GOST R 34.11-94 hash function
 * Copyright (C) 2012 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
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


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g10lib.h"
#include "bithelp.h"
#include "bufhelp.h"
#include "cipher.h"
#include "hash-common.h"

#include "gost.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
  gcry_md_block_ctx_t bctx;
  union {
    u32 h[8];
    byte result[32];
  };
  u32 sigma[8];
  u32 len;
  int cryptopro;
} GOSTR3411_CONTEXT;

static unsigned int
transform (void *c, const unsigned char *data, size_t nblks);

static void
gost3411_init (void *context, unsigned int flags)
{
  GOSTR3411_CONTEXT *hd = context;

  (void)flags;

  memset (hd->h, 0, 32);
  memset (hd->sigma, 0, 32);

  hd->bctx.nblocks = 0;
  hd->bctx.count = 0;
  hd->bctx.blocksize_shift = _gcry_ctz(32);
  hd->bctx.bwrite = transform;
  hd->cryptopro = 0;
}

static void
gost3411_cp_init (void *context, unsigned int flags)
{
  GOSTR3411_CONTEXT *hd = context;
  gost3411_init (context, flags);
  hd->cryptopro = 1;
}

static void
do_p (u32 *p, u32 *u, u32 *v)
{
  int k;
  u32 t[8];

  for (k = 0; k < 8; k++)
    t[k] = u[k] ^ v[k];

  k = 0;
  p[k+0] = ((t[0] >> (8*k)) & 0xff) << 0 |
           ((t[2] >> (8*k)) & 0xff) << 8 |
           ((t[4] >> (8*k)) & 0xff) << 16 |
           ((t[6] >> (8*k)) & 0xff) << 24;
  p[k+4] = ((t[1] >> (8*k)) & 0xff) << 0 |
           ((t[3] >> (8*k)) & 0xff) << 8 |
           ((t[5] >> (8*k)) & 0xff) << 16 |
           ((t[7] >> (8*k)) & 0xff) << 24;

  k = 1;
  p[k+0] = ((t[0] >> (8*k)) & 0xff) << 0 |
           ((t[2] >> (8*k)) & 0xff) << 8 |
           ((t[4] >> (8*k)) & 0xff) << 16 |
           ((t[6] >> (8*k)) & 0xff) << 24;
  p[k+4] = ((t[1] >> (8*k)) & 0xff) << 0 |
           ((t[3] >> (8*k)) & 0xff) << 8 |
           ((t[5] >> (8*k)) & 0xff) << 16 |
           ((t[7] >> (8*k)) & 0xff) << 24;

  k = 2;
  p[k+0] = ((t[0] >> (8*k)) & 0xff) << 0 |
           ((t[2] >> (8*k)) & 0xff) << 8 |
           ((t[4] >> (8*k)) & 0xff) << 16 |
           ((t[6] >> (8*k)) & 0xff) << 24;
  p[k+4] = ((t[1] >> (8*k)) & 0xff) << 0 |
           ((t[3] >> (8*k)) & 0xff) << 8 |
           ((t[5] >> (8*k)) & 0xff) << 16 |
           ((t[7] >> (8*k)) & 0xff) << 24;

  k = 3;
  p[k+0] = ((t[0] >> (8*k)) & 0xff) << 0 |
           ((t[2] >> (8*k)) & 0xff) << 8 |
           ((t[4] >> (8*k)) & 0xff) << 16 |
           ((t[6] >> (8*k)) & 0xff) << 24;
  p[k+4] = ((t[1] >> (8*k)) & 0xff) << 0 |
           ((t[3] >> (8*k)) & 0xff) << 8 |
           ((t[5] >> (8*k)) & 0xff) << 16 |
           ((t[7] >> (8*k)) & 0xff) << 24;
}

static void
do_a (u32 *u)
{
  u32 t[2];
  int i;
  memcpy(t, u, 2*4);
  for (i = 0; i < 6; i++)
    u[i] = u[i+2];
  u[6] = u[0] ^ t[0];
  u[7] = u[1] ^ t[1];
}
/* apply do_a twice: 1 2 3 4 -> 3 4 1^2 2^3 */
static void
do_a2 (u32 *u)
{
  u32 t[4];
  int i;
  memcpy (t, u, 16);
  memcpy (u, u + 4, 16);
  for (i = 0; i < 2; i++)
    {
      u[4+i] = t[i] ^ t[i + 2];
      u[6+i] = u[i] ^ t[i + 2];
    }
}

static void
do_apply_c2 (u32 *u)
{
  u[ 0] ^= 0xff00ff00;
  u[ 1] ^= 0xff00ff00;
  u[ 2] ^= 0x00ff00ff;
  u[ 3] ^= 0x00ff00ff;
  u[ 4] ^= 0x00ffff00;
  u[ 5] ^= 0xff0000ff;
  u[ 6] ^= 0x000000ff;
  u[ 7] ^= 0xff00ffff;
}

#define do_chi_step12(e) \
  e[6] ^= ((e[6] >> 16) ^ e[7] ^ (e[7] >> 16) ^ e[4] ^ (e[5] >>16)) & 0xffff;

#define do_chi_step13(e) \
  e[6] ^= ((e[7] ^ (e[7] >> 16) ^ e[0] ^ (e[4] >> 16) ^ e[6]) & 0xffff) << 16;

#define do_chi_doublestep(e, i) \
  e[i] ^= (e[i] >> 16) ^ (e[(i+1)%8] << 16) ^ e[(i+1)%8] ^ (e[(i+1)%8] >> 16) ^ (e[(i+2)%8] << 16) ^ e[(i+6)%8] ^ (e[(i+7)%8] >> 16); \
  e[i] ^= (e[i] << 16);

static void
do_chi_submix12 (u32 *e, u32 *x)
{
  e[6] ^= x[0];
  e[7] ^= x[1];
  e[0] ^= x[2];
  e[1] ^= x[3];
  e[2] ^= x[4];
  e[3] ^= x[5];
  e[4] ^= x[6];
  e[5] ^= x[7];
}

static void
do_chi_submix13 (u32 *e, u32 *x)
{
  e[6] ^= (x[0] << 16) | (x[7] >> 16);
  e[7] ^= (x[1] << 16) | (x[0] >> 16);
  e[0] ^= (x[2] << 16) | (x[1] >> 16);
  e[1] ^= (x[3] << 16) | (x[2] >> 16);
  e[2] ^= (x[4] << 16) | (x[3] >> 16);
  e[3] ^= (x[5] << 16) | (x[4] >> 16);
  e[4] ^= (x[6] << 16) | (x[5] >> 16);
  e[5] ^= (x[7] << 16) | (x[6] >> 16);
}

static void
do_add (u32 *s, u32 *a)
{
  u32 carry = 0;
  int i;

  for (i = 0; i < 8; i++)
    {
      u32 op = carry + a[i];
      s[i] += op;
      carry = (a[i] > op) || (op > s[i]);
    }
}

static unsigned int
do_hash_step (GOSTR3411_CONTEXT *hd, u32 *h, u32 *m)
{
  u32 u[8], v[8];
  u32 s[8];
  u32 k[8];
  unsigned int burn;
  int i;

  memcpy (u, h, 32);
  memcpy (v, m, 32);

  for (i = 0; i < 4; i++) {
    do_p (k, u, v);

    burn = _gcry_gost_enc_data (k, &s[2*i], &s[2*i+1], h[2*i], h[2*i+1], hd->cryptopro);

    do_a (u);
    if (i == 1)
      do_apply_c2 (u);
    do_a2 (v);
  }

  for (i = 0; i < 5; i++)
    {
      do_chi_doublestep (s, 0);
      do_chi_doublestep (s, 1);
      do_chi_doublestep (s, 2);
      do_chi_doublestep (s, 3);
      do_chi_doublestep (s, 4);
      /* That is in total 12 + 1 + 61 = 74 = 16 * 4 + 10 rounds */
      if (i == 4)
        break;
      do_chi_doublestep (s, 5);
      if (i == 0)
        do_chi_submix12(s, m);
      do_chi_step12 (s);
      if (i == 0)
        do_chi_submix13(s, h);
      do_chi_step13 (s);
      do_chi_doublestep (s, 7);
    }

  memcpy (h, s+5, 12);
  memcpy (h+3, s, 20);

  return /* burn_stack */ 4 * sizeof(void*) /* func call (ret addr + args) */ +
                          4 * 32 + 2 * sizeof(int) /* stack */ +
                          max(burn /* _gcry_gost_enc_one */,
                              sizeof(void*) * 2 /* do_a2 call */ +
                              16 + sizeof(int) /* do_a2 stack */ );
}

static unsigned int
transform_blk (void *ctx, const unsigned char *data)
{
  GOSTR3411_CONTEXT *hd = ctx;
  u32 m[8];
  unsigned int burn;
  int i;

  for (i = 0; i < 8; i++)
    m[i] = buf_get_le32(data + i*4);
  burn = do_hash_step (hd, hd->h, m);
  do_add (hd->sigma, m);

  return /* burn_stack */ burn + 3 * sizeof(void*) + 32 + 2 * sizeof(void*);
}


static unsigned int
transform ( void *c, const unsigned char *data, size_t nblks )
{
  unsigned int burn;

  do
    {
      burn = transform_blk (c, data);
      data += 32;
    }
  while (--nblks);

  return burn;
}


/*
   The routine finally terminates the computation and returns the
   digest.  The handle is prepared for a new cycle, but adding bytes
   to the handle will the destroy the returned buffer.  Returns: 32
   bytes with the message the digest.  */
static void
gost3411_final (void *context)
{
  GOSTR3411_CONTEXT *hd = context;
  size_t padlen = 0;
  u32 l[8];
  int i;
  MD_NBLOCKS_TYPE nblocks;

  if (hd->bctx.count > 0)
    {
      padlen = 32 - hd->bctx.count;
      memset (hd->bctx.buf + hd->bctx.count, 0, padlen);
      hd->bctx.count += padlen;
      _gcry_md_block_write (hd, NULL, 0); /* flush */;
    }

  if (hd->bctx.count != 0)
    return; /* Something went wrong */

  memset (l, 0, 32);

  nblocks = hd->bctx.nblocks;
  if (padlen)
    {
      nblocks --;
      l[0] = 256 - padlen * 8;
    }
  l[0] |= nblocks << 8;
  nblocks >>= 24;

  for (i = 1; i < 8 && nblocks != 0; i++)
    {
      l[i] = nblocks;
      nblocks >>= 24;
    }

  do_hash_step (hd, hd->h, l);
  do_hash_step (hd, hd->h, hd->sigma);
  for (i = 0; i < 8; i++)
    hd->h[i] = le_bswap32(hd->h[i]);
}

static byte *
gost3411_read (void *context)
{
  GOSTR3411_CONTEXT *hd = context;

  return hd->result;
}

static const unsigned char asn[6] = /* Object ID is 1.2.643.2.2.3 */
  { 0x2a, 0x85, 0x03, 0x02, 0x02, 0x03 };

static const gcry_md_oid_spec_t oid_spec_gostr3411[] =
  {
    /* iso.member-body.ru.rans.cryptopro.3 (gostR3411-94-with-gostR3410-2001) */
    { "1.2.643.2.2.3" },
    /* iso.member-body.ru.rans.cryptopro.9 (gostR3411-94) */
    { "1.2.643.2.2.9" },
    {NULL},
  };

const gcry_md_spec_t _gcry_digest_spec_gost3411_94 =
  {
    GCRY_MD_GOSTR3411_94, {0, 0},
    "GOSTR3411_94", NULL, 0, NULL, 32,
    gost3411_init, _gcry_md_block_write, gost3411_final, gost3411_read, NULL,
    NULL,
    sizeof (GOSTR3411_CONTEXT)
  };
const gcry_md_spec_t _gcry_digest_spec_gost3411_cp =
  {
    GCRY_MD_GOSTR3411_CP, {0, 0},
    "GOSTR3411_CP", asn, DIM (asn), oid_spec_gostr3411, 32,
    gost3411_cp_init, _gcry_md_block_write, gost3411_final, gost3411_read, NULL,
    NULL,
    sizeof (GOSTR3411_CONTEXT)
  };
