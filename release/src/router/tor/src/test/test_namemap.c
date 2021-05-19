/* Copyright (c) 2018-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "test/test.h"

#include "lib/cc/torint.h"
#include "lib/container/namemap.h"
#include "lib/container/namemap_st.h"
#include "lib/malloc/malloc.h"

#include <stdio.h>
#include <string.h>

static void
test_namemap_empty(void *arg)
{
  (void)arg;

  namemap_t m;
  namemap_init(&m);
  namemap_t m2 = NAMEMAP_INIT();

  tt_uint_op(0, OP_EQ, namemap_get_size(&m));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m, "hello"));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m, "hello"));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m, "hello128"));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m, ""));
  tt_uint_op(0, OP_EQ, namemap_get_size(&m));

  tt_uint_op(0, OP_EQ, namemap_get_size(&m2));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m2, "hello"));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m2, "hello"));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m2, "hello128"));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m2, ""));
  tt_uint_op(0, OP_EQ, namemap_get_size(&m));

 done:
  namemap_clear(&m);
  namemap_clear(&m2);
}

static void
test_namemap_toolong(void *arg)
{
  (void)arg;
  namemap_t m;
  char *ok = NULL;
  char *toolong = NULL;
  namemap_init(&m);

  ok = tor_malloc_zero(MAX_NAMEMAP_NAME_LEN+1);
  memset(ok, 'x', MAX_NAMEMAP_NAME_LEN);

  toolong = tor_malloc_zero(MAX_NAMEMAP_NAME_LEN+2);
  memset(toolong, 'x', MAX_NAMEMAP_NAME_LEN+1);

  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m, ok));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m, toolong));
  unsigned u1 = namemap_get_or_create_id(&m, toolong);
  unsigned u2 = namemap_get_or_create_id(&m, ok);
  tt_uint_op(u1, OP_EQ, NAMEMAP_ERR);
  tt_uint_op(u2, OP_NE, NAMEMAP_ERR);
  tt_uint_op(u2, OP_EQ, namemap_get_id(&m, ok));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m, toolong));

  tt_str_op(ok, OP_EQ, namemap_get_name(&m, u2));
  tt_ptr_op(NULL, OP_EQ, namemap_get_name(&m, u1));

 done:
  tor_free(ok);
  tor_free(toolong);
  namemap_clear(&m);
}

static void
test_namemap_blackbox(void *arg)
{
  (void)arg;

  namemap_t m1, m2;
  namemap_init(&m1);
  namemap_init(&m2);

  unsigned u1 = namemap_get_or_create_id(&m1, "hello");
  unsigned u2 = namemap_get_or_create_id(&m1, "world");
  tt_uint_op(u1, OP_NE, NAMEMAP_ERR);
  tt_uint_op(u2, OP_NE, NAMEMAP_ERR);
  tt_uint_op(u1, OP_NE, u2);

  tt_uint_op(u1, OP_EQ, namemap_get_id(&m1, "hello"));
  tt_uint_op(u1, OP_EQ, namemap_get_or_create_id(&m1, "hello"));
  tt_uint_op(u2, OP_EQ, namemap_get_id(&m1, "world"));
  tt_uint_op(u2, OP_EQ, namemap_get_or_create_id(&m1, "world"));

  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m1, "HELLO"));
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m2, "hello"));

  unsigned u3 = namemap_get_or_create_id(&m2, "hola");
  tt_uint_op(u3, OP_NE, NAMEMAP_ERR);
  tt_uint_op(NAMEMAP_ERR, OP_EQ, namemap_get_id(&m1, "hola"));
  tt_uint_op(u3, OP_EQ, namemap_get_or_create_id(&m2, "hola"));
  tt_uint_op(u3, OP_EQ, namemap_get_id(&m2, "hola"));

  unsigned int u4 = namemap_get_or_create_id(&m1, "hola");
  tt_uint_op(u4, OP_NE, NAMEMAP_ERR);
  tt_uint_op(u4, OP_EQ, namemap_get_id(&m1, "hola"));
  tt_uint_op(u3, OP_EQ, namemap_get_id(&m2, "hola"));

  tt_str_op("hello", OP_EQ, namemap_get_name(&m1, u1));
  tt_str_op("world", OP_EQ, namemap_get_name(&m1, u2));
  tt_str_op("hola", OP_EQ, namemap_get_name(&m2, u3));
  tt_str_op("hola", OP_EQ, namemap_get_name(&m1, u4));

  tt_ptr_op(NULL, OP_EQ, namemap_get_name(&m2, u3 + 10));

 done:
  namemap_clear(&m1);
  namemap_clear(&m2);
}

static void
test_namemap_internals(void *arg)
{
  (void)arg;
  // This test actually assumes know something about the identity layout.
  namemap_t m;
  namemap_init(&m);

  tt_uint_op(0, OP_EQ, namemap_get_or_create_id(&m, "that"));
  tt_uint_op(0, OP_EQ, namemap_get_or_create_id(&m, "that"));
  tt_uint_op(1, OP_EQ, namemap_get_or_create_id(&m, "is"));
  tt_uint_op(1, OP_EQ, namemap_get_or_create_id(&m, "is"));

  tt_uint_op(0, OP_EQ, namemap_get_id(&m, "that"));
  tt_uint_op(0, OP_EQ, namemap_get_id(&m, "that"));
  tt_uint_op(1, OP_EQ, namemap_get_id(&m, "is"));
  tt_uint_op(2, OP_EQ, namemap_get_or_create_id(&m, "not"));
  tt_uint_op(1, OP_EQ, namemap_get_or_create_id(&m, "is"));
  tt_uint_op(2, OP_EQ, namemap_get_or_create_id(&m, "not"));

 done:
  namemap_clear(&m);
}

static void
test_namemap_fmt(void *arg)
{
  (void)arg;
  namemap_t m = NAMEMAP_INIT();

  unsigned a = namemap_get_or_create_id(&m, "greetings");
  unsigned b = namemap_get_or_create_id(&m, "earthlings");

  tt_str_op(namemap_fmt_name(&m, a), OP_EQ, "greetings");
  tt_str_op(namemap_fmt_name(&m, b), OP_EQ, "earthlings");
  tt_int_op(a, OP_NE, 100);
  tt_int_op(b, OP_NE, 100);
  tt_str_op(namemap_fmt_name(&m, 100), OP_EQ, "{100}");

 done:
  namemap_clear(&m);
}

#define T(name) \
  { #name, test_namemap_ ## name , 0, NULL, NULL }

struct testcase_t namemap_tests[] = {
  T(empty),
  T(toolong),
  T(blackbox),
  T(internals),
  T(fmt),
  END_OF_TESTCASES
};
