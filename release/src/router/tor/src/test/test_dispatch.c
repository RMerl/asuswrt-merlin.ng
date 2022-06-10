/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define DISPATCH_NEW_PRIVATE
#define DISPATCH_PRIVATE

#include "test/test.h"

#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_cfg.h"
#include "lib/dispatch/dispatch_st.h"
#include "lib/dispatch/msgtypes.h"

#include "lib/log/escape.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"

#include <stdio.h>
#include <string.h>

static dispatch_t *dispatcher_in_use=NULL;

static void
test_dispatch_max_in_u16_sl(void *arg)
{
  (void)arg;
  smartlist_t *sl = smartlist_new();
  uint16_t nums[] = { 10, 20, 30 };
  tt_int_op(-1, OP_EQ, max_in_u16_sl(sl, -1));

  smartlist_add(sl, NULL);
  tt_int_op(-1, OP_EQ, max_in_u16_sl(sl, -1));

  smartlist_add(sl, &nums[1]);
  tt_int_op(20, OP_EQ, max_in_u16_sl(sl, -1));

  smartlist_add(sl, &nums[0]);
  tt_int_op(20, OP_EQ, max_in_u16_sl(sl, -1));

  smartlist_add(sl, NULL);
  tt_int_op(20, OP_EQ, max_in_u16_sl(sl, -1));

  smartlist_add(sl, &nums[2]);
  tt_int_op(30, OP_EQ, max_in_u16_sl(sl, -1));

 done:
  smartlist_free(sl);
}

/* Construct an empty dispatch_t. */
static void
test_dispatch_empty(void *arg)
{
  (void)arg;

  dispatch_t *d=NULL;
  dispatch_cfg_t *cfg=NULL;

  cfg = dcfg_new();
  d = dispatch_new(cfg);
  tt_assert(d);

 done:
  dispatch_free(d);
  dcfg_free(cfg);
}

static int total_recv1_simple = 0;
static int total_recv2_simple = 0;

static void
simple_recv1(const msg_t *m)
{
  total_recv1_simple += m->aux_data__.u64;
}

static char *recv2_received = NULL;

static void
simple_recv2(const msg_t *m)
{
  tor_free(recv2_received);
  recv2_received = dispatch_fmt_msg_data(dispatcher_in_use, m);

  total_recv2_simple += m->aux_data__.u64*10;
}

/* Construct a dispatch_t with two messages, make sure that they both get
 * delivered. */
static void
test_dispatch_simple(void *arg)
{
  (void)arg;

  dispatch_t *d=NULL;
  dispatch_cfg_t *cfg=NULL;
  int r;

  cfg = dcfg_new();
  r = dcfg_msg_set_type(cfg,0,0);
  r += dcfg_msg_set_chan(cfg,0,0);
  r += dcfg_add_recv(cfg,0,1,simple_recv1);
  r += dcfg_msg_set_type(cfg,1,0);
  r += dcfg_msg_set_chan(cfg,1,0);
  r += dcfg_add_recv(cfg,1,1,simple_recv2);
  r += dcfg_add_recv(cfg,1,1,simple_recv2); /* second copy */
  tt_int_op(r, OP_EQ, 0);

  d = dispatch_new(cfg);
  tt_assert(d);
  dispatcher_in_use = d;

  msg_aux_data_t data = {.u64 = 7};
  r = dispatch_send(d, 99, 0, 0, 0, data);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(total_recv1_simple, OP_EQ, 0);

  r = dispatch_flush(d, 0, INT_MAX);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(total_recv1_simple, OP_EQ, 7);
  tt_int_op(total_recv2_simple, OP_EQ, 0);

  total_recv1_simple = 0;
  r = dispatch_send(d, 99, 0, 1, 0, data);
  tt_int_op(r, OP_EQ, 0);
  r = dispatch_flush(d, 0, INT_MAX);
  tt_int_op(total_recv1_simple, OP_EQ, 0);
  tt_int_op(total_recv2_simple, OP_EQ, 140);

  tt_str_op(recv2_received, OP_EQ, "<>"); // no format function was set.

 done:
  dispatch_free(d);
  dcfg_free(cfg);
  tor_free(recv2_received);
}

/* Construct a dispatch_t with a message and no receiver; make sure that it
 * gets dropped properly. */
static void
test_dispatch_no_recipient(void *arg)
{
  (void)arg;

  dispatch_t *d=NULL;
  dispatch_cfg_t *cfg=NULL;
  int r;

  cfg = dcfg_new();
  r = dcfg_msg_set_type(cfg,0,0);
  r += dcfg_msg_set_chan(cfg,0,0);
  tt_int_op(r, OP_EQ, 0);

  d = dispatch_new(cfg);
  tt_assert(d);
  dispatcher_in_use = d;

  msg_aux_data_t data = { .u64 = 7};
  r = dispatch_send(d, 99, 0, 0, 0, data);
  tt_int_op(r, OP_EQ, 0);

  r = dispatch_flush(d, 0, INT_MAX);
  tt_int_op(r, OP_EQ, 0);

 done:
  dispatch_free(d);
  dcfg_free(cfg);
}

struct coord_t { int x; int y; };
static void
free_coord(msg_aux_data_t d)
{
  tor_free(d.ptr);
}
static char *
fmt_coord(msg_aux_data_t d)
{
  char *v;
  struct coord_t *c = d.ptr;
  tor_asprintf(&v, "[%d, %d]", c->x, c->y);
  return v;
}
static dispatch_typefns_t coord_fns = {
  .fmt_fn = fmt_coord,
  .free_fn = free_coord,
};
static void
alert_run_immediate(dispatch_t *d, channel_id_t ch, void *arg)
{
  (void)arg;
  dispatch_flush(d, ch, INT_MAX);
}

static char *received_data=NULL;

static void
recv_typed_data(const msg_t *m)
{
  tor_free(received_data);
  received_data = dispatch_fmt_msg_data(dispatcher_in_use, m);
}

static void
test_dispatch_with_types(void *arg)
{
  (void)arg;

  dispatch_t *d=NULL;
  dispatch_cfg_t *cfg=NULL;
  int r;

  cfg = dcfg_new();
  r = dcfg_msg_set_type(cfg,5,3);
  r += dcfg_msg_set_chan(cfg,5,2);
  r += dcfg_add_recv(cfg,5,0,recv_typed_data);
  r += dcfg_type_set_fns(cfg,3,&coord_fns);
  tt_int_op(r, OP_EQ, 0);

  d = dispatch_new(cfg);
  tt_assert(d);
  dispatcher_in_use = d;

  /* Make this message get run immediately. */
  r = dispatch_set_alert_fn(d, 2, alert_run_immediate, NULL);
  tt_int_op(r, OP_EQ, 0);

  struct coord_t *xy = tor_malloc(sizeof(*xy));
  xy->x = 13;
  xy->y = 37;
  msg_aux_data_t data = {.ptr = xy};
  r = dispatch_send(d, 99/*sender*/, 2/*channel*/, 5/*msg*/, 3/*type*/, data);
  tt_int_op(r, OP_EQ, 0);
  tt_str_op(received_data, OP_EQ, "[13, 37]");

 done:
  dispatch_free(d);
  dcfg_free(cfg);
  tor_free(received_data);
  dispatcher_in_use = NULL;
}

static void
test_dispatch_bad_type_setup(void *arg)
{
  (void)arg;
  static dispatch_typefns_t fns;
  dispatch_cfg_t *cfg = dcfg_new();

  tt_int_op(0, OP_EQ, dcfg_type_set_fns(cfg, 7, &coord_fns));

  fns = coord_fns;
  fns.fmt_fn = NULL;
  tt_int_op(-1, OP_EQ, dcfg_type_set_fns(cfg, 7, &fns));

  fns = coord_fns;
  fns.free_fn = NULL;
  tt_int_op(-1, OP_EQ, dcfg_type_set_fns(cfg, 7, &fns));

  fns = coord_fns;
  tt_int_op(0, OP_EQ, dcfg_type_set_fns(cfg, 7, &fns));

 done:
  dcfg_free(cfg);
}

#define T(name)                                                 \
  { #name, test_dispatch_ ## name, TT_FORK, NULL, NULL }

struct testcase_t dispatch_tests[] = {
  T(max_in_u16_sl),
  T(empty),
  T(simple),
  T(no_recipient),
  T(with_types),
  T(bad_type_setup),
  END_OF_TESTCASES
};
