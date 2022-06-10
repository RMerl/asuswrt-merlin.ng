/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file scheduler_kist.c
 * @brief Implements the KIST cell scheduler.
 **/

#define SCHEDULER_KIST_PRIVATE

#include "core/or/or.h"
#include "lib/buf/buffers.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "feature/nodelist/networkstatus.h"
#define CHANNEL_OBJECT_PRIVATE
#include "core/or/channel.h"
#include "core/or/channeltls.h"
#define SCHEDULER_PRIVATE
#include "core/or/scheduler.h"
#include "lib/math/fp.h"

#include "core/or/or_connection_st.h"

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_KIST_SUPPORT
/* Kernel interface needed for KIST. */
#include <netinet/tcp.h>
#include <linux/sockios.h>
#endif /* HAVE_KIST_SUPPORT */

/*****************************************************************************
 * Data structures and supporting functions
 *****************************************************************************/

/* Socket_table hash table stuff. The socket_table keeps track of per-socket
 * limit information imposed by kist and used by kist. */

static uint32_t
socket_table_ent_hash(const socket_table_ent_t *ent)
{
  return (uint32_t)ent->chan->global_identifier;
}

static unsigned
socket_table_ent_eq(const socket_table_ent_t *a, const socket_table_ent_t *b)
{
  return a->chan == b->chan;
}

typedef HT_HEAD(socket_table_s, socket_table_ent_t) socket_table_t;

static socket_table_t socket_table = HT_INITIALIZER();

HT_PROTOTYPE(socket_table_s, socket_table_ent_t, node, socket_table_ent_hash,
             socket_table_ent_eq);
HT_GENERATE2(socket_table_s, socket_table_ent_t, node, socket_table_ent_hash,
             socket_table_ent_eq, 0.6, tor_reallocarray, tor_free_);

/* outbuf_table hash table stuff. The outbuf_table keeps track of which
 * channels have data sitting in their outbuf so the kist scheduler can force
 * a write from outbuf to kernel periodically during a run and at the end of a
 * run. */

typedef struct outbuf_table_ent_t {
  HT_ENTRY(outbuf_table_ent_t) node;
  channel_t *chan;
} outbuf_table_ent_t;

static uint32_t
outbuf_table_ent_hash(const outbuf_table_ent_t *ent)
{
  return (uint32_t)ent->chan->global_identifier;
}

static unsigned
outbuf_table_ent_eq(const outbuf_table_ent_t *a, const outbuf_table_ent_t *b)
{
  return a->chan->global_identifier == b->chan->global_identifier;
}

HT_PROTOTYPE(outbuf_table_s, outbuf_table_ent_t, node, outbuf_table_ent_hash,
             outbuf_table_ent_eq);
HT_GENERATE2(outbuf_table_s, outbuf_table_ent_t, node, outbuf_table_ent_hash,
             outbuf_table_ent_eq, 0.6, tor_reallocarray, tor_free_);

/*****************************************************************************
 * Other internal data
 *****************************************************************************/

/* Store the last time the scheduler was run so we can decide when to next run
 * the scheduler based on it. */
static monotime_t scheduler_last_run;
/* This is a factor for the extra_space calculation in kist per-socket limits.
 * It is the number of extra congestion windows we want to write to the kernel.
 */
static double sock_buf_size_factor = 1.0;
/* How often the scheduler runs. */
STATIC int sched_run_interval = KIST_SCHED_RUN_INTERVAL_DEFAULT;

#ifdef HAVE_KIST_SUPPORT
/* Indicate if KIST lite mode is on or off. We can disable it at runtime.
 * Important to have because of the KISTLite -> KIST possible transition. */
static unsigned int kist_lite_mode = 0;
/* Indicate if we don't have the kernel support. This can happen if the kernel
 * changed and it doesn't recognized the values passed to the syscalls needed
 * by KIST. In that case, fallback to the naive approach. */
static unsigned int kist_no_kernel_support = 0;
#else /* !defined(HAVE_KIST_SUPPORT) */
static unsigned int kist_lite_mode = 1;
#endif /* defined(HAVE_KIST_SUPPORT) */

/*****************************************************************************
 * Internally called function implementations
 *****************************************************************************/

/* Little helper function to get the length of a channel's output buffer */
static inline size_t
channel_outbuf_length(channel_t *chan)
{
  tor_assert(chan);
  /* In theory, this can not happen because we can not scheduler a channel
   * without a connection that has its outbuf initialized. Just in case, bug
   * on this so we can understand a bit more why it happened. */
  if (SCHED_BUG(BASE_CHAN_TO_TLS(chan)->conn == NULL, chan)) {
    return 0;
  }
  return buf_datalen(TO_CONN(BASE_CHAN_TO_TLS(chan)->conn)->outbuf);
}

/* Little helper function for HT_FOREACH_FN. */
static int
each_channel_write_to_kernel(outbuf_table_ent_t *ent, void *data)
{
  (void) data; /* Make compiler happy. */
  channel_write_to_kernel(ent->chan);
  return 0; /* Returning non-zero removes the element from the table. */
}

/* Free the given outbuf table entry ent. */
static int
free_outbuf_info_by_ent(outbuf_table_ent_t *ent, void *data)
{
  (void) data; /* Make compiler happy. */
  log_debug(LD_SCHED, "Freeing outbuf table entry from chan=%" PRIu64,
            ent->chan->global_identifier);
  tor_free(ent);
  return 1; /* So HT_FOREACH_FN will remove the element */
}

/* Free the given socket table entry ent. */
static int
free_socket_info_by_ent(socket_table_ent_t *ent, void *data)
{
  (void) data; /* Make compiler happy. */
  log_debug(LD_SCHED, "Freeing socket table entry from chan=%" PRIu64,
            ent->chan->global_identifier);
  tor_free(ent);
  return 1; /* So HT_FOREACH_FN will remove the element */
}

/* Clean up socket_table. Probably because the KIST sched impl is going away */
static void
free_all_socket_info(void)
{
  HT_FOREACH_FN(socket_table_s, &socket_table, free_socket_info_by_ent, NULL);
  HT_CLEAR(socket_table_s, &socket_table);
}

static socket_table_ent_t *
socket_table_search(socket_table_t *table, const channel_t *chan)
{
  socket_table_ent_t search, *ent = NULL;
  search.chan = chan;
  ent = HT_FIND(socket_table_s, table, &search);
  return ent;
}

/* Free a socket entry in table for the given chan. */
static void
free_socket_info_by_chan(socket_table_t *table, const channel_t *chan)
{
  socket_table_ent_t *ent = NULL;
  ent = socket_table_search(table, chan);
  if (!ent)
    return;
  log_debug(LD_SCHED, "scheduler free socket info for chan=%" PRIu64,
            chan->global_identifier);
  HT_REMOVE(socket_table_s, table, ent);
  free_socket_info_by_ent(ent, NULL);
}

/* Perform system calls for the given socket in order to calculate kist's
 * per-socket limit as documented in the function body. */
MOCK_IMPL(void,
update_socket_info_impl, (socket_table_ent_t *ent))
{
#ifdef HAVE_KIST_SUPPORT
  int64_t tcp_space, extra_space;
  tor_assert(ent);
  tor_assert(ent->chan);
  const tor_socket_t sock =
    TO_CONN(CONST_BASE_CHAN_TO_TLS(ent->chan)->conn)->s;
  struct tcp_info tcp;
  socklen_t tcp_info_len = sizeof(tcp);

  if (kist_no_kernel_support || kist_lite_mode) {
    goto fallback;
  }

  /* Gather information */
  if (getsockopt(sock, SOL_TCP, TCP_INFO, (void *)&(tcp), &tcp_info_len) < 0) {
    if (errno == EINVAL) {
      /* Oops, this option is not provided by the kernel, we'll have to
       * disable KIST entirely. This can happen if tor was built on a machine
       * with the support previously or if the kernel was updated and lost the
       * support. */
      log_notice(LD_SCHED, "Looks like our kernel doesn't have the support "
                           "for KIST anymore. We will fallback to the naive "
                           "approach. Remove KIST from the Schedulers list "
                           "to disable.");
      kist_no_kernel_support = 1;
    }
    goto fallback;
  }
  if (ioctl(sock, SIOCOUTQNSD, &(ent->notsent)) < 0) {
    if (errno == EINVAL) {
      log_notice(LD_SCHED, "Looks like our kernel doesn't have the support "
                           "for KIST anymore. We will fallback to the naive "
                           "approach. Remove KIST from the Schedulers list "
                           "to disable.");
      /* Same reason as the above. */
      kist_no_kernel_support = 1;
    }
    goto fallback;
  }
  ent->cwnd = tcp.tcpi_snd_cwnd;
  ent->unacked = tcp.tcpi_unacked;
  ent->mss = tcp.tcpi_snd_mss;

  /* In order to reduce outbound kernel queuing delays and thus improve Tor's
   * ability to prioritize circuits, KIST wants to set a socket write limit
   * that is near the amount that the socket would be able to immediately send
   * into the Internet.
   *
   * We first calculate how much the socket could send immediately (assuming
   * completely full packets) according to the congestion window and the number
   * of unacked packets.
   *
   * Then we add a little extra space in a controlled way. We do this so any
   * when the kernel gets ACKs back for data currently sitting in the "TCP
   * space", it will already have some more data to send immediately. It will
   * not have to wait for the scheduler to run again. The amount of extra space
   * is a factor of the current congestion window. With the suggested
   * sock_buf_size_factor value of 1.0, we allow at most 2*cwnd bytes to sit in
   * the kernel: 1 cwnd on the wire waiting for ACKs and 1 cwnd ready and
   * waiting to be sent when those ACKs finally come.
   *
   * In the below diagram, we see some bytes in the TCP-space (denoted by '*')
   * that have be sent onto the wire and are waiting for ACKs. We have a little
   * more room in "TCP space" that we can fill with data that will be
   * immediately sent. We also see the "extra space" KIST calculates. The sum
   * of the empty "TCP space" and the "extra space" is the kist-imposed write
   * limit for this socket.
   *
   * <----------------kernel-outbound-socket-queue----------------|
   * <*********---------------------------------------------------|
   * |----TCP-space-----|----extra-space-----|
   * |------------------|
   *                    ^ ((cwnd - unacked) * mss) bytes
   *                    |--------------------|
   *                                         ^ ((cwnd * mss) * factor) bytes
   */

  /* These values from the kernel are uint32_t, they will always fit into a
   * int64_t tcp_space variable but if the congestion window cwnd is smaller
   * than the unacked packets, the remaining TCP space is set to 0. */
  if (ent->cwnd >= ent->unacked) {
    tcp_space = (ent->cwnd - ent->unacked) * (int64_t)(ent->mss);
  } else {
    tcp_space = 0;
  }

  /* The clamp_double_to_int64 makes sure the first part fits into an int64_t.
   * In fact, if sock_buf_size_factor is still forced to be >= 0 in config.c,
   * then it will be positive for sure. Then we subtract a uint32_t. Getting a
   * negative value is OK, see after how it is being handled. */
  extra_space =
    clamp_double_to_int64(
                 (ent->cwnd * (int64_t)ent->mss) * sock_buf_size_factor) -
    ent->notsent - (int64_t)channel_outbuf_length((channel_t *) ent->chan);
  if ((tcp_space + extra_space) < 0) {
    /* This means that the "notsent" queue is just too big so we shouldn't put
     * more in the kernel for now. */
    ent->limit = 0;
  } else {
    /* The positive sum of two int64_t will always fit into an uint64_t.
     * And we know this will always be positive, since we checked above. */
    ent->limit = (uint64_t)tcp_space + (uint64_t)extra_space;
  }
  return;

#else /* !defined(HAVE_KIST_SUPPORT) */
  goto fallback;
#endif /* defined(HAVE_KIST_SUPPORT) */

 fallback:
  /* If all of a sudden we don't have kist support, we just zero out all the
   * variables for this socket since we don't know what they should be. We
   * also allow the socket to write as much as it can from the estimated
   * number of cells the lower layer can accept, effectively returning it to
   * Vanilla scheduler behavior. */
  ent->cwnd = ent->unacked = ent->mss = ent->notsent = 0;
  /* This function calls the specialized channel object (currently channeltls)
   * and ask how many cells it can write on the outbuf which we then multiply
   * by the size of the cells for this channel. The cast is because this
   * function requires a non-const channel object, meh. */
  ent->limit = channel_num_cells_writeable((channel_t *) ent->chan) *
               (get_cell_network_size(ent->chan->wide_circ_ids) +
                TLS_PER_CELL_OVERHEAD);
}

/* Given a socket that isn't in the table, add it.
 * Given a socket that is in the table, re-init values that need init-ing
 * every scheduling run
 */
static void
init_socket_info(socket_table_t *table, const channel_t *chan)
{
  socket_table_ent_t *ent = NULL;
  ent = socket_table_search(table, chan);
  if (!ent) {
    log_debug(LD_SCHED, "scheduler init socket info for chan=%" PRIu64,
              chan->global_identifier);
    ent = tor_malloc_zero(sizeof(*ent));
    ent->chan = chan;
    HT_INSERT(socket_table_s, table, ent);
  }
  ent->written = 0;
}

/* Add chan to the outbuf table if it isn't already in it. If it is, then don't
 * do anything */
static void
outbuf_table_add(outbuf_table_t *table, channel_t *chan)
{
  outbuf_table_ent_t search, *ent;
  search.chan = chan;
  ent = HT_FIND(outbuf_table_s, table, &search);
  if (!ent) {
    log_debug(LD_SCHED, "scheduler init outbuf info for chan=%" PRIu64,
              chan->global_identifier);
    ent = tor_malloc_zero(sizeof(*ent));
    ent->chan = chan;
    HT_INSERT(outbuf_table_s, table, ent);
  }
}

static void
outbuf_table_remove(outbuf_table_t *table, channel_t *chan)
{
  outbuf_table_ent_t search, *ent;
  search.chan = chan;
  ent = HT_FIND(outbuf_table_s, table, &search);
  if (ent) {
    HT_REMOVE(outbuf_table_s, table, ent);
    free_outbuf_info_by_ent(ent, NULL);
  }
}

/* Set the scheduler running interval. */
static void
set_scheduler_run_interval(void)
{
  int old_sched_run_interval = sched_run_interval;
  sched_run_interval = kist_scheduler_run_interval();
  if (old_sched_run_interval != sched_run_interval) {
    log_info(LD_SCHED, "Scheduler KIST changing its running interval "
                       "from %" PRId32 " to %" PRId32,
             old_sched_run_interval, sched_run_interval);
  }
}

/* Return true iff the channel hasn't hit its kist-imposed write limit yet */
static int
socket_can_write(socket_table_t *table, const channel_t *chan)
{
  socket_table_ent_t *ent = NULL;
  ent = socket_table_search(table, chan);
  if (SCHED_BUG(!ent, chan)) {
    return 1; // Just return true, saying that kist wouldn't limit the socket
  }

  /* We previously calculated a write limit for this socket. In the below
   * calculation, first determine how much room is left in bytes. Then divide
   * that by the amount of space a cell takes. If there's room for at least 1
   * cell, then KIST will allow the socket to write. */
  int64_t kist_limit_space =
    (int64_t) (ent->limit - ent->written) /
    (CELL_MAX_NETWORK_SIZE + TLS_PER_CELL_OVERHEAD);
  return kist_limit_space > 0;
}

/* Update the channel's socket kernel information. */
static void
update_socket_info(socket_table_t *table, const channel_t *chan)
{
  socket_table_ent_t *ent = NULL;
  ent = socket_table_search(table, chan);
  if (SCHED_BUG(!ent, chan)) {
    return; // Whelp. Entry didn't exist for some reason so nothing to do.
  }
  update_socket_info_impl(ent);
  log_debug(LD_SCHED, "chan=%" PRIu64 " updated socket info, limit: %" PRIu64
                      ", cwnd: %" PRIu32 ", unacked: %" PRIu32
                      ", notsent: %" PRIu32 ", mss: %" PRIu32,
            ent->chan->global_identifier, ent->limit, ent->cwnd, ent->unacked,
            ent->notsent, ent->mss);
}

/* Increment the channel's socket written value by the number of bytes. */
static void
update_socket_written(socket_table_t *table, channel_t *chan, size_t bytes)
{
  socket_table_ent_t *ent = NULL;
  ent = socket_table_search(table, chan);
  if (SCHED_BUG(!ent, chan)) {
    return; // Whelp. Entry didn't exist so nothing to do.
  }

  log_debug(LD_SCHED, "chan=%" PRIu64 " wrote %lu bytes, old was %" PRIi64,
            chan->global_identifier, (unsigned long) bytes, ent->written);

  ent->written += bytes;
}

/*
 * A naive KIST impl would write every single cell all the way to the kernel.
 * That would take a lot of system calls. A less bad KIST impl would write a
 * channel's outbuf to the kernel only when we are switching to a different
 * channel. But if we have two channels with equal priority, we end up writing
 * one cell for each and bouncing back and forth. This KIST impl avoids that
 * by only writing a channel's outbuf to the kernel if it has 8 cells or more
 * in it.
 *
 * Note: The number 8 has been picked for no particular reasons except that it
 * is 4096 bytes which is a common number for buffering. A TLS record can hold
 * up to 16KiB thus using 8 cells means that a relay will at most send a TLS
 * record of 4KiB or 1/4 of the maximum capacity of a TLS record.
 */
MOCK_IMPL(int, channel_should_write_to_kernel,
          (outbuf_table_t *table, channel_t *chan))
{
  outbuf_table_add(table, chan);
  /* CELL_MAX_NETWORK_SIZE * 8 because we only want to write the outbuf to the
   * kernel if there's 8 or more cells waiting */
  return channel_outbuf_length(chan) > (CELL_MAX_NETWORK_SIZE * 8);
}

/* Little helper function to write a channel's outbuf all the way to the
 * kernel */
MOCK_IMPL(void, channel_write_to_kernel, (channel_t *chan))
{
  tor_assert(chan);

  /* This is possible because a channel might have an outbuf table entry even
   * though it has no more cells in its outbuf. Just move on. */
  size_t outbuf_len = channel_outbuf_length(chan);
  if (outbuf_len == 0) {
    return;
  }

  log_debug(LD_SCHED, "Writing %lu bytes to kernel for chan %" PRIu64,
            (unsigned long) outbuf_len, chan->global_identifier);

  /* Note that 'connection_handle_write()' may change the scheduler state of
   * the channel during the scheduling loop with
   * 'connection_or_flushed_some()' -> 'scheduler_channel_wants_writes()'.
   * This side-effect will only occur if the channel is currently in the
   * 'SCHED_CHAN_WAITING_TO_WRITE' or 'SCHED_CHAN_IDLE' states, which KIST
   * rarely uses, so it should be fine unless KIST begins using these states
   * in the future. */
  connection_handle_write(TO_CONN(BASE_CHAN_TO_TLS(chan)->conn), 0);
}

/* Return true iff the scheduler has work to perform. */
static int
have_work(void)
{
  smartlist_t *cp = get_channels_pending();
  IF_BUG_ONCE(!cp) {
    return 0; // channels_pending doesn't exist so... no work?
  }
  return smartlist_len(cp) > 0;
}

/* Function of the scheduler interface: free_all() */
static void
kist_free_all(void)
{
  free_all_socket_info();
}

/* Function of the scheduler interface: on_channel_free() */
static void
kist_on_channel_free_fn(const channel_t *chan)
{
  free_socket_info_by_chan(&socket_table, chan);
}

/* Function of the scheduler interface: on_new_consensus() */
static void
kist_scheduler_on_new_consensus(void)
{
  set_scheduler_run_interval();
}

/* Function of the scheduler interface: on_new_options() */
static void
kist_scheduler_on_new_options(void)
{
  sock_buf_size_factor = get_options()->KISTSockBufSizeFactor;

  /* Calls kist_scheduler_run_interval which calls get_options(). */
  set_scheduler_run_interval();
}

/* Function of the scheduler interface: init() */
static void
kist_scheduler_init(void)
{
  /* When initializing the scheduler, the last run could be 0 because it is
   * declared static or a value in the past that was set when it was last
   * used. In both cases, we want to initialize it to now so we don't risk
   * using the value 0 which doesn't play well with our monotonic time
   * interface.
   *
   * One side effect is that the first scheduler run will be at the next tick
   * that is in now + 10 msec (KIST_SCHED_RUN_INTERVAL_DEFAULT) by default. */
  monotime_get(&scheduler_last_run);

  kist_scheduler_on_new_options();
  IF_BUG_ONCE(sched_run_interval == 0) {
    log_warn(LD_SCHED, "We are initing the KIST scheduler and noticed the "
             "KISTSchedRunInterval is telling us to not use KIST. That's "
             "weird! We'll continue using KIST, but at %" PRId32 "ms.",
             KIST_SCHED_RUN_INTERVAL_DEFAULT);
    sched_run_interval = KIST_SCHED_RUN_INTERVAL_DEFAULT;
  }
}

/* Function of the scheduler interface: schedule() */
static void
kist_scheduler_schedule(void)
{
  struct monotime_t now;
  struct timeval next_run;
  int64_t diff;

  if (!have_work()) {
    return;
  }
  monotime_get(&now);

  /* If time is really monotonic, we can never have now being smaller than the
   * last scheduler run. The scheduler_last_run at first is set to 0.
   * Unfortunately, not all platforms guarantee monotonic time so we log at
   * info level but don't make it more noisy. */
  diff = monotime_diff_msec(&scheduler_last_run, &now);
  if (diff < 0) {
    log_info(LD_SCHED, "Monotonic time between now and last run of scheduler "
                       "is negative: %" PRId64 ". Setting diff to 0.", diff);
    diff = 0;
  }
  if (diff < sched_run_interval) {
    next_run.tv_sec = 0;
    /* Takes 1000 ms -> us. This will always be valid because diff can NOT be
     * negative and can NOT be bigger than sched_run_interval so values can
     * only go from 1000 usec (diff set to interval - 1) to 100000 usec (diff
     * set to 0) for the maximum allowed run interval (100ms). */
    next_run.tv_usec = (int) ((sched_run_interval - diff) * 1000);
    /* Re-adding an event reschedules it. It does not duplicate it. */
    scheduler_ev_add(&next_run);
  } else {
    scheduler_ev_active();
  }
}

/* Function of the scheduler interface: run() */
static void
kist_scheduler_run(void)
{
  /* Define variables */
  channel_t *chan = NULL; // current working channel
  /* The last distinct chan served in a sched loop. */
  channel_t *prev_chan = NULL;
  int flush_result; // temporarily store results from flush calls
  /* Channels to be re-adding to pending at the end */
  smartlist_t *to_readd = NULL;
  smartlist_t *cp = get_channels_pending();

  outbuf_table_t outbuf_table = HT_INITIALIZER();

  /* For each pending channel, collect new kernel information */
  SMARTLIST_FOREACH_BEGIN(cp, const channel_t *, pchan) {
      init_socket_info(&socket_table, pchan);
      update_socket_info(&socket_table, pchan);
  } SMARTLIST_FOREACH_END(pchan);

  log_debug(LD_SCHED, "Running the scheduler. %d channels pending",
            smartlist_len(cp));

  /* The main scheduling loop. Loop until there are no more pending channels */
  while (smartlist_len(cp) > 0) {
    /* get best channel */
    chan = smartlist_pqueue_pop(cp, scheduler_compare_channels,
                                offsetof(channel_t, sched_heap_idx));
    if (SCHED_BUG(!chan, NULL)) {
      /* Some-freaking-how a NULL got into the channels_pending. That should
       * never happen, but it should be harmless to ignore it and keep looping.
       */
      continue;
    }
    outbuf_table_add(&outbuf_table, chan);

    /* if we have switched to a new channel, consider writing the previous
     * channel's outbuf to the kernel. */
    if (!prev_chan) {
      prev_chan = chan;
    }
    if (prev_chan != chan) {
      if (channel_should_write_to_kernel(&outbuf_table, prev_chan)) {
        channel_write_to_kernel(prev_chan);
        outbuf_table_remove(&outbuf_table, prev_chan);
      }
      prev_chan = chan;
    }

    /* Only flush and write if the per-socket limit hasn't been hit */
    if (socket_can_write(&socket_table, chan)) {
      /* flush to channel queue/outbuf */
      flush_result = (int)channel_flush_some_cells(chan, 1); // 1 for num cells
      /* XXX: While flushing cells, it is possible that the connection write
       * fails leading to the channel to be closed which triggers a release
       * and free its entry in the socket table. And because of a engineering
       * design issue, the error is not propagated back so we don't get an
       * error at this point. So before we continue, make sure the channel is
       * open and if not just ignore it. See #23751. */
      if (!CHANNEL_IS_OPEN(chan)) {
        /* Channel isn't open so we put it back in IDLE mode. It is either
         * renegotiating its TLS session or about to be released. */
        scheduler_set_channel_state(chan, SCHED_CHAN_IDLE);
        continue;
      }
      /* flush_result has the # cells flushed */
      if (flush_result > 0) {
        update_socket_written(&socket_table, chan, flush_result *
                              (CELL_MAX_NETWORK_SIZE + TLS_PER_CELL_OVERHEAD));
      } else {
        /* XXX: This can happen because tor sometimes does flush in an
         * opportunistic way cells from the circuit to the outbuf so the
         * channel can end up here without having anything to flush nor needed
         * to write to the kernel. Hopefully we'll fix that soon but for now
         * we have to handle this case which happens kind of often. */
        log_debug(LD_SCHED,
                 "We didn't flush anything on a chan that we think "
                 "can write and wants to write. The channel's state is '%s' "
                 "and in scheduler state '%s'. We're going to mark it as "
                 "waiting_for_cells (as that's most likely the issue) and "
                 "stop scheduling it this round.",
                 channel_state_to_string(chan->state),
                 get_scheduler_state_string(chan->scheduler_state));
        scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_FOR_CELLS);
        continue;
      }
    }

    /* Decide what to do with the channel now */

    if (!channel_more_to_flush(chan) &&
        !socket_can_write(&socket_table, chan)) {

      /* Case 1: no more cells to send, and cannot write */

      /*
       * You might think we should put the channel in SCHED_CHAN_IDLE. And
       * you're probably correct. While implementing KIST, we found that the
       * scheduling system would sometimes lose track of channels when we did
       * that. We suspect it has to do with the difference between "can't
       * write because socket/outbuf is full" and KIST's "can't write because
       * we've arbitrarily decided that that's enough for now." Sometimes
       * channels run out of cells at the same time they hit their
       * kist-imposed write limit and maybe the rest of Tor doesn't put the
       * channel back in pending when it is supposed to.
       *
       * This should be investigated again. It is as simple as changing
       * SCHED_CHAN_WAITING_FOR_CELLS to SCHED_CHAN_IDLE and seeing if Tor
       * starts having serious throughput issues. Best done in shadow/chutney.
       */
      scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_FOR_CELLS);
    } else if (!channel_more_to_flush(chan)) {

      /* Case 2: no more cells to send, but still open for writes */

      scheduler_set_channel_state(chan, SCHED_CHAN_WAITING_FOR_CELLS);
    } else if (!socket_can_write(&socket_table, chan)) {

      /* Case 3: cells to send, but cannot write */

      /*
       * We want to write, but can't. If we left the channel in
       * channels_pending, we would never exit the scheduling loop. We need to
       * add it to a temporary list of channels to be added to channels_pending
       * after the scheduling loop is over. They can hopefully be taken care of
       * in the next scheduling round.
       */
      if (!to_readd) {
        to_readd = smartlist_new();
      }
      smartlist_add(to_readd, chan);
    } else {

      /* Case 4: cells to send, and still open for writes */

      scheduler_set_channel_state(chan, SCHED_CHAN_PENDING);
      if (!SCHED_BUG(chan->sched_heap_idx != -1, chan)) {
        smartlist_pqueue_add(cp, scheduler_compare_channels,
                             offsetof(channel_t, sched_heap_idx), chan);
      }
    }
  } /* End of main scheduling loop */

  /* Write the outbuf of any channels that still have data */
  HT_FOREACH_FN(outbuf_table_s, &outbuf_table, each_channel_write_to_kernel,
                NULL);
  /* We are done with it. */
  HT_FOREACH_FN(outbuf_table_s, &outbuf_table, free_outbuf_info_by_ent, NULL);
  HT_CLEAR(outbuf_table_s, &outbuf_table);

  log_debug(LD_SCHED, "len pending=%d, len to_readd=%d",
            smartlist_len(cp),
            (to_readd ? smartlist_len(to_readd) : -1));

  /* Re-add any channels we need to */
  if (to_readd) {
    SMARTLIST_FOREACH_BEGIN(to_readd, channel_t *, readd_chan) {
      scheduler_set_channel_state(readd_chan, SCHED_CHAN_PENDING);
      if (!smartlist_contains(cp, readd_chan)) {
        if (!SCHED_BUG(readd_chan->sched_heap_idx != -1, readd_chan)) {
          /* XXXX Note that the check above is in theory redundant with
           * the smartlist_contains check.  But let's make sure we're
           * not messing anything up, and leave them both for now. */
          smartlist_pqueue_add(cp, scheduler_compare_channels,
                             offsetof(channel_t, sched_heap_idx), readd_chan);
        }
      }
    } SMARTLIST_FOREACH_END(readd_chan);
    smartlist_free(to_readd);
  }

  monotime_get(&scheduler_last_run);
}

/*****************************************************************************
 * Externally called function implementations not called through scheduler_t
 *****************************************************************************/

/* Stores the kist scheduler function pointers. */
static scheduler_t kist_scheduler = {
  .type = SCHEDULER_KIST,
  .free_all = kist_free_all,
  .on_channel_free = kist_on_channel_free_fn,
  .init = kist_scheduler_init,
  .on_new_consensus = kist_scheduler_on_new_consensus,
  .schedule = kist_scheduler_schedule,
  .run = kist_scheduler_run,
  .on_new_options = kist_scheduler_on_new_options,
};

/* Return the KIST scheduler object. If it didn't exists, return a newly
 * allocated one but init() is not called. */
scheduler_t *
get_kist_scheduler(void)
{
  return &kist_scheduler;
}

/* Check the torrc (and maybe consensus) for the configured KIST scheduler run
 * interval.
 * - If torrc > 0, then return the positive torrc value (should use KIST, and
 *   should use the set value)
 * - If torrc == 0, then look in the consensus for what the value should be.
 *   - If == 0, then return 0 (don't use KIST)
 *   - If > 0, then return the positive consensus value
 *   - If consensus doesn't say anything, return 10 milliseconds, default.
 */
int
kist_scheduler_run_interval(void)
{
  int run_interval = get_options()->KISTSchedRunInterval;

  if (run_interval != 0) {
    log_debug(LD_SCHED, "Found KISTSchedRunInterval=%" PRId32 " in torrc. "
                        "Using that.", run_interval);
    return run_interval;
  }

  log_debug(LD_SCHED, "KISTSchedRunInterval=0, turning to the consensus.");

  /* Will either be the consensus value or the default. Note that 0 can be
   * returned which means the consensus wants us to NOT use KIST. */
  return networkstatus_get_param(NULL, "KISTSchedRunInterval",
                                 KIST_SCHED_RUN_INTERVAL_DEFAULT,
                                 KIST_SCHED_RUN_INTERVAL_MIN,
                                 KIST_SCHED_RUN_INTERVAL_MAX);
}

/* Set KISTLite mode that is KIST without kernel support. */
void
scheduler_kist_set_lite_mode(void)
{
  kist_lite_mode = 1;
  kist_scheduler.type = SCHEDULER_KIST_LITE;
  log_info(LD_SCHED,
           "Setting KIST scheduler without kernel support (KISTLite mode)");
}

/* Set KIST mode that is KIST with kernel support. */
void
scheduler_kist_set_full_mode(void)
{
  kist_lite_mode = 0;
  kist_scheduler.type = SCHEDULER_KIST;
  log_info(LD_SCHED,
           "Setting KIST scheduler with kernel support (KIST mode)");
}

#ifdef HAVE_KIST_SUPPORT

/* Return true iff the scheduler subsystem should use KIST. */
int
scheduler_can_use_kist(void)
{
  if (kist_no_kernel_support) {
    /* We have no kernel support so we can't use KIST. */
    return 0;
  }

  /* We do have the support, time to check if we can get the interval that the
   * consensus can be disabling. */
  int run_interval = kist_scheduler_run_interval();
  log_debug(LD_SCHED, "Determined KIST sched_run_interval should be "
                      "%" PRId32 ". Can%s use KIST.",
           run_interval, (run_interval > 0 ? "" : " not"));
  return run_interval > 0;
}

#else /* !defined(HAVE_KIST_SUPPORT) */

int
scheduler_can_use_kist(void)
{
  return 0;
}

#endif /* defined(HAVE_KIST_SUPPORT) */
