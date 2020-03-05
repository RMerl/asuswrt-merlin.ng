/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef NETWORKSTATUS_VOTER_INFO_ST_H
#define NETWORKSTATUS_VOTER_INFO_ST_H

/** Information about a single voter in a vote or a consensus. */
struct networkstatus_voter_info_t {
  /** Declared SHA-1 digest of this voter's identity key */
  char identity_digest[DIGEST_LEN];
  char *nickname; /**< Nickname of this voter */
  /** Digest of this voter's "legacy" identity key, if any.  In vote only; for
   * consensuses, we treat legacy keys as additional signers. */
  char legacy_id_digest[DIGEST_LEN];
  char *address; /**< Address of this voter, in string format. */
  uint32_t addr; /**< Address of this voter, in IPv4, in host order. */
  uint16_t dir_port; /**< Directory port of this voter */
  uint16_t or_port; /**< OR port of this voter */
  char *contact; /**< Contact information for this voter. */
  char vote_digest[DIGEST_LEN]; /**< Digest of this voter's vote, as signed. */

  /* Nothing from here on is signed. */
  /** The signature of the document and the signature's status. */
  smartlist_t *sigs;
};

#endif /* !defined(NETWORKSTATUS_VOTER_INFO_ST_H) */
