/* Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file metrics.c
 * @brief Metrics subsystem.
 **/

#include "orconfig.h"

#include "core/or/or.h"

#include "lib/encoding/confline.h"
#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"
#include "lib/metrics/metrics_store.h"
#include "lib/net/resolve.h"
#include "lib/string/printf.h"
#include "lib/net/nettypes.h"
#include "lib/net/address.h"

#include "core/mainloop/connection.h"
#include "core/or/connection_or.h"
#include "core/or/connection_st.h"
#include "core/or/policies.h"
#include "core/or/port_cfg_st.h"
#include "core/proto/proto_http.h"

#include "feature/dircommon/directory.h"
#include "feature/metrics/metrics.h"

#include "app/config/config.h"
#include "app/main/subsysmgr.h"

/** Metrics format driver set by the MetricsPort option. */
static metrics_format_t the_format = METRICS_FORMAT_PROMETHEUS;

/** Return true iff the given peer address is allowed by our MetricsPortPolicy
 * option that is is in that list. */
static bool
metrics_request_allowed(const tor_addr_t *peer_addr)
{
  tor_assert(peer_addr);

  return metrics_policy_permits_address(peer_addr);
}

/** Helper: For a metrics port connection, write the HTTP response header
 * using the data length passed. */
static void
write_metrics_http_response(const size_t data_len, connection_t *conn)
{
  char date[RFC1123_TIME_LEN+1];
  buf_t *buf = buf_new_with_capacity(128 + data_len);

  format_rfc1123_time(date, approx_time());
  buf_add_printf(buf, "HTTP/1.0 200 OK\r\nDate: %s\r\n", date);
  buf_add_printf(buf, "Content-Type: text/plain; charset=utf-8\r\n");
  buf_add_printf(buf, "Content-Length: %" TOR_PRIuSZ "\r\n", data_len);
  buf_add_string(buf, "\r\n");

  connection_buf_add_buf(conn, buf);
  buf_free(buf);
}

/** Return newly allocated buffer containing the output of all subsystems
 * having metrics.
 *
 * This is used to output the content on the MetricsPort. */
buf_t *
metrics_get_output(const metrics_format_t fmt)
{
  buf_t *data = buf_new();

  /* Go over all subsystems that exposes a metrics store. */
  for (unsigned i = 0; i < n_tor_subsystems; ++i) {
    const smartlist_t *stores;
    const subsys_fns_t *sys = tor_subsystems[i];

    /* Skip unsupported subsystems. */
    if (!sys->supported) {
      continue;
    }

    if (sys->get_metrics && (stores = sys->get_metrics())) {
      SMARTLIST_FOREACH_BEGIN(stores, const metrics_store_t *, store) {
        metrics_store_get_output(fmt, store, data);
      } SMARTLIST_FOREACH_END(store);
    }
  }

  return data;
}

/** Process what is in the inbuf of this connection of type metrics.
 *
 * Return 0 on success else -1 on error for which the connection is marked for
 * close. */
int
metrics_connection_process_inbuf(connection_t *conn)
{
  int ret = -1;
  char *headers = NULL, *command = NULL, *url = NULL;
  const char *errmsg = NULL;

  tor_assert(conn);
  tor_assert(conn->type == CONN_TYPE_METRICS);

  if (!metrics_request_allowed(&conn->addr)) {
    /* Close connection. Don't bother returning anything if you are not
     * allowed by being on the policy list. */
    errmsg = NULL;
    goto err;
  }

  const int http_status =
    connection_fetch_from_buf_http(conn, &headers, 1024, NULL, NULL, 1024, 0);
  if (http_status < 0) {
    errmsg = "HTTP/1.0 400 Bad Request\r\n\r\n";
    goto err;
  } else if (http_status == 0) {
    /* no HTTP request yet. */
    ret = 0;
    goto done;
  }

  const int cmd_status = parse_http_command(headers, &command, &url);
  if (cmd_status < 0) {
    errmsg = "HTTP/1.0 400 Bad Request\r\n\r\n";
    goto err;
  } else if (strcmpstart(command, "GET")) {
    errmsg = "HTTP/1.0 405 Method Not Allowed\r\n\r\n";
    goto err;
  }
  tor_assert(url);

  /* Where we expect the query to come for. */
#define EXPECTED_URL_PATH "/metrics"
#define EXPECTED_URL_PATH_LEN (sizeof(EXPECTED_URL_PATH) - 1) /* No NUL */

  if (!strcmpstart(url, EXPECTED_URL_PATH) &&
      strlen(url) == EXPECTED_URL_PATH_LEN) {
    buf_t *data = metrics_get_output(the_format);

    write_metrics_http_response(buf_datalen(data), conn);
    connection_buf_add_buf(conn, data);
    buf_free(data);
  } else {
    errmsg = "HTTP/1.0 404 Not Found\r\n\r\n";
    goto err;
  }

  ret = 0;
  goto done;

 err:
  if (errmsg) {
    log_info(LD_EDGE, "HTTP metrics error: saying %s", escaped(errmsg));
    connection_buf_add(errmsg, strlen(errmsg), conn);
  }
  connection_mark_and_flush(conn);

 done:
  tor_free(headers);
  tor_free(command);
  tor_free(url);

  return ret;
}

/** Parse metrics ports from options. On success, add the port to the ports
 * list and return 0. On failure, set err_msg_out to a newly allocated string
 * describing the problem and return -1. */
int
metrics_parse_ports(or_options_t *options, smartlist_t *ports,
                    char **err_msg_out)
{
  int num_elems, ok = 0, ret = -1;
  const char *addrport_str = NULL, *fmt_str = NULL;
  smartlist_t *elems = NULL;
  port_cfg_t *cfg = NULL;

  tor_assert(options);
  tor_assert(ports);

  /* No metrics port to configure, just move on . */
  if (!options->MetricsPort_lines) {
    return 0;
  }

  elems = smartlist_new();

  /* Split between the protocol and the address/port. */
  num_elems = smartlist_split_string(elems,
                                     options->MetricsPort_lines->value, " ",
                                     SPLIT_SKIP_SPACE | SPLIT_IGNORE_BLANK, 2);
  if (num_elems < 1) {
    *err_msg_out = tor_strdup("MetricsPort is missing port.");
    goto end;
  }

  addrport_str = smartlist_get(elems, 0);
  if (num_elems >= 2) {
    /* Parse the format if any. */
    fmt_str = smartlist_get(elems, 1);
    if (!strcasecmp(fmt_str, "prometheus")) {
      the_format = METRICS_FORMAT_PROMETHEUS;
    } else {
      tor_asprintf(err_msg_out, "MetricsPort unknown format: %s", fmt_str);
      goto end;
    }
  }

  /* Port configuration with default address. */
  cfg = port_cfg_new(0);
  cfg->type = CONN_TYPE_METRICS_LISTENER;

  /* Parse the port first. Then an address if any can be found. */
  cfg->port = (int) tor_parse_long(addrport_str, 10, 0, 65535, &ok, NULL);
  if (ok) {
    tor_addr_parse(&cfg->addr, "127.0.0.1");
  } else {
    /* We probably have a host:port situation */
    if (tor_addr_port_lookup(addrport_str, &cfg->addr,
                             (uint16_t *) &cfg->port) < 0) {
      *err_msg_out = tor_strdup("MetricsPort address/port failed to parse or "
                                "resolve.");
      goto end;
    }
  }
  /* Add it to the ports list. */
  smartlist_add(ports, cfg);

  /* It is set. MetricsPort doesn't support the NoListen options or such that
   * would prevent from being a real listener port. */
  options->MetricsPort_set = 1;

  /* Success. */
  ret = 0;

 end:
  if (ret != 0) {
    port_cfg_free(cfg);
  }
  SMARTLIST_FOREACH(elems, char *, e, tor_free(e));
  smartlist_free(elems);
  return ret;
}

/** Called when conn has gotten its socket closed. */
int
metrics_connection_reached_eof(connection_t *conn)
{
  tor_assert(conn);

  log_info(LD_EDGE, "Metrics connection reached EOF. Closing.");
  connection_mark_for_close(conn);
  return 0;
}

/** Called when conn has no more bytes left on its outbuf. Return 0 indicating
 * success. */
int
metrics_connection_finished_flushing(connection_t *conn)
{
  tor_assert(conn);
  return 0;
}

/** Initialize the subsystem. */
void
metrics_init(void)
{
}

/** Cleanup and free any global memory of this subsystem. */
void
metrics_cleanup(void)
{
}
