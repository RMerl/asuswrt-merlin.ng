/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file laplace.c
 *
 * \brief Implements a Laplace distribution, used for adding noise to things.
 **/

#include "orconfig.h"
#include "lib/math/laplace.h"
#include "lib/math/fp.h"

#include "lib/log/util_bug.h"

#include <math.h>
#include <stdlib.h>

/** Transform a random value <b>p</b> from the uniform distribution in
 * [0.0, 1.0[ into a Laplace distributed value with location parameter
 * <b>mu</b> and scale parameter <b>b</b>. Truncate the final result
 * to be an integer in [INT64_MIN, INT64_MAX]. */
int64_t
sample_laplace_distribution(double mu, double b, double p)
{
  double result;
  tor_assert(p >= 0.0 && p < 1.0);

  /* This is the "inverse cumulative distribution function" from:
   * https://en.wikipedia.org/wiki/Laplace_distribution */
  if (p <= 0.0) {
    /* Avoid taking log(0.0) == -INFINITY, as some processors or compiler
     * options can cause the program to trap. */
    return INT64_MIN;
  }

  result = mu - b * (p > 0.5 ? 1.0 : -1.0)
                  * tor_mathlog(1.0 - 2.0 * fabs(p - 0.5));

  return clamp_double_to_int64(result);
}

/** Add random noise between INT64_MIN and INT64_MAX coming from a Laplace
 * distribution with mu = 0 and b = <b>delta_f</b>/<b>epsilon</b> to
 * <b>signal</b> based on the provided <b>random</b> value in [0.0, 1.0[.
 * The epsilon value must be between ]0.0, 1.0]. delta_f must be greater
 * than 0. */
int64_t
add_laplace_noise(int64_t signal_, double random_, double delta_f,
                  double epsilon)
{
  int64_t noise;

  /* epsilon MUST be between ]0.0, 1.0] */
  tor_assert(epsilon > 0.0 && epsilon <= 1.0);
  /* delta_f MUST be greater than 0. */
  tor_assert(delta_f > 0.0);

  /* Just add noise, no further signal */
  noise = sample_laplace_distribution(0.0,
                                      delta_f / epsilon,
                                      random_);

  /* Clip (signal + noise) to [INT64_MIN, INT64_MAX] */
  if (noise > 0 && INT64_MAX - noise < signal_)
    return INT64_MAX;
  else if (noise < 0 && INT64_MIN - noise > signal_)
    return INT64_MIN;
  else
    return signal_ + noise;
}
