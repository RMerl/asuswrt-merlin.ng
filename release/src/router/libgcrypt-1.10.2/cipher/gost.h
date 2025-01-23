/* gost.h - GOST 28147-89 implementation
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

#ifndef _GCRY_GOST_H
#define _GCRY_GOST_H

typedef struct {
  u32 key[8];
  const u32 *sbox;
  unsigned int mesh_counter;
  unsigned int mesh_limit;
} GOST28147_context;

/* This is a simple interface that will be used by GOST R 34.11-94 */
unsigned int _gcry_gost_enc_data (const u32 *key,
    u32 *o1, u32 *o2, u32 n1, u32 n2, int cryptopro);

#endif
