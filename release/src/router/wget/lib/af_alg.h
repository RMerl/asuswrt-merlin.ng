/* af_alg.h - Compute message digests from file streams and buffers.
   Copyright (C) 2018-2022 Free Software Foundation, Inc.

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

/* Written by Matteo Croce <mcroce@redhat.com>, 2018.
   Documentation by Bruno Haible <bruno@clisp.org>, 2018.  */

/* Declare specific functions for computing message digests
   using the Linux kernel crypto API, if available.  This kernel API gives
   access to specialized crypto instructions (that would also be available
   in user space) or to crypto devices (not directly available in user space).

   For a more complete set of facilities that use the Linux kernel crypto API,
   look at libkcapi.  */

#ifndef AF_ALG_H
# define AF_ALG_H 1

# include <stdio.h>
# include <errno.h>

# ifdef __cplusplus
extern "C" {
# endif

# if USE_LINUX_CRYPTO_API

/* Compute a message digest of a memory region.

   The memory region starts at BUFFER and is LEN bytes long.

   ALG is the message digest algorithm; see the file /proc/crypto.

   RESBLOCK points to a block of HASHLEN bytes, for the result.
   HASHLEN must be the length of the message digest, in bytes, in particular:

      alg    | hashlen
      -------+--------
      md5    | 16
      sha1   | 20
      sha224 | 28
      sha256 | 32
      sha384 | 48
      sha512 | 64

   If successful, fill RESBLOCK and return 0.
   Upon failure, return a negated error number.  */
int
afalg_buffer (const char *buffer, size_t len, const char *alg,
              void *resblock, ssize_t hashlen);

/* Compute a message digest of data read from STREAM.

   STREAM is an open file stream.  The last operation on STREAM should
   not be 'ungetc', and if STREAM is also open for writing it should
   have been fflushed since its last write.  Read from the current
   position to the end of STREAM.  Handle regular files efficiently.

   ALG is the message digest algorithm; see the file /proc/crypto.

   RESBLOCK points to a block of HASHLEN bytes, for the result.
   HASHLEN must be the length of the message digest, in bytes, in particular:

      alg    | hashlen
      -------+--------
      md5    | 16
      sha1   | 20
      sha224 | 28
      sha256 | 32
      sha384 | 48
      sha512 | 64

   If successful, fill RESBLOCK and return 0.
   Upon failure, return a negated error number.
   Unless returning 0 or -EIO, restore STREAM's file position so that
   the caller can fall back on some other method.  */
int
afalg_stream (FILE *stream, const char *alg,
              void *resblock, ssize_t hashlen);

# else

static inline int
afalg_buffer (const char *buffer, size_t len, const char *alg,
              void *resblock, ssize_t hashlen)
{
  return -EAFNOSUPPORT;
}

static inline int
afalg_stream (FILE *stream, const char *alg,
              void *resblock, ssize_t hashlen)
{
  return -EAFNOSUPPORT;
}

# endif

# ifdef __cplusplus
}
# endif

#endif /* AF_ALG_H */
