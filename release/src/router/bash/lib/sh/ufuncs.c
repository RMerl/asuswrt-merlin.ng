/* ufuncs - sleep and alarm functions that understand fractional values */

/* Copyright (C) 2008,2009 Free Software Foundation, Inc.

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

#include "config.h"

#include "bashtypes.h"

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

#if defined (HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined (HAVE_SELECT)
#  include "posixselect.h"
#endif

/* A version of `alarm' using setitimer if it's available. */

#if defined (HAVE_SETITIMER)
unsigned int
falarm(secs, usecs)
     unsigned int secs, usecs;
{
  struct itimerval it, oit;

  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;

  it.it_value.tv_sec = secs;
  it.it_value.tv_usec = usecs;

  if (setitimer(ITIMER_REAL, &it, &oit) < 0)
    return (-1);		/* XXX will be converted to unsigned */

  /* Backwards compatibility with alarm(3) */
  if (oit.it_value.tv_usec)
    oit.it_value.tv_sec++;
  return (oit.it_value.tv_sec);
}
#else
int
falarm (secs, usecs)
     unsigned int secs, usecs;
{
  if (secs == 0 && usecs == 0)
    return (alarm (0));

  if (secs == 0 || usecs >= 500000)
    {
      secs++;
      usecs = 0;
    }
  return (alarm (secs));
}
#endif /* !HAVE_SETITIMER */

/* A version of sleep using fractional seconds and select.  I'd like to use
   `usleep', but it's already taken */

#if defined (HAVE_TIMEVAL) && defined (HAVE_SELECT)
int
fsleep(sec, usec)
     unsigned int sec, usec;
{
  struct timeval tv;

  tv.tv_sec = sec;
  tv.tv_usec = usec;

  return select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv);
}
#else /* !HAVE_TIMEVAL || !HAVE_SELECT */
int
fsleep(sec, usec)
     long sec, usec;
{
  if (usec >= 500000)	/* round */
   sec++;
  return (sleep(sec));
}
#endif /* !HAVE_TIMEVAL || !HAVE_SELECT */
