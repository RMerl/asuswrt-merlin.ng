/* Copyright (c) 2017-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_TEST_HELPERS_H
#define TOR_TEST_HELPERS_H

#define BUFFERS_PRIVATE

#include "core/or/or.h"
#include "tinytest.h"

const char *get_yesterday_date_str(void);

circuit_t * dummy_origin_circuit_new(int num_cells);

/* Number of descriptors contained in test_descriptors.txt. */
#define HELPER_NUMBER_OF_DESCRIPTORS 8

void helper_setup_fake_routerlist(void);

#define GET(path) "GET " path " HTTP/1.0\r\n\r\n"
void connection_write_to_buf_mock(const char *string, size_t len,
                                  connection_t *conn, int compressed);
char *buf_get_contents(buf_t *buf, size_t *sz_out);

int mock_tor_addr_lookup__fail_on_bad_addrs(const char *name,
                                            uint16_t family, tor_addr_t *out);

connection_t *test_conn_get_connection(uint8_t state,
                                       uint8_t type, uint8_t purpose);
or_options_t *helper_parse_options(const char *conf);

extern const char TEST_DESCRIPTORS[];

void *helper_setup_pubsub(const struct testcase_t *);
int helper_cleanup_pubsub(const struct testcase_t *, void *);

extern const struct testcase_setup_t helper_pubsub_setup;

#endif /* !defined(TOR_TEST_HELPERS_H) */

