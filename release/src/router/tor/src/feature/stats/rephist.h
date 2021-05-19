/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file rephist.h
 * \brief Header file for rephist.c.
 **/

#ifndef TOR_REPHIST_H
#define TOR_REPHIST_H

void rep_hist_init(void);
void rep_hist_dump_stats(time_t now, int severity);

void rep_hist_make_router_pessimal(const char *id, time_t when);

void rep_history_clean(time_t before);

void rep_hist_note_router_reachable(const char *id, const tor_addr_t *at_addr,
                                    const uint16_t at_port, time_t when);
void rep_hist_note_router_unreachable(const char *id, time_t when);
int rep_hist_record_mtbf_data(time_t now, int missing_means_down);
int rep_hist_load_mtbf_data(time_t now);

time_t rep_hist_downrate_old_runs(time_t now);
long rep_hist_get_uptime(const char *id, time_t when);
double rep_hist_get_stability(const char *id, time_t when);
double rep_hist_get_weighted_fractional_uptime(const char *id, time_t when);
long rep_hist_get_weighted_time_known(const char *id, time_t when);
int rep_hist_have_measured_enough_stability(void);

void rep_hist_exit_stats_init(time_t now);
void rep_hist_reset_exit_stats(time_t now);
void rep_hist_exit_stats_term(void);
char *rep_hist_format_exit_stats(time_t now);
time_t rep_hist_exit_stats_write(time_t now);
void rep_hist_note_exit_bytes(uint16_t port, size_t num_written,
                              size_t num_read);
void rep_hist_note_exit_stream_opened(uint16_t port);

void rep_hist_buffer_stats_init(time_t now);
void rep_hist_buffer_stats_add_circ(circuit_t *circ,
                                    time_t end_of_interval);
time_t rep_hist_buffer_stats_write(time_t now);
void rep_hist_buffer_stats_term(void);
void rep_hist_add_buffer_stats(double mean_num_cells_in_queue,
     double mean_time_cells_in_queue, uint32_t processed_cells);
char *rep_hist_format_buffer_stats(time_t now);
void rep_hist_reset_buffer_stats(time_t now);

void rep_hist_desc_stats_init(time_t now);
void rep_hist_note_desc_served(const char * desc);
void rep_hist_desc_stats_term(void);
time_t rep_hist_desc_stats_write(time_t now);

void rep_hist_note_circuit_handshake_requested(uint16_t type);
void rep_hist_note_circuit_handshake_assigned(uint16_t type);
void rep_hist_log_circuit_handshake_stats(time_t now);

MOCK_DECL(int, rep_hist_get_circuit_handshake_requested, (uint16_t type));
MOCK_DECL(int, rep_hist_get_circuit_handshake_assigned, (uint16_t type));

void rep_hist_hs_stats_init(time_t now);
void rep_hist_hs_stats_term(void);
time_t rep_hist_hs_stats_write(time_t now);
char *rep_hist_get_hs_stats_string(void);
void rep_hist_seen_new_rp_cell(void);
void rep_hist_stored_maybe_new_hs(const crypto_pk_t *pubkey);

void rep_hist_free_all(void);

void rep_hist_note_negotiated_link_proto(unsigned link_proto,
                                         int started_here);
void rep_hist_log_link_protocol_counts(void);

extern uint64_t rephist_total_alloc;
extern uint32_t rephist_total_num;
#ifdef TOR_UNIT_TESTS
extern int onion_handshakes_requested[MAX_ONION_HANDSHAKE_TYPE+1];
extern int onion_handshakes_assigned[MAX_ONION_HANDSHAKE_TYPE+1];
#endif

/**
 * Represents the type of a cell for padding accounting
 */
typedef enum padding_type_t {
    /** A RELAY_DROP cell */
    PADDING_TYPE_DROP,
    /** A CELL_PADDING cell */
    PADDING_TYPE_CELL,
    /** Total counts of padding and non-padding together */
    PADDING_TYPE_TOTAL,
    /** Total cell counts for all padding-enabled channels */
    PADDING_TYPE_ENABLED_TOTAL,
    /** CELL_PADDING counts for all padding-enabled channels */
    PADDING_TYPE_ENABLED_CELL
} padding_type_t;

/** The amount of time over which the padding cell counts were counted */
#define REPHIST_CELL_PADDING_COUNTS_INTERVAL (24*60*60)
void rep_hist_padding_count_read(padding_type_t type);
void rep_hist_padding_count_write(padding_type_t type);
char *rep_hist_get_padding_count_lines(void);
void rep_hist_reset_padding_counts(void);
void rep_hist_prep_published_padding_counts(time_t now);
void rep_hist_padding_count_timers(uint64_t num_timers);

#endif /* !defined(TOR_REPHIST_H) */
