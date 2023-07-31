/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file hs_circuit.h
 * \brief Header file containing circuit data for the whole HS subsystem.
 **/

#ifndef TOR_HS_CIRCUIT_H
#define TOR_HS_CIRCUIT_H

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_ed25519.h"

#include "feature/hs/hs_service.h"

/* Cleanup function when the circuit is closed or freed. */
void hs_circ_cleanup_on_close(circuit_t *circ);
void hs_circ_cleanup_on_free(circuit_t *circ);
void hs_circ_cleanup_on_repurpose(circuit_t *circ);

/* Circuit API. */
int hs_circ_service_intro_has_opened(hs_service_t *service,
                                     hs_service_intro_point_t *ip,
                                     const hs_service_descriptor_t *desc,
                                     origin_circuit_t *circ);
void hs_circ_service_rp_has_opened(const hs_service_t *service,
                                   origin_circuit_t *circ);
int hs_circ_launch_intro_point(hs_service_t *service,
                               const hs_service_intro_point_t *ip,
                               extend_info_t *ei,
                               bool direct_conn);
int hs_circ_launch_rendezvous_point(const hs_service_t *service,
                                    const curve25519_public_key_t *onion_key,
                                    const uint8_t *rendezvous_cookie);
void hs_circ_retry_service_rendezvous_point(const origin_circuit_t *circ);

origin_circuit_t *hs_circ_service_get_intro_circ(
                                      const hs_service_intro_point_t *ip);
origin_circuit_t *hs_circ_service_get_established_intro_circ(
                                      const hs_service_intro_point_t *ip);

/* Cell API. */
int hs_circ_handle_intro_established(const hs_service_t *service,
                                     const hs_service_intro_point_t *ip,
                                     origin_circuit_t *circ,
                                     const uint8_t *payload,
                                     size_t payload_len);
struct hs_subcredential_t;
int hs_circ_handle_introduce2(const hs_service_t *service,
                              const origin_circuit_t *circ,
                              hs_service_intro_point_t *ip,
                              const struct hs_subcredential_t *subcredential,
                              const uint8_t *payload, size_t payload_len);
int hs_circ_send_introduce1(origin_circuit_t *intro_circ,
                            origin_circuit_t *rend_circ,
                            const hs_desc_intro_point_t *ip,
                            const struct hs_subcredential_t *subcredential);
int hs_circ_send_establish_rendezvous(origin_circuit_t *circ);

/* e2e circuit API. */

int hs_circuit_setup_e2e_rend_circ(origin_circuit_t *circ,
                                   const uint8_t *ntor_key_seed,
                                   size_t seed_len,
                                   int is_service_side);
int hs_circuit_setup_e2e_rend_circ_legacy_client(origin_circuit_t *circ,
                                          const uint8_t *rend_cell_body);

bool hs_circ_is_rend_sent_in_intro1(const origin_circuit_t *circ);

void hs_circ_setup_congestion_control(origin_circuit_t *origin_circ,
                                      uint8_t sendme_inc,
                                      bool is_single_onion);

#ifdef HS_CIRCUIT_PRIVATE

struct hs_ntor_rend_cell_keys_t;

STATIC hs_ident_circuit_t *
create_rp_circuit_identifier(const hs_service_t *service,
                             const uint8_t *rendezvous_cookie,
                             const curve25519_public_key_t *server_pk,
                             const struct hs_ntor_rend_cell_keys_t *keys);

struct hs_cell_introduce2_data_t;
MOCK_DECL(STATIC void,
launch_rendezvous_point_circuit,(const hs_service_t *service,
                                 const hs_service_intro_point_t *ip,
                                const struct hs_cell_introduce2_data_t *data));

#endif /* defined(HS_CIRCUIT_PRIVATE) */

#endif /* !defined(TOR_HS_CIRCUIT_H) */
