/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file rend_service_descriptor_st.h
 * @brief Parsed v2 HS descriptor structure.
 **/

#ifndef REND_SERVICE_DESCRIPTOR_ST_H
#define REND_SERVICE_DESCRIPTOR_ST_H

#define REND_PROTOCOL_VERSION_BITMASK_WIDTH 16

/** Information used to connect to a hidden service.  Used on both the
 * service side and the client side. */
struct rend_service_descriptor_t {
  crypto_pk_t *pk; /**< This service's public key. */
  int version; /**< Version of the descriptor format: 0 or 2. */
  time_t timestamp; /**< Time when the descriptor was generated. */
  /** Bitmask: which introduce/rendezvous protocols are supported?
   * (We allow bits '0', '1', '2' and '3' to be set.) */
  unsigned protocols : REND_PROTOCOL_VERSION_BITMASK_WIDTH;
  /** List of the service's introduction points.  Elements are removed if
   * introduction attempts fail. */
  smartlist_t *intro_nodes;
  /** Has descriptor been uploaded to all hidden service directories? */
  int all_uploads_performed;
  /** List of hidden service directories to which an upload request for
   * this descriptor could be sent. Smartlist exists only when at least one
   * of the previous upload requests failed (otherwise it's not important
   * to know which uploads succeeded and which not). */
  smartlist_t *successful_uploads;
};

#endif /* !defined(REND_SERVICE_DESCRIPTOR_ST_H) */
