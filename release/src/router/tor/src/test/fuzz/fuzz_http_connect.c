/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define BUFFERS_PRIVATE
#define CONNECTION_EDGE_PRIVATE

#include "core/or/or.h"
#include "lib/err/backtrace.h"
#include "lib/buf/buffers.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_edge.h"
#include "core/proto/proto_socks.h"
#include "lib/log/log.h"

#include "core/or/entry_connection_st.h"
#include "core/or/socks_request_st.h"

#include "test/fuzz/fuzzing.h"

static void
mock_connection_write_to_buf_impl_(const char *string, size_t len,
                                   connection_t *conn, int compressed)
{
  log_debug(LD_GENERAL, "%sResponse:\n%u\nConnection: %p\n%s\n",
            compressed ? "Compressed " : "", (unsigned)len, conn, string);
}

static void
mock_connection_mark_unattached_ap_(entry_connection_t *conn, int endreason,
                                    int line, const char *file)
{
  (void)conn;
  (void)endreason;
  (void)line;
  (void)file;
}

static int
mock_connection_ap_rewrite_and_attach_if_allowed(entry_connection_t *conn,
                                                 origin_circuit_t *circ,
                                                 crypt_path_t *cpath)
{
  (void)conn;
  (void)circ;
  (void)cpath;
  return 0;
}

int
fuzz_init(void)
{
  /* Set up fake response handler */
  MOCK(connection_write_to_buf_impl_, mock_connection_write_to_buf_impl_);
  /* Set up the fake handler functions */
  MOCK(connection_mark_unattached_ap_, mock_connection_mark_unattached_ap_);
  MOCK(connection_ap_rewrite_and_attach_if_allowed,
       mock_connection_ap_rewrite_and_attach_if_allowed);

  return 0;
}

int
fuzz_cleanup(void)
{
  UNMOCK(connection_write_to_buf_impl_);
  UNMOCK(connection_mark_unattached_ap_);
  UNMOCK(connection_ap_rewrite_and_attach_if_allowed);
  return 0;
}

int
fuzz_main(const uint8_t *stdin_buf, size_t data_size)
{
  entry_connection_t conn;

  /* Set up the fake connection */
  memset(&conn, 0, sizeof(conn));
  conn.edge_.base_.type = CONN_TYPE_AP;
  conn.edge_.base_.state = AP_CONN_STATE_HTTP_CONNECT_WAIT;
  conn.socks_request = tor_malloc_zero(sizeof(socks_request_t));
  conn.socks_request->listener_type = CONN_TYPE_AP_HTTP_CONNECT_LISTENER;

  conn.edge_.base_.inbuf = buf_new_with_data((char*)stdin_buf, data_size);
  if (!conn.edge_.base_.inbuf) {
    log_debug(LD_GENERAL, "Zero-Length-Input\n");
    goto done;
  }

  /* Parse the headers */
  int rv = connection_ap_process_http_connect(&conn);

  /* TODO: check the output is correctly parsed based on the input */

  log_debug(LD_GENERAL, "Result:\n%d\n", rv);

  goto done;

 done:
  /* Reset. */
  socks_request_free(conn.socks_request);
  buf_free(conn.edge_.base_.inbuf);
  conn.edge_.base_.inbuf = NULL;

  return 0;
}

