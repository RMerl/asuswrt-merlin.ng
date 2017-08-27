/* timespec -- System time interface

   Copyright (C) 2000, 2002, 2004-2005, 2007, 2009-2011 Free Software
   Foundation, Inc.

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

#if ! defined TIMESPEC_H
# define TIMESPEC_H

# include <time.h>

/* Return negative, zero, positive if A < B, A == B, A > B, respectively.

   For each time stamp T, this code assumes that either:

     * T.tv_nsec is in the range 0..999999999; or
     * T.tv_sec corresponds to a valid leap second on a host that supports
       leap seconds, and T.tv_nsec is in the range 1000000000..1999999999; or
     * T.tv_sec is the minimum time_t value and T.tv_nsec is -1; or
       T.tv_sec is the maximum time_t value and T.tv_nsec is 2000000000.
       This allows for special struct timespec values that are less or
       greater than all possible valid time stamps.

   In all these cases, it is safe to subtract two tv_nsec values and
   convert the result to integer without worrying about overflow on
   any platform of interest to the GNU project, since all such
   platforms have 32-bit int or wider.

   Replacing "(int) (a.tv_nsec - b.tv_nsec)" with something like
   "a.tv_nsec < b.tv_nsec ? -1 : a.tv_nsec > b.tv_nsec" would cause
   this function to work in some cases where the above assumption is
   violated, but not in all cases (e.g., a.tv_sec==1, a.tv_nsec==-2,
   b.tv_sec==0, b.tv_nsec==999999999) and is arguably not worth the
   extra instructions.  Using a subtraction has the advantage of
   detecting some invalid cases on platforms that detect integer
   overflow.

   The (int) cast avoids a gcc -Wconversion warning.  */

static inline int
timespec_cmp (struct timespec a, struct timespec b)
{
  return (a.tv_sec < b.tv_sec ? -1
          : a.tv_sec > b.tv_sec ? 1
          : (int) (a.tv_nsec - b.tv_nsec));
}

/* Return -1, 0, 1, depending on the sign of A.  A.tv_nsec must be
   nonnegative.  */
static inline int
timespec_sign (struct timespec a)
{
  return a.tv_sec < 0 ? -1 : a.tv_sec || a.tv_nsec;
}

struct timespec timespec_add (struct timespec, struct timespec);
struct timespec timespec_sub (struct timespec, struct timespec);
struct timespec dtotimespec (double);

/* Return an approximation to A, of type 'double'.  */
static inline double
timespectod (struct timespec a)
{
  return a.tv_sec + a.tv_nsec / 1e9;
}

void gettime (struct timespec *);
int settime (struct timespec const *);

#endif
