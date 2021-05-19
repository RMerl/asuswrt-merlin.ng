/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file rend_authorized_client_st.h
 * @brief Hidden-service authorized client structure.
 **/

#ifndef REND_AUTHORIZED_CLIENT_ST_H
#define REND_AUTHORIZED_CLIENT_ST_H

/** Hidden-service side configuration of client authorization. */
struct rend_authorized_client_t {
  char *client_name;
  uint8_t descriptor_cookie[REND_DESC_COOKIE_LEN];
  crypto_pk_t *client_key;
};

#endif /* !defined(REND_AUTHORIZED_CLIENT_ST_H) */
