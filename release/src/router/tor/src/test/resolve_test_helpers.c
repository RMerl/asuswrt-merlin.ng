/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file resolve_test_helpers.c
 * @brief Helper functions for mocking libc's blocking hostname lookup
 *   facilities.
 **/

#define RESOLVE_PRIVATE
#include "orconfig.h"
#include "test/resolve_test_helpers.h"
#include "lib/net/address.h"
#include "lib/net/resolve.h"
#include "test/test.h"

#include <stdio.h>
#include <string.h>

/**
 * Mock replacement for our getaddrinfo/gethostbyname wrapper.
 **/
static int
replacement_host_lookup(const char *name, uint16_t family, tor_addr_t *addr)
{
  static const struct lookup_table_ent {
    const char *name;
    const char *ipv4;
    const char *ipv6;
  } entries[] = {
    { "localhost", "127.0.0.1", "::1" },
    { "torproject.org", "198.51.100.6", "2001:DB8::700" },
    { NULL, NULL, NULL },
  };

  int r = -1;

  for (unsigned i = 0; entries[i].name != NULL; ++i) {
    if (!strcasecmp(name, entries[i].name)) {
      if (family == AF_INET6) {
        int s = tor_addr_parse(addr, entries[i].ipv6);
        tt_int_op(s, OP_EQ, AF_INET6);
      } else {
        int s = tor_addr_parse(addr, entries[i].ipv4);
        tt_int_op(s, OP_EQ, AF_INET);
      }
      r = 0;
      break;
    }
  }

  log_debug(LD_GENERAL, "resolve(%s,%d) => %s",
            name, family, r == 0 ? fmt_addr(addr) : "-1");

  return r;
 done:
  return -1;
}

/**
 * Set up a mock replacement for our wrapper on libc's resolver code.
 *
 * According to our replacement, only "localhost" and "torproject.org"
 * are real addresses; everything else doesn't exist.
 *
 * Use this function to avoid using the DNS resolver during unit tests;
 * call unmock_hostname_resolver() when you're done.
 **/
void
mock_hostname_resolver(void)
{
  MOCK(tor_addr_lookup_host_impl, replacement_host_lookup);
}

/**
 * Unmock our wrappers for libc's blocking hostname resolver code.
 **/
void
unmock_hostname_resolver(void)
{
  UNMOCK(tor_addr_lookup_host_impl);
}
