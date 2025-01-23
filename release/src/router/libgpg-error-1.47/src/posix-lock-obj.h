/* posic-lock-obj.h - Declaration of the POSIX lock object
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

#ifndef POSIX_LOCK_OBJ_H
#define POSIX_LOCK_OBJ_H

#define LOCK_ABI_NOT_AVAILABLE (-1)
#if USE_POSIX_THREADS
# define LOCK_ABI_VERSION 1
#else
# define LOCK_ABI_VERSION LOCK_ABI_NOT_AVAILABLE
#endif

typedef struct
{
  long vers;
#if USE_POSIX_THREADS
  union {
    pthread_mutex_t mtx;
    long *dummy;
  } u;
#endif
} _gpgrt_lock_t;


#endif /*POSIX_LOCK_OBJ_H*/
