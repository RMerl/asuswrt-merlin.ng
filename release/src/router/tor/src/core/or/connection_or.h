/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file connection_or.h
 * \brief Header file for connection_or.c.
 **/

#ifndef TOR_CONNECTION_OR_H
#define TOR_CONNECTION_OR_H

struct ed25519_public_key_t;
struct ed25519_keypair_t;

or_connection_t *TO_OR_CONN(connection_t *);

#define OR_CONN_STATE_MIN_ 1
/** State for a connection to an OR: waiting for connect() to finish. */
#define OR_CONN_STATE_CONNECTING 1
/** State for a connection to an OR: waiting for proxy handshake to complete */
#define OR_CONN_STATE_PROXY_HANDSHAKING 2
/** State for an OR connection client: SSL is handshaking, not done
 * yet. */
#define OR_CONN_STATE_TLS_HANDSHAKING 3
/** State for a connection to an OR: We're doing a second SSL handshake for
 * renegotiation purposes. (V2 handshake only.) */
#define OR_CONN_STATE_TLS_CLIENT_RENEGOTIATING 4
/** State for a connection at an OR: We're waiting for the client to
 * renegotiate (to indicate a v2 handshake) or send a versions cell (to
 * indicate a v3 handshake) */
#define OR_CONN_STATE_TLS_SERVER_RENEGOTIATING 5
/** State for an OR connection: We're done with our SSL handshake, we've done
 * renegotiation, but we haven't yet negotiated link protocol versions and
 * sent a netinfo cell. */
#define OR_CONN_STATE_OR_HANDSHAKING_V2 6
/** State for an OR connection: We're done with our SSL handshake, but we
 * haven't yet negotiated link protocol versions, done a V3 handshake, and
 * sent a netinfo cell. */
#define OR_CONN_STATE_OR_HANDSHAKING_V3 7
/** State for an OR connection: Ready to send/receive cells. */
#define OR_CONN_STATE_OPEN 8
#define OR_CONN_STATE_MAX_ 8

void connection_or_clear_identity(or_connection_t *conn);
void connection_or_clear_identity_map(void);
void clear_broken_connection_map(int disable);
or_connection_t *connection_or_get_for_extend(const char *digest,
                                              const tor_addr_t *target_addr,
                                              const char **msg_out,
                                              int *launch_out);

void connection_or_block_renegotiation(or_connection_t *conn);
int connection_or_reached_eof(or_connection_t *conn);
int connection_or_process_inbuf(or_connection_t *conn);
ssize_t connection_or_num_cells_writeable(or_connection_t *conn);
int connection_or_flushed_some(or_connection_t *conn);
int connection_or_finished_flushing(or_connection_t *conn);
int connection_or_finished_connecting(or_connection_t *conn);
void connection_or_about_to_close(or_connection_t *conn);
int connection_or_digest_is_known_relay(const char *id_digest);
void connection_or_update_token_buckets(smartlist_t *conns,
                                        const or_options_t *options);

void connection_or_connect_failed(or_connection_t *conn,
                                  int reason, const char *msg);
void connection_or_notify_error(or_connection_t *conn,
                                int reason, const char *msg);
MOCK_DECL(or_connection_t *,
          connection_or_connect,
          (const tor_addr_t *addr, uint16_t port,
           const char *id_digest,
           const struct ed25519_public_key_t *ed_id,
           channel_tls_t *chan));

void connection_or_close_normally(or_connection_t *orconn, int flush);
MOCK_DECL(void,connection_or_close_for_error,
          (or_connection_t *orconn, int flush));

void connection_or_report_broken_states(int severity, int domain);

MOCK_DECL(int,connection_tls_start_handshake,(or_connection_t *conn,
                                              int receiving));
int connection_tls_continue_handshake(or_connection_t *conn);
void connection_or_set_canonical(or_connection_t *or_conn,
                                 int is_canonical);

int connection_init_or_handshake_state(or_connection_t *conn,
                                       int started_here);
void connection_or_init_conn_from_address(or_connection_t *conn,
                                    const tor_addr_t *addr,
                                    uint16_t port,
                                    const char *rsa_id_digest,
                                    const struct ed25519_public_key_t *ed_id,
                                    int started_here);
int connection_or_client_learned_peer_id(or_connection_t *conn,
                              const uint8_t *rsa_peer_id,
                              const struct ed25519_public_key_t *ed_peer_id);
time_t connection_or_client_used(or_connection_t *conn);
MOCK_DECL(int, connection_or_get_num_circuits, (or_connection_t *conn));
void or_handshake_state_free_(or_handshake_state_t *state);
#define or_handshake_state_free(state) \
  FREE_AND_NULL(or_handshake_state_t, or_handshake_state_free_, (state))
void or_handshake_state_record_cell(or_connection_t *conn,
                                    or_handshake_state_t *state,
                                    const cell_t *cell,
                                    int incoming);
void or_handshake_state_record_var_cell(or_connection_t *conn,
                                        or_handshake_state_t *state,
                                        const var_cell_t *cell,
                                        int incoming);

int connection_or_set_state_open(or_connection_t *conn);
void connection_or_write_cell_to_buf(const cell_t *cell,
                                     or_connection_t *conn);
MOCK_DECL(void,connection_or_write_var_cell_to_buf,(const var_cell_t *cell,
                                                   or_connection_t *conn));
int connection_or_send_versions(or_connection_t *conn, int v3_plus);
MOCK_DECL(int,connection_or_send_netinfo,(or_connection_t *conn));
int connection_or_send_certs_cell(or_connection_t *conn);
int connection_or_send_auth_challenge_cell(or_connection_t *conn);
int authchallenge_type_is_supported(uint16_t challenge_type);
int authchallenge_type_is_better(uint16_t challenge_type_a,
                                 uint16_t challenge_type_b);
var_cell_t *connection_or_compute_authenticate_cell_body(
                              or_connection_t *conn,
                              const int authtype,
                              crypto_pk_t *signing_key,
                              const struct ed25519_keypair_t *ed_signing_key,
                              int server);
MOCK_DECL(int,connection_or_send_authenticate_cell,
          (or_connection_t *conn, int type));

int is_or_protocol_version_known(uint16_t version);

void cell_pack(packed_cell_t *dest, const cell_t *src, int wide_circ_ids);
int var_cell_pack_header(const var_cell_t *cell, char *hdr_out,
                         int wide_circ_ids);
var_cell_t *var_cell_new(uint16_t payload_len);
var_cell_t *var_cell_copy(const var_cell_t *src);
void var_cell_free_(var_cell_t *cell);
#define var_cell_free(cell) FREE_AND_NULL(var_cell_t, var_cell_free_, (cell))

/* DOCDOC */
#define MIN_LINK_PROTO_FOR_WIDE_CIRC_IDS 4
#define MIN_LINK_PROTO_FOR_CHANNEL_PADDING 5
#define MAX_LINK_PROTO MIN_LINK_PROTO_FOR_CHANNEL_PADDING

int connection_or_single_set_badness_(time_t now,
                                      or_connection_t *or_conn,
                                      int force);
void connection_or_group_set_badness_(smartlist_t *group, int force);

#ifdef CONNECTION_OR_PRIVATE
STATIC int should_connect_to_relay(const or_connection_t *or_conn);
STATIC void note_or_connect_failed(const or_connection_t *or_conn);
#endif

#ifdef TOR_UNIT_TESTS
extern int certs_cell_ed25519_disabled_for_testing;
extern int testing__connection_or_pretend_TLSSECRET_is_supported;
#endif

#endif /* !defined(TOR_CONNECTION_OR_H) */
