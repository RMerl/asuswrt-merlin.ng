/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file addsub.c
 *
 * \brief Helpers for addition and subtraction.
 *
 * Currently limited to non-wrapping (saturating) addition.
 **/

#include "lib/intmath/addsub.h"
#include "lib/cc/compat_compiler.h"

/* Helper: safely add two uint32_t's, capping at UINT32_MAX rather
 * than overflow */
uint32_t
tor_add_u32_nowrap(uint32_t a, uint32_t b)
{
  /* a+b > UINT32_MAX check, without overflow */
  if (PREDICT_UNLIKELY(a > UINT32_MAX - b)) {
    return UINT32_MAX;
  } else {
    return a+b;
  }
}
