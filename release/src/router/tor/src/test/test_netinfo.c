/* Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "core/or/or.h"
#include "trunnel/netinfo.h"
#include "test/test.h"

static void
test_netinfo_unsupported_addr(void *arg)
{
  const uint8_t wire_data[] =
    { // TIME
      0x00, 0x00, 0x00, 0x01,
      // OTHERADDR
      0x04, // ATYPE
      0x04, // ALEN
      0x08, 0x08, 0x08, 0x08, // AVAL
      0x01, // NMYADDR
      0x03, // ATYPE (unsupported)
      0x05, // ALEN
      'a', 'd', 'r', 'r', '!' // AVAL (unsupported)
    };

  (void)arg;

  netinfo_cell_t *parsed_cell = NULL;

  ssize_t parsed = netinfo_cell_parse(&parsed_cell, wire_data,
                  sizeof(wire_data));

  tt_assert(parsed == sizeof(wire_data));

  netinfo_addr_t *addr = netinfo_cell_get_my_addrs(parsed_cell, 0);
  tt_assert(addr);

  tt_int_op(3, OP_EQ, netinfo_addr_get_addr_type(addr));
  tt_int_op(5, OP_EQ, netinfo_addr_get_len(addr));

 done:
  netinfo_cell_free(parsed_cell);
}

struct testcase_t netinfo_tests[] = {
  { "unsupported_addr", test_netinfo_unsupported_addr, 0, NULL, NULL },
  END_OF_TESTCASES
};

