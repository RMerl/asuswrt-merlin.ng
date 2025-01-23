/* ecc-ecdh.c  -  Elliptic Curve Diffie-Hellman key agreement
 * Copyright (C) 2019 g10 Code GmbH
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "g10lib.h"
#include "mpi.h"
#include "cipher.h"
#include "context.h"
#include "ec-context.h"
#include "ecc-common.h"

#define ECC_CURVE25519_BYTES 32
#define ECC_CURVE448_BYTES   56

static gpg_err_code_t
prepare_ec (mpi_ec_t *r_ec, const char *name)
{
  int flags = 0;

  if (!strcmp (name, "Curve25519"))
    flags = PUBKEY_FLAG_DJB_TWEAK;

  return _gcry_mpi_ec_internal_new (r_ec, &flags, "ecc_mul_point", NULL, name);
}

unsigned int
_gcry_ecc_get_algo_keylen (int curveid)
{
  unsigned int len = 0;

  if (curveid == GCRY_ECC_CURVE25519)
    len = ECC_CURVE25519_BYTES;
  else if (curveid == GCRY_ECC_CURVE448)
    len = ECC_CURVE448_BYTES;

  return len;
}

gpg_error_t
_gcry_ecc_mul_point (int curveid, unsigned char *result,
                     const unsigned char *scalar, const unsigned char *point)
{
  unsigned int nbits;
  unsigned int nbytes;
  const char *curve;
  gpg_err_code_t err;
  gcry_mpi_t mpi_k;
  mpi_ec_t ec;
  mpi_point_struct Q;
  gcry_mpi_t x;
  unsigned int len;
  unsigned char *buf;

  if (curveid == GCRY_ECC_CURVE25519)
    curve = "Curve25519";
  else if (curveid == GCRY_ECC_CURVE448)
    curve = "X448";
  else
    return gpg_error (GPG_ERR_UNKNOWN_CURVE);

  err = prepare_ec (&ec, curve);
  if (err)
    return err;

  nbits = ec->nbits;
  nbytes = (nbits + 7)/8;

  mpi_k = _gcry_mpi_set_opaque_copy (NULL, scalar, nbytes*8);
  x = mpi_new (nbits);
  point_init (&Q);

  if (point)
    {
      gcry_mpi_t mpi_u = _gcry_mpi_set_opaque_copy (NULL, point, nbytes*8);
      mpi_point_struct P;

      point_init (&P);
      err = _gcry_ecc_mont_decodepoint (mpi_u, ec, &P);
      _gcry_mpi_release (mpi_u);
      if (err)
        goto leave;
      _gcry_mpi_ec_mul_point (&Q, mpi_k, &P, ec);
      point_free (&P);
    }
  else
    _gcry_mpi_ec_mul_point (&Q, mpi_k, ec->G, ec);

  _gcry_mpi_ec_get_affine (x, NULL, &Q, ec);

  buf = _gcry_mpi_get_buffer (x, nbytes, &len, NULL);
  if (!buf)
    err = gpg_error_from_syserror ();
  else
    {
      memcpy (result, buf, nbytes);
      xfree (buf);
    }

 leave:
  _gcry_mpi_release (x);
  point_free (&Q);
  _gcry_mpi_release (mpi_k);
  _gcry_mpi_ec_free (ec);
  return err;
}
