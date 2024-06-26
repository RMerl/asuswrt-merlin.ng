/* Declarations of functions and data types used for SHA512 and SHA384 sum
   library functions.
   Copyright (C) 2005-2006, 2008-2024 Free Software Foundation, Inc.

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

#ifndef SHA512_H
# define SHA512_H 1

/* This file uses HAVE_OPENSSL_SHA512.  */
# if !_GL_CONFIG_H_INCLUDED
#  error "Please include config.h first."
# endif

# include <stdio.h>
# include "u64.h"

# if HAVE_OPENSSL_SHA512
#  ifndef OPENSSL_API_COMPAT
#   define OPENSSL_API_COMPAT 0x10101000L /* FIXME: Use OpenSSL 1.1+ API.  */
#  endif
/* If <openssl/macros.h> would give a compile-time error, don't use OpenSSL.  */
#  include <openssl/opensslv.h>
#  if OPENSSL_VERSION_MAJOR >= 3
#   include <openssl/configuration.h>
#   if (OPENSSL_CONFIGURED_API \
        < (OPENSSL_API_COMPAT < 0x900000L ? OPENSSL_API_COMPAT : \
           ((OPENSSL_API_COMPAT >> 28) & 0xF) * 10000 \
           + ((OPENSSL_API_COMPAT >> 20) & 0xFF) * 100 \
           + ((OPENSSL_API_COMPAT >> 12) & 0xFF)))
#    undef HAVE_OPENSSL_SHA512
#   endif
#  endif
#  if HAVE_OPENSSL_SHA512
#   include <openssl/sha.h>
#  endif
# endif

# ifdef __cplusplus
extern "C" {
# endif

enum { SHA384_DIGEST_SIZE = 384 / 8 };
enum { SHA512_DIGEST_SIZE = 512 / 8 };

# if HAVE_OPENSSL_SHA512
#  define GL_OPENSSL_NAME 384
#  include "gl_openssl.h"
#  define GL_OPENSSL_NAME 512
#  include "gl_openssl.h"
# else
/* Structure to save state of computation between the single steps.  */
struct sha512_ctx
{
  u64 state[8];

  u64 total[2];
  size_t buflen;  /* ≥ 0, ≤ 256 */
  u64 buffer[32]; /* 256 bytes; the first buflen bytes are in use */
};

/* Initialize structure containing state of computation. */
extern void sha512_init_ctx (struct sha512_ctx *ctx);
extern void sha384_init_ctx (struct sha512_ctx *ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is necessary that LEN is a multiple of 128!!! */
extern void sha512_process_block (const void *buffer, size_t len,
                                  struct sha512_ctx *ctx);

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is NOT required that LEN is a multiple of 128.  */
extern void sha512_process_bytes (const void *buffer, size_t len,
                                  struct sha512_ctx *ctx);

/* Process the remaining bytes in the buffer and put result from CTX
   in first 64 (48) bytes following RESBUF.  The result is always in little
   endian byte order, so that a byte-wise output yields to the wanted
   ASCII representation of the message digest.  */
extern void *sha512_finish_ctx (struct sha512_ctx *ctx, void *restrict resbuf);
extern void *sha384_finish_ctx (struct sha512_ctx *ctx, void *restrict resbuf);


/* Put result from CTX in first 64 (48) bytes following RESBUF.  The result is
   always in little endian byte order, so that a byte-wise output yields
   to the wanted ASCII representation of the message digest.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
extern void *sha512_read_ctx (const struct sha512_ctx *ctx,
                              void *restrict resbuf);
extern void *sha384_read_ctx (const struct sha512_ctx *ctx,
                              void *restrict resbuf);


/* Compute SHA512 (SHA384) message digest for LEN bytes beginning at BUFFER.
   The result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
extern void *sha512_buffer (const char *buffer, size_t len,
                            void *restrict resblock);
extern void *sha384_buffer (const char *buffer, size_t len,
                            void *restrict resblock);

# endif

/* Compute SHA512 (SHA384) message digest for bytes read from STREAM.
   STREAM is an open file stream.  Regular files are handled more efficiently.
   The contents of STREAM from its current position to its end will be read.
   The case that the last operation on STREAM was an 'ungetc' is not supported.
   The resulting message digest number will be written into the 64 (48) bytes
   beginning at RESBLOCK.  */
extern int sha512_stream (FILE *stream, void *resblock);
extern int sha384_stream (FILE *stream, void *resblock);


# ifdef __cplusplus
}
# endif

#endif

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
