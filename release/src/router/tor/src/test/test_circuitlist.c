/* Copyright (c) 2013-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CHANNEL_OBJECT_PRIVATE
#define CIRCUITBUILD_PRIVATE
#define CIRCUITLIST_PRIVATE
#define HS_CIRCUITMAP_PRIVATE
#include "core/or/or.h"
#include "core/or/channel.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitmux_ewma.h"
#include "feature/hs/hs_circuitmap.h"
#include "test/test.h"
#include "test/log_test_helpers.h"

#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

#include "lib/container/bitarray.h"

static channel_t *
new_fake_channel(void)
{
  channel_t *chan = tor_malloc_zero(sizeof(channel_t));
  channel_init(chan);
  return chan;
}

static struct {
  int ncalls;
  void *cmux;
  void *circ;
  cell_direction_t dir;
} cam;

static void
circuitmux_attach_mock(circuitmux_t *cmux, circuit_t *circ,
                         cell_direction_t dir)
{
  ++cam.ncalls;
  cam.cmux = cmux;
  cam.circ = circ;
  cam.dir = dir;
}

static struct {
  int ncalls;
  void *cmux;
  void *circ;
} cdm;

static void
circuitmux_detach_mock(circuitmux_t *cmux, circuit_t *circ)
{
  ++cdm.ncalls;
  cdm.cmux = cmux;
  cdm.circ = circ;
}

#define GOT_CMUX_ATTACH(mux_, circ_, dir_) do {  \
    tt_int_op(cam.ncalls, OP_EQ, 1);                \
    tt_ptr_op(cam.cmux, OP_EQ, (mux_));             \
    tt_ptr_op(cam.circ, OP_EQ, (circ_));            \
    tt_int_op(cam.dir, OP_EQ, (dir_));              \
    memset(&cam, 0, sizeof(cam));                \
  } while (0)

#define GOT_CMUX_DETACH(mux_, circ_) do {        \
    tt_int_op(cdm.ncalls, OP_EQ, 1);                \
    tt_ptr_op(cdm.cmux, OP_EQ, (mux_));             \
    tt_ptr_op(cdm.circ, OP_EQ, (circ_));            \
    memset(&cdm, 0, sizeof(cdm));                \
  } while (0)

static void
test_clist_maps(void *arg)
{
  channel_t *ch1 = new_fake_channel();
  channel_t *ch2 = new_fake_channel();
  channel_t *ch3 = new_fake_channel();
  or_circuit_t *or_c1=NULL, *or_c2=NULL;

  (void) arg;

  MOCK(circuitmux_attach_circuit, circuitmux_attach_mock);
  MOCK(circuitmux_detach_circuit, circuitmux_detach_mock);
  memset(&cam, 0, sizeof(cam));
  memset(&cdm, 0, sizeof(cdm));

  tt_assert(ch1);
  tt_assert(ch2);
  tt_assert(ch3);

  ch1->cmux = tor_malloc(1);
  ch2->cmux = tor_malloc(1);
  ch3->cmux = tor_malloc(1);

  or_c1 = or_circuit_new(100, ch2);
  tt_assert(or_c1);
  GOT_CMUX_ATTACH(ch2->cmux, or_c1, CELL_DIRECTION_IN);
  tt_int_op(or_c1->p_circ_id, OP_EQ, 100);
  tt_ptr_op(or_c1->p_chan, OP_EQ, ch2);

  or_c2 = or_circuit_new(100, ch1);
  tt_assert(or_c2);
  GOT_CMUX_ATTACH(ch1->cmux, or_c2, CELL_DIRECTION_IN);
  tt_int_op(or_c2->p_circ_id, OP_EQ, 100);
  tt_ptr_op(or_c2->p_chan, OP_EQ, ch1);

  circuit_set_n_circid_chan(TO_CIRCUIT(or_c1), 200, ch1);
  GOT_CMUX_ATTACH(ch1->cmux, or_c1, CELL_DIRECTION_OUT);

  circuit_set_n_circid_chan(TO_CIRCUIT(or_c2), 200, ch2);
  GOT_CMUX_ATTACH(ch2->cmux, or_c2, CELL_DIRECTION_OUT);

  tt_ptr_op(circuit_get_by_circid_channel(200, ch1), OP_EQ, TO_CIRCUIT(or_c1));
  tt_ptr_op(circuit_get_by_circid_channel(200, ch2), OP_EQ, TO_CIRCUIT(or_c2));
  tt_ptr_op(circuit_get_by_circid_channel(100, ch2), OP_EQ, TO_CIRCUIT(or_c1));
  /* Try the same thing again, to test the "fast" path. */
  tt_ptr_op(circuit_get_by_circid_channel(100, ch2), OP_EQ, TO_CIRCUIT(or_c1));
  tt_assert(circuit_id_in_use_on_channel(100, ch2));
  tt_assert(! circuit_id_in_use_on_channel(101, ch2));

  /* Try changing the circuitid and channel of that circuit. */
  circuit_set_p_circid_chan(or_c1, 500, ch3);
  GOT_CMUX_DETACH(ch2->cmux, TO_CIRCUIT(or_c1));
  GOT_CMUX_ATTACH(ch3->cmux, TO_CIRCUIT(or_c1), CELL_DIRECTION_IN);
  tt_ptr_op(circuit_get_by_circid_channel(100, ch2), OP_EQ, NULL);
  tt_assert(! circuit_id_in_use_on_channel(100, ch2));
  tt_ptr_op(circuit_get_by_circid_channel(500, ch3), OP_EQ, TO_CIRCUIT(or_c1));

  /* Now let's see about destroy handling. */
  tt_assert(! circuit_id_in_use_on_channel(205, ch2));
  tt_assert(circuit_id_in_use_on_channel(200, ch2));
  channel_note_destroy_pending(ch2, 200);
  channel_note_destroy_pending(ch2, 205);
  channel_note_destroy_pending(ch1, 100);
  tt_assert(circuit_id_in_use_on_channel(205, ch2));
  tt_assert(circuit_id_in_use_on_channel(200, ch2));
  tt_assert(circuit_id_in_use_on_channel(100, ch1));

  tt_assert(TO_CIRCUIT(or_c2)->n_delete_pending != 0);
  tt_ptr_op(circuit_get_by_circid_channel(200, ch2), OP_EQ, TO_CIRCUIT(or_c2));
  tt_ptr_op(circuit_get_by_circid_channel(100, ch1), OP_EQ, TO_CIRCUIT(or_c2));

  /* Okay, now free ch2 and make sure that the circuit ID is STILL not
   * usable, because we haven't declared the destroy to be nonpending */
  tt_int_op(cdm.ncalls, OP_EQ, 0);
  circuit_free_(TO_CIRCUIT(or_c2));
  or_c2 = NULL; /* prevent free */
  tt_int_op(cdm.ncalls, OP_EQ, 2);
  memset(&cdm, 0, sizeof(cdm));
  tt_assert(circuit_id_in_use_on_channel(200, ch2));
  tt_assert(circuit_id_in_use_on_channel(100, ch1));
  tt_ptr_op(circuit_get_by_circid_channel(200, ch2), OP_EQ, NULL);
  tt_ptr_op(circuit_get_by_circid_channel(100, ch1), OP_EQ, NULL);

  /* Now say that the destroy is nonpending */
  channel_note_destroy_not_pending(ch2, 200);
  tt_ptr_op(circuit_get_by_circid_channel(200, ch2), OP_EQ, NULL);
  channel_note_destroy_not_pending(ch1, 100);
  tt_ptr_op(circuit_get_by_circid_channel(100, ch1), OP_EQ, NULL);
  tt_assert(! circuit_id_in_use_on_channel(200, ch2));
  tt_assert(! circuit_id_in_use_on_channel(100, ch1));

 done:
  if (or_c1)
    circuit_free_(TO_CIRCUIT(or_c1));
  if (or_c2)
    circuit_free_(TO_CIRCUIT(or_c2));
  if (ch1)
    tor_free(ch1->cmux);
  if (ch2)
    tor_free(ch2->cmux);
  if (ch3)
    tor_free(ch3->cmux);
  tor_free(ch1);
  tor_free(ch2);
  tor_free(ch3);
  UNMOCK(circuitmux_attach_circuit);
  UNMOCK(circuitmux_detach_circuit);
}

static void
test_rend_token_maps(void *arg)
{
  or_circuit_t *c1, *c2, *c3, *c4;
  origin_circuit_t *c5;
  const uint8_t tok1[REND_TOKEN_LEN] = "The cat can't tell y";
  const uint8_t tok2[REND_TOKEN_LEN] = "ou its name, and it ";
  const uint8_t tok3[REND_TOKEN_LEN] = "doesn't really care.";
  /* -- Adapted from a quote by Fredrik Lundh. */

  (void)arg;
  (void)tok1; //xxxx

  hs_circuitmap_init();

  c1 = or_circuit_new(0, NULL);
  c2 = or_circuit_new(0, NULL);
  c3 = or_circuit_new(0, NULL);
  c4 = or_circuit_new(0, NULL);
  c5 = origin_circuit_new();

  /* Make sure we really filled up the tok* variables */
  tt_int_op(tok1[REND_TOKEN_LEN-1], OP_EQ, 'y');
  tt_int_op(tok2[REND_TOKEN_LEN-1], OP_EQ, ' ');
  tt_int_op(tok3[REND_TOKEN_LEN-1], OP_EQ, '.');

  /* No maps; nothing there. */
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok1));
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok1));

  hs_circuitmap_register_rend_circ_relay_side(c1, tok1);
  hs_circuitmap_register_intro_circ_v2_relay_side(c2, tok2);

  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok3));
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok3));
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok2));
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok1));

  /* Without purpose set, we don't get the circuits */
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok1));
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok2));

  c1->base_.purpose = CIRCUIT_PURPOSE_REND_POINT_WAITING;
  c2->base_.purpose = CIRCUIT_PURPOSE_INTRO_POINT;

  /* Okay, make sure they show up now. */
  tt_ptr_op(c1, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok1));
  tt_ptr_op(c2, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok2));

  /* Two items at the same place with the same token. */
  c3->base_.purpose = CIRCUIT_PURPOSE_REND_POINT_WAITING;
  hs_circuitmap_register_rend_circ_relay_side(c3, tok2);
  tt_ptr_op(c2, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok2));
  tt_ptr_op(c3, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok2));

  /* Marking a circuit makes it not get returned any more */
  circuit_mark_for_close(TO_CIRCUIT(c1), END_CIRC_REASON_FINISHED);
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok1));
  circuit_free_(TO_CIRCUIT(c1));
  c1 = NULL;

  /* Freeing a circuit makes it not get returned any more. */
  circuit_free_(TO_CIRCUIT(c2));
  c2 = NULL;
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok2));

  /* c3 -- are you still there? */
  tt_ptr_op(c3, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok2));
  /* Change its cookie.  This never happens in Tor per se, but hey. */
  c3->base_.purpose = CIRCUIT_PURPOSE_INTRO_POINT;
  hs_circuitmap_register_intro_circ_v2_relay_side(c3, tok3);

  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok2));
  tt_ptr_op(c3, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok3));

  /* Now replace c3 with c4. */
  c4->base_.purpose = CIRCUIT_PURPOSE_INTRO_POINT;
  hs_circuitmap_register_intro_circ_v2_relay_side(c4, tok3);

  tt_ptr_op(c4, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok3));

  tt_ptr_op(TO_CIRCUIT(c3)->hs_token, OP_EQ, NULL);
  tt_ptr_op(TO_CIRCUIT(c4)->hs_token, OP_NE, NULL);
  tt_mem_op(TO_CIRCUIT(c4)->hs_token->token, OP_EQ, tok3, REND_TOKEN_LEN);

  /* Now clear c4's cookie. */
  hs_circuitmap_remove_circuit(TO_CIRCUIT(c4));
  tt_ptr_op(TO_CIRCUIT(c4)->hs_token, OP_EQ, NULL);
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok3));

  /* Now let's do a check for the client-side rend circuitmap */
  c5->base_.purpose = CIRCUIT_PURPOSE_C_ESTABLISH_REND;
  hs_circuitmap_register_rend_circ_client_side(c5, tok1);

  tt_ptr_op(c5, OP_EQ, hs_circuitmap_get_rend_circ_client_side(tok1));
  tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_client_side(tok2));

 done:
  if (c1)
    circuit_free_(TO_CIRCUIT(c1));
  if (c2)
    circuit_free_(TO_CIRCUIT(c2));
  if (c3)
    circuit_free_(TO_CIRCUIT(c3));
  if (c4)
    circuit_free_(TO_CIRCUIT(c4));
  if (c5)
    circuit_free_(TO_CIRCUIT(c5));
}

static void
mock_channel_dump_statistics(channel_t *chan, int severity)
{
  (void)chan;
  (void)severity;
}

static void
test_pick_circid(void *arg)
{
  bitarray_t *ba = NULL;
  channel_t *chan1, *chan2;
  circid_t circid;
  int i;
  (void) arg;

  MOCK(channel_dump_statistics, mock_channel_dump_statistics);

  chan1 = tor_malloc_zero(sizeof(channel_t));
  chan2 = tor_malloc_zero(sizeof(channel_t));
  chan2->wide_circ_ids = 1;

  chan1->cmux = circuitmux_alloc();
  chan2->cmux = circuitmux_alloc();

  /* CIRC_ID_TYPE_NEITHER is supposed to create a warning. */
  chan1->circ_id_type = CIRC_ID_TYPE_NEITHER;
  setup_full_capture_of_logs(LOG_WARN);
  tt_int_op(0, OP_EQ, get_unique_circ_id_by_chan(chan1));
  expect_single_log_msg_containing("Trying to pick a circuit ID for a "
                           "connection from a client with no identity.");
  teardown_capture_of_logs();

  /* Basic tests, with no collisions */
  chan1->circ_id_type = CIRC_ID_TYPE_LOWER;
  for (i = 0; i < 50; ++i) {
    circid = get_unique_circ_id_by_chan(chan1);
    tt_uint_op(0, OP_LT, circid);
    tt_uint_op(circid, OP_LT, (1<<15));
  }
  chan1->circ_id_type = CIRC_ID_TYPE_HIGHER;
  for (i = 0; i < 50; ++i) {
    circid = get_unique_circ_id_by_chan(chan1);
    tt_uint_op((1<<15), OP_LT, circid);
    tt_uint_op(circid, OP_LT, (1<<16));
  }

  chan2->circ_id_type = CIRC_ID_TYPE_LOWER;
  for (i = 0; i < 50; ++i) {
    circid = get_unique_circ_id_by_chan(chan2);
    tt_uint_op(0, OP_LT, circid);
    tt_uint_op(circid, OP_LT, (1u<<31));
  }
  chan2->circ_id_type = CIRC_ID_TYPE_HIGHER;
  for (i = 0; i < 50; ++i) {
    circid = get_unique_circ_id_by_chan(chan2);
    tt_uint_op((1u<<31), OP_LT, circid);
  }

  /* Now make sure that we can behave well when we are full up on circuits */
  chan1->circ_id_type = CIRC_ID_TYPE_LOWER;
  chan2->circ_id_type = CIRC_ID_TYPE_LOWER;
  chan1->wide_circ_ids = chan2->wide_circ_ids = 0;
  ba = bitarray_init_zero((1<<15));
  for (i = 0; i < (1<<15); ++i) {
    circid = get_unique_circ_id_by_chan(chan1);
    if (circid == 0) {
      tt_int_op(i, OP_GT, (1<<14));
      break;
    }
    tt_uint_op(circid, OP_LT, (1<<15));
    tt_assert(! bitarray_is_set(ba, circid));
    bitarray_set(ba, circid);
    channel_mark_circid_unusable(chan1, circid);
  }
  tt_int_op(i, OP_LT, (1<<15));
  /* Make sure that being full on chan1 does not interfere with chan2 */
  for (i = 0; i < 100; ++i) {
    circid = get_unique_circ_id_by_chan(chan2);
    tt_uint_op(circid, OP_GT, 0);
    tt_uint_op(circid, OP_LT, (1<<15));
    channel_mark_circid_unusable(chan2, circid);
  }

 done:
  circuitmux_free(chan1->cmux);
  circuitmux_free(chan2->cmux);
  tor_free(chan1);
  tor_free(chan2);
  bitarray_free(ba);
  circuit_free_all();
  teardown_capture_of_logs();
  UNMOCK(channel_dump_statistics);
}

/** Test that the circuit pools of our HS circuitmap are isolated based on
 *  their token type. */
static void
test_hs_circuitmap_isolation(void *arg)
{
  or_circuit_t *circ1 = NULL;
  origin_circuit_t *circ2 = NULL;
  or_circuit_t *circ3 = NULL;
  origin_circuit_t *circ4 = NULL;

  (void)arg;

  hs_circuitmap_init();

  {
    const uint8_t tok1[REND_TOKEN_LEN] = "bet i got some of th";

    circ1 = or_circuit_new(0, NULL);
    tt_assert(circ1);
    circ1->base_.purpose = CIRCUIT_PURPOSE_REND_POINT_WAITING;

    /* check that circuitmap is empty right? */
    tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok1));

    /* Register circ1 with tok1 as relay-side rend circ */
    hs_circuitmap_register_rend_circ_relay_side(circ1, tok1);

    /* check that service-side getters don't work */
    tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_service_side(tok1));
    tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_intro_circ_v2_service_side(tok1));

    /* Check that the right getter works. */
    tt_ptr_op(circ1, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok1));
  }

  {
    const uint8_t tok2[REND_TOKEN_LEN] = "you dont know anythi";

    circ2 = origin_circuit_new();
    tt_assert(circ2);
    circ2->base_.purpose = CIRCUIT_PURPOSE_S_ESTABLISH_INTRO;
    circ3 = or_circuit_new(0, NULL);
    tt_assert(circ3);
    circ3->base_.purpose = CIRCUIT_PURPOSE_INTRO_POINT;
    circ4 = origin_circuit_new();
    tt_assert(circ4);
    circ4->base_.purpose = CIRCUIT_PURPOSE_S_ESTABLISH_INTRO;

    /* Register circ2 with tok2 as service-side intro v2 circ */
    hs_circuitmap_register_intro_circ_v2_service_side(circ2, tok2);
    /* Register circ3 with tok2 again but for different purpose */
    hs_circuitmap_register_intro_circ_v2_relay_side(circ3, tok2);

    /* Check that the getters work */
    tt_ptr_op(circ2, OP_EQ,
              hs_circuitmap_get_intro_circ_v2_service_side(tok2));
    tt_ptr_op(circ3, OP_EQ, hs_circuitmap_get_intro_circ_v2_relay_side(tok2));

    /* Register circ4 with tok2: it should override circ2 */
    hs_circuitmap_register_intro_circ_v2_service_side(circ4, tok2);

    /* check that relay-side getters don't work */
    tt_ptr_op(NULL, OP_EQ, hs_circuitmap_get_rend_circ_relay_side(tok2));

    /* Check that the getter returns circ4; the last circuit registered with
     * that token. */
    tt_ptr_op(circ4, OP_EQ,
              hs_circuitmap_get_intro_circ_v2_service_side(tok2));
  }

 done:
  circuit_free_(TO_CIRCUIT(circ1));
  circuit_free_(TO_CIRCUIT(circ2));
  circuit_free_(TO_CIRCUIT(circ3));
  circuit_free_(TO_CIRCUIT(circ4));
}

struct testcase_t circuitlist_tests[] = {
  { "maps", test_clist_maps, TT_FORK, NULL, NULL },
  { "rend_token_maps", test_rend_token_maps, TT_FORK, NULL, NULL },
  { "pick_circid", test_pick_circid, TT_FORK, NULL, NULL },
  { "hs_circuitmap_isolation", test_hs_circuitmap_isolation,
    TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
