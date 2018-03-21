#include <tommath_private.h>
#ifdef BN_MP_N_ROOT_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library was designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tstdenis82@gmail.com, http://libtom.org
 */

/* wrapper function for mp_n_root_ex()
 * computes c = (a)**(1/b) such that (c)**b <= a and (c+1)**b > a
 */
int mp_n_root (mp_int * a, mp_digit b, mp_int * c)
{
  return mp_n_root_ex(a, b, c, 0);
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
