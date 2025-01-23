/* ec-internal.h - Internal declarations of ec*.c
 * Copyright (C) 2013 g10 Code GmbH
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

#ifndef GCRY_EC_INTERNAL_H
#define GCRY_EC_INTERNAL_H

#include <config.h>

void _gcry_mpi_ec_ed25519_mod (gcry_mpi_t a);

#ifndef ASM_DISABLED
void _gcry_mpi_ec_nist192_mod (gcry_mpi_t w, mpi_ec_t ctx);
void _gcry_mpi_ec_nist224_mod (gcry_mpi_t w, mpi_ec_t ctx);
void _gcry_mpi_ec_nist256_mod (gcry_mpi_t w, mpi_ec_t ctx);
void _gcry_mpi_ec_nist384_mod (gcry_mpi_t w, mpi_ec_t ctx);
void _gcry_mpi_ec_nist521_mod (gcry_mpi_t w, mpi_ec_t ctx);
#else
# define _gcry_mpi_ec_nist192_mod NULL
# define _gcry_mpi_ec_nist224_mod NULL
# define _gcry_mpi_ec_nist256_mod NULL
# define _gcry_mpi_ec_nist384_mod NULL
# define _gcry_mpi_ec_nist521_mod NULL
#endif

#ifdef HAVE_GCC_INLINE_ASM_S390X
int _gcry_s390x_ec_hw_mul_point (mpi_point_t result, gcry_mpi_t scalar,
				 mpi_point_t point, mpi_ec_t ctx);
# define mpi_ec_hw_mul_point _gcry_s390x_ec_hw_mul_point
#else
# define mpi_ec_hw_mul_point(r,s,p,c) (-1)
#endif

#endif /*GCRY_EC_INTERNAL_H*/
