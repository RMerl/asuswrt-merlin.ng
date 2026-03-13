/* base32.h -- Encode binary data using printable characters.
   Copyright (C) 2004-2006, 2009-2024 Free Software Foundation, Inc.
   Adapted from Simon Josefsson's base64 code by Gijs van Tulder.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef BASE32_H
#define BASE32_H

/* This file uses _GL_INLINE_HEADER_BEGIN.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get idx_t.  */
#include <idx.h>

/* Pacify GCC in isubase32.  */
#if 4 < __GNUC__ + (3 <= __GNUC_MINOR__) && !defined __clang__
# pragma GCC diagnostic ignored "-Wtype-limits"
#endif

_GL_INLINE_HEADER_BEGIN
#ifndef BASE32_INLINE
# define BASE32_INLINE _GL_INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* This uses that the expression (n+(k-1))/k means the smallest
   integer >= n/k, i.e., the ceiling of n/k.  */
#define BASE32_LENGTH(inlen) ((((inlen) + 4) / 5) * 8)

struct base32_decode_context
{
  int i;
  char buf[8];
};

extern signed char const base32_to_int[256];

BASE32_INLINE bool
isubase32 (unsigned char ch)
{
  return ch < sizeof base32_to_int && 0 <= base32_to_int[ch];
}

BASE32_INLINE bool
isbase32 (char ch)
{
  return isubase32 (ch);
}

extern void base32_encode (const char *restrict in, idx_t inlen,
                           char *restrict out, idx_t outlen);

extern idx_t base32_encode_alloc (const char *in, idx_t inlen, char **out);

/* Initialize decode-context buffer, CTX.  */
BASE32_INLINE void
base32_decode_ctx_init (struct base32_decode_context *ctx)
{
  ctx->i = 0;
}

extern bool base32_decode_ctx (struct base32_decode_context *ctx,
                               const char *restrict in, idx_t inlen,
                               char *restrict out, idx_t *outlen);

extern bool base32_decode_alloc_ctx (struct base32_decode_context *ctx,
                                     const char *in, idx_t inlen,
                                     char **out, idx_t *outlen);

#define base32_decode(in, inlen, out, outlen) \
        base32_decode_ctx (NULL, in, inlen, out, outlen)

#define base32_decode_alloc(in, inlen, out, outlen) \
        base32_decode_alloc_ctx (NULL, in, inlen, out, outlen)

#ifdef __cplusplus
}
#endif

_GL_INLINE_HEADER_END

#endif /* BASE32_H */
