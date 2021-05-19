/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file order.c
 * \brief Functions for finding the n'th element of an array.
 **/

#include <stdlib.h>

#include "lib/container/order.h"
#include "lib/log/util_bug.h"

/** Declare a function called <b>funcname</b> that acts as a find_nth_FOO
 * function for an array of type <b>elt_t</b>*.
 *
 * NOTE: The implementation kind of sucks: It's O(n log n), whereas finding
 * the kth element of an n-element list can be done in O(n).  Then again, this
 * implementation is not in critical path, and it is obviously correct. */
#define IMPLEMENT_ORDER_FUNC(funcname, elt_t)                   \
  static int                                                    \
  _cmp_ ## elt_t(const void *_a, const void *_b)                \
  {                                                             \
    const elt_t *a = _a, *b = _b;                               \
    if (*a<*b)                                                  \
      return -1;                                                \
    else if (*a>*b)                                             \
      return 1;                                                 \
    else                                                        \
      return 0;                                                 \
  }                                                             \
  elt_t                                                         \
  funcname(elt_t *array, int n_elements, int nth)               \
  {                                                             \
    tor_assert(nth >= 0);                                       \
    tor_assert(nth < n_elements);                               \
    qsort(array, n_elements, sizeof(elt_t), _cmp_ ##elt_t);     \
    return array[nth];                                          \
  }

IMPLEMENT_ORDER_FUNC(find_nth_int, int)
IMPLEMENT_ORDER_FUNC(find_nth_time, time_t)
IMPLEMENT_ORDER_FUNC(find_nth_double, double)
IMPLEMENT_ORDER_FUNC(find_nth_uint32, uint32_t)
IMPLEMENT_ORDER_FUNC(find_nth_int32, int32_t)
IMPLEMENT_ORDER_FUNC(find_nth_long, long)
