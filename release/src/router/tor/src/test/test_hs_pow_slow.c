/* Copyright (c) 2020-2023, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_hs_pow_slow.c
 * \brief Slower (solve + verify) tests for service proof-of-work defenses.
 */

#define HS_SERVICE_PRIVATE

#include "lib/cc/compat_compiler.h"
#include "lib/cc/torint.h"

#include "test/test.h"
#include "test/test_helpers.h"
#include "test/log_test_helpers.h"
#include "test/rng_test_helpers.h"

#include "app/config/config.h"
#include "feature/hs/hs_pow.h"

static int
testing_one_hs_pow_solution(const hs_pow_solution_t *ref_solution,
                            const ed25519_public_key_t *service_blinded_id,
                            const uint8_t *seed)
{
  int retval = -1;
  hs_pow_solution_t sol_buffer;
  hs_pow_service_state_t *s = tor_malloc_zero(sizeof(hs_pow_service_state_t));
  s->rend_request_pqueue = smartlist_new();

  memcpy(s->seed_previous, seed, HS_POW_SEED_LEN);

  const unsigned num_variants = 10;
  const unsigned num_attempts = 3;

  for (unsigned variant = 0; variant < num_variants; variant++) {
    hs_pow_remove_seed_from_cache(seed);

    for (unsigned attempt = 0; attempt < num_attempts; attempt++) {
      int expected = -1;
      memcpy(&sol_buffer, ref_solution, sizeof sol_buffer);

      /* One positive test, and a few negative tests of corrupted solutions */
      if (variant == 0) {
        if (attempt == 0) {
          /* Only the first attempt should succeed (nonce replay) */
          expected = 0;
        }
      } else if (variant & 1) {
        sol_buffer.nonce[variant / 2 % HS_POW_NONCE_LEN]++;
      } else {
        sol_buffer.equix_solution[variant / 2 % HS_POW_EQX_SOL_LEN]++;
      }

      tt_int_op(expected, OP_EQ,
                hs_pow_verify(service_blinded_id, s, &sol_buffer));
    }
  }

  retval = 0;
done:
  hs_pow_free_service_state(s);
  return retval;
}

static void
test_hs_pow_vectors(void *arg)
{
  (void)arg;

  /* All test vectors include a solve, verify, and fail-verify phase
   * as well as a test of the nonce replay cache. The initial nonce for the
   * solution search is set via the solver's RNG data. The amount of solve
   * time during test execution can be tuned based on how far away from the
   * winning nonce our solve_rng value is set.
   */
  static const struct {
    uint32_t effort;
    const char *solve_rng_hex;
    const char *seed_hex;
    const char *service_blinded_id_hex;
    const char *nonce_hex;
    const char *sol_hex;
  } vectors[] = {
    {
      0, "55555555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "55555555555555555555555555555555", "4312f87ceab844c78e1c793a913812d7"
    },
    {
      1, "55555555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "55555555555555555555555555555555", "84355542ab2b3f79532ef055144ac5ab"
    },
    {
      1, "55555555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111110",
      "55555555555555555555555555555555", "115e4b70da858792fc205030b8c83af9"
    },
    {
      2, "55555555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "55555555555555555555555555555555", "4600a93a535ed76dc746c99942ab7de2"
    },
    {
      10, "55555555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "56555555555555555555555555555555", "128bbda5df2929c3be086de2aad34aed"
    },
    {
      10, "ffffffffffffffffffffffffffffffff",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "01000000000000000000000000000000", "203af985537fadb23f3ed5873b4c81ce"
    },
    {
      1337, "7fffffffffffffffffffffffffffffff",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "4111111111111111111111111111111111111111111111111111111111111111",
      "01000000000000000000000000000000", "31c377cb72796ed80ae77df6ac1d6bfd"
    },
    {
      31337, "34a20000000000000000000000000000",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "36a20000000000000000000000000000", "ca6899b91113aaf7536f28db42526bff"
    },
    {
      100, "55555555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "56555555555555555555555555555555", "3a4122a240bd7abfc922ab3cbb9479ed"
    },
    {
      1000, "d3555555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "d4555555555555555555555555555555", "338cc08f57697ce8ac2e4b453057d6e9"
    },
    {
      10000, "c5715555555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "c8715555555555555555555555555555", "9f2d3d4ed831ac96ad34c25fb59ff3e2"
    },
    {
      100000, "418d5655555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "428d5655555555555555555555555555", "9863f3acd2d15adfd244a7ca61d4c6ff"
    },
    {
      1000000, "58217255555555555555555555555555",
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      "1111111111111111111111111111111111111111111111111111111111111111",
      "59217255555555555555555555555555", "0f3db97b9cac20c1771680a1a34848d3"
    },
    {
      1, "d0aec1669384bfe5ed39cd724d6c7954",
      "c52be1f8a5e6cc3b8fb71cfdbe272cbc91d4d035400f2f94fb0d0074794e0a07",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "d1aec1669384bfe5ed39cd724d6c7954", "462606e5f8c2f3f844127b8bfdd6b4ff"
    },
    {
      1, "b4d0e611e6935750fcf9406aae131f62",
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "b4d0e611e6935750fcf9406aae131f62", "9f3fbd50b1a83fb63284bde44318c0fd"
    },
    {
      1, "b4d0e611e6935750fcf9406aae131f62",
      "9dfbd06d86fed8e12de3ab214e1a63ea61f46253fe08346a20378da70c4a327d",
      "bec632eb76123956f99a06d394fcbee8f135b8ed01f2e90aabe404cb0346744a",
      "b4d0e611e6935750fcf9406aae131f62", "161baa7490356292d020065fdbe55ffc"
    },
    {
      1, "40559fdbc34326d9d2f18ed277469c63",
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "40559fdbc34326d9d2f18ed277469c63", "fa649c6a2c5c0bb6a3511b9ea4b448d1"
    },
    {
      10000, "34569fdbc34326d9d2f18ed277469c63",
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "36569fdbc34326d9d2f18ed277469c63", "2802951e623c74adc443ab93e99633ee"
    },
    {
      100000, "2cff9fdbc34326d9d2f18ed277469c63",
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "2eff9fdbc34326d9d2f18ed277469c63", "400cb091139f86b352119f6e131802d6"
    },
    {
      1000000, "5243b3dbc34326d9d2f18ed277469c63",
      "86fb0acf4932cda44dbb451282f415479462dd10cb97ff5e7e8e2a53c3767a7f",
      "bfd298428562e530c52bdb36d81a0e293ef4a0e94d787f0f8c0c611f4f9e78ed",
      "5543b3dbc34326d9d2f18ed277469c63", "b47c718b56315e9697173a6bac1feaa4"
    },
  };

  const unsigned num_vectors = sizeof vectors / sizeof vectors[0];
  for (unsigned vec_i = 0; vec_i < num_vectors; vec_i++) {
    const char *seed_hex = vectors[vec_i].seed_hex;
    const char *service_blinded_id_hex = vectors[vec_i].service_blinded_id_hex;
    const char *solve_rng_hex = vectors[vec_i].solve_rng_hex;
    const char *nonce_hex = vectors[vec_i].nonce_hex;
    const char *sol_hex = vectors[vec_i].sol_hex;

    uint8_t rng_bytes[HS_POW_NONCE_LEN];
    hs_pow_solution_t output;
    hs_pow_solution_t solution = { 0 };
    hs_pow_solver_inputs_t input = {
      .effort = vectors[vec_i].effort,
      .CompiledProofOfWorkHash = -1
    };

    tt_int_op(strlen(service_blinded_id_hex), OP_EQ, 2 * HS_POW_ID_LEN);
    tt_int_op(strlen(seed_hex), OP_EQ, 2 * sizeof input.seed);
    tt_int_op(strlen(solve_rng_hex), OP_EQ, 2 * sizeof rng_bytes);
    tt_int_op(strlen(nonce_hex), OP_EQ, 2 * sizeof solution.nonce);
    tt_int_op(strlen(sol_hex), OP_EQ, 2 * sizeof solution.equix_solution);

    tt_int_op(base16_decode((char*)input.service_blinded_id.pubkey,
                            HS_POW_ID_LEN, service_blinded_id_hex,
                            2 * HS_POW_ID_LEN),
                            OP_EQ, HS_POW_ID_LEN);
    tt_int_op(base16_decode((char*)input.seed, HS_POW_SEED_LEN,
                            seed_hex, 2 * HS_POW_SEED_LEN),
                            OP_EQ, HS_POW_SEED_LEN);
    tt_int_op(base16_decode((char*)rng_bytes, sizeof rng_bytes,
                            solve_rng_hex, 2 * sizeof rng_bytes),
                            OP_EQ, HS_POW_NONCE_LEN);
    tt_int_op(base16_decode((char*)&solution.nonce, sizeof solution.nonce,
                            nonce_hex, 2 * sizeof solution.nonce),
                            OP_EQ, HS_POW_NONCE_LEN);
    tt_int_op(base16_decode((char*)&solution.equix_solution,
                            sizeof solution.equix_solution,
                            sol_hex, 2 * sizeof solution.equix_solution),
                            OP_EQ, HS_POW_EQX_SOL_LEN);
    memcpy(solution.seed_head, input.seed, HS_POW_SEED_HEAD_LEN);

    memset(&output, 0xaa, sizeof output);
    testing_enable_prefilled_rng(rng_bytes, HS_POW_NONCE_LEN);
    tt_int_op(0, OP_EQ, hs_pow_solve(&input, &output));
    testing_disable_prefilled_rng();

    tt_mem_op(solution.seed_head, OP_EQ, output.seed_head,
              sizeof output.seed_head);
    tt_mem_op(solution.nonce, OP_EQ, output.nonce,
              sizeof output.nonce);
    tt_mem_op(&solution.equix_solution, OP_EQ, &output.equix_solution,
              sizeof output.equix_solution);

    tt_int_op(testing_one_hs_pow_solution(&output, &input.service_blinded_id,
                                          input.seed), OP_EQ, 0);
  }

 done:
  testing_disable_prefilled_rng();
  hs_pow_remove_seed_from_cache(NULL);
}

struct testcase_t slow_hs_pow_tests[] = {
  { "vectors", test_hs_pow_vectors, 0, NULL, NULL },
  END_OF_TESTCASES
};
