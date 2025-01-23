/* ec-hw-s390x.c -  zSeries ECC acceleration
 * Copyright (C) 2021 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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
#include <errno.h>

#ifdef HAVE_GCC_INLINE_ASM_S390X

#include "mpi-internal.h"
#include "g10lib.h"
#include "context.h"
#include "ec-context.h"
#include "ec-internal.h"

#include "../cipher/bufhelp.h"
#include "../cipher/asm-inline-s390x.h"


#define S390X_PCC_PARAM_BLOCK_SIZE 4096


extern void reverse_buffer (unsigned char *buffer, unsigned int length);

static int s390_mul_point_montgomery (mpi_point_t result, gcry_mpi_t scalar,
				      mpi_point_t point, mpi_ec_t ctx,
				      byte *param_block_buf);


static int
mpi_copy_to_raw(byte *raw, unsigned int raw_nbytes, gcry_mpi_t a)
{
  unsigned int num_to_zero;
  unsigned int nbytes;
  int i, j;

  if (mpi_has_sign (a))
    return -1;

  if (mpi_get_flag (a, GCRYMPI_FLAG_OPAQUE))
    {
      unsigned int nbits;
      byte *buf;

      buf = mpi_get_opaque (a, &nbits);
      nbytes = (nbits + 7) / 8;

      if (raw_nbytes < nbytes)
	return -1;

      num_to_zero = raw_nbytes - nbytes;
      if (num_to_zero > 0)
        memset (raw, 0, num_to_zero);
      if (nbytes > 0)
	memcpy (raw + num_to_zero, buf, nbytes);

      return 0;
    }

  nbytes = a->nlimbs * BYTES_PER_MPI_LIMB;
  if (raw_nbytes < nbytes)
    return -1;

  num_to_zero = raw_nbytes - nbytes;
  if (num_to_zero > 0)
    memset (raw, 0, num_to_zero);

  for (j = a->nlimbs - 1, i = 0; i < a->nlimbs; i++, j--)
    {
      buf_put_be64(raw + num_to_zero + i * BYTES_PER_MPI_LIMB, a->d[j]);
    }

  return 0;
}

int
_gcry_s390x_ec_hw_mul_point (mpi_point_t result, gcry_mpi_t scalar,
			     mpi_point_t point, mpi_ec_t ctx)
{
  byte param_block_buf[S390X_PCC_PARAM_BLOCK_SIZE];
  byte *param_out_x = NULL;
  byte *param_out_y = NULL;
  byte *param_in_x = NULL;
  byte *param_in_y = NULL;
  byte *param_scalar = NULL;
  unsigned int field_nbits;
  unsigned int pcc_func;
  gcry_mpi_t x, y;
  gcry_mpi_t d = NULL;
  int rc = -1;

  if (ctx->name == NULL)
    return -1;

  if (!(_gcry_get_hw_features () & HWF_S390X_MSA_9))
    return -1; /* ECC acceleration not supported by HW. */

  if (ctx->model == MPI_EC_MONTGOMERY)
    return s390_mul_point_montgomery (result, scalar, point, ctx,
				      param_block_buf);

  if (ctx->model == MPI_EC_WEIERSTRASS && ctx->nbits == 256 &&
      strcmp (ctx->name, "NIST P-256") == 0)
    {
      struct pcc_param_block_nistp256_s
      {
	byte out_x[256 / 8];
	byte out_y[256 / 8];
	byte in_x[256 / 8];
	byte in_y[256 / 8];
	byte scalar[256 / 8];
	byte c_and_ribm[64];
      } *params = (void *)param_block_buf;

      memset (params->c_and_ribm, 0, sizeof(params->c_and_ribm));

      pcc_func = PCC_FUNCTION_NIST_P256;
      field_nbits = 256;
      param_out_x = params->out_x;
      param_out_y = params->out_y;
      param_in_x = params->in_x;
      param_in_y = params->in_y;
      param_scalar = params->scalar;
    }
  else if (ctx->model == MPI_EC_WEIERSTRASS && ctx->nbits == 384 &&
           strcmp (ctx->name, "NIST P-384") == 0)
    {
      struct pcc_param_block_nistp384_s
      {
	byte out_x[384 / 8];
	byte out_y[384 / 8];
	byte in_x[384 / 8];
	byte in_y[384 / 8];
	byte scalar[384 / 8];
	byte c_and_ribm[64];
      } *params = (void *)param_block_buf;

      memset (params->c_and_ribm, 0, sizeof(params->c_and_ribm));

      pcc_func = PCC_FUNCTION_NIST_P384;
      field_nbits = 384;
      param_out_x = params->out_x;
      param_out_y = params->out_y;
      param_in_x = params->in_x;
      param_in_y = params->in_y;
      param_scalar = params->scalar;
    }
  else if (ctx->model == MPI_EC_WEIERSTRASS && ctx->nbits == 521 &&
           strcmp (ctx->name, "NIST P-521") == 0)
    {
      struct pcc_param_block_nistp521_s
      {
	byte out_x[640 / 8]; /* note: first 14 bytes not modified by pcc */
	byte out_y[640 / 8]; /* note: first 14 bytes not modified by pcc */
	byte in_x[640 / 8];
	byte in_y[640 / 8];
	byte scalar[640 / 8];
	byte c_and_ribm[64];
      } *params = (void *)param_block_buf;

      memset (params->out_x, 0, 14);
      memset (params->out_y, 0, 14);
      memset (params->c_and_ribm, 0, sizeof(params->c_and_ribm));

      pcc_func = PCC_FUNCTION_NIST_P521;
      field_nbits = 640;
      param_out_x = params->out_x;
      param_out_y = params->out_y;
      param_in_x = params->in_x;
      param_in_y = params->in_y;
      param_scalar = params->scalar;
    }
  else if (ctx->model == MPI_EC_EDWARDS && ctx->nbits == 255 &&
           strcmp (ctx->name, "Ed25519") == 0)
    {
      struct pcc_param_block_ed25519_s
      {
	byte out_x[256 / 8];
	byte out_y[256 / 8];
	byte in_x[256 / 8];
	byte in_y[256 / 8];
	byte scalar[256 / 8];
	byte c_and_ribm[64];
      } *params = (void *)param_block_buf;

      memset (params->c_and_ribm, 0, sizeof(params->c_and_ribm));

      pcc_func = PCC_FUNCTION_ED25519;
      field_nbits = 256;
      param_out_x = params->out_x;
      param_out_y = params->out_y;
      param_in_x = params->in_x;
      param_in_y = params->in_y;
      param_scalar = params->scalar;
    }
  else if (ctx->model == MPI_EC_EDWARDS && ctx->nbits == 448 &&
           strcmp (ctx->name, "Ed448") == 0)
    {
      struct pcc_param_block_ed448_s
      {
	byte out_x[512 / 8]; /* note: first 8 bytes not modified by pcc */
	byte out_y[512 / 8]; /* note: first 8 bytes not modified by pcc */
	byte in_x[512 / 8];
	byte in_y[512 / 8];
	byte scalar[512 / 8];
	byte c_and_ribm[64];
      } *params = (void *)param_block_buf;

      memset (params->out_x, 0, 8);
      memset (params->out_y, 0, 8);
      memset (params->c_and_ribm, 0, sizeof(params->c_and_ribm));

      pcc_func = PCC_FUNCTION_ED448;
      field_nbits = 512;
      param_out_x = params->out_x;
      param_out_y = params->out_y;
      param_in_x = params->in_x;
      param_in_y = params->in_y;
      param_scalar = params->scalar;
    }

  if (param_scalar == NULL)
    return -1; /* No curve match. */

  if (!(pcc_query () & km_function_to_mask (pcc_func)))
    return -1; /* HW does not support acceleration for this curve. */

  x = mpi_new (0);
  y = mpi_new (0);

  if (_gcry_mpi_ec_get_affine (x, y, point, ctx) < 0)
    {
      /* Point at infinity. */
      goto out;
    }

  if (mpi_has_sign (scalar) || mpi_cmp (scalar, ctx->n) >= 0)
    {
      d = mpi_is_secure (scalar) ? mpi_snew (ctx->nbits) : mpi_new (ctx->nbits);
      _gcry_mpi_mod (d, scalar, ctx->n);
    }
  else
    {
      d = scalar;
    }

  if (mpi_copy_to_raw (param_in_x, field_nbits / 8, x) < 0)
    goto out;

  if (mpi_copy_to_raw (param_in_y, field_nbits / 8, y) < 0)
    goto out;

  if (mpi_copy_to_raw (param_scalar, field_nbits / 8, d) < 0)
    goto out;

  if (pcc_scalar_multiply (pcc_func, param_block_buf) != 0)
    goto out;

  _gcry_mpi_set_buffer (result->x, param_out_x, field_nbits / 8, 0);
  _gcry_mpi_set_buffer (result->y, param_out_y, field_nbits / 8, 0);
  mpi_set_ui (result->z, 1);
  mpi_normalize (result->x);
  mpi_normalize (result->y);
  if (ctx->model == MPI_EC_EDWARDS)
    mpi_point_resize (result, ctx);

  rc = 0;

out:
  if (d != scalar)
    mpi_release (d);
  mpi_release (y);
  mpi_release (x);
  wipememory (param_block_buf, S390X_PCC_PARAM_BLOCK_SIZE);

  return rc;
}


static int
s390_mul_point_montgomery (mpi_point_t result, gcry_mpi_t scalar,
			   mpi_point_t point, mpi_ec_t ctx,
			   byte *param_block_buf)
{
  byte *param_out_x = NULL;
  byte *param_in_x = NULL;
  byte *param_scalar = NULL;
  unsigned int field_nbits;
  unsigned int pcc_func;
  gcry_mpi_t x;
  gcry_mpi_t d = NULL;
  int rc = -1;

  if (ctx->nbits == 255 && strcmp (ctx->name, "Curve25519") == 0)
    {
      struct pcc_param_block_x25519_s
      {
	byte out_x[256 / 8];
	byte in_x[256 / 8];
	byte scalar[256 / 8];
	byte c_and_ribm[64];
      } *params = (void *)param_block_buf;

      memset (params->c_and_ribm, 0, sizeof(params->c_and_ribm));

      pcc_func = PCC_FUNCTION_X25519;
      field_nbits = 256;
      param_out_x = params->out_x;
      param_in_x = params->in_x;
      param_scalar = params->scalar;
    }
  else if (ctx->nbits == 448 && strcmp (ctx->name, "X448") == 0)
    {
      struct pcc_param_block_x448_s
      {
	byte out_x[512 / 8]; /* note: first 8 bytes not modified by pcc */
	byte in_x[512 / 8];
	byte scalar[512 / 8];
	byte c_and_ribm[64];
      } *params = (void *)param_block_buf;

      memset (params->out_x, 0, 8);
      memset (params->c_and_ribm, 0, sizeof(params->c_and_ribm));

      pcc_func = PCC_FUNCTION_X448;
      field_nbits = 512;
      param_out_x = params->out_x;
      param_in_x = params->in_x;
      param_scalar = params->scalar;
    }

  if (param_scalar == NULL)
    return -1; /* No curve match. */

  if (!(pcc_query () & km_function_to_mask (pcc_func)))
    return -1; /* HW does not support acceleration for this curve. */

  x = mpi_new (0);

  if (mpi_is_opaque (scalar))
    {
      const unsigned int pbits = ctx->nbits;
      unsigned int n;
      unsigned char *raw;

      raw = _gcry_mpi_get_opaque_copy (scalar, &n);
      if ((n + 7) / 8 != (pbits + 7) / 8)
        log_fatal ("scalar size (%d) != prime size (%d)\n",
                   (n + 7) / 8, (pbits + 7) / 8);

      reverse_buffer (raw, (n + 7 ) / 8);
      if ((pbits % 8))
        raw[0] &= (1 << (pbits % 8)) - 1;
      raw[0] |= (1 << ((pbits + 7) % 8));
      raw[(pbits + 7) / 8 - 1] &= (256 - ctx->h);
      d = mpi_is_secure (scalar) ? mpi_snew (pbits) : mpi_new (pbits);
      _gcry_mpi_set_buffer (d, raw, (n + 7) / 8, 0);
      xfree (raw);
    }
  else
    {
      d = scalar;
    }

  if (_gcry_mpi_ec_get_affine (x, NULL, point, ctx) < 0)
    {
      /* Point at infinity. */
      goto out;
    }

  if (mpi_copy_to_raw (param_in_x, field_nbits / 8, x) < 0)
    goto out;

  if (mpi_copy_to_raw (param_scalar, field_nbits / 8, d) < 0)
    goto out;

  if (pcc_scalar_multiply (pcc_func, param_block_buf) != 0)
    goto out;

  _gcry_mpi_set_buffer (result->x, param_out_x, field_nbits / 8, 0);
  mpi_set_ui (result->z, 1);
  mpi_point_resize (result, ctx);

  rc = 0;

out:
  if (d != scalar)
    mpi_release (d);
  mpi_release (x);
  wipememory (param_block_buf, S390X_PCC_PARAM_BLOCK_SIZE);

  return rc;
}

#endif /* HAVE_GCC_INLINE_ASM_S390X */
