/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#include <math.h>

#define SCHEDULER_KIST_PRIVATE
#define CHANNEL_OBJECT_PRIVATE
#define CHANNEL_FILE_PRIVATE
#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/evloop/compat_libevent.h"
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#include "core/mainloop/connection.h"
#include "feature/nodelist/networkstatus.h"
#define SCHEDULER_PRIVATE
#include "core/or/scheduler.h"

/* Test suite stuff */
#include "test/test.h"
#include "test/fakechans.h"

/* Shamelessly stolen from compat_libevent.c */
#define V(major, minor, patch) \
  (((major) << 24) | ((minor) << 16) | ((patch) << 8))

/******************************************************************************
 * Statistical info
 *****************************************************************************/
static int scheduler_compare_channels_mock_ctr = 0;
static int scheduler_run_mock_ctr = 0;

/******************************************************************************
 * Utility functions and things we need to mock
 *****************************************************************************/
static or_options_t mocked_options;
static const or_options_t *
mock_get_options(void)
{
  return &mocked_options;
}

static void
cleanup_scheduler_options(void)
{
  if (mocked_options.SchedulerTypes_) {
    SMARTLIST_FOREACH(mocked_options.SchedulerTypes_, int *, i, tor_free(i));
    smartlist_free(mocked_options.SchedulerTypes_);
    mocked_options.SchedulerTypes_ = NULL;
  }
}

static void
set_scheduler_options(int val)
{
  int *type;

  if (mocked_options.SchedulerTypes_ == NULL) {
    mocked_options.SchedulerTypes_ = smartlist_new();
  }
  type = tor_malloc_zero(sizeof(int));
  *type = val;
  smartlist_add(mocked_options.SchedulerTypes_, type);
}

static void
clear_options(void)
{
  cleanup_scheduler_options();
  memset(&mocked_options, 0, sizeof(mocked_options));
}

static int32_t
mock_vanilla_networkstatus_get_param(
    const networkstatus_t *ns, const char *param_name, int32_t default_val,
    int32_t min_val, int32_t max_val)
{
  (void)ns;
  (void)default_val;
  (void)min_val;
  (void)max_val;
  // only support KISTSchedRunInterval right now
  tor_assert(strcmp(param_name, "KISTSchedRunInterval")==0);
  return 0;
}

static int32_t
mock_kist_networkstatus_get_param(
    const networkstatus_t *ns, const char *param_name, int32_t default_val,
    int32_t min_val, int32_t max_val)
{
  (void)ns;
  (void)default_val;
  (void)min_val;
  (void)max_val;
  // only support KISTSchedRunInterval right now
  tor_assert(strcmp(param_name, "KISTSchedRunInterval")==0);
  return 12;
}

static int
scheduler_compare_channels_mock(const void *c1_v,
                                const void *c2_v)
{
  uintptr_t p1, p2;

  p1 = (uintptr_t)(c1_v);
  p2 = (uintptr_t)(c2_v);

  ++scheduler_compare_channels_mock_ctr;

  if (p1 == p2) return 0;
  else if (p1 < p2) return 1;
  else return -1;
}

static void
scheduler_run_noop_mock(void)
{
  ++scheduler_run_mock_ctr;
}

static circuitmux_t *mock_ccm_tgt_1 = NULL;
static circuitmux_t *mock_ccm_tgt_2 = NULL;
static circuitmux_t *mock_cgp_tgt_1 = NULL;
static circuitmux_policy_t *mock_cgp_val_1 = NULL;
static circuitmux_t *mock_cgp_tgt_2 = NULL;
static circuitmux_policy_t *mock_cgp_val_2 = NULL;

static const circuitmux_policy_t *
circuitmux_get_policy_mock(circuitmux_t *cmux)
{
  const circuitmux_policy_t *result = NULL;

  tt_assert(cmux != NULL);
  if (cmux) {
    if (cmux == mock_cgp_tgt_1) result = mock_cgp_val_1;
    else if (cmux == mock_cgp_tgt_2) result = mock_cgp_val_2;
    else result = circuitmux_get_policy__real(cmux);
  }

 done:
  return result;
}

static int
circuitmux_compare_muxes_mock(circuitmux_t *cmux_1,
                              circuitmux_t *cmux_2)
{
  int result = 0;

  tt_assert(cmux_1 != NULL);
  tt_assert(cmux_2 != NULL);

  if (cmux_1 != cmux_2) {
    if (cmux_1 == mock_ccm_tgt_1 && cmux_2 == mock_ccm_tgt_2) result = -1;
    else if (cmux_1 == mock_ccm_tgt_2 && cmux_2 == mock_ccm_tgt_1) {
      result = 1;
    } else {
      if (cmux_1 == mock_ccm_tgt_1 || cmux_1 == mock_ccm_tgt_2) result = -1;
      else if (cmux_2 == mock_ccm_tgt_1 || cmux_2 == mock_ccm_tgt_2) {
        result = 1;
      } else {
        result = circuitmux_compare_muxes__real(cmux_1, cmux_2);
      }
    }
  }
  /* else result = 0 always */

 done:
  return result;
}

typedef struct {
  const channel_t *chan;
  ssize_t cells;
} flush_mock_channel_t;

static smartlist_t *chans_for_flush_mock = NULL;

static void
channel_flush_some_cells_mock_free_all(void)
{
  if (chans_for_flush_mock) {
    SMARTLIST_FOREACH_BEGIN(chans_for_flush_mock,
                            flush_mock_channel_t *,
                            flush_mock_ch) {
      SMARTLIST_DEL_CURRENT(chans_for_flush_mock, flush_mock_ch);
      tor_free(flush_mock_ch);
    } SMARTLIST_FOREACH_END(flush_mock_ch);

    smartlist_free(chans_for_flush_mock);
    chans_for_flush_mock = NULL;
  }
}

static void
channel_flush_some_cells_mock_set(channel_t *chan, ssize_t num_cells)
{
  int found = 0;

  if (!chan) return;
  if (num_cells <= 0) return;

  if (!chans_for_flush_mock) {
    chans_for_flush_mock = smartlist_new();
  }

  SMARTLIST_FOREACH_BEGIN(chans_for_flush_mock,
                          flush_mock_channel_t *,
                          flush_mock_ch) {
    if (flush_mock_ch != NULL && flush_mock_ch->chan != NULL) {
      if (flush_mock_ch->chan == chan) {
        /* Found it */
        flush_mock_ch->cells = num_cells;
        found = 1;
        break;
      }
    } else {
      /* That shouldn't be there... */
      SMARTLIST_DEL_CURRENT(chans_for_flush_mock, flush_mock_ch);
      tor_free(flush_mock_ch);
    }
  } SMARTLIST_FOREACH_END(flush_mock_ch);

  if (! found) {
    /* The loop didn't find it */
    flush_mock_channel_t *flush_mock_ch;
    flush_mock_ch = tor_malloc_zero(sizeof(*flush_mock_ch));
    flush_mock_ch->chan = chan;
    flush_mock_ch->cells = num_cells;
    smartlist_add(chans_for_flush_mock, flush_mock_ch);
  }
}

static int
channel_more_to_flush_mock(channel_t *chan)
{
  tor_assert(chan);

  flush_mock_channel_t *found_mock_ch = NULL;

  SMARTLIST_FOREACH_BEGIN(chans_for_flush_mock,
                          flush_mock_channel_t *,
                          flush_mock_ch) {
    if (flush_mock_ch != NULL && flush_mock_ch->chan != NULL) {
      if (flush_mock_ch->chan == chan) {
        /* Found it */
        found_mock_ch = flush_mock_ch;
        break;
      }
    } else {
      /* That shouldn't be there... */
      SMARTLIST_DEL_CURRENT(chans_for_flush_mock, flush_mock_ch);
      tor_free(flush_mock_ch);
    }
  } SMARTLIST_FOREACH_END(flush_mock_ch);

  tor_assert(found_mock_ch);

  /* Check if any circuits would like to queue some */
  /* special for the mock: return the number of cells (instead of 1), or zero
   * if nothing to flush */
  return (found_mock_ch->cells > 0 ? (int)found_mock_ch->cells : 0 );
}

static void
channel_write_to_kernel_mock(channel_t *chan)
{
  (void)chan;
  //log_debug(LD_SCHED, "chan=%d writing to kernel",
  //    (int)chan->global_identifier);
}

static int
channel_should_write_to_kernel_mock(outbuf_table_t *ot, channel_t *chan)
{
  (void)ot;
  (void)chan;
  return 1;
  /* We could make this more complicated if we wanted. But I don't think doing
   * so tests much of anything */
  //static int called_counter = 0;
  //if (++called_counter >= 3) {
  //  called_counter -= 3;
  //  log_debug(LD_SCHED, "chan=%d should write to kernel",
  //      (int)chan->global_identifier);
  //  return 1;
  //}
  //return 0;
}

static ssize_t
channel_flush_some_cells_mock(channel_t *chan, ssize_t num_cells)
{
  ssize_t flushed = 0, max;
  char unlimited = 0;
  flush_mock_channel_t *found = NULL;

  tt_ptr_op(chan, OP_NE, NULL);
  if (chan) {
    if (num_cells < 0) {
      num_cells = 0;
      unlimited = 1;
    }

    /* Check if we have it */
    if (chans_for_flush_mock != NULL) {
      SMARTLIST_FOREACH_BEGIN(chans_for_flush_mock,
                              flush_mock_channel_t *,
                              flush_mock_ch) {
        if (flush_mock_ch != NULL && flush_mock_ch->chan != NULL) {
          if (flush_mock_ch->chan == chan) {
            /* Found it */
            found = flush_mock_ch;
            break;
          }
        } else {
          /* That shouldn't be there... */
          SMARTLIST_DEL_CURRENT(chans_for_flush_mock, flush_mock_ch);
          tor_free(flush_mock_ch);
        }
      } SMARTLIST_FOREACH_END(flush_mock_ch);

      if (found) {
        /* We found one */
        if (found->cells < 0) found->cells = 0;

        if (unlimited) max = found->cells;
        else max = MIN(found->cells, num_cells);

        flushed += max;
        found->cells -= max;
      }
    }
  }

 done:
  return flushed;
}

static void
update_socket_info_impl_mock(socket_table_ent_t *ent)
{
  ent->cwnd = ent->unacked = ent->mss = ent->notsent = 0;
  ent->limit = INT_MAX;
}

static void
perform_channel_state_tests(int KISTSchedRunInterval, int sched_type)
{
  channel_t *ch1 = NULL, *ch2 = NULL;
  int old_count;

  /* setup options so we're sure about what sched we are running */
  MOCK(get_options, mock_get_options);
  clear_options();
  mocked_options.KISTSchedRunInterval = KISTSchedRunInterval;
  set_scheduler_options(sched_type);

  /* Set up scheduler */
  scheduler_init();
  /*
   * Install the compare channels mock so we can test
   * scheduler_touch_channel().
   */
  MOCK(scheduler_compare_channels, scheduler_compare_channels_mock);
  /*
   * Disable scheduler_run so we can just check the state transitions
   * without having to make everything it might call work too.
   */
  ((scheduler_t *) the_scheduler)->run = scheduler_run_noop_mock;

  tt_int_op(smartlist_len(channels_pending), OP_EQ, 0);

  /* Set up a fake channel */
  ch1 = new_fake_channel();
  tt_assert(ch1);

  /* Start it off in OPENING */
  ch1->state = CHANNEL_STATE_OPENING;
  /* Try to register it */
  channel_register(ch1);
  tt_assert(ch1->registered);

  /* It should start off in SCHED_CHAN_IDLE */
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);

  /* Now get another one */
  ch2 = new_fake_channel();
  tt_assert(ch2);
  ch2->state = CHANNEL_STATE_OPENING;
  channel_register(ch2);
  tt_assert(ch2->registered);

  /* Send ch1 to SCHED_CHAN_WAITING_TO_WRITE */
  scheduler_channel_has_waiting_cells(ch1);
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_TO_WRITE);

  /* This should send it to SCHED_CHAN_PENDING */
  scheduler_channel_wants_writes(ch1);
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 1);

  /* Now send ch2 to SCHED_CHAN_WAITING_FOR_CELLS */
  scheduler_channel_wants_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* Drop ch2 back to idle */
  scheduler_channel_doesnt_want_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);

  /* ...and back to SCHED_CHAN_WAITING_FOR_CELLS */
  scheduler_channel_wants_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* ...and this should kick ch2 into SCHED_CHAN_PENDING */
  scheduler_channel_has_waiting_cells(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 2);

  /* This should send ch2 to SCHED_CHAN_WAITING_TO_WRITE */
  scheduler_channel_doesnt_want_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_TO_WRITE);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 1);

  /* ...and back to SCHED_CHAN_PENDING */
  scheduler_channel_wants_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 2);

  /* Now we exercise scheduler_touch_channel */
  old_count = scheduler_compare_channels_mock_ctr;
  scheduler_touch_channel(ch1);
  tt_assert(scheduler_compare_channels_mock_ctr > old_count);

  /* Release the ch2 and then do it another time to make sure it doesn't blow
   * up and we are still in a quiescent state. */
  scheduler_release_channel(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 1);
  /* Cheat a bit so make the release more confused but also will tells us if
   * the release did put the channel in the right state. */
  ch2->scheduler_state = SCHED_CHAN_PENDING;
  scheduler_release_channel(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 1);

  /* Close */
  channel_mark_for_close(ch1);
  tt_int_op(ch1->state, OP_EQ, CHANNEL_STATE_CLOSING);
  channel_mark_for_close(ch2);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_CLOSING);
  channel_closed(ch1);
  tt_int_op(ch1->state, OP_EQ, CHANNEL_STATE_CLOSED);
  ch1 = NULL;
  channel_closed(ch2);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_CLOSED);
  ch2 = NULL;

  /* Shut things down */

  channel_free_all();
  scheduler_free_all();

 done:
  tor_free(ch1);
  tor_free(ch2);

  UNMOCK(scheduler_compare_channels);
  UNMOCK(get_options);
  cleanup_scheduler_options();

  return;
}

static void
test_scheduler_compare_channels(void *arg)
{
  /* We don't actually need whole fake channels... */
  channel_t c1, c2;
  /* ...and some dummy circuitmuxes too */
  circuitmux_t *cm1 = NULL, *cm2 = NULL;
  int result;

  (void)arg;

  /* We can't actually see sizeof(circuitmux_t) from here */
  cm1 = tor_malloc_zero(sizeof(void *));
  cm2 = tor_malloc_zero(sizeof(void *));

  c1.cmux = cm1;
  c2.cmux = cm2;

  /* Configure circuitmux_get_policy() mock */
  mock_cgp_tgt_1 = cm1;
  mock_cgp_tgt_2 = cm2;

  /*
   * This is to test the different-policies case, which uses the policy
   * cast to an uintptr_t as an arbitrary but definite thing to compare.
   */
  mock_cgp_val_1 = tor_malloc_zero(16);
  mock_cgp_val_2 = tor_malloc_zero(16);
  if ( ((uintptr_t) mock_cgp_val_1) > ((uintptr_t) mock_cgp_val_2) ) {
    void *tmp = mock_cgp_val_1;
    mock_cgp_val_1 = mock_cgp_val_2;
    mock_cgp_val_2 = tmp;
  }

  MOCK(circuitmux_get_policy, circuitmux_get_policy_mock);

  /* Now set up circuitmux_compare_muxes() mock using cm1/cm2 */
  mock_ccm_tgt_1 = cm1;
  mock_ccm_tgt_2 = cm2;
  MOCK(circuitmux_compare_muxes, circuitmux_compare_muxes_mock);

  /* Equal-channel case */
  result = scheduler_compare_channels(&c1, &c1);
  tt_int_op(result, OP_EQ, 0);

  /* Distinct channels, distinct policies */
  result = scheduler_compare_channels(&c1, &c2);
  tt_int_op(result, OP_EQ, -1);
  result = scheduler_compare_channels(&c2, &c1);
  tt_int_op(result, OP_EQ, 1);

  /* Distinct channels, same policy */
  tor_free(mock_cgp_val_2);
  mock_cgp_val_2 = mock_cgp_val_1;
  result = scheduler_compare_channels(&c1, &c2);
  tt_int_op(result, OP_EQ, -1);
  result = scheduler_compare_channels(&c2, &c1);
  tt_int_op(result, OP_EQ, 1);

 done:

  UNMOCK(circuitmux_compare_muxes);
  mock_ccm_tgt_1 = NULL;
  mock_ccm_tgt_2 = NULL;

  UNMOCK(circuitmux_get_policy);
  mock_cgp_tgt_1 = NULL;
  mock_cgp_tgt_2 = NULL;

  tor_free(cm1);
  tor_free(cm2);

  if (mock_cgp_val_1 != mock_cgp_val_2)
    tor_free(mock_cgp_val_1);
  tor_free(mock_cgp_val_2);
  mock_cgp_val_1 = NULL;
  mock_cgp_val_2 = NULL;

  return;
}

/******************************************************************************
 * The actual tests!
 *****************************************************************************/

static void
test_scheduler_loop_vanilla(void *arg)
{
  (void)arg;
  channel_t *ch1 = NULL, *ch2 = NULL;
  void (*run_func_ptr)(void);

  /* setup options so we're sure about what sched we are running */
  MOCK(get_options, mock_get_options);
  clear_options();
  set_scheduler_options(SCHEDULER_VANILLA);
  mocked_options.KISTSchedRunInterval = 0;

  /* Set up scheduler */
  scheduler_init();
  /*
   * Install the compare channels mock so we can test
   * scheduler_touch_channel().
   */
  MOCK(scheduler_compare_channels, scheduler_compare_channels_mock);
  /*
   * Disable scheduler_run so we can just check the state transitions
   * without having to make everything it might call work too.
   */
  run_func_ptr = the_scheduler->run;
  ((scheduler_t *) the_scheduler)->run = scheduler_run_noop_mock;

  tt_int_op(smartlist_len(channels_pending), OP_EQ, 0);

  /* Set up a fake channel */
  ch1 = new_fake_channel();
  ch1->magic = TLS_CHAN_MAGIC;
  tt_assert(ch1);

  /* Start it off in OPENING */
  ch1->state = CHANNEL_STATE_OPENING;
  /* Try to register it */
  channel_register(ch1);
  tt_assert(ch1->registered);
  /* Finish opening it */
  channel_change_state_open(ch1);

  /* It should start off in SCHED_CHAN_IDLE */
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);

  /* Now get another one */
  ch2 = new_fake_channel();
  ch2->magic = TLS_CHAN_MAGIC;
  tt_assert(ch2);
  ch2->state = CHANNEL_STATE_OPENING;
  channel_register(ch2);
  tt_assert(ch2->registered);
  /*
   * Don't open ch2; then channel_num_cells_writeable() will return
   * zero and we'll get coverage of that exception case in scheduler_run()
   */

  tt_int_op(ch1->state, OP_EQ, CHANNEL_STATE_OPEN);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_OPENING);

  /* Send it to SCHED_CHAN_WAITING_TO_WRITE */
  scheduler_channel_has_waiting_cells(ch1);
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_TO_WRITE);

  /* This should send it to SCHED_CHAN_PENDING */
  scheduler_channel_wants_writes(ch1);
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 1);

  /* Now send ch2 to SCHED_CHAN_WAITING_FOR_CELLS */
  scheduler_channel_wants_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* Drop ch2 back to idle */
  scheduler_channel_doesnt_want_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);

  /* ...and back to SCHED_CHAN_WAITING_FOR_CELLS */
  scheduler_channel_wants_writes(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* ...and this should kick ch2 into SCHED_CHAN_PENDING */
  scheduler_channel_has_waiting_cells(ch2);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 2);

  /*
   * Now we've got two pending channels and need to fire off
   * the scheduler run() that we kept.
   */
  run_func_ptr();

  /*
   * Assert that they're still in the states we left and aren't still
   * pending
   */
  tt_int_op(ch1->state, OP_EQ, CHANNEL_STATE_OPEN);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_OPENING);
  tt_assert(ch1->scheduler_state != SCHED_CHAN_PENDING);
  tt_assert(ch2->scheduler_state != SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 0);

  /* Now, finish opening ch2, and get both back to pending */
  channel_change_state_open(ch2);
  scheduler_channel_wants_writes(ch1);
  scheduler_channel_wants_writes(ch2);
  scheduler_channel_has_waiting_cells(ch1);
  scheduler_channel_has_waiting_cells(ch2);
  tt_int_op(ch1->state, OP_EQ, CHANNEL_STATE_OPEN);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_OPEN);
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(channels_pending), OP_EQ, 2);

  /* Now, set up the channel_flush_some_cells() mock */
  MOCK(channel_flush_some_cells, channel_flush_some_cells_mock);
  /*
   * 16 cells on ch1 means it'll completely drain into the 32 cells
   * fakechan's num_cells_writeable() returns.
   */
  channel_flush_some_cells_mock_set(ch1, 16);
  /*
   * This one should get sent back to pending, since num_cells_writeable()
   * will still return non-zero.
   */
  channel_flush_some_cells_mock_set(ch2, 48);

  /*
   * And re-run the scheduler run() loop with non-zero returns from
   * channel_flush_some_cells() this time.
   */
  run_func_ptr();

  /*
   * ch1 should have gone to SCHED_CHAN_WAITING_FOR_CELLS, with 16 flushed
   * and 32 writeable.
   */
  tt_int_op(ch1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);
  /*
   * ...ch2 should also have gone to SCHED_CHAN_WAITING_FOR_CELLS, with
   * channel_more_to_flush() returning false and channel_num_cells_writeable()
   * > 0/
   */
  tt_int_op(ch2->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* Close */
  channel_mark_for_close(ch1);
  tt_int_op(ch1->state, OP_EQ, CHANNEL_STATE_CLOSING);
  channel_mark_for_close(ch2);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_CLOSING);
  channel_closed(ch1);
  tt_int_op(ch1->state, OP_EQ, CHANNEL_STATE_CLOSED);
  ch1 = NULL;
  channel_closed(ch2);
  tt_int_op(ch2->state, OP_EQ, CHANNEL_STATE_CLOSED);
  ch2 = NULL;

  /* Shut things down */
  channel_flush_some_cells_mock_free_all();
  channel_free_all();
  scheduler_free_all();

 done:
  tor_free(ch1);
  tor_free(ch2);
  cleanup_scheduler_options();

  UNMOCK(channel_flush_some_cells);
  UNMOCK(scheduler_compare_channels);
  UNMOCK(get_options);
}

static void
test_scheduler_loop_kist(void *arg)
{
  (void) arg;

#ifndef HAVE_KIST_SUPPORT
  return;
#endif

  channel_t *ch1 = new_fake_channel(), *ch2 = new_fake_channel();
  channel_t *ch3 = new_fake_channel();

  /* setup options so we're sure about what sched we are running */
  MOCK(get_options, mock_get_options);
  MOCK(channel_flush_some_cells, channel_flush_some_cells_mock);
  MOCK(channel_more_to_flush, channel_more_to_flush_mock);
  MOCK(channel_write_to_kernel, channel_write_to_kernel_mock);
  MOCK(channel_should_write_to_kernel, channel_should_write_to_kernel_mock);
  MOCK(update_socket_info_impl, update_socket_info_impl_mock);
  clear_options();
  mocked_options.KISTSchedRunInterval = 11;
  set_scheduler_options(SCHEDULER_KIST);
  scheduler_init();

  tt_assert(ch1);
  ch1->magic = TLS_CHAN_MAGIC;
  ch1->state = CHANNEL_STATE_OPENING;
  channel_register(ch1);
  tt_assert(ch1->registered);
  channel_change_state_open(ch1);
  scheduler_channel_has_waiting_cells(ch1);
  scheduler_channel_wants_writes(ch1);
  channel_flush_some_cells_mock_set(ch1, 5);

  tt_assert(ch2);
  ch2->magic = TLS_CHAN_MAGIC;
  ch2->state = CHANNEL_STATE_OPENING;
  channel_register(ch2);
  tt_assert(ch2->registered);
  channel_change_state_open(ch2);
  scheduler_channel_has_waiting_cells(ch2);
  scheduler_channel_wants_writes(ch2);
  channel_flush_some_cells_mock_set(ch2, 5);

  the_scheduler->run();

  scheduler_channel_has_waiting_cells(ch1);
  channel_flush_some_cells_mock_set(ch1, 5);

  the_scheduler->run();

  scheduler_channel_has_waiting_cells(ch1);
  channel_flush_some_cells_mock_set(ch1, 5);
  scheduler_channel_has_waiting_cells(ch2);
  channel_flush_some_cells_mock_set(ch2, 5);

  the_scheduler->run();

  channel_flush_some_cells_mock_free_all();

  /* We'll try to run this closed channel threw the scheduler loop and make
   * sure it ends up in the right state. */
  tt_assert(ch3);
  ch3->magic = TLS_CHAN_MAGIC;
  ch3->state = CHANNEL_STATE_OPEN;
  circuitmux_free(ch3->cmux);
  ch3->cmux = circuitmux_alloc();
  channel_register(ch3);
  tt_assert(ch3->registered);

  ch3->scheduler_state = SCHED_CHAN_WAITING_FOR_CELLS;
  scheduler_channel_has_waiting_cells(ch3);
  /* Should be in the pending list now waiting to be handled. */
  tt_int_op(ch3->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);
  /* By running the scheduler on a closed channel, it should end up in the
   * IDLE state and not in the pending channel list. */
  ch3->state = CHANNEL_STATE_CLOSED;
  the_scheduler->run();
  tt_int_op(ch3->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);

 done:
  /* Prep the channel so the free() function doesn't explode. */
  ch1->state = ch2->state = ch3->state = CHANNEL_STATE_CLOSED;
  ch1->registered = ch2->registered = ch3->registered = 0;
  channel_free(ch1);
  channel_free(ch2);
  channel_free(ch3);
  UNMOCK(update_socket_info_impl);
  UNMOCK(channel_should_write_to_kernel);
  UNMOCK(channel_write_to_kernel);
  UNMOCK(channel_more_to_flush);
  UNMOCK(channel_flush_some_cells);
  UNMOCK(get_options);
  scheduler_free_all();
  return;
}

static void
test_scheduler_channel_states(void *arg)
{
  (void)arg;
  perform_channel_state_tests(-1, SCHEDULER_VANILLA);
  perform_channel_state_tests(11, SCHEDULER_KIST_LITE);
#ifdef HAVE_KIST_SUPPORT
  perform_channel_state_tests(11, SCHEDULER_KIST);
#endif
}

static void
test_scheduler_initfree(void *arg)
{
  (void)arg;

  tt_ptr_op(channels_pending, OP_EQ, NULL);
  tt_ptr_op(run_sched_ev, OP_EQ, NULL);

  MOCK(get_options, mock_get_options);
  set_scheduler_options(SCHEDULER_KIST);
  set_scheduler_options(SCHEDULER_KIST_LITE);
  set_scheduler_options(SCHEDULER_VANILLA);

  scheduler_init();

  tt_ptr_op(channels_pending, OP_NE, NULL);
  tt_ptr_op(run_sched_ev, OP_NE, NULL);
  /* We have specified nothing in the torrc and there's no consensus so the
   * KIST scheduler is what should be in use */
  tt_ptr_op(the_scheduler, OP_EQ, get_kist_scheduler());
  tt_int_op(sched_run_interval, OP_EQ, 10);

  scheduler_free_all();

  tt_ptr_op(channels_pending, OP_EQ, NULL);
  tt_ptr_op(run_sched_ev, OP_EQ, NULL);

 done:
  UNMOCK(get_options);
  cleanup_scheduler_options();
  return;
}

static void
test_scheduler_can_use_kist(void *arg)
{
  (void)arg;

  int res_should, res_freq;
  MOCK(get_options, mock_get_options);

  /* Test force enabling of KIST */
  clear_options();
  mocked_options.KISTSchedRunInterval = 1234;
  res_should = scheduler_can_use_kist();
  res_freq = kist_scheduler_run_interval();
#ifdef HAVE_KIST_SUPPORT
  tt_int_op(res_should, OP_EQ, 1);
#else /* HAVE_KIST_SUPPORT */
  tt_int_op(res_should, OP_EQ, 0);
#endif /* HAVE_KIST_SUPPORT */
  tt_int_op(res_freq, OP_EQ, 1234);

  /* Test defer to consensus, but no consensus available */
  clear_options();
  mocked_options.KISTSchedRunInterval = 0;
  res_should = scheduler_can_use_kist();
  res_freq = kist_scheduler_run_interval();
#ifdef HAVE_KIST_SUPPORT
  tt_int_op(res_should, OP_EQ, 1);
#else /* HAVE_KIST_SUPPORT */
  tt_int_op(res_should, OP_EQ, 0);
#endif /* HAVE_KIST_SUPPORT */
  tt_int_op(res_freq, OP_EQ, 10);

  /* Test defer to consensus, and kist consensus available */
  MOCK(networkstatus_get_param, mock_kist_networkstatus_get_param);
  clear_options();
  mocked_options.KISTSchedRunInterval = 0;
  res_should = scheduler_can_use_kist();
  res_freq = kist_scheduler_run_interval();
#ifdef HAVE_KIST_SUPPORT
  tt_int_op(res_should, OP_EQ, 1);
#else /* HAVE_KIST_SUPPORT */
  tt_int_op(res_should, OP_EQ, 0);
#endif /* HAVE_KIST_SUPPORT */
  tt_int_op(res_freq, OP_EQ, 12);
  UNMOCK(networkstatus_get_param);

  /* Test defer to consensus, and vanilla consensus available */
  MOCK(networkstatus_get_param, mock_vanilla_networkstatus_get_param);
  clear_options();
  mocked_options.KISTSchedRunInterval = 0;
  res_should = scheduler_can_use_kist();
  res_freq = kist_scheduler_run_interval();
  tt_int_op(res_should, OP_EQ, 0);
  tt_int_op(res_freq, OP_EQ, 0);
  UNMOCK(networkstatus_get_param);

 done:
  UNMOCK(get_options);
  return;
}

static void
test_scheduler_ns_changed(void *arg)
{
  (void) arg;

  /*
   * Currently no scheduler implementations use the old/new consensuses passed
   * in scheduler_notify_networkstatus_changed, so it is okay to pass NULL.
   *
   * "But then what does test actually exercise???" It tests that
   * scheduler_notify_networkstatus_changed fetches the correct value from the
   * consensus, and then switches the scheduler if necessasry.
   */

  MOCK(get_options, mock_get_options);
  clear_options();
  set_scheduler_options(SCHEDULER_KIST);
  set_scheduler_options(SCHEDULER_VANILLA);

  tt_ptr_op(the_scheduler, OP_EQ, NULL);

  /* Change from vanilla to kist via consensus */
  the_scheduler = get_vanilla_scheduler();
  MOCK(networkstatus_get_param, mock_kist_networkstatus_get_param);
  scheduler_notify_networkstatus_changed();
  UNMOCK(networkstatus_get_param);
#ifdef HAVE_KIST_SUPPORT
  tt_ptr_op(the_scheduler, OP_EQ, get_kist_scheduler());
#else
  tt_ptr_op(the_scheduler, OP_EQ, get_vanilla_scheduler());
#endif

  /* Change from kist to vanilla via consensus */
  the_scheduler = get_kist_scheduler();
  MOCK(networkstatus_get_param, mock_vanilla_networkstatus_get_param);
  scheduler_notify_networkstatus_changed();
  UNMOCK(networkstatus_get_param);
  tt_ptr_op(the_scheduler, OP_EQ, get_vanilla_scheduler());

  /* Doesn't change when using KIST */
  the_scheduler = get_kist_scheduler();
  MOCK(networkstatus_get_param, mock_kist_networkstatus_get_param);
  scheduler_notify_networkstatus_changed();
  UNMOCK(networkstatus_get_param);
#ifdef HAVE_KIST_SUPPORT
  tt_ptr_op(the_scheduler, OP_EQ, get_kist_scheduler());
#else
  tt_ptr_op(the_scheduler, OP_EQ, get_vanilla_scheduler());
#endif

  /* Doesn't change when using vanilla */
  the_scheduler = get_vanilla_scheduler();
  MOCK(networkstatus_get_param, mock_vanilla_networkstatus_get_param);
  scheduler_notify_networkstatus_changed();
  UNMOCK(networkstatus_get_param);
  tt_ptr_op(the_scheduler, OP_EQ, get_vanilla_scheduler());

 done:
  UNMOCK(get_options);
  cleanup_scheduler_options();
  return;
}

/*
 * Mocked functions for the kist_pending_list test.
 */

static int mock_flush_some_cells_num = 1;
static int mock_more_to_flush = 0;
static int mock_update_socket_info_limit = 0;

static ssize_t
channel_flush_some_cells_mock_var(channel_t *chan, ssize_t num_cells)
{
  (void) chan;
  (void) num_cells;
  return mock_flush_some_cells_num;
}

/* Because when we flush cells, it is possible that the connection outbuf gets
 * fully drained, the wants to write scheduler event is fired back while we
 * are in the scheduler loop so this mock function does it for us.
 * Furthermore, the socket limit is set to 0 so once this is triggered, it
 * informs the scheduler that it can't write on the socket anymore. */
static void
channel_write_to_kernel_mock_trigger_24700(channel_t *chan)
{
  static int chan_id_seen[2] = {0};
  if (++chan_id_seen[chan->global_identifier - 1] > 1) {
    tt_assert(0);
  }

  scheduler_channel_wants_writes(chan);

 done:
  return;
}

static int
channel_more_to_flush_mock_var(channel_t *chan)
{
  (void) chan;
  return mock_more_to_flush;
}

static void
update_socket_info_impl_mock_var(socket_table_ent_t *ent)
{
  ent->cwnd = ent->unacked = ent->mss = ent->notsent = 0;
  ent->limit = mock_update_socket_info_limit;
}

static void
test_scheduler_kist_pending_list(void *arg)
{
  (void) arg;

#ifndef HAVE_KIST_SUPPORT
  return;
#endif

  /* This is for testing the channel flow with the pending list that is
   * depending on the channel state, what will be the expected behavior of the
   * scheduler with that list.
   *
   * For instance, we want to catch double channel add or removing a channel
   * that doesn't exists, or putting a channel in the list in a wrong state.
   * Essentially, this will articifically test cases of the KIST main loop and
   * entry point in the channel subsystem.
   *
   * In part, this is to also catch things like #24700 and provide a test bed
   * for more testing in the future like so. */

  /* Mocking a series of scheduler function to control the flow of the
   * scheduler loop to test every use cases and assess the pending list. */
  MOCK(get_options, mock_get_options);
  MOCK(channel_flush_some_cells, channel_flush_some_cells_mock_var);
  MOCK(channel_more_to_flush, channel_more_to_flush_mock_var);
  MOCK(update_socket_info_impl, update_socket_info_impl_mock_var);
  MOCK(channel_write_to_kernel, channel_write_to_kernel_mock);
  MOCK(channel_should_write_to_kernel, channel_should_write_to_kernel_mock);

  /* Setup options so we're sure about what sched we are running */
  mocked_options.KISTSchedRunInterval = 10;
  set_scheduler_options(SCHEDULER_KIST);

  /* Init scheduler. */
  scheduler_init();

  /* Initialize a channel. We'll need a second channel for the #24700 bug
   * test. */
  channel_t *chan1 = new_fake_channel();
  channel_t *chan2 = new_fake_channel();
  tt_assert(chan1);
  tt_assert(chan2);
  chan1->magic = chan2->magic = TLS_CHAN_MAGIC;
  channel_register(chan1);
  channel_register(chan2);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);
  tt_int_op(chan1->sched_heap_idx, OP_EQ, -1);
  tt_int_op(chan2->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);
  tt_int_op(chan2->sched_heap_idx, OP_EQ, -1);

  /* Once a channel becomes OPEN, it always have at least one cell in it so
   * the scheduler is notified that the channel wants to write so this is the
   * first step. Might not make sense to you but it is the way it is. */
  scheduler_channel_wants_writes(chan1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);

  /* Signal the scheduler that it has waiting cells which means the channel
   * will get scheduled. */
  scheduler_channel_has_waiting_cells(chan1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);
  /* Subsequent call should not add it more times. It is possible we add many
   * cells in rapid succession before the channel is scheduled. */
  scheduler_channel_has_waiting_cells(chan1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);
  scheduler_channel_has_waiting_cells(chan1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);

  /* We'll flush one cell and make it that the socket can write but no more to
   * flush else we end up in an infinite loop. We expect the channel to be put
   * in waiting for cells state and the pending list empty. */
  mock_update_socket_info_limit = INT_MAX;
  mock_more_to_flush = 0;
  the_scheduler->run();
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* Lets make believe that a cell is now in the channel but this time the
   * channel can't write so obviously it has more to flush. We expect the
   * channel to be back in the pending list. */
  scheduler_channel_has_waiting_cells(chan1);
  mock_update_socket_info_limit = 0;
  mock_more_to_flush = 1;
  the_scheduler->run();
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);

  /* Channel is in the pending list now, during that time, we'll trigger a
   * wants to write event because maybe the channel buffers were emptied in
   * the meantime. This is possible because once the connection outbuf is
   * flushed down the low watermark, the scheduler is notified.
   *
   * We expect the channel to NOT be added in the pending list again and stay
   * in PENDING state. */
  scheduler_channel_wants_writes(chan1);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);

  /* Make it that the channel can write now but has nothing else to flush. We
   * expect that it is removed from the pending list and waiting for cells. */
  mock_update_socket_info_limit = INT_MAX;
  mock_more_to_flush = 0;
  the_scheduler->run();
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* While waiting for cells, lets say we were able to write more things on
   * the connection outbuf (unlikely that this can happen but let say it
   * does). We expect the channel to stay in waiting for cells. */
  scheduler_channel_wants_writes(chan1);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* We'll not put it in the pending list and make the flush cell fail with 0
   * cell flushed. We expect that it is put back in waiting for cells. */
  scheduler_channel_has_waiting_cells(chan1);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  mock_flush_some_cells_num = 0;
  the_scheduler->run();
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_FOR_CELLS);

  /* Set the channel to a state where it doesn't want to write more. We expect
   * that the channel becomes idle. */
  scheduler_channel_doesnt_want_writes(chan1);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_IDLE);

  /* Some cells arrive on the channel now. We expect it to go back in waiting
   * to write. You might wonder why it is not put in the pending list? Because
   * once the channel becomes OPEN again (the doesn't want to write event only
   * occurs if the channel goes in MAINT mode), if there are cells in the
   * channel, the wants to write event is triggered thus putting the channel
   * in pending mode.
   *
   * Else, if no cells, it stays IDLE and then once a cell comes in, it should
   * go in waiting to write which is a BUG itself because the channel can't be
   * scheduled until a second cell comes in. Hopefully, #24554 will fix that
   * for KIST. */
  scheduler_channel_has_waiting_cells(chan1);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_TO_WRITE);

  /* Second cell comes in, unfortunately, it won't get scheduled until a wants
   * to write event occurs like described above. */
  scheduler_channel_has_waiting_cells(chan1);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 0);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_WAITING_TO_WRITE);

  /* Unblock everything putting the channel in the pending list. */
  scheduler_channel_wants_writes(chan1);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 1);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);

  /* Testing bug #24700 which is the situation where we have at least two
   * different channels in the pending list. The first one gets flushed and
   * bytes are written on the wire which triggers a wants to write event
   * because the outbuf is below the low watermark. The bug was that this
   * exact channel was added back in the pending list because its state wasn't
   * PENDING.
   *
   * The following does some ninja-tsu to try to make it happen. We need two
   * different channels so we create a second one and add it to the pending
   * list. Then, we have a custom function when we write to kernel that does
   * two important things:
   *
   *  1) Calls scheduler_channel_wants_writes(chan) on the channel.
   *  2) Keeps track of how many times it sees the channel going through. If
   *     that limit goes > 1, it means we've added the channel twice in the
   *     pending list.
   *
   * In the end, we expect both channels to be in the pending list after this
   * scheduler run. */

  /* Put the second channel in the pending list. */
  scheduler_channel_wants_writes(chan2);
  scheduler_channel_has_waiting_cells(chan2);
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 2);
  tt_int_op(chan2->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);

  /* This makes it that the first pass on socket_can_write() will be true but
   * then when a single cell is flushed (514 + 29 bytes), the second call to
   * socket_can_write() will be false. If it wasn't sending back false on the
   * second run, we end up in an infinite loop of the scheduler. */
  mock_update_socket_info_limit = 600;
  /* We want to hit "Case 3:" of the scheduler so channel_more_to_flush() is
   * true but socket_can_write() has to be false on the second check on the
   * channel. */
  mock_more_to_flush = 1;
  mock_flush_some_cells_num = 1;
  MOCK(channel_write_to_kernel, channel_write_to_kernel_mock_trigger_24700);
  the_scheduler->run();
  tt_int_op(smartlist_len(get_channels_pending()), OP_EQ, 2);
  tt_int_op(chan1->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);
  tt_int_op(chan2->scheduler_state, OP_EQ, SCHED_CHAN_PENDING);

 done:
  chan1->state = chan2->state = CHANNEL_STATE_CLOSED;
  chan1->registered = chan2->registered = 0;
  channel_free(chan1);
  channel_free(chan2);
  scheduler_free_all();

  UNMOCK(get_options);
  UNMOCK(channel_flush_some_cells);
  UNMOCK(channel_more_to_flush);
  UNMOCK(update_socket_info_impl);
  UNMOCK(channel_write_to_kernel);
  UNMOCK(channel_should_write_to_kernel);
}

struct testcase_t scheduler_tests[] = {
  { "compare_channels", test_scheduler_compare_channels,
    TT_FORK, NULL, NULL },
  { "channel_states", test_scheduler_channel_states, TT_FORK, NULL, NULL },
  { "initfree", test_scheduler_initfree, TT_FORK, NULL, NULL },
  { "loop_vanilla", test_scheduler_loop_vanilla, TT_FORK, NULL, NULL },
  { "loop_kist", test_scheduler_loop_kist, TT_FORK, NULL, NULL },
  { "ns_changed", test_scheduler_ns_changed, TT_FORK, NULL, NULL},
  { "should_use_kist", test_scheduler_can_use_kist, TT_FORK, NULL, NULL },
  { "kist_pending_list", test_scheduler_kist_pending_list, TT_FORK,
    NULL, NULL },
  END_OF_TESTCASES
};

