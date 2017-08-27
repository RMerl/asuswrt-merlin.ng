/* GIO - GLib Input, Output and Streaming Library
 * 
 * Copyright (C) 2006-2007 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 */

#include "config.h"
#include <errno.h>
#include "gioerror.h"


/**
 * SECTION:gioerror
 * @short_description: Error helper functions
 * @include: gio/gio.h
 *
 * Contains helper functions for reporting errors to the user.
 **/

/**
 * g_io_error_quark:
 *
 * Gets the GIO Error Quark.
 *
 * Return value: a #GQuark.
 **/
G_DEFINE_QUARK (g-io-error-quark, g_io_error)

/**
 * g_io_error_from_errno:
 * @err_no: Error number as defined in errno.h.
 *
 * Converts errno.h error codes into GIO error codes.
 *
 * Returns: #GIOErrorEnum value for the given errno.h error number.
 **/
GIOErrorEnum
g_io_error_from_errno (gint err_no)
{
  switch (err_no)
    {
#ifdef EEXIST
    case EEXIST:
      return G_IO_ERROR_EXISTS;
      break;
#endif

#ifdef EISDIR
    case EISDIR:
      return G_IO_ERROR_IS_DIRECTORY;
      break;
#endif

#ifdef EACCES
    case EACCES:
      return G_IO_ERROR_PERMISSION_DENIED;
      break;
#endif

#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      return G_IO_ERROR_FILENAME_TOO_LONG;
      break;
#endif

#ifdef ENOENT
    case ENOENT:
      return G_IO_ERROR_NOT_FOUND;
      break;
#endif

#ifdef ENOTDIR
    case ENOTDIR:
      return G_IO_ERROR_NOT_DIRECTORY;
      break;
#endif

#ifdef EROFS
    case EROFS:
      return G_IO_ERROR_READ_ONLY;
      break;
#endif

#ifdef ELOOP
    case ELOOP:
      return G_IO_ERROR_TOO_MANY_LINKS;
      break;
#endif

#ifdef ENOSPC
    case ENOSPC:
      return G_IO_ERROR_NO_SPACE;
      break;
#endif

#ifdef ENOMEM
    case ENOMEM:
      return G_IO_ERROR_NO_SPACE;
      break;
#endif
      
#ifdef EINVAL
    case EINVAL:
      return G_IO_ERROR_INVALID_ARGUMENT;
      break;
#endif

#ifdef EPERM
    case EPERM:
      return G_IO_ERROR_PERMISSION_DENIED;
      break;
#endif

#ifdef ECANCELED
    case ECANCELED:
      return G_IO_ERROR_CANCELLED;
      break;
#endif

#if defined(ENOTEMPTY) && (!defined (EEXIST) || (ENOTEMPTY != EEXIST))
    case ENOTEMPTY:
      return G_IO_ERROR_NOT_EMPTY;
      break;
#endif

#ifdef ENOTSUP
    case ENOTSUP:
      return G_IO_ERROR_NOT_SUPPORTED;
      break;
#endif

#ifdef ETIMEDOUT
    case ETIMEDOUT:
      return G_IO_ERROR_TIMED_OUT;
      break;
#endif

#ifdef EBUSY
    case EBUSY:
      return G_IO_ERROR_BUSY;
      break;
#endif

/* some magic to deal with EWOULDBLOCK and EAGAIN.
 * apparently on HP-UX these are actually defined to different values,
 * but on Linux, for example, they are the same.
 */
#if defined(EWOULDBLOCK) && defined(EAGAIN) && EWOULDBLOCK == EAGAIN
    /* we have both and they are the same: only emit one case. */
    case EAGAIN:
      return G_IO_ERROR_WOULD_BLOCK;
      break;
#else
    /* else: consider each of them separately.  this handles both the
     * case of having only one and the case where they are different values.
     */
# ifdef EAGAIN
    case EAGAIN:
      return G_IO_ERROR_WOULD_BLOCK;
      break;
# endif

# ifdef EWOULDBLOCK
    case EWOULDBLOCK:
      return G_IO_ERROR_WOULD_BLOCK;
      break;
# endif
#endif

#ifdef EMFILE
    case EMFILE:
      return G_IO_ERROR_TOO_MANY_OPEN_FILES;
      break;
#endif

#ifdef EADDRINUSE
    case EADDRINUSE:
      return G_IO_ERROR_ADDRESS_IN_USE;
      break;
#endif

#ifdef EHOSTUNREACH
    case EHOSTUNREACH:
      return G_IO_ERROR_HOST_UNREACHABLE;
      break;
#endif

#ifdef ENETUNREACH
    case ENETUNREACH:
      return G_IO_ERROR_NETWORK_UNREACHABLE;
      break;
#endif

#ifdef ECONNREFUSED
    case ECONNREFUSED:
      return G_IO_ERROR_CONNECTION_REFUSED;
      break;
#endif

#ifdef EPIPE
    case EPIPE:
      return G_IO_ERROR_BROKEN_PIPE;
      break;
#endif

    default:
      return G_IO_ERROR_FAILED;
      break;
    }
}

#ifdef G_OS_WIN32

/**
 * g_io_error_from_win32_error:
 * @error_code: Windows error number.
 *
 * Converts some common error codes into GIO error codes. The
 * fallback value G_IO_ERROR_FAILED is returned for error codes not
 * handled.
 *
 * Returns: #GIOErrorEnum value for the given error number.
 *
 * Since: 2.26
 **/
GIOErrorEnum
g_io_error_from_win32_error (gint error_code)
{
  switch (error_code)
    {
    default:
      return G_IO_ERROR_FAILED;
      break;
    }
}

#endif
