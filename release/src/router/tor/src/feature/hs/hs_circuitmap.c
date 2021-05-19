/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_circuitmap.c
 *
 * \brief Hidden service circuitmap: A hash table that maps binary tokens to
 *  introduction and rendezvous circuits; it's used:
 *  (a) by relays acting as intro points and rendezvous points
 *  (b) by hidden services to find intro and rend circuits and
 *  (c) by HS clients to find rendezvous circuits.
 **/

#define HS_CIRCUITMAP_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "core/or/circuitlist.h"
#include "feature/hs/hs_circuitmap.h"

#include "core/or/or_circuit_st.h"
#include "core/or/origin_circuit_st.h"

/************************** HS circuitmap code *******************************/

/** This is the hidden service circuitmap. It's a hash table that maps
   introduction and rendezvous tokens to specific circuits such that given a
   token it's easy to find the corresponding circuit. */
static struct hs_circuitmap_ht *the_hs_circuitmap = NULL;

/** This is a helper function used by the hash table code (HT_). It returns 1
 * if two circuits have the same HS token. */
static int
hs_circuits_have_same_token(const circuit_t *first_circuit,
                            const circuit_t *second_circuit)
{
  const hs_token_t *first_token;
  const hs_token_t *second_token;

  tor_assert(first_circuit);
  tor_assert(second_circuit);

  first_token = first_circuit->hs_token;
  second_token = second_circuit->hs_token;

  /* Both circs must have a token */
  if (BUG(!first_token) || BUG(!second_token)) {
    return 0;
  }

  if (first_token->type != second_token->type) {
    return 0;
  }

  if (first_token->token_len != second_token->token_len)
    return 0;

  return tor_memeq(first_token->token,
                   second_token->token,
                   first_token->token_len);
}

/** This is a helper function for the hash table code (HT_). It hashes a
 * circuit HS token into an unsigned int for use as a key by the hash table
 * routines.*/
static inline unsigned int
hs_circuit_hash_token(const circuit_t *circuit)
{
  tor_assert(circuit->hs_token);

  return (unsigned) siphash24g(circuit->hs_token->token,
                               circuit->hs_token->token_len);
}

/** Register the circuitmap hash table */
HT_PROTOTYPE(hs_circuitmap_ht, // The name of the hashtable struct
             circuit_t,    // The name of the element struct,
             hs_circuitmap_node,        // The name of HT_ENTRY member
             hs_circuit_hash_token, hs_circuits_have_same_token);

HT_GENERATE2(hs_circuitmap_ht, circuit_t, hs_circuitmap_node,
             hs_circuit_hash_token, hs_circuits_have_same_token,
             0.6, tor_reallocarray, tor_free_);

#ifdef TOR_UNIT_TESTS

/** Return the global HS circuitmap. Used by unittests. */
hs_circuitmap_ht *
get_hs_circuitmap(void)
{
  return the_hs_circuitmap;
}

#endif /* defined(TOR_UNIT_TESTS) */

/****************** HS circuitmap utility functions **************************/

/** Return a new HS token of type <b>type</b> containing <b>token</b>. */
static hs_token_t *
hs_token_new(hs_token_type_t type, size_t token_len,
             const uint8_t *token)
{
  tor_assert(token);

  hs_token_t *hs_token = tor_malloc_zero(sizeof(hs_token_t));
  hs_token->type = type;
  hs_token->token_len = token_len;
  hs_token->token = tor_memdup(token, token_len);

  return hs_token;
}

#define hs_token_free(val) \
  FREE_AND_NULL(hs_token_t, hs_token_free_, (val))

/** Free memory allocated by this <b>hs_token</b>. */
static void
hs_token_free_(hs_token_t *hs_token)
{
  if (!hs_token) {
    return;
  }

  tor_free(hs_token->token);
  tor_free(hs_token);
}

/** Return the circuit from the circuitmap with token <b>search_token</b>. */
static circuit_t *
get_circuit_with_token(hs_token_t *search_token)
{
  tor_assert(the_hs_circuitmap);

  /* We use a dummy circuit object for the hash table search routine. */
  circuit_t search_circ;
  search_circ.hs_token = search_token;
  return HT_FIND(hs_circuitmap_ht, the_hs_circuitmap, &search_circ);
}

/** Helper function that registers <b>circ</b> with <b>token</b> on the HS
   circuitmap. This function steals reference of <b>token</b>. */
static void
hs_circuitmap_register_impl(circuit_t *circ, hs_token_t *token)
{
  tor_assert(circ);
  tor_assert(token);
  tor_assert(the_hs_circuitmap);

  /* If this circuit already has a token, clear it. */
  if (circ->hs_token) {
    hs_circuitmap_remove_circuit(circ);
  }

  /* Kill old circuits with the same token. We want new intro/rend circuits to
     take precedence over old ones, so that HSes and clients and reestablish
     killed circuits without changing the HS token. */
  {
    circuit_t *found_circ;
    found_circ = get_circuit_with_token(token);
    if (found_circ) {
      hs_circuitmap_remove_circuit(found_circ);
      if (!found_circ->marked_for_close) {
        circuit_mark_for_close(found_circ, END_CIRC_REASON_FINISHED);
      }
    }
  }

  /* Register circuit and token to circuitmap. */
  circ->hs_token = token;
  HT_INSERT(hs_circuitmap_ht, the_hs_circuitmap, circ);
}

/** Helper function: Register <b>circ</b> of <b>type</b> on the HS
 *  circuitmap. Use the HS <b>token</b> as the key to the hash table.  If
 *  <b>token</b> is not set, clear the circuit of any HS tokens. */
static void
hs_circuitmap_register_circuit(circuit_t *circ,
                               hs_token_type_t type, size_t token_len,
                               const uint8_t *token)
{
  hs_token_t *hs_token = NULL;

  /* Create a new token and register it to the circuitmap */
  tor_assert(token);
  hs_token = hs_token_new(type, token_len, token);
  tor_assert(hs_token);
  hs_circuitmap_register_impl(circ, hs_token);
}

/** Helper function for hs_circuitmap_get_origin_circuit() and
 * hs_circuitmap_get_or_circuit(). Because only circuit_t are indexed in the
 * circuitmap, this function returns object type so the specialized functions
 * using this helper can upcast it to the right type.
 *
 * Return NULL if not such circuit is found. */
static circuit_t *
hs_circuitmap_get_circuit_impl(hs_token_type_t type,
                               size_t token_len,
                               const uint8_t *token,
                               uint8_t wanted_circ_purpose)
{
  circuit_t *found_circ = NULL;

  tor_assert(the_hs_circuitmap);

  /* Check the circuitmap if we have a circuit with this token */
  {
    hs_token_t *search_hs_token = hs_token_new(type, token_len, token);
    tor_assert(search_hs_token);
    found_circ = get_circuit_with_token(search_hs_token);
    hs_token_free(search_hs_token);
  }

  /* Check that the circuit is useful to us */
  if (!found_circ ||
      found_circ->purpose != wanted_circ_purpose ||
      found_circ->marked_for_close) {
    return NULL;
  }

  return found_circ;
}

/** Helper function: Query circuitmap for origin circuit with <b>token</b> of
 * size <b>token_len</b> and <b>type</b>.  Only returns a circuit with purpose
 * equal to the <b>wanted_circ_purpose</b> parameter and if it is NOT marked
 * for close. Return NULL if no such circuit is found. */
static origin_circuit_t *
hs_circuitmap_get_origin_circuit(hs_token_type_t type,
                                 size_t token_len,
                                 const uint8_t *token,
                                 uint8_t wanted_circ_purpose)
{
  circuit_t *circ;
  tor_assert(token);
  tor_assert(CIRCUIT_PURPOSE_IS_ORIGIN(wanted_circ_purpose));

  circ = hs_circuitmap_get_circuit_impl(type, token_len, token,
                                        wanted_circ_purpose);
  if (!circ) {
    return NULL;
  }

  tor_assert(CIRCUIT_IS_ORIGIN(circ));
  return TO_ORIGIN_CIRCUIT(circ);
}

/** Helper function: Query circuitmap for OR circuit with <b>token</b> of size
 * <b>token_len</b> and <b>type</b>.  Only returns a circuit with purpose equal
 * to the <b>wanted_circ_purpose</b> parameter and if it is NOT marked for
 * close. Return NULL if no such circuit is found. */
static or_circuit_t *
hs_circuitmap_get_or_circuit(hs_token_type_t type,
                             size_t token_len,
                             const uint8_t *token,
                             uint8_t wanted_circ_purpose)
{
  circuit_t *circ;
  tor_assert(token);
  tor_assert(!CIRCUIT_PURPOSE_IS_ORIGIN(wanted_circ_purpose));

  circ = hs_circuitmap_get_circuit_impl(type, token_len, token,
                                        wanted_circ_purpose);
  if (!circ) {
    return NULL;
  }

  tor_assert(CIRCUIT_IS_ORCIRC(circ));
  return TO_OR_CIRCUIT(circ);
}

/************** Public circuitmap API ****************************************/

/**** Public relay-side getters: */

/** Public function: Return v2 and v3 introduction circuit to this relay.
 * Always return a newly allocated list for which it is the caller's
 * responsibility to free it. */
smartlist_t *
hs_circuitmap_get_all_intro_circ_relay_side(void)
{
  circuit_t **iter;
  smartlist_t *circuit_list = smartlist_new();

  HT_FOREACH(iter, hs_circuitmap_ht, the_hs_circuitmap) {
    circuit_t *circ = *iter;

    /* An origin circuit or purpose is wrong or the hs token is not set to be
     * a v2 or v3 intro relay side type, we ignore the circuit. Else, we have
     * a match so add it to our list. */
    if (CIRCUIT_IS_ORIGIN(circ) ||
        circ->purpose != CIRCUIT_PURPOSE_INTRO_POINT ||
        (circ->hs_token->type != HS_TOKEN_INTRO_V3_RELAY_SIDE &&
         circ->hs_token->type != HS_TOKEN_INTRO_V2_RELAY_SIDE)) {
      continue;
    }
    smartlist_add(circuit_list, circ);
  }

  return circuit_list;
}

/** Public function: Return a v3 introduction circuit to this relay with
 * <b>auth_key</b>. Return NULL if no such circuit is found in the
 * circuitmap. */
or_circuit_t *
hs_circuitmap_get_intro_circ_v3_relay_side(
                                          const ed25519_public_key_t *auth_key)
{
  return hs_circuitmap_get_or_circuit(HS_TOKEN_INTRO_V3_RELAY_SIDE,
                                      ED25519_PUBKEY_LEN, auth_key->pubkey,
                                      CIRCUIT_PURPOSE_INTRO_POINT);
}

/** Public function: Return v2 introduction circuit to this relay with
 * <b>digest</b>. Return NULL if no such circuit is found in the circuitmap. */
or_circuit_t *
hs_circuitmap_get_intro_circ_v2_relay_side(const uint8_t *digest)
{
  return hs_circuitmap_get_or_circuit(HS_TOKEN_INTRO_V2_RELAY_SIDE,
                                      REND_TOKEN_LEN, digest,
                                      CIRCUIT_PURPOSE_INTRO_POINT);
}

/** Public function: Return rendezvous circuit to this relay with rendezvous
 * <b>cookie</b>. Return NULL if no such circuit is found in the circuitmap. */
or_circuit_t *
hs_circuitmap_get_rend_circ_relay_side(const uint8_t *cookie)
{
  return hs_circuitmap_get_or_circuit(HS_TOKEN_REND_RELAY_SIDE,
                                      REND_TOKEN_LEN, cookie,
                                      CIRCUIT_PURPOSE_REND_POINT_WAITING);
}

/** Public relay-side setters: */

/** Public function: Register rendezvous circuit with key <b>cookie</b> to the
 * circuitmap. */
void
hs_circuitmap_register_rend_circ_relay_side(or_circuit_t *circ,
                                            const uint8_t *cookie)
{
  hs_circuitmap_register_circuit(TO_CIRCUIT(circ),
                                 HS_TOKEN_REND_RELAY_SIDE,
                                 REND_TOKEN_LEN, cookie);
}
/** Public function: Register v2 intro circuit with key <b>digest</b> to the
 * circuitmap. */
void
hs_circuitmap_register_intro_circ_v2_relay_side(or_circuit_t *circ,
                                                const uint8_t *digest)
{
  hs_circuitmap_register_circuit(TO_CIRCUIT(circ),
                                 HS_TOKEN_INTRO_V2_RELAY_SIDE,
                                 REND_TOKEN_LEN, digest);
}

/** Public function: Register v3 intro circuit with key <b>auth_key</b> to the
 * circuitmap. */
void
hs_circuitmap_register_intro_circ_v3_relay_side(or_circuit_t *circ,
                                          const ed25519_public_key_t *auth_key)
{
  hs_circuitmap_register_circuit(TO_CIRCUIT(circ),
                                 HS_TOKEN_INTRO_V3_RELAY_SIDE,
                                 ED25519_PUBKEY_LEN, auth_key->pubkey);
}

/**** Public servide-side getters: */

/** Public function: Return v3 introduction circuit with <b>auth_key</b>
 * originating from this hidden service. Return NULL if no such circuit is
 * found in the circuitmap. */
origin_circuit_t *
hs_circuitmap_get_intro_circ_v3_service_side(const
                                             ed25519_public_key_t *auth_key)
{
  origin_circuit_t *circ = NULL;

  /* Check first for established intro circuits */
  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_INTRO_V3_SERVICE_SIDE,
                                          ED25519_PUBKEY_LEN, auth_key->pubkey,
                                          CIRCUIT_PURPOSE_S_INTRO);
  if (circ) {
    return circ;
  }

  /* ...if nothing found, check for pending intro circs */
  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_INTRO_V3_SERVICE_SIDE,
                                          ED25519_PUBKEY_LEN, auth_key->pubkey,
                                          CIRCUIT_PURPOSE_S_ESTABLISH_INTRO);

  return circ;
}

/** Public function: Return v2 introduction circuit originating from this
 * hidden service with <b>digest</b>. Return NULL if no such circuit is found
 * in the circuitmap. */
origin_circuit_t *
hs_circuitmap_get_intro_circ_v2_service_side(const uint8_t *digest)
{
  origin_circuit_t *circ = NULL;

  /* Check first for established intro circuits */
  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_INTRO_V2_SERVICE_SIDE,
                                          REND_TOKEN_LEN, digest,
                                          CIRCUIT_PURPOSE_S_INTRO);
  if (circ) {
    return circ;
  }

  /* ...if nothing found, check for pending intro circs */
  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_INTRO_V2_SERVICE_SIDE,
                                          REND_TOKEN_LEN, digest,
                                          CIRCUIT_PURPOSE_S_ESTABLISH_INTRO);

  return circ;
}

/** Public function: Return rendezvous circuit originating from this hidden
 * service with rendezvous <b>cookie</b>. Return NULL if no such circuit is
 * found in the circuitmap. */
origin_circuit_t *
hs_circuitmap_get_rend_circ_service_side(const uint8_t *cookie)
{
  origin_circuit_t *circ = NULL;

  /* Try to check if we have a connecting circuit. */
  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_REND_SERVICE_SIDE,
                                          REND_TOKEN_LEN, cookie,
                                          CIRCUIT_PURPOSE_S_CONNECT_REND);
  if (circ) {
    return circ;
  }

  /* Then try for connected circuit. */
  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_REND_SERVICE_SIDE,
                                          REND_TOKEN_LEN, cookie,
                                          CIRCUIT_PURPOSE_S_REND_JOINED);
  return circ;
}

/** Public function: Return client-side rendezvous circuit with rendezvous
 * <b>cookie</b>. It will look for circuits with the following purposes:

 * a) CIRCUIT_PURPOSE_C_REND_READY: Established rend circuit (received
 *    RENDEZVOUS_ESTABLISHED). Waiting for RENDEZVOUS2 from service, and for
 *    INTRODUCE_ACK from intro point.
 *
 * b) CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED: Established rend circuit and
 *    introduce circuit acked. Waiting for RENDEZVOUS2 from service.
 *
 * c) CIRCUIT_PURPOSE_C_REND_JOINED: Established rend circuit and received
 *    RENDEZVOUS2 from service.
 *
 * d) CIRCUIT_PURPOSE_C_ESTABLISH_REND: Rend circuit open but not yet
 *    established.
 *
 * Return NULL if no such circuit is found in the circuitmap. */
origin_circuit_t *
hs_circuitmap_get_rend_circ_client_side(const uint8_t *cookie)
{
  origin_circuit_t *circ = NULL;

  circ = hs_circuitmap_get_established_rend_circ_client_side(cookie);
  if (circ) {
    return circ;
  }

  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_REND_CLIENT_SIDE,
                                          REND_TOKEN_LEN, cookie,
                                          CIRCUIT_PURPOSE_C_ESTABLISH_REND);
  return circ;
}

/**  Public function: Return client-side established rendezvous circuit with
 *  rendezvous <b>cookie</b>. It will look for circuits with the following
 *  purposes:
 *
 * a) CIRCUIT_PURPOSE_C_REND_READY: Established rend circuit (received
 *    RENDEZVOUS_ESTABLISHED). Waiting for RENDEZVOUS2 from service, and for
 *    INTRODUCE_ACK from intro point.
 *
 * b) CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED: Established rend circuit and
 *    introduce circuit acked. Waiting for RENDEZVOUS2 from service.
 *
 * c) CIRCUIT_PURPOSE_C_REND_JOINED: Established rend circuit and received
 *    RENDEZVOUS2 from service.
 *
 * Return NULL if no such circuit is found in the circuitmap. */
origin_circuit_t *
hs_circuitmap_get_established_rend_circ_client_side(const uint8_t *cookie)
{
  origin_circuit_t *circ = NULL;

  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_REND_CLIENT_SIDE,
                                          REND_TOKEN_LEN, cookie,
                                          CIRCUIT_PURPOSE_C_REND_READY);
  if (circ) {
    return circ;
  }

  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_REND_CLIENT_SIDE,
                                          REND_TOKEN_LEN, cookie,
                                 CIRCUIT_PURPOSE_C_REND_READY_INTRO_ACKED);
  if (circ) {
    return circ;
  }

  circ = hs_circuitmap_get_origin_circuit(HS_TOKEN_REND_CLIENT_SIDE,
                                          REND_TOKEN_LEN, cookie,
                                          CIRCUIT_PURPOSE_C_REND_JOINED);
  return circ;
}

/**** Public servide-side setters: */

/** Public function: Register v2 intro circuit with key <b>digest</b> to the
 * circuitmap. */
void
hs_circuitmap_register_intro_circ_v2_service_side(origin_circuit_t *circ,
                                                  const uint8_t *digest)
{
  hs_circuitmap_register_circuit(TO_CIRCUIT(circ),
                                 HS_TOKEN_INTRO_V2_SERVICE_SIDE,
                                 REND_TOKEN_LEN, digest);
}

/** Public function: Register v3 intro circuit with key <b>auth_key</b> to the
 * circuitmap. */
void
hs_circuitmap_register_intro_circ_v3_service_side(origin_circuit_t *circ,
                                          const ed25519_public_key_t *auth_key)
{
  hs_circuitmap_register_circuit(TO_CIRCUIT(circ),
                                 HS_TOKEN_INTRO_V3_SERVICE_SIDE,
                                 ED25519_PUBKEY_LEN, auth_key->pubkey);
}

/** Public function: Register rendezvous circuit with key <b>cookie</b> to the
 * circuitmap. */
void
hs_circuitmap_register_rend_circ_service_side(origin_circuit_t *circ,
                                              const uint8_t *cookie)
{
  hs_circuitmap_register_circuit(TO_CIRCUIT(circ),
                                 HS_TOKEN_REND_SERVICE_SIDE,
                                 REND_TOKEN_LEN, cookie);
}

/** Public function: Register rendezvous circuit with key <b>cookie</b> to the
 * client-side circuitmap. */
void
hs_circuitmap_register_rend_circ_client_side(origin_circuit_t *or_circ,
                                             const uint8_t *cookie)
{
  circuit_t *circ = TO_CIRCUIT(or_circ);
  { /* Basic circ purpose sanity checking */
    tor_assert_nonfatal(circ->purpose == CIRCUIT_PURPOSE_C_ESTABLISH_REND);
  }

  hs_circuitmap_register_circuit(circ, HS_TOKEN_REND_CLIENT_SIDE,
                                 REND_TOKEN_LEN, cookie);
}

/**** Misc public functions: */

/** Public function: Remove this circuit from the HS circuitmap. Clear its HS
 *  token, and remove it from the hashtable. */
void
hs_circuitmap_remove_circuit(circuit_t *circ)
{
  tor_assert(the_hs_circuitmap);

  if (!circ || !circ->hs_token) {
    return;
  }

  /* Remove circ from circuitmap */
  circuit_t *tmp;
  tmp = HT_REMOVE(hs_circuitmap_ht, the_hs_circuitmap, circ);
  /* ... and ensure the removal was successful. */
  if (tmp) {
    tor_assert(tmp == circ);
  } else {
    log_warn(LD_BUG, "Could not find circuit (%u) in circuitmap.",
             circ->n_circ_id);
  }

  /* Clear token from circ */
  hs_token_free(circ->hs_token);
  circ->hs_token = NULL;
}

/** Public function: Initialize the global HS circuitmap. */
void
hs_circuitmap_init(void)
{
  tor_assert(!the_hs_circuitmap);

  the_hs_circuitmap = tor_malloc_zero(sizeof(struct hs_circuitmap_ht));
  HT_INIT(hs_circuitmap_ht, the_hs_circuitmap);
}

/** Public function: Free all memory allocated by the global HS circuitmap. */
void
hs_circuitmap_free_all(void)
{
  if (the_hs_circuitmap) {
    HT_CLEAR(hs_circuitmap_ht, the_hs_circuitmap);
    tor_free(the_hs_circuitmap);
  }
}
