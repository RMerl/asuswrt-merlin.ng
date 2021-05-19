/*
 * Copyright (c) 2019-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypt_path.c
 *
 * \brief Functions dealing with layered circuit encryption. This file aims to
 *   provide an API around the crypt_path_t structure which holds crypto
 *   information about a specific hop of a circuit.
 *
 * TODO: We should eventually move all functions dealing and manipulating
 *   crypt_path_t to this file, so that eventually we encapsulate more and more
 *   of crypt_path_t. Here are some more functions that can be moved here with
 *   some more effort:
 *
 *   - circuit_list_path_impl()
 *   - Functions dealing with cpaths in HSv2 create_rend_cpath() and
 *     create_rend_cpath_legacy()
 *   - The cpath related parts of rend_service_receive_introduction() and
 *     rend_client_send_introduction().
 **/

#define CRYPT_PATH_PRIVATE

#include "core/or/or.h"
#include "core/or/crypt_path.h"

#include "core/crypto/relay_crypto.h"
#include "core/crypto/onion_crypto.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/extendinfo.h"

#include "lib/crypt_ops/crypto_dh.h"
#include "lib/crypt_ops/crypto_util.h"

#include "core/or/crypt_path_st.h"
#include "core/or/cell_st.h"

/** Add <b>new_hop</b> to the end of the doubly-linked-list <b>head_ptr</b>.
 * This function is used to extend cpath by another hop.
 */
void
cpath_extend_linked_list(crypt_path_t **head_ptr, crypt_path_t *new_hop)
{
  if (*head_ptr) {
    new_hop->next = (*head_ptr);
    new_hop->prev = (*head_ptr)->prev;
    (*head_ptr)->prev->next = new_hop;
    (*head_ptr)->prev = new_hop;
  } else {
    *head_ptr = new_hop;
    new_hop->prev = new_hop->next = new_hop;
  }
}

/** Create a new hop, annotate it with information about its
 * corresponding router <b>choice</b>, and append it to the
 * end of the cpath <b>head_ptr</b>. */
int
cpath_append_hop(crypt_path_t **head_ptr, extend_info_t *choice)
{
  crypt_path_t *hop = tor_malloc_zero(sizeof(crypt_path_t));

  /* link hop into the cpath, at the end. */
  cpath_extend_linked_list(head_ptr, hop);

  hop->magic = CRYPT_PATH_MAGIC;
  hop->state = CPATH_STATE_CLOSED;

  hop->extend_info = extend_info_dup(choice);

  hop->package_window = circuit_initial_package_window();
  hop->deliver_window = CIRCWINDOW_START;

  return 0;
}

/** Verify that cpath <b>cp</b> has all of its invariants
 * correct. Trigger an assert if anything is invalid.
 */
void
cpath_assert_ok(const crypt_path_t *cp)
{
  const crypt_path_t *start = cp;

  do {
    cpath_assert_layer_ok(cp);
    /* layers must be in sequence of: "open* awaiting? closed*" */
    if (cp != start) {
      if (cp->state == CPATH_STATE_AWAITING_KEYS) {
        tor_assert(cp->prev->state == CPATH_STATE_OPEN);
      } else if (cp->state == CPATH_STATE_OPEN) {
        tor_assert(cp->prev->state == CPATH_STATE_OPEN);
      }
    }
    cp = cp->next;
    tor_assert(cp);
  } while (cp != start);
}

/** Verify that cpath layer <b>cp</b> has all of its invariants
 * correct. Trigger an assert if anything is invalid.
 */
void
cpath_assert_layer_ok(const crypt_path_t *cp)
{
//  tor_assert(cp->addr); /* these are zero for rendezvous extra-hops */
//  tor_assert(cp->port);
  tor_assert(cp);
  tor_assert(cp->magic == CRYPT_PATH_MAGIC);
  switch (cp->state)
    {
    case CPATH_STATE_OPEN:
      relay_crypto_assert_ok(&cp->pvt_crypto);
      FALLTHROUGH;
    case CPATH_STATE_CLOSED:
      /*XXXX Assert that there's no handshake_state either. */
      tor_assert(!cp->rend_dh_handshake_state);
      break;
    case CPATH_STATE_AWAITING_KEYS:
      /* tor_assert(cp->dh_handshake_state); */
      break;
    default:
      log_fn(LOG_ERR, LD_BUG, "Unexpected state %d", cp->state);
      tor_assert(0);
    }
  tor_assert(cp->package_window >= 0);
  tor_assert(cp->deliver_window >= 0);
}

/** Initialize cpath-\>{f|b}_{crypto|digest} from the key material in key_data.
 *
 * If <b>is_hs_v3</b> is set, this cpath will be used for next gen hidden
 * service circuits and <b>key_data</b> must be at least
 * HS_NTOR_KEY_EXPANSION_KDF_OUT_LEN bytes in length.
 *
 * If <b>is_hs_v3</b> is not set, key_data must contain CPATH_KEY_MATERIAL_LEN
 * bytes, which are used as follows:
 *   - 20 to initialize f_digest
 *   - 20 to initialize b_digest
 *   - 16 to key f_crypto
 *   - 16 to key b_crypto
 *
 * (If 'reverse' is true, then f_XX and b_XX are swapped.)
 *
 * Return 0 if init was successful, else -1 if it failed.
 */
int
cpath_init_circuit_crypto(crypt_path_t *cpath,
                          const char *key_data, size_t key_data_len,
                          int reverse, int is_hs_v3)
{

  tor_assert(cpath);
  return relay_crypto_init(&cpath->pvt_crypto, key_data, key_data_len,
                           reverse, is_hs_v3);
}

/** Deallocate space associated with the cpath node <b>victim</b>. */
void
cpath_free(crypt_path_t *victim)
{
  if (!victim)
    return;

  relay_crypto_clear(&victim->pvt_crypto);
  onion_handshake_state_release(&victim->handshake_state);
  crypto_dh_free(victim->rend_dh_handshake_state);
  extend_info_free(victim->extend_info);

  memwipe(victim, 0xBB, sizeof(crypt_path_t)); /* poison memory */
  tor_free(victim);
}

/********************** cpath crypto API *******************************/

/** Encrypt or decrypt <b>payload</b> using the crypto of <b>cpath</b>. Actual
 *  operation decided by <b>is_decrypt</b>.  */
void
cpath_crypt_cell(const crypt_path_t *cpath, uint8_t *payload, bool is_decrypt)
{
  if (is_decrypt) {
    relay_crypt_one_payload(cpath->pvt_crypto.b_crypto, payload);
  } else {
    relay_crypt_one_payload(cpath->pvt_crypto.f_crypto, payload);
  }
}

/** Getter for the incoming digest of <b>cpath</b>. */
struct crypto_digest_t *
cpath_get_incoming_digest(const crypt_path_t *cpath)
{
  return cpath->pvt_crypto.b_digest;
}

/** Set the right integrity digest on the outgoing <b>cell</b> based on the
 *  cell payload and update the forward digest of <b>cpath</b>. */
void
cpath_set_cell_forward_digest(crypt_path_t *cpath, cell_t *cell)
{
  relay_set_digest(cpath->pvt_crypto.f_digest, cell);
}

/************ cpath sendme API ***************************/

/** Return the sendme_digest of this <b>cpath</b>. */
uint8_t *
cpath_get_sendme_digest(crypt_path_t *cpath)
{
  return relay_crypto_get_sendme_digest(&cpath->pvt_crypto);
}

/** Record the cell digest, indicated by is_foward_digest or not, as the
 * SENDME cell digest. */
void
cpath_sendme_record_cell_digest(crypt_path_t *cpath, bool is_foward_digest)
{
  tor_assert(cpath);
  relay_crypto_record_sendme_digest(&cpath->pvt_crypto, is_foward_digest);
}

/************ other cpath functions ***************************/

/** Return the first non-open hop in cpath, or return NULL if all
 * hops are open. */
crypt_path_t *
cpath_get_next_non_open_hop(crypt_path_t *cpath)
{
  crypt_path_t *hop = cpath;
  do {
    if (hop->state != CPATH_STATE_OPEN)
      return hop;
    hop = hop->next;
  } while (hop != cpath);
  return NULL;
}

#ifdef TOR_UNIT_TESTS

/** Unittest helper function: Count number of hops in cpath linked list. */
unsigned int
cpath_get_n_hops(crypt_path_t **head_ptr)
{
  unsigned int n_hops = 0;
  crypt_path_t *tmp;

  if (!*head_ptr) {
    return 0;
  }

  tmp = *head_ptr;
  do {
    n_hops++;
    tmp = tmp->next;
  } while (tmp != *head_ptr);

  return n_hops;
}

#endif /* defined(TOR_UNIT_TESTS) */
