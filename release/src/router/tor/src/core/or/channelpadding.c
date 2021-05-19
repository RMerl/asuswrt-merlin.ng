/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file channelpadding.c
 * @brief Link-level padding code.
 **/

/* CHANNEL_OBJECT_PRIVATE define needed for an O(1) implementation of
 * channelpadding_channel_to_channelinfo() */
#define CHANNEL_OBJECT_PRIVATE

#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/channelpadding.h"
#include "core/or/channeltls.h"
#include "app/config/config.h"
#include "feature/nodelist/networkstatus.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_or.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/mainloop/mainloop.h"
#include "feature/stats/rephist.h"
#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "lib/time/compat_time.h"
#include "feature/rend/rendservice.h"
#include "lib/evloop/timers.h"

#include "core/or/cell_st.h"
#include "core/or/or_connection_st.h"

STATIC int32_t channelpadding_get_netflow_inactive_timeout_ms(
                                                           const channel_t *);
STATIC int channelpadding_send_disable_command(channel_t *);
STATIC int64_t channelpadding_compute_time_until_pad_for_netflow(channel_t *);

/** The total number of pending channelpadding timers */
static uint64_t total_timers_pending;

/** These are cached consensus parameters for netflow */
/** The timeout lower bound that is allowed before sending padding */
static int consensus_nf_ito_low;
/** The timeout upper bound that is allowed before sending padding */
static int consensus_nf_ito_high;
/** The timeout lower bound that is allowed before sending reduced padding */
static int consensus_nf_ito_low_reduced;
/** The timeout upper bound that is allowed before sending reduced padding */
static int consensus_nf_ito_high_reduced;
/** The connection timeout between relays */
static int consensus_nf_conntimeout_relays;
/** The connection timeout for client connections */
static int consensus_nf_conntimeout_clients;
/** Should we pad before circuits are actually used for client data? */
static int consensus_nf_pad_before_usage;
/** Should we pad relay-to-relay connections? */
static int consensus_nf_pad_relays;
/** Should we pad rosos connections? */
static int consensus_nf_pad_single_onion;

#define TOR_MSEC_PER_SEC 1000
#define TOR_USEC_PER_MSEC 1000

/**
 * How often do we get called by the connection housekeeping (ie: once
 * per second) */
#define TOR_HOUSEKEEPING_CALLBACK_MSEC 1000
/**
 * Additional extra time buffer on the housekeeping callback, since
 * it can be delayed. This extra slack is used to decide if we should
 * schedule a timer or wait for the next callback. */
#define TOR_HOUSEKEEPING_CALLBACK_SLACK_MSEC 100

/**
 * This macro tells us if either end of the channel is connected to a client.
 * (If we're not a server, we're definitely a client. If the channel thinks
 *  it's a client, use that. Then finally verify in the consensus).
 */
#define CHANNEL_IS_CLIENT(chan, options) \
  (!public_server_mode((options)) || channel_is_client(chan) || \
      !connection_or_digest_is_known_relay((chan)->identity_digest))

/**
 * This function is called to update cached consensus parameters every time
 * there is a consensus update. This allows us to move the consensus param
 * search off of the critical path, so it does not need to be evaluated
 * for every single connection, every second.
 */
void
channelpadding_new_consensus_params(const networkstatus_t *ns)
{
#define DFLT_NETFLOW_INACTIVE_KEEPALIVE_LOW 1500
#define DFLT_NETFLOW_INACTIVE_KEEPALIVE_HIGH 9500
#define DFLT_NETFLOW_INACTIVE_KEEPALIVE_MIN 0
#define DFLT_NETFLOW_INACTIVE_KEEPALIVE_MAX 60000
  consensus_nf_ito_low = networkstatus_get_param(ns, "nf_ito_low",
      DFLT_NETFLOW_INACTIVE_KEEPALIVE_LOW,
      DFLT_NETFLOW_INACTIVE_KEEPALIVE_MIN,
      DFLT_NETFLOW_INACTIVE_KEEPALIVE_MAX);
  consensus_nf_ito_high = networkstatus_get_param(ns, "nf_ito_high",
      DFLT_NETFLOW_INACTIVE_KEEPALIVE_HIGH,
      consensus_nf_ito_low,
      DFLT_NETFLOW_INACTIVE_KEEPALIVE_MAX);

#define DFLT_NETFLOW_REDUCED_KEEPALIVE_LOW 9000
#define DFLT_NETFLOW_REDUCED_KEEPALIVE_HIGH 14000
#define DFLT_NETFLOW_REDUCED_KEEPALIVE_MIN 0
#define DFLT_NETFLOW_REDUCED_KEEPALIVE_MAX 60000
  consensus_nf_ito_low_reduced =
    networkstatus_get_param(ns, "nf_ito_low_reduced",
        DFLT_NETFLOW_REDUCED_KEEPALIVE_LOW,
        DFLT_NETFLOW_REDUCED_KEEPALIVE_MIN,
        DFLT_NETFLOW_REDUCED_KEEPALIVE_MAX);

  consensus_nf_ito_high_reduced =
    networkstatus_get_param(ns, "nf_ito_high_reduced",
        DFLT_NETFLOW_REDUCED_KEEPALIVE_HIGH,
        consensus_nf_ito_low_reduced,
        DFLT_NETFLOW_REDUCED_KEEPALIVE_MAX);

#define CONNTIMEOUT_RELAYS_DFLT (60*60) // 1 hour
#define CONNTIMEOUT_RELAYS_MIN 60
#define CONNTIMEOUT_RELAYS_MAX (7*24*60*60) // 1 week
  consensus_nf_conntimeout_relays =
    networkstatus_get_param(ns, "nf_conntimeout_relays",
        CONNTIMEOUT_RELAYS_DFLT,
        CONNTIMEOUT_RELAYS_MIN,
        CONNTIMEOUT_RELAYS_MAX);

#define CIRCTIMEOUT_CLIENTS_DFLT (30*60) // 30 minutes
#define CIRCTIMEOUT_CLIENTS_MIN 60
#define CIRCTIMEOUT_CLIENTS_MAX (24*60*60) // 24 hours
  consensus_nf_conntimeout_clients =
    networkstatus_get_param(ns, "nf_conntimeout_clients",
        CIRCTIMEOUT_CLIENTS_DFLT,
        CIRCTIMEOUT_CLIENTS_MIN,
        CIRCTIMEOUT_CLIENTS_MAX);

  consensus_nf_pad_before_usage =
    networkstatus_get_param(ns, "nf_pad_before_usage", 1, 0, 1);

  consensus_nf_pad_relays =
    networkstatus_get_param(ns, "nf_pad_relays", 0, 0, 1);

  consensus_nf_pad_single_onion =
    networkstatus_get_param(ns,
                            CHANNELPADDING_SOS_PARAM,
                            CHANNELPADDING_SOS_DEFAULT, 0, 1);
}

/**
 * Get a random netflow inactive timeout keepalive period in milliseconds,
 * the range for which is determined by consensus parameters, negotiation,
 * configuration, or default values. The consensus parameters enforce the
 * minimum possible value, to avoid excessively frequent padding.
 *
 * The ranges for this value were chosen to be low enough to ensure that
 * routers do not emit a new netflow record for a connection due to it
 * being idle.
 *
 * Specific timeout values for major routers are listed in Proposal 251.
 * No major router appeared capable of setting an inactive timeout below 10
 * seconds, so we set the defaults below that value, since we can always
 * scale back if it ends up being too much padding.
 *
 * Returns the next timeout period (in milliseconds) after which we should
 * send a padding packet, or 0 if padding is disabled.
 */
STATIC int32_t
channelpadding_get_netflow_inactive_timeout_ms(const channel_t *chan)
{
  int low_timeout = consensus_nf_ito_low;
  int high_timeout = consensus_nf_ito_high;
  int X1, X2;

  if (low_timeout == 0 && low_timeout == high_timeout)
    return 0; // No padding

  /* If we have negotiated different timeout values, use those, but
   * don't allow them to be lower than the consensus ones */
  if (chan->padding_timeout_low_ms && chan->padding_timeout_high_ms) {
    low_timeout = MAX(low_timeout, chan->padding_timeout_low_ms);
    high_timeout = MAX(high_timeout, chan->padding_timeout_high_ms);
  }

  if (low_timeout == high_timeout)
    return low_timeout; // No randomization

  /*
   * This MAX() hack is here because we apply the timeout on both the client
   * and the server. This creates the situation where the total time before
   * sending a packet in either direction is actually
   * min(client_timeout,server_timeout).
   *
   * If X is a random variable uniform from 0..R-1 (where R=high-low),
   * then Y=max(X,X) has Prob(Y == i) = (2.0*i + 1)/(R*R).
   *
   * If we create a third random variable Z=min(Y,Y), then it turns out that
   * Exp[Z] ~= Exp[X]. Here's a table:
   *
   *    R     Exp[X]    Exp[Z]    Exp[min(X,X)]   Exp[max(X,X)]
   *  2000     999.5    1066        666.2           1332.8
   *  3000    1499.5    1599.5      999.5           1999.5
   *  5000    2499.5    2666       1666.2           3332.8
   *  6000    2999.5    3199.5     1999.5           3999.5
   *  7000    3499.5    3732.8     2332.8           4666.2
   *  8000    3999.5    4266.2     2666.2           5332.8
   *  10000   4999.5    5328       3332.8           6666.2
   *  15000   7499.5    7995       4999.5           9999.5
   *  20000   9900.5    10661      6666.2           13332.8
   *
   * In other words, this hack makes it so that when both the client and
   * the guard are sending this padding, then the averages work out closer
   * to the midpoint of the range, making the overhead easier to tune.
   * If only one endpoint is padding (for example: if the relay does not
   * support padding, but the client has set ConnectionPadding 1; or
   * if the relay does support padding, but the client has set
   * ReducedConnectionPadding 1), then the defense will still prevent
   * record splitting, but with less overhead than the midpoint
   * (as seen by the Exp[max(X,X)] column).
   *
   * To calculate average padding packet frequency (and thus overhead),
   * index into the table by picking a row based on R = high-low. Then,
   * use the appropriate column (Exp[Z] for two-sided padding, and
   * Exp[max(X,X)] for one-sided padding). Finally, take this value
   * and add it to the low timeout value. This value is the average
   * frequency which padding packets will be sent.
   */

  X1 = crypto_rand_int(high_timeout - low_timeout);
  X2 = crypto_rand_int(high_timeout - low_timeout);
  return low_timeout + MAX(X1, X2);
}

/**
 * Update this channel's padding settings based on the PADDING_NEGOTIATE
 * contents.
 *
 * Returns -1 on error; 1 on success.
 */
int
channelpadding_update_padding_for_channel(channel_t *chan,
                const channelpadding_negotiate_t *pad_vars)
{
  if (pad_vars->version != 0) {
    static ratelim_t version_limit = RATELIM_INIT(600);

    log_fn_ratelim(&version_limit,LOG_PROTOCOL_WARN,LD_PROTOCOL,
           "Got a PADDING_NEGOTIATE cell with an unknown version. Ignoring.");
    return -1;
  }

  // We should not allow malicious relays to disable or reduce padding for
  // us as clients. In fact, we should only accept this cell at all if we're
  // operating as a relay. Bridges should not accept it from relays, either
  // (only from their clients).
  if ((get_options()->BridgeRelay &&
       connection_or_digest_is_known_relay(chan->identity_digest)) ||
      !get_options()->ORPort_set) {
    static ratelim_t relay_limit = RATELIM_INIT(600);

    log_fn_ratelim(&relay_limit,LOG_PROTOCOL_WARN,LD_PROTOCOL,
           "Got a PADDING_NEGOTIATE from relay at %s (%s). "
           "This should not happen.",
           channel_describe_peer(chan),
           hex_str(chan->identity_digest, DIGEST_LEN));
    return -1;
  }

  chan->padding_enabled = (pad_vars->command == CHANNELPADDING_COMMAND_START);

  /* Min must not be lower than the current consensus parameter
     nf_ito_low. */
  chan->padding_timeout_low_ms = MAX(consensus_nf_ito_low,
                                     pad_vars->ito_low_ms);

  /* Max must not be lower than ito_low_ms */
  chan->padding_timeout_high_ms = MAX(chan->padding_timeout_low_ms,
                                      pad_vars->ito_high_ms);

  log_fn(LOG_INFO,LD_OR,
         "Negotiated padding=%d, lo=%d, hi=%d on %"PRIu64,
         chan->padding_enabled, chan->padding_timeout_low_ms,
         chan->padding_timeout_high_ms,
         (chan->global_identifier));

  return 1;
}

/**
 * Sends a CELL_PADDING_NEGOTIATE on the channel to tell the other side not
 * to send padding.
 *
 * Returns -1 on error, 0 on success.
 */
STATIC int
channelpadding_send_disable_command(channel_t *chan)
{
  channelpadding_negotiate_t disable;
  cell_t cell;

  tor_assert(chan);
  tor_assert(BASE_CHAN_TO_TLS(chan)->conn->link_proto >=
             MIN_LINK_PROTO_FOR_CHANNEL_PADDING);

  memset(&cell, 0, sizeof(cell_t));
  memset(&disable, 0, sizeof(channelpadding_negotiate_t));
  cell.command = CELL_PADDING_NEGOTIATE;

  channelpadding_negotiate_set_command(&disable, CHANNELPADDING_COMMAND_STOP);

  if (channelpadding_negotiate_encode(cell.payload, CELL_PAYLOAD_SIZE,
                                      &disable) < 0)
    return -1;

  if (chan->write_cell(chan, &cell) == 1)
    return 0;
  else
    return -1;
}

/**
 * Sends a CELL_PADDING_NEGOTIATE on the channel to tell the other side to
 * resume sending padding at some rate.
 *
 * Returns -1 on error, 0 on success.
 */
int
channelpadding_send_enable_command(channel_t *chan, uint16_t low_timeout,
                                   uint16_t high_timeout)
{
  channelpadding_negotiate_t enable;
  cell_t cell;

  tor_assert(chan);
  tor_assert(BASE_CHAN_TO_TLS(chan)->conn->link_proto >=
             MIN_LINK_PROTO_FOR_CHANNEL_PADDING);

  memset(&cell, 0, sizeof(cell_t));
  memset(&enable, 0, sizeof(channelpadding_negotiate_t));
  cell.command = CELL_PADDING_NEGOTIATE;

  channelpadding_negotiate_set_command(&enable, CHANNELPADDING_COMMAND_START);
  channelpadding_negotiate_set_ito_low_ms(&enable, low_timeout);
  channelpadding_negotiate_set_ito_high_ms(&enable, high_timeout);

  if (channelpadding_negotiate_encode(cell.payload, CELL_PAYLOAD_SIZE,
                                      &enable) < 0)
    return -1;

  if (chan->write_cell(chan, &cell) == 1)
    return 0;
  else
    return -1;
}

/**
 * Sends a CELL_PADDING cell on a channel if it has been idle since
 * our callback was scheduled.
 *
 * This function also clears the pending padding timer and the callback
 * flags.
 */
static void
channelpadding_send_padding_cell_for_callback(channel_t *chan)
{
  cell_t cell;

  /* Check that the channel is still valid and open */
  if (!chan || chan->state != CHANNEL_STATE_OPEN) {
    if (chan) chan->pending_padding_callback = 0;
    log_fn(LOG_INFO,LD_OR,
           "Scheduled a netflow padding cell, but connection already closed.");
    return;
  }

  /* We should have a pending callback flag set. */
  if (BUG(chan->pending_padding_callback == 0))
    return;

  chan->pending_padding_callback = 0;

  if (monotime_coarse_is_zero(&chan->next_padding_time) ||
      chan->has_queued_writes(chan) ||
      (chan->cmux && circuitmux_num_cells(chan->cmux))) {
    /* We must have been active before the timer fired */
    monotime_coarse_zero(&chan->next_padding_time);
    return;
  }

  {
    monotime_coarse_t now;
    monotime_coarse_get(&now);

    log_fn(LOG_INFO,LD_OR,
        "Sending netflow keepalive on %"PRIu64" to %s (%s) after "
        "%"PRId64" ms. Delta %"PRId64"ms",
        (chan->global_identifier),
        safe_str_client(channel_describe_peer(chan)),
        safe_str_client(hex_str(chan->identity_digest, DIGEST_LEN)),
        (monotime_coarse_diff_msec(&chan->timestamp_xfer,&now)),
        (
                   monotime_coarse_diff_msec(&chan->next_padding_time,&now)));
  }

  /* Clear the timer */
  monotime_coarse_zero(&chan->next_padding_time);

  /* Send the padding cell. This will cause the channel to get a
   * fresh timestamp_active */
  memset(&cell, 0, sizeof(cell));
  cell.command = CELL_PADDING;
  chan->write_cell(chan, &cell);
}

/**
 * tor_timer callback function for us to send padding on an idle channel.
 *
 * This function just obtains the channel from the callback handle, ensures
 * it is still valid, and then hands it off to
 * channelpadding_send_padding_cell_for_callback(), which checks if
 * the channel is still idle before sending padding.
 */
static void
channelpadding_send_padding_callback(tor_timer_t *timer, void *args,
                                     const struct monotime_t *when)
{
  channel_t *chan = channel_handle_get((struct channel_handle_t*)args);
  (void)timer; (void)when;

  if (chan && CHANNEL_CAN_HANDLE_CELLS(chan)) {
    /* Hrmm.. It might be nice to have an equivalent to assert_connection_ok
     * for channels. Then we could get rid of the channeltls dependency */
    tor_assert(TO_CONN(BASE_CHAN_TO_TLS(chan)->conn)->magic ==
               OR_CONNECTION_MAGIC);
    assert_connection_ok(TO_CONN(BASE_CHAN_TO_TLS(chan)->conn), approx_time());

    channelpadding_send_padding_cell_for_callback(chan);
  } else {
     log_fn(LOG_INFO,LD_OR,
            "Channel closed while waiting for timer.");
  }

  total_timers_pending--;
}

/**
 * Schedules a callback to send padding on a channel in_ms milliseconds from
 * now.
 *
 * Returns CHANNELPADDING_WONTPAD on error, CHANNELPADDING_PADDING_SENT if we
 * sent the packet immediately without a timer, and
 * CHANNELPADDING_PADDING_SCHEDULED if we decided to schedule a timer.
 */
static channelpadding_decision_t
channelpadding_schedule_padding(channel_t *chan, int in_ms)
{
  struct timeval timeout;
  tor_assert(!chan->pending_padding_callback);

  if (in_ms <= 0) {
    chan->pending_padding_callback = 1;
    channelpadding_send_padding_cell_for_callback(chan);
    return CHANNELPADDING_PADDING_SENT;
  }

  timeout.tv_sec = in_ms/TOR_MSEC_PER_SEC;
  timeout.tv_usec = (in_ms%TOR_USEC_PER_MSEC)*TOR_USEC_PER_MSEC;

  if (!chan->timer_handle) {
    chan->timer_handle = channel_handle_new(chan);
  }

  if (chan->padding_timer) {
    timer_set_cb(chan->padding_timer,
                 channelpadding_send_padding_callback,
                 chan->timer_handle);
  } else {
    chan->padding_timer = timer_new(channelpadding_send_padding_callback,
                                    chan->timer_handle);
  }
  timer_schedule(chan->padding_timer, &timeout);

  rep_hist_padding_count_timers(++total_timers_pending);

  chan->pending_padding_callback = 1;
  return CHANNELPADDING_PADDING_SCHEDULED;
}

/**
 * Calculates the number of milliseconds from now to schedule a padding cell.
 *
 * Returns the number of milliseconds from now (relative) to schedule the
 * padding callback. If the padding timer is more than 1.1 seconds in the
 * future, we return -1, to avoid scheduling excessive callbacks. If padding
 * is disabled in the consensus, we return -2.
 *
 * Side-effects: Updates chan->next_padding_time_ms, storing an (absolute, not
 * relative) millisecond representation of when we should send padding, unless
 * other activity happens first. This side-effect allows us to avoid
 * scheduling a libevent callback until we're within 1.1 seconds of the padding
 * time.
 */
#define CHANNELPADDING_TIME_LATER -1
#define CHANNELPADDING_TIME_DISABLED -2
STATIC int64_t
channelpadding_compute_time_until_pad_for_netflow(channel_t *chan)
{
  monotime_coarse_t now;
  monotime_coarse_get(&now);

  if (monotime_coarse_is_zero(&chan->next_padding_time)) {
    /* If the below line or crypto_rand_int() shows up on a profile,
     * we can avoid getting a timeout until we're at least nf_ito_lo
     * from a timeout window. That will prevent us from setting timers
     * on connections that were active up to 1.5 seconds ago.
     * Idle connections should only call this once every 5.5s on average
     * though, so that might be a micro-optimization for little gain. */
    int32_t padding_timeout =
        channelpadding_get_netflow_inactive_timeout_ms(chan);

    if (!padding_timeout)
      return CHANNELPADDING_TIME_DISABLED;

    monotime_coarse_add_msec(&chan->next_padding_time,
                             &chan->timestamp_xfer,
                             padding_timeout);
  }

  const int64_t ms_till_pad =
    monotime_coarse_diff_msec(&now, &chan->next_padding_time);

  /* If the next padding time is beyond the maximum possible consensus value,
   * then this indicates a clock jump, so just send padding now. This is
   * better than using monotonic time because we want to avoid the situation
   * where we wait around forever for monotonic time to move forward after
   * a clock jump far into the past.
   */
  if (ms_till_pad > DFLT_NETFLOW_INACTIVE_KEEPALIVE_MAX) {
    tor_fragile_assert();
    log_warn(LD_BUG,
        "Channel padding timeout scheduled %"PRId64"ms in the future. "
        "Did the monotonic clock just jump?",
        (ms_till_pad));
    return 0; /* Clock jumped: Send padding now */
  }

  /* If the timeout will expire before the next time we're called (1000ms
     from now, plus some slack), then calculate the number of milliseconds
     from now which we should send padding, so we can schedule a callback
     then.
   */
  if (ms_till_pad < (TOR_HOUSEKEEPING_CALLBACK_MSEC +
                       TOR_HOUSEKEEPING_CALLBACK_SLACK_MSEC)) {
    /* If the padding time is in the past, that means that libevent delayed
     * calling the once-per-second callback due to other work taking too long.
     * See https://bugs.torproject.org/22212 and
     * https://bugs.torproject.org/16585. This is a systemic problem
     * with being single-threaded, but let's emit a notice if this
     * is long enough in the past that we might have missed a netflow window,
     * and allowed a router to emit a netflow frame, just so we don't forget
     * about it entirely.. */
#define NETFLOW_MISSED_WINDOW (150000 - DFLT_NETFLOW_INACTIVE_KEEPALIVE_HIGH)
    if (ms_till_pad < 0) {
      int severity = (ms_till_pad < -NETFLOW_MISSED_WINDOW)
                      ? LOG_NOTICE : LOG_INFO;
      log_fn(severity, LD_OR,
              "Channel padding timeout scheduled %"PRId64"ms in the past. ",
             (-ms_till_pad));
      return 0; /* Clock jumped: Send padding now */
    }

    return ms_till_pad;
  }
  return CHANNELPADDING_TIME_LATER;
}

/**
 * Returns a randomized value for channel idle timeout in seconds.
 * The channel idle timeout governs how quickly we close a channel
 * after its last circuit has disappeared.
 *
 * There are three classes of channels:
 *  1. Client+non-canonical. These live for 3-4.5 minutes
 *  2. relay to relay. These live for 45-75 min by default
 *  3. Reduced padding clients. These live for 1.5-2.25 minutes.
 *
 * Also allows the default relay-to-relay value to be controlled by the
 * consensus.
 */
unsigned int
channelpadding_get_channel_idle_timeout(const channel_t *chan,
                                        int is_canonical)
{
  const or_options_t *options = get_options();
  unsigned int timeout;

  /* Non-canonical and client channels only last for 3-4.5 min when idle */
  if (!is_canonical || CHANNEL_IS_CLIENT(chan, options)) {
#define CONNTIMEOUT_CLIENTS_BASE 180 // 3 to 4.5 min
    timeout = CONNTIMEOUT_CLIENTS_BASE
        + crypto_rand_int(CONNTIMEOUT_CLIENTS_BASE/2);
  } else { // Canonical relay-to-relay channels
    // 45..75min or consensus +/- 25%
    timeout = consensus_nf_conntimeout_relays;
    timeout = 3*timeout/4 + crypto_rand_int(timeout/2);
  }

  /* If ReducedConnectionPadding is set, we want to halve the duration of
   * the channel idle timeout, since reducing the additional time that
   * a channel stays open will reduce the total overhead for making
   * new channels. This reduction in overhead/channel expense
   * is important for mobile users. The option cannot be set by relays.
   *
   * We also don't reduce any values for timeout that the user explicitly
   * set.
   */
  if (options->ReducedConnectionPadding
      && !options->CircuitsAvailableTimeout) {
    timeout /= 2;
  }

  return timeout;
}

/**
 * This function controls how long we keep idle circuits open,
 * and how long we build predicted circuits. This behavior is under
 * the control of channelpadding because circuit availability is the
 * dominant factor in channel lifespan, which influences total padding
 * overhead.
 *
 * Returns a randomized number of seconds in a range from
 * CircuitsAvailableTimeout to 2*CircuitsAvailableTimeout. This value is halved
 * if ReducedConnectionPadding is set. The default value of
 * CircuitsAvailableTimeout can be controlled by the consensus.
 */
int
channelpadding_get_circuits_available_timeout(void)
{
  const or_options_t *options = get_options();
  int timeout = options->CircuitsAvailableTimeout;

  if (!timeout) {
    timeout = consensus_nf_conntimeout_clients;

    /* If ReducedConnectionPadding is set, we want to halve the duration of
     * the channel idle timeout, since reducing the additional time that
     * a channel stays open will reduce the total overhead for making
     * new connections. This reduction in overhead/connection expense
     * is important for mobile users. The option cannot be set by relays.
     *
     * We also don't reduce any values for timeout that the user explicitly
     * set.
     */
    if (options->ReducedConnectionPadding) {
      // half the value to 15..30min by default
      timeout /= 2;
    }
  }

  // 30..60min by default
  timeout = timeout + crypto_rand_int(timeout);

  tor_assert(timeout >= 0);

  return timeout;
}

/**
 * Calling this function on a channel causes it to tell the other side
 * not to send padding, and disables sending padding from this side as well.
 */
void
channelpadding_disable_padding_on_channel(channel_t *chan)
{
  chan->padding_enabled = 0;

  // Send cell to disable padding on the other end
  channelpadding_send_disable_command(chan);
}

/**
 * Calling this function on a channel causes it to tell the other side
 * not to send padding, and reduces the rate that padding is sent from
 * this side.
 */
void
channelpadding_reduce_padding_on_channel(channel_t *chan)
{
  /* Padding can be forced and reduced by clients, regardless of if
   * the channel supports it. So we check for support here before
   * sending any commands. */
  if (chan->padding_enabled) {
    channelpadding_send_disable_command(chan);
  }

  chan->padding_timeout_low_ms = consensus_nf_ito_low_reduced;
  chan->padding_timeout_high_ms = consensus_nf_ito_high_reduced;

  log_fn(LOG_INFO,LD_OR,
         "Reduced padding on channel %"PRIu64": lo=%d, hi=%d",
         (chan->global_identifier),
         chan->padding_timeout_low_ms, chan->padding_timeout_high_ms);
}

/**
 * This function is called once per second by run_connection_housekeeping(),
 * but only if the channel is still open, valid, and non-wedged.
 *
 * It decides if and when we should send a padding cell, and if needed,
 * schedules a callback to send that cell at the appropriate time.
 *
 * Returns an enum that represents the current padding decision state.
 * Return value is currently used only by unit tests.
 */
channelpadding_decision_t
channelpadding_decide_to_pad_channel(channel_t *chan)
{
  const or_options_t *options = get_options();

  /* Only pad open channels */
  if (chan->state != CHANNEL_STATE_OPEN)
    return CHANNELPADDING_WONTPAD;

  if (chan->channel_usage == CHANNEL_USED_FOR_FULL_CIRCS) {
    if (!consensus_nf_pad_before_usage)
      return CHANNELPADDING_WONTPAD;
  } else if (chan->channel_usage != CHANNEL_USED_FOR_USER_TRAFFIC) {
    return CHANNELPADDING_WONTPAD;
  }

  if (chan->pending_padding_callback)
    return CHANNELPADDING_PADDING_ALREADY_SCHEDULED;

  /* Don't pad the channel if we didn't negotiate it, but still
   * allow clients to force padding if options->ChannelPadding is
   * explicitly set to 1.
   */
  if (!chan->padding_enabled && options->ConnectionPadding != 1) {
    return CHANNELPADDING_WONTPAD;
  }

  if (rend_service_allow_non_anonymous_connection(options) &&
      !consensus_nf_pad_single_onion) {
    /* If the consensus just changed values, this channel may still
     * think padding is enabled. Negotiate it off. */
    if (chan->padding_enabled)
      channelpadding_disable_padding_on_channel(chan);

    return CHANNELPADDING_WONTPAD;
  }

  /* There should always be a cmux on the circuit. After that,
   * only schedule padding if there are no queued writes and no
   * queued cells in circuitmux queues. */
  if (chan->cmux && !chan->has_queued_writes(chan) &&
      !circuitmux_num_cells(chan->cmux)) {
    int is_client_channel = 0;

    if (CHANNEL_IS_CLIENT(chan, options)) {
       is_client_channel = 1;
    }

    /* If nf_pad_relays=1 is set in the consensus, we pad
     * on *all* idle connections, relay-relay or relay-client.
     * Otherwise pad only for client+bridge cons */
    if (is_client_channel || consensus_nf_pad_relays) {
      int64_t pad_time_ms =
          channelpadding_compute_time_until_pad_for_netflow(chan);

      if (pad_time_ms == CHANNELPADDING_TIME_DISABLED) {
        return CHANNELPADDING_WONTPAD;
      } else if (pad_time_ms == CHANNELPADDING_TIME_LATER) {
        chan->currently_padding = 1;
        return CHANNELPADDING_PADLATER;
      } else {
        if (BUG(pad_time_ms > INT_MAX)) {
          pad_time_ms = INT_MAX;
        }
       /* We have to schedule a callback because we're called exactly once per
        * second, but we don't want padding packets to go out exactly on an
        * integer multiple of seconds. This callback will only be scheduled
        * if we're within 1.1 seconds of the padding time.
        */
        chan->currently_padding = 1;
        return channelpadding_schedule_padding(chan, (int)pad_time_ms);
      }
    } else {
      chan->currently_padding = 0;
      return CHANNELPADDING_WONTPAD;
    }
  } else {
    return CHANNELPADDING_PADLATER;
  }
}
