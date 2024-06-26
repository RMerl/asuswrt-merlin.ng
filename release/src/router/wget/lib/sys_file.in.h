/* Provide a more complete sys/file.h.

   Copyright (C) 2007-2024 Free Software Foundation, Inc.

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

/* Written by Richard W.M. Jones.  */

#ifndef _@GUARD_PREFIX@_SYS_FILE_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  */
#if @HAVE_SYS_FILE_H@
# @INCLUDE_NEXT@ @NEXT_SYS_FILE_H@
#endif

#ifndef _@GUARD_PREFIX@_SYS_FILE_H
#define _@GUARD_PREFIX@_SYS_FILE_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#ifndef LOCK_SH
/* Operations for the 'flock' call (same as Linux kernel constants).  */
# define LOCK_SH 1       /* Shared lock.  */
# define LOCK_EX 2       /* Exclusive lock.  */
# define LOCK_UN 8       /* Unlock.  */

/* Can be OR'd into one of the above.  */
# define LOCK_NB 4       /* Don't block when locking.  */
#endif

/* The definition of _GL_WARN_ON_USE is copied here.  */

#if @GNULIB_FLOCK@
/* Apply or remove advisory locks on an open file.
   Return 0 if successful, otherwise -1 and errno set.  */
# if !@HAVE_FLOCK@
extern int flock (int fd, int operation);
# endif
#elif defined GNULIB_POSIXCHECK
# undef flock
# if HAVE_RAW_DECL_FLOCK
_GL_WARN_ON_USE (flock, "flock is unportable - "
                 "use gnulib module flock for portability");
# endif
#endif


#endif /* _@GUARD_PREFIX@_SYS_FILE_H */
#endif /* _@GUARD_PREFIX@_SYS_FILE_H */
