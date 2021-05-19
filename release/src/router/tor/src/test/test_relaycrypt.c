/* Copyright 2001-2004 Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CRYPT_PATH_PRIVATE

#include "core/or/or.h"
#include "core/or/circuitbuild.h"
#define CIRCUITLIST_PRIVATE
#include "core/or/circuitlist.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "core/or/relay.h"
#include "core/crypto/relay_crypto.h"
#include "core/or/crypt_path.h"
#include "core/or/cell_st.h"
#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

#include "test/test.h"

static const char KEY_MATERIAL[3][CPATH_KEY_MATERIAL_LEN] = {
  "    'My public key is in this signed x509 object', said Tom assertively.",
  "'Let's chart the pedal phlanges in the tomb', said Tom cryptographically",
  "     'Segmentation fault bugs don't _just happen_', said Tom seethingly.",
};

typedef struct testing_circuitset_t {
  or_circuit_t *or_circ[3];
  origin_circuit_t *origin_circ;
} testing_circuitset_t;

static int testing_circuitset_teardown(const struct testcase_t *testcase,
                                       void *ptr);

static void *
testing_circuitset_setup(const struct testcase_t *testcase)
{
  testing_circuitset_t *cs = tor_malloc_zero(sizeof(testing_circuitset_t));
  int i;

  for (i=0; i<3; ++i) {
    cs->or_circ[i] = or_circuit_new(0, NULL);
    tt_int_op(0, OP_EQ,
              relay_crypto_init(&cs->or_circ[i]->crypto,
                                KEY_MATERIAL[i], sizeof(KEY_MATERIAL[i]),
                                0, 0));
  }

  cs->origin_circ = origin_circuit_new();
  cs->origin_circ->base_.purpose = CIRCUIT_PURPOSE_C_GENERAL;
  for (i=0; i<3; ++i) {
    crypt_path_t *hop = tor_malloc_zero(sizeof(*hop));
    relay_crypto_init(&hop->pvt_crypto, KEY_MATERIAL[i],
                      sizeof(KEY_MATERIAL[i]), 0, 0);
    hop->state = CPATH_STATE_OPEN;
    cpath_extend_linked_list(&cs->origin_circ->cpath, hop);
    tt_ptr_op(hop, OP_EQ, cs->origin_circ->cpath->prev);
  }

  return cs;
 done:
  testing_circuitset_teardown(testcase, cs);
  return NULL;
}

static int
testing_circuitset_teardown(const struct testcase_t *testcase, void *ptr)
{
  (void)testcase;
  testing_circuitset_t *cs = ptr;
  int i;
  for (i=0; i<3; ++i) {
    circuit_free_(TO_CIRCUIT(cs->or_circ[i]));
  }
  circuit_free_(TO_CIRCUIT(cs->origin_circ));
  tor_free(cs);
  return 1;
}

static const struct testcase_setup_t relaycrypt_setup = {
  testing_circuitset_setup, testing_circuitset_teardown
};

/* Test encrypting a cell to the final hop on a circuit, decrypting it
 * at each hop, and recognizing it at the other end.  Then do it again
 * and again as the state evolves. */
static void
test_relaycrypt_outbound(void *arg)
{
  testing_circuitset_t *cs = arg;
  tt_assert(cs);

  relay_header_t rh;
  cell_t orig;
  cell_t encrypted;
  int i, j;

  for (i = 0; i < 50; ++i) {
    crypto_rand((char *)&orig, sizeof(orig));

    relay_header_unpack(&rh, orig.payload);
    rh.recognized = 0;
    memset(rh.integrity, 0, sizeof(rh.integrity));
    relay_header_pack(orig.payload, &rh);

    memcpy(&encrypted, &orig, sizeof(orig));

    /* Encrypt the cell to the last hop */
    relay_encrypt_cell_outbound(&encrypted, cs->origin_circ,
                                cs->origin_circ->cpath->prev);

    for (j = 0; j < 3; ++j) {
      crypt_path_t *layer_hint = NULL;
      char recognized = 0;
      int r = relay_decrypt_cell(TO_CIRCUIT(cs->or_circ[j]),
                                 &encrypted,
                                 CELL_DIRECTION_OUT,
                                 &layer_hint, &recognized);
      tt_int_op(r, OP_EQ, 0);
      tt_ptr_op(layer_hint, OP_EQ, NULL);
      tt_int_op(recognized != 0, OP_EQ, j == 2);
    }

    tt_mem_op(orig.payload, OP_EQ, encrypted.payload, CELL_PAYLOAD_SIZE);
  }

 done:
  ;
}

/* As above, but simulate inbound cells from the last hop. */
static void
test_relaycrypt_inbound(void *arg)
{
  testing_circuitset_t *cs = arg;
  tt_assert(cs);

  relay_header_t rh;
  cell_t orig;
  cell_t encrypted;
  int i, j;

  for (i = 0; i < 50; ++i) {
    crypto_rand((char *)&orig, sizeof(orig));

    relay_header_unpack(&rh, orig.payload);
    rh.recognized = 0;
    memset(rh.integrity, 0, sizeof(rh.integrity));
    relay_header_pack(orig.payload, &rh);

    memcpy(&encrypted, &orig, sizeof(orig));

    /* Encrypt the cell to the last hop */
    relay_encrypt_cell_inbound(&encrypted, cs->or_circ[2]);

    crypt_path_t *layer_hint = NULL;
    char recognized = 0;
    int r;
    for (j = 1; j >= 0; --j) {
      r = relay_decrypt_cell(TO_CIRCUIT(cs->or_circ[j]),
                             &encrypted,
                             CELL_DIRECTION_IN,
                             &layer_hint, &recognized);
      tt_int_op(r, OP_EQ, 0);
      tt_ptr_op(layer_hint, OP_EQ, NULL);
      tt_int_op(recognized, OP_EQ, 0);
    }

    relay_decrypt_cell(TO_CIRCUIT(cs->origin_circ),
                       &encrypted,
                       CELL_DIRECTION_IN,
                       &layer_hint, &recognized);
    tt_int_op(r, OP_EQ, 0);
    tt_int_op(recognized, OP_EQ, 1);
    tt_ptr_op(layer_hint, OP_EQ, cs->origin_circ->cpath->prev);

    tt_mem_op(orig.payload, OP_EQ, encrypted.payload, CELL_PAYLOAD_SIZE);
  }
 done:
  ;
}

#define TEST(name) \
  { # name, test_relaycrypt_ ## name, 0, &relaycrypt_setup, NULL }

struct testcase_t relaycrypt_tests[] = {
  TEST(outbound),
  TEST(inbound),
  END_OF_TESTCASES
};

