/* Copyright (c) 2017-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_pow.c
 * \brief Contains code to handle proof-of-work computations
 * when a hidden service is defending against DoS attacks.
 **/

#include <stdio.h>

#include "core/or/or.h"
#include "app/config/config.h"
#include "ext/ht.h"
#include "ext/compat_blake2.h"
#include "core/or/circuitlist.h"
#include "core/or/origin_circuit_st.h"
#include "ext/equix/include/equix.h"
#include "feature/hs/hs_cache.h"
#include "feature/hs/hs_descriptor.h"
#include "feature/hs/hs_circuitmap.h"
#include "feature/hs/hs_client.h"
#include "feature/hs/hs_pow.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_format.h"
#include "lib/arch/bytes.h"
#include "lib/cc/ctassert.h"
#include "core/mainloop/cpuworker.h"
#include "lib/evloop/workqueue.h"
#include "lib/time/compat_time.h"

/** Replay cache set up */
/** Cache entry for (nonce, seed) replay protection. */
typedef struct nonce_cache_entry_t {
  HT_ENTRY(nonce_cache_entry_t) node;
  struct {
    uint8_t nonce[HS_POW_NONCE_LEN];
    uint8_t seed_head[HS_POW_SEED_HEAD_LEN];
  } bytes;
} nonce_cache_entry_t;

/** Return true if the two (nonce, seed) replay cache entries are the same */
static inline int
nonce_cache_entries_eq_(const struct nonce_cache_entry_t *entry1,
                        const struct nonce_cache_entry_t *entry2)
{
  return fast_memeq(&entry1->bytes, &entry2->bytes, sizeof entry1->bytes);
}

/** Hash function to hash the (nonce, seed) tuple entry. */
static inline unsigned
nonce_cache_entry_hash_(const struct nonce_cache_entry_t *ent)
{
  return (unsigned)siphash24g(&ent->bytes, sizeof ent->bytes);
}

static HT_HEAD(nonce_cache_table_ht, nonce_cache_entry_t)
  nonce_cache_table = HT_INITIALIZER();

HT_PROTOTYPE(nonce_cache_table_ht, nonce_cache_entry_t, node,
             nonce_cache_entry_hash_, nonce_cache_entries_eq_);

HT_GENERATE2(nonce_cache_table_ht, nonce_cache_entry_t, node,
             nonce_cache_entry_hash_, nonce_cache_entries_eq_, 0.6,
             tor_reallocarray_, tor_free_);

/** This is a callback used to check replay cache entries against a provided
 * seed head, or NULL to operate on the entire cache. Matching entries return
 * 1 and their internal cache entry is freed, non-matching entries return 0. */
static int
nonce_cache_entry_match_seed_and_free(nonce_cache_entry_t *ent, void *data)
{
  if (data == NULL ||
      fast_memeq(ent->bytes.seed_head, data, HS_POW_SEED_HEAD_LEN)) {
    tor_free(ent);
    return 1;
  }
  return 0;
}

/** Helper: Increment a given nonce and set it in the challenge at the right
 * offset. Use by the solve function. */
static inline void
increment_and_set_nonce(uint8_t *nonce, uint8_t *challenge)
{
  for (unsigned i = 0; i < HS_POW_NONCE_LEN; i++) {
    uint8_t prev = nonce[i];
    if (++nonce[i] > prev) {
      break;
    }
  }
  memcpy(challenge + HS_POW_NONCE_OFFSET, nonce, HS_POW_NONCE_LEN);
}

/* Helper: Build EquiX challenge (P || ID || C || N || INT_32(E)) and return
 * a newly allocated buffer containing it. */
static uint8_t *
build_equix_challenge(const ed25519_public_key_t *blinded_id,
                      const uint8_t *seed, const uint8_t *nonce,
                      const uint32_t effort)
{
  size_t offset = 0;
  uint8_t *challenge = tor_malloc_zero(HS_POW_CHALLENGE_LEN);

  CTASSERT(HS_POW_ID_LEN == sizeof *blinded_id);
  tor_assert_nonfatal(!ed25519_public_key_is_zero(blinded_id));

  log_debug(LD_REND,
            "Constructing EquiX challenge with "
            "blinded service id %s, effort: %d",
            safe_str_client(ed25519_fmt(blinded_id)),
            effort);

  memcpy(challenge + offset, HS_POW_PSTRING, HS_POW_PSTRING_LEN);
  offset += HS_POW_PSTRING_LEN;
  memcpy(challenge + offset, blinded_id, HS_POW_ID_LEN);
  offset += HS_POW_ID_LEN;
  memcpy(challenge + offset, seed, HS_POW_SEED_LEN);
  offset += HS_POW_SEED_LEN;
  tor_assert(HS_POW_NONCE_OFFSET == offset);
  memcpy(challenge + offset, nonce, HS_POW_NONCE_LEN);
  offset += HS_POW_NONCE_LEN;
  set_uint32(challenge + offset, tor_htonl(effort));
  offset += HS_POW_EFFORT_LEN;
  tor_assert(HS_POW_CHALLENGE_LEN == offset);

  return challenge;
}

/** Helper: Return true iff the given challenge and solution for the given
 * effort do validate as in: R * E <= UINT32_MAX. */
static bool
validate_equix_challenge(const uint8_t *challenge,
                         const uint8_t *solution_bytes,
                         const uint32_t effort)
{
  /* Fail if R * E > UINT32_MAX. */
  uint8_t hash_result[HS_POW_HASH_LEN];
  blake2b_state b2_state;

  if (BUG(blake2b_init(&b2_state, HS_POW_HASH_LEN) < 0)) {
    return false;
  }

  /* Construct: blake2b(C || N || E || S) */
  blake2b_update(&b2_state, challenge, HS_POW_CHALLENGE_LEN);
  blake2b_update(&b2_state, solution_bytes, HS_POW_EQX_SOL_LEN);
  blake2b_final(&b2_state, hash_result, HS_POW_HASH_LEN);

  /* Scale to 64 bit so we can avoid 32 bit overflow. */
  uint64_t RE = tor_htonl(get_uint32(hash_result)) * (uint64_t) effort;

  return RE <= UINT32_MAX;
}

/** Helper: Convert equix_solution to a byte array in little-endian order */
static void
pack_equix_solution(const equix_solution *sol_in,
                    uint8_t *bytes_out)
{
  for (unsigned i = 0; i < EQUIX_NUM_IDX; i++) {
    bytes_out[i*2+0] = (uint8_t)sol_in->idx[i];
    bytes_out[i*2+1] = (uint8_t)(sol_in->idx[i] >> 8);
  }
}

/** Helper: Build an equix_solution from its corresponding byte array. */
static void
unpack_equix_solution(const uint8_t *bytes_in,
                      equix_solution *sol_out)
{
  for (unsigned i = 0; i < EQUIX_NUM_IDX; i++) {
    sol_out->idx[i] = (uint16_t)bytes_in[i*2+0] |
                      (uint16_t)bytes_in[i*2+1] << 8;
  }
}

/** Helper: Map the CompiledProofOfWorkHash configuration option to its
 * corresponding equix_ctx_flags bit. */
static equix_ctx_flags
hs_pow_equix_option_flags(int CompiledProofOfWorkHash)
{
  if (CompiledProofOfWorkHash == 0) {
    return 0;
  } else if (CompiledProofOfWorkHash == 1) {
    return EQUIX_CTX_MUST_COMPILE;
  } else {
    tor_assert_nonfatal(CompiledProofOfWorkHash == -1);
    return EQUIX_CTX_TRY_COMPILE;
  }
}

/** Solve the EquiX/blake2b PoW scheme using the parameters in pow_params, and
 * store the solution in pow_solution_out. Returns 0 on success and -1
 * otherwise. Called by a client, from a cpuworker thread. */
int
hs_pow_solve(const hs_pow_solver_inputs_t *pow_inputs,
             hs_pow_solution_t *pow_solution_out)
{
  int ret = -1;
  uint8_t nonce[HS_POW_NONCE_LEN];
  uint8_t *challenge = NULL;
  equix_ctx *ctx = NULL;

  tor_assert(pow_inputs);
  tor_assert(pow_solution_out);
  const uint32_t effort = pow_inputs->effort;

  /* Generate a random nonce N. */
  crypto_rand((char *)nonce, sizeof nonce);

  /* Build EquiX challenge string */
  challenge = build_equix_challenge(&pow_inputs->service_blinded_id,
                                    pow_inputs->seed, nonce, effort);

  /* This runs on a cpuworker, let's not access global get_options().
   * Instead, the particular options we need are captured in pow_inputs. */
  ctx = equix_alloc(EQUIX_CTX_SOLVE |
    hs_pow_equix_option_flags(pow_inputs->CompiledProofOfWorkHash));
  if (!ctx) {
    goto end;
  }

  uint8_t sol_bytes[HS_POW_EQX_SOL_LEN];
  monotime_t start_time;
  monotime_get(&start_time);
  log_info(LD_REND, "Solving proof of work (effort %u)", effort);

  for (;;) {
    /* Calculate solutions to S = equix_solve(C || N || E),  */
    equix_solutions_buffer buffer;
    equix_result result;
    result = equix_solve(ctx, challenge, HS_POW_CHALLENGE_LEN, &buffer);
    switch (result) {

      case EQUIX_OK:
        for (unsigned i = 0; i < buffer.count; i++) {
          pack_equix_solution(&buffer.sols[i], sol_bytes);

          /* Check an Equi-X solution against the effort threshold */
          if (validate_equix_challenge(challenge, sol_bytes, effort)) {
            /* Store the nonce N. */
            memcpy(pow_solution_out->nonce, nonce, HS_POW_NONCE_LEN);
            /* Store the effort E. */
            pow_solution_out->effort = effort;
            /* We only store the first 4 bytes of the seed C. */
            memcpy(pow_solution_out->seed_head, pow_inputs->seed,
               sizeof(pow_solution_out->seed_head));
            /* Store the solution S */
            memcpy(&pow_solution_out->equix_solution,
                   sol_bytes, sizeof sol_bytes);

            monotime_t end_time;
            monotime_get(&end_time);
            int64_t duration_usec = monotime_diff_usec(&start_time, &end_time);
            log_info(LD_REND, "Proof of work solution (effort %u) found "
                     "using %s implementation in %u.%06u seconds",
                     effort,
                    (EQUIX_SOLVER_DID_USE_COMPILER & buffer.flags)
                       ? "compiled" : "interpreted",
                    (unsigned)(duration_usec / 1000000),
                    (unsigned)(duration_usec % 1000000));

            /* Indicate success and we are done. */
            ret = 0;
            goto end;
          }
        }
        break;

      case EQUIX_FAIL_CHALLENGE:
        /* This happens occasionally due to HashX rejecting some program
         * configurations. For our purposes here it's the same as count==0.
         * Increment the nonce and try again. */
        break;

      case EQUIX_FAIL_COMPILE:
        /* The interpreter is disabled and the compiler failed */
        log_warn(LD_REND, "Proof of work solver failed, "
                 "compile error with no fallback enabled.");
        goto end;

      /* These failures are not applicable to equix_solve, but included for
       * completeness and to satisfy exhaustive enum warnings. */
      case EQUIX_FAIL_ORDER:
      case EQUIX_FAIL_PARTIAL_SUM:
      case EQUIX_FAIL_FINAL_SUM:
      /* And these really should not happen, and indicate
       * programming errors if they do. */
      case EQUIX_FAIL_NO_SOLVER:
      case EQUIX_FAIL_INTERNAL:
      default:
        tor_assert_nonfatal_unreached();
        goto end;
    }

    /* No solutions for this nonce and/or none that passed the effort
     * threshold, increment and try again. */
    increment_and_set_nonce(nonce, challenge);
  }

 end:
  tor_free(challenge);
  equix_free(ctx);
  return ret;
}

/** Verify the solution in pow_solution using the service's current PoW
 * parameters found in pow_state. Returns 0 on success and -1 otherwise. Called
 * by the service. */
int
hs_pow_verify(const ed25519_public_key_t *service_blinded_id,
              const hs_pow_service_state_t *pow_state,
              const hs_pow_solution_t *pow_solution)
{
  int ret = -1;
  uint8_t *challenge = NULL;
  nonce_cache_entry_t search, *entry = NULL;
  equix_ctx *ctx = NULL;
  const uint8_t *seed = NULL;

  tor_assert(pow_state);
  tor_assert(pow_solution);
  tor_assert(service_blinded_id);
  tor_assert_nonfatal(!ed25519_public_key_is_zero(service_blinded_id));

  /* Find a valid seed C that starts with the seed head. Fail if no such seed
   * exists. */
  if (fast_memeq(pow_state->seed_current, pow_solution->seed_head,
                 HS_POW_SEED_HEAD_LEN)) {
    seed = pow_state->seed_current;
  } else if (fast_memeq(pow_state->seed_previous, pow_solution->seed_head,
                        HS_POW_SEED_HEAD_LEN)) {
    seed = pow_state->seed_previous;
  } else {
    log_warn(LD_REND, "Seed head didn't match either seed.");
    goto done;
  }

  /* Fail if N = POW_NONCE is present in the replay cache. */
  memcpy(search.bytes.nonce, pow_solution->nonce, HS_POW_NONCE_LEN);
  memcpy(search.bytes.seed_head, pow_solution->seed_head,
         HS_POW_SEED_HEAD_LEN);
  entry = HT_FIND(nonce_cache_table_ht, &nonce_cache_table, &search);
  if (entry) {
    log_warn(LD_REND, "Found (nonce, seed) tuple in the replay cache.");
    goto done;
  }

  /* Build the challenge with the params we have. */
  challenge = build_equix_challenge(service_blinded_id, seed,
                                    pow_solution->nonce, pow_solution->effort);

  if (!validate_equix_challenge(challenge, pow_solution->equix_solution,
                                pow_solution->effort)) {
    log_warn(LD_REND, "Verification of challenge effort in PoW failed.");
    goto done;
  }

  ctx = equix_alloc(EQUIX_CTX_VERIFY |
    hs_pow_equix_option_flags(get_options()->CompiledProofOfWorkHash));
  if (!ctx) {
    goto done;
  }

  /* Fail if equix_verify() != EQUIX_OK */
  equix_solution equix_sol;
  unpack_equix_solution(pow_solution->equix_solution, &equix_sol);
  equix_result result = equix_verify(ctx, challenge, HS_POW_CHALLENGE_LEN,
                                     &equix_sol);
  if (result != EQUIX_OK) {
    log_warn(LD_REND, "Verification of EquiX solution in PoW failed.");
    goto done;
  }

  /* PoW verified successfully. */
  ret = 0;

  /* Add the (nonce, seed) tuple to the replay cache. */
  entry = tor_malloc_zero(sizeof(nonce_cache_entry_t));
  memcpy(entry->bytes.nonce, pow_solution->nonce, HS_POW_NONCE_LEN);
  memcpy(entry->bytes.seed_head, pow_solution->seed_head,
         HS_POW_SEED_HEAD_LEN);
  HT_INSERT(nonce_cache_table_ht, &nonce_cache_table, entry);

 done:
  tor_free(challenge);
  equix_free(ctx);
  return ret;
}

/** Remove entries from the (nonce, seed) replay cache which are for the seed
 * beginning with seed_head. If seed_head is NULL, remove all cache entries. */
void
hs_pow_remove_seed_from_cache(const uint8_t *seed_head)
{
  /* If nonce_cache_entry_has_seed returns 1, the entry is removed. */
  HT_FOREACH_FN(nonce_cache_table_ht, &nonce_cache_table,
                nonce_cache_entry_match_seed_and_free, (void*)seed_head);
}

/** Free a given PoW service state. */
void
hs_pow_free_service_state(hs_pow_service_state_t *state)
{
  if (state == NULL) {
    return;
  }
  rend_pqueue_clear(state);
  tor_assert(smartlist_len(state->rend_request_pqueue) == 0);
  smartlist_free(state->rend_request_pqueue);
  mainloop_event_free(state->pop_pqueue_ev);
  tor_free(state);
}

/* =====
   Thread workers
   =====*/

/**
 * An object passed to a worker thread that will try to solve the pow.
 */
typedef struct pow_worker_job_t {

  /** Inputs for the PoW solver (seed, chosen effort) */
  hs_pow_solver_inputs_t pow_inputs;

  /** State: we'll look these up to figure out how to proceed after. */
  uint32_t intro_circ_identifier;
  uint8_t rend_circ_cookie[HS_REND_COOKIE_LEN];

  /** Output: The worker thread will malloc and write its answer here,
   * or set it to NULL if it produced no useful answer. */
  hs_pow_solution_t *pow_solution_out;

} pow_worker_job_t;

/**
 * Worker function. This function runs inside a worker thread and receives
 * a pow_worker_job_t as its input.
 */
static workqueue_reply_t
pow_worker_threadfn(void *state_, void *work_)
{
  (void)state_;
  pow_worker_job_t *job = work_;
  job->pow_solution_out = tor_malloc_zero(sizeof(hs_pow_solution_t));

  if (hs_pow_solve(&job->pow_inputs, job->pow_solution_out)) {
    tor_free(job->pow_solution_out);
    job->pow_solution_out = NULL; /* how we signal that we came up empty */
  }
  return WQ_RPL_REPLY;
}

/**
 * Helper: release all storage held in <b>job</b>.
 */
static void
pow_worker_job_free(pow_worker_job_t *job)
{
  if (!job)
    return;
  tor_free(job->pow_solution_out);
  tor_free(job);
}

/**
 * Worker function: This function runs in the main thread, and receives
 * a pow_worker_job_t that the worker thread has already processed.
 */
static void
pow_worker_replyfn(void *work_)
{
  tor_assert(in_main_thread());
  tor_assert(work_);

  pow_worker_job_t *job = work_;

  /* Look up the circuits that we're going to use this pow in.
   * There's room for improvement here. We already had a fast mapping to
   * rend circuits from some kind of identifier that we can keep in a
   * pow_worker_job_t, but we don't have that index for intro circs at this
   * time. If the linear search in circuit_get_by_global_id() is ever a
   * noticeable bottleneck we should add another map.
   */
  origin_circuit_t *intro_circ =
    circuit_get_by_global_id(job->intro_circ_identifier);
  origin_circuit_t *rend_circ =
    hs_circuitmap_get_established_rend_circ_client_side(job->rend_circ_cookie);

  /* try to re-create desc and ip */
  const ed25519_public_key_t *service_identity_pk = NULL;
  const hs_descriptor_t *desc = NULL;
  const hs_desc_intro_point_t *ip = NULL;
  if (intro_circ)
    service_identity_pk = &intro_circ->hs_ident->identity_pk;
  if (service_identity_pk)
    desc = hs_cache_lookup_as_client(service_identity_pk);
  if (desc)
    ip = find_desc_intro_point_by_ident(intro_circ->hs_ident, desc);

  if (intro_circ && rend_circ && service_identity_pk && desc && ip &&
      job->pow_solution_out) {

    /* successful pow solve, and circs still here */
    log_info(LD_REND, "Got a PoW solution we like! Shipping it!");

    /* Set flag to reflect that the HS we are attempting to rendezvous has PoW
     * defenses enabled, and as such we will need to be more lenient with
     * timing out while waiting for the service-side circuit to be built. */
    rend_circ->hs_with_pow_circ = 1;

    /* Remember the PoW effort we chose, for client-side rend circuits. */
    rend_circ->hs_pow_effort = job->pow_inputs.effort;

    // and then send that intro cell
    if (send_introduce1(intro_circ, rend_circ,
                        desc, job->pow_solution_out, ip) < 0) {
      /* if it failed, mark the intro point as ready to start over */
      intro_circ->hs_currently_solving_pow = 0;
    }

  } else {
    if (!job->pow_solution_out) {
      log_warn(LD_REND, "PoW cpuworker returned with no solution");
    } else {
      log_info(LD_REND, "PoW solution completed but we can "
                        "no longer locate its circuit");
    }
    if (intro_circ) {
      intro_circ->hs_currently_solving_pow = 0;
    }
  }

  pow_worker_job_free(job);
}

/**
 * Queue the job of solving the pow in a worker thread.
 */
int
hs_pow_queue_work(uint32_t intro_circ_identifier,
                  const uint8_t *rend_circ_cookie,
                  const hs_pow_solver_inputs_t *pow_inputs)
{
  tor_assert(in_main_thread());
  tor_assert(rend_circ_cookie);
  tor_assert(pow_inputs);
  tor_assert_nonfatal(
    !ed25519_public_key_is_zero(&pow_inputs->service_blinded_id));

  pow_worker_job_t *job = tor_malloc_zero(sizeof(*job));
  job->intro_circ_identifier = intro_circ_identifier;
  memcpy(&job->rend_circ_cookie, rend_circ_cookie,
         sizeof job->rend_circ_cookie);
  memcpy(&job->pow_inputs, pow_inputs, sizeof job->pow_inputs);

  workqueue_entry_t *work;
  work = cpuworker_queue_work(WQ_PRI_LOW,
                              pow_worker_threadfn,
                              pow_worker_replyfn,
                              job);
  if (!work) {
    pow_worker_job_free(job);
    return -1;
  }
  return 0;
}
