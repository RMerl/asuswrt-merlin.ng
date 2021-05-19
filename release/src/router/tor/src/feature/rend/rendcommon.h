/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rendcommon.h
 * \brief Header file for rendcommon.c.
 **/

#ifndef TOR_RENDCOMMON_H
#define TOR_RENDCOMMON_H

typedef enum rend_intro_point_failure_t {
  INTRO_POINT_FAILURE_GENERIC     = 0,
  INTRO_POINT_FAILURE_TIMEOUT     = 1,
  INTRO_POINT_FAILURE_UNREACHABLE = 2,
} rend_intro_point_failure_t;

int rend_cmp_service_ids(const char *one, const char *two);

void rend_process_relay_cell(circuit_t *circ, const crypt_path_t *layer_hint,
                             int command, size_t length,
                             const uint8_t *payload);

void rend_service_descriptor_free_(rend_service_descriptor_t *desc);
#define rend_service_descriptor_free(desc) \
  FREE_AND_NULL(rend_service_descriptor_t, rend_service_descriptor_free_, \
                (desc))
int rend_get_service_id(crypto_pk_t *pk, char *out);
void rend_encoded_v2_service_descriptor_free_(
                               rend_encoded_v2_service_descriptor_t *desc);
#define rend_encoded_v2_service_descriptor_free(desc) \
  FREE_AND_NULL(rend_encoded_v2_service_descriptor_t, \
                rend_encoded_v2_service_descriptor_free_, (desc))
void rend_intro_point_free_(rend_intro_point_t *intro);
#define rend_intro_point_free(intro) \
  FREE_AND_NULL(rend_intro_point_t, rend_intro_point_free_, (intro))

int rend_valid_v2_service_id(const char *query);
int rend_valid_descriptor_id(const char *query);
int rend_valid_client_name(const char *client_name);
int rend_encode_v2_descriptors(smartlist_t *descs_out,
                               rend_service_descriptor_t *desc, time_t now,
                               uint8_t period, rend_auth_type_t auth_type,
                               crypto_pk_t *client_key,
                               smartlist_t *client_cookies);
int rend_compute_v2_desc_id(char *desc_id_out, const char *service_id,
                            const char *descriptor_cookie,
                            time_t now, uint8_t replica);
void rend_get_descriptor_id_bytes(char *descriptor_id_out,
                                  const char *service_id,
                                  const char *secret_id_part);
int hid_serv_get_responsible_directories(smartlist_t *responsible_dirs,
                                         const char *id);

int rend_circuit_pk_digest_eq(const origin_circuit_t *ocirc,
                              const uint8_t *digest);

char *rend_auth_encode_cookie(const uint8_t *cookie_in,
                              rend_auth_type_t auth_type);
int rend_auth_decode_cookie(const char *cookie_in,
                            uint8_t *cookie_out,
                            rend_auth_type_t *auth_type_out,
                            char **err_msg_out);

int rend_allow_non_anonymous_connection(const or_options_t* options);
int rend_non_anonymous_mode_enabled(const or_options_t *options);

void assert_circ_anonymity_ok(const origin_circuit_t *circ,
                              const or_options_t *options);

#ifdef RENDCOMMON_PRIVATE

STATIC int
rend_desc_v2_is_parsable(rend_encoded_v2_service_descriptor_t *desc);

#endif /* defined(RENDCOMMON_PRIVATE) */

#endif /* !defined(TOR_RENDCOMMON_H) */

