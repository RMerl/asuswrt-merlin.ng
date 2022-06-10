/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file circuitbuild_relay.c
 * @brief Implements the details of exteding circuits (by relaying extend
 * cells as create cells, and answering create cells).
 *
 * On the server side, this module handles the logic of responding to
 * RELAY_EXTEND requests, using circuit_extend() and onionskin_answer().
 *
 * The shared client and server code is in core/or/circuitbuild.c.
 **/

#include "orconfig.h"
#include "feature/relay/circuitbuild_relay.h"

#include "lib/crypt_ops/crypto_rand.h"

#include "core/or/or.h"
#include "app/config/config.h"

#include "core/crypto/relay_crypto.h"

#include "core/or/cell_st.h"
#include "core/or/circuit_st.h"
#include "core/or/extend_info_st.h"
#include "core/or/or_circuit_st.h"

#include "core/or/channel.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/extendinfo.h"
#include "core/or/onion.h"
#include "core/or/relay.h"

#include "feature/nodelist/nodelist.h"

#include "feature/relay/router.h"
#include "feature/relay/routermode.h"
#include "feature/relay/selftest.h"

/* Before replying to an extend cell, check the state of the circuit
 * <b>circ</b>, and the configured tor mode.
 *
 * <b>circ</b> must not be NULL.
 *
 * If the state and mode are valid, return 0.
 * Otherwise, if they are invalid, log a protocol warning, and return -1.
 */
STATIC int
circuit_extend_state_valid_helper(const struct circuit_t *circ)
{
  if (!server_mode(get_options())) {
    circuitbuild_warn_client_extend();
    return -1;
  }

  IF_BUG_ONCE(!circ) {
    return -1;
  }

  if (circ->n_chan) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "n_chan already set. Bug/attack. Closing.");
    return -1;
  }

  if (circ->n_hop) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "conn to next hop already launched. Bug/attack. Closing.");
    return -1;
  }

  return 0;
}

/* Make sure the extend cell <b>ec</b> has an ed25519 link specifier.
 *
 * First, check that the RSA node id is valid.
 * If the node id is valid, add the ed25519 link specifier (if required),
 * and return 0.
 *
 * Otherwise, if the node id is invalid, log a protocol warning,
 * and return -1.(And do not modify the extend cell.)
 *
 * Must be called before circuit_extend_lspec_valid_helper().
 */
STATIC int
circuit_extend_add_ed25519_helper(struct extend_cell_t *ec)
{
  IF_BUG_ONCE(!ec) {
    return -1;
  }

  /* Check if they asked us for 0000..0000. We support using
   * an empty fingerprint for the first hop (e.g. for a bridge relay),
   * but we don't want to let clients send us extend cells for empty
   * fingerprints -- a) because it opens the user up to a mitm attack,
   * and b) because it lets an attacker force the relay to hold open a
   * new TLS connection for each extend request. */
  if (tor_digest_is_zero((const char*)ec->node_id)) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Client asked me to extend without specifying an id_digest.");
    return -1;
  }

  /* Fill in ed_pubkey if it was not provided and we can infer it from
   * our networkstatus */
  if (ed25519_public_key_is_zero(&ec->ed_pubkey)) {
    const node_t *node = node_get_by_id((const char*)ec->node_id);
    const ed25519_public_key_t *node_ed_id = NULL;
    if (node &&
        node_supports_ed25519_link_authentication(node, 1) &&
        (node_ed_id = node_get_ed25519_id(node))) {
      ed25519_pubkey_copy(&ec->ed_pubkey, node_ed_id);
    }
  }

  return 0;
}

/* Make sure the extend cell <b>ec</b> has an IPv4 address if the relay
 * supports in, and if not, fill it in. */
STATIC int
circuit_extend_add_ipv4_helper(struct extend_cell_t *ec)
{
  IF_BUG_ONCE(!ec) {
    return -1;
  }

  const node_t *node = node_get_by_id((const char *) ec->node_id);
  if (node) {
    tor_addr_port_t node_ipv4;
    node_get_prim_orport(node, &node_ipv4);
    if (tor_addr_is_null(&ec->orport_ipv4.addr) &&
        !tor_addr_is_null(&node_ipv4.addr)) {
      tor_addr_copy(&ec->orport_ipv4.addr, &node_ipv4.addr);
      ec->orport_ipv4.port = node_ipv4.port;
    }
  }

  return 0;
}

/* Make sure the extend cell <b>ec</b> has an IPv6 address if the relay
 * supports in, and if not, fill it in. */
STATIC int
circuit_extend_add_ipv6_helper(struct extend_cell_t *ec)
{
  IF_BUG_ONCE(!ec) {
    return -1;
  }

  const node_t *node = node_get_by_id((const char *) ec->node_id);
  if (node) {
    tor_addr_port_t node_ipv6;
    node_get_pref_ipv6_orport(node, &node_ipv6);
    if (tor_addr_is_null(&ec->orport_ipv6.addr) &&
        !tor_addr_is_null(&node_ipv6.addr)) {
      tor_addr_copy(&ec->orport_ipv6.addr, &node_ipv6.addr);
      ec->orport_ipv6.port = node_ipv6.port;
    }
  }

  return 0;
}

/* Check if the address and port in the tor_addr_port_t <b>ap</b> are valid,
 * and are allowed by the current ExtendAllowPrivateAddresses config.
 *
 * If they are valid, return true.
 * Otherwise, if they are invalid, return false.
 *
 * If <b>log_zero_addrs</b> is true, log warnings about zero addresses at
 * <b>log_level</b>. If <b>log_internal_addrs</b> is true, log warnings about
 * internal addresses at <b>log_level</b>.
 */
static bool
circuit_extend_addr_port_is_valid(const struct tor_addr_port_t *ap,
                                  bool log_zero_addrs, bool log_internal_addrs,
                                  int log_level)
{
  /* It's safe to print the family. But we don't want to print the address,
   * unless specifically configured to do so. (Zero addresses aren't sensitive,
   * But some internal addresses might be.)*/

  if (!tor_addr_port_is_valid_ap(ap, 0)) {
    if (log_zero_addrs) {
      log_fn(log_level, LD_PROTOCOL,
             "Client asked me to extend to a zero destination port or "
             "%s address '%s'.",
             fmt_addr_family(&ap->addr), safe_str(fmt_addrport_ap(ap)));
    }
    return false;
  }

  if (tor_addr_is_internal(&ap->addr, 0) &&
      !get_options()->ExtendAllowPrivateAddresses) {
    if (log_internal_addrs) {
      log_fn(log_level, LD_PROTOCOL,
             "Client asked me to extend to a private %s address '%s'.",
             fmt_addr_family(&ap->addr),
             safe_str(fmt_and_decorate_addr(&ap->addr)));
    }
    return false;
  }

  return true;
}

/* Before replying to an extend cell, check the link specifiers in the extend
 * cell <b>ec</b>, which was received on the circuit <b>circ</b>.
 *
 * If they are valid, return 0.
 * Otherwise, if they are invalid, log a protocol warning, and return -1.
 *
 * Must be called after circuit_extend_add_ed25519_helper().
 */
STATIC int
circuit_extend_lspec_valid_helper(const struct extend_cell_t *ec,
                                  const struct circuit_t *circ)
{
  IF_BUG_ONCE(!ec) {
    return -1;
  }

  IF_BUG_ONCE(!circ) {
    return -1;
  }

  /* Check the addresses, without logging */
  const int ipv4_valid = circuit_extend_addr_port_is_valid(&ec->orport_ipv4,
                                                           false, false, 0);
  const int ipv6_valid = circuit_extend_addr_port_is_valid(&ec->orport_ipv6,
                                                           false, false, 0);
  /* We need at least one valid address */
  if (!ipv4_valid && !ipv6_valid) {
    /* Now, log the invalid addresses at protocol warning level */
    circuit_extend_addr_port_is_valid(&ec->orport_ipv4,
                                      true, true, LOG_PROTOCOL_WARN);
    circuit_extend_addr_port_is_valid(&ec->orport_ipv6,
                                      true, true, LOG_PROTOCOL_WARN);
    /* And fail */
    return -1;
  } else if (!ipv4_valid) {
    /* Always log unexpected internal addresses, but go on to use the other
     * valid address */
    circuit_extend_addr_port_is_valid(&ec->orport_ipv4,
                                      false, true, LOG_PROTOCOL_WARN);
  } else if (!ipv6_valid) {
    circuit_extend_addr_port_is_valid(&ec->orport_ipv6,
                                      false, true, LOG_PROTOCOL_WARN);
  }

  IF_BUG_ONCE(circ->magic != OR_CIRCUIT_MAGIC) {
    return -1;
  }

  const channel_t *p_chan = CONST_TO_OR_CIRCUIT(circ)->p_chan;

  IF_BUG_ONCE(!p_chan) {
    return -1;
  }

  /* Next, check if we're being asked to connect to the hop that the
   * extend cell came from. There isn't any reason for that, and it can
   * assist circular-path attacks. */
  if (tor_memeq(ec->node_id, p_chan->identity_digest, DIGEST_LEN)) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Client asked me to extend back to the previous hop.");
    return -1;
  }

  /* Check the previous hop Ed25519 ID too */
  if (! ed25519_public_key_is_zero(&ec->ed_pubkey) &&
      ed25519_pubkey_eq(&ec->ed_pubkey, &p_chan->ed25519_identity)) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Client asked me to extend back to the previous hop "
           "(by Ed25519 ID).");
    return -1;
  }

  return 0;
}

/* If possible, return a supported, non-NULL IP address.
 *
 * If both addresses are supported and non-NULL, choose one uniformly at
 * random.
 *
 * If we have an IPv6-only extend, but IPv6 is not supported, returns NULL.
 * If both addresses are NULL, also returns NULL. */
STATIC const tor_addr_port_t *
circuit_choose_ip_ap_for_extend(const tor_addr_port_t *ipv4_ap,
                                const tor_addr_port_t *ipv6_ap)
{
  const bool ipv6_supported = router_can_extend_over_ipv6(get_options());

  /* If IPv6 is not supported, we can't use the IPv6 address. */
  if (!ipv6_supported) {
    ipv6_ap = NULL;
  }

  /* If there is no IPv6 address, IPv4 is always supported.
   * Until clients include IPv6 ORPorts, and most relays support IPv6,
   * this is the most common case. */
  if (!ipv6_ap) {
    return ipv4_ap;
  }

  /* If there is no IPv4 address, return the (possibly NULL) IPv6 address. */
  if (!ipv4_ap) {
    return ipv6_ap;
  }

  /* Now we have an IPv4 and an IPv6 address, and IPv6 is supported.
   * So make an IPv6 connection at random, with probability 1 in N.
   *   1 means "always IPv6 (and no IPv4)"
   *   2 means "equal probability of IPv4 or IPv6"
   *   ... (and so on) ...
   *   (UINT_MAX - 1) means "almost always IPv4 (and almost never IPv6)"
   * To disable IPv6, set ipv6_supported to 0.
   */
#define IPV6_CONNECTION_ONE_IN_N 2

  bool choose_ipv6 = crypto_fast_rng_one_in_n(get_thread_fast_rng(),
                                              IPV6_CONNECTION_ONE_IN_N);
  if (choose_ipv6) {
    return ipv6_ap;
  } else {
    return ipv4_ap;
  }
}

/* When there is no open channel for an extend cell <b>ec</b>, set up the
 * circuit <b>circ</b> to wait for a new connection.
 *
 * If <b>should_launch</b> is true, open a new connection. (Otherwise, we are
 * already waiting for a new connection to the same relay.)
 *
 * Check if IPv6 extends are supported by our current configuration. If they
 * are, new connections may be made over IPv4 or IPv6. (IPv4 connections are
 * always supported.)
 */
STATIC void
circuit_open_connection_for_extend(const struct extend_cell_t *ec,
                                   struct circuit_t *circ,
                                   int should_launch)
{
  /* We have to check circ first, so we can close it on all other failures */
  IF_BUG_ONCE(!circ) {
    /* We can't mark a NULL circuit for close. */
    return;
  }

  /* Now we know that circ is not NULL */
  IF_BUG_ONCE(!ec) {
    circuit_mark_for_close(circ, END_CIRC_REASON_CONNECTFAILED);
    return;
  }

  /* Check the addresses, without logging */
  const int ipv4_valid = circuit_extend_addr_port_is_valid(&ec->orport_ipv4,
                                                           false, false, 0);
  const int ipv6_valid = circuit_extend_addr_port_is_valid(&ec->orport_ipv6,
                                                           false, false, 0);

  IF_BUG_ONCE(!ipv4_valid && !ipv6_valid) {
    /* circuit_extend_lspec_valid_helper() should have caught this */
    circuit_mark_for_close(circ, END_CIRC_REASON_CONNECTFAILED);
    return;
  }

  const tor_addr_port_t *chosen_ap = circuit_choose_ip_ap_for_extend(
                                        ipv4_valid ? &ec->orport_ipv4 : NULL,
                                        ipv6_valid ? &ec->orport_ipv6 : NULL);
  if (!chosen_ap) {
    /* An IPv6-only extend, but IPv6 is not supported */
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Received IPv6-only extend, but we don't have an IPv6 ORPort.");
    circuit_mark_for_close(circ, END_CIRC_REASON_CONNECTFAILED);
    return;
  }

  circ->n_hop = extend_info_new(NULL /*nickname*/,
                                (const char*)ec->node_id,
                                &ec->ed_pubkey,
                                NULL, /*onion_key*/
                                NULL, /*curve25519_key*/
                                &chosen_ap->addr,
                                chosen_ap->port,
                                NULL /* protover summary */,
                                false);

  circ->n_chan_create_cell = tor_memdup(&ec->create_cell,
                                        sizeof(ec->create_cell));

  circuit_set_state(circ, CIRCUIT_STATE_CHAN_WAIT);

  if (should_launch) {
    /* we should try to open a connection */
    channel_t *n_chan = channel_connect_for_circuit(circ->n_hop);
    if (!n_chan) {
      log_info(LD_CIRC,"Launching n_chan failed. Closing circuit.");
      circuit_mark_for_close(circ, END_CIRC_REASON_CONNECTFAILED);
      return;
    }
    log_debug(LD_CIRC,"connecting in progress (or finished). Good.");
  }
}

/** Take the 'extend' <b>cell</b>, pull out addr/port plus the onion
 * skin and identity digest for the next hop. If we're already connected,
 * pass the onion skin to the next hop using a create cell; otherwise
 * launch a new OR connection, and <b>circ</b> will notice when the
 * connection succeeds or fails.
 *
 * Return -1 if we want to warn and tear down the circuit, else return 0.
 */
int
circuit_extend(struct cell_t *cell, struct circuit_t *circ)
{
  channel_t *n_chan;
  relay_header_t rh;
  extend_cell_t ec;
  const char *msg = NULL;
  int should_launch = 0;

  IF_BUG_ONCE(!cell) {
    return -1;
  }

  IF_BUG_ONCE(!circ) {
    return -1;
  }

  if (circuit_extend_state_valid_helper(circ) < 0)
    return -1;

  relay_header_unpack(&rh, cell->payload);

  if (extend_cell_parse(&ec, rh.command,
                        cell->payload+RELAY_HEADER_SIZE,
                        rh.length) < 0) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Can't parse extend cell. Closing circuit.");
    return -1;
  }

  if (circuit_extend_add_ed25519_helper(&ec) < 0)
    return -1;

  if (circuit_extend_lspec_valid_helper(&ec, circ) < 0)
    return -1;

  if (circuit_extend_add_ipv4_helper(&ec) < 0)
    return -1;

  if (circuit_extend_add_ipv6_helper(&ec) < 0)
    return -1;

  /* Check the addresses, without logging */
  const int ipv4_valid = circuit_extend_addr_port_is_valid(&ec.orport_ipv4,
                                                           false, false, 0);
  const int ipv6_valid = circuit_extend_addr_port_is_valid(&ec.orport_ipv6,
                                                           false, false, 0);
  IF_BUG_ONCE(!ipv4_valid && !ipv6_valid) {
    /* circuit_extend_lspec_valid_helper() should have caught this */
    return -1;
  }

  n_chan = channel_get_for_extend((const char*)ec.node_id,
                                  &ec.ed_pubkey,
                                  ipv4_valid ? &ec.orport_ipv4.addr : NULL,
                                  ipv6_valid ? &ec.orport_ipv6.addr : NULL,
                                  false,
                                  &msg,
                                  &should_launch);

  if (!n_chan) {
    /* We can't use fmt_addr*() twice in the same function call,
     * because it uses a static buffer. */
    log_debug(LD_CIRC|LD_OR, "Next router IPv4 (%s): %s.",
              fmt_addrport_ap(&ec.orport_ipv4),
              msg ? msg : "????");
    log_debug(LD_CIRC|LD_OR, "Next router IPv6 (%s).",
              fmt_addrport_ap(&ec.orport_ipv6));

    circuit_open_connection_for_extend(&ec, circ, should_launch);

    /* return success. The onion/circuit/etc will be taken care of
     * automatically (may already have been) whenever n_chan reaches
     * OR_CONN_STATE_OPEN.
     */
    return 0;
  } else {
    /* Connection is already established.
     * So we need to extend the circuit to the next hop. */
    tor_assert(!circ->n_hop);
    circ->n_chan = n_chan;
    log_debug(LD_CIRC,
              "n_chan is %s.",
              channel_describe_peer(n_chan));

    if (circuit_deliver_create_cell(circ, &ec.create_cell, 1) < 0)
      return -1;

    return 0;
  }
}

/** On a relay, accept a create cell, initialise a circuit, and send a
 * created cell back.
 *
 * Given:
 *   - a response payload consisting of:
 *     - the <b>created_cell</b> and
 *     - an optional <b>rend_circ_nonce</b>, and
 *   - <b>keys</b> of length <b>keys_len</b>, which must be
 *     CPATH_KEY_MATERIAL_LEN;
 * then:
 *   - initialize the circuit <b>circ</b>'s cryptographic material,
 *   - set the circuit's state to open, and
 *   - send a created cell back on that circuit.
 *
 * If we haven't found our ORPorts reachable yet, and the channel meets the
 * necessary conditions, mark the relevant ORPorts as reachable.
 *
 * Returns -1 if cell or circuit initialisation fails.
 */
int
onionskin_answer(struct or_circuit_t *circ,
                 const created_cell_t *created_cell,
                 const char *keys, size_t keys_len,
                 const uint8_t *rend_circ_nonce)
{
  cell_t cell;

  IF_BUG_ONCE(!circ) {
    return -1;
  }

  IF_BUG_ONCE(!created_cell) {
    return -1;
  }

  IF_BUG_ONCE(!keys) {
    return -1;
  }

  IF_BUG_ONCE(!rend_circ_nonce) {
    return -1;
  }

  tor_assert(keys_len == CPATH_KEY_MATERIAL_LEN);

  if (created_cell_format(&cell, created_cell) < 0) {
    log_warn(LD_BUG,"couldn't format created cell (type=%d, len=%d).",
             (int)created_cell->cell_type, (int)created_cell->handshake_len);
    return -1;
  }
  cell.circ_id = circ->p_circ_id;

  circuit_set_state(TO_CIRCUIT(circ), CIRCUIT_STATE_OPEN);

  log_debug(LD_CIRC,"init digest forward 0x%.8x, backward 0x%.8x.",
            (unsigned int)get_uint32(keys),
            (unsigned int)get_uint32(keys+20));
  if (relay_crypto_init(&circ->crypto, keys, keys_len, 0, 0)<0) {
    log_warn(LD_BUG,"Circuit initialization failed.");
    return -1;
  }

  memcpy(circ->rend_circ_nonce, rend_circ_nonce, DIGEST_LEN);

  int used_create_fast = (created_cell->cell_type == CELL_CREATED_FAST);

  append_cell_to_circuit_queue(TO_CIRCUIT(circ),
                               circ->p_chan, &cell, CELL_DIRECTION_IN, 0);
  log_debug(LD_CIRC,"Finished sending '%s' cell.",
            used_create_fast ? "created_fast" : "created");

  /* Ignore the local bit when ExtendAllowPrivateAddresses is set:
   * it violates the assumption that private addresses are local.
   * Also, many test networks run on local addresses, and
   * TestingTorNetwork sets ExtendAllowPrivateAddresses. */
  if ((!channel_is_local(circ->p_chan)
       || get_options()->ExtendAllowPrivateAddresses)
      && !channel_is_outgoing(circ->p_chan)) {
    /* Okay, it's a create cell from a non-local connection
     * that we didn't initiate. Presumably this means that create cells
     * can reach us too. But what address can they reach us on? */
    const tor_addr_t *my_supposed_addr = &circ->p_chan->addr_according_to_peer;
    if (router_addr_is_my_published_addr(my_supposed_addr)) {
      /* Great, this create cell came on connection where the peer says
       * that the our address is an address we're actually advertising!
       * That should mean that we're reachable.  But before we finally
       * declare ourselves reachable, make sure that the address listed
       * by the peer is the same family as the peer is actually using.
       */
      tor_addr_t remote_addr;
      int family = tor_addr_family(my_supposed_addr);
      if (channel_get_addr_if_possible(circ->p_chan, &remote_addr) &&
          tor_addr_family(&remote_addr) == family) {
        router_orport_found_reachable(family);
      }
    }
  }

  return 0;
}
