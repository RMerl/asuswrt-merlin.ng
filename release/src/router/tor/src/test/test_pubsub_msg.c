/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define DISPATCH_PRIVATE

#include "test/test.h"

#include "lib/dispatch/dispatch.h"
#include "lib/dispatch/dispatch_naming.h"
#include "lib/dispatch/dispatch_st.h"
#include "lib/dispatch/msgtypes.h"
#include "lib/pubsub/pubsub_flags.h"
#include "lib/pubsub/pub_binding_st.h"
#include "lib/pubsub/pubsub_build.h"
#include "lib/pubsub/pubsub_builder_st.h"
#include "lib/pubsub/pubsub_connect.h"
#include "lib/pubsub/pubsub_publish.h"

#include "lib/log/escape.h"
#include "lib/malloc/malloc.h"
#include "lib/string/printf.h"

#include <stdio.h>
#include <string.h>

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
static dispatch_typefns_t stringfns = {
  .free_fn = ex_str_free,
  .fmt_fn = ex_str_fmt
};

// We're using the lowest-level publish/subscribe logic here, to avoid the
// pubsub_macros.h macros and just test the dispatch core.  We'll use a string
// type for everything.

#define DECLARE_MESSAGE(suffix)                         \
  static pub_binding_t pub_binding_##suffix;            \
  static int msg_received_##suffix = 0;                 \
  static void recv_msg_##suffix(const msg_t *m) {       \
    (void)m;                                            \
    ++msg_received_##suffix;                            \
  }                                                     \
  EAT_SEMICOLON

#define ADD_PUBLISH(binding_suffix, subsys, channel, msg, flags)        \
  STMT_BEGIN {                                                          \
    con = pubsub_connector_for_subsystem(builder,                       \
                                           get_subsys_id(#subsys));     \
    pubsub_add_pub_(con, &pub_binding_##binding_suffix,                 \
                      get_channel_id(#channel),                         \
                      get_message_id(#msg), get_msg_type_id("string"),  \
                      (flags), __FILE__, __LINE__);                     \
    pubsub_connector_free(con);                                         \
  } STMT_END

#define ADD_SUBSCRIBE(hook_suffix, subsys, channel, msg, flags)         \
  STMT_BEGIN {                                                          \
    con = pubsub_connector_for_subsystem(builder,                       \
                                           get_subsys_id(#subsys));     \
    pubsub_add_sub_(con, recv_msg_##hook_suffix,                        \
                      get_channel_id(#channel),                         \
                      get_message_id(#msg), get_msg_type_id("string"),  \
                      (flags), __FILE__, __LINE__);                     \
    pubsub_connector_free(con);                                         \
  } STMT_END

#define SEND(binding_suffix, val)                          \
  STMT_BEGIN {                                             \
    msg_aux_data_t data_;                                  \
    data_.ptr = tor_strdup(val);                           \
    pubsub_pub_(&pub_binding_##binding_suffix, data_);     \
  } STMT_END

DECLARE_MESSAGE(msg1);
DECLARE_MESSAGE(msg2);
DECLARE_MESSAGE(msg3);
DECLARE_MESSAGE(msg4);
DECLARE_MESSAGE(msg5);

static smartlist_t *strings_received = NULL;
static void
recv_msg_copy_string(const msg_t *m)
{
  const char *s = m->aux_data__.ptr;
  smartlist_add(strings_received, tor_strdup(s));
}

static void *
setup_dispatcher(const struct testcase_t *testcase)
{
  (void)testcase;
  pubsub_builder_t *builder = pubsub_builder_new();
  pubsub_connector_t *con;

  {
    con = pubsub_connector_for_subsystem(builder, get_subsys_id("types"));
    pubsub_connector_register_type_(con,
                                    get_msg_type_id("string"),
                                    &stringfns,
                                    "nowhere.c", 99);
    pubsub_connector_free(con);
  }
  // message1 has one publisher and one subscriber.
  ADD_PUBLISH(msg1, sys1, main, message1, 0);
  ADD_SUBSCRIBE(msg1, sys2, main, message1, 0);

  // message2 has a publisher and a stub subscriber.
  ADD_PUBLISH(msg2, sys1, main, message2, 0);
  ADD_SUBSCRIBE(msg2, sys2, main, message2, DISP_FLAG_STUB);

  // message3 has a publisher and three subscribers.
  ADD_PUBLISH(msg3, sys1, main, message3, 0);
  ADD_SUBSCRIBE(msg3, sys2, main, message3, 0);
  ADD_SUBSCRIBE(msg3, sys3, main, message3, 0);
  ADD_SUBSCRIBE(msg3, sys4, main, message3, 0);

  // message4 has one publisher and two subscribers, but it's on another
  // channel.
  ADD_PUBLISH(msg4, sys2, other, message4, 0);
  ADD_SUBSCRIBE(msg4, sys1, other, message4, 0);
  ADD_SUBSCRIBE(msg4, sys3, other, message4, 0);

  // message5 has a huge number of recipients.
  ADD_PUBLISH(msg5, sys3, main, message5, 0);
  ADD_SUBSCRIBE(msg5, sys4, main, message5, 0);
  ADD_SUBSCRIBE(msg5, sys5, main, message5, 0);
  ADD_SUBSCRIBE(msg5, sys6, main, message5, 0);
  ADD_SUBSCRIBE(msg5, sys7, main, message5, 0);
  ADD_SUBSCRIBE(msg5, sys8, main, message5, 0);
  for (int i = 0; i < 1000-5; ++i) {
    char *sys;
    tor_asprintf(&sys, "xsys-%d", i);
    con = pubsub_connector_for_subsystem(builder, get_subsys_id(sys));
    pubsub_add_sub_(con, recv_msg_copy_string,
                    get_channel_id("main"),
                    get_message_id("message5"),
                    get_msg_type_id("string"), 0, "here", 100);
    pubsub_connector_free(con);
    tor_free(sys);
  }

  return pubsub_builder_finalize(builder, NULL);
}

static int
cleanup_dispatcher(const struct testcase_t *testcase, void *dispatcher_)
{
  (void)testcase;
  dispatch_t *dispatcher = dispatcher_;
  dispatch_free(dispatcher);
  return 1;
}

static const struct testcase_setup_t dispatcher_setup = {
  setup_dispatcher, cleanup_dispatcher
};

static void
test_pubsub_msg_minimal(void *arg)
{
  dispatch_t *d = arg;

  tt_int_op(0, OP_EQ, msg_received_msg1);
  SEND(msg1, "hello world");
  tt_int_op(0, OP_EQ, msg_received_msg1); // hasn't actually arrived yet.

  tt_int_op(0, OP_EQ, dispatch_flush(d, get_channel_id("main"), 1000));
  tt_int_op(1, OP_EQ, msg_received_msg1); // we got the message!

 done:
  ;
}

static void
test_pubsub_msg_send_to_stub(void *arg)
{
  dispatch_t *d = arg;

  tt_int_op(0, OP_EQ, msg_received_msg2);
  SEND(msg2, "hello silence");
  tt_int_op(0, OP_EQ, msg_received_msg2); // hasn't actually arrived yet.

  tt_int_op(0, OP_EQ, dispatch_flush(d, get_channel_id("main"), 1000));
  tt_int_op(0, OP_EQ, msg_received_msg2); // doesn't arrive -- stub hook.

 done:
  ;
}

static void
test_pubsub_msg_cancel_msgs(void *arg)
{
  dispatch_t *d = arg;

  tt_int_op(0, OP_EQ, msg_received_msg1);
  for (int i = 0; i < 100; ++i) {
    SEND(msg1, "hello world");
  }
  tt_int_op(0, OP_EQ, msg_received_msg1); // hasn't actually arrived yet.

  tt_int_op(0, OP_EQ, dispatch_flush(d, get_channel_id("main"), 10));
  tt_int_op(10, OP_EQ, msg_received_msg1); // we got the message 10 times.

  // At this point, the dispatcher will be freed with queued, undelivered
  // messages.
 done:
  ;
}

struct alertfn_target {
  dispatch_t *d;
  channel_id_t ch;
  int count;
};
static void
alertfn_generic(dispatch_t *d, channel_id_t ch, void *arg)
{
  struct alertfn_target *t = arg;
  tt_ptr_op(d, OP_EQ, t->d);
  tt_int_op(ch, OP_EQ, t->ch);
  ++t->count;
 done:
  ;
}

static void
test_pubsub_msg_alertfns(void *arg)
{
  dispatch_t *d = arg;
  struct alertfn_target ch1_a = { d, get_channel_id("main"), 0 };
  struct alertfn_target ch2_a = { d, get_channel_id("other"), 0 };

  tt_int_op(0, OP_EQ,
            dispatch_set_alert_fn(d, get_channel_id("main"),
                                  alertfn_generic, &ch1_a));
  tt_int_op(0, OP_EQ,
            dispatch_set_alert_fn(d, get_channel_id("other"),
                                  alertfn_generic, &ch2_a));

  SEND(msg3, "hello");
  tt_int_op(ch1_a.count, OP_EQ, 1);
  SEND(msg3, "world");
  tt_int_op(ch1_a.count, OP_EQ, 1); // only the first message sends an alert
  tt_int_op(ch2_a.count, OP_EQ, 0); // no alert for 'other'

  SEND(msg4, "worse things happen in C");
  tt_int_op(ch2_a.count, OP_EQ, 1);

  // flush the first (main) channel...
  tt_int_op(0, OP_EQ, dispatch_flush(d, get_channel_id("main"), 1000));
  tt_int_op(6, OP_EQ, msg_received_msg3); // 3 subscribers, 2 instances.

  // now that the main channel is flushed, sending another message on it
  // starts another alert.
  tt_int_op(ch1_a.count, OP_EQ, 1);
  SEND(msg1, "plover");
  tt_int_op(ch1_a.count, OP_EQ, 2);
  tt_int_op(ch2_a.count, OP_EQ, 1);

 done:
  ;
}

/* try more than N_FAST_FNS hooks on msg5 */
static void
test_pubsub_msg_many_hooks(void *arg)
{
  dispatch_t *d = arg;
  strings_received = smartlist_new();

  tt_int_op(0, OP_EQ, msg_received_msg5);
  SEND(msg5, "hello world");
  tt_int_op(0, OP_EQ, msg_received_msg5);
  tt_int_op(0, OP_EQ, smartlist_len(strings_received));

  tt_int_op(0, OP_EQ, dispatch_flush(d, get_channel_id("main"), 100000));
  tt_int_op(5, OP_EQ, msg_received_msg5);
  tt_int_op(995, OP_EQ, smartlist_len(strings_received));

 done:
  SMARTLIST_FOREACH(strings_received, char *, s, tor_free(s));
  smartlist_free(strings_received);
}

#define T(name)                                                 \
  { #name, test_pubsub_msg_ ## name , TT_FORK,                \
      &dispatcher_setup, NULL }

struct testcase_t pubsub_msg_tests[] = {
  T(minimal),
  T(send_to_stub),
  T(cancel_msgs),
  T(alertfns),
  T(many_hooks),
  END_OF_TESTCASES
};
