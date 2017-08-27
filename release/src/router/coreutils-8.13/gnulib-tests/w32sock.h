/* -*- buffer-read-only: t -*- vi: set ro: */
/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* w32sock.h --- internal auxilliary functions for Windows socket functions

   Copyright (C) 2008-2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Paolo Bonzini */

#include <errno.h>

/* Get O_RDWR and O_BINARY.  */
#include <fcntl.h>

/* Get _get_osfhandle() and _open_osfhandle().  */
#include <io.h>

#define FD_TO_SOCKET(fd)   ((SOCKET) _get_osfhandle ((fd)))
#define SOCKET_TO_FD(fh)   (_open_osfhandle ((long) (fh), O_RDWR | O_BINARY))

static inline void
set_winsock_errno (void)
{
  int err = WSAGetLastError ();

  /* Map some WSAE* errors to the runtime library's error codes.  */
  switch (err)
    {
    case WSA_INVALID_HANDLE:
      errno = EBADF;
      break;
    case WSA_NOT_ENOUGH_MEMORY:
      errno = ENOMEM;
      break;
    case WSA_INVALID_PARAMETER:
      errno = EINVAL;
      break;
    case WSAEWOULDBLOCK:
      errno = EWOULDBLOCK;
      break;
    case WSAENAMETOOLONG:
      errno = ENAMETOOLONG;
      break;
    case WSAENOTEMPTY:
      errno = ENOTEMPTY;
      break;
    default:
      errno = (err > 10000 && err < 10025) ? err - 10000 : err;
      break;
    }
}
