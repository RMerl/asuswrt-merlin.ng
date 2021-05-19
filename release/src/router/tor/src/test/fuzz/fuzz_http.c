/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define BUFFERS_PRIVATE
#define DIRCACHE_PRIVATE

#include "core/or/or.h"
#include "lib/err/backtrace.h"
#include "lib/buf/buffers.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "feature/dircache/dircache.h"
#include "lib/log/log.h"

#include "feature/dircommon/dir_connection_st.h"

#include "test/fuzz/fuzzing.h"

static void
mock_connection_write_to_buf_impl_(const char *string, size_t len,
                                   connection_t *conn, int compressed)
{
  log_debug(LD_GENERAL, "%sResponse:\n%u\nConnection: %p\n%s\n",
            compressed ? "Compressed " : "", (unsigned)len, conn, string);
}

static int
mock_directory_handle_command_get(dir_connection_t *conn,
                                      const char *headers,
                                      const char *body,
                                      size_t body_len)
{
  (void)conn;

  log_debug(LD_GENERAL, "Method:\nGET\n");

  if (headers) {
    log_debug(LD_GENERAL, "Header-Length:\n%u\n", (unsigned)strlen(headers));
    log_debug(LD_GENERAL, "Headers:\n%s\n", headers);
  }

  log_debug(LD_GENERAL, "Body-Length:\n%u\n", (unsigned)body_len);
  if (body) {
    log_debug(LD_GENERAL, "Body:\n%s\n", body);
  }

  /* Always tell the caller we succeeded */
  return 0;
}

static int
mock_directory_handle_command_post(dir_connection_t *conn,
                                       const char *headers,
                                       const char *body,
                                       size_t body_len)
{
  (void)conn;

  log_debug(LD_GENERAL, "Method:\nPOST\n");

  if (headers) {
    log_debug(LD_GENERAL, "Header-Length:\n%u\n", (unsigned)strlen(headers));
    log_debug(LD_GENERAL, "Headers:\n%s\n", headers);
  }

  log_debug(LD_GENERAL, "Body-Length:\n%u\n", (unsigned)body_len);
  if (body) {
    log_debug(LD_GENERAL, "Body:\n%s\n", body);
  }

  /* Always tell the caller we succeeded */
  return 0;
}

int
fuzz_init(void)
{
  /* Set up fake response handler */
  MOCK(connection_write_to_buf_impl_, mock_connection_write_to_buf_impl_);
  /* Set up the fake handler functions */
  MOCK(directory_handle_command_get, mock_directory_handle_command_get);
  MOCK(directory_handle_command_post, mock_directory_handle_command_post);

  return 0;
}

int
fuzz_cleanup(void)
{
  UNMOCK(connection_write_to_buf_impl_);
  UNMOCK(directory_handle_command_get);
  UNMOCK(directory_handle_command_post);
  return 0;
}

int
fuzz_main(const uint8_t *stdin_buf, size_t data_size)
{
  dir_connection_t dir_conn;

  /* Set up the fake connection */
  memset(&dir_conn, 0, sizeof(dir_connection_t));
  dir_conn.base_.type = CONN_TYPE_DIR;
  /* Apparently tor sets this before directory_handle_command() is called. */
  dir_conn.base_.address = tor_strdup("replace-this-address.example.com");

  dir_conn.base_.inbuf = buf_new_with_data((char*)stdin_buf, data_size);
  if (!dir_conn.base_.inbuf) {
    log_debug(LD_GENERAL, "Zero-Length-Input\n");
    goto done;
  }

  /* Parse the headers */
  int rv = directory_handle_command(&dir_conn);

  /* TODO: check the output is correctly parsed based on the input */

  /* Report the parsed origin address */
  if (dir_conn.base_.address) {
    log_debug(LD_GENERAL, "Address:\n%s\n", dir_conn.base_.address);
  }

  log_debug(LD_GENERAL, "Result:\n%d\n", rv);

 done:
  /* Reset. */
  tor_free(dir_conn.base_.address);
  buf_free(dir_conn.base_.inbuf);
  dir_conn.base_.inbuf = NULL;

  return 0;
}
