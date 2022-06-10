/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file risky_options.c
 * \brief List compile-time options that might make Tor less reliable.
 **/

#include "orconfig.h"
#include "app/main/risky_options.h"

/** A space-separated list of the compile-time options might make Tor less
 *  reliable or secure.  These options mainly exist for testing or debugging.
 */
const char risky_option_list[] =
  ""
#ifdef DISABLE_ASSERTS_IN_TEST
  " --disable-asserts-in-test"
#endif
#ifdef TOR_UNIT_TESTS
  " TOR_UNIT_TESTS"
#endif
#ifdef ENABLE_RESTART_DEBUGGING
  " --enable-restart-debugging"
#endif
#ifdef ALL_BUGS_ARE_FATAL
  " --enable-all-bugs-are-fatal"
#endif
#ifdef DISABLE_MEMORY_SENTINELS
  " --disable-memory-sentinels"
#endif
  ;
