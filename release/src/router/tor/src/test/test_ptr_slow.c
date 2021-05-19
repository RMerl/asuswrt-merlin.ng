/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "core/or/or.h"
#include "test/test.h"
#include "test/ptr_helpers.h"

#include <stdint.h>
#include <limits.h>

/** Assert that <b>a</b> can be cast to void * and back. */
static void
assert_int_voidptr_roundtrip(int a)
{
  intptr_t ap = (intptr_t)a;
  void *b = cast_intptr_to_voidstar(ap);
  intptr_t c = cast_voidstar_to_intptr(b);
  void *d = cast_intptr_to_voidstar(c);

  tt_assert(ap == c);
  tt_assert(b == d);

 done:
  return;
}

/** Test for possibility of casting `int` to `void *` and back. */
static void
test_int_voidstar_interop(void *arg)
{
  int a;
  (void)arg;

  for (a = -1024; a <= 1024; a++) {
    assert_int_voidptr_roundtrip(a);
  }

  for (a = INT_MIN; a <= INT_MIN+1024; a++) {
    assert_int_voidptr_roundtrip(a);
  }

  for (a = INT_MAX-1024; a < INT_MAX; a++) {
    assert_int_voidptr_roundtrip(a);
  }

  a = 1;
  for (unsigned long i = 0; i < sizeof(int) * 8; i++) {
    assert_int_voidptr_roundtrip(a);
    a = (a << 1);
  }
}

/** Assert that <b>a</b> can be cast to void * and back. */
static void
assert_uint_voidptr_roundtrip(unsigned int a)
{
 uintptr_t ap = (uintptr_t)a;
 void *b = cast_uintptr_to_voidstar(ap);
 uintptr_t c = cast_voidstar_to_uintptr(b);
 void *d = cast_uintptr_to_voidstar(c);

 tt_assert(ap == c);
 tt_assert(b == d);

 done:
  return;
}

/** Test for possibility of casting `int` to `void *` and back. */
static void
test_uint_voidstar_interop(void *arg)
{
  unsigned int a;
  (void)arg;

  for (a = 0; a <= 1024; a++) {
    assert_uint_voidptr_roundtrip(a);
  }

  for (a = UINT_MAX-1024; a < UINT_MAX; a++) {
    assert_uint_voidptr_roundtrip(a);
  }

  a = 1;
  for (unsigned long i = 0; i < sizeof(int) * 8; i++) {
    assert_uint_voidptr_roundtrip(a);
    a = (a << 1);
  }
}

struct testcase_t slow_ptr_tests[] = {
  { .name = "int_voidstar_interop",
    .fn = test_int_voidstar_interop,
    .flags = 0,
    .setup = NULL,
    .setup_data = NULL },
  { .name = "uint_voidstar_interop",
    .fn = test_uint_voidstar_interop,
    .flags = 0,
    .setup = NULL,
    .setup_data = NULL },
  END_OF_TESTCASES
};
