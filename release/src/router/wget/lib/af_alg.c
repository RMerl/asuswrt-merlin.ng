/* af_alg.c - Compute message digests from file streams and buffers.
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

/* Written by Matteo Croce <mcroce@redhat.com>, 2018.  */

#include <config.h>

#include "af_alg.h"

#if USE_LINUX_CRYPTO_API

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <linux/if_alg.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>

#include "sys-limits.h"

#define BLOCKSIZE 32768

/* Return a newly created socket for ALG.
   On error, return a negative error number.  */
static int
alg_socket (char const *alg)
{
  struct sockaddr_alg salg = {
    .salg_family = AF_ALG,
    .salg_type = "hash",
  };
  /* Copy alg into salg.salg_name, without calling strcpy nor strlen.  */
  for (size_t i = 0; (salg.salg_name[i] = alg[i]) != '\0'; i++)
    if (i == sizeof salg.salg_name - 1)
      /* alg is too long.  */
      return -EINVAL;

  int cfd = socket (AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
  if (cfd < 0)
    return -EAFNOSUPPORT;
  int ofd = (bind (cfd, (struct sockaddr *) &salg, sizeof salg) == 0
             ? accept4 (cfd, NULL, 0, SOCK_CLOEXEC)
             : -1);
  close (cfd);
  return ofd < 0 ? -EAFNOSUPPORT : ofd;
}

int
afalg_buffer (const char *buffer, size_t len, const char *alg,
              void *resblock, ssize_t hashlen)
{
  /* On Linux < 4.9, the value for an empty stream is wrong (all zeroes).
     See <https://patchwork.kernel.org/patch/9308641/>.
     This was not fixed properly until November 2016,
     see <https://patchwork.kernel.org/patch/9434741/>.  */
  if (len == 0)
    return -EAFNOSUPPORT;

  int ofd = alg_socket (alg);
  if (ofd < 0)
    return ofd;

  int result;

  for (;;)
    {
      ssize_t size = (len > BLOCKSIZE ? BLOCKSIZE : len);
      if (send (ofd, buffer, size, MSG_MORE) != size)
        {
          result = -EAFNOSUPPORT;
          break;
        }
      buffer += size;
      len -= size;
      if (len == 0)
        {
          result = read (ofd, resblock, hashlen) == hashlen ? 0 : -EAFNOSUPPORT;
          break;
        }
    }

  close (ofd);
  return result;
}

int
afalg_stream (FILE *stream, const char *alg,
              void *resblock, ssize_t hashlen)
{
  int ofd = alg_socket (alg);
  if (ofd < 0)
    return ofd;

  /* If STREAM's size is known and nonzero and not too large, attempt
     sendfile to pipe the data.  The nonzero restriction avoids issues
     with /proc files that pretend to be empty, and lets the classic
     read-write loop work around an empty-input bug noted below.  */
  int fd = fileno (stream);
  int result;
  struct stat st;
  off_t off = ftello (stream);
  if (0 <= off && fstat (fd, &st) == 0
      && (S_ISREG (st.st_mode) || S_TYPEISSHM (&st) || S_TYPEISTMO (&st))
      && off < st.st_size && st.st_size - off < SYS_BUFSIZE_MAX)
    {
      /* Make sure the offset of fileno (stream) reflects how many bytes
         have been read from stream before this function got invoked.
         Note: fflush on an input stream after ungetc does not work as expected
         on some platforms.  Therefore this situation is not supported here.  */
      if (fflush (stream))
        result = -EIO;
      else
        {
          off_t nbytes = st.st_size - off;
          if (sendfile (ofd, fd, &off, nbytes) == nbytes)
            {
              if (read (ofd, resblock, hashlen) == hashlen)
                {
                  /* The input buffers of stream are no longer valid.  */
                  if (lseek (fd, off, SEEK_SET) != (off_t)-1)
                    result = 0;
                  else
                    /* The file position of fd has not changed.  */
                    result = -EAFNOSUPPORT;
                }
              else
                /* The file position of fd has not changed.  */
                result = -EAFNOSUPPORT;
            }
          else
            /* The file position of fd has not changed.  */
            result = -EAFNOSUPPORT;
       }
    }
  else
    {
      /* sendfile not possible, do a classic read-write loop.  */

      /* Number of bytes to seek (backwards) in case of error.  */
      off_t nseek = 0;

      for (;;)
        {
          char buf[BLOCKSIZE];
          /* When the stream is not seekable, start with a single-byte block,
             so that we can use ungetc() in the case that send() fails.  */
          size_t blocksize = (nseek == 0 && off < 0 ? 1 : BLOCKSIZE);
          ssize_t size = fread (buf, 1, blocksize, stream);
          if (size == 0)
            {
              /* On Linux < 4.9, the value for an empty stream is wrong (all 0).
                 See <https://patchwork.kernel.org/patch/9308641/>.
                 This was not fixed properly until November 2016,
                 see <https://patchwork.kernel.org/patch/9434741/>.  */
              result = ferror (stream) ? -EIO : nseek == 0 ? -EAFNOSUPPORT : 0;
              break;
            }
          nseek -= size;
          if (send (ofd, buf, size, MSG_MORE) != size)
            {
              if (nseek == -1)
                {
                  /* 1 byte of pushback buffer is guaranteed on stream, even
                     if stream is not seekable.  */
                  ungetc ((unsigned char) buf[0], stream);
                  result = -EAFNOSUPPORT;
                }
              else if (fseeko (stream, nseek, SEEK_CUR) == 0)
                /* The position of stream has been restored.  */
                result = -EAFNOSUPPORT;
              else
                result = -EIO;
              break;
            }

          /* Don't assume that EOF is sticky. See:
             <https://sourceware.org/bugzilla/show_bug.cgi?id=19476>.  */
          if (feof (stream))
            {
              result = 0;
              break;
            }
        }

      if (result == 0 && read (ofd, resblock, hashlen) != hashlen)
        {
          if (nseek == 0 || fseeko (stream, nseek, SEEK_CUR) == 0)
            /* The position of stream has been restored.  */
            result = -EAFNOSUPPORT;
          else
            result = -EIO;
        }
    }
  close (ofd);
  return result;
}

#endif
