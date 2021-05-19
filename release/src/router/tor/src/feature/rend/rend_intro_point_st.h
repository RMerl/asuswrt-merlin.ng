/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file rend_intro_point_st.h
 * @brief v2 hidden service introduction point structure.
 **/

#ifndef REND_INTRO_POINT_ST_H
#define REND_INTRO_POINT_ST_H

struct replaycache_t;
struct crypto_pk_t;

/** Introduction point information.  Used both in rend_service_t (on
 * the service side) and in rend_service_descriptor_t (on both the
 * client and service side). */
struct rend_intro_point_t {
  extend_info_t *extend_info; /**< Extend info for connecting to this
                               * introduction point via a multi-hop path. */
  struct crypto_pk_t *intro_key; /**< Introduction key that replaces the
                                  * service key, if this descriptor is V2. */

  /** (Client side only) Flag indicating that a timeout has occurred
   * after sending an INTRODUCE cell to this intro point.  After a
   * timeout, an intro point should not be tried again during the same
   * hidden service connection attempt, but it may be tried again
   * during a future connection attempt. */
  unsigned int timed_out : 1;

  /** (Client side only) The number of times we have failed to build a
   * circuit to this intro point for some reason other than our
   * circuit-build timeout.  See also MAX_INTRO_POINT_REACHABILITY_FAILURES. */
  unsigned int unreachable_count : 3;

  /** (Service side only) Flag indicating that this intro point was
   * included in the last HS descriptor we generated. */
  unsigned int listed_in_last_desc : 1;

  /** (Service side only) A replay cache recording the RSA-encrypted parts
   * of INTRODUCE2 cells this intro point's circuit has received.  This is
   * used to prevent replay attacks. */
  struct replaycache_t *accepted_intro_rsa_parts;

  /** (Service side only) Count of INTRODUCE2 cells accepted from this
   * intro point.
   */
  int accepted_introduce2_count;

  /** (Service side only) Maximum number of INTRODUCE2 cells that this IP
   * will accept. This is a random value between
   * INTRO_POINT_MIN_LIFETIME_INTRODUCTIONS and
   * INTRO_POINT_MAX_LIFETIME_INTRODUCTIONS. */
  int max_introductions;

  /** (Service side only) The time at which this intro point was first
   * published, or -1 if this intro point has not yet been
   * published. */
  time_t time_published;

  /** (Service side only) The time at which this intro point should
   * (start to) expire, or -1 if we haven't decided when this intro
   * point should expire. */
  time_t time_to_expire;

  /** (Service side only) The amount of circuit creation we've made to this
   * intro point. This is incremented every time we do a circuit relaunch on
   * this object which is triggered when the circuit dies but the node is
   * still in the consensus. After MAX_INTRO_POINT_CIRCUIT_RETRIES, we give
   * up on it. */
  unsigned int circuit_retries;

  /** (Service side only) Set if this intro point has an established circuit
   * and unset if it doesn't. */
  unsigned int circuit_established:1;
};

#endif /* !defined(REND_INTRO_POINT_ST_H) */
