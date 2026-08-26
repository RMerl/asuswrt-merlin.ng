/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define RELAY_CRYPTO_CGO_PRIVATE
#define USE_AES_RAW

#include "orconfig.h"
#include "core/or/or.h"
#include "test/test.h"
#include "lib/cc/compat_compiler.h"
#include "lib/crypt_ops/aes.h"
#include "ext/polyval/polyval.h"
#include "core/crypto/relay_crypto_cgo.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "core/or/cell_st.h"

#include "test/cgo_vectors.inc"

static const int AESBITS[] = { 128, 192, 256 };

static void
test_crypto_cgo_et_roundtrip(void *arg)
{
  (void)arg;
  uint8_t key[32 + 16]; // max
  uint8_t tweak_h[16];
  uint8_t tweak_x_r[493];
  uint8_t block[16], block_orig[16];
  cgo_et_t et1, et2;
  memset(&et1, 0, sizeof(et1));
  memset(&et2, 0, sizeof(et2));

  et_tweak_t tweak = {
    .uiv = {
      .h = tweak_h,
      .cmd = 7,
    },
    .x_r = tweak_x_r,
  };

  for (int bi = 0; bi < (int) ARRAY_LENGTH(AESBITS); ++bi) {
    const int aesbits = AESBITS[bi];

    for (int i = 0; i < 16; ++i) {
      crypto_rand((char*)key, sizeof(key));
      crypto_rand((char*)tweak_h, sizeof(tweak_h));
      crypto_rand((char*)tweak_x_r, sizeof(tweak_x_r));
      crypto_rand((char*)block_orig, sizeof(block_orig));
      memcpy(block, block_orig, 16);
      cgo_et_init(&et1, aesbits, true, key);
      cgo_et_init(&et2, aesbits, false, key);

      // encrypt-then-decrypt should round-trip.
      cgo_et_encrypt(&et1, tweak, block);
      tt_mem_op(block, OP_NE, block_orig, 16);
      cgo_et_decrypt(&et2, tweak, block);
      tt_mem_op(block, OP_EQ, block_orig, 16);

      // decrypt-then-encrypt should round-trip.
      cgo_et_decrypt(&et2, tweak, block);
      tt_mem_op(block, OP_NE, block_orig, 16);
      cgo_et_encrypt(&et1, tweak, block);
      tt_mem_op(block, OP_EQ, block_orig, 16);

      cgo_et_clear(&et1);
      cgo_et_clear(&et2);
    }
  }
 done:
  cgo_et_clear(&et1);
  cgo_et_clear(&et2);
}

static void
test_crypto_cgo_uiv_roundtrip(void *arg)
{
  (void)arg;
  uint8_t key[64 + 32]; // max
  uint8_t tweak_h[16];
  uint8_t cell[509], cell_orig[509];
  cgo_uiv_t uiv1, uiv2;
  memset(&uiv1, 0, sizeof(uiv1));
  memset(&uiv2, 0, sizeof(uiv2));

  uiv_tweak_t tweak = {
    .h = tweak_h,
    .cmd = 4,
  };

  for (int bi = 0; bi < (int) ARRAY_LENGTH(AESBITS); ++bi) {
    const int aesbits = AESBITS[bi];

    for (int i = 0; i < 16; ++i) {
      crypto_rand((char*)key, sizeof(key));
      crypto_rand((char*)tweak_h, sizeof(tweak_h));
      crypto_rand((char*)cell_orig, sizeof(cell_orig));
      memcpy(cell, cell_orig, sizeof(cell_orig));

      cgo_uiv_init(&uiv1, aesbits, true, key);
      cgo_uiv_init(&uiv2, aesbits, false, key);

      // encrypt-then-decrypt should round-trip.
      cgo_uiv_encrypt(&uiv1, tweak, cell);
      tt_mem_op(cell, OP_NE, cell_orig, sizeof(cell));
      cgo_uiv_decrypt(&uiv2, tweak, cell);
      tt_mem_op(cell, OP_EQ, cell_orig, sizeof(cell));

      // decrypt-then-encrypt should round-trip.
      cgo_uiv_decrypt(&uiv2, tweak, cell);
      tt_mem_op(cell, OP_NE, cell_orig, sizeof(cell));
      cgo_uiv_encrypt(&uiv1, tweak, cell);
      tt_mem_op(cell, OP_EQ, cell_orig, sizeof(cell));

      cgo_uiv_clear(&uiv1);
      cgo_uiv_clear(&uiv2);
    }
  }
 done:
  cgo_uiv_clear(&uiv1);
  cgo_uiv_clear(&uiv2);
}

#define UNHEX(out,inp) STMT_BEGIN {                                     \
    size_t inplen = strlen(inp);                                        \
    tt_int_op(sizeof(out), OP_EQ, inplen / 2);                          \
    int r = base16_decode((char*)(out), sizeof(out), inp, inplen);      \
    tt_int_op(r, OP_EQ, sizeof(out));                                   \
  } STMT_END

#define UNHEX2(out,inp1,inp2) STMT_BEGIN {                              \
    tor_free(unhex_tmp);                                                \
    tor_asprintf(&unhex_tmp, "%s%s", inp1, inp2);                       \
    UNHEX(out, unhex_tmp);                                              \
  } STMT_END

static void
test_crypto_cgo_et_testvec(void *arg)
{
  (void)arg;
  cgo_et_t et;
  memset(&et, 0, sizeof(et));

  for (int i = 0; i < (int)ARRAY_LENGTH(ET_TESTVECS); ++i) {
    const struct et_testvec *tv = &ET_TESTVECS[i];
    uint8_t keys[32];
    uint8_t tweaks[16 + 1 + 493];
    uint8_t block[16], expect[16];
    UNHEX(keys, tv->keys);
    UNHEX(tweaks, tv->tweaks);
    UNHEX(block, tv->block);
    UNHEX(expect, tv->expect);

    et_tweak_t tweak = {
      .uiv = {
        .h = tweaks,
        .cmd = tweaks[16],
      },
      .x_r = tweaks + 17,
    };

    cgo_et_init(&et, 128, tv->encrypt, keys);
    if (tv->encrypt) {
      cgo_et_encrypt(&et, tweak, block);
    } else {
      cgo_et_decrypt(&et, tweak, block);
    }
    cgo_et_clear(&et);

    tt_mem_op(block, OP_EQ, expect, 16);
  }

 done:
  cgo_et_clear(&et);
}

static void
test_crypto_cgo_prf_testvec(void *arg)
{
  (void)arg;
  cgo_prf_t prf;
  memset(&prf, 0, sizeof(prf));

  for (int i = 0; i < (int)ARRAY_LENGTH(PRF_TESTVECS); ++i) {
    const struct prf_testvec *tv = &PRF_TESTVECS[i];
    uint8_t keys[32];
    uint8_t input[16];
    uint8_t expect_t0[493];
    uint8_t expect_t1[80];
    uint8_t output[493]; // max
    UNHEX(keys, tv->keys);
    UNHEX(input, tv->input);

    cgo_prf_init(&prf, 128, keys);
    if (tv->t == 0) {
      UNHEX(expect_t0, tv->expect);
      memset(output, 0, sizeof(output));
      cgo_prf_xor_t0(&prf, input, output);
      tt_mem_op(output, OP_EQ, expect_t0, PRF_T0_DATA_LEN);
    } else {
      tt_int_op(tv->t, OP_EQ, 1);
      UNHEX(expect_t1, tv->expect);
      cgo_prf_gen_t1(&prf, input, output, sizeof(expect_t1));
      tt_mem_op(output, OP_EQ, expect_t1, sizeof(expect_t1));
    }
    cgo_prf_clear(&prf);
  }
 done:
  cgo_prf_clear(&prf);
}

static void
test_crypto_cgo_uiv_testvec(void *arg)
{
  (void)arg;
  cgo_uiv_t uiv;
  memset(&uiv, 0, sizeof(uiv));

  for (int i = 0; i < (int)ARRAY_LENGTH(UIV_TESTVECS); ++i) {
    const struct uiv_testvec *tv = &UIV_TESTVECS[i];
    uint8_t keys[64];
    uint8_t tweaks[17];
    uint8_t x_l[16], x_r[493];
    uint8_t y_l[16], y_r[493];
    uint8_t cell[509];
    UNHEX(keys, tv->keys);
    UNHEX(tweaks, tv->tweaks);
    UNHEX(x_l, tv->x_l);
    UNHEX(x_r, tv->x_r);
    UNHEX(y_l, tv->y.y_l);
    UNHEX(y_r, tv->y.y_r);

    uiv_tweak_t tweak = {
      .h = tweaks,
      .cmd = tweaks[16]
    };
    memcpy(cell, x_l, 16);
    memcpy(cell+16, x_r, 493);

    cgo_uiv_init(&uiv, 128, tv->encrypt, keys);
    if (tv->encrypt) {
      cgo_uiv_encrypt(&uiv, tweak, cell);
    } else {
      cgo_uiv_decrypt(&uiv, tweak, cell);
    }
    cgo_uiv_clear(&uiv);

    tt_mem_op(cell, OP_EQ, y_l, 16);
    tt_mem_op(cell+16, OP_EQ, y_r, 493);
  }

 done:
  cgo_uiv_clear(&uiv);
}

#include "core/crypto/relay_crypto_st.h"

static void
test_crypto_cgo_uiv_update_testvec(void *arg)
{
  (void)arg;
  cgo_uiv_t uiv;
  cgo_uiv_t uiv2;
  memset(&uiv, 0, sizeof(uiv));
  memset(&uiv2, 0, sizeof(uiv2));

  uint8_t tw[16];
  memset(tw, 42, sizeof(tw));
  uiv_tweak_t tweak = {
      .h = tw,
      .cmd = 42
  };

  for (int i = 0; i < (int)ARRAY_LENGTH(UIV_UPDATE_TESTVECS); ++i) {
    const struct uiv_update_testvec *tv = &UIV_UPDATE_TESTVECS[i];
    uint8_t keys[64];
    uint8_t nonce[16];
    uint8_t new_keys[64];
    uint8_t new_nonce[16];
    UNHEX(keys, tv->keys);
    UNHEX(nonce, tv->nonce);
    UNHEX(new_keys, tv->output.new_keys);
    UNHEX(new_nonce, tv->output.new_nonce);

    cgo_uiv_init(&uiv, 128, true, keys);
    cgo_uiv_update(&uiv, 128, true, nonce);
    // Make sure that the recorded keys are what we expect.
    tt_mem_op(uiv.uiv_keys_, OP_EQ, new_keys, 64);
    tt_mem_op(nonce, OP_EQ, new_nonce, 16);

    // Construct a new UIV from these keys and make sure it acts like this one.
    uint8_t cell[509], cell2[509];
    crypto_rand((char*)cell, sizeof(cell));
    memcpy(cell2, cell, 509);
    cgo_uiv_init(&uiv2, 128, true, uiv.uiv_keys_);
    cgo_uiv_encrypt(&uiv, tweak, cell);
    cgo_uiv_encrypt(&uiv2, tweak, cell2);
    tt_mem_op(cell, OP_EQ, cell2, 509);

    cgo_uiv_clear(&uiv);
    cgo_uiv_clear(&uiv2);
  }
 done:
  cgo_uiv_clear(&uiv);
  cgo_uiv_clear(&uiv2);
}

static void
test_crypto_cgo_fwd(void *arg)
{
  (void)arg;

  #define N_HOPS 3
  uint8_t key_material[N_HOPS][112]; // max.
  cgo_crypt_t *client[N_HOPS];
  cgo_crypt_t *relays[N_HOPS];

  memset(client, 0, sizeof(client));
  memset(relays, 0, sizeof(relays));

  for (int bi = 0; bi < (int) ARRAY_LENGTH(AESBITS); ++bi) {
    const int aesbits = AESBITS[bi];

    size_t klen = cgo_key_material_len(aesbits);
    tt_uint_op(klen, OP_LE, sizeof(key_material[0]));
    crypto_rand((char*)&key_material, sizeof(key_material));
    for (int i = 0; i < N_HOPS; ++i) {
      client[i] = cgo_crypt_new(CGO_MODE_CLIENT_FORWARD,
                                aesbits, key_material[i], klen);
      relays[i] = cgo_crypt_new(CGO_MODE_RELAY_FORWARD,
                                aesbits, key_material[i], klen);
    }
    for (int trial = 0; trial < 64; ++trial) {
      int target_hop = crypto_rand_int(3);
      cell_t cell, cell_orig;
      uint8_t tag_client[SENDME_TAG_LEN_CGO];
      const uint8_t *tagp = NULL;

      memset(&cell, 0, sizeof(cell));
      if (crypto_rand_int(2) == 0) {
        cell.command = CELL_RELAY;
      } else {
        cell.command = CELL_RELAY_EARLY;
      }
      crypto_rand((char*) cell.payload+SENDME_TAG_LEN_CGO,
                  sizeof(cell.payload)-SENDME_TAG_LEN_CGO);
      memcpy(&cell_orig, &cell, sizeof(cell));

      // First the client encrypts the cell...
      cgo_crypt_client_originate(client[target_hop], &cell, &tagp);
      tt_assert(tagp);
      memcpy(tag_client, tagp, SENDME_TAG_LEN_CGO);
      for (int i = target_hop - 1; i >= 0; --i) {
        cgo_crypt_client_forward(client[i], &cell);
      }

      // Now the relays handle the cell...
      bool cell_recognized = false;
      for (int i = 0; i < N_HOPS; ++i) {
        tagp = NULL;
        cgo_crypt_relay_forward(relays[i], &cell, &tagp);
        if (tagp) {
          tt_int_op(i, OP_EQ, target_hop);
          tt_mem_op(tagp, OP_EQ, tag_client, SENDME_TAG_LEN_CGO);
          cell_recognized = true;
          break;
        }
      }
      tt_assert(cell_recognized);
      tt_int_op(cell.command, OP_EQ, cell_orig.command);
      tt_mem_op(cell.payload + SENDME_TAG_LEN_CGO, OP_EQ,
                cell_orig.payload + SENDME_TAG_LEN_CGO,
                sizeof(cell.payload) - SENDME_TAG_LEN_CGO);
    }
    for (int i = 0; i < N_HOPS; ++i) {
      cgo_crypt_free(client[i]);
      cgo_crypt_free(relays[i]);
    }
  }

 done:
  for (int i = 0; i < N_HOPS; ++i) {
    cgo_crypt_free(client[i]);
    cgo_crypt_free(relays[i]);
  }
#undef N_HOPS
}

static void
test_crypto_cgo_rev(void *arg)
{
  (void)arg;

  #define N_HOPS 3
  uint8_t key_material[N_HOPS][112]; // max.
  cgo_crypt_t *client[N_HOPS];
  cgo_crypt_t *relays[N_HOPS];

  memset(client, 0, sizeof(client));
  memset(relays, 0, sizeof(relays));

  for (int bi = 0; bi < (int) ARRAY_LENGTH(AESBITS); ++bi) {
    const int aesbits = AESBITS[bi];

    size_t klen = cgo_key_material_len(aesbits);
    tt_uint_op(klen, OP_LE, sizeof(key_material[0]));
    crypto_rand((char*)&key_material, sizeof(key_material));
    for (int i = 0; i < N_HOPS; ++i) {
      client[i] = cgo_crypt_new(CGO_MODE_CLIENT_BACKWARD,
                                 aesbits, key_material[i], klen);
      relays[i] = cgo_crypt_new(CGO_MODE_RELAY_BACKWARD,
                                aesbits, key_material[i], klen);
    }
    for (int trial = 0; trial < 64; ++trial) {
      int origin_hop = crypto_rand_int(3);
      cell_t cell, cell_orig;
      uint8_t tag_relay[SENDME_TAG_LEN_CGO];
      const uint8_t *tagp = NULL;

      memset(&cell, 0, sizeof(cell));
      cell.command = CELL_RELAY;
      crypto_rand((char*) cell.payload+SENDME_TAG_LEN_CGO,
                  sizeof(cell.payload)-SENDME_TAG_LEN_CGO);
      memcpy(&cell_orig, &cell, sizeof(cell));

      // First the specified relay encrypts the cell...
      cgo_crypt_relay_originate(relays[origin_hop], &cell, &tagp);
      tt_assert(tagp);
      memcpy(tag_relay, tagp, SENDME_TAG_LEN_CGO);
      for (int i = origin_hop - 1; i >= 0; --i) {
        cgo_crypt_relay_backward(relays[i], &cell);
      }

      // Now the client handles the cell.
      bool cell_recognized = false;
      for (int i = 0; i < N_HOPS; ++i) {
        tagp = NULL;
        cgo_crypt_client_backward(client[i], &cell, &tagp);
        if (tagp) {
          tt_int_op(i, OP_EQ, origin_hop);
          tt_mem_op(tagp, OP_EQ, tag_relay, SENDME_TAG_LEN_CGO);
          cell_recognized = true;
          break;
        }
      }
      tt_assert(cell_recognized);
      tt_int_op(cell.command, OP_EQ, cell_orig.command);
      tt_mem_op(cell.payload + SENDME_TAG_LEN_CGO, OP_EQ,
                cell_orig.payload + SENDME_TAG_LEN_CGO,
                sizeof(cell.payload) - SENDME_TAG_LEN_CGO);
    }
    for (int i = 0; i < N_HOPS; ++i) {
      cgo_crypt_free(client[i]);
      cgo_crypt_free(relays[i]);
    }
  }

 done:
  for (int i = 0; i < N_HOPS; ++i) {
    cgo_crypt_free(client[i]);
    cgo_crypt_free(relays[i]);
  }
#undef N_HOPS
}

static void
test_crypto_cgo_relay_testvec(void *arg)
{
  (void)arg;
  char *unhex_tmp = NULL;
  cgo_crypt_t *cgo = NULL;
  for (int i = 0; i < (int)ARRAY_LENGTH(CGO_RELAY_TESTVECS); ++i) {
    const struct cgo_relay_testvec *tv = &CGO_RELAY_TESTVECS[i];
    const int aesbits = 128;
    uint8_t keys[80], expect_keys[80], expect_tprime[SENDME_TAG_LEN_CGO],
      cmd[1];
    cell_t cell;
    cell_t expect_cell;
    tt_int_op(sizeof(keys), OP_EQ, cgo_key_material_len(aesbits));
    UNHEX2(keys, tv->state_in.keys, tv->state_in.nonce);
    cgo_mode_t mode =
      tv->inbound ? CGO_MODE_RELAY_BACKWARD : CGO_MODE_RELAY_FORWARD;
    cgo = cgo_crypt_new(mode, aesbits, keys, sizeof(keys));
    tt_assert(cgo);
    UNHEX(cgo->tprime, tv->state_in.tprime);
    memset(&cell, 0, sizeof(cell));
    UNHEX(cmd, tv->cmd);
    cell.command = cmd[0];
    UNHEX2(cell.payload, tv->tag, tv->msg);

    memset(&expect_cell, 0, sizeof(expect_cell));
    expect_cell.command = cell.command;
    UNHEX2(expect_cell.payload,
           tv->output.result.t_out, tv->output.result.msg_out);
    UNHEX2(expect_keys,
           tv->output.state.keys, tv->output.state.nonce);
    UNHEX(expect_tprime, tv->output.state.tprime);

    if (tv->inbound) {
      cgo_crypt_relay_backward(cgo, &cell);
    } else {
      const uint8_t *tagp = NULL;
      cgo_crypt_relay_forward(cgo, &cell, &tagp);
      tt_ptr_op(tagp, OP_EQ, NULL);
    }

    tt_mem_op(cell.payload, OP_EQ, expect_cell.payload, sizeof(cell.payload));
    tt_mem_op(cgo->uiv.uiv_keys_, OP_EQ, expect_keys, 64);
    tt_mem_op(cgo->nonce, OP_EQ, expect_keys + 64, 16);
    tt_mem_op(cgo->tprime, OP_EQ, expect_tprime, 16);

    cgo_crypt_free(cgo);
  }
 done:
  tor_free(unhex_tmp);
  cgo_crypt_free(cgo);
}

static void
test_crypto_cgo_relay_originate_testvec(void *arg)
{
  (void)arg;
  char *unhex_tmp = NULL;
  cgo_crypt_t *cgo = NULL;
  for (int i = 0; i < (int)ARRAY_LENGTH(CGO_RELAY_ORIGINATE_TESTVECS); ++i) {
    const struct cgo_relay_originate_testvec *tv =
      &CGO_RELAY_ORIGINATE_TESTVECS[i];
    const int aesbits = 128;
    uint8_t keys[80], expect_keys[80], expect_tprime[SENDME_TAG_LEN_CGO],
      cmd[1];
    cell_t cell;
    cell_t expect_cell;
    tt_int_op(sizeof(keys), OP_EQ, cgo_key_material_len(aesbits));
    UNHEX2(keys, tv->state_in.keys, tv->state_in.nonce);
    cgo = cgo_crypt_new(CGO_MODE_RELAY_BACKWARD, aesbits, keys, sizeof(keys));
    tt_assert(cgo);
    UNHEX(cgo->tprime, tv->state_in.tprime);
    memset(&cell, 0, sizeof(cell));
    UNHEX(cmd, tv->cmd);
    cell.command = cmd[0];
    UNHEX2(cell.payload, "00000000000000000000000000000000", tv->msg);

    memset(&expect_cell, 0, sizeof(expect_cell));
    expect_cell.command = cell.command;
    UNHEX2(expect_cell.payload,
           tv->output.result.t_out, tv->output.result.msg_out);
    UNHEX2(expect_keys,
           tv->output.state.keys, tv->output.state.nonce);
    UNHEX(expect_tprime, tv->output.state.tprime);

    const uint8_t *tagp = NULL;
    cgo_crypt_relay_originate(cgo, &cell, &tagp);
    tt_ptr_op(tagp, OP_NE, NULL);

    tt_mem_op(cell.payload, OP_EQ, expect_cell.payload, sizeof(cell.payload));
    tt_mem_op(cgo->uiv.uiv_keys_, OP_EQ, expect_keys, 64);
    tt_mem_op(cgo->nonce, OP_EQ, expect_keys + 64, 16);
    tt_mem_op(cgo->tprime, OP_EQ, expect_tprime, 16);

    cgo_crypt_free(cgo);
  }
 done:
  tor_free(unhex_tmp);
  cgo_crypt_free(cgo);
}

static void
test_crypto_cgo_client_originate_testvec(void *arg)
{
  (void) arg;
  char *unhex_tmp = NULL;
  cgo_crypt_t *cgo[3] = { NULL, NULL, NULL };
  for (int tv_i = 0; tv_i < (int)ARRAY_LENGTH(CGO_CLIENT_ORIGINATE_TESTVECS);
       ++tv_i) {
    const struct cgo_client_originate_testvec *tv =
      &CGO_CLIENT_ORIGINATE_TESTVECS[tv_i];
    const int aesbits = 128;
    uint8_t keys[80], expect_keys[80], expect_tprime[SENDME_TAG_LEN_CGO],
      cmd[1];
    cell_t cell;
    cell_t expect_cell;
    for (int i = 0; i < 3; ++i) {
      tt_int_op(sizeof(keys), OP_EQ, cgo_key_material_len(aesbits));
      UNHEX2(keys, tv->state_in[i].keys, tv->state_in[i].nonce);
      cgo[i] = cgo_crypt_new(CGO_MODE_CLIENT_FORWARD,
                             aesbits, keys, sizeof(keys));
      tt_assert(cgo[i]);
      UNHEX(cgo[i]->tprime, tv->state_in[i].tprime);
    }

    memset(&cell, 0, sizeof(cell));
    UNHEX(cmd, tv->cmd);
    cell.command = cmd[0];
    UNHEX2(cell.payload, "00000000000000000000000000000000", tv->msg);

    memset(&expect_cell, 0, sizeof(expect_cell));
    expect_cell.command = cell.command;
    UNHEX2(expect_cell.payload,
           tv->output.result.t_out, tv->output.result.msg_out);

    tt_int_op(tv->target_hop, OP_GE, 1); // Hop is 0-indexed.
    int target_hop = tv->target_hop - 1;
    const uint8_t *tagp = NULL;
    cgo_crypt_client_originate(cgo[target_hop], &cell, &tagp);
    tt_ptr_op(tagp, OP_NE, NULL);
    for (int i = target_hop - 1; i >= 0; --i) {
      cgo_crypt_client_forward(cgo[i], &cell);
    }
    tt_mem_op(cell.payload, OP_EQ, expect_cell.payload, sizeof(cell.payload));

    for (int i = 0; i < 3; ++i) {
      UNHEX2(expect_keys,
             tv->output.state[i].keys, tv->output.state[i].nonce);
      UNHEX(expect_tprime, tv->output.state[i].tprime);

      tt_mem_op(cgo[i]->uiv.uiv_keys_, OP_EQ, expect_keys, 64);
      tt_mem_op(cgo[i]->nonce, OP_EQ, expect_keys + 64, 16);
      tt_mem_op(cgo[i]->tprime, OP_EQ, expect_tprime, 16);
    }

    for (int i = 0; i < 3; ++i)
      cgo_crypt_free(cgo[i]);
  }
 done:
  tor_free(unhex_tmp);
  for (int i = 0; i < 3; ++i)
    cgo_crypt_free(cgo[i]);
}

struct testcase_t crypto_cgo_tests[] = {
  { "et_roundtrip", test_crypto_cgo_et_roundtrip, 0, NULL, NULL },
  { "et_testvec", test_crypto_cgo_et_testvec, 0, NULL, NULL },
  { "prf_testvec", test_crypto_cgo_prf_testvec, 0, NULL, NULL },
  { "uiv_roundtrip", test_crypto_cgo_uiv_roundtrip, 0, NULL, NULL },
  { "uiv_testvec", test_crypto_cgo_uiv_testvec, 0, NULL, NULL },
  { "uiv_update_testvec", test_crypto_cgo_uiv_update_testvec, 0, NULL, NULL },
  { "cgo_fwd", test_crypto_cgo_fwd, 0, NULL, NULL, },
  { "cgo_rev", test_crypto_cgo_rev, 0, NULL, NULL, },
  { "cgo_relay_testvec", test_crypto_cgo_relay_testvec, 0, NULL, NULL },
  { "cgo_relay_originate_testvec", test_crypto_cgo_relay_originate_testvec,
    0, NULL, NULL },
  { "cgo_client_originate_testvec", test_crypto_cgo_client_originate_testvec,
    0, NULL, NULL },
  END_OF_TESTCASES
};
