/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"

#define BUFFERS_PRIVATE
#include "core/or/or.h"

#include "lib/buf/buffers.h"
#include "lib/err/backtrace.h"
#include "lib/log/log.h"
#include "core/proto/proto_socks.h"
#include "feature/client/addressmap.h"

#include "test/fuzz/fuzzing.h"

int
fuzz_init(void)
{
  addressmap_init();
  return 0;
}

int
fuzz_cleanup(void)
{
  addressmap_free_all();
  return 0;
}

int
fuzz_main(const uint8_t *stdin_buf, size_t data_size)
{
  buf_t *buffer = buf_new_with_data((char*)stdin_buf, data_size);
  if (!buffer) {
    tor_assert(data_size==0);
    buffer = buf_new();
  }

  socks_request_t *request = socks_request_new();

  int r = fetch_from_buf_socks(buffer, request, 0, 0);
  log_info(LD_GENERAL, "Socks request status: %d", r);

  /* Reset. */
  buf_free(buffer);
  socks_request_free(request);

  return 0;
}
