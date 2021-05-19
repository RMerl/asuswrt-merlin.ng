/* Copyright (c) 2019-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_proto_haproxy.c
 * \brief Tests for our HAProxy protocol parser code
 */

#define PROTO_HAPROXY_PRIVATE

#include "test/test.h"
#include "core/proto/proto_haproxy.h"
#include "test/log_test_helpers.h"

static void
test_format_proxy_header_line(void *arg)
{
  tor_addr_t addr;
  tor_addr_port_t *addr_port = NULL;
  char *output = NULL;

  (void) arg;

  /* IPv4 address. */
  tor_addr_parse(&addr, "192.168.1.2");
  addr_port = tor_addr_port_new(&addr, 8000);
  output = haproxy_format_proxy_header_line(addr_port);

  tt_str_op(output, OP_EQ, "PROXY TCP4 0.0.0.0 192.168.1.2 0 8000\r\n");

  tor_free(addr_port);
  tor_free(output);

  /* IPv6 address. */
  tor_addr_parse(&addr, "123:45:6789::5005:11");
  addr_port = tor_addr_port_new(&addr, 8000);
  output = haproxy_format_proxy_header_line(addr_port);

  tt_str_op(output, OP_EQ, "PROXY TCP6 :: 123:45:6789::5005:11 0 8000\r\n");

  tor_free(addr_port);
  tor_free(output);

  /* UNIX socket address. */
  memset(&addr, 0, sizeof(addr));
  addr.family = AF_UNIX;
  addr_port = tor_addr_port_new(&addr, 8000);
  output = haproxy_format_proxy_header_line(addr_port);

  /* If it's not an IPv4 or IPv6 address, haproxy_format_proxy_header_line
   * must return NULL. */
  tt_ptr_op(output, OP_EQ, NULL);

  tor_free(addr_port);
  tor_free(output);

 done:
  tor_free(addr_port);
  tor_free(output);
}

struct testcase_t proto_haproxy_tests[] = {
  { "format_proxy_header_line", test_format_proxy_header_line, 0, NULL, NULL },

  END_OF_TESTCASES
};
