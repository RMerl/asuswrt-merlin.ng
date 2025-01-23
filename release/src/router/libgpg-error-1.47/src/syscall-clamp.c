/* syscall-clamp.c - Syscall clamp related stuff
 * Copyright (C) 2016, 2017 g10 Code GmbH
 *
 * This file is part of Libgpg-error.
 *
 * Libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <https://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <config.h>

#include "gpgrt-int.h"

/*
 * Functions called before and after blocking syscalls.
 * gpgrt_set_syscall_clamp is used to set them.
 */
static void (*pre_syscall_func)(void);
static void (*post_syscall_func)(void);


/*
 * Register the syscall clamp.  These two functions are called
 * immediately before and after a possible blocking system call.  This
 * should be used before any I/O happens.  The function is commonly
 * used with the nPth library:
 *
 *    gpgrt_set_syscall_clamp (npth_unprotect, npth_protect);
 *
 * These functions may not modify ERRNO.
 *
 * Setting the clamp is not thread-safe and should thus be done as
 * early as possible.
 */
void
_gpgrt_set_syscall_clamp (void (*pre)(void), void (*post)(void))
{
  pre_syscall_func = pre;
  post_syscall_func = post;
}

/*
 * Return the current sycall clamp functions.  This can be used by
 * other libraries which have blocking functions.
 */
void
_gpgrt_get_syscall_clamp (void (**r_pre)(void), void (**r_post)(void))
{
  *r_pre  = pre_syscall_func;
  *r_post = post_syscall_func;
}


/* Call this function before a blocking system or libc call.  */
void
_gpgrt_pre_syscall (void)
{
  if (pre_syscall_func)
    pre_syscall_func ();
}


/* Call this function after a blocking system or libc call.  */
void
_gpgrt_post_syscall (void)
{
  if (post_syscall_func)
    post_syscall_func ();
}
