/* Copyright (c) 2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_conflux_cell.
 * \brief Test conflux cells.
 */

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"

#include "core/or/conflux_cell.h"
#include "core/or/conflux_st.h"
#include "trunnel/conflux.h"

#include "lib/crypt_ops/crypto_rand.h"

static void
test_link(void *arg)
{
  cell_t cell;
  conflux_cell_link_t link;
  conflux_cell_link_t *decoded_link = NULL;

  (void) arg;

  memset(&link, 0, sizeof(link));

  link.desired_ux = CONFLUX_UX_HIGH_THROUGHPUT;
  link.last_seqno_recv = 0;
  link.last_seqno_sent = 0;
  link.version = 0x01;

  crypto_rand((char *) link.nonce, sizeof(link.nonce));

  ssize_t cell_len = build_link_cell(&link, cell.payload+RELAY_HEADER_SIZE);
  tt_int_op(cell_len, OP_GT, 0);

  decoded_link = conflux_cell_parse_link(&cell, cell_len);
  tt_assert(decoded_link);

  uint8_t buf[RELAY_PAYLOAD_SIZE];
  ssize_t enc_cell_len = build_link_cell(decoded_link, buf);
  tt_int_op(cell_len, OP_EQ, enc_cell_len);

  /* Validate the original link object with the decoded one. */
  tt_mem_op(&link, OP_EQ, decoded_link, sizeof(link));
  tor_free(decoded_link);

 done:
  tor_free(decoded_link);
}

struct testcase_t conflux_cell_tests[] = {
  { "link", test_link, TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};

