/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file onion.c
 * \brief Functions to queue create cells,
 * and parse and create the CREATE cell and its allies.
 *
 * This module has a few functions, all related to the CREATE/CREATED
 * handshake that we use on links in order to create a circuit, and the
 * related EXTEND/EXTENDED handshake that we use over circuits in order to
 * extend them an additional hop.
 *
 * Clients invoke these functions when creating or extending a circuit,
 * from circuitbuild.c.
 *
 * Relays invoke these functions when they receive a CREATE or EXTEND
 * cell in command.c or relay.c, in order to queue the pending request.
 * They also invoke them from cpuworker.c, which handles dispatching
 * onionskin requests to different worker threads.
 *
 * <br>
 *
 * This module also handles:
 *  <ul>
 *  <li> Queueing incoming onionskins on the relay side before passing
 *      them to worker threads.
 *   <li>Expiring onionskins on the relay side if they have waited for
 *     too long.
 *   <li>Packaging private keys on the server side in order to pass
 *     them to worker threads.
 *   <li>Encoding and decoding CREATE, CREATED, CREATE2, and CREATED2 cells.
 *   <li>Encoding and decodign EXTEND, EXTENDED, EXTEND2, and EXTENDED2
 *    relay cells.
 * </ul>
 **/

#include "core/or/or.h"

#include "app/config/config.h"
#include "core/crypto/onion_crypto.h"
#include "core/crypto/onion_fast.h"
#include "core/crypto/onion_ntor.h"
#include "core/crypto/onion_tap.h"
#include "core/or/onion.h"
#include "feature/nodelist/networkstatus.h"

#include "core/or/cell_st.h"

// trunnel
#include "trunnel/ed25519_cert.h"

/** Helper: return 0 if <b>cell</b> appears valid, -1 otherwise. If
 * <b>unknown_ok</b> is true, allow cells with handshake types we don't
 * recognize. */
static int
check_create_cell(const create_cell_t *cell, int unknown_ok)
{
  switch (cell->cell_type) {
  case CELL_CREATE:
    if (cell->handshake_type != ONION_HANDSHAKE_TYPE_TAP &&
        cell->handshake_type != ONION_HANDSHAKE_TYPE_NTOR)
      return -1;
    break;
  case CELL_CREATE_FAST:
    if (cell->handshake_type != ONION_HANDSHAKE_TYPE_FAST)
      return -1;
    break;
  case CELL_CREATE2:
    break;
  default:
    return -1;
  }

  switch (cell->handshake_type) {
  case ONION_HANDSHAKE_TYPE_TAP:
    if (cell->handshake_len != TAP_ONIONSKIN_CHALLENGE_LEN)
      return -1;
    break;
  case ONION_HANDSHAKE_TYPE_FAST:
    if (cell->handshake_len != CREATE_FAST_LEN)
      return -1;
    break;
  case ONION_HANDSHAKE_TYPE_NTOR:
    if (cell->handshake_len != NTOR_ONIONSKIN_LEN)
      return -1;
    break;
  case ONION_HANDSHAKE_TYPE_NTOR_V3:
    /* ntor v3 has variable length fields that are checked
     * elsewhere. Fall through to always valid here. */
    break;
  default:
    if (! unknown_ok)
      return -1;
  }

  return 0;
}

/** Write the various parameters into the create cell. Separate from
 * create_cell_parse() to make unit testing easier.
 */
void
create_cell_init(create_cell_t *cell_out, uint8_t cell_type,
                 uint16_t handshake_type, uint16_t handshake_len,
                 const uint8_t *onionskin)
{
  memset(cell_out, 0, sizeof(*cell_out));

  cell_out->cell_type = cell_type;
  cell_out->handshake_type = handshake_type;
  cell_out->handshake_len = handshake_len;
  memcpy(cell_out->onionskin, onionskin, handshake_len);
}

/** Helper: parse the CREATE2 payload at <b>p</b>, which could be up to
 * <b>p_len</b> bytes long, and use it to fill the fields of
 * <b>cell_out</b>. Return 0 on success and -1 on failure.
 *
 * Note that part of the body of an EXTEND2 cell is a CREATE2 payload, so
 * this function is also used for parsing those.
 */
static int
parse_create2_payload(create_cell_t *cell_out, const uint8_t *p, size_t p_len)
{
  uint16_t handshake_type, handshake_len;

  if (p_len < 4)
    return -1;

  handshake_type = ntohs(get_uint16(p));
  handshake_len = ntohs(get_uint16(p+2));

  if (handshake_len > CELL_PAYLOAD_SIZE - 4 || handshake_len > p_len - 4)
    return -1;
  if (handshake_type == ONION_HANDSHAKE_TYPE_FAST)
    return -1;

  create_cell_init(cell_out, CELL_CREATE2, handshake_type, handshake_len,
                   p+4);
  return 0;
}

/** Magic string which, in a CREATE or EXTEND cell, indicates that a seeming
 * TAP payload is really an ntor payload.  We'd do away with this if every
 * relay supported EXTEND2, but we want to be able to extend from A to B with
 * ntor even when A doesn't understand EXTEND2 and so can't generate a
 * CREATE2 cell.
 **/
#define NTOR_CREATE_MAGIC "ntorNTORntorNTOR"

/** Parse a CREATE, CREATE_FAST, or CREATE2 cell from <b>cell_in</b> into
 * <b>cell_out</b>. Return 0 on success, -1 on failure. (We reject some
 * syntactically valid CREATE2 cells that we can't generate or react to.) */
int
create_cell_parse(create_cell_t *cell_out, const cell_t *cell_in)
{
  switch (cell_in->command) {
  case CELL_CREATE:
    if (tor_memeq(cell_in->payload, NTOR_CREATE_MAGIC, 16)) {
      create_cell_init(cell_out, CELL_CREATE, ONION_HANDSHAKE_TYPE_NTOR,
                       NTOR_ONIONSKIN_LEN, cell_in->payload+16);
    } else {
      create_cell_init(cell_out, CELL_CREATE, ONION_HANDSHAKE_TYPE_TAP,
                       TAP_ONIONSKIN_CHALLENGE_LEN, cell_in->payload);
    }
    break;
  case CELL_CREATE_FAST:
    create_cell_init(cell_out, CELL_CREATE_FAST, ONION_HANDSHAKE_TYPE_FAST,
                     CREATE_FAST_LEN, cell_in->payload);
    break;
  case CELL_CREATE2:
    if (parse_create2_payload(cell_out, cell_in->payload,
                              CELL_PAYLOAD_SIZE) < 0)
      return -1;
    break;
  default:
    return -1;
  }

  return check_create_cell(cell_out, 0);
}

/** Helper: return 0 if <b>cell</b> appears valid, -1 otherwise. */
static int
check_created_cell(const created_cell_t *cell)
{
  switch (cell->cell_type) {
  case CELL_CREATED:
    if (cell->handshake_len != TAP_ONIONSKIN_REPLY_LEN &&
        cell->handshake_len != NTOR_REPLY_LEN)
      return -1;
    break;
  case CELL_CREATED_FAST:
    if (cell->handshake_len != CREATED_FAST_LEN)
      return -1;
    break;
  case CELL_CREATED2:
    if (cell->handshake_len > RELAY_PAYLOAD_SIZE-2)
      return -1;
    break;
  }

  return 0;
}

/** Parse a CREATED, CREATED_FAST, or CREATED2 cell from <b>cell_in</b> into
 * <b>cell_out</b>. Return 0 on success, -1 on failure. */
int
created_cell_parse(created_cell_t *cell_out, const cell_t *cell_in)
{
  memset(cell_out, 0, sizeof(*cell_out));

  switch (cell_in->command) {
  case CELL_CREATED:
    cell_out->cell_type = CELL_CREATED;
    cell_out->handshake_len = TAP_ONIONSKIN_REPLY_LEN;
    memcpy(cell_out->reply, cell_in->payload, TAP_ONIONSKIN_REPLY_LEN);
    break;
  case CELL_CREATED_FAST:
    cell_out->cell_type = CELL_CREATED_FAST;
    cell_out->handshake_len = CREATED_FAST_LEN;
    memcpy(cell_out->reply, cell_in->payload, CREATED_FAST_LEN);
    break;
  case CELL_CREATED2:
    {
      const uint8_t *p = cell_in->payload;
      cell_out->cell_type = CELL_CREATED2;
      cell_out->handshake_len = ntohs(get_uint16(p));
      if (cell_out->handshake_len > CELL_PAYLOAD_SIZE - 2)
        return -1;
      memcpy(cell_out->reply, p+2, cell_out->handshake_len);
      break;
    }
  }

  return check_created_cell(cell_out);
}

/** Helper: return 0 if <b>cell</b> appears valid, -1 otherwise. */
static int
check_extend_cell(const extend_cell_t *cell)
{
  const bool is_extend2 = (cell->cell_type == RELAY_COMMAND_EXTEND2);

  if (tor_digest_is_zero((const char*)cell->node_id))
    return -1;
  if (!tor_addr_port_is_valid_ap(&cell->orport_ipv4, 0)) {
    /* EXTEND cells must have an IPv4 address. */
    if (!is_extend2) {
      return -1;
    }
    /* EXTEND2 cells must have at least one IP address.
     * It can be IPv4 or IPv6. */
    if (!tor_addr_port_is_valid_ap(&cell->orport_ipv6, 0)) {
      return -1;
    }
  }
  if (cell->create_cell.cell_type == CELL_CREATE) {
    if (cell->cell_type != RELAY_COMMAND_EXTEND)
      return -1;
  } else if (cell->create_cell.cell_type == CELL_CREATE2) {
    if (cell->cell_type != RELAY_COMMAND_EXTEND2 &&
        cell->cell_type != RELAY_COMMAND_EXTEND)
      return -1;
  } else {
    /* In particular, no CREATE_FAST cells are allowed */
    return -1;
  }
  if (cell->create_cell.handshake_type == ONION_HANDSHAKE_TYPE_FAST)
    return -1;

  return check_create_cell(&cell->create_cell, 1);
}

static int
extend_cell_from_extend1_cell_body(extend_cell_t *cell_out,
                                   const extend1_cell_body_t *cell)
{
  tor_assert(cell_out);
  tor_assert(cell);
  memset(cell_out, 0, sizeof(*cell_out));
  tor_addr_make_unspec(&cell_out->orport_ipv4.addr);
  tor_addr_make_unspec(&cell_out->orport_ipv6.addr);

  cell_out->cell_type = RELAY_COMMAND_EXTEND;
  tor_addr_from_ipv4h(&cell_out->orport_ipv4.addr, cell->ipv4addr);
  cell_out->orport_ipv4.port = cell->port;
  if (tor_memeq(cell->onionskin, NTOR_CREATE_MAGIC, 16)) {
    cell_out->create_cell.cell_type = CELL_CREATE2;
    cell_out->create_cell.handshake_type = ONION_HANDSHAKE_TYPE_NTOR;
    cell_out->create_cell.handshake_len = NTOR_ONIONSKIN_LEN;
    memcpy(cell_out->create_cell.onionskin, cell->onionskin + 16,
           NTOR_ONIONSKIN_LEN);
  } else {
    cell_out->create_cell.cell_type = CELL_CREATE;
    cell_out->create_cell.handshake_type = ONION_HANDSHAKE_TYPE_TAP;
    cell_out->create_cell.handshake_len = TAP_ONIONSKIN_CHALLENGE_LEN;
    memcpy(cell_out->create_cell.onionskin, cell->onionskin,
           TAP_ONIONSKIN_CHALLENGE_LEN);
  }
  memcpy(cell_out->node_id, cell->identity, DIGEST_LEN);
  return 0;
}

static int
create_cell_from_create2_cell_body(create_cell_t *cell_out,
                                   const create2_cell_body_t *cell)
{
  tor_assert(cell_out);
  tor_assert(cell);
  memset(cell_out, 0, sizeof(create_cell_t));
  if (BUG(cell->handshake_len > sizeof(cell_out->onionskin))) {
    /* This should be impossible because there just isn't enough room in the
     * input cell to make the handshake_len this large and provide a
     * handshake_data to match. */
    return -1;
  }

  cell_out->cell_type = CELL_CREATE2;
  cell_out->handshake_type = cell->handshake_type;
  cell_out->handshake_len = cell->handshake_len;
  memcpy(cell_out->onionskin,
       create2_cell_body_getconstarray_handshake_data(cell),
       cell->handshake_len);
  return 0;
}

static int
extend_cell_from_extend2_cell_body(extend_cell_t *cell_out,
                                   const extend2_cell_body_t *cell)
{
  tor_assert(cell_out);
  tor_assert(cell);
  int found_ipv4 = 0, found_ipv6 = 0, found_rsa_id = 0, found_ed_id = 0;
  memset(cell_out, 0, sizeof(*cell_out));
  tor_addr_make_unspec(&cell_out->orport_ipv4.addr);
  tor_addr_make_unspec(&cell_out->orport_ipv6.addr);
  cell_out->cell_type = RELAY_COMMAND_EXTEND2;

  unsigned i;
  for (i = 0; i < cell->n_spec; ++i) {
    const link_specifier_t *ls = extend2_cell_body_getconst_ls(cell, i);
    switch (ls->ls_type) {
      case LS_IPV4:
        if (found_ipv4)
          continue;
        found_ipv4 = 1;
        tor_addr_from_ipv4h(&cell_out->orport_ipv4.addr, ls->un_ipv4_addr);
        cell_out->orport_ipv4.port = ls->un_ipv4_port;
        break;
      case LS_IPV6:
        if (found_ipv6)
          continue;
        found_ipv6 = 1;
        tor_addr_from_ipv6_bytes(&cell_out->orport_ipv6.addr,
                                 ls->un_ipv6_addr);
        cell_out->orport_ipv6.port = ls->un_ipv6_port;
        break;
      case LS_LEGACY_ID:
        if (found_rsa_id)
          return -1;
        found_rsa_id = 1;
        memcpy(cell_out->node_id, ls->un_legacy_id, 20);
        break;
      case LS_ED25519_ID:
        if (found_ed_id)
          return -1;
        found_ed_id = 1;
        memcpy(cell_out->ed_pubkey.pubkey, ls->un_ed25519_id, 32);
        break;
      default:
        /* Ignore this, whatever it is. */
        break;
    }
  }

  /* EXTEND2 cells must have an RSA ID */
  if (!found_rsa_id)
    return -1;

  /* EXTEND2 cells must have at least one IP address */
  if (!found_ipv4 && !found_ipv6)
    return -1;

  return create_cell_from_create2_cell_body(&cell_out->create_cell,
                                            cell->create2);
}

/** Parse an EXTEND or EXTEND2 cell (according to <b>command</b>) from the
 * <b>payload_length</b> bytes of <b>payload</b> into <b>cell_out</b>. Return
 * 0 on success, -1 on failure. */
MOCK_IMPL(int,
extend_cell_parse,(extend_cell_t *cell_out,
                   const uint8_t command,
                   const uint8_t *payload,
                   size_t payload_length))
{

  tor_assert(cell_out);
  tor_assert(payload);

  if (payload_length > RELAY_PAYLOAD_SIZE)
    return -1;

  switch (command) {
  case RELAY_COMMAND_EXTEND:
    {
      extend1_cell_body_t *cell = NULL;
      if (extend1_cell_body_parse(&cell, payload, payload_length)<0 ||
          cell == NULL) {
        if (cell)
          extend1_cell_body_free(cell);
        return -1;
      }
      int r = extend_cell_from_extend1_cell_body(cell_out, cell);
      extend1_cell_body_free(cell);
      if (r < 0)
        return r;
    }
    break;
  case RELAY_COMMAND_EXTEND2:
    {
      extend2_cell_body_t *cell = NULL;
      if (extend2_cell_body_parse(&cell, payload, payload_length) < 0 ||
          cell == NULL) {
        if (cell)
          extend2_cell_body_free(cell);
        return -1;
      }
      int r = extend_cell_from_extend2_cell_body(cell_out, cell);
      extend2_cell_body_free(cell);
      if (r < 0)
        return r;
    }
    break;
  default:
    return -1;
  }

  return check_extend_cell(cell_out);
}

/** Helper: return 0 if <b>cell</b> appears valid, -1 otherwise. */
static int
check_extended_cell(const extended_cell_t *cell)
{
  tor_assert(cell);
  if (cell->created_cell.cell_type == CELL_CREATED) {
    if (cell->cell_type != RELAY_COMMAND_EXTENDED)
      return -1;
  } else if (cell->created_cell.cell_type == CELL_CREATED2) {
    if (cell->cell_type != RELAY_COMMAND_EXTENDED2)
      return -1;
  } else {
    return -1;
  }

  return check_created_cell(&cell->created_cell);
}

/** Parse an EXTENDED or EXTENDED2 cell (according to <b>command</b>) from the
 * <b>payload_length</b> bytes of <b>payload</b> into <b>cell_out</b>. Return
 * 0 on success, -1 on failure. */
int
extended_cell_parse(extended_cell_t *cell_out,
                    const uint8_t command, const uint8_t *payload,
                    size_t payload_len)
{
  tor_assert(cell_out);
  tor_assert(payload);

  memset(cell_out, 0, sizeof(*cell_out));
  if (payload_len > RELAY_PAYLOAD_SIZE)
    return -1;

  switch (command) {
  case RELAY_COMMAND_EXTENDED:
    if (payload_len != TAP_ONIONSKIN_REPLY_LEN)
      return -1;
    cell_out->cell_type = RELAY_COMMAND_EXTENDED;
    cell_out->created_cell.cell_type = CELL_CREATED;
    cell_out->created_cell.handshake_len = TAP_ONIONSKIN_REPLY_LEN;
    memcpy(cell_out->created_cell.reply, payload, TAP_ONIONSKIN_REPLY_LEN);
    break;
  case RELAY_COMMAND_EXTENDED2:
    {
      cell_out->cell_type = RELAY_COMMAND_EXTENDED2;
      cell_out->created_cell.cell_type = CELL_CREATED2;
      cell_out->created_cell.handshake_len = ntohs(get_uint16(payload));
      if (cell_out->created_cell.handshake_len > RELAY_PAYLOAD_SIZE - 2 ||
          cell_out->created_cell.handshake_len > payload_len - 2)
        return -1;
      memcpy(cell_out->created_cell.reply, payload+2,
             cell_out->created_cell.handshake_len);
    }
    break;
  default:
    return -1;
  }

  return check_extended_cell(cell_out);
}

/** Fill <b>cell_out</b> with a correctly formatted version of the
 * CREATE{,_FAST,2} cell in <b>cell_in</b>. Return 0 on success, -1 on
 * failure.  This is a cell we didn't originate if <b>relayed</b> is true. */
static int
create_cell_format_impl(cell_t *cell_out, const create_cell_t *cell_in,
                        int relayed)
{
  uint8_t *p;
  size_t space;
  if (check_create_cell(cell_in, relayed) < 0)
    return -1;

  memset(cell_out->payload, 0, sizeof(cell_out->payload));
  cell_out->command = cell_in->cell_type;

  p = cell_out->payload;
  space = sizeof(cell_out->payload);

  switch (cell_in->cell_type) {
  case CELL_CREATE:
    if (BUG(cell_in->handshake_type == ONION_HANDSHAKE_TYPE_NTOR_V3)) {
      log_warn(LD_BUG, "Create cells cannot contain ntorv3.");
      return -1;
    }

    if (cell_in->handshake_type == ONION_HANDSHAKE_TYPE_NTOR) {
      memcpy(p, NTOR_CREATE_MAGIC, 16);
      p += 16;
      space -= 16;
    }
    FALLTHROUGH;
  case CELL_CREATE_FAST:
    tor_assert(cell_in->handshake_len <= space);
    memcpy(p, cell_in->onionskin, cell_in->handshake_len);
    break;
  case CELL_CREATE2:
    tor_assert(cell_in->handshake_len <= sizeof(cell_out->payload)-4);
    set_uint16(cell_out->payload, htons(cell_in->handshake_type));
    set_uint16(cell_out->payload+2, htons(cell_in->handshake_len));
    memcpy(cell_out->payload + 4, cell_in->onionskin, cell_in->handshake_len);
    break;
  default:
    return -1;
  }

  return 0;
}

int
create_cell_format(cell_t *cell_out, const create_cell_t *cell_in)
{
  return create_cell_format_impl(cell_out, cell_in, 0);
}

int
create_cell_format_relayed(cell_t *cell_out, const create_cell_t *cell_in)
{
  return create_cell_format_impl(cell_out, cell_in, 1);
}

/** Fill <b>cell_out</b> with a correctly formatted version of the
 * CREATED{,_FAST,2} cell in <b>cell_in</b>. Return 0 on success, -1 on
 * failure. */
int
created_cell_format(cell_t *cell_out, const created_cell_t *cell_in)
{
  if (check_created_cell(cell_in) < 0)
    return -1;

  memset(cell_out->payload, 0, sizeof(cell_out->payload));
  cell_out->command = cell_in->cell_type;

  switch (cell_in->cell_type) {
  case CELL_CREATED:
  case CELL_CREATED_FAST:
    tor_assert(cell_in->handshake_len <= sizeof(cell_out->payload));
    memcpy(cell_out->payload, cell_in->reply, cell_in->handshake_len);
    break;
  case CELL_CREATED2:
    tor_assert(cell_in->handshake_len <= sizeof(cell_out->payload)-2);
    set_uint16(cell_out->payload, htons(cell_in->handshake_len));
    memcpy(cell_out->payload + 2, cell_in->reply, cell_in->handshake_len);
    break;
  default:
    return -1;
  }
  return 0;
}

/** Return true iff we are configured (by torrc or by the networkstatus
 * parameters) to use Ed25519 identities in our Extend2 cells. */
static int
should_include_ed25519_id_extend_cells(const networkstatus_t *ns,
                                       const or_options_t *options)
{
  if (options->ExtendByEd25519ID != -1)
    return options->ExtendByEd25519ID; /* The user has an opinion. */

  return (int) networkstatus_get_param(ns, "ExtendByEd25519ID",
                                       0 /* default */,
                                       0 /* min */,
                                       1 /*max*/);
}

/** Format the EXTEND{,2} cell in <b>cell_in</b>, storing its relay payload in
 * <b>payload_out</b>, the number of bytes used in *<b>len_out</b>, and the
 * relay command in *<b>command_out</b>. The <b>payload_out</b> must have
 * RELAY_PAYLOAD_SIZE bytes available.  Return 0 on success, -1 on failure. */
int
extend_cell_format(uint8_t *command_out, uint16_t *len_out,
                   uint8_t *payload_out, const extend_cell_t *cell_in)
{
  uint8_t *p;
  if (check_extend_cell(cell_in) < 0)
    return -1;

  p = payload_out;

  memset(p, 0, RELAY_PAYLOAD_SIZE);

  switch (cell_in->cell_type) {
  case RELAY_COMMAND_EXTEND:
    {
      if (BUG(cell_in->create_cell.handshake_type ==
              ONION_HANDSHAKE_TYPE_NTOR_V3)) {
        log_warn(LD_BUG, "Extend cells cannot contain ntorv3!");
        return -1;
      }
      *command_out = RELAY_COMMAND_EXTEND;
      *len_out = 6 + TAP_ONIONSKIN_CHALLENGE_LEN + DIGEST_LEN;
      set_uint32(p, tor_addr_to_ipv4n(&cell_in->orport_ipv4.addr));
      set_uint16(p+4, htons(cell_in->orport_ipv4.port));
      if (cell_in->create_cell.handshake_type == ONION_HANDSHAKE_TYPE_NTOR) {
        memcpy(p+6, NTOR_CREATE_MAGIC, 16);
        memcpy(p+22, cell_in->create_cell.onionskin, NTOR_ONIONSKIN_LEN);
      } else {
        memcpy(p+6, cell_in->create_cell.onionskin,
               TAP_ONIONSKIN_CHALLENGE_LEN);
      }
      memcpy(p+6+TAP_ONIONSKIN_CHALLENGE_LEN, cell_in->node_id, DIGEST_LEN);
    }
    break;
  case RELAY_COMMAND_EXTEND2:
    {
      uint8_t n_specifiers = 1;
      *command_out = RELAY_COMMAND_EXTEND2;
      extend2_cell_body_t *cell = extend2_cell_body_new();
      link_specifier_t *ls;
      if (tor_addr_port_is_valid_ap(&cell_in->orport_ipv4, 0)) {
        /* Maybe IPv4 specifier first. */
        ++n_specifiers;
        ls = link_specifier_new();
        extend2_cell_body_add_ls(cell, ls);
        ls->ls_type = LS_IPV4;
        ls->ls_len = 6;
        ls->un_ipv4_addr = tor_addr_to_ipv4h(&cell_in->orport_ipv4.addr);
        ls->un_ipv4_port = cell_in->orport_ipv4.port;
      }
      {
        /* Then RSA id */
        ls = link_specifier_new();
        extend2_cell_body_add_ls(cell, ls);
        ls->ls_type = LS_LEGACY_ID;
        ls->ls_len = DIGEST_LEN;
        memcpy(ls->un_legacy_id, cell_in->node_id, DIGEST_LEN);
      }
      if (should_include_ed25519_id_extend_cells(NULL, get_options()) &&
          !ed25519_public_key_is_zero(&cell_in->ed_pubkey)) {
        /* Then, maybe, the ed25519 id! */
        ++n_specifiers;
        ls = link_specifier_new();
        extend2_cell_body_add_ls(cell, ls);
        ls->ls_type = LS_ED25519_ID;
        ls->ls_len = 32;
        memcpy(ls->un_ed25519_id, cell_in->ed_pubkey.pubkey, 32);
      }
      if (tor_addr_port_is_valid_ap(&cell_in->orport_ipv6, 0)) {
        /* Then maybe IPv6 specifier. */
        ++n_specifiers;
        ls = link_specifier_new();
        extend2_cell_body_add_ls(cell, ls);
        ls->ls_type = LS_IPV6;
        ls->ls_len = 18;
        tor_addr_copy_ipv6_bytes(ls->un_ipv6_addr,
                                 &cell_in->orport_ipv6.addr);
        ls->un_ipv6_port = cell_in->orport_ipv6.port;
      }
      cell->n_spec = n_specifiers;

      /* Now, the handshake */
      cell->create2 = create2_cell_body_new();
      cell->create2->handshake_type = cell_in->create_cell.handshake_type;
      cell->create2->handshake_len = cell_in->create_cell.handshake_len;
      create2_cell_body_setlen_handshake_data(cell->create2,
                                         cell_in->create_cell.handshake_len);
      memcpy(create2_cell_body_getarray_handshake_data(cell->create2),
             cell_in->create_cell.onionskin,
             cell_in->create_cell.handshake_len);

      ssize_t len_encoded = extend2_cell_body_encode(
                             payload_out, RELAY_PAYLOAD_SIZE,
                             cell);
      extend2_cell_body_free(cell);
      if (len_encoded < 0 || len_encoded > UINT16_MAX)
        return -1;
      *len_out = (uint16_t) len_encoded;
    }
    break;
  default:
    return -1;
  }

  return 0;
}

/** Format the EXTENDED{,2} cell in <b>cell_in</b>, storing its relay payload
 * in <b>payload_out</b>, the number of bytes used in *<b>len_out</b>, and the
 * relay command in *<b>command_out</b>. The <b>payload_out</b> must have
 * RELAY_PAYLOAD_SIZE bytes available.  Return 0 on success, -1 on failure. */
int
extended_cell_format(uint8_t *command_out, uint16_t *len_out,
                     uint8_t *payload_out, const extended_cell_t *cell_in)
{
  uint8_t *p;
  if (check_extended_cell(cell_in) < 0)
    return -1;

  p = payload_out;
  memset(p, 0, RELAY_PAYLOAD_SIZE);

  switch (cell_in->cell_type) {
  case RELAY_COMMAND_EXTENDED:
    {
      *command_out = RELAY_COMMAND_EXTENDED;
      *len_out = TAP_ONIONSKIN_REPLY_LEN;
      memcpy(payload_out, cell_in->created_cell.reply,
             TAP_ONIONSKIN_REPLY_LEN);
    }
    break;
  case RELAY_COMMAND_EXTENDED2:
    {
      *command_out = RELAY_COMMAND_EXTENDED2;
      *len_out = 2 + cell_in->created_cell.handshake_len;
      set_uint16(payload_out, htons(cell_in->created_cell.handshake_len));
      if (2+cell_in->created_cell.handshake_len > RELAY_PAYLOAD_SIZE)
        return -1;
      memcpy(payload_out+2, cell_in->created_cell.reply,
             cell_in->created_cell.handshake_len);
    }
    break;
  default:
    return -1;
  }

  return 0;
}
