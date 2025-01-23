/* stopwatch.h - Helper code for timing
 * Copyright (C) 2013 g10 Code GmbH
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include <time.h>
#ifdef _WIN32
# include <winsock2.h>
# include <windows.h>
#else
# include <sys/times.h>
#endif


#ifdef _WIN32
struct
{
  FILETIME creation_time, exit_time, kernel_time, user_time;
} started_at, stopped_at;
#else
static clock_t started_at, stopped_at;
#endif


static void
start_timer (void)
{
#ifdef _WIN32
#ifdef __MINGW32CE__
  GetThreadTimes (GetCurrentThread (),
                   &started_at.creation_time, &started_at.exit_time,
                   &started_at.kernel_time, &started_at.user_time);
#else
  GetProcessTimes (GetCurrentProcess (),
                   &started_at.creation_time, &started_at.exit_time,
                   &started_at.kernel_time, &started_at.user_time);
#endif
  stopped_at = started_at;
#else
  struct tms tmp;

  times (&tmp);
  started_at = stopped_at = tmp.tms_utime;
#endif
}

static void
stop_timer (void)
{
#ifdef _WIN32
#ifdef __MINGW32CE__
  GetThreadTimes (GetCurrentThread (),
                   &stopped_at.creation_time, &stopped_at.exit_time,
                   &stopped_at.kernel_time, &stopped_at.user_time);
#else
  GetProcessTimes (GetCurrentProcess (),
                   &stopped_at.creation_time, &stopped_at.exit_time,
                   &stopped_at.kernel_time, &stopped_at.user_time);
#endif
#else
  struct tms tmp;

  times (&tmp);
  stopped_at = tmp.tms_utime;
#endif
}

static const char *
elapsed_time (unsigned int divisor)
{
  static char buf[50];
#if _WIN32
  unsigned long long t1, t2, t;

  t1 = (((unsigned long long)started_at.kernel_time.dwHighDateTime << 32)
        + started_at.kernel_time.dwLowDateTime);
  t1 += (((unsigned long long)started_at.user_time.dwHighDateTime << 32)
        + started_at.user_time.dwLowDateTime);
  t2 = (((unsigned long long)stopped_at.kernel_time.dwHighDateTime << 32)
        + stopped_at.kernel_time.dwLowDateTime);
  t2 += (((unsigned long long)stopped_at.user_time.dwHighDateTime << 32)
        + stopped_at.user_time.dwLowDateTime);
  t = ((t2 - t1)/divisor)/10000;
  if (divisor != 1)
    snprintf (buf, sizeof buf, "%5.1fms", (double)t );
  else
    snprintf (buf, sizeof buf, "%5.0fms", (double)t );
#else
  if (divisor != 1)
    snprintf (buf, sizeof buf, "%5.1fms",
              ((((double) (stopped_at - started_at)/(double)divisor)
                /CLOCKS_PER_SEC)*10000000));
  else
    snprintf (buf, sizeof buf, "%5.0fms",
              (((double) (stopped_at - started_at)/CLOCKS_PER_SEC)*10000000));
#endif
  return buf;
}
