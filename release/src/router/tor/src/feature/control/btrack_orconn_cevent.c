/* Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file btrack_orconn_cevent.c
 * \brief Emit bootstrap status events for OR connections
 *
 * We do some decoding of the raw OR_CONN_STATE_* values.  For
 * example, OR_CONN_STATE_CONNECTING means the first TCP connect()
 * completing, regardless of whether it's directly to a relay instead
 * of a proxy or a PT.
 **/

#include <stdbool.h>

#include "core/or/or.h"

#define BTRACK_ORCONN_PRIVATE

#include "core/or/orconn_event.h"
#include "feature/control/btrack_orconn.h"
#include "feature/control/btrack_orconn_cevent.h"
#include "feature/control/control_events.h"

/**
 * Have we completed our first OR connection?
 *
 * Block display of application circuit progress until we do, to avoid
 * some misleading behavior of jumping to high progress.
 **/
static bool bto_first_orconn = false;

/** Is the ORCONN using a pluggable transport? */
static bool
using_pt(const bt_orconn_t *bto)
{
  return bto->proxy_type == PROXY_PLUGGABLE;
}

/** Is the ORCONN using a non-PT proxy? */
static bool
using_proxy(const bt_orconn_t *bto)
{
  switch (bto->proxy_type) {
  case PROXY_CONNECT:
  case PROXY_SOCKS4:
  case PROXY_SOCKS5:
  case PROXY_HAPROXY:
    return true;
  default:
    return false;
  }
}

/**
 * Emit control events when we have updated our idea of the best state
 * that any OR connection has reached.
 *
 * Do some decoding of the ORCONN states depending on whether a PT or
 * a proxy is in use.
 **/
void
bto_cevent_anyconn(const bt_orconn_t *bto)
{
  switch (bto->state) {
  case OR_CONN_STATE_CONNECTING:
    /* Exactly what kind of thing we're connecting to isn't
     * information we directly get from the states in connection_or.c,
     * so decode it here. */
    if (using_pt(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_CONN_PT, 0);
    else if (using_proxy(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_CONN_PROXY, 0);
    else
      control_event_bootstrap(BOOTSTRAP_STATUS_CONN, 0);
    break;
  case OR_CONN_STATE_PROXY_HANDSHAKING:
    /* Similarly, starting a proxy handshake means the TCP connect()
     * succeeded to the proxy.  Let's be specific about what kind of
     * proxy. */
    if (using_pt(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_CONN_DONE_PT, 0);
    else if (using_proxy(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_CONN_DONE_PROXY, 0);
    break;
  case OR_CONN_STATE_TLS_HANDSHAKING:
    control_event_bootstrap(BOOTSTRAP_STATUS_CONN_DONE, 0);
    break;
  case OR_CONN_STATE_TLS_CLIENT_RENEGOTIATING:
  case OR_CONN_STATE_OR_HANDSHAKING_V2:
  case OR_CONN_STATE_OR_HANDSHAKING_V3:
    control_event_bootstrap(BOOTSTRAP_STATUS_HANDSHAKE, 0);
    break;
  case OR_CONN_STATE_OPEN:
    control_event_bootstrap(BOOTSTRAP_STATUS_HANDSHAKE_DONE, 0);
    /* Unblock directory progress display */
    control_event_boot_first_orconn();
    /* Unblock apconn progress display */
    bto_first_orconn = true;
    break;
  default:
    break;
  }
}

/**
 * Emit control events when we have updated our idea of the best state
 * that any application circuit OR connection has reached.
 *
 * Do some decoding of the ORCONN states depending on whether a PT or
 * a proxy is in use.
 **/
void
bto_cevent_apconn(const bt_orconn_t *bto)
{
  if (!bto_first_orconn)
    return;

  switch (bto->state) {
  case OR_CONN_STATE_CONNECTING:
    /* Exactly what kind of thing we're connecting to isn't
     * information we directly get from the states in connection_or.c,
     * so decode it here. */
    if (using_pt(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_AP_CONN_PT, 0);
    else if (using_proxy(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_AP_CONN_PROXY, 0);
    else
      control_event_bootstrap(BOOTSTRAP_STATUS_AP_CONN, 0);
    break;
  case OR_CONN_STATE_PROXY_HANDSHAKING:
    /* Similarly, starting a proxy handshake means the TCP connect()
     * succeeded to the proxy.  Let's be specific about what kind of
     * proxy. */
    if (using_pt(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_AP_CONN_DONE_PT, 0);
    else if (using_proxy(bto))
      control_event_bootstrap(BOOTSTRAP_STATUS_AP_CONN_DONE_PROXY, 0);
    break;
  case OR_CONN_STATE_TLS_HANDSHAKING:
    control_event_bootstrap(BOOTSTRAP_STATUS_AP_CONN_DONE, 0);
    break;
  case OR_CONN_STATE_TLS_CLIENT_RENEGOTIATING:
  case OR_CONN_STATE_OR_HANDSHAKING_V2:
  case OR_CONN_STATE_OR_HANDSHAKING_V3:
    control_event_bootstrap(BOOTSTRAP_STATUS_AP_HANDSHAKE, 0);
    break;
  case OR_CONN_STATE_OPEN:
    control_event_bootstrap(BOOTSTRAP_STATUS_AP_HANDSHAKE_DONE, 0);
    break;
  default:
    break;
  }
}

/** Forget that we completed our first OR connection */
void
bto_cevent_reset(void)
{
  bto_first_orconn = false;
}
