/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/* Unit tests for handling different kinds of relay cell */

#define CIRCUITLIST_PRIVATE
#define NETWORKSTATUS_PRIVATE
#define SENDME_PRIVATE
#define RELAY_PRIVATE

#include "core/or/circuit_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/circuitlist.h"
#include "core/or/relay.h"
#include "core/or/sendme.h"

#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/networkstatus_st.h"

#include "lib/crypt_ops/crypto_digest.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

static void
setup_mock_consensus(void)
{
  current_md_consensus = current_ns_consensus =
    tor_malloc_zero(sizeof(networkstatus_t));
  current_md_consensus->net_params = smartlist_new();
  current_md_consensus->routerstatus_list = smartlist_new();
}

static void
free_mock_consensus(void)
{
  SMARTLIST_FOREACH(current_md_consensus->routerstatus_list, void *, r,
                    tor_free(r));
  smartlist_free(current_md_consensus->routerstatus_list);
  smartlist_free(current_ns_consensus->net_params);
  tor_free(current_ns_consensus);
}

static void
test_v1_record_digest(void *arg)
{
  or_circuit_t *or_circ = NULL;
  circuit_t *circ = NULL;

  (void) arg;

  /* Create our dummy circuit. */
  or_circ = or_circuit_new(1, NULL);
  /* Points it to the OR circuit now. */
  circ = TO_CIRCUIT(or_circ);

  /* The package window has to be a multiple of CIRCWINDOW_INCREMENT minus 1
   * in order to catch the CIRCWINDOW_INCREMENT-nth cell. Try something that
   * shouldn't be noted. */
  circ->package_window = CIRCWINDOW_INCREMENT;
  sendme_record_cell_digest_on_circ(circ, NULL);
  tt_assert(!circ->sendme_last_digests);

  /* This should work now. Package window at CIRCWINDOW_INCREMENT + 1. */
  circ->package_window++;
  sendme_record_cell_digest_on_circ(circ, NULL);
  tt_assert(circ->sendme_last_digests);
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 1);

  /* Next cell in the package window shouldn't do anything. */
  circ->package_window++;
  sendme_record_cell_digest_on_circ(circ, NULL);
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 1);

  /* The next CIRCWINDOW_INCREMENT should add one more digest. */
  circ->package_window = (CIRCWINDOW_INCREMENT * 2) + 1;
  sendme_record_cell_digest_on_circ(circ, NULL);
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 2);

 done:
  circuit_free_(circ);
}

static void
test_v1_consensus_params(void *arg)
{
  (void) arg;

  setup_mock_consensus();
  tt_assert(current_md_consensus);

  /* Both zeroes. */
  smartlist_add(current_md_consensus->net_params,
                (void *) "sendme_emit_min_version=0");
  smartlist_add(current_md_consensus->net_params,
                (void *) "sendme_accept_min_version=0");
  tt_int_op(get_emit_min_version(), OP_EQ, 0);
  tt_int_op(get_accept_min_version(), OP_EQ, 0);
  smartlist_clear(current_md_consensus->net_params);

  /* Both ones. */
  smartlist_add(current_md_consensus->net_params,
                (void *) "sendme_emit_min_version=1");
  smartlist_add(current_md_consensus->net_params,
                (void *) "sendme_accept_min_version=1");
  tt_int_op(get_emit_min_version(), OP_EQ, 1);
  tt_int_op(get_accept_min_version(), OP_EQ, 1);
  smartlist_clear(current_md_consensus->net_params);

  /* Different values from each other. */
  smartlist_add(current_md_consensus->net_params,
                (void *) "sendme_emit_min_version=1");
  smartlist_add(current_md_consensus->net_params,
                (void *) "sendme_accept_min_version=0");
  tt_int_op(get_emit_min_version(), OP_EQ, 1);
  tt_int_op(get_accept_min_version(), OP_EQ, 0);
  smartlist_clear(current_md_consensus->net_params);

  /* Validate is the cell version is coherent with our internal default value
   * and the one in the consensus. */
  smartlist_add(current_md_consensus->net_params,
                (void *) "sendme_accept_min_version=1");
  /* Minimum acceptable value is 1. */
  tt_int_op(cell_version_can_be_handled(1), OP_EQ, true);
  /* Minimum acceptable value is 1 so a cell version of 0 is refused. */
  tt_int_op(cell_version_can_be_handled(0), OP_EQ, false);

 done:
  free_mock_consensus();
}

static void
test_v1_build_cell(void *arg)
{
  uint8_t payload[RELAY_PAYLOAD_SIZE], digest[DIGEST_LEN];
  ssize_t ret;
  crypto_digest_t *cell_digest = NULL;
  or_circuit_t *or_circ = NULL;
  circuit_t *circ = NULL;

  (void) arg;

  or_circ = or_circuit_new(1, NULL);
  circ = TO_CIRCUIT(or_circ);
  circ->sendme_last_digests = smartlist_new();

  cell_digest = crypto_digest_new();
  tt_assert(cell_digest);
  crypto_digest_add_bytes(cell_digest, "AAAAAAAAAAAAAAAAAAAA", 20);
  crypto_digest_get_digest(cell_digest, (char *) digest, sizeof(digest));
  smartlist_add(circ->sendme_last_digests, tor_memdup(digest, sizeof(digest)));

  /* SENDME v1 payload is 3 bytes + 20 bytes digest. See spec. */
  ret = build_cell_payload_v1(digest, payload);
  tt_int_op(ret, OP_EQ, 23);

  /* Validation. */

  /* An empty payload means SENDME version 0 thus valid. */
  tt_int_op(sendme_is_valid(circ, payload, 0), OP_EQ, true);
  /* Current phoney digest should have been popped. */
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 0);

  /* An unparseable cell means invalid. */
  setup_full_capture_of_logs(LOG_INFO);
  tt_int_op(sendme_is_valid(circ, (const uint8_t *) "A", 1), OP_EQ, false);
  expect_log_msg_containing("Unparseable SENDME cell received. "
                            "Closing circuit.");
  teardown_capture_of_logs();

  /* No cell digest recorded for this. */
  setup_full_capture_of_logs(LOG_INFO);
  tt_int_op(sendme_is_valid(circ, payload, sizeof(payload)), OP_EQ, false);
  expect_log_msg_containing("We received a SENDME but we have no cell digests "
                            "to match. Closing circuit.");
  teardown_capture_of_logs();

  /* Note the wrong digest in the circuit, cell should fail validation. */
  circ->package_window = CIRCWINDOW_INCREMENT + 1;
  sendme_record_cell_digest_on_circ(circ, NULL);
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 1);
  setup_full_capture_of_logs(LOG_INFO);
  tt_int_op(sendme_is_valid(circ, payload, sizeof(payload)), OP_EQ, false);
  /* After a validation, the last digests is always popped out. */
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 0);
  expect_log_msg_containing("SENDME v1 cell digest do not match.");
  teardown_capture_of_logs();

  /* Record the cell digest into the circuit, cell should validate. */
  memcpy(or_circ->crypto.sendme_digest, digest, sizeof(digest));
  circ->package_window = CIRCWINDOW_INCREMENT + 1;
  sendme_record_cell_digest_on_circ(circ, NULL);
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 1);
  tt_int_op(sendme_is_valid(circ, payload, sizeof(payload)), OP_EQ, true);
  /* After a validation, the last digests is always popped out. */
  tt_int_op(smartlist_len(circ->sendme_last_digests), OP_EQ, 0);

 done:
  crypto_digest_free(cell_digest);
  circuit_free_(circ);
}

static void
test_cell_payload_pad(void *arg)
{
  size_t pad_offset, payload_len, expected_offset;

  (void) arg;

  /* Offset should be 0, not enough room for padding. */
  payload_len = RELAY_PAYLOAD_SIZE;
  pad_offset = get_pad_cell_offset(payload_len);
  tt_int_op(pad_offset, OP_EQ, 0);
  tt_int_op(CELL_PAYLOAD_SIZE - pad_offset, OP_LE, CELL_PAYLOAD_SIZE);

  /* Still no room because we keep 4 extra bytes. */
  pad_offset = get_pad_cell_offset(payload_len - 4);
  tt_int_op(pad_offset, OP_EQ, 0);
  tt_int_op(CELL_PAYLOAD_SIZE - pad_offset, OP_LE, CELL_PAYLOAD_SIZE);

  /* We should have 1 byte of padding. Meaning, the offset should be the
   * CELL_PAYLOAD_SIZE minus 1 byte. */
  expected_offset = CELL_PAYLOAD_SIZE - 1;
  pad_offset = get_pad_cell_offset(payload_len - 5);
  tt_int_op(pad_offset, OP_EQ, expected_offset);
  tt_int_op(CELL_PAYLOAD_SIZE - pad_offset, OP_LE, CELL_PAYLOAD_SIZE);

  /* Now some arbitrary small payload length. The cell size is header + 10 +
   * extra 4 bytes we keep so the offset should be there. */
  expected_offset = RELAY_HEADER_SIZE + 10 + 4;
  pad_offset = get_pad_cell_offset(10);
  tt_int_op(pad_offset, OP_EQ, expected_offset);
  tt_int_op(CELL_PAYLOAD_SIZE - pad_offset, OP_LE, CELL_PAYLOAD_SIZE);

  /* Data length of 0. */
  expected_offset = RELAY_HEADER_SIZE + 4;
  pad_offset = get_pad_cell_offset(0);
  tt_int_op(pad_offset, OP_EQ, expected_offset);
  tt_int_op(CELL_PAYLOAD_SIZE - pad_offset, OP_LE, CELL_PAYLOAD_SIZE);

 done:
  ;
}

static void
test_cell_version_validation(void *arg)
{
  (void) arg;

  /* We currently only support up to SENDME_MAX_SUPPORTED_VERSION so we are
   * going to test the boundaries there. */

  tt_assert(cell_version_can_be_handled(SENDME_MAX_SUPPORTED_VERSION));

  /* Version below our supported should pass. */
  tt_assert(cell_version_can_be_handled(SENDME_MAX_SUPPORTED_VERSION - 1));

  /* Extra version from our supported should fail. */
  tt_assert(!cell_version_can_be_handled(SENDME_MAX_SUPPORTED_VERSION + 1));

  /* Simple check for version 0. */
  tt_assert(cell_version_can_be_handled(0));

  /* We MUST handle the default cell version that we emit or accept. */
  tt_assert(cell_version_can_be_handled(SENDME_EMIT_MIN_VERSION_DEFAULT));
  tt_assert(cell_version_can_be_handled(SENDME_ACCEPT_MIN_VERSION_DEFAULT));

 done:
  ;
}

/* check our decisions about how much stuff to put into relay cells. */
static void
test_package_payload_len(void *arg)
{
  (void)arg;
  /* this is not a real circuit: it only has the fields needed for this
   * test. */
  circuit_t *c = tor_malloc_zero(sizeof(circuit_t));

  /* check initial conditions. */
  circuit_reset_sendme_randomness(c);
  tt_assert(! c->have_sent_sufficiently_random_cell);
  tt_int_op(c->send_randomness_after_n_cells, OP_GE, CIRCWINDOW_INCREMENT / 2);
  tt_int_op(c->send_randomness_after_n_cells, OP_LT, CIRCWINDOW_INCREMENT);

  /* We have a bunch of cells before we need to send randomness, so the first
   * few can be packaged full. */
  int initial = c->send_randomness_after_n_cells;
  size_t n = connection_edge_get_inbuf_bytes_to_package(10000, 0, c);
  tt_uint_op(RELAY_PAYLOAD_SIZE, OP_EQ, n);
  n = connection_edge_get_inbuf_bytes_to_package(95000, 1, c);
  tt_uint_op(RELAY_PAYLOAD_SIZE, OP_EQ, n);
  tt_int_op(c->send_randomness_after_n_cells, OP_EQ, initial - 2);

  /* If package_partial isn't set, we won't package a partially full cell at
   * all. */
  n = connection_edge_get_inbuf_bytes_to_package(RELAY_PAYLOAD_SIZE-1, 0, c);
  tt_int_op(n, OP_EQ, 0);
  /* no change in our state, since nothing was sent. */
  tt_assert(! c->have_sent_sufficiently_random_cell);
  tt_int_op(c->send_randomness_after_n_cells, OP_EQ, initial - 2);

  /* If package_partial is set and the partial cell is not going to have
   * _enough_ randomness, we package it, but we don't consider ourselves to
   * have sent a sufficiently random cell. */
  n = connection_edge_get_inbuf_bytes_to_package(RELAY_PAYLOAD_SIZE-1, 1, c);
  tt_int_op(n, OP_EQ, RELAY_PAYLOAD_SIZE-1);
  tt_assert(! c->have_sent_sufficiently_random_cell);
  tt_int_op(c->send_randomness_after_n_cells, OP_EQ, initial - 3);

  /* Make sure we set have_set_sufficiently_random_cell as appropriate. */
  n = connection_edge_get_inbuf_bytes_to_package(RELAY_PAYLOAD_SIZE-64, 1, c);
  tt_int_op(n, OP_EQ, RELAY_PAYLOAD_SIZE-64);
  tt_assert(c->have_sent_sufficiently_random_cell);
  tt_int_op(c->send_randomness_after_n_cells, OP_EQ, initial - 4);

  /* Now let's look at what happens when we get down to zero. Since we have
   * sent a sufficiently random cell, we will not force this one to have a gap.
   */
  c->send_randomness_after_n_cells = 0;
  n = connection_edge_get_inbuf_bytes_to_package(10000, 1, c);
  tt_int_op(n, OP_EQ, RELAY_PAYLOAD_SIZE);
  /* Now these will be reset. */
  tt_assert(! c->have_sent_sufficiently_random_cell);
  tt_int_op(c->send_randomness_after_n_cells, OP_GE,
            CIRCWINDOW_INCREMENT / 2 - 1);

  /* What would happen if we hadn't sent a sufficiently random cell? */
  c->send_randomness_after_n_cells = 0;
  n = connection_edge_get_inbuf_bytes_to_package(10000, 1, c);
  const size_t reduced_payload_size = RELAY_PAYLOAD_SIZE - 4 - 16;
  tt_int_op(n, OP_EQ, reduced_payload_size);
  /* Now these will be reset. */
  tt_assert(! c->have_sent_sufficiently_random_cell);
  tt_int_op(c->send_randomness_after_n_cells, OP_GE,
            CIRCWINDOW_INCREMENT / 2 - 1);

  /* Here is a fun case: if it's time to package a small cell, then
   * package_partial==0 should mean we accept that many bytes.
   */
  c->send_randomness_after_n_cells = 0;
  n = connection_edge_get_inbuf_bytes_to_package(reduced_payload_size, 0, c);
  tt_int_op(n, OP_EQ, reduced_payload_size);

 done:
  tor_free(c);
}

struct testcase_t sendme_tests[] = {
  { "v1_record_digest", test_v1_record_digest, TT_FORK,
    NULL, NULL },
  { "v1_consensus_params", test_v1_consensus_params, TT_FORK,
    NULL, NULL },
  { "v1_build_cell", test_v1_build_cell, TT_FORK,
    NULL, NULL },
  { "cell_payload_pad", test_cell_payload_pad, TT_FORK,
    NULL, NULL },
  { "cell_version_validation", test_cell_version_validation, TT_FORK,
    NULL, NULL },
  { "package_payload_len", test_package_payload_len, 0, NULL, NULL },

  END_OF_TESTCASES
};
