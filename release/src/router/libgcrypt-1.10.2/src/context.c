/* context.c - Context management
 * Copyright (C) 2013  g10 Code GmbH
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
#include <stdarg.h>
#include <unistd.h>

#include "g10lib.h"
#include "mpi.h"
#include "context.h"

#define CTX_MAGIC "cTx"
#define CTX_MAGIC_LEN 3


/* The definition of the generic context object.  The public typedef
   gcry_ctx_t is used to access it.  */
struct gcry_context
{
  char magic[CTX_MAGIC_LEN]; /* Magic value to cross check that this
                                is really a context object. */
  char type;     /* The type of the context (CONTEXT_TYPE_foo).  */

  void (*deinit)(void*); /* Function used to free the private part. */
  PROPERLY_ALIGNED_TYPE u;
};


/* Allocate a fresh generic context of contect TYPE and allocate
   LENGTH extra bytes for private use of the type handler. DEINIT is a
   function used called to deinitialize the private part; it may be
   NULL if de-initialization is not required.  Returns NULL and sets
   ERRNO if memory allocation failed.  */
gcry_ctx_t
_gcry_ctx_alloc (int type, size_t length, void (*deinit)(void*))
{
  gcry_ctx_t ctx;

  switch (type)
    {
    case CONTEXT_TYPE_EC:
    case CONTEXT_TYPE_RANDOM_OVERRIDE:
      break;
    default:
      log_bug ("bad context type %d given to _gcry_ctx_alloc\n", type);
      break;
    }

  if (length < sizeof (PROPERLY_ALIGNED_TYPE))
    length = sizeof (PROPERLY_ALIGNED_TYPE);

  ctx = xtrycalloc (1, sizeof *ctx - sizeof (PROPERLY_ALIGNED_TYPE) + length);
  if (!ctx)
    return NULL;
  memcpy (ctx->magic, CTX_MAGIC, CTX_MAGIC_LEN);
  ctx->type = type;
  ctx->deinit = deinit;

  return ctx;
}


/* Return a pointer to the private part of the context CTX.  TYPE is
   the requested context type.  Using an explicit type allows to cross
   check the type and eventually allows to store several private
   contexts in one context object.  The function does not return an
   error but aborts if the provided CTX is not valid.  */
void *
_gcry_ctx_get_pointer (gcry_ctx_t ctx, int type)
{
  if (!ctx || memcmp (ctx->magic, CTX_MAGIC, CTX_MAGIC_LEN))
    log_fatal ("bad pointer %p passed to _gcry_ctx_get_pointer\n", ctx);
  if (ctx->type != type)
    log_fatal ("wrong context type %d request for context %p of type %d\n",
               type, ctx, ctx->type);
  return &ctx->u;
}

/* Return a pointer to the private part of the context CTX.  TYPE is
   the requested context type.  Using an explicit type allows to cross
   check the type and eventually allows to store several private
   contexts in one context object.  In contrast to
   _gcry_ctx_get_pointer, this function returns NULL if no context for
   the given type was found.  If CTX is NULL the function does not
   abort but returns NULL.  */
void *
_gcry_ctx_find_pointer (gcry_ctx_t ctx, int type)
{
  if (!ctx)
    return NULL;
  if (memcmp (ctx->magic, CTX_MAGIC, CTX_MAGIC_LEN))
    log_fatal ("bad pointer %p passed to _gcry_ctx_get_pointer\n", ctx);
  if (ctx->type != type)
    return NULL;
  return &ctx->u;
}


/* Release the generic context CTX.  */
void
_gcry_ctx_release (gcry_ctx_t ctx)
{
  if (!ctx)
    return;
  if (memcmp (ctx->magic, CTX_MAGIC, CTX_MAGIC_LEN))
    log_fatal ("bad pointer %p passed to gcry_ctx_relase\n", ctx);
  switch (ctx->type)
    {
    case CONTEXT_TYPE_EC:
    case CONTEXT_TYPE_RANDOM_OVERRIDE:
      break;
    default:
      log_fatal ("bad context type %d detected in gcry_ctx_relase\n",
                 ctx->type);
      break;
    }
  if (ctx->deinit)
    ctx->deinit (&ctx->u);
  xfree (ctx);
}
