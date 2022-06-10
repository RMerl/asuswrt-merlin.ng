/* Copyright (c) 2014-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CIRCUITBUILD_PRIVATE
#define RELAY_PRIVATE
#define BWHIST_PRIVATE
#include "core/or/or.h"
#include "core/or/circuitbuild.h"
#include "core/or/circuitlist.h"
#include "core/or/channeltls.h"
#include "feature/stats/bwhist.h"
#include "core/or/relay.h"
#include "lib/container/order.h"
#include "lib/encoding/confline.h"
/* For init/free stuff */
#include "core/or/scheduler.h"

#include "core/or/cell_st.h"
#include "core/or/or_circuit_st.h"

#define RESOLVE_ADDR_PRIVATE
#include "feature/nodelist/dirlist.h"
#include "feature/relay/relay_find_addr.h"
#include "feature/relay/routermode.h"
#include "feature/dirclient/dir_server_st.h"

#define CONFIG_PRIVATE
#include "app/config/config.h"
#include "app/config/resolve_addr.h"

/* Test suite stuff */
#include "test/test.h"
#include "test/fakechans.h"
#include "test/fakecircs.h"

static void test_relay_append_cell_to_circuit_queue(void *arg);

static int
mock_server_mode_true(const or_options_t *options)
{
  (void) options;
  return 1;
}

static void
assert_circuit_ok_mock(const circuit_t *c)
{
  (void) c;
  return;
}

static void
test_relay_close_circuit(void *arg)
{
  channel_t *nchan = NULL, *pchan = NULL;
  or_circuit_t *orcirc = NULL;
  cell_t *cell = NULL;
  int old_count, new_count;

  (void)arg;

  /* Make fake channels to be nchan and pchan for the circuit */
  nchan = new_fake_channel();
  tt_assert(nchan);

  pchan = new_fake_channel();
  tt_assert(pchan);

  /* Make a fake orcirc */
  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circuitmux_attach_circuit(nchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_OUT);
  circuitmux_attach_circuit(pchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_IN);

  /* Make a cell */
  cell = tor_malloc_zero(sizeof(cell_t));
  make_fake_cell(cell);

  MOCK(scheduler_channel_has_waiting_cells,
       scheduler_channel_has_waiting_cells_mock);
  MOCK(assert_circuit_ok,
       assert_circuit_ok_mock);

  /* Append it */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), nchan, cell,
                               CELL_DIRECTION_OUT, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  /* Now try the reverse direction */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), pchan, cell,
                               CELL_DIRECTION_IN, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  /* Ensure our write totals are 0 */
  tt_u64_op(find_largest_max(write_array, 86400), OP_EQ, 0);

  /* Mark the circuit for close */
  circuit_mark_for_close(TO_CIRCUIT(orcirc), 0);

  /* Check our write totals. */
  advance_obs(write_array);
  commit_max(write_array);
  /* Check for two cells plus overhead */
  tt_u64_op(find_largest_max(write_array, 86400), OP_EQ,
                             2*(get_cell_network_size(nchan->wide_circ_ids)
                                +TLS_PER_CELL_OVERHEAD));

  UNMOCK(scheduler_channel_has_waiting_cells);

  /* Get rid of the fake channels */
  MOCK(scheduler_release_channel, scheduler_release_channel_mock);
  channel_mark_for_close(nchan);
  channel_mark_for_close(pchan);
  UNMOCK(scheduler_release_channel);

  /* Shut down channels */
  channel_free_all();

 done:
  tor_free(cell);
  if (orcirc) {
    circuitmux_detach_circuit(nchan->cmux, TO_CIRCUIT(orcirc));
    circuitmux_detach_circuit(pchan->cmux, TO_CIRCUIT(orcirc));
    cell_queue_clear(&orcirc->base_.n_chan_cells);
    cell_queue_clear(&orcirc->p_chan_cells);
  }
  free_fake_orcirc(orcirc);
  free_fake_channel(nchan);
  free_fake_channel(pchan);
  UNMOCK(assert_circuit_ok);

  return;
}

static void
test_relay_append_cell_to_circuit_queue(void *arg)
{
  channel_t *nchan = NULL, *pchan = NULL;
  or_circuit_t *orcirc = NULL;
  cell_t *cell = NULL;
  int old_count, new_count;

  (void)arg;

  /* Make fake channels to be nchan and pchan for the circuit */
  nchan = new_fake_channel();
  tt_assert(nchan);

  pchan = new_fake_channel();
  tt_assert(pchan);

  /* Make a fake orcirc */
  orcirc = new_fake_orcirc(nchan, pchan);
  tt_assert(orcirc);
  circuitmux_attach_circuit(nchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_OUT);
  circuitmux_attach_circuit(pchan->cmux, TO_CIRCUIT(orcirc),
                            CELL_DIRECTION_IN);

  /* Make a cell */
  cell = tor_malloc_zero(sizeof(cell_t));
  make_fake_cell(cell);

  MOCK(scheduler_channel_has_waiting_cells,
       scheduler_channel_has_waiting_cells_mock);

  /* Append it */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), nchan, cell,
                               CELL_DIRECTION_OUT, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  /* Now try the reverse direction */
  old_count = get_mock_scheduler_has_waiting_cells_count();
  append_cell_to_circuit_queue(TO_CIRCUIT(orcirc), pchan, cell,
                               CELL_DIRECTION_IN, 0);
  new_count = get_mock_scheduler_has_waiting_cells_count();
  tt_int_op(new_count, OP_EQ, old_count + 1);

  UNMOCK(scheduler_channel_has_waiting_cells);

  /* Get rid of the fake channels */
  MOCK(scheduler_release_channel, scheduler_release_channel_mock);
  channel_mark_for_close(nchan);
  channel_mark_for_close(pchan);
  UNMOCK(scheduler_release_channel);

  /* Shut down channels */
  channel_free_all();

 done:
  tor_free(cell);
  if (orcirc) {
    circuitmux_detach_circuit(nchan->cmux, TO_CIRCUIT(orcirc));
    circuitmux_detach_circuit(pchan->cmux, TO_CIRCUIT(orcirc));
    cell_queue_clear(&orcirc->base_.n_chan_cells);
    cell_queue_clear(&orcirc->p_chan_cells);
  }
  free_fake_orcirc(orcirc);
  free_fake_channel(nchan);
  free_fake_channel(pchan);

  return;
}

static void
test_suggested_address(void *arg)
{
  int ret;
  const char *untrusted_id = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  dir_server_t *ds = NULL;
  tor_addr_t ipv4_addr, ipv6_addr, cache_addr;
  tor_addr_t trusted_addr, untrusted_addr;
  tor_addr_port_t trusted_ap_v6 = { .port = 443 };

  (void) arg;

  MOCK(server_mode, mock_server_mode_true);

  /* Unstrusted relay source. */
  ret = tor_addr_parse(&untrusted_addr, "8.8.8.8");
  tt_int_op(ret, OP_EQ, AF_INET);

  /* Add gabelmoo as a trusted directory authority. */
  ret = tor_addr_parse(&trusted_addr, "[2001:638:a000:4140::ffff:189]");
  tt_int_op(ret, OP_EQ, AF_INET6);
  tor_addr_copy(&trusted_ap_v6.addr, &trusted_addr);

  ds = trusted_dir_server_new("gabelmoo", "131.188.40.189", 80, 443,
                              &trusted_ap_v6,
                              "F2044413DAC2E02E3D6BCF4735A19BCA1DE97281",
                              "ED03BB616EB2F60BEC80151114BB25CEF515B226",
                              V3_DIRINFO, 1.0);
  tt_assert(ds);
  dir_server_add(ds);

  /* 1. Valid IPv4 from a trusted authority (gabelmoo). */
  ret = tor_addr_parse(&ipv4_addr, "1.2.3.4");
  relay_address_new_suggestion(&ipv4_addr, &ds->ipv4_addr, ds->digest);
  resolved_addr_get_suggested(AF_INET, &cache_addr);
  tt_assert(tor_addr_eq(&cache_addr, &ipv4_addr));
  resolve_addr_reset_suggested(AF_INET);

  /* 2. Valid IPv6 from a trusted authority (gabelmoo). */
  ret = tor_addr_parse(&ipv6_addr, "[4242::4242]");
  relay_address_new_suggestion(&ipv6_addr, &ds->ipv6_addr, ds->digest);
  resolved_addr_get_suggested(AF_INET6, &cache_addr);
  tt_assert(tor_addr_eq(&cache_addr, &ipv6_addr));
  resolve_addr_reset_suggested(AF_INET6);

  /* 3. Valid IPv4 but untrusted source. */
  ret = tor_addr_parse(&ipv4_addr, "1.2.3.4");
  relay_address_new_suggestion(&ipv4_addr, &untrusted_addr, untrusted_id);
  resolved_addr_get_suggested(AF_INET, &cache_addr);
  tt_assert(tor_addr_is_unspec(&cache_addr));

  /* 4. Valid IPv6 but untrusted source. */
  ret = tor_addr_parse(&ipv6_addr, "[4242::4242]");
  relay_address_new_suggestion(&ipv6_addr, &untrusted_addr, untrusted_id);
  resolved_addr_get_suggested(AF_INET6, &cache_addr);
  tt_assert(tor_addr_is_unspec(&cache_addr));

  /* 5. Internal IPv4 from a trusted authority (gabelmoo). */
  ret = tor_addr_parse(&ipv4_addr, "127.0.0.1");
  relay_address_new_suggestion(&ipv4_addr, &ds->ipv4_addr, ds->digest);
  resolved_addr_get_suggested(AF_INET, &cache_addr);
  tt_assert(tor_addr_is_unspec(&cache_addr));

  /* 6. Internal IPv6 from a trusted authority (gabelmoo). */
  ret = tor_addr_parse(&ipv6_addr, "[::1]");
  relay_address_new_suggestion(&ipv6_addr, &ds->ipv6_addr, ds->digest);
  resolved_addr_get_suggested(AF_INET6, &cache_addr);
  tt_assert(tor_addr_is_unspec(&cache_addr));

  /* 7. IPv4 from a trusted authority (gabelmoo). */
  relay_address_new_suggestion(&ds->ipv4_addr, &ds->ipv4_addr, ds->digest);
  resolved_addr_get_suggested(AF_INET, &cache_addr);
  tt_assert(tor_addr_is_unspec(&cache_addr));

  /* 8. IPv6 from a trusted authority (gabelmoo). */
  relay_address_new_suggestion(&ds->ipv6_addr, &ds->ipv6_addr, ds->digest);
  resolved_addr_get_suggested(AF_INET6, &cache_addr);
  tt_assert(tor_addr_is_unspec(&cache_addr));

 done:
  dirlist_free_all();

  UNMOCK(server_mode);
}

static void
test_find_addr_to_publish(void *arg)
{
  int family;
  bool ret;
  tor_addr_t ipv4_addr, ipv6_addr, cache_addr;
  or_options_t *options;

  (void) arg;

  options = options_new();
  options_init(options);

  /* Populate our resolved cache with a valid IPv4 and IPv6. */
  family = tor_addr_parse(&ipv4_addr, "1.2.3.4");
  tt_int_op(family, OP_EQ, AF_INET);
  resolved_addr_set_last(&ipv4_addr, RESOLVED_ADDR_CONFIGURED, NULL);
  resolved_addr_get_last(AF_INET, &cache_addr);
  tt_assert(tor_addr_eq(&ipv4_addr, &cache_addr));

  family = tor_addr_parse(&ipv6_addr, "[4242::4242]");
  tt_int_op(family, OP_EQ, AF_INET6);
  resolved_addr_set_last(&ipv6_addr, RESOLVED_ADDR_CONFIGURED, NULL);
  resolved_addr_get_last(AF_INET6, &cache_addr);
  tt_assert(tor_addr_eq(&ipv6_addr, &cache_addr));

  /* Setup ORPort config. */
  {
    int n, w, r;
    char *msg = NULL;

    config_line_append(&options->ORPort_lines, "ORPort", "9001");

    r = parse_ports(options, 0, &msg, &n, &w);
    tt_int_op(r, OP_EQ, 0);
  }

  /* 1. Address located in the resolved cache. */
  ret = relay_find_addr_to_publish(options, AF_INET,
                                   RELAY_FIND_ADDR_CACHE_ONLY, &cache_addr);
  tt_assert(ret);
  tt_assert(tor_addr_eq(&ipv4_addr, &cache_addr));

  ret = relay_find_addr_to_publish(options, AF_INET6,
                                   RELAY_FIND_ADDR_CACHE_ONLY, &cache_addr);
  tt_assert(ret);
  tt_assert(tor_addr_eq(&ipv6_addr, &cache_addr));
  resolved_addr_reset_last(AF_INET);
  resolved_addr_reset_last(AF_INET6);

  /* 2. No IP in the resolve cache, go to the suggested cache. We will ignore
   *    the find_my_address() code path because that is extensively tested in
   *    another unit tests. */
  resolved_addr_set_suggested(&ipv4_addr);
  ret = relay_find_addr_to_publish(options, AF_INET,
                                   RELAY_FIND_ADDR_CACHE_ONLY, &cache_addr);
  tt_assert(ret);
  tt_assert(tor_addr_eq(&ipv4_addr, &cache_addr));

  resolved_addr_set_suggested(&ipv6_addr);
  ret = relay_find_addr_to_publish(options, AF_INET6,
                                   RELAY_FIND_ADDR_CACHE_ONLY, &cache_addr);
  tt_assert(ret);
  tt_assert(tor_addr_eq(&ipv6_addr, &cache_addr));
  resolve_addr_reset_suggested(AF_INET);
  resolve_addr_reset_suggested(AF_INET6);

  /* 3. No IP anywhere. */
  ret = relay_find_addr_to_publish(options, AF_INET,
                                   RELAY_FIND_ADDR_CACHE_ONLY, &cache_addr);
  tt_assert(!ret);
  ret = relay_find_addr_to_publish(options, AF_INET6,
                                   RELAY_FIND_ADDR_CACHE_ONLY, &cache_addr);
  tt_assert(!ret);

 done:
  or_options_free(options);
}

struct testcase_t relay_tests[] = {
  { "append_cell_to_circuit_queue", test_relay_append_cell_to_circuit_queue,
    TT_FORK, NULL, NULL },
  { "close_circ_rephist", test_relay_close_circuit,
    TT_FORK, NULL, NULL },
  { "suggested_address", test_suggested_address,
    TT_FORK, NULL, NULL },
  { "find_addr_to_publish", test_find_addr_to_publish,
    TT_FORK, NULL, NULL },

  END_OF_TESTCASES
};
