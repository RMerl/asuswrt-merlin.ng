/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file prob_distr.c
 *
 * \brief
 *  Implements various probability distributions.
 *  Almost all code is courtesy of Riastradh.
 *
 * \details
 * Here are some details that might help you understand this file:
 *
 * - Throughout this file, `eps' means the largest relative error of a
 *   correctly rounded floating-point operation, which in binary64
 *   floating-point arithmetic is 2^-53.  Here the relative error of a
 *   true value x from a computed value y is |x - y|/|x|.  This
 *   definition of epsilon is conventional for numerical analysts when
 *   writing error analyses.  (If your libm doesn't provide correctly
 *   rounded exp and log, their relative error is usually below 2*2^-53
 *   and probably closer to 1.1*2^-53 instead.)
 *
 *   The C constant DBL_EPSILON is actually twice this, and should
 *   perhaps rather be named ulp(1) -- that is, it is the distance from
 *   1 to the next greater floating-point number, which is usually of
 *   more interest to programmers and hardware engineers.
 *
 *   Since this file is concerned mainly with error bounds rather than
 *   with low-level bit-hacking of floating-point numbers, we adopt the
 *   numerical analysts' definition in the comments, though we do use
 *   DBL_EPSILON in a handful of places where it is convenient to use
 *   some function of eps = DBL_EPSILON/2 in a case analysis.
 *
 * - In various functions (e.g. sample_log_logistic()) we jump through hoops so
 *   that we can use reals closer to 0 than closer to 1, since we achieve much
 *   greater accuracy for floating point numbers near 0. In particular, we can
 *   represent differences as small as 10^-300 for numbers near 0, but of no
 *   less than 10^-16 for numbers near 1.
 **/

#define PROB_DISTR_PRIVATE

#include "orconfig.h"

#include "lib/math/prob_distr.h"

#include "lib/crypt_ops/crypto_rand.h"
#include "lib/cc/ctassert.h"
#include "lib/log/util_bug.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

#ifndef COCCI
/** Declare a function that downcasts from a generic dist struct to the actual
 *  subtype probability distribution it represents. */
#define DECLARE_PROB_DISTR_DOWNCAST_FN(name) \
  static inline                           \
  const struct name##_t *                             \
  dist_to_const_##name(const struct dist_t *obj) {    \
    tor_assert(obj->ops == &name##_ops);            \
    return SUBTYPE_P(obj, struct name ## _t, base);   \
  }
DECLARE_PROB_DISTR_DOWNCAST_FN(uniform)
DECLARE_PROB_DISTR_DOWNCAST_FN(geometric)
DECLARE_PROB_DISTR_DOWNCAST_FN(logistic)
DECLARE_PROB_DISTR_DOWNCAST_FN(log_logistic)
DECLARE_PROB_DISTR_DOWNCAST_FN(genpareto)
DECLARE_PROB_DISTR_DOWNCAST_FN(weibull)
#endif /* !defined(COCCI) */

/**
 * Count number of one bits in 32-bit word.
 */
static unsigned
bitcount32(uint32_t x)
{

  /* Count two-bit groups.  */
  x -= (x >> 1) & UINT32_C(0x55555555);

  /* Count four-bit groups.  */
  x = ((x >> 2) & UINT32_C(0x33333333)) + (x & UINT32_C(0x33333333));

  /* Count eight-bit groups.  */
  x = (x + (x >> 4)) & UINT32_C(0x0f0f0f0f);

  /* Sum all eight-bit groups, and extract the sum.  */
  return (x * UINT32_C(0x01010101)) >> 24;
}

/**
 * Count leading zeros in 32-bit word.
 */
static unsigned
clz32(uint32_t x)
{

  /* Round up to a power of two.  */
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;

  /* Subtract count of one bits from 32.  */
  return (32 - bitcount32(x));
}

/*
 * Some lemmas that will be used throughout this file to prove various error
 * bounds:
 *
 * Lemma 1.  If |d| <= 1/2, then 1/(1 + d) <= 2.
 *
 * Proof.  If 0 <= d <= 1/2, then 1 + d >= 1, so that 1/(1 + d) <= 1.
 * If -1/2 <= d <= 0, then 1 + d >= 1/2, so that 1/(1 + d) <= 2.  QED.
 *
 * Lemma 2. If b = a*(1 + d)/(1 + d') for |d'| < 1/2 and nonzero a, b,
 * then b = a*(1 + e) for |e| <= 2|d' - d|.
 *
 * Proof.  |a - b|/|a|
 *             = |a - a*(1 + d)/(1 + d')|/|a|
 *             = |1 - (1 + d)/(1 + d')|
 *             = |(1 + d' - 1 - d)/(1 + d')|
 *             = |(d' - d)/(1 + d')|
 *            <= 2|d' - d|, by Lemma 1,
 *
 * QED.
 *
 * Lemma 3.  For |d|, |d'| < 1/4,
 *
 *     |log((1 + d)/(1 + d'))| <= 4|d - d'|.
 *
 * Proof.  Write
 *
 *     log((1 + d)/(1 + d'))
 *      = log(1 + (1 + d)/(1 + d') - 1)
 *      = log(1 + (1 + d - 1 - d')/(1 + d')
 *      = log(1 + (d - d')/(1 + d')).
 *
 * By Lemma 1, |(d - d')/(1 + d')| < 2|d' - d| < 1, so the Taylor
 * series of log(1 + x) converges absolutely for (d - d')/(1 + d'),
 * and thus we have
 *
 *     |log(1 + (d - d')/(1 + d'))|
 *      = |\sum_{n=1}^\infty ((d - d')/(1 + d'))^n/n|
 *     <= \sum_{n=1}^\infty |(d - d')/(1 + d')|^n/n
 *     <= \sum_{n=1}^\infty |2(d' - d)|^n/n
 *     <= \sum_{n=1}^\infty |2(d' - d)|^n
 *      = 1/(1 - |2(d' - d)|)
 *     <= 4|d' - d|,
 *
 * QED.
 *
 * Lemma 4.  If 1/e <= 1 + x <= e, then
 *
 *     log(1 + (1 + d) x) = (1 + d') log(1 + x)
 *
 * for |d'| < 8|d|.
 *
 * Proof.  Write
 *
 *     log(1 + (1 + d) x)
 *     = log(1 + x + x*d)
 *     = log((1 + x) (1 + x + x*d)/(1 + x))
 *     = log(1 + x) + log((1 + x + x*d)/(1 + x))
 *     = log(1 + x) (1 + log((1 + x + x*d)/(1 + x))/log(1 + x)).
 *
 * The relative error is bounded by
 *
 *     |log((1 + x + x*d)/(1 + x))/log(1 + x)|
 *     <= 4|x + x*d - x|/|log(1 + x)|, by Lemma 3,
 *      = 4|x*d|/|log(1 + x)|
 *      < 8|d|,
 *
 * since in this range 0 < 1 - 1/e < x/log(1 + x) <= e - 1 < 2.  QED.
 */

/**
 * Compute the logistic function: f(x) = 1/(1 + e^{-x}) = e^x/(1 + e^x).
 * Maps a log-odds-space probability in [-infinity, +infinity] into a
 * direct-space probability in [0,1].  Inverse of logit.
 *
 * Ill-conditioned for large x; the identity logistic(-x) = 1 -
 * logistic(x) and the function logistichalf(x) = logistic(x) - 1/2 may
 * help to rearrange a computation.
 *
 * This implementation gives relative error bounded by 7 eps.
 */
STATIC double
logistic(double x)
{
  if (x <= log(DBL_EPSILON/2)) {
    /*
     * If x <= log(DBL_EPSILON/2) = log(eps), then e^x <= eps. In this case
     * we will approximate the logistic() function with e^x because the
     * relative error is less than eps. Here is a calculation of the
     * relative error between the logistic() function and e^x and a proof
     * that it's less than eps:
     *
     *     |e^x - e^x/(1 + e^x)|/|e^x/(1 + e^x)|
     *     <= |1 - 1/(1 + e^x)|*|1 + e^x|
     *      = |e^x/(1 + e^x)|*|1 + e^x|
     *      = |e^x|
     *     <= eps.
     */
    return exp(x); /* return e^x */
  } else if (x <= -log(DBL_EPSILON/2)) {
    /*
     * e^{-x} > 0, so 1 + e^{-x} > 1, and 0 < 1/(1 +
     * e^{-x}) < 1; further, since e^{-x} < 1 + e^{-x}, we
     * also have 0 < 1/(1 + e^{-x}) < 1.  Thus, if exp has
     * relative error d0, + has relative error d1, and /
     * has relative error d2, then we get
     *
     *     (1 + d2)/[(1 + (1 + d0) e^{-x})(1 + d1)]
     *     = (1 + d0)/[1 + e^{-x} + d0 e^{-x}
     *                     + d1 + d1 e^{-x} + d0 d1 e^{-x}]
     *     = (1 + d0)/[(1 + e^{-x})
     *                 * (1 + d0 e^{-x}/(1 + e^{-x})
     *                      + d1/(1 + e^{-x})
     *                      + d0 d1 e^{-x}/(1 + e^{-x}))].
     *     = (1 + d0)/[(1 + e^{-x})(1 + d')]
     *     = [1/(1 + e^{-x})] (1 + d0)/(1 + d')
     *
     * where
     *
     *     d' = d0 e^{-x}/(1 + e^{-x})
     *          + d1/(1 + e^{-x})
     *          + d0 d1 e^{-x}/(1 + e^{-x}).
     *
     * By Lemma 2 this relative error is bounded by
     *
     *     2|d0 - d'|
     *      = 2|d0 - d0 e^{-x}/(1 + e^{-x})
     *             - d1/(1 + e^{-x})
     *             - d0 d1 e^{-x}/(1 + e^{-x})|
     *     <= 2|d0| + 2|d0 e^{-x}/(1 + e^{-x})|
     *             + 2|d1/(1 + e^{-x})|
     *             + 2|d0 d1 e^{-x}/(1 + e^{-x})|
     *     <= 2|d0| + 2|d0| + 2|d1| + 2|d0 d1|
     *     <= 4|d0| + 2|d1| + 2|d0 d1|
     *     <= 6 eps + 2 eps^2.
     */
    return 1/(1 + exp(-x));
  } else {
    /*
     * e^{-x} <= eps, so the relative error of 1 from 1/(1
     * + e^{-x}) is
     *
     *     |1/(1 + e^{-x}) - 1|/|1/(1 + e^{-x})|
     *      = |e^{-x}/(1 + e^{-x})|/|1/(1 + e^{-x})|
     *      = |e^{-x}|
     *     <= eps.
     *
     * This computation avoids an intermediate overflow
     * exception, although the effect on the result is
     * harmless.
     *
     * XXX Should maybe raise inexact here.
     */
    return 1;
  }
}

/**
 * Compute the logit function: log p/(1 - p).  Defined on [0,1].  Maps
 * a direct-space probability in [0,1] to a log-odds-space probability
 * in [-infinity, +infinity].  Inverse of logistic.
 *
 * Ill-conditioned near 1/2 and 1; the identity logit(1 - p) =
 * -logit(p) and the function logithalf(p0) = logit(1/2 + p0) may help
 * to rearrange a computation for p in [1/(1 + e), 1 - 1/(1 + e)].
 *
 * This implementation gives relative error bounded by 10 eps.
 */
STATIC double
logit(double p)
{

  /* logistic(-1) <= p <= logistic(+1) */
  if (1/(1 + exp(1)) <= p && p <= 1/(1 + exp(-1))) {
    /*
     * For inputs near 1/2, we want to compute log1p(near
     * 0) rather than log(near 1), so write this as:
     *
     * log(p/(1 - p)) = -log((1 - p)/p)
     * = -log(1 + (1 - p)/p - 1)
     * = -log(1 + (1 - p - p)/p)
     * = -log(1 + (1 - 2p)/p).
     *
     * Since p = 2p/2 <= 1 <= 2*2p = 4p, the floating-point
     * evaluation of 1 - 2p is exact; the only error arises
     * from division and log1p.  First, note that if
     * logistic(-1) <= p <= logistic(+1), (1 - 2p)/p lies
     * in the bounds of Lemma 4.
     *
     * If division has relative error d0 and log1p has
     * relative error d1, the outcome is
     *
     *     -(1 + d1) log(1 + (1 - 2p) (1 + d0)/p)
     *     = -(1 + d1) (1 + d') log(1 + (1 - 2p)/p)
     *     = -(1 + d1 + d' + d1 d') log(1 + (1 - 2p)/p).
     *
     * where |d'| < 8|d0| by Lemma 4.  The relative error
     * is then bounded by
     *
     *     |d1 + d' + d1 d'|
     *     <= |d1| + 8|d0| + 8|d1 d0|
     *     <= 9 eps + 8 eps^2.
     */
    return -log1p((1 - 2*p)/p);
  } else {
    /*
     * For inputs near 0, although 1 - p may be rounded to
     * 1, it doesn't matter much because the magnitude of
     * the result is so much larger.  For inputs near 1, we
     * can compute 1 - p exactly, although the precision on
     * the input is limited so we won't ever get more than
     * about 700 for the output.
     *
     * If - has relative error d0, / has relative error d1,
     * and log has relative error d2, then
     *
     *     (1 + d2) log((1 + d0) p/[(1 - p)(1 + d1)])
     *     = (1 + d2) [log(p/(1 - p)) + log((1 + d0)/(1 + d1))]
     *     = log(p/(1 - p)) + d2 log(p/(1 - p))
     *       + (1 + d2) log((1 + d0)/(1 + d1))
     *     = log(p/(1 - p))*[1 + d2 +
     *         + (1 + d2) log((1 + d0)/(1 + d1))/log(p/(1 - p))]
     *
     * Since 0 <= p < logistic(-1) or logistic(+1) < p <=
     * 1, we have |log(p/(1 - p))| > 1.  Hence this error
     * is bounded by
     *
     *     |d2 + (1 + d2) log((1 + d0)/(1 + d1))/log(p/(1 - p))|
     *     <= |d2| + |(1 + d2) log((1 + d0)/(1 + d1))
     *                      / log(p/(1 - p))|
     *     <= |d2| + |(1 + d2) log((1 + d0)/(1 + d1))|
     *     <= |d2| + 4|(1 + d2) (d0 - d1)|, by Lemma 3,
     *     <= |d2| + 4|d0 - d1 + d2 d0 - d1 d0|
     *     <= |d2| + 4|d0| + 4|d1| + 4|d2 d0| + 4|d1 d0|
     *     <= 9 eps + 8 eps^2.
     */
    return log(p/(1 - p));
  }
}

/**
 * Compute the logit function, translated in input by 1/2: logithalf(p)
 * = logit(1/2 + p).  Defined on [-1/2, 1/2].  Inverse of logistichalf.
 *
 * Ill-conditioned near +/-1/2.  If |p0| > 1/2 - 1/(1 + e), it may be
 * better to compute 1/2 + p0 or -1/2 - p0 and to use logit instead.
 * This implementation gives relative error bounded by 34 eps.
 */
STATIC double
logithalf(double p0)
{

  if (fabs(p0) <= 0.5 - 1/(1 + exp(1))) {
    /*
     * logit(1/2 + p0)
     * = log((1/2 + p0)/(1 - (1/2 + p0)))
     * = log((1/2 + p0)/(1/2 - p0))
     * = log(1 + (1/2 + p0)/(1/2 - p0) - 1)
     * = log(1 + (1/2 + p0 - (1/2 - p0))/(1/2 - p0))
     * = log(1 + (1/2 + p0 - 1/2 + p0)/(1/2 - p0))
     * = log(1 + 2 p0/(1/2 - p0))
     *
     * If the error of subtraction is d0, the error of
     * division is d1, and the error of log1p is d2, then
     * what we compute is
     *
     *  (1 + d2) log(1 + (1 + d1) 2 p0/[(1 + d0) (1/2 - p0)])
     *  = (1 + d2) log(1 + (1 + d') 2 p0/(1/2 - p0))
     *  = (1 + d2) (1 + d'') log(1 + 2 p0/(1/2 - p0))
     *  = (1 + d2 + d'' + d2 d'') log(1 + 2 p0/(1/2 - p0)),
     *
     * where |d'| < 2|d0 - d1| <= 4 eps by Lemma 2, and
     * |d''| < 8|d'| < 32 eps by Lemma 4 since
     *
     *  1/e <= 1 + 2*p0/(1/2 - p0) <= e
     *
     * when |p0| <= 1/2 - 1/(1 + e).  Hence the relative
     * error is bounded by
     *
     *  |d2 + d'' + d2 d''|
     *  <= |d2| + |d''| + |d2 d''|
     *  <= |d1| + 32 |d0| + 32 |d1 d0|
     *  <= 33 eps + 32 eps^2.
     */
    return log1p(2*p0/(0.5 - p0));
  } else {
    /*
     * We have a choice of computing logit(1/2 + p0) or
     * -logit(1 - (1/2 + p0)) = -logit(1/2 - p0).  It
     * doesn't matter which way we do this: either way,
     * since 1/2 p0 <= 1/2 <= 2 p0, the sum and difference
     * are computed exactly.  So let's do the one that
     * skips the final negation.
     *
     * The result is
     *
     *  (1 + d1) log((1 + d0) (1/2 + p0)/[(1 + d2) (1/2 - p0)])
     *  = (1 + d1) (1 + log((1 + d0)/(1 + d2))
     *                  / log((1/2 + p0)/(1/2 - p0)))
     *    * log((1/2 + p0)/(1/2 - p0))
     *  = (1 + d') log((1/2 + p0)/(1/2 - p0))
     *  = (1 + d') logit(1/2 + p0)
     *
     * where
     *
     *  d' = d1 + log((1 + d0)/(1 + d2))/logit(1/2 + p0)
     *       + d1 log((1 + d0)/(1 + d2))/logit(1/2 + p0).
     *
     * For |p| > 1/2 - 1/(1 + e), logit(1/2 + p0) > 1.
     * Provided |d0|, |d2| < 1/4, by Lemma 3 we have
     *
     *  |log((1 + d0)/(1 + d2))| <= 4|d0 - d2|.
     *
     * Hence the relative error is bounded by
     *
     *  |d'| <= |d1| + 4|d0 - d2| + 4|d1| |d0 - d2|
     *       <= |d1| + 4|d0| + 4|d2| + 4|d1 d0| + 4|d1 d2|
     *       <= 9 eps + 8 eps^2.
     */
    return log((0.5 + p0)/(0.5 - p0));
  }
}

/*
 * The following random_uniform_01 is tailored for IEEE 754 binary64
 * floating-point or smaller.  It can be adapted to larger
 * floating-point formats like i387 80-bit or IEEE 754 binary128, but
 * it may require sampling more bits.
 */
CTASSERT(FLT_RADIX == 2);
CTASSERT(-DBL_MIN_EXP <= 1021);
CTASSERT(DBL_MANT_DIG <= 53);

/**
 * Draw a floating-point number in [0, 1] with uniform distribution.
 *
 * Note that the probability of returning 0 is less than 2^-1074, so
 * callers need not check for it.  However, callers that cannot handle
 * rounding to 1 must deal with that, because it occurs with
 * probability 2^-54, which is small but nonnegligible.
 */
STATIC double
random_uniform_01(void)
{
  uint32_t z, x, hi, lo;
  double s;

  /*
   * Draw an exponent, geometrically distributed, but give up if
   * we get a run of more than 1088 zeros, which really means the
   * system is broken.
   */
  z = 0;
  while ((x = crypto_fast_rng_get_u32(get_thread_fast_rng())) == 0) {
    if (z >= 1088)
      /* Your bit sampler is broken.  Go home.  */
      return 0;
    z += 32;
  }
  z += clz32(x);

  /*
   * Pick 32-bit halves of an odd normalized significand.
   * Picking it odd breaks ties in the subsequent rounding, which
   * occur only with measure zero in the uniform distribution on
   * [0, 1].
   */
  hi = crypto_fast_rng_get_u32(get_thread_fast_rng()) | UINT32_C(0x80000000);
  lo = crypto_fast_rng_get_u32(get_thread_fast_rng()) | UINT32_C(0x00000001);

  /* Round to nearest scaled significand in [2^63, 2^64].  */
  s = hi*(double)4294967296 + lo;

  /* Rescale into [1/2, 1] and apply exponent in one swell foop.  */
  return s * ldexp(1, -(64 + z));
}

/*******************************************************************/

/* Functions for specific probability distributions start here: */

/*
 * Logistic(mu, sigma) distribution, supported on (-infinity,+infinity)
 *
 * This is the uniform distribution on [0,1] mapped into log-odds
 * space, scaled by sigma and translated by mu.
 *
 * pdf(x) = e^{-(x - mu)/sigma} sigma (1 + e^{-(x - mu)/sigma})^2
 * cdf(x) = 1/(1 + e^{-(x - mu)/sigma}) = logistic((x - mu)/sigma)
 * sf(x) = 1 - cdf(x) = 1 - logistic((x - mu)/sigma = logistic(-(x - mu)/sigma)
 * icdf(p) = mu + sigma log p/(1 - p) = mu + sigma logit(p)
 * isf(p) = mu + sigma log (1 - p)/p = mu - sigma logit(p)
 */

/**
 * Compute the CDF of the Logistic(mu, sigma) distribution: the
 * logistic function.  Well-conditioned for negative inputs and small
 * positive inputs; ill-conditioned for large positive inputs.
 */
STATIC double
cdf_logistic(double x, double mu, double sigma)
{
  return logistic((x - mu)/sigma);
}

/**
 * Compute the SF of the Logistic(mu, sigma) distribution: the logistic
 * function reflected over the y axis.  Well-conditioned for positive
 * inputs and small negative inputs; ill-conditioned for large negative
 * inputs.
 */
STATIC double
sf_logistic(double x, double mu, double sigma)
{
  return logistic(-(x - mu)/sigma);
}

/**
 * Compute the inverse of the CDF of the Logistic(mu, sigma)
 * distribution: the logit function.  Well-conditioned near 0;
 * ill-conditioned near 1/2 and 1.
 */
STATIC double
icdf_logistic(double p, double mu, double sigma)
{
  return mu + sigma*logit(p);
}

/**
 * Compute the inverse of the SF of the Logistic(mu, sigma)
 * distribution: the -logit function.  Well-conditioned near 0;
 * ill-conditioned near 1/2 and 1.
 */
STATIC double
isf_logistic(double p, double mu, double sigma)
{
  return mu - sigma*logit(p);
}

/*
 * LogLogistic(alpha, beta) distribution, supported on (0, +infinity).
 *
 * This is the uniform distribution on [0,1] mapped into odds space,
 * scaled by positive alpha and shaped by positive beta.
 *
 * Equivalent to computing exp of a Logistic(log alpha, 1/beta) sample.
 * (Name arises because the pdf has LogLogistic(x; alpha, beta) =
 * Logistic(log x; log alpha, 1/beta) and mathematicians got their
 * covariance contravariant.)
 *
 * pdf(x) = (beta/alpha) (x/alpha)^{beta - 1}/(1 + (x/alpha)^beta)^2
 *        = (1/e^mu sigma) (x/e^mu)^{1/sigma - 1} /
 *              (1 + (x/e^mu)^{1/sigma})^2
 * cdf(x) = 1/(1 + (x/alpha)^-beta) = 1/(1 + (x/e^mu)^{-1/sigma})
 *        = 1/(1 + (e^{log x}/e^mu)^{-1/sigma})
 *        = 1/(1 + (e^{log x - mu})^{-1/sigma})
 *        = 1/(1 + e^{-(log x - mu)/sigma})
 *        = logistic((log x - mu)/sigma)
 *        = logistic((log x - log alpha)/(1/beta))
 * sf(x) = 1 - 1/(1 + (x/alpha)^-beta)
 *       = (x/alpha)^-beta/(1 + (x/alpha)^-beta)
 *       = 1/((x/alpha)^beta + 1)
 *       = 1/(1 + (x/alpha)^beta)
 * icdf(p) = alpha (p/(1 - p))^{1/beta}
 *         = alpha e^{logit(p)/beta}
 *         = e^{mu + sigma logit(p)}
 * isf(p) = alpha ((1 - p)/p)^{1/beta}
 *        = alpha e^{-logit(p)/beta}
 *        = e^{mu - sigma logit(p)}
 */

/**
 * Compute the CDF of the LogLogistic(alpha, beta) distribution.
 * Well-conditioned for all x and alpha, and the condition number
 *
 *      -beta/[1 + (x/alpha)^{-beta}]
 *
 * grows linearly with beta.
 *
 * Loosely, the relative error of this implementation is bounded by
 *
 *      4 eps + 2 eps^2 + O(beta eps),
 *
 * so don't bother trying this for beta anywhere near as large as
 * 1/eps, around which point it levels off at 1.
 */
STATIC double
cdf_log_logistic(double x, double alpha, double beta)
{
  /*
   * Let d0 be the error of x/alpha; d1, of pow; d2, of +; and
   * d3, of the final quotient.  The exponentiation gives
   *
   *    ((1 + d0) x/alpha)^{-beta}
   *    = (x/alpha)^{-beta} (1 + d0)^{-beta}
   *    = (x/alpha)^{-beta} (1 + (1 + d0)^{-beta} - 1)
   *    = (x/alpha)^{-beta} (1 + d')
   *
   * where d' = (1 + d0)^{-beta} - 1.  If y = (x/alpha)^{-beta},
   * the denominator is
   *
   *    (1 + d2) (1 + (1 + d1) (1 + d') y)
   *    = (1 + d2) (1 + y + (d1 + d' + d1 d') y)
   *    = 1 + y + (1 + d2) (d1 + d' + d1 d') y
   *    = (1 + y) (1 + (1 + d2) (d1 + d' + d1 d') y/(1 + y))
   *    = (1 + y) (1 + d''),
   *
   * where d'' = (1 + d2) (d1 + d' + d1 d') y/(1 + y).  The
   * final result is
   *
   *    (1 + d3) / [(1 + d2) (1 + d'') (1 + y)]
   *    = (1 + d''') / (1 + y)
   *
   * for |d'''| <= 2|d3 - d''| by Lemma 2 as long as |d''| < 1/2
   * (which may not be the case for very large beta).  This
   * relative error is therefore bounded by
   *
   *    |d'''|
   *    <= 2|d3 - d''|
   *    <= 2|d3| + 2|(1 + d2) (d1 + d' + d1 d') y/(1 + y)|
   *    <= 2|d3| + 2|(1 + d2) (d1 + d' + d1 d')|
   *     = 2|d3| + 2|d1 + d' + d1 d' + d2 d1 + d2 d' + d2 d1 d'|
   *      <= 2|d3| + 2|d1| + 2|d'| + 2|d1 d'| + 2|d2 d1| + 2|d2 d'|
   *         + 2|d2 d1 d'|
   *      <= 4 eps + 2 eps^2 + (2 + 2 eps + 2 eps^2) |d'|.
   *
   * Roughly, |d'| = |(1 + d0)^{-beta} - 1| grows like beta eps,
   * until it levels off at 1.
   */
  return 1/(1 + pow(x/alpha, -beta));
}

/**
 * Compute the SF of the LogLogistic(alpha, beta) distribution.
 * Well-conditioned for all x and alpha, and the condition number
 *
 *      beta/[1 + (x/alpha)^beta]
 *
 * grows linearly with beta.
 *
 * Loosely, the relative error of this implementation is bounded by
 *
 *      4 eps + 2 eps^2 + O(beta eps)
 *
 * so don't bother trying this for beta anywhere near as large as
 * 1/eps, beyond which point it grows unbounded.
 */
STATIC double
sf_log_logistic(double x, double alpha, double beta)
{
  /*
   * The error analysis here is essentially the same as in
   * cdf_log_logistic, except that rather than levelling off at
   * 1, |(1 + d0)^beta - 1| grows unbounded.
   */
  return 1/(1 + pow(x/alpha, beta));
}

/**
 * Compute the inverse of the CDF of the LogLogistic(alpha, beta)
 * distribution.  Ill-conditioned for p near 1 and beta near 0 with
 * condition number 1/[beta (1 - p)].
 */
STATIC double
icdf_log_logistic(double p, double alpha, double beta)
{
  return alpha*pow(p/(1 - p), 1/beta);
}

/**
 * Compute the inverse of the SF of the LogLogistic(alpha, beta)
 * distribution.  Ill-conditioned for p near 1 and for large beta, with
 * condition number -1/[beta (1 - p)].
 */
STATIC double
isf_log_logistic(double p, double alpha, double beta)
{
  return alpha*pow((1 - p)/p, 1/beta);
}

/*
 * Weibull(lambda, k) distribution, supported on (0, +infinity).
 *
 * pdf(x) = (k/lambda) (x/lambda)^{k - 1} e^{-(x/lambda)^k}
 * cdf(x) = 1 - e^{-(x/lambda)^k}
 * icdf(p) = lambda * (-log (1 - p))^{1/k}
 * sf(x) = e^{-(x/lambda)^k}
 * isf(p) = lambda * (-log p)^{1/k}
 */

/**
 * Compute the CDF of the Weibull(lambda, k) distribution.
 * Well-conditioned for small x and k, and for large lambda --
 * condition number
 *
 *      -k (x/lambda)^k exp(-(x/lambda)^k)/[exp(-(x/lambda)^k) - 1]
 *
 * grows linearly with k, x^k, and lambda^{-k}.
 */
STATIC double
cdf_weibull(double x, double lambda, double k)
{
  return -expm1(-pow(x/lambda, k));
}

/**
 * Compute the SF of the Weibull(lambda, k) distribution.
 * Well-conditioned for small x and k, and for large lambda --
 * condition number
 *
 *      -k (x/lambda)^k
 *
 * grows linearly with k, x^k, and lambda^{-k}.
 */
STATIC double
sf_weibull(double x, double lambda, double k)
{
  return exp(-pow(x/lambda, k));
}

/**
 * Compute the inverse of the CDF of the Weibull(lambda, k)
 * distribution.  Ill-conditioned for p near 1, and for k near 0;
 * condition number is
 *
 *      (p/(1 - p))/(k log(1 - p)).
 */
STATIC double
icdf_weibull(double p, double lambda, double k)
{
  return lambda*pow(-log1p(-p), 1/k);
}

/**
 * Compute the inverse of the SF of the Weibull(lambda, k)
 * distribution.  Ill-conditioned for p near 0, and for k near 0;
 * condition number is
 *
 *      1/(k log(p)).
 */
STATIC double
isf_weibull(double p, double lambda, double k)
{
  return lambda*pow(-log(p), 1/k);
}

/*
 * GeneralizedPareto(mu, sigma, xi), supported on (mu, +infinity) for
 * nonnegative xi, or (mu, mu - sigma/xi) for negative xi.
 *
 * Samples:
 * = mu - sigma log U, if xi = 0;
 * = mu + sigma (U^{-xi} - 1)/xi = mu + sigma*expm1(-xi log U)/xi, if xi =/= 0,
 * where U is uniform on (0,1].
 * = mu + sigma (e^{xi X} - 1)/xi,
 * where X has standard exponential distribution.
 *
 * pdf(x) = sigma^{-1} (1 + xi (x - mu)/sigma)^{-(1 + 1/xi)}
 * cdf(x) = 1 - (1 + xi (x - mu)/sigma)^{-1/xi}
 *        = 1 - e^{-log(1 + xi (x - mu)/sigma)/xi}
 *        --> 1 - e^{-(x - mu)/sigma}  as  xi --> 0
 * sf(x) = (1 + xi (x - mu)/sigma)^{-1/xi}
 *       --> e^{-(x - mu)/sigma}  as  xi --> 0
 * icdf(p) = mu + sigma*(p^{-xi} - 1)/xi
 *         = mu + sigma*expm1(-xi log p)/xi
 *         --> mu + sigma*log p  as  xi --> 0
 * isf(p) = mu + sigma*((1 - p)^{xi} - 1)/xi
 *        = mu + sigma*expm1(-xi log1p(-p))/xi
 *        --> mu + sigma*log1p(-p)  as  xi --> 0
 */

/**
 * Compute the CDF of the GeneralizedPareto(mu, sigma, xi)
 * distribution.  Well-conditioned everywhere.  For standard
 * distribution (mu=0, sigma=1), condition number
 *
 *      (x/(1 + x xi)) / ((1 + x xi)^{1/xi} - 1)
 *
 * is bounded by 1, attained only at x = 0.
 */
STATIC double
cdf_genpareto(double x, double mu, double sigma, double xi)
{
  double x_0 = (x - mu)/sigma;

  /*
   * log(1 + xi x_0)/xi
   * = (-1/xi) \sum_{n=1}^infinity (-xi x_0)^n/n
   * = (-1/xi) (-xi x_0 + \sum_{n=2}^infinity (-xi x_0)^n/n)
   * = x_0 - (1/xi) \sum_{n=2}^infinity (-xi x_0)^n/n
   * = x_0 - x_0 \sum_{n=2}^infinity (-xi x_0)^{n-1}/n
   * = x_0 (1 - d),
   *
   * where d = \sum_{n=2}^infinity (-xi x_0)^{n-1}/n.  If |xi| <
   * eps/4|x_0|, then
   *
   * |d| <= \sum_{n=2}^infinity (eps/4)^{n-1}/n
   *     <= \sum_{n=2}^infinity (eps/4)^{n-1}
   *      = \sum_{n=1}^infinity (eps/4)^n
   *      = (eps/4) \sum_{n=0}^infinity (eps/4)^n
   *      = (eps/4)/(1 - eps/4)
   *      < eps/2
   *
   * for any 0 < eps < 2.  Thus, the relative error of x_0 from
   * log(1 + xi x_0)/xi is bounded by eps.
   */
  if (fabs(xi) < 1e-17/x_0)
    return -expm1(-x_0);
  else
    return -expm1(-log1p(xi*x_0)/xi);
}

/**
 * Compute the SF of the GeneralizedPareto(mu, sigma, xi) distribution.
 * For standard distribution (mu=0, sigma=1), ill-conditioned for xi
 * near 0; condition number
 *
 *      -x (1 + x xi)^{(-1 - xi)/xi}/(1 + x xi)^{-1/xi}
 *      = -x (1 + x xi)^{-1/xi - 1}/(1 + x xi)^{-1/xi}
 *      = -(x/(1 + x xi)) (1 + x xi)^{-1/xi}/(1 + x xi)^{-1/xi}
 *      = -x/(1 + x xi)
 *
 * is bounded by 1/xi.
 */
STATIC double
sf_genpareto(double x, double mu, double sigma, double xi)
{
  double x_0 = (x - mu)/sigma;

  if (fabs(xi) < 1e-17/x_0)
    return exp(-x_0);
  else
    return exp(-log1p(xi*x_0)/xi);
}

/**
 * Compute the inverse of the CDF of the GeneralizedPareto(mu, sigma,
 * xi) distribution.  Ill-conditioned for p near 1; condition number is
 *
 *      xi (p/(1 - p))/(1 - (1 - p)^xi)
 */
STATIC double
icdf_genpareto(double p, double mu, double sigma, double xi)
{
  /*
   * To compute f(xi) = (U^{-xi} - 1)/xi = (e^{-xi log U} - 1)/xi
   * for xi near zero (note f(xi) --> -log U as xi --> 0), write
   * the absolutely convergent Taylor expansion
   *
   * f(xi) = (1/xi)*(-xi log U + \sum_{n=2}^infinity (-xi log U)^n/n!
   *       = -log U + (1/xi)*\sum_{n=2}^infinity (-xi log U)^n/n!
   *       = -log U + \sum_{n=2}^infinity xi^{n-1} (-log U)^n/n!
   *       = -log U - log U \sum_{n=2}^infinity (-xi log U)^{n-1}/n!
   *       = -log U (1 + \sum_{n=2}^infinity (-xi log U)^{n-1}/n!).
   *
   * Let d = \sum_{n=2}^infinity (-xi log U)^{n-1}/n!.  What do we
   * lose if we discard it and use -log U as an approximation to
   * f(xi)?  If |xi| < eps/-4log U, then
   *
   * |d| <= \sum_{n=2}^infinity |xi log U|^{n-1}/n!
   *     <= \sum_{n=2}^infinity (eps/4)^{n-1}/n!
   *     <= \sum_{n=1}^infinity (eps/4)^n
   *      = (eps/4) \sum_{n=0}^infinity (eps/4)^n
   *      = (eps/4)/(1 - eps/4)
   *      < eps/2,
   *
   * for any 0 < eps < 2.  Hence, as long as |xi| < eps/-2log U,
   * f(xi) = -log U (1 + d) for |d| <= eps/2.  |d| is the
   * relative error of f(xi) from -log U; from this bound, the
   * relative error of -log U from f(xi) is at most (eps/2)/(1 -
   * eps/2) = eps/2 + (eps/2)^2 + (eps/2)^3 + ... < eps for 0 <
   * eps < 1.  Since -log U < 1000 for all U in (0, 1] in
   * binary64 floating-point, we can safely cut xi off at 1e-20 <
   * eps/4000 and attain <1ulp error from series truncation.
   */
  if (fabs(xi) <= 1e-20)
    return mu - sigma*log1p(-p);
  else
    return mu + sigma*expm1(-xi*log1p(-p))/xi;
}

/**
 * Compute the inverse of the SF of the GeneralizedPareto(mu, sigma,
 * xi) distribution.  Ill-conditioned for p near 1; condition number is
 *
 *      -xi/(1 - p^{-xi})
 */
STATIC double
isf_genpareto(double p, double mu, double sigma, double xi)
{
  if (fabs(xi) <= 1e-20)
    return mu - sigma*log(p);
  else
    return mu + sigma*expm1(-xi*log(p))/xi;
}

/*******************************************************************/

/**
 * Deterministic samplers, parametrized by uniform integer and (0,1]
 * samples.  No guarantees are made about _which_ mapping from the
 * integer and (0,1] samples these use; all that is guaranteed is the
 * distribution of the outputs conditioned on a uniform distribution on
 * the inputs.  The automatic tests in test_prob_distr.c double-check
 * the particular mappings we use.
 *
 * Beware: Unlike random_uniform_01(), these are not guaranteed to be
 * supported on all possible outputs.  See Ilya Mironov, `On the
 * Significance of the Least Significant Bits for Differential
 * Privacy', for an example of what can go wrong if you try to use
 * these to conceal information from an adversary but you expose the
 * specific full-precision floating-point values.
 *
 * Note: None of these samplers use rejection sampling; they are all
 * essentially inverse-CDF transforms with tweaks.  If you were to add,
 * say, a Gamma sampler with the Marsaglia-Tsang method, you would have
 * to parametrize it by a potentially infinite stream of uniform (and
 * perhaps normal) samples rather than a fixed number, which doesn't
 * make for quite as nice automatic testing as for these.
 */

/**
 * Deterministically sample from the interval [a, b], indexed by a
 * uniform random floating-point number p0 in (0, 1].
 *
 * Note that even if p0 is nonzero, the result may be equal to a, if
 * ulp(a)/2 is nonnegligible, e.g. if a = 1.  For maximum resolution,
 * arrange |a| <= |b|.
 */
STATIC double
sample_uniform_interval(double p0, double a, double b)
{
  /*
   * XXX Prove that the distribution is, in fact, uniform on
   * [a,b], particularly around p0 = 1, or at least has very
   * small deviation from uniform, quantified appropriately
   * (e.g., like in Monahan 1984, or by KL divergence).  It
   * almost certainly does but it would be nice to quantify the
   * error.
   */
  if ((a <= 0 && 0 <= b) || (b <= 0 && 0 <= a)) {
    /*
     * When ab < 0, (1 - t) a + t b is monotonic, since for
     * a <= b it is a sum of nondecreasing functions of t,
     * and for b <= a, of nonincreasing functions of t.
     * Further, clearly at 0 and 1 it attains a and b,
     * respectively.  Hence it is bounded within [a, b].
     */
    return (1 - p0)*a + p0*b;
  } else {
    /*
     * a + (b - a) t is monotonic -- it is obviously a
     * nondecreasing function of t for a <= b.  Further, it
     * attains a at 0, and while it may overshoot b at 1,
     * we have a
     *
     * Theorem.  If 0 <= t < 1, then the floating-point
     * evaluation of a + (b - a) t is bounded in [a, b].
     *
     * Lemma 1.  If 0 <= t < 1 is a floating-point number,
     * then for any normal floating-point number x except
     * the smallest in magnitude, |round(x*t)| < |x|.
     *
     * Proof.  WLOG, assume x >= 0.  Since the rounding
     * function and t |---> x*t are nondecreasing, their
     * composition t |---> round(x*t) is also
     * nondecreasing, so it suffices to consider the
     * largest floating-point number below 1, in particular
     * t = 1 - ulp(1)/2.
     *
     * Case I: If x is a power of two, then the next
     * floating-point number below x is x - ulp(x)/2 = x -
     * x*ulp(1)/2 = x*(1 - ulp(1)/2) = x*t, so, since x*t
     * is a floating-point number, multiplication is exact,
     * and thus round(x*t) = x*t < x.
     *
     * Case II: If x is not a power of two, then the
     * greatest lower bound of real numbers rounded to x is
     * x - ulp(x)/2 = x - ulp(T(x))/2 = x - T(x)*ulp(1)/2,
     * where T(X) is the largest power of two below x.
     * Anything below this bound is rounded to a
     * floating-point number smaller than x, and x*t = x*(1
     * - ulp(1)/2) = x - x*ulp(1)/2 < x - T(x)*ulp(1)/2
     * since T(x) < x, so round(x*t) < x*t < x.  QED.
     *
     * Lemma 2.  If x and y are subnormal, then round(x +
     * y) = x + y.
     *
     * Proof.  It is a matter of adding the significands,
     * since if we treat subnormals as having an implicit
     * zero bit before the `binary' point, their exponents
     * are all the same.  There is at most one carry/borrow
     * bit, which can always be accommodated either in a
     * subnormal, or, at largest, in the implicit one bit
     * of a normal.
     *
     * Lemma 3.  Let x and y be floating-point numbers.  If
     * round(x - y) is subnormal or zero, then it is equal
     * to x - y.
     *
     * Proof.  Case I (equal): round(x - y) = 0 iff x = y;
     * hence if round(x - y) = 0, then round(x - y) = 0 = x
     * - y.
     *
     * Case II (subnormal/subnormal): If x and y are both
     * subnormal, this follows directly from Lemma 2.
     *
     * Case IIIa (normal/subnormal): If x is normal and y
     * is subnormal, then x and y must share sign, or else
     * x - y would be larger than x and thus rounded to
     * normal.  If s is the smallest normal positive
     * floating-point number, |x| < 2s since by
     * construction 2s - |y| is normal for all subnormal y.
     * This means that x and y must have the same exponent,
     * so the difference is the difference of significands,
     * which is exact.
     *
     * Case IIIb (subnormal/normal): Same as case IIIa for
     * -(y - x).
     *
     * Case IV (normal/normal): If x and y are both normal,
     * then they must share sign, or else x - y would be
     * larger than x and thus rounded to normal.  Note that
     * |y| < 2|x|, for if |y| >= 2|x|, then |x| - |y| <=
     * -|x| but -|x| is normal like x.  Also, |x|/2 < |y|:
     * if |x|/2 is subnormal, it must hold because y is
     * normal; if |x|/2 is normal, then |x|/2 >= s, so
     * since |x| - |y| < s,
     *
     *  |x|/2 = |x| - |x|/2 <= |x| - s <= |y|;
     *
     * that is, |x|/2 < |y| < 2|x|, so by the Sterbenz
     * lemma, round(x - y) = x - y.  QED.
     *
     * Proof of theorem.  WLOG, assume 0 <= a <= b.  Since
     * round(a + round(round(b - a)*t) is nondecreasing in
     * t and attains a at 0, the lower end of the bound is
     * trivial; we must show the upper end of the bound
     * strictly.  It suffices to show this for the largest
     * floating-point number below 1, namely 1 - ulp(1)/2.
     *
     * Case I: round(b - a) is normal.  Then it is at most
     * the smallest floating-point number above b - a.  By
     * Lemma 1, round(round(b - a)*t) < round(b - a).
     * Since the inequality is strict, and since
     * round(round(b - a)*t) is a floating-point number
     * below round(b - a), and since there are no
     * floating-point numbers between b - a and round(b -
     * a), we must have round(round(b - a)*t) < b - a.
     * Then since y |---> round(a + y) is nondecreasing, we
     * must have
     *
     *  round(a + round(round(b - a)*t))
     *  <= round(a + (b - a))
     *   = round(b) = b.
     *
     * Case II: round(b - a) is subnormal.  In this case,
     * Lemma 1 falls apart -- we are not guaranteed the
     * strict inequality.  However, by Lemma 3, the
     * difference is exact: round(b - a) = b - a.  Thus,
     *
     *  round(a + round(round(b - a)*t))
     *  <= round(a + round((b - a)*t))
     *  <= round(a + (b - a))
     *   = round(b)
     *   = b,
     *
     * QED.
     */

    /* p0 is restricted to [0,1], but we use >= to silence -Wfloat-equal.  */
    if (p0 >= 1)
      return b;
    return a + (b - a)*p0;
  }
}

/**
 * Deterministically sample from the standard logistic distribution,
 * indexed by a uniform random 32-bit integer s and uniform random
 * floating-point numbers t and p0 in (0, 1].
 */
STATIC double
sample_logistic(uint32_t s, double t, double p0)
{
  double sign = (s & 1) ? -1 : +1;
  double r;

  /*
   * We carve up the interval (0, 1) into subregions to compute
   * the inverse CDF precisely:
   *
   * A = (0, 1/(1 + e)] ---> (-infinity, -1]
   * B = [1/(1 + e), 1/2] ---> [-1, 0]
   * C = [1/2, 1 - 1/(1 + e)] ---> [0, 1]
   * D = [1 - 1/(1 + e), 1) ---> [1, +infinity)
   *
   * Cases D and C are mirror images of cases A and B,
   * respectively, so we choose between them by the sign chosen
   * by a fair coin toss.  We choose between cases A and B by a
   * coin toss weighted by
   *
   *    2/(1 + e) = 1 - [1/2 - 1/(1 + e)]/(1/2):
   *
   * if it comes up heads, scale p0 into a uniform (0, 1/(1 + e)]
   * sample p; if it comes up tails, scale p0 into a uniform (0,
   * 1/2 - 1/(1 + e)] sample and compute the inverse CDF of p =
   * 1/2 - p0.
   */
  if (t <= 2/(1 + exp(1))) {
    /* p uniform in (0, 1/(1 + e)], represented by p.  */
    p0 /= 1 + exp(1);
    r = logit(p0);
  } else {
    /*
     * p uniform in [1/(1 + e), 1/2), actually represented
     * by p0 = 1/2 - p uniform in (0, 1/2 - 1/(1 + e)], so
     * that p = 1/2 - p.
     */
    p0 *= 0.5 - 1/(1 + exp(1));
    r = logithalf(p0);
  }

  /*
   * We have chosen from the negative half of the standard
   * logistic distribution, which is symmetric with the positive
   * half.  Now use the sign to choose uniformly between them.
   */
  return sign*r;
}

/**
 * Deterministically sample from the logistic distribution scaled by
 * sigma and translated by mu.
 */
static double
sample_logistic_locscale(uint32_t s, double t, double p0, double mu,
    double sigma)
{

  return mu + sigma*sample_logistic(s, t, p0);
}

/**
 * Deterministically sample from the standard log-logistic
 * distribution, indexed by a uniform random 32-bit integer s and a
 * uniform random floating-point number p0 in (0, 1].
 */
STATIC double
sample_log_logistic(uint32_t s, double p0)
{

  /*
   * Carve up the interval (0, 1) into (0, 1/2] and [1/2, 1); the
   * condition numbers of the icdf and the isf coincide at 1/2.
   */
  p0 *= 0.5;
  if ((s & 1) == 0) {
    /* p = p0 in (0, 1/2] */
    return p0/(1 - p0);
  } else {
    /* p = 1 - p0 in [1/2, 1) */
    return (1 - p0)/p0;
  }
}

/**
 * Deterministically sample from the log-logistic distribution with
 * scale alpha and shape beta.
 */
static double
sample_log_logistic_scaleshape(uint32_t s, double p0, double alpha,
    double beta)
{
  double x = sample_log_logistic(s, p0);

  return alpha*pow(x, 1/beta);
}

/**
 * Deterministically sample from the standard exponential distribution,
 * indexed by a uniform random 32-bit integer s and a uniform random
 * floating-point number p0 in (0, 1].
 */
static double
sample_exponential(uint32_t s, double p0)
{
  /*
   * We would like to evaluate log(p) for p near 0, and log1p(-p)
   * for p near 1.  Simply carve the interval into (0, 1/2] and
   * [1/2, 1) by a fair coin toss.
   */
  p0 *= 0.5;
  if ((s & 1) == 0)
    /* p = p0 in (0, 1/2] */
    return -log(p0);
  else
    /* p = 1 - p0 in [1/2, 1) */
    return -log1p(-p0);
}

/**
 * Deterministically sample from a Weibull distribution with scale
 * lambda and shape k -- just an exponential with a shape parameter in
 * addition to a scale parameter.  (Yes, lambda really is the scale,
 * _not_ the rate.)
 */
STATIC double
sample_weibull(uint32_t s, double p0, double lambda, double k)
{

  return lambda*pow(sample_exponential(s, p0), 1/k);
}

/**
 * Deterministically sample from the generalized Pareto distribution
 * with shape xi, indexed by a uniform random 32-bit integer s and a
 * uniform random floating-point number p0 in (0, 1].
 */
STATIC double
sample_genpareto(uint32_t s, double p0, double xi)
{
  double x = sample_exponential(s, p0);

  /*
   * Write f(xi) = (e^{xi x} - 1)/xi for xi near zero as the
   * absolutely convergent Taylor series
   *
   * f(x) = (1/xi) (xi x + \sum_{n=2}^infinity (xi x)^n/n!)
   *      = x + (1/xi) \sum_{n=2}^infinity (xi x)^n/n!
   *      = x + \sum_{n=2}^infinity xi^{n-1} x^n/n!
   *      = x + x \sum_{n=2}^infinity (xi x)^{n-1}/n!
   *      = x (1 + \sum_{n=2}^infinity (xi x)^{n-1}/n!).
   *
   * d = \sum_{n=2}^infinity (xi x)^{n-1}/n! is the relative error
   * of f(x) from x.  If |xi| < eps/4x, then
   *
   * |d| <= \sum_{n=2}^infinity |xi x|^{n-1}/n!
   *     <= \sum_{n=2}^infinity (eps/4)^{n-1}/n!
   *     <= \sum_{n=1}^infinity (eps/4)
   *      = (eps/4) \sum_{n=0}^infinity (eps/4)^n
   *      = (eps/4)/(1 - eps/4)
   *      < eps/2,
   *
   * for any 0 < eps < 2.  Hence, as long as |xi| < eps/2x, f(xi)
   * = x (1 + d) for |d| <= eps/2, so x = f(xi) (1 + d') for |d'|
   * <= eps.  What bound should we use for x?
   *
   * - If x is exponentially distributed, x > 200 with
   *   probability below e^{-200} << 2^{-256}, i.e. never.
   *
   * - If x is computed by -log(U) for U in (0, 1], x is
   *   guaranteed to be below 1000 in IEEE 754 binary64
   *   floating-point.
   *
   * We can safely cut xi off at 1e-20 < eps/4000 and attain an
   * error bounded by 0.5 ulp for this expression.
   */
  return (fabs(xi) < 1e-20 ? x : expm1(xi*x)/xi);
}

/**
 * Deterministically sample from a generalized Pareto distribution with
 * shape xi, scaled by sigma and translated by mu.
 */
static double
sample_genpareto_locscale(uint32_t s, double p0, double mu, double sigma,
    double xi)
{

  return mu + sigma*sample_genpareto(s, p0, xi);
}

/**
 * Deterministically sample from the geometric distribution with
 * per-trial success probability p.
 **/
// clang-format off
/*
 * XXX Quantify the error (KL divergence?) of this
 * ceiling-of-exponential sampler from a true geometric distribution,
 * which we could get by rejection sampling.  Relevant papers:
 *
 *      John F. Monahan, `Accuracy in Random Number Generation',
 *      Mathematics of Computation 45(172), October 1984, pp. 559--568.
https://pdfs.semanticscholar.org/aca6/74b96da1df77b2224e8cfc5dd6d61a471632.pdf
 *      Karl Bringmann and Tobias Friedrich, `Exact and Efficient
 *      Generation of Geometric Random Variates and Random Graphs', in
 *      Proceedings of the 40th International Colloaquium on Automata,
 *      Languages, and Programming -- ICALP 2013, Springer LNCS 7965,
 *      pp.267--278.
 *      https://doi.org/10.1007/978-3-642-39206-1_23
 *      https://people.mpi-inf.mpg.de/~kbringma/paper/2013ICALP-1.pdf
 */
// clang-format on
static double
sample_geometric(uint32_t s, double p0, double p)
{
  double x = sample_exponential(s, p0);

  /* This is actually a check against 1, but we do >= so that the compiler
     does not raise a -Wfloat-equal */
  if (p >= 1)
    return 1;

  return ceil(-x/log1p(-p));
}

/*******************************************************************/

/** Public API for probability distributions:
 *
 *  These are wrapper functions on top of the various probability distribution
 *  operations using the generic <b>dist</b> structure.

 *  These are the functions that should be used by consumers of this API.
 */

/** Returns the name of the distribution in <b>dist</b>. */
const char *
dist_name(const struct dist_t *dist)
{
  return dist->ops->name;
}

/* Sample a value from <b>dist</b> and return it. */
double
dist_sample(const struct dist_t *dist)
{
  return dist->ops->sample(dist);
}

/** Compute the CDF of <b>dist</b> at <b>x</b>. */
double
dist_cdf(const struct dist_t *dist, double x)
{
  return dist->ops->cdf(dist, x);
}

/** Compute the SF (Survival function) of <b>dist</b> at <b>x</b>. */
double
dist_sf(const struct dist_t *dist, double x)
{
  return dist->ops->sf(dist, x);
}

/** Compute the iCDF (Inverse CDF) of <b>dist</b> at <b>x</b>. */
double
dist_icdf(const struct dist_t *dist, double p)
{
  return dist->ops->icdf(dist, p);
}

/** Compute the iSF (Inverse Survival function) of <b>dist</b> at <b>x</b>. */
double
dist_isf(const struct dist_t *dist, double p)
{
  return dist->ops->isf(dist, p);
}

/** Functions for uniform distribution */

static double
uniform_sample(const struct dist_t *dist)
{
  const struct uniform_t *U = dist_to_const_uniform(dist);
  double p0 = random_uniform_01();

  return sample_uniform_interval(p0, U->a, U->b);
}

static double
uniform_cdf(const struct dist_t *dist, double x)
{
  const struct uniform_t *U = dist_to_const_uniform(dist);
  if (x < U->a)
    return 0;
  else if (x < U->b)
    return (x - U->a)/(U->b - U->a);
  else
    return 1;
}

static double
uniform_sf(const struct dist_t *dist, double x)
{
  const struct uniform_t *U = dist_to_const_uniform(dist);

  if (x > U->b)
    return 0;
  else if (x > U->a)
    return (U->b - x)/(U->b - U->a);
  else
    return 1;
}

static double
uniform_icdf(const struct dist_t *dist, double p)
{
  const struct uniform_t *U = dist_to_const_uniform(dist);
  double w = U->b - U->a;

  return (p < 0.5 ? (U->a + w*p) : (U->b - w*(1 - p)));
}

static double
uniform_isf(const struct dist_t *dist, double p)
{
  const struct uniform_t *U = dist_to_const_uniform(dist);
  double w = U->b - U->a;

  return (p < 0.5 ? (U->b - w*p) : (U->a + w*(1 - p)));
}

const struct dist_ops_t uniform_ops = {
  .name = "uniform",
  .sample = uniform_sample,
  .cdf = uniform_cdf,
  .sf = uniform_sf,
  .icdf = uniform_icdf,
  .isf = uniform_isf,
};

/*******************************************************************/

/** Private functions for each probability distribution. */

/** Functions for logistic distribution: */

static double
logistic_sample(const struct dist_t *dist)
{
  const struct logistic_t *L = dist_to_const_logistic(dist);
  uint32_t s = crypto_fast_rng_get_u32(get_thread_fast_rng());
  double t = random_uniform_01();
  double p0 = random_uniform_01();

  return sample_logistic_locscale(s, t, p0, L->mu, L->sigma);
}

static double
logistic_cdf(const struct dist_t *dist, double x)
{
  const struct logistic_t *L = dist_to_const_logistic(dist);
  return cdf_logistic(x, L->mu, L->sigma);
}

static double
logistic_sf(const struct dist_t *dist, double x)
{
  const struct logistic_t *L = dist_to_const_logistic(dist);
  return sf_logistic(x, L->mu, L->sigma);
}

static double
logistic_icdf(const struct dist_t *dist, double p)
{
  const struct logistic_t *L = dist_to_const_logistic(dist);
  return icdf_logistic(p, L->mu, L->sigma);
}

static double
logistic_isf(const struct dist_t *dist, double p)
{
  const struct logistic_t *L = dist_to_const_logistic(dist);
  return isf_logistic(p, L->mu, L->sigma);
}

const struct dist_ops_t logistic_ops = {
  .name = "logistic",
  .sample = logistic_sample,
  .cdf = logistic_cdf,
  .sf = logistic_sf,
  .icdf = logistic_icdf,
  .isf = logistic_isf,
};

/** Functions for log-logistic distribution: */

static double
log_logistic_sample(const struct dist_t *dist)
{
  const struct log_logistic_t *LL = dist_to_const_log_logistic(dist);
  uint32_t s = crypto_fast_rng_get_u32(get_thread_fast_rng());
  double p0 = random_uniform_01();

  return sample_log_logistic_scaleshape(s, p0, LL->alpha, LL->beta);
}

static double
log_logistic_cdf(const struct dist_t *dist, double x)
{
  const struct log_logistic_t *LL = dist_to_const_log_logistic(dist);
  return cdf_log_logistic(x, LL->alpha, LL->beta);
}

static double
log_logistic_sf(const struct dist_t *dist, double x)
{
  const struct log_logistic_t *LL = dist_to_const_log_logistic(dist);
  return sf_log_logistic(x, LL->alpha, LL->beta);
}

static double
log_logistic_icdf(const struct dist_t *dist, double p)
{
  const struct log_logistic_t *LL = dist_to_const_log_logistic(dist);
  return icdf_log_logistic(p, LL->alpha, LL->beta);
}

static double
log_logistic_isf(const struct dist_t *dist, double p)
{
  const struct log_logistic_t *LL = dist_to_const_log_logistic(dist);
  return isf_log_logistic(p, LL->alpha, LL->beta);
}

const struct dist_ops_t log_logistic_ops = {
  .name = "log logistic",
  .sample = log_logistic_sample,
  .cdf = log_logistic_cdf,
  .sf = log_logistic_sf,
  .icdf = log_logistic_icdf,
  .isf = log_logistic_isf,
};

/** Functions for Weibull distribution */

static double
weibull_sample(const struct dist_t *dist)
{
  const struct weibull_t *W = dist_to_const_weibull(dist);
  uint32_t s = crypto_fast_rng_get_u32(get_thread_fast_rng());
  double p0 = random_uniform_01();

  return sample_weibull(s, p0, W->lambda, W->k);
}

static double
weibull_cdf(const struct dist_t *dist, double x)
{
  const struct weibull_t *W = dist_to_const_weibull(dist);
  return cdf_weibull(x, W->lambda, W->k);
}

static double
weibull_sf(const struct dist_t *dist, double x)
{
  const struct weibull_t *W = dist_to_const_weibull(dist);
  return sf_weibull(x, W->lambda, W->k);
}

static double
weibull_icdf(const struct dist_t *dist, double p)
{
  const struct weibull_t *W = dist_to_const_weibull(dist);
  return icdf_weibull(p, W->lambda, W->k);
}

static double
weibull_isf(const struct dist_t *dist, double p)
{
  const struct weibull_t *W = dist_to_const_weibull(dist);
  return isf_weibull(p, W->lambda, W->k);
}

const struct dist_ops_t weibull_ops = {
  .name = "Weibull",
  .sample = weibull_sample,
  .cdf = weibull_cdf,
  .sf = weibull_sf,
  .icdf = weibull_icdf,
  .isf = weibull_isf,
};

/** Functions for generalized Pareto distributions */

static double
genpareto_sample(const struct dist_t *dist)
{
  const struct genpareto_t *GP = dist_to_const_genpareto(dist);
  uint32_t s = crypto_fast_rng_get_u32(get_thread_fast_rng());
  double p0 = random_uniform_01();

  return sample_genpareto_locscale(s, p0, GP->mu, GP->sigma, GP->xi);
}

static double
genpareto_cdf(const struct dist_t *dist, double x)
{
  const struct genpareto_t *GP = dist_to_const_genpareto(dist);
  return cdf_genpareto(x, GP->mu, GP->sigma, GP->xi);
}

static double
genpareto_sf(const struct dist_t *dist, double x)
{
  const struct genpareto_t *GP = dist_to_const_genpareto(dist);
  return sf_genpareto(x, GP->mu, GP->sigma, GP->xi);
}

static double
genpareto_icdf(const struct dist_t *dist, double p)
{
  const struct genpareto_t *GP = dist_to_const_genpareto(dist);
  return icdf_genpareto(p, GP->mu, GP->sigma, GP->xi);
}

static double
genpareto_isf(const struct dist_t *dist, double p)
{
  const struct genpareto_t *GP = dist_to_const_genpareto(dist);
  return isf_genpareto(p, GP->mu, GP->sigma, GP->xi);
}

const struct dist_ops_t genpareto_ops = {
  .name = "generalized Pareto",
  .sample = genpareto_sample,
  .cdf = genpareto_cdf,
  .sf = genpareto_sf,
  .icdf = genpareto_icdf,
  .isf = genpareto_isf,
};

/** Functions for geometric distribution on number of trials before success */

static double
geometric_sample(const struct dist_t *dist)
{
  const struct geometric_t *G = dist_to_const_geometric(dist);
  uint32_t s = crypto_fast_rng_get_u32(get_thread_fast_rng());
  double p0 = random_uniform_01();

  return sample_geometric(s, p0, G->p);
}

static double
geometric_cdf(const struct dist_t *dist, double x)
{
  const struct geometric_t *G = dist_to_const_geometric(dist);

  if (x < 1)
    return 0;
  /* 1 - (1 - p)^floor(x) = 1 - e^{floor(x) log(1 - p)} */
  return -expm1(floor(x)*log1p(-G->p));
}

static double
geometric_sf(const struct dist_t *dist, double x)
{
  const struct geometric_t *G = dist_to_const_geometric(dist);

  if (x < 1)
    return 0;
  /* (1 - p)^floor(x) = e^{ceil(x) log(1 - p)} */
  return exp(floor(x)*log1p(-G->p));
}

static double
geometric_icdf(const struct dist_t *dist, double p)
{
  const struct geometric_t *G = dist_to_const_geometric(dist);

  return log1p(-p)/log1p(-G->p);
}

static double
geometric_isf(const struct dist_t *dist, double p)
{
  const struct geometric_t *G = dist_to_const_geometric(dist);

  return log(p)/log1p(-G->p);
}

const struct dist_ops_t geometric_ops = {
  .name = "geometric (1-based)",
  .sample = geometric_sample,
  .cdf = geometric_cdf,
  .sf = geometric_sf,
  .icdf = geometric_icdf,
  .isf = geometric_isf,
};
