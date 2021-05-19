/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/* Unit tests for handling different kinds of relay cell */

#define RELAY_PRIVATE
#define CIRCUITLIST_PRIVATE
#define CONNECTION_EDGE_PRIVATE
#define CONNECTION_PRIVATE

#include "core/or/or.h"
#include "core/mainloop/mainloop.h"
#include "app/config/config.h"
#include "core/mainloop/connection.h"
#include "lib/crypt_ops/crypto_cipher.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/connection_edge.h"
#include "core/or/sendme.h"
#include "core/or/relay.h"
#include "test/test.h"
#include "test/log_test_helpers.h"

#include "core/or/cell_st.h"
#include "core/or/crypt_path_st.h"
#include "core/or/entry_connection_st.h"
#include "core/or/origin_circuit_st.h"
#include "core/or/socks_request_st.h"
#include "core/or/half_edge_st.h"

#include "feature/client/circpathbias.h"

static int srm_ncalls;
static entry_connection_t *srm_conn;
static int srm_atype;
static size_t srm_alen;
static int srm_answer_is_set;
static uint8_t srm_answer[512];
static int srm_ttl;
static time_t srm_expires;

/* Mock replacement for connection_ap_hannshake_socks_resolved() */
static void
socks_resolved_mock(entry_connection_t *conn,
                    int answer_type,
                    size_t answer_len,
                    const uint8_t *answer,
                    int ttl,
                    time_t expires)
{
  srm_ncalls++;
  srm_conn = conn;
  srm_atype = answer_type;
  srm_alen = answer_len;
  if (answer) {
    memset(srm_answer, 0, sizeof(srm_answer));
    memcpy(srm_answer, answer, answer_len < 512 ? answer_len : 512);
    srm_answer_is_set = 1;
  } else {
    srm_answer_is_set = 0;
  }
  srm_ttl = ttl;
  srm_expires = expires;
}

static int mum_ncalls;
static entry_connection_t *mum_conn;
static int mum_endreason;

/* Mock replacement for connection_mark_unattached_ap_() */
static void
mark_unattached_mock(entry_connection_t *conn, int endreason,
                     int line, const char *file)
{
  ++mum_ncalls;
  mum_conn = conn;
  mum_endreason = endreason;
  (void) line;
  (void) file;
}

/* Helper: Return a newly allocated and initialized origin circuit with
 * purpose and flags. A default HS identifier is set to an ed25519
 * authentication key for introduction point. */
static origin_circuit_t *
helper_create_origin_circuit(int purpose, int flags)
{
  origin_circuit_t *circ = NULL;

  circ = origin_circuit_init(purpose, flags);
  tor_assert(circ);
  circ->cpath = tor_malloc_zero(sizeof(crypt_path_t));
  circ->cpath->magic = CRYPT_PATH_MAGIC;
  circ->cpath->state = CPATH_STATE_OPEN;
  circ->cpath->package_window = circuit_initial_package_window();
  circ->cpath->deliver_window = CIRCWINDOW_START;
  circ->cpath->prev = circ->cpath;
  /* Create a default HS identifier. */
  circ->hs_ident = tor_malloc_zero(sizeof(hs_ident_circuit_t));

  return circ;
}

static void
mock_connection_mark_unattached_ap_(entry_connection_t *conn, int endreason,
                                    int line, const char *file)
{
  (void) line;
  (void) file;
  conn->edge_.end_reason = endreason;
}

static void
mock_mark_circ_for_close(circuit_t *circ, int reason, int line,
                          const char *file)
{
  (void)reason; (void)line; (void)file;

  circ->marked_for_close = 1;
  return;
}

static void
mock_mark_for_close(connection_t *conn,
                        int line, const char *file)
{
  (void)line;
  (void)file;

  conn->marked_for_close = 1;
  return;
}

static void
mock_start_reading(connection_t *conn)
{
  (void)conn;
  return;
}

static int
mock_send_command(streamid_t stream_id, circuit_t *circ,
                               uint8_t relay_command, const char *payload,
                               size_t payload_len, crypt_path_t *cpath_layer,
                               const char *filename, int lineno)
{
 (void)stream_id; (void)circ;
 (void)relay_command; (void)payload;
 (void)payload_len; (void)cpath_layer;
 (void)filename; (void)lineno;

 return 0;
}

static entry_connection_t *
fake_entry_conn(origin_circuit_t *oncirc, streamid_t id)
{
  edge_connection_t *edgeconn;
  entry_connection_t *entryconn;

  entryconn = entry_connection_new(CONN_TYPE_AP, AF_INET);
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn);
  edgeconn->base_.state = AP_CONN_STATE_CONNECT_WAIT;
  edgeconn->deliver_window = STREAMWINDOW_START;
  edgeconn->package_window = STREAMWINDOW_START;

  edgeconn->stream_id = id;
  edgeconn->on_circuit = TO_CIRCUIT(oncirc);
  edgeconn->cpath_layer = oncirc->cpath;

  return entryconn;
}

#define PACK_CELL(id, cmd, body_s) do {                                  \
    memset(&cell, 0, sizeof(cell));                                     \
    memset(&rh, 0, sizeof(rh));                                         \
    memcpy(cell.payload+RELAY_HEADER_SIZE, (body_s), sizeof((body_s))-1); \
    rh.length = sizeof((body_s))-1;                                     \
    rh.command = (cmd);                                                 \
    rh.stream_id = (id);                                                \
    relay_header_pack((uint8_t*)&cell.payload, &rh);                    \
  } while (0)
#define ASSERT_COUNTED_BW() do { \
    tt_int_op(circ->n_delivered_read_circ_bw, OP_EQ, delivered+rh.length); \
    tt_int_op(circ->n_overhead_read_circ_bw, OP_EQ,                      \
              overhead+RELAY_PAYLOAD_SIZE-rh.length);               \
    delivered = circ->n_delivered_read_circ_bw;                          \
    overhead = circ->n_overhead_read_circ_bw;                            \
 } while (0)
#define ASSERT_UNCOUNTED_BW() do { \
    tt_int_op(circ->n_delivered_read_circ_bw, OP_EQ, delivered); \
    tt_int_op(circ->n_overhead_read_circ_bw, OP_EQ, overhead); \
 } while (0)

static int
subtest_circbw_halfclosed(origin_circuit_t *circ, streamid_t init_id)
{
  cell_t cell;
  relay_header_t rh;
  edge_connection_t *edgeconn;
  entry_connection_t *entryconn2=NULL;
  entry_connection_t *entryconn3=NULL;
  entry_connection_t *entryconn4=NULL;
  int delivered = circ->n_delivered_read_circ_bw;
  int overhead = circ->n_overhead_read_circ_bw;

  /* Make new entryconns */
  entryconn2 = fake_entry_conn(circ, init_id);
  entryconn2->socks_request->has_finished = 1;
  entryconn3 = fake_entry_conn(circ, init_id+1);
  entryconn3->socks_request->has_finished = 1;
  entryconn4 = fake_entry_conn(circ, init_id+2);
  entryconn4->socks_request->has_finished = 1;
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn2);
  edgeconn->package_window = 23;
  edgeconn->base_.state = AP_CONN_STATE_OPEN;

  int data_cells = edgeconn->deliver_window;
  int sendme_cells = (STREAMWINDOW_START-edgeconn->package_window)
                             /STREAMWINDOW_INCREMENT;
  ENTRY_TO_CONN(entryconn2)->marked_for_close = 0;
  connection_edge_reached_eof(edgeconn);

  /* Data cell not in the half-opened list */
  PACK_CELL(4000, RELAY_COMMAND_DATA, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                       circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Sendme cell not in the half-opened list */
  PACK_CELL(4000, RELAY_COMMAND_SENDME, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Connected cell not in the half-opened list */
  PACK_CELL(4000, RELAY_COMMAND_CONNECTED, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Resolved cell not in the half-opened list */
  PACK_CELL(4000, RELAY_COMMAND_RESOLVED, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Connected cell: not counted -- we were open */
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn2);
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_CONNECTED, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* DATA cells up to limit */
  while (data_cells > 0) {
    ENTRY_TO_CONN(entryconn2)->marked_for_close = 0;
    PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_DATA, "Data1234");
    if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
      pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
    else
      connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                       circ->cpath);
    ASSERT_COUNTED_BW();
    data_cells--;
  }
  ENTRY_TO_CONN(entryconn2)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_DATA, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* SENDME cells up to limit */
  while (sendme_cells > 0) {
    ENTRY_TO_CONN(entryconn2)->marked_for_close = 0;
    PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_SENDME, "Data1234");
    if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
      pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
    else
      connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                       circ->cpath);
    ASSERT_COUNTED_BW();
    sendme_cells--;
  }
  ENTRY_TO_CONN(entryconn2)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_SENDME, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Only one END cell */
  ENTRY_TO_CONN(entryconn2)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_END, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  ENTRY_TO_CONN(entryconn2)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_END, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  edgeconn = ENTRY_TO_EDGE_CONN(entryconn3);
  edgeconn->base_.state = AP_CONN_STATE_OPEN;
  ENTRY_TO_CONN(entryconn3)->marked_for_close = 0;
  /* sendme cell on open entryconn with full window */
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_SENDME, "Data1234");
  int ret =
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  tt_int_op(ret, OP_EQ, -END_CIRC_REASON_TORPROTOCOL);
  ASSERT_UNCOUNTED_BW();

  /* connected cell on a after EOF */
  ENTRY_TO_CONN(entryconn3)->marked_for_close = 0;
  edgeconn->base_.state = AP_CONN_STATE_CONNECT_WAIT;
  connection_edge_reached_eof(edgeconn);
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_CONNECTED, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ),  NULL,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  ENTRY_TO_CONN(entryconn3)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_CONNECTED, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ),  NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* DATA and SENDME after END cell */
  ENTRY_TO_CONN(entryconn3)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_END, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ),  NULL,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  ENTRY_TO_CONN(entryconn3)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_SENDME, "Data1234");
  ret =
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  tt_int_op(ret, OP_NE, -END_CIRC_REASON_TORPROTOCOL);
  ASSERT_UNCOUNTED_BW();

  ENTRY_TO_CONN(entryconn3)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_DATA, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Resolved: 1 counted, more not */
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn4);
  entryconn4->socks_request->command = SOCKS_COMMAND_RESOLVE;
  edgeconn->base_.state = AP_CONN_STATE_RESOLVE_WAIT;
  edgeconn->on_circuit = TO_CIRCUIT(circ);
  ENTRY_TO_CONN(entryconn4)->marked_for_close = 0;
  connection_edge_reached_eof(edgeconn);

  ENTRY_TO_CONN(entryconn4)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_RESOLVED,
            "\x04\x04\x12\x00\x00\x01\x00\x00\x02\x00");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  ENTRY_TO_CONN(entryconn4)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_RESOLVED,
            "\x04\x04\x12\x00\x00\x01\x00\x00\x02\x00");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Data not counted after resolved */
  ENTRY_TO_CONN(entryconn4)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_DATA, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* End not counted after resolved */
  ENTRY_TO_CONN(entryconn4)->marked_for_close = 0;
  PACK_CELL(edgeconn->stream_id, RELAY_COMMAND_END, "Data1234");
  if (circ->base_.purpose == CIRCUIT_PURPOSE_PATH_BIAS_TESTING)
    pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  else
    connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  connection_free_minimal(ENTRY_TO_CONN(entryconn2));
  connection_free_minimal(ENTRY_TO_CONN(entryconn3));
  connection_free_minimal(ENTRY_TO_CONN(entryconn4));
  return 1;
 done:
  connection_free_minimal(ENTRY_TO_CONN(entryconn2));
  connection_free_minimal(ENTRY_TO_CONN(entryconn3));
  connection_free_minimal(ENTRY_TO_CONN(entryconn4));
  return 0;
}

static int
halfstream_insert(origin_circuit_t *circ, edge_connection_t *edgeconn,
                  streamid_t *streams, int num, int random)
{
  int inserted = 0;

  /* Insert num random elements */
  while (inserted < num) {
    streamid_t id;

    if (random)
      id = (streamid_t)crypto_rand_int(65535)+1;
    else
      id = get_unique_stream_id_by_circ(circ);

    edgeconn->stream_id = id;

    /* Ensure it isn't there */
    if (connection_half_edge_find_stream_id(circ->half_streams, id)) {
      continue;
    }

    connection_half_edge_add(edgeconn, circ);
    if (streams)
      streams[inserted] = id;
    inserted++;
  }

  return inserted;
}

static void
subtest_halfstream_insertremove(int num)
{
  origin_circuit_t *circ =
      helper_create_origin_circuit(CIRCUIT_PURPOSE_C_GENERAL, 0);
  edge_connection_t *edgeconn;
  entry_connection_t *entryconn;
  streamid_t *streams = tor_malloc_zero(num*sizeof(streamid_t));
  int i = 0;

  circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
  circ->cpath->deliver_window = CIRCWINDOW_START;

  entryconn = fake_entry_conn(circ, 23);
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn);

  /* Explicitly test all operations on an absent stream list */
  tt_int_op(connection_half_edge_is_valid_data(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_connected(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_sendme(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_resolved(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
            23), OP_EQ, 0);

  /* Insert a duplicate element; verify that other elements absent;
   * ensure removing it once works */
  edgeconn->stream_id = 23;
  connection_half_edge_add(edgeconn, circ);
  connection_half_edge_add(edgeconn, circ);
  connection_half_edge_add(edgeconn, circ);

  /* Verify that other elements absent */
  tt_int_op(connection_half_edge_is_valid_data(circ->half_streams,
            22), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_connected(circ->half_streams,
            22), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_sendme(circ->half_streams,
            22), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_resolved(circ->half_streams,
            22), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
            22), OP_EQ, 0);

  tt_int_op(connection_half_edge_is_valid_data(circ->half_streams,
            24), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_connected(circ->half_streams,
            24), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_sendme(circ->half_streams,
            24), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_resolved(circ->half_streams,
            24), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
            24), OP_EQ, 0);

  /* Verify we only remove it once */
  tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
            23), OP_EQ, 1);
  tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
            23), OP_EQ, 0);

  halfstream_insert(circ, edgeconn, streams, num, 1);

  /* Remove half of them */
  for (i = 0; i < num/2; i++) {
    tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
                                                streams[i]),
              OP_EQ, 1);
  }

  /* Verify first half of list is gone */
  for (i = 0; i < num/2; i++) {
    tt_ptr_op(connection_half_edge_find_stream_id(circ->half_streams,
                                                  streams[i]),
              OP_EQ, NULL);
  }

  /* Verify second half of list is present */
  for (; i < num; i++) {
    tt_ptr_op(connection_half_edge_find_stream_id(circ->half_streams,
                                                  streams[i]),
              OP_NE, NULL);
  }

  /* Remove other half. Verify list is empty. */
  for (i = num/2; i < num; i++) {
    tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
                                                streams[i]),
              OP_EQ, 1);
  }
  tt_int_op(smartlist_len(circ->half_streams), OP_EQ, 0);

  /* Explicitly test all operations on an empty stream list */
  tt_int_op(connection_half_edge_is_valid_data(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_connected(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_sendme(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_resolved(circ->half_streams,
            23), OP_EQ, 0);
  tt_int_op(connection_half_edge_is_valid_end(circ->half_streams,
            23), OP_EQ, 0);

  /* For valgrind, leave some around then free the circ */
  halfstream_insert(circ, edgeconn, NULL, 10, 0);

 done:
  tor_free(streams);
  circuit_free_(TO_CIRCUIT(circ));
  connection_free_minimal(ENTRY_TO_CONN(entryconn));
}

static void
test_halfstream_insertremove(void *arg)
{
  (void)arg;

  /* Suppress the WARN message we generate in this test */
  setup_full_capture_of_logs(LOG_WARN);

  /* Test insertion and removal with a few different sizes */
  subtest_halfstream_insertremove(10);
  subtest_halfstream_insertremove(100);
  subtest_halfstream_insertremove(1000);
}

static void
test_halfstream_wrap(void *arg)
{
  origin_circuit_t *circ =
      helper_create_origin_circuit(CIRCUIT_PURPOSE_C_GENERAL, 0);
  edge_connection_t *edgeconn;
  entry_connection_t *entryconn;

  circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
  circ->cpath->deliver_window = CIRCWINDOW_START;

  entryconn = fake_entry_conn(circ, 23);
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn);

  (void)arg;

  /* Suppress the WARN message we generate in this test */
  setup_full_capture_of_logs(LOG_WARN);
  MOCK(connection_mark_for_close_internal_, mock_mark_for_close);

  /* Verify that get_unique_stream_id_by_circ() can wrap uint16_t */
  circ->next_stream_id = 65530;
  halfstream_insert(circ, edgeconn, NULL, 7, 0);
  tt_int_op(circ->next_stream_id, OP_EQ, 2);
  tt_int_op(smartlist_len(circ->half_streams), OP_EQ, 7);

  /* Insert full-1 */
  halfstream_insert(circ, edgeconn, NULL,
                    65534-smartlist_len(circ->half_streams), 0);
  tt_int_op(smartlist_len(circ->half_streams), OP_EQ, 65534);

  /* Verify that we can get_unique_stream_id_by_circ() successfully */
  edgeconn->stream_id = get_unique_stream_id_by_circ(circ);
  tt_int_op(edgeconn->stream_id, OP_NE, 0); /* 0 is failure */

  /* Insert an opened stream on the circ with that id */
  ENTRY_TO_CONN(entryconn)->marked_for_close = 0;
  edgeconn->base_.state = AP_CONN_STATE_CONNECT_WAIT;
  circ->p_streams = edgeconn;

  /* Verify that get_unique_stream_id_by_circ() fails */
  tt_int_op(get_unique_stream_id_by_circ(circ), OP_EQ, 0); /* 0 is failure */

  /* eof the one opened stream. Verify it is now in half-closed */
  tt_int_op(smartlist_len(circ->half_streams), OP_EQ, 65534);
  connection_edge_reached_eof(edgeconn);
  tt_int_op(smartlist_len(circ->half_streams), OP_EQ, 65535);

  /* Verify get_unique_stream_id_by_circ() fails due to full half-closed */
  circ->p_streams = NULL;
  tt_int_op(get_unique_stream_id_by_circ(circ), OP_EQ, 0); /* 0 is failure */

 done:
  circuit_free_(TO_CIRCUIT(circ));
  connection_free_minimal(ENTRY_TO_CONN(entryconn));
  UNMOCK(connection_mark_for_close_internal_);
}

static void
test_circbw_relay(void *arg)
{
  cell_t cell;
  relay_header_t rh;
  tor_addr_t addr;
  edge_connection_t *edgeconn;
  entry_connection_t *entryconn1=NULL;
  origin_circuit_t *circ;
  int delivered = 0;
  int overhead = 0;

  (void)arg;

  MOCK(connection_mark_unattached_ap_, mock_connection_mark_unattached_ap_);
  MOCK(connection_start_reading, mock_start_reading);
  MOCK(connection_mark_for_close_internal_, mock_mark_for_close);
  MOCK(relay_send_command_from_edge_, mock_send_command);
  MOCK(circuit_mark_for_close_, mock_mark_circ_for_close);

  circ = helper_create_origin_circuit(CIRCUIT_PURPOSE_C_GENERAL, 0);
  circ->cpath->state = CPATH_STATE_AWAITING_KEYS;
  circ->cpath->deliver_window = CIRCWINDOW_START;

  entryconn1 = fake_entry_conn(circ, 1);
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn1);

  /* Stream id 0: Not counted */
  PACK_CELL(0, RELAY_COMMAND_END, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Stream id 1: Counted */
  PACK_CELL(1, RELAY_COMMAND_END, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  /* Properly formatted connect cell: counted */
  PACK_CELL(1, RELAY_COMMAND_CONNECTED, "Data1234");
  tor_addr_parse(&addr, "30.40.50.60");
  rh.length = connected_cell_format_payload(cell.payload+RELAY_HEADER_SIZE,
                                            &addr, 1024);
  relay_header_pack((uint8_t*)&cell.payload, &rh);                    \
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  /* Properly formatted resolved cell in correct state: counted */
  edgeconn->base_.state = AP_CONN_STATE_RESOLVE_WAIT;
  entryconn1->socks_request->command = SOCKS_COMMAND_RESOLVE;
  edgeconn->on_circuit = TO_CIRCUIT(circ);
  PACK_CELL(1, RELAY_COMMAND_RESOLVED,
            "\x04\x04\x12\x00\x00\x01\x00\x00\x02\x00");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  edgeconn->base_.state = AP_CONN_STATE_OPEN;
  entryconn1->socks_request->has_finished = 1;

  /* Connected cell after open: not counted */
  PACK_CELL(1, RELAY_COMMAND_CONNECTED, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Resolved cell after open: not counted */
  PACK_CELL(1, RELAY_COMMAND_RESOLVED, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Drop cell: not counted */
  PACK_CELL(1, RELAY_COMMAND_DROP, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Data cell on stream 0: not counted */
  PACK_CELL(0, RELAY_COMMAND_DATA, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Data cell on open connection: counted */
  ENTRY_TO_CONN(entryconn1)->marked_for_close = 0;
  PACK_CELL(1, RELAY_COMMAND_DATA, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  /* Empty Data cell on open connection: not counted */
  ENTRY_TO_CONN(entryconn1)->marked_for_close = 0;
  PACK_CELL(1, RELAY_COMMAND_DATA, "");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Sendme on valid stream: counted */
  edgeconn->package_window -= STREAMWINDOW_INCREMENT;
  PACK_CELL(1, RELAY_COMMAND_SENDME, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  /* Sendme on valid stream with full window: not counted */
  PACK_CELL(1, RELAY_COMMAND_SENDME, "Data1234");
  edgeconn->package_window = STREAMWINDOW_START;
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Sendme on unknown stream: not counted */
  PACK_CELL(1, RELAY_COMMAND_SENDME, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Sendme on circuit with full window: not counted */
  PACK_CELL(0, RELAY_COMMAND_SENDME, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Sendme on circuit with non-full window: counted */
  PACK_CELL(0, RELAY_COMMAND_SENDME, "");
  /* Recording a cell, the window is updated after decryption so off by one in
   * order to record and then we process it with the proper window. */
  circ->cpath->package_window = 901;
  sendme_record_cell_digest_on_circ(TO_CIRCUIT(circ), circ->cpath);
  circ->cpath->package_window = 900;
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  /* Invalid extended cell: not counted */
  PACK_CELL(1, RELAY_COMMAND_EXTENDED2, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Invalid extended cell: not counted */
  PACK_CELL(1, RELAY_COMMAND_EXTENDED, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Invalid HS cell: not counted */
  PACK_CELL(1, RELAY_COMMAND_ESTABLISH_INTRO, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* "Valid" HS cell in expected state: counted */
  TO_CIRCUIT(circ)->purpose = CIRCUIT_PURPOSE_C_ESTABLISH_REND;
  PACK_CELL(1, RELAY_COMMAND_RENDEZVOUS_ESTABLISHED, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  /* End cell on non-closed connection: counted */
  PACK_CELL(1, RELAY_COMMAND_END, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), edgeconn,
                                     circ->cpath);
  ASSERT_COUNTED_BW();

  /* End cell on connection that already got one: not counted */
  PACK_CELL(1, RELAY_COMMAND_END, "Data1234");
  connection_edge_process_relay_cell(&cell, TO_CIRCUIT(circ), NULL,
                                     circ->cpath);
  ASSERT_UNCOUNTED_BW();

  /* Simulate closed stream on entryconn, then test: */
  if (!subtest_circbw_halfclosed(circ, 2))
    goto done;

  circ->base_.purpose = CIRCUIT_PURPOSE_PATH_BIAS_TESTING;
  if (!subtest_circbw_halfclosed(circ, 6))
    goto done;

  /* Path bias: truncated */
  tt_int_op(circ->base_.marked_for_close, OP_EQ, 0);
  PACK_CELL(0, RELAY_COMMAND_TRUNCATED, "Data1234");
  pathbias_count_valid_cells(TO_CIRCUIT(circ), &cell);
  tt_int_op(circ->base_.marked_for_close, OP_EQ, 1);

 done:
  UNMOCK(connection_start_reading);
  UNMOCK(connection_mark_unattached_ap_);
  UNMOCK(connection_mark_for_close_internal_);
  UNMOCK(relay_send_command_from_edge_);
  UNMOCK(circuit_mark_for_close_);
  circuit_free_(TO_CIRCUIT(circ));
  connection_free_minimal(ENTRY_TO_CONN(entryconn1));
}

/* Tests for connection_edge_process_resolved_cell().

   The point of ..process_resolved_cell() is to handle an incoming cell
   on an entry connection, and call connection_mark_unattached_ap() and/or
   connection_ap_handshake_socks_resolved().
 */
static void
test_relaycell_resolved(void *arg)
{
  entry_connection_t *entryconn;
  edge_connection_t *edgeconn;
  cell_t cell;
  relay_header_t rh;
  int r;
  or_options_t *options = get_options_mutable();

#define SET_CELL(s) do {                                                \
    memset(&cell, 0, sizeof(cell));                                     \
    memset(&rh, 0, sizeof(rh));                                         \
    memcpy(cell.payload + RELAY_HEADER_SIZE, (s), sizeof((s))-1);       \
    rh.length = sizeof((s))-1;                                          \
    rh.command = RELAY_COMMAND_RESOLVED;                                \
  } while (0)
#define MOCK_RESET() do {                       \
    srm_ncalls = mum_ncalls = 0;                \
  } while (0)
#define ASSERT_MARK_CALLED(reason) do {         \
    tt_int_op(mum_ncalls, OP_EQ, 1);               \
    tt_ptr_op(mum_conn, OP_EQ, entryconn);         \
    tt_int_op(mum_endreason, OP_EQ, (reason));     \
  } while (0)
#define ASSERT_RESOLVED_CALLED(atype, answer, ttl, expires) do {  \
    tt_int_op(srm_ncalls, OP_EQ, 1);                                 \
    tt_ptr_op(srm_conn, OP_EQ, entryconn);                           \
    tt_int_op(srm_atype, OP_EQ, (atype));                            \
    if ((answer) != NULL) {                                          \
      tt_int_op(srm_alen, OP_EQ, sizeof(answer)-1);                  \
      tt_int_op(srm_alen, OP_LT, 512);                                \
      tt_int_op(srm_answer_is_set, OP_EQ, 1);                        \
      tt_mem_op(srm_answer, OP_EQ, answer, sizeof(answer)-1);        \
    } else {                                                      \
      tt_int_op(srm_answer_is_set, OP_EQ, 0);                        \
    }                                                             \
    tt_int_op(srm_ttl, OP_EQ, ttl);                                  \
    tt_i64_op(srm_expires, OP_EQ, expires);                          \
  } while (0)

  (void)arg;

  MOCK(connection_mark_unattached_ap_, mark_unattached_mock);
  MOCK(connection_ap_handshake_socks_resolved, socks_resolved_mock);

  options->ClientDNSRejectInternalAddresses = 0;

  SET_CELL(/* IPv4: 127.0.1.2, ttl 256 */
           "\x04\x04\x7f\x00\x01\x02\x00\x00\x01\x00"
           /* IPv4: 18.0.0.1, ttl 512 */
           "\x04\x04\x12\x00\x00\x01\x00\x00\x02\x00"
           /* IPv6: 2003::3, ttl 1024 */
           "\x06\x10"
           "\x20\x02\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x03"
           "\x00\x00\x04\x00");

  entryconn = entry_connection_new(CONN_TYPE_AP, AF_INET);
  edgeconn = ENTRY_TO_EDGE_CONN(entryconn);

  /* Try with connection in non-RESOLVE_WAIT state: cell gets ignored */
  MOCK_RESET();
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(srm_ncalls, OP_EQ, 0);
  tt_int_op(mum_ncalls, OP_EQ, 0);

  /* Now put it in the right state. */
  ENTRY_TO_CONN(entryconn)->state = AP_CONN_STATE_RESOLVE_WAIT;
  entryconn->socks_request->command = SOCKS_COMMAND_RESOLVE;
  entryconn->entry_cfg.ipv4_traffic = 1;
  entryconn->entry_cfg.ipv6_traffic = 1;
  entryconn->entry_cfg.prefer_ipv6 = 0;

  /* We prefer ipv4, so we should get the first ipv4 answer */
  MOCK_RESET();
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_IPV4, "\x7f\x00\x01\x02", 256, -1);

  /* But we may be discarding private answers. */
  MOCK_RESET();
  options->ClientDNSRejectInternalAddresses = 1;
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_IPV4, "\x12\x00\x00\x01", 512, -1);

  /* now prefer ipv6, and get the first ipv6 answer */
  entryconn->entry_cfg.prefer_ipv6 = 1;
  MOCK_RESET();
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_IPV6,
                         "\x20\x02\x00\x00\x00\x00\x00\x00"
                         "\x00\x00\x00\x00\x00\x00\x00\x03",
                         1024, -1);

  /* With a cell that only has IPv4, we report IPv4 even if we prefer IPv6 */
  MOCK_RESET();
  SET_CELL("\x04\x04\x12\x00\x00\x01\x00\x00\x02\x00");
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_IPV4, "\x12\x00\x00\x01", 512, -1);

  /* But if we don't allow IPv4, we report nothing if the cell contains only
   * ipv4 */
  MOCK_RESET();
  entryconn->entry_cfg.ipv4_traffic = 0;
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_ERROR, NULL, -1, -1);

  /* If we wanted hostnames, we report nothing, since we only had IPs. */
  MOCK_RESET();
  entryconn->entry_cfg.ipv4_traffic = 1;
  entryconn->socks_request->command = SOCKS_COMMAND_RESOLVE_PTR;
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_ERROR, NULL, -1, -1);

  /* A hostname cell is fine though. */
  MOCK_RESET();
  SET_CELL("\x00\x0fwww.example.com\x00\x01\x00\x00");
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_HOSTNAME, "www.example.com", 65536, -1);

  /* error on malformed cell */
  MOCK_RESET();
  entryconn->socks_request->command = SOCKS_COMMAND_RESOLVE;
  SET_CELL("\x04\x04\x01\x02\x03\x04"); /* no ttl */
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_TORPROTOCOL);
  tt_int_op(srm_ncalls, OP_EQ, 0);

  /* error on all addresses private */
  MOCK_RESET();
  SET_CELL(/* IPv4: 127.0.1.2, ttl 256 */
           "\x04\x04\x7f\x00\x01\x02\x00\x00\x01\x00"
           /* IPv4: 192.168.1.1, ttl 256 */
           "\x04\x04\xc0\xa8\x01\x01\x00\x00\x01\x00");
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_TORPROTOCOL);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_ERROR_TRANSIENT, NULL, 0, TIME_MAX);

  /* Legit error code */
  MOCK_RESET();
  SET_CELL("\xf0\x15" "quiet and meaningless" "\x00\x00\x0f\xff");
  r = connection_edge_process_resolved_cell(edgeconn, &cell, &rh);
  tt_int_op(r, OP_EQ, 0);
  ASSERT_MARK_CALLED(END_STREAM_REASON_DONE|
                     END_STREAM_REASON_FLAG_ALREADY_SOCKS_REPLIED);
  ASSERT_RESOLVED_CALLED(RESOLVED_TYPE_ERROR_TRANSIENT, NULL, -1, -1);

 done:
  UNMOCK(connection_mark_unattached_ap_);
  UNMOCK(connection_ap_handshake_socks_resolved);
}

struct testcase_t relaycell_tests[] = {
  { "resolved", test_relaycell_resolved, TT_FORK, NULL, NULL },
  { "circbw", test_circbw_relay, TT_FORK, NULL, NULL },
  { "halfstream", test_halfstream_insertremove, TT_FORK, NULL, NULL },
  { "streamwrap", test_halfstream_wrap, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
