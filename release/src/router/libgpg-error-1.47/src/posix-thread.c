/* posix-thread.c - GPGRT thread functions for POSIX systems
   Copyright (C) 2014 g10 Code GmbH

   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_W32_SYSTEM
# error This module may not be build for Windows.
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>  /* Get posix option macros.  */

#if USE_POSIX_THREADS
# ifdef _POSIX_PRIORITY_SCHEDULING
#  include <sched.h>
# endif
#elif USE_SOLARIS_THREADS
# include <thread.h>
#endif

#include "gpgrt-int.h"

#include "thread.h"



gpg_err_code_t
_gpgrt_yield (void)
{
#if USE_POSIX_THREADS
# ifdef _POSIX_PRIORITY_SCHEDULING
  _gpgrt_pre_syscall ();
  sched_yield ();
  _gpgrt_post_syscall ();
# else
  return GPG_ERR_NOT_SUPPORTED;
# endif
#elif USE_SOLARIS_THREADS
  _gpgrt_pre_syscall ();
  thr_yield ();
  _gpgrt_post_syscall ();
#else
  return GPG_ERR_NOT_SUPPORTED;
#endif

  return 0;
}
