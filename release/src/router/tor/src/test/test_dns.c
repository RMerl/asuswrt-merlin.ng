/* Copyright (c) 2015-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "core/or/or.h"
#include "test/test.h"

#define DNS_PRIVATE

#include "feature/relay/dns.h"
#include "core/mainloop/connection.h"
#include "core/or/connection_edge.h"
#include "feature/relay/router.h"

#include "core/or/edge_connection_st.h"
#include "core/or/or_circuit_st.h"
#include "app/config/or_options_st.h"
#include "app/config/config.h"

#include <event2/event.h>
#include <event2/dns.h>

#ifdef HAVE_EVDNS_BASE_GET_NAMESERVER_ADDR

static or_options_t options = {
  .ORPort_set = 1,
};

static const or_options_t *
mock_get_options(void)
{
  return &options;
}

static void
test_dns_configure_ns_fallback(void *arg)
{
  (void)arg;
  tor_addr_t *nameserver_addr = NULL;

  MOCK(get_options, mock_get_options);

  options.ServerDNSResolvConfFile = (char *)"no_such_file!!!";

  dns_init(); // calls configure_nameservers()

  tt_int_op(number_of_configured_nameservers(), OP_EQ, 1);

  nameserver_addr = configured_nameserver_address(0);

  tt_assert(tor_addr_family(nameserver_addr) == AF_INET);
  tt_assert(tor_addr_eq_ipv4h(nameserver_addr, 0x7f000001));

#ifndef _WIN32
  tor_free(nameserver_addr);

  options.ServerDNSResolvConfFile = (char *)"/dev/null";

  dns_init();

  tt_int_op(number_of_configured_nameservers(), OP_EQ, 1);

  nameserver_addr = configured_nameserver_address(0);

  tt_assert(tor_addr_family(nameserver_addr) == AF_INET);
  tt_assert(tor_addr_eq_ipv4h(nameserver_addr, 0x7f000001));
#endif /* !defined(_WIN32) */

  UNMOCK(get_options);

 done:
  tor_free(nameserver_addr);
  return;
}

#endif /* defined(HAVE_EVDNS_BASE_GET_NAMESERVER_ADDR) */

static void
test_dns_clip_ttl(void *arg)
{
  (void)arg;

  uint32_t ttl_mid = MIN_DNS_TTL / 2 + MAX_DNS_TTL / 2;

  tt_int_op(clip_dns_ttl(MIN_DNS_TTL - 1),OP_EQ,MIN_DNS_TTL);
  tt_int_op(clip_dns_ttl(ttl_mid),OP_EQ,MAX_DNS_TTL);
  tt_int_op(clip_dns_ttl(MAX_DNS_TTL + 1),OP_EQ,MAX_DNS_TTL);

  done:
  return;
}

static int resolve_retval = 0;
static int resolve_made_conn_pending = 0;
static char *resolved_name = NULL;
static cached_resolve_t *cache_entry_mock = NULL;

static int n_fake_impl = 0;

static int dns_resolve_dns_resolve_impl(edge_connection_t *exitconn,
                     int is_resolve, or_circuit_t *oncirc,
                     char **hostname_out, int *made_connection_pending_out,
                     cached_resolve_t **resolve_out);
ATTR_UNUSED static int dns_resolve_dns_resolve_impl_called = 0;

/** This will be our configurable substitute for <b>dns_resolve_impl</b> in
 * dns.c. It will return <b>resolve_retval</b>,
 * and set <b>resolve_made_conn_pending</b> to
 * <b>made_connection_pending_out</b>. It will set <b>hostname_out</b>
 * to a duplicate of <b>resolved_name</b> and it will set <b>resolve_out</b>
 * to <b>cache_entry</b>. Lastly, it will increment <b>n_fake_impl</b< by
 * 1.
 */
static int
dns_resolve_dns_resolve_impl(edge_connection_t *exitconn, int is_resolve,
                     or_circuit_t *oncirc, char **hostname_out,
                     int *made_connection_pending_out,
                     cached_resolve_t **resolve_out)
{
  (void)oncirc;
  (void)exitconn;
  (void)is_resolve;

  if (made_connection_pending_out)
    *made_connection_pending_out = resolve_made_conn_pending;

  if (hostname_out && resolved_name)
    *hostname_out = tor_strdup(resolved_name);

  if (resolve_out && cache_entry_mock)
    *resolve_out = cache_entry_mock;

  n_fake_impl++;

  return resolve_retval;
}

static edge_connection_t *conn_for_resolved_cell = NULL;

static int n_send_resolved_cell_replacement = 0;
static uint8_t last_answer_type = 0;
static cached_resolve_t *last_resolved;

static void
dns_resolve_send_resolved_cell(edge_connection_t *conn, uint8_t answer_type,
                       const cached_resolve_t *resolved)
{
  conn_for_resolved_cell = conn;

  last_answer_type = answer_type;
  last_resolved = (cached_resolve_t *)resolved;

  n_send_resolved_cell_replacement++;
}

static int n_send_resolved_hostname_cell_replacement = 0;

static char *last_resolved_hostname = NULL;

static void
dns_resolve_send_resolved_hostname_cell(edge_connection_t *conn,
                                const char *hostname)
{
  conn_for_resolved_cell = conn;

  tor_free(last_resolved_hostname);
  last_resolved_hostname = tor_strdup(hostname);

  n_send_resolved_hostname_cell_replacement++;
}

static int n_dns_cancel_pending_resolve_replacement = 0;

static void
dns_resolve_dns_cancel_pending_resolve(const char *address)
{
  (void) address;
  n_dns_cancel_pending_resolve_replacement++;
}

static int n_connection_free = 0;
static connection_t *last_freed_conn = NULL;

static void
dns_resolve_connection_free_(connection_t *conn)
{
   n_connection_free++;

   last_freed_conn = conn;
}

static void
test_dns_resolve(void *arg)
{
  (void) arg;
  int retval;
  int prev_n_send_resolved_hostname_cell_replacement;
  int prev_n_send_resolved_cell_replacement;
  int prev_n_connection_free;
  cached_resolve_t *fake_resolved = tor_malloc(sizeof(cached_resolve_t));
  edge_connection_t *exitconn = tor_malloc(sizeof(edge_connection_t));
  edge_connection_t *nextconn = tor_malloc(sizeof(edge_connection_t));

  or_circuit_t *on_circuit = tor_malloc(sizeof(or_circuit_t));
  memset(on_circuit,0,sizeof(or_circuit_t));
  on_circuit->base_.magic = OR_CIRCUIT_MAGIC;

  memset(fake_resolved,0,sizeof(cached_resolve_t));
  memset(exitconn,0,sizeof(edge_connection_t));
  memset(nextconn,0,sizeof(edge_connection_t));

  MOCK(dns_resolve_impl,
       dns_resolve_dns_resolve_impl);
  MOCK(send_resolved_cell,
       dns_resolve_send_resolved_cell);
  MOCK(send_resolved_hostname_cell,
       dns_resolve_send_resolved_hostname_cell);

  /*
   * CASE 1: dns_resolve_impl returns 1 and sets a hostname. purpose is
   * EXIT_PURPOSE_RESOLVE.
   *
   * We want dns_resolve() to call send_resolved_hostname_cell() for a
   * given exit connection (represented by edge_connection_t object)
   * with a hostname it received from _impl.
   */

  prev_n_send_resolved_hostname_cell_replacement =
  n_send_resolved_hostname_cell_replacement;

  exitconn->base_.purpose = EXIT_PURPOSE_RESOLVE;
  exitconn->on_circuit = &(on_circuit->base_);

  resolve_retval = 1;
  resolved_name = tor_strdup("www.torproject.org");

  retval = dns_resolve(exitconn);

  tt_int_op(retval,OP_EQ,1);
  tt_str_op(resolved_name,OP_EQ,last_resolved_hostname);
  tt_assert(conn_for_resolved_cell == exitconn);
  tt_int_op(n_send_resolved_hostname_cell_replacement,OP_EQ,
            prev_n_send_resolved_hostname_cell_replacement + 1);
  tt_assert(exitconn->on_circuit == NULL);

  tor_free(last_resolved_hostname);
  // implies last_resolved_hostname = NULL;

  /* CASE 2: dns_resolve_impl returns 1, but does not set hostname.
   * Instead, it yields cached_resolve_t object.
   *
   * We want dns_resolve to call send_resolved_cell on exitconn with
   * RESOLVED_TYPE_AUTO and the cached_resolve_t object from _impl.
   */

  tor_free(resolved_name);
  resolved_name = NULL;

  exitconn->on_circuit = &(on_circuit->base_);

  cache_entry_mock = fake_resolved;

  prev_n_send_resolved_cell_replacement =
  n_send_resolved_cell_replacement;

  retval = dns_resolve(exitconn);

  tt_int_op(retval,OP_EQ,1);
  tt_assert(conn_for_resolved_cell == exitconn);
  tt_int_op(n_send_resolved_cell_replacement,OP_EQ,
            prev_n_send_resolved_cell_replacement + 1);
  tt_assert(last_resolved == fake_resolved);
  tt_int_op(last_answer_type,OP_EQ,0xff);
  tt_assert(exitconn->on_circuit == NULL);

  /* CASE 3: The purpose of exit connection is not EXIT_PURPOSE_RESOLVE
   * and _impl returns 1.
   *
   * We want dns_resolve to prepend exitconn to n_streams linked list.
   * We don't want it to send any cells about hostname being resolved.
   */

  exitconn->base_.purpose = EXIT_PURPOSE_CONNECT;
  exitconn->on_circuit = &(on_circuit->base_);

  on_circuit->n_streams = nextconn;

  prev_n_send_resolved_cell_replacement =
  n_send_resolved_cell_replacement;

  prev_n_send_resolved_hostname_cell_replacement =
  n_send_resolved_hostname_cell_replacement;

  retval = dns_resolve(exitconn);

  tt_int_op(retval,OP_EQ,1);
  tt_assert(on_circuit->n_streams == exitconn);
  tt_assert(exitconn->next_stream == nextconn);
  tt_int_op(prev_n_send_resolved_cell_replacement,OP_EQ,
            n_send_resolved_cell_replacement);
  tt_int_op(prev_n_send_resolved_hostname_cell_replacement,OP_EQ,
            n_send_resolved_hostname_cell_replacement);

  /* CASE 4: _impl returns 0.
   *
   * We want dns_resolve() to set exitconn state to
   * EXIT_CONN_STATE_RESOLVING and prepend exitconn to resolving_streams
   * linked list.
   */

  exitconn->on_circuit = &(on_circuit->base_);

  resolve_retval = 0;

  exitconn->next_stream = NULL;
  on_circuit->resolving_streams = nextconn;

  retval = dns_resolve(exitconn);

  tt_int_op(retval,OP_EQ,0);
  tt_int_op(exitconn->base_.state,OP_EQ,EXIT_CONN_STATE_RESOLVING);
  tt_assert(on_circuit->resolving_streams == exitconn);
  tt_assert(exitconn->next_stream == nextconn);

  /* CASE 5: _impl returns -1 when purpose of exitconn is
   * EXIT_PURPOSE_RESOLVE. We want dns_resolve to call send_resolved_cell
   * on exitconn with type being RESOLVED_TYPE_ERROR.
   */

  MOCK(dns_cancel_pending_resolve,
       dns_resolve_dns_cancel_pending_resolve);
  MOCK(connection_free_,
       dns_resolve_connection_free_);

  exitconn->on_circuit = &(on_circuit->base_);
  exitconn->base_.purpose = EXIT_PURPOSE_RESOLVE;

  resolve_retval = -1;

  prev_n_send_resolved_cell_replacement =
  n_send_resolved_cell_replacement;

  prev_n_connection_free = n_connection_free;

  retval = dns_resolve(exitconn);

  tt_int_op(retval,OP_EQ,-1);
  tt_int_op(n_send_resolved_cell_replacement,OP_EQ,
            prev_n_send_resolved_cell_replacement + 1);
  tt_int_op(last_answer_type,OP_EQ,RESOLVED_TYPE_ERROR);
  tt_int_op(n_dns_cancel_pending_resolve_replacement,OP_EQ,1);
  tt_int_op(n_connection_free,OP_EQ,prev_n_connection_free + 1);
  tt_assert(last_freed_conn == TO_CONN(exitconn));

  done:
  UNMOCK(dns_resolve_impl);
  UNMOCK(send_resolved_cell);
  UNMOCK(send_resolved_hostname_cell);
  UNMOCK(dns_cancel_pending_resolve);
  UNMOCK(connection_free_);
  tor_free(on_circuit);
  tor_free(exitconn);
  tor_free(nextconn);
  tor_free(resolved_name);
  tor_free(fake_resolved);
  tor_free(last_resolved_hostname);
  return;
}

/** Create an <b>edge_connection_t</b> instance that is considered a
 * valid exit connection by asserts in dns_resolve_impl.
 */
static edge_connection_t *
create_valid_exitconn(void)
{
  edge_connection_t *exitconn = tor_malloc_zero(sizeof(edge_connection_t));
  TO_CONN(exitconn)->type = CONN_TYPE_EXIT;
  TO_CONN(exitconn)->magic = EDGE_CONNECTION_MAGIC;
  TO_CONN(exitconn)->purpose = EXIT_PURPOSE_RESOLVE;
  TO_CONN(exitconn)->state = EXIT_CONN_STATE_RESOLVING;
  exitconn->base_.s = TOR_INVALID_SOCKET;

  return exitconn;
}

/*
 * Given that <b>exitconn->base_.address</b> is IP address string, we
 * want dns_resolve_impl() to parse it and store in
 * <b>exitconn->base_.addr</b>. We expect dns_resolve_impl to return 1.
 * Lastly, we want it to set the TTL value to default one for DNS queries.
 */

static void
test_dns_impl_addr_is_ip(void *arg)
{
  int retval;
  int made_pending;
  const tor_addr_t *resolved_addr;
  tor_addr_t addr_to_compare;

  (void)arg;

  tor_addr_parse(&addr_to_compare, "8.8.8.8");

  or_circuit_t *on_circ = tor_malloc_zero(sizeof(or_circuit_t));

  edge_connection_t *exitconn = create_valid_exitconn();

  TO_CONN(exitconn)->address = tor_strdup("8.8.8.8");

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            NULL);

  resolved_addr = &(exitconn->base_.addr);

  tt_int_op(retval,OP_EQ,1);
  tt_assert(tor_addr_eq(resolved_addr, (const tor_addr_t *)&addr_to_compare));
  tt_int_op(exitconn->address_ttl,OP_EQ,DEFAULT_DNS_TTL);

  done:
  tor_free(on_circ);
  tor_free(TO_CONN(exitconn)->address);
  tor_free(exitconn);
  return;
}

/** Given that Tor instance is not configured as an exit node, we want
 * dns_resolve_impl() to fail with return value -1.
 */
static int
dns_impl_non_exit_router_my_exit_policy_is_reject_star(void)
{
  return 1;
}

static void
test_dns_impl_non_exit(void *arg)
{
  int retval;
  int made_pending;

  edge_connection_t *exitconn = create_valid_exitconn();
  or_circuit_t *on_circ = tor_malloc_zero(sizeof(or_circuit_t));

  (void)arg;

  TO_CONN(exitconn)->address = tor_strdup("torproject.org");

  MOCK(router_my_exit_policy_is_reject_star,
       dns_impl_non_exit_router_my_exit_policy_is_reject_star);

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            NULL);

  tt_int_op(retval,OP_EQ,-1);

  done:
  tor_free(TO_CONN(exitconn)->address);
  tor_free(exitconn);
  tor_free(on_circ);
  UNMOCK(router_my_exit_policy_is_reject_star);
  return;
}

/** Given that address is not a valid destination (as judged by
 * address_is_invalid_destination() function), we want dns_resolve_impl()
 * function to fail with return value -1.
 */

static int
dns_impl_addr_is_invalid_dest_router_my_exit_policy_is_reject_star(void)
{
  return 0;
}

static void
test_dns_impl_addr_is_invalid_dest(void *arg)
{
  int retval;
  int made_pending;

  edge_connection_t *exitconn = create_valid_exitconn();
  or_circuit_t *on_circ = tor_malloc_zero(sizeof(or_circuit_t));

  (void)arg;

  MOCK(router_my_exit_policy_is_reject_star,
       dns_impl_addr_is_invalid_dest_router_my_exit_policy_is_reject_star);

  TO_CONN(exitconn)->address = tor_strdup("invalid#@!.org");

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            NULL);

  tt_int_op(retval,OP_EQ,-1);

  done:
  UNMOCK(router_my_exit_policy_is_reject_star);
  tor_free(TO_CONN(exitconn)->address);
  tor_free(exitconn);
  tor_free(on_circ);
  return;
}

/** Given that address is a malformed PTR name, we want dns_resolve_impl to
 * fail.
 */

static int
dns_impl_malformed_ptr_router_my_exit_policy_is_reject_star(void)
{
  return 0;
}

static void
test_dns_impl_malformed_ptr(void *arg)
{
  int retval;
  int made_pending;

  edge_connection_t *exitconn = create_valid_exitconn();
  or_circuit_t *on_circ = tor_malloc_zero(sizeof(or_circuit_t));

  (void)arg;

  TO_CONN(exitconn)->address = tor_strdup("1.0.0.127.in-addr.arpa");

  MOCK(router_my_exit_policy_is_reject_star,
       dns_impl_malformed_ptr_router_my_exit_policy_is_reject_star);

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            NULL);

  tt_int_op(retval,OP_EQ,-1);

  tor_free(TO_CONN(exitconn)->address);

  TO_CONN(exitconn)->address =
  tor_strdup("z01234567890123456789.in-addr.arpa");

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            NULL);

  tt_int_op(retval,OP_EQ,-1);

  done:
  UNMOCK(router_my_exit_policy_is_reject_star);
  tor_free(TO_CONN(exitconn)->address);
  tor_free(exitconn);
  tor_free(on_circ);
  return;
}

/* Given that there is already a pending resolve for the given address,
 * we want dns_resolve_impl to append our exit connection to list
 * of pending connections for the pending DNS request and return 0.
 */

static int
dns_impl_cache_hit_pending_router_my_exit_policy_is_reject_star(void)
{
  return 0;
}

static void
test_dns_impl_cache_hit_pending(void *arg)
{
  int retval;
  int made_pending = 0;

  pending_connection_t *pending_conn = NULL;

  edge_connection_t *exitconn = create_valid_exitconn();
  or_circuit_t *on_circ = tor_malloc_zero(sizeof(or_circuit_t));

  cached_resolve_t *cache_entry = tor_malloc_zero(sizeof(cached_resolve_t));
  cache_entry->magic = CACHED_RESOLVE_MAGIC;
  cache_entry->state = CACHE_STATE_PENDING;
  cache_entry->minheap_idx = -1;
  cache_entry->expire = time(NULL) + 60 * 60;

  (void)arg;

  TO_CONN(exitconn)->address = tor_strdup("torproject.org");

  strlcpy(cache_entry->address, TO_CONN(exitconn)->address,
          sizeof(cache_entry->address));

  MOCK(router_my_exit_policy_is_reject_star,
       dns_impl_cache_hit_pending_router_my_exit_policy_is_reject_star);

  dns_init();

  dns_insert_cache_entry(cache_entry);

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            NULL);

  tt_int_op(retval,OP_EQ,0);
  tt_int_op(made_pending,OP_EQ,1);

  pending_conn = cache_entry->pending_connections;

  tt_assert(pending_conn != NULL);
  tt_assert(pending_conn->conn == exitconn);

  done:
  UNMOCK(router_my_exit_policy_is_reject_star);
  tor_free(on_circ);
  tor_free(TO_CONN(exitconn)->address);
  tor_free(cache_entry->pending_connections);
  tor_free(cache_entry);
  tor_free(exitconn);
  return;
}

/* Given that a finished DNS resolve is available in our cache, we want
 * dns_resolve_impl() return it to called via resolve_out and pass the
 * handling to set_exitconn_info_from_resolve function.
 */
static int
dns_impl_cache_hit_cached_router_my_exit_policy_is_reject_star(void)
{
  return 0;
}

static edge_connection_t *last_exitconn = NULL;
static cached_resolve_t *last_resolve = NULL;

static int
dns_impl_cache_hit_cached_set_exitconn_info_from_resolve(
                                   edge_connection_t *exitconn,
                                   const cached_resolve_t *resolve,
                                   char **hostname_out)
{
  last_exitconn = exitconn;
  last_resolve = (cached_resolve_t *)resolve;

  (void)hostname_out;

  return 0;
}

static void
test_dns_impl_cache_hit_cached(void *arg)
{
  int retval;
  int made_pending = 0;

  edge_connection_t *exitconn = create_valid_exitconn();
  or_circuit_t *on_circ = tor_malloc_zero(sizeof(or_circuit_t));

  cached_resolve_t *resolve_out = NULL;

  cached_resolve_t *cache_entry = tor_malloc_zero(sizeof(cached_resolve_t));
  cache_entry->magic = CACHED_RESOLVE_MAGIC;
  cache_entry->state = CACHE_STATE_CACHED;
  cache_entry->minheap_idx = -1;
  cache_entry->expire = time(NULL) + 60 * 60;

  (void)arg;

  TO_CONN(exitconn)->address = tor_strdup("torproject.org");

  strlcpy(cache_entry->address, TO_CONN(exitconn)->address,
          sizeof(cache_entry->address));

  MOCK(router_my_exit_policy_is_reject_star,
       dns_impl_cache_hit_cached_router_my_exit_policy_is_reject_star);
  MOCK(set_exitconn_info_from_resolve,
       dns_impl_cache_hit_cached_set_exitconn_info_from_resolve);

  dns_init();

  dns_insert_cache_entry(cache_entry);

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            &resolve_out);

  tt_int_op(retval,OP_EQ,0);
  tt_int_op(made_pending,OP_EQ,0);
  tt_assert(resolve_out == cache_entry);

  tt_assert(last_exitconn == exitconn);
  tt_assert(last_resolve == cache_entry);

  done:
  UNMOCK(router_my_exit_policy_is_reject_star);
  UNMOCK(set_exitconn_info_from_resolve);
  tor_free(on_circ);
  tor_free(TO_CONN(exitconn)->address);
  tor_free(cache_entry->pending_connections);
  tor_free(cache_entry);
  return;
}

/* Given that there are neither pending nor pre-cached resolve for a given
 * address, we want dns_resolve_impl() to create a new cached_resolve_t
 * object, mark it as pending, insert it into the cache, attach the exit
 * connection to list of pending connections and call launch_resolve()
 * with the cached_resolve_t object it created.
 */
static int
dns_impl_cache_miss_router_my_exit_policy_is_reject_star(void)
{
  return 0;
}

static cached_resolve_t *last_launched_resolve = NULL;

static int
dns_impl_cache_miss_launch_resolve(cached_resolve_t *resolve)
{
  last_launched_resolve = resolve;

  return 0;
}

static void
test_dns_impl_cache_miss(void *arg)
{
  int retval;
  int made_pending = 0;

  pending_connection_t *pending_conn = NULL;

  edge_connection_t *exitconn = create_valid_exitconn();
  or_circuit_t *on_circ = tor_malloc_zero(sizeof(or_circuit_t));

  cached_resolve_t *cache_entry = NULL;
  cached_resolve_t query;

  (void)arg;

  TO_CONN(exitconn)->address = tor_strdup("torproject.org");

  strlcpy(query.address, TO_CONN(exitconn)->address, sizeof(query.address));

  MOCK(router_my_exit_policy_is_reject_star,
       dns_impl_cache_miss_router_my_exit_policy_is_reject_star);
  MOCK(launch_resolve,
       dns_impl_cache_miss_launch_resolve);

  dns_init();

  retval = dns_resolve_impl(exitconn, 1, on_circ, NULL, &made_pending,
                            NULL);

  tt_int_op(retval,OP_EQ,0);
  tt_int_op(made_pending,OP_EQ,1);

  cache_entry = dns_get_cache_entry(&query);

  tt_assert(cache_entry);

  pending_conn = cache_entry->pending_connections;

  tt_assert(pending_conn != NULL);
  tt_assert(pending_conn->conn == exitconn);

  tt_assert(last_launched_resolve == cache_entry);
  tt_str_op(cache_entry->address,OP_EQ,TO_CONN(exitconn)->address);

  done:
  UNMOCK(router_my_exit_policy_is_reject_star);
  UNMOCK(launch_resolve);
  tor_free(on_circ);
  tor_free(TO_CONN(exitconn)->address);
  if (cache_entry)
    tor_free(cache_entry->pending_connections);
  tor_free(cache_entry);
  tor_free(exitconn);
  return;
}

struct testcase_t dns_tests[] = {
#ifdef HAVE_EVDNS_BASE_GET_NAMESERVER_ADDR
   { "configure_ns_fallback", test_dns_configure_ns_fallback,
     TT_FORK, NULL, NULL },
#endif
   { "clip_ttl", test_dns_clip_ttl, TT_FORK, NULL, NULL },
   { "resolve", test_dns_resolve, TT_FORK, NULL, NULL },
   { "impl_addr_is_ip", test_dns_impl_addr_is_ip, TT_FORK, NULL, NULL },
   { "impl_non_exit", test_dns_impl_non_exit, TT_FORK, NULL, NULL },
   { "impl_addr_is_invalid_dest", test_dns_impl_addr_is_invalid_dest,
     TT_FORK, NULL, NULL },
   { "impl_malformed_ptr", test_dns_impl_malformed_ptr, TT_FORK, NULL, NULL },
   { "impl_cache_hit_pending", test_dns_impl_cache_hit_pending,
     TT_FORK, NULL, NULL },
   { "impl_cache_hit_cached", test_dns_impl_cache_hit_cached,
     TT_FORK, NULL, NULL },
   { "impl_cache_miss", test_dns_impl_cache_miss, TT_FORK, NULL, NULL },
   END_OF_TESTCASES
};
