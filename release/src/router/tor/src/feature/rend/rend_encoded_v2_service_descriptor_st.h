/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file rend_encoded_v2_service_descriptor_st.h
 * @brief Encoded v2 HS descriptor structure.
 **/

#ifndef REND_ENCODED_V2_SERVICE_DESCRIPTOR_ST_H
#define REND_ENCODED_V2_SERVICE_DESCRIPTOR_ST_H

/** ASCII-encoded v2 hidden service descriptor. */
struct rend_encoded_v2_service_descriptor_t {
  char desc_id[DIGEST_LEN]; /**< Descriptor ID. */
  char *desc_str; /**< Descriptor string. */
};

#endif /* !defined(REND_ENCODED_V2_SERVICE_DESCRIPTOR_ST_H) */
