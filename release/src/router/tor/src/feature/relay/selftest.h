/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file selftest.h
 * \brief Header file for selftest.c.
 **/

#ifndef TOR_SELFTEST_H
#define TOR_SELFTEST_H

struct or_options_t;
int check_whether_orport_reachable(const struct or_options_t *options);
int check_whether_dirport_reachable(const struct or_options_t *options);

void router_do_reachability_checks(int test_or, int test_dir);
void router_orport_found_reachable(void);
void router_dirport_found_reachable(void);
void router_perform_bandwidth_test(int num_circs, time_t now);

#endif /* !defined(TOR_SELFTEST_H) */
