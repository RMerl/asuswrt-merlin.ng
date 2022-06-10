/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file fdio.c
 *
 * \brief Low-level compatibility wrappers for fd-based IO.
 **/

#include "orconfig.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "lib/fdio/fdio.h"
#include "lib/cc/torint.h"
#include "lib/err/torerr.h"

#include <stdlib.h>
#include <stdio.h>

/* Some old versions of Unix didn't define constants for these values,
 * and instead expect you to say 0, 1, or 2. */

/** @cond */
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
/** @endcond */

/** Return the position of <b>fd</b> with respect to the start of the file. */
off_t
tor_fd_getpos(int fd)
{
#ifdef _WIN32
  return (off_t) _lseeki64(fd, 0, SEEK_CUR);
#else
  return (off_t) lseek(fd, 0, SEEK_CUR);
#endif
}

/** Move <b>fd</b> to the end of the file. Return -1 on error, 0 on success.
 * If the file is a pipe, do nothing and succeed.
 **/
int
tor_fd_seekend(int fd)
{
#ifdef _WIN32
  return _lseeki64(fd, 0, SEEK_END) < 0 ? -1 : 0;
#else
  off_t rc = lseek(fd, 0, SEEK_END) < 0 ? -1 : 0;
#ifdef ESPIPE
  /* If we get an error and ESPIPE, then it's a pipe or a socket of a fifo:
   * no need to worry. */
  if (rc < 0 && errno == ESPIPE)
    rc = 0;
#endif /* defined(ESPIPE) */
  return (rc < 0) ? -1 : 0;
#endif /* defined(_WIN32) */
}

/** Move <b>fd</b> to position <b>pos</b> in the file. Return -1 on error, 0
 * on success. */
int
tor_fd_setpos(int fd, off_t pos)
{
#ifdef _WIN32
  return _lseeki64(fd, pos, SEEK_SET) < 0 ? -1 : 0;
#else
  return lseek(fd, pos, SEEK_SET) < 0 ? -1 : 0;
#endif
}

/** Replacement for ftruncate(fd, 0): move to the front of the file and remove
 * all the rest of the file. Return -1 on error, 0 on success. */
int
tor_ftruncate(int fd)
{
  /* Rumor has it that some versions of ftruncate do not move the file pointer.
   */
  if (tor_fd_setpos(fd, 0) < 0)
    return -1;

#ifdef _WIN32
  return _chsize(fd, 0);
#else
  return ftruncate(fd, 0);
#endif
}

/** Minimal version of write_all, for use by logging. */
int
write_all_to_fd_minimal(int fd, const char *buf, size_t count)
{
  size_t written = 0;
  raw_assert(count < SSIZE_MAX);

  while (written < count) {
    ssize_t result = write(fd, buf+written, count-written);
    if (result<0)
      return -1;
    written += result;
  }
  return 0;
}
