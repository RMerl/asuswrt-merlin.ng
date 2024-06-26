/* Provide a non-threads replacement for the POSIX raise function.

   Copyright (C) 2002-2003, 2005-2006, 2009-2024 Free Software Foundation, Inc.

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

/* written by Jim Meyering and Bruno Haible */

#include <config.h>

/* Specification.  */
#include <signal.h>

#if HAVE_RAISE
/* Native Windows platform.  */

# include <errno.h>

# if HAVE_MSVC_INVALID_PARAMETER_HANDLER
#  include "msvc-inval.h"
# endif

# if HAVE_MSVC_INVALID_PARAMETER_HANDLER
/* Forward declaration.  */
static int raise_nothrow (int sig);
# else
#  define raise_nothrow raise
# endif

#else
/* An old Unix platform.  */

# include <unistd.h>

#endif

int
raise (int sig)
#undef raise
{
#if GNULIB_defined_signal_blocking && GNULIB_defined_SIGPIPE
  if (sig == SIGPIPE)
    return _gl_raise_SIGPIPE ();
#endif

#if HAVE_RAISE
  return raise_nothrow (sig);
#else
  return kill (getpid (), sig);
#endif
}

#if HAVE_RAISE && HAVE_MSVC_INVALID_PARAMETER_HANDLER
static int
raise_nothrow (int sig)
{
  int result;

  TRY_MSVC_INVAL
    {
      result = raise (sig);
    }
  CATCH_MSVC_INVAL
    {
      result = -1;
      errno = EINVAL;
    }
  DONE_MSVC_INVAL;

  return result;
}
#endif
