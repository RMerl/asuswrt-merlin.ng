/* Duplicate an open file descriptor to a specified file descriptor.

   Copyright (C) 1999, 2004-2007, 2009-2024 Free Software Foundation, Inc.

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

/* written by Paul Eggert */

#include <config.h>

/* Specification.  */
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#undef dup2

#if defined _WIN32 && ! defined __CYGWIN__

/* Get declarations of the native Windows API functions.  */
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

# if HAVE_MSVC_INVALID_PARAMETER_HANDLER
#  include "msvc-inval.h"
# endif

/* Get _get_osfhandle.  */
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif

# if HAVE_MSVC_INVALID_PARAMETER_HANDLER
static int
dup2_nothrow (int fd, int desired_fd)
{
  int result;

  TRY_MSVC_INVAL
    {
      result = _dup2 (fd, desired_fd);
    }
  CATCH_MSVC_INVAL
    {
      errno = EBADF;
      result = -1;
    }
  DONE_MSVC_INVAL;

  return result;
}
# else
#  define dup2_nothrow _dup2
# endif

static int
ms_windows_dup2 (int fd, int desired_fd)
{
  int result;

  /* If fd is closed, mingw hangs on dup2 (fd, fd).  If fd is open,
     dup2 (fd, fd) returns 0, but all further attempts to use fd in
     future dup2 calls will hang.  */
  if (fd == desired_fd)
    {
      if ((HANDLE) _get_osfhandle (fd) == INVALID_HANDLE_VALUE)
        {
          errno = EBADF;
          return -1;
        }
      return fd;
    }

  /* Wine 1.0.1 return 0 when desired_fd is negative but not -1:
     https://bugs.winehq.org/show_bug.cgi?id=21289 */
  if (desired_fd < 0)
    {
      errno = EBADF;
      return -1;
    }

  result = dup2_nothrow (fd, desired_fd);

  if (result == 0)
    result = desired_fd;

  return result;
}

# define dup2 ms_windows_dup2

#elif defined __KLIBC__

# include <InnoTekLIBC/backend.h>

static int
klibc_dup2dirfd (int fd, int desired_fd)
{
  int tempfd;
  int dupfd;

  tempfd = open ("NUL", O_RDONLY);
  if (tempfd == -1)
    return -1;

  if (tempfd == desired_fd)
    {
      close (tempfd);

      char path[_MAX_PATH];
      if (__libc_Back_ioFHToPath (fd, path, sizeof (path)))
        return -1;

      return open(path, O_RDONLY);
    }

  dupfd = klibc_dup2dirfd (fd, desired_fd);

  close (tempfd);

  return dupfd;
}

static int
klibc_dup2 (int fd, int desired_fd)
{
  int dupfd;
  struct stat sbuf;

  dupfd = dup2 (fd, desired_fd);
  if (dupfd == -1 && errno == ENOTSUP \
      && !fstat (fd, &sbuf) && S_ISDIR (sbuf.st_mode))
    {
      close (desired_fd);

      return klibc_dup2dirfd (fd, desired_fd);
    }

  return dupfd;
}

# define dup2 klibc_dup2
#endif

int
rpl_dup2 (int fd, int desired_fd)
{
  int result;

#ifdef F_GETFL
  /* On Linux kernels 2.6.26-2.6.29, dup2 (fd, fd) returns -EBADF.
     On Cygwin 1.5.x, dup2 (1, 1) returns 0.
     On Cygwin 1.7.17, dup2 (1, -1) dumps core.
     On Cygwin 1.7.25, dup2 (1, 256) can dump core.
     On Haiku, dup2 (fd, fd) mistakenly clears FD_CLOEXEC.  */
# if HAVE_SETDTABLESIZE
  setdtablesize (desired_fd + 1);
# endif
  if (desired_fd < 0)
    fd = desired_fd;
  if (fd == desired_fd)
    return fcntl (fd, F_GETFL) == -1 ? -1 : fd;
#endif

  result = dup2 (fd, desired_fd);

  /* Correct an errno value on FreeBSD 6.1 and Cygwin 1.5.x.  */
  if (result == -1 && errno == EMFILE)
    errno = EBADF;
#if REPLACE_FCHDIR
  if (fd != desired_fd && result != -1)
    result = _gl_register_dup (fd, result);
#endif
  return result;
}
