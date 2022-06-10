/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define DISPATCH_PRIVATE
#define PUBSUB_PRIVATE

#include "test/test.h"

#include "lib/cc/torint.h"
#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_naming.h"
#include "lib/dispatch/dispatch_st.h"
#include "lib/dispatch/msgtypes.h"
#include "lib/pubsub/pubsub_macros.h"
#include "lib/pubsub/pubsub_build.h"
#include "lib/pubsub/pubsub_builder_st.h"

#include "lib/log/escape.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"

#include "test/log_test_helpers.h"

#include <stdio.h>
#include <string.h>

static char *
ex_int_fmt(msg_aux_data_t aux)
{
  int val = (int) aux.u64;
  char *r=NULL;
  tor_asprintf(&r, "%d", val);
  return r;
}

static char *
ex_str_fmt(msg_aux_data_t aux)
{
  return esc_for_log(aux.ptr);
}

static void
ex_str_free(msg_aux_data_t aux)
{
  tor_free_(aux.ptr);
}

static dispatch_typefns_t intfns = {
  .fmt_fn = ex_int_fmt
};

static dispatch_typefns_t stringfns = {
  .free_fn = ex_str_free,
  .fmt_fn = ex_str_fmt
};

DECLARE_MESSAGE_INT(bunch_of_coconuts, int, int);
DECLARE_PUBLISH(bunch_of_coconuts);
DECLARE_SUBSCRIBE(bunch_of_coconuts, coconut_recipient_cb);

DECLARE_MESSAGE(yes_we_have_no, string, char *);
DECLARE_PUBLISH(yes_we_have_no);
DECLARE_SUBSCRIBE(yes_we_have_no, absent_item_cb);

static void
coconut_recipient_cb(const msg_t *m, int n_coconuts)
{
  (void)m;
  (void)n_coconuts;
}

static void
absent_item_cb(const msg_t *m, const char *fruitname)
{
  (void)m;
  (void)fruitname;
}

#define FLAG_SKIP 99999

static void
seed_dispatch_builder(pubsub_builder_t *b,
                      unsigned fl1, unsigned fl2, unsigned fl3, unsigned fl4)
{
  pubsub_connector_t *c = NULL;

  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("sys1"));
    DISPATCH_REGISTER_TYPE(c, int, &intfns);
    if (fl1 != FLAG_SKIP)
      DISPATCH_ADD_PUB_(c, main, bunch_of_coconuts, fl1);
    if (fl2 != FLAG_SKIP)
      DISPATCH_ADD_SUB_(c, main, yes_we_have_no, fl2);
    pubsub_connector_free(c);
  }

  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("sys2"));
    DISPATCH_REGISTER_TYPE(c, string, &stringfns);
    if (fl3 != FLAG_SKIP)
      DISPATCH_ADD_PUB_(c, main, yes_we_have_no, fl3);
    if (fl4 != FLAG_SKIP)
      DISPATCH_ADD_SUB_(c, main, bunch_of_coconuts, fl4);
    pubsub_connector_free(c);
  }
}

static void
seed_pubsub_builder_basic(pubsub_builder_t *b)
{
  seed_dispatch_builder(b, 0, 0, 0, 0);
}

/* Regular builder with valid types and messages.
 */
static void
test_pubsub_build_types_ok(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;
  pubsub_items_t *items = NULL;

  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);

  dispatcher = pubsub_builder_finalize(b, &items);
  b = NULL;
  tt_assert(dispatcher);
  tt_assert(items);
  tt_int_op(smartlist_len(items->items), OP_EQ, 4);

  // Make sure that the bindings got build correctly.
  SMARTLIST_FOREACH_BEGIN(items->items, pubsub_cfg_t *, item) {
    if (item->is_publish) {
      tt_assert(item->pub_binding);
      tt_ptr_op(item->pub_binding->dispatch_ptr, OP_EQ, dispatcher);
    }
  } SMARTLIST_FOREACH_END(item);

  tt_int_op(dispatcher->n_types, OP_GE, 2);
  tt_assert(dispatcher->typefns);

  tt_assert(dispatcher->typefns[get_msg_type_id("int")].fmt_fn == ex_int_fmt);
  tt_assert(dispatcher->typefns[get_msg_type_id("string")].fmt_fn ==
            ex_str_fmt);

  // Now clear the bindings, like we would do before freeing the
  // the dispatcher.
  pubsub_items_clear_bindings(items);
  SMARTLIST_FOREACH_BEGIN(items->items, pubsub_cfg_t *, item) {
    if (item->is_publish) {
      tt_assert(item->pub_binding);
      tt_ptr_op(item->pub_binding->dispatch_ptr, OP_EQ, NULL);
    }
  } SMARTLIST_FOREACH_END(item);

 done:
  pubsub_connector_free(c);
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  pubsub_items_free(items);
}

/* We fail if the same type is defined in two places with different functions.
 */
static void
test_pubsub_build_types_decls_conflict(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;

  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);
  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("sys3"));
    // Extra declaration of int: we don't allow this.
    DISPATCH_REGISTER_TYPE(c, int, &stringfns);
    pubsub_connector_free(c);
  }

  setup_full_capture_of_logs(LOG_WARN);
  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher == NULL);
  // expect_log_msg_containing("(int) declared twice"); // XXXX

 done:
  pubsub_connector_free(c);
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  teardown_capture_of_logs();
}

/* If a message ID exists but nobody is publishing or subscribing to it,
 * that's okay. */
static void
test_pubsub_build_unused_message(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;

  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);

  // This message isn't actually generated by anyone, but that will be fine:
  // we just log it at info.
  get_message_id("unused");
  setup_capture_of_logs(LOG_INFO);

  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher);
  expect_log_msg_containing(
     "Nobody is publishing or subscribing to message");

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  teardown_capture_of_logs();
}

/* Publishing or subscribing to a message with no subscribers / publishers
 * should fail and warn. */
static void
test_pubsub_build_missing_pubsub(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;

  b = pubsub_builder_new();
  seed_dispatch_builder(b, 0, 0, FLAG_SKIP, FLAG_SKIP);

  setup_full_capture_of_logs(LOG_WARN);
  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher == NULL);

  expect_log_msg_containing(
       "Message \"bunch_of_coconuts\" has publishers, but no subscribers.");
  expect_log_msg_containing(
       "Message \"yes_we_have_no\" has subscribers, but no publishers.");

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  teardown_capture_of_logs();
}

/* Make sure that a stub publisher or subscriber prevents an error from
 * happening even if there are no other publishers/subscribers for a message
 */
static void
test_pubsub_build_stub_pubsub(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;

  b = pubsub_builder_new();
  seed_dispatch_builder(b, 0, 0, DISP_FLAG_STUB, DISP_FLAG_STUB);

  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher);

  // 1 subscriber.
  tt_int_op(1, OP_EQ,
            dispatcher->table[get_message_id("yes_we_have_no")]->n_enabled);
  // no subscribers
  tt_ptr_op(NULL, OP_EQ,
            dispatcher->table[get_message_id("bunch_of_coconuts")]);

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
}

/* Only one channel per msg id. */
static void
test_pubsub_build_channels_conflict(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;

  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);
  pub_binding_t btmp;

  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("problems"));
    /* Usually the DISPATCH_ADD_PUB macro would keep us from using
     * the wrong channel */
    pubsub_add_pub_(c, &btmp, get_channel_id("hithere"),
                    get_message_id("bunch_of_coconuts"),
                    get_msg_type_id("int"),
                    0 /* flags */,
                    "somewhere.c", 22);
    pubsub_connector_free(c);
  };

  setup_full_capture_of_logs(LOG_WARN);
  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher == NULL);

  expect_log_msg_containing("Message \"bunch_of_coconuts\" is associated "
                            "with multiple inconsistent channels.");

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  teardown_capture_of_logs();
}

/* Only one type per msg id. */
static void
test_pubsub_build_types_conflict(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;

  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);
  pub_binding_t btmp;

  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("problems"));
    /* Usually the DISPATCH_ADD_PUB macro would keep us from using
     * the wrong channel */
    pubsub_add_pub_(c, &btmp, get_channel_id("hithere"),
                    get_message_id("bunch_of_coconuts"),
                    get_msg_type_id("string"),
                    0 /* flags */,
                    "somewhere.c", 22);
    pubsub_connector_free(c);
  };

  setup_full_capture_of_logs(LOG_WARN);
  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher == NULL);

  expect_log_msg_containing("Message \"bunch_of_coconuts\" is associated "
                            "with multiple inconsistent message types.");

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  teardown_capture_of_logs();
}

/* The same module can't publish and subscribe the same message */
static void
test_pubsub_build_pubsub_same(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;

  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);

  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("sys1"));
    // already publishing this.
    DISPATCH_ADD_SUB(c, main, bunch_of_coconuts);
    pubsub_connector_free(c);
  };

  setup_full_capture_of_logs(LOG_WARN);
  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher == NULL);

  expect_log_msg_containing("Message \"bunch_of_coconuts\" is published "
                            "and subscribed by the same subsystem \"sys1\".");

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  teardown_capture_of_logs();
}

/* More than one subsystem may publish or subscribe, and that's okay. */
static void
test_pubsub_build_pubsub_multi(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;

  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);
  pub_binding_t btmp;

  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("sys3"));
    DISPATCH_ADD_SUB(c, main, bunch_of_coconuts);
    pubsub_add_pub_(c, &btmp, get_channel_id("main"),
                    get_message_id("yes_we_have_no"),
                    get_msg_type_id("string"),
                    0 /* flags */,
                    "somewhere.c", 22);
    pubsub_connector_free(c);
  };

  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher);

  // 1 subscribers
  tt_int_op(1, OP_EQ,
            dispatcher->table[get_message_id("yes_we_have_no")]->n_enabled);
  // 2 subscribers.
  dtbl_entry_t *ent =
    dispatcher->table[get_message_id("bunch_of_coconuts")];
  tt_int_op(2, OP_EQ, ent->n_enabled);
  tt_int_op(2, OP_EQ, ent->n_fns);
  tt_ptr_op(ent->rcv[0].fn, OP_EQ, recv_fn__bunch_of_coconuts);
  tt_ptr_op(ent->rcv[1].fn, OP_EQ, recv_fn__bunch_of_coconuts);

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
}

static void
some_other_coconut_hook(const msg_t *m)
{
  (void)m;
}

/* Subscribe hooks should be build correctly when there are a bunch of
 * them. */
static void
test_pubsub_build_sub_many(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;
  char *sysname = NULL;
  b = pubsub_builder_new();
  seed_pubsub_builder_basic(b);

  int i;
  for (i = 1; i < 100; ++i) {
    tor_asprintf(&sysname, "system%d",i);
    c = pubsub_connector_for_subsystem(b, get_subsys_id(sysname));
    if (i % 7) {
      DISPATCH_ADD_SUB(c, main, bunch_of_coconuts);
    } else {
      pubsub_add_sub_(c, some_other_coconut_hook,
                      get_channel_id("main"),
                      get_message_id("bunch_of_coconuts"),
                      get_msg_type_id("int"),
                      0 /* flags */,
                      "somewhere.c", 22);
    }
    pubsub_connector_free(c);
    tor_free(sysname);
  };

  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher);

  dtbl_entry_t *ent =
    dispatcher->table[get_message_id("bunch_of_coconuts")];
  tt_int_op(100, OP_EQ, ent->n_enabled);
  tt_int_op(100, OP_EQ, ent->n_fns);
  tt_ptr_op(ent->rcv[0].fn, OP_EQ, recv_fn__bunch_of_coconuts);
  tt_ptr_op(ent->rcv[1].fn, OP_EQ, recv_fn__bunch_of_coconuts);
  tt_ptr_op(ent->rcv[76].fn, OP_EQ, recv_fn__bunch_of_coconuts);
  tt_ptr_op(ent->rcv[77].fn, OP_EQ, some_other_coconut_hook);
  tt_ptr_op(ent->rcv[78].fn, OP_EQ, recv_fn__bunch_of_coconuts);

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  tor_free(sysname);
}

/* It's fine to declare the excl flag. */
static void
test_pubsub_build_excl_ok(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;

  b = pubsub_builder_new();
  // Try one excl/excl pair and one excl/non pair.
  seed_dispatch_builder(b, DISP_FLAG_EXCL, 0,
                        DISP_FLAG_EXCL, DISP_FLAG_EXCL);

  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher);

  // 1 subscribers
  tt_int_op(1, OP_EQ,
            dispatcher->table[get_message_id("yes_we_have_no")]->n_enabled);
  // 1 subscriber.
  tt_int_op(1, OP_EQ,
            dispatcher->table[get_message_id("bunch_of_coconuts")]->n_enabled);

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
}

/* but if you declare the excl flag, you need to mean it. */
static void
test_pubsub_build_excl_bad(void *arg)
{
  (void)arg;
  pubsub_builder_t *b = NULL;
  dispatch_t *dispatcher = NULL;
  pubsub_connector_t *c = NULL;

  b = pubsub_builder_new();
  seed_dispatch_builder(b, DISP_FLAG_EXCL, DISP_FLAG_EXCL,
                        0, 0);

  {
    c = pubsub_connector_for_subsystem(b, get_subsys_id("sys3"));
    DISPATCH_ADD_PUB_(c, main, bunch_of_coconuts, 0);
    DISPATCH_ADD_SUB_(c, main, yes_we_have_no, 0);
    pubsub_connector_free(c);
  };

  setup_full_capture_of_logs(LOG_WARN);
  dispatcher = pubsub_builder_finalize(b, NULL);
  b = NULL;
  tt_assert(dispatcher == NULL);

  expect_log_msg_containing("has multiple publishers, but at least one is "
                            "marked as exclusive.");
  expect_log_msg_containing("has multiple subscribers, but at least one is "
                            "marked as exclusive.");

 done:
  pubsub_builder_free(b);
  dispatch_free(dispatcher);
  teardown_capture_of_logs();
}

#define T(name, flags)                                          \
  { #name, test_pubsub_build_ ## name , (flags), NULL, NULL }

struct testcase_t pubsub_build_tests[] = {
  T(types_ok, TT_FORK),
  T(types_decls_conflict, TT_FORK),
  T(unused_message, TT_FORK),
  T(missing_pubsub, TT_FORK),
  T(stub_pubsub, TT_FORK),
  T(channels_conflict, TT_FORK),
  T(types_conflict, TT_FORK),
  T(pubsub_same, TT_FORK),
  T(pubsub_multi, TT_FORK),
  T(sub_many, TT_FORK),
  T(excl_ok, TT_FORK),
  T(excl_bad, TT_FORK),
  END_OF_TESTCASES
};
