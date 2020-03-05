/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file circuitbuild.h
 * \brief Header file for circuitbuild.c.
 **/

#ifndef TOR_CIRCUITBUILD_H
#define TOR_CIRCUITBUILD_H

struct ed25519_public_key_t;
struct curve25519_public_key_t;

int route_len_for_purpose(uint8_t purpose, extend_info_t *exit_ei);
char *circuit_list_path(origin_circuit_t *circ, int verbose);
char *circuit_list_path_for_controller(origin_circuit_t *circ);
void circuit_log_path(int severity, unsigned int domain,
                      origin_circuit_t *circ);
origin_circuit_t *origin_circuit_init(uint8_t purpose, int flags);
origin_circuit_t *circuit_establish_circuit(uint8_t purpose,
                                            extend_info_t *exit,
                                            int flags);
struct circuit_guard_state_t *origin_circuit_get_guard_state(
                                            origin_circuit_t *circ);
int circuit_handle_first_hop(origin_circuit_t *circ);
void circuit_n_chan_done(channel_t *chan, int status,
                         int close_origin_circuits);
int inform_testing_reachability(void);
int circuit_timeout_want_to_count_circ(const origin_circuit_t *circ);
int circuit_send_next_onion_skin(origin_circuit_t *circ);
void circuit_note_clock_jumped(int64_t seconds_elapsed, bool was_idle);
int circuit_extend(cell_t *cell, circuit_t *circ);
struct created_cell_t;
int circuit_finish_handshake(origin_circuit_t *circ,
                             const struct created_cell_t *created_cell);
int circuit_truncated(origin_circuit_t *circ, int reason);
int onionskin_answer(or_circuit_t *circ,
                     const struct created_cell_t *created_cell,
                     const char *keys, size_t keys_len,
                     const uint8_t *rend_circ_nonce);
MOCK_DECL(int, circuit_all_predicted_ports_handled, (time_t now,
                                                     int *need_uptime,
                                                     int *need_capacity));

int circuit_append_new_exit(origin_circuit_t *circ, extend_info_t *info);
int circuit_extend_to_new_exit(origin_circuit_t *circ, extend_info_t *info);
extend_info_t *extend_info_new(const char *nickname,
                               const char *rsa_id_digest,
                               const struct ed25519_public_key_t *ed_id,
                               crypto_pk_t *onion_key,
                               const struct curve25519_public_key_t *ntor_key,
                               const tor_addr_t *addr, uint16_t port);
extend_info_t *extend_info_from_node(const node_t *r, int for_direct_connect);
extend_info_t *extend_info_dup(extend_info_t *info);
void extend_info_free_(extend_info_t *info);
#define extend_info_free(info) \
  FREE_AND_NULL(extend_info_t, extend_info_free_, (info))
int extend_info_addr_is_allowed(const tor_addr_t *addr);
int extend_info_supports_tap(const extend_info_t* ei);
int extend_info_supports_ntor(const extend_info_t* ei);
int circuit_can_use_tap(const origin_circuit_t *circ);
int circuit_has_usable_onion_key(const origin_circuit_t *circ);
int extend_info_has_preferred_onion_key(const extend_info_t* ei);
const uint8_t *build_state_get_exit_rsa_id(cpath_build_state_t *state);
const node_t *build_state_get_exit_node(cpath_build_state_t *state);
const char *build_state_get_exit_nickname(cpath_build_state_t *state);

struct circuit_guard_state_t;

const node_t *choose_good_entry_server(uint8_t purpose,
                           cpath_build_state_t *state,
                           struct circuit_guard_state_t **guard_state_out);
void circuit_upgrade_circuits_from_guard_wait(void);

#ifdef CIRCUITBUILD_PRIVATE
STATIC circid_t get_unique_circ_id_by_chan(channel_t *chan);
STATIC int new_route_len(uint8_t purpose, extend_info_t *exit_ei,
                         const smartlist_t *nodes);
MOCK_DECL(STATIC int, count_acceptable_nodes, (const smartlist_t *nodes,
                                               int direct));

STATIC int onion_extend_cpath(origin_circuit_t *circ);

STATIC int
onion_pick_cpath_exit(origin_circuit_t *circ, extend_info_t *exit_ei,
                      int is_hs_v3_rp_circuit);

#endif /* defined(CIRCUITBUILD_PRIVATE) */

#endif /* !defined(TOR_CIRCUITBUILD_H) */
