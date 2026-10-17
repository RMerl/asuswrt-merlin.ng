// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 1992, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
 */

typedef struct {
	long    quot;
	long    rem;
} ldiv_t;
/* Return the `ldiv_t' representation of NUMER over DENOM.  */
ldiv_t
ldiv (long int numer, long int denom)
{
  ldiv_t result;

  result.quot = numer / denom;
  result.rem = numer % denom;

  /* The ANSI standard says that |QUOT| <= |NUMER / DENOM|, where
     NUMER / DENOM is to be computed in infinite precision.  In
     other words, we should always truncate the quotient towards
     zero, never -infinity.  Machine division and remainer may
     work either way when one or both of NUMER or DENOM is
     negative.  If only one is negative and QUOT has been
     truncated towards -infinity, REM will have the same sign as
     DENOM and the opposite sign of NUMER; if both are negative
     and QUOT has been truncated towards -infinity, REM will be
     positive (will have the opposite sign of NUMER).  These are
     considered `wrong'.  If both are NUM and DENOM are positive,
     RESULT will always be positive.  This all boils down to: if
     NUMER >= 0, but REM < 0, we got the wrong answer.  In that
     case, to get the right answer, add 1 to QUOT and subtract
     DENOM from REM.  */

  if (numer >= 0 && result.rem < 0)
    {
      ++result.quot;
      result.rem -= denom;
    }

  return result;
}
