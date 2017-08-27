/* -*- buffer-read-only: t -*- vi: set ro: */
/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* Macros for not-a-number.
   Copyright (C) 2007-2011 Free Software Foundation, Inc.

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


/* NaNf () returns a 'float' not-a-number.  */

/* The Compaq (ex-DEC) C 6.4 compiler chokes on the expression 0.0 / 0.0.  */
#ifdef __DECC
static float
NaNf ()
{
  static float zero = 0.0f;
  return zero / zero;
}
#else
# define NaNf() (0.0f / 0.0f)
#endif


/* NaNd () returns a 'double' not-a-number.  */

/* The Compaq (ex-DEC) C 6.4 compiler chokes on the expression 0.0 / 0.0.  */
#ifdef __DECC
static double
NaNd ()
{
  static double zero = 0.0;
  return zero / zero;
}
#else
# define NaNd() (0.0 / 0.0)
#endif


/* NaNl () returns a 'long double' not-a-number.  */

/* On Irix 6.5, gcc 3.4.3 can't compute compile-time NaN, and needs the
   runtime type conversion.  */
#ifdef __sgi
static long double NaNl ()
{
  double zero = 0.0;
  return zero / zero;
}
#else
# define NaNl() (0.0L / 0.0L)
#endif
