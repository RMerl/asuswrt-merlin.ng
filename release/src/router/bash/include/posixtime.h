/* posixtime.h -- wrapper for time.h, sys/times.h mess. */

/* Copyright (C) 1999 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _POSIXTIME_H_
#define _POSIXTIME_H_

/* include this after config.h */
/* Some systems require this, mostly for the definition of `struct timezone'.
   For example, Dynix/ptx has that definition in <time.h> rather than
      sys/time.h */
#if defined (TIME_WITH_SYS_TIME)
#  include <sys/time.h>
#  include <time.h>
#else
#  if defined (HAVE_SYS_TIME_H)
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

#if !defined (HAVE_SYSCONF) || !defined (_SC_CLK_TCK)
#  if !defined (CLK_TCK)
#    if defined (HZ)
#      define CLK_TCK	HZ
#    else
#      define CLK_TCK	60			/* 60HZ */
#    endif
#  endif /* !CLK_TCK */
#endif /* !HAVE_SYSCONF && !_SC_CLK_TCK */

#endif /* _POSIXTIME_H_ */
