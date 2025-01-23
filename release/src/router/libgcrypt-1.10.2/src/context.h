/* context.h - Declarations for the context management
 * Copyright (C) 2013  g10 Code GmbH
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

#ifndef GCRY_CONTEXT_H
#define GCRY_CONTEXT_H

/* Context types as used in struct gcry_context.  */
#define CONTEXT_TYPE_EC 1  /* The context is used with EC functions.  */
#define CONTEXT_TYPE_RANDOM_OVERRIDE 2  /* Used with pubkey functions.  */

gcry_ctx_t _gcry_ctx_alloc (int type, size_t length, void (*deinit)(void*));
void *_gcry_ctx_get_pointer (gcry_ctx_t ctx, int type);
void *_gcry_ctx_find_pointer (gcry_ctx_t ctx, int type);


#endif /*GCRY_CONTEXT_H*/
