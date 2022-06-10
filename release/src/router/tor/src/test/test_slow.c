/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_slow.c
 * \brief Slower unit tests for many pieces of the lower level Tor modules.
 **/

#include "orconfig.h"

#include <stdio.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "core/or/or.h"
#include "test/test.h"

struct testgroup_t testgroups[] = {
  { "slow/crypto/", slow_crypto_tests },
  { "slow/process/", slow_process_tests },
  { "slow/prob_distr/", slow_stochastic_prob_distr_tests },
  { "slow/ptr/", slow_ptr_tests },
  END_OF_GROUPS
};

