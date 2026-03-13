/* Declaration of functions and data types used for MD5 sum computing
   library functions.
   Copyright (C) 1995-1997, 1999-2001, 2004-2006, 2008-2024 Free Software
   Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef _MD5_H
#define _MD5_H 1

/* This file uses HAVE_OPENSSL_MD5.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdio.h>
#include <stdint.h>

# if HAVE_OPENSSL_MD5
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
#    undef HAVE_OPENSSL_MD5
#   endif
#  endif
#  if HAVE_OPENSSL_MD5
#   include <openssl/md5.h>
#  endif
# endif

#define MD5_DIGEST_SIZE 16
#define MD5_BLOCK_SIZE 64

#if defined __clang__
  /* clang really only groks GNU C 4.2, regardless of its value of __GNUC__.  */
# undef __GNUC_PREREQ
# define __GNUC_PREREQ(maj, min) ((maj) < 4 + ((min) <= 2))
#endif
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) ((maj) < __GNUC__ + ((min) <= __GNUC_MINOR__))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif

#ifndef __THROW
# if defined __cplusplus && (__GNUC_PREREQ (2,8) || __clang_major__ >= 4)
#  if __cplusplus >= 201103L
#   define __THROW      noexcept (true)
#  else
#   define __THROW      throw ()
#  endif
# else
#  define __THROW
# endif
#endif

#ifndef _LIBC
# define __md5_buffer md5_buffer
# define __md5_finish_ctx md5_finish_ctx
# define __md5_init_ctx md5_init_ctx
# define __md5_process_block md5_process_block
# define __md5_process_bytes md5_process_bytes
# define __md5_read_ctx md5_read_ctx
# define __md5_stream md5_stream
#endif

# ifdef __cplusplus
extern "C" {
# endif

# if HAVE_OPENSSL_MD5
#  define GL_OPENSSL_NAME 5
#  include "gl_openssl.h"
# else
/* Structure to save state of computation between the single steps.  */
struct md5_ctx
{
  uint32_t A;
  uint32_t B;
  uint32_t C;
  uint32_t D;

  uint32_t total[2];
  uint32_t buflen;     /* ≥ 0, ≤ 128 */
  uint32_t buffer[32]; /* 128 bytes; the first buflen bytes are in use */
};

/*
 * The following three functions are build up the low level used in
 * the functions 'md5_stream' and 'md5_buffer'.
 */

/* Initialize structure containing state of computation.
   (RFC 1321, 3.3: Step 3)  */
extern void __md5_init_ctx (struct md5_ctx *ctx) __THROW;

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is necessary that LEN is a multiple of 64!!! */
extern void __md5_process_block (const void *buffer, size_t len,
                                 struct md5_ctx *ctx) __THROW;

/* Starting with the result of former calls of this function (or the
   initialization function update the context for the next LEN bytes
   starting at BUFFER.
   It is NOT required that LEN is a multiple of 64.  */
extern void __md5_process_bytes (const void *buffer, size_t len,
                                 struct md5_ctx *ctx) __THROW;

/* Process the remaining bytes in the buffer and put result from CTX
   in first 16 bytes following RESBUF.  The result is always in little
   endian byte order, so that a byte-wise output yields to the wanted
   ASCII representation of the message digest.  */
extern void *__md5_finish_ctx (struct md5_ctx *ctx, void *restrict resbuf)
     __THROW;


/* Put result from CTX in first 16 bytes following RESBUF.  The result is
   always in little endian byte order, so that a byte-wise output yields
   to the wanted ASCII representation of the message digest.  */
extern void *__md5_read_ctx (const struct md5_ctx *ctx, void *restrict resbuf)
     __THROW;


/* Compute MD5 message digest for LEN bytes beginning at BUFFER.  The
   result is always in little endian byte order, so that a byte-wise
   output yields to the wanted ASCII representation of the message
   digest.  */
extern void *__md5_buffer (const char *buffer, size_t len,
                           void *restrict resblock) __THROW;

# endif

/* Compute MD5 message digest for bytes read from STREAM.
   STREAM is an open file stream.  Regular files are handled more efficiently.
   The contents of STREAM from its current position to its end will be read.
   The case that the last operation on STREAM was an 'ungetc' is not supported.
   The resulting message digest number will be written into the 16 bytes
   beginning at RESBLOCK.  */
extern int __md5_stream (FILE *stream, void *resblock) __THROW;


# ifdef __cplusplus
}
# endif

#endif /* md5.h */

/*
 * Hey Emacs!
 * Local Variables:
 * coding: utf-8
 * End:
 */
