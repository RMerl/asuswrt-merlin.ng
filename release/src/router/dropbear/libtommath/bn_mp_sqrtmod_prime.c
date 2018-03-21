#include <tommath_private.h>
#ifdef BN_MP_SQRTMOD_PRIME_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* Tonelli-Shanks algorithm
 * https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm
 * https://gmplib.org/list-archives/gmp-discuss/2013-April/005300.html
 *
 */

int mp_sqrtmod_prime(mp_int *n, mp_int *prime, mp_int *ret)
{
  int res, legendre;
  mp_int t1, C, Q, S, Z, M, T, R, two;
  mp_digit i;

  /* first handle the simple cases */
  if (mp_cmp_d(n, 0) == MP_EQ) {
    mp_zero(ret);
    return MP_OKAY;
  }
  if (mp_cmp_d(prime, 2) == MP_EQ)                              return MP_VAL; /* prime must be odd */
  if ((res = mp_jacobi(n, prime, &legendre)) != MP_OKAY)        return res;
  if (legendre == -1)                                           return MP_VAL; /* quadratic non-residue mod prime */

  if ((res = mp_init_multi(&t1, &C, &Q, &S, &Z, &M, &T, &R, &two, NULL)) != MP_OKAY) {
	return res;
  }

  /* SPECIAL CASE: if prime mod 4 == 3
   * compute directly: res = n^(prime+1)/4 mod prime
   * Handbook of Applied Cryptography algorithm 3.36
   */
  if ((res = mp_mod_d(prime, 4, &i)) != MP_OKAY)                goto cleanup;
  if (i == 3) {
    if ((res = mp_add_d(prime, 1, &t1)) != MP_OKAY)             goto cleanup;
    if ((res = mp_div_2(&t1, &t1)) != MP_OKAY)                  goto cleanup;
    if ((res = mp_div_2(&t1, &t1)) != MP_OKAY)                  goto cleanup;
    if ((res = mp_exptmod(n, &t1, prime, ret)) != MP_OKAY)      goto cleanup;
    res = MP_OKAY;
    goto cleanup;
  }

  /* NOW: Tonelli-Shanks algorithm */

  /* factor out powers of 2 from prime-1, defining Q and S as: prime-1 = Q*2^S */
  if ((res = mp_copy(prime, &Q)) != MP_OKAY)                    goto cleanup;
  if ((res = mp_sub_d(&Q, 1, &Q)) != MP_OKAY)                   goto cleanup;
  /* Q = prime - 1 */
  mp_zero(&S);
  /* S = 0 */
  while (mp_iseven(&Q) != MP_NO) {
    if ((res = mp_div_2(&Q, &Q)) != MP_OKAY)                    goto cleanup;
    /* Q = Q / 2 */
    if ((res = mp_add_d(&S, 1, &S)) != MP_OKAY)                 goto cleanup;
    /* S = S + 1 */
  }

  /* find a Z such that the Legendre symbol (Z|prime) == -1 */
  if ((res = mp_set_int(&Z, 2)) != MP_OKAY)                     goto cleanup;
  /* Z = 2 */
  while(1) {
    if ((res = mp_jacobi(&Z, prime, &legendre)) != MP_OKAY)     goto cleanup;
    if (legendre == -1) break;
    if ((res = mp_add_d(&Z, 1, &Z)) != MP_OKAY)                 goto cleanup;
    /* Z = Z + 1 */
  }

  if ((res = mp_exptmod(&Z, &Q, prime, &C)) != MP_OKAY)         goto cleanup;
  /* C = Z ^ Q mod prime */
  if ((res = mp_add_d(&Q, 1, &t1)) != MP_OKAY)                  goto cleanup;
  if ((res = mp_div_2(&t1, &t1)) != MP_OKAY)                    goto cleanup;
  /* t1 = (Q + 1) / 2 */
  if ((res = mp_exptmod(n, &t1, prime, &R)) != MP_OKAY)         goto cleanup;
  /* R = n ^ ((Q + 1) / 2) mod prime */
  if ((res = mp_exptmod(n, &Q, prime, &T)) != MP_OKAY)          goto cleanup;
  /* T = n ^ Q mod prime */
  if ((res = mp_copy(&S, &M)) != MP_OKAY)                       goto cleanup;
  /* M = S */
  if ((res = mp_set_int(&two, 2)) != MP_OKAY)                   goto cleanup;

  res = MP_VAL;
  while (1) {
    if ((res = mp_copy(&T, &t1)) != MP_OKAY)                    goto cleanup;
    i = 0;
    while (1) {
      if (mp_cmp_d(&t1, 1) == MP_EQ) break;
      if ((res = mp_exptmod(&t1, &two, prime, &t1)) != MP_OKAY) goto cleanup;
      i++;
    }
    if (i == 0) {
      if ((res = mp_copy(&R, ret)) != MP_OKAY)                  goto cleanup;
      res = MP_OKAY;
      goto cleanup;
    }
    if ((res = mp_sub_d(&M, i, &t1)) != MP_OKAY)                goto cleanup;
    if ((res = mp_sub_d(&t1, 1, &t1)) != MP_OKAY)               goto cleanup;
    if ((res = mp_exptmod(&two, &t1, prime, &t1)) != MP_OKAY)   goto cleanup;
    /* t1 = 2 ^ (M - i - 1) */
    if ((res = mp_exptmod(&C, &t1, prime, &t1)) != MP_OKAY)     goto cleanup;
    /* t1 = C ^ (2 ^ (M - i - 1)) mod prime */
    if ((res = mp_sqrmod(&t1, prime, &C)) != MP_OKAY)           goto cleanup;
    /* C = (t1 * t1) mod prime */
    if ((res = mp_mulmod(&R, &t1, prime, &R)) != MP_OKAY)       goto cleanup;
    /* R = (R * t1) mod prime */
    if ((res = mp_mulmod(&T, &C, prime, &T)) != MP_OKAY)        goto cleanup;
    /* T = (T * C) mod prime */
    mp_set(&M, i);
    /* M = i */
  }

cleanup:
  mp_clear_multi(&t1, &C, &Q, &S, &Z, &M, &T, &R, &two, NULL);
  return res;
}

#endif
