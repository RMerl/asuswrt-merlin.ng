/* -*- buffer-read-only: t -*- vi: set ro: */
/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* Assist in file system timestamp tests.
   Copyright (C) 2009-2011 Free Software Foundation, Inc.

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

/* Written by Eric Blake <ebb9@byu.net>, 2009.  */

#ifndef GLTEST_NAP_H
# define GLTEST_NAP_H

/* Sleep long enough to notice a timestamp difference on the file
   system in the current directory.  Assumes that BASE is defined,
   and requires that the test module depends on usleep.  */
static void
nap (void)
{
  static long delay;
  if (!delay)
    {
      /* Initialize only once, by sleeping for 20 milliseconds (needed
         since xfs has a quantization of about 10 milliseconds, even
         though it has a granularity of 1 nanosecond, and since NTFS
         has a default quantization of 15.25 milliseconds, even though
         it has a granularity of 100 nanoseconds).  If the seconds
         differ, repeat the test one more time (in case we crossed a
         quantization boundary on a file system with 1 second
         resolution).  If we can't observe a difference in only the
         nanoseconds, then fall back to 1 second if the time is odd,
         and 2 seconds (needed for FAT) if time is even.  */
      struct stat st1;
      struct stat st2;
      ASSERT (close (creat (BASE "tmp", 0600)) == 0);
      ASSERT (stat (BASE "tmp", &st1) == 0);
      ASSERT (unlink (BASE "tmp") == 0);
      delay = 20000;
      usleep (delay);
      ASSERT (close (creat (BASE "tmp", 0600)) == 0);
      ASSERT (stat (BASE "tmp", &st2) == 0);
      ASSERT (unlink (BASE "tmp") == 0);
      if (st1.st_mtime != st2.st_mtime)
        {
          /* Seconds differ, give it one more shot.  */
          st1 = st2;
          usleep (delay);
          ASSERT (close (creat (BASE "tmp", 0600)) == 0);
          ASSERT (stat (BASE "tmp", &st2) == 0);
          ASSERT (unlink (BASE "tmp") == 0);
        }
      if (! (st1.st_mtime == st2.st_mtime
             && get_stat_mtime_ns (&st1) < get_stat_mtime_ns (&st2)))
        delay = (st1.st_mtime & 1) ? 1000000 : 2000000;
    }
  usleep (delay);
}

#endif /* GLTEST_NAP_H */
