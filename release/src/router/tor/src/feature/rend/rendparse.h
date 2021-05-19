/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rendparse.h
 * \brief Header file for rendparse.c.
 **/

#ifndef TOR_REND_PARSE_H
#define TOR_REND_PARSE_H

int rend_parse_v2_service_descriptor(rend_service_descriptor_t **parsed_out,
                                     char *desc_id_out,
                                     char **intro_points_encrypted_out,
                                     size_t *intro_points_encrypted_size_out,
                                     size_t *encoded_size_out,
                                     const char **next_out, const char *desc,
                                     int as_hsdir);
int rend_decrypt_introduction_points(char **ipos_decrypted,
                                     size_t *ipos_decrypted_size,
                                     const char *descriptor_cookie,
                                     const char *ipos_encrypted,
                                     size_t ipos_encrypted_size);
int rend_parse_introduction_points(rend_service_descriptor_t *parsed,
                                   const char *intro_points_encoded,
                                   size_t intro_points_encoded_size);
int rend_parse_client_keys(strmap_t *parsed_clients, const char *str);

#endif /* !defined(TOR_REND_PARSE_H) */
