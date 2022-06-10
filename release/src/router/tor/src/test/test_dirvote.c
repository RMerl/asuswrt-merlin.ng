/* Copyright (c) 2020-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file test_dirvote.c
 * \brief Unit tests for dirvote related functions
 */
#define DIRVOTE_PRIVATE

#include "core/or/or.h"
#include "feature/dirauth/dirvote.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/signed_descriptor_st.h"

#include "test/test.h"

/**
 * This struct holds the various information that are needed for router
 * comparison. Each router in the test function has one, and they are all
 * put in a global digestmap, router_properties
 */
typedef struct router_values_t {
  int is_running;
  int is_auth;
  int bw_kb;
  char digest[DIGEST_LEN];
} router_values_t;
/**
 * This typedef makes declaring digests easier and less verbose
 */
typedef char sha1_digest_t[DIGEST_LEN];

// Use of global variable is justified because the functions that have to be
// mocked take as arguments objects we have no control over
static digestmap_t *router_properties = NULL;
// Use of global variable is justified by its use in nodelist.c
// and is necessary to avoid memory leaks when mocking the
// function node_get_by_id
static node_t *running_node;
static node_t *non_running_node;

/* Allocate memory to the global variables that represent a running
 * and non-running node
 */
#define ALLOCATE_MOCK_NODES()                    \
  running_node = tor_malloc(sizeof(node_t));     \
  running_node->is_running = 1;                  \
  non_running_node = tor_malloc(sizeof(node_t)); \
  non_running_node->is_running = 0;

/* Free the memory allocated to the mock nodes */
#define FREE_MOCK_NODES() \
  tor_free(running_node); \
  tor_free(non_running_node);

static int
mock_router_digest_is_trusted(const char *digest, dirinfo_type_t type)
{
  (void)type;
  router_values_t *mock_status;
  mock_status = digestmap_get(router_properties, digest);
  if (!mock_status) {
    return -1;
  }
  return mock_status->is_auth;
}

static const node_t *
mock_node_get_by_id(const char *identity_digest)
{
  router_values_t *status;
  status = digestmap_get(router_properties, identity_digest);
  if (!status) {
    return NULL;
  }
  if (status->is_running)
    return running_node;
  else
    return non_running_node;
}

static uint32_t
mock_dirserv_get_bw(const routerinfo_t *ri)
{
  const char *digest = ri->cache_info.identity_digest;
  router_values_t *status;
  status = digestmap_get(router_properties, digest);
  if (!status) {
    return -1;
  }
  return status->bw_kb;
}

/** Generate a pointer to a router_values_t struct with the arguments as
 * field values, and return it
 * The returned pointer has to be freed by the caller.
 */
static router_values_t *
router_values_new(int running, int auth, int bw, char *digest)
{
  router_values_t *status = tor_malloc(sizeof(router_values_t));
  memcpy(status->digest, digest, sizeof(status->digest));
  status->is_running = running;
  status->bw_kb = bw;
  status->is_auth = auth;
  return status;
}

/** Given a router_values_t struct, generate a pointer to a routerinfo struct.
 * In the cache_info member, put the identity digest, and depending on
 * the family argument, fill the IPv4 or IPv6 address. Return the pointer.
 * The returned pointer has to be freed by the caller.
 */
static routerinfo_t *
routerinfo_new(router_values_t *status, int family, int addr)
{
  routerinfo_t *ri = tor_malloc(sizeof(routerinfo_t));
  signed_descriptor_t cache_info;
  memcpy(cache_info.identity_digest, status->digest,
         sizeof(cache_info.identity_digest));
  ri->cache_info = cache_info;
  tor_addr_t ipv6, ipv4;
  ipv6.family = family;
  ipv4.family = family;
  // Set the address of the other IP version to 0
  if (family == AF_INET) {
    ipv4.addr.in_addr.s_addr = addr;
    for (size_t i = 0; i < 16; i++) {
      ipv6.addr.in6_addr.s6_addr[i] = 0;
    }
  } else {
    for (size_t i = 0; i < 16; i++) {
      ipv6.addr.in6_addr.s6_addr[i] = addr;
    }
    ipv4.addr.in_addr.s_addr = 0;
  }
  ri->ipv6_addr = ipv6;
  ri->ipv4_addr = ipv4;
  return ri;
}

static void
test_dirvote_compare_routerinfo_usefulness(void *arg)
{
  (void)arg;
  MOCK(router_digest_is_trusted_dir_type, mock_router_digest_is_trusted);
  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(dirserv_get_bandwidth_for_router_kb, mock_dirserv_get_bw);
  ALLOCATE_MOCK_NODES();
  router_properties = digestmap_new();

  // The router one is the "least useful" router, every router is compared to
  // it
  sha1_digest_t digest_one = "aaaa";
  router_values_t *status_one = router_values_new(0, 0, 0, digest_one);
  digestmap_set(router_properties, status_one->digest, status_one);
  sha1_digest_t digest_two = "bbbb";
  router_values_t *status_two = router_values_new(0, 1, 0, digest_two);
  digestmap_set(router_properties, status_two->digest, status_two);
  sha1_digest_t digest_three = "cccc";
  router_values_t *status_three = router_values_new(1, 0, 0, digest_three);
  digestmap_set(router_properties, status_three->digest, status_three);
  sha1_digest_t digest_four = "dddd";
  router_values_t *status_four = router_values_new(0, 0, 128, digest_four);
  digestmap_set(router_properties, status_four->digest, status_four);
  sha1_digest_t digest_five = "9999";
  router_values_t *status_five = router_values_new(0, 0, 0, digest_five);
  digestmap_set(router_properties, status_five->digest, status_five);

  // A router that has auth status is more useful than a non-auth one
  routerinfo_t *first = routerinfo_new(status_one, AF_INET, 0xf);
  routerinfo_t *second = routerinfo_new(status_two, AF_INET, 0xf);
  int a = compare_routerinfo_usefulness(first, second);
  tt_assert(a == 1);
  tor_free(second);

  // A running router is more useful than a non running one
  routerinfo_t *third = routerinfo_new(status_three, AF_INET, 0xf);
  a = compare_routerinfo_usefulness(first, third);
  tt_assert(a == 1);
  tor_free(third);

  // A higher bandwidth is more useful
  routerinfo_t *fourth = routerinfo_new(status_four, AF_INET, 0xf);
  a = compare_routerinfo_usefulness(first, fourth);
  tt_assert(a == 1);
  tor_free(fourth);

  // In case of tie, the digests are compared
  routerinfo_t *fifth = routerinfo_new(status_five, AF_INET, 0xf);
  a = compare_routerinfo_usefulness(first, fifth);
  tt_assert(a > 0);
  tor_free(fifth);

done:
  UNMOCK(router_digest_is_trusted_dir_type);
  UNMOCK(node_get_by_id);
  UNMOCK(dirserv_get_bandwidth_for_router_kb);
  FREE_MOCK_NODES();
  digestmap_free(router_properties, NULL);
  tor_free(status_one);
  tor_free(status_two);
  tor_free(status_three);
  tor_free(status_four);
  tor_free(status_five);
  tor_free(first);
}

static void
test_dirvote_compare_routerinfo_by_ipv4(void *arg)
{
  (void)arg;
  MOCK(router_digest_is_trusted_dir_type, mock_router_digest_is_trusted);
  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(dirserv_get_bandwidth_for_router_kb, mock_dirserv_get_bw);

  ALLOCATE_MOCK_NODES();
  router_properties = digestmap_new();
  sha1_digest_t digest_one = "aaaa";
  router_values_t *status_one = router_values_new(0, 0, 0, digest_one);
  digestmap_set(router_properties, status_one->digest, status_one);
  sha1_digest_t digest_two = "bbbb";
  router_values_t *status_two = router_values_new(0, 1, 0, digest_two);
  digestmap_set(router_properties, status_two->digest, status_two);

  // Both routers have an IPv4 address
  routerinfo_t *first = routerinfo_new(status_one, AF_INET, 1);
  routerinfo_t *second = routerinfo_new(status_two, AF_INET, 0xf);

  // The first argument's address precedes the seconds' one
  int a = compare_routerinfo_by_ipv4((const void **)&first,
                                     (const void **)&second);
  tt_assert(a < 0);
  // The second argument's address precedes the first' one
  a = compare_routerinfo_by_ipv4((const void **)&second,
                                 (const void **)&first);
  tt_assert(a > 0);
  tor_addr_copy(&(second->ipv4_addr), &(first->ipv6_addr));
  // The addresses are equal, they are compared by usefulness,
  // and first is less useful than second
  a = compare_routerinfo_by_ipv4((const void **)&first,
                                 (const void **)&second);
  tt_assert(a == 1);
done:
  UNMOCK(router_digest_is_trusted_dir_type);
  UNMOCK(node_get_by_id);
  UNMOCK(dirserv_get_bandwidth_for_router_kb);
  FREE_MOCK_NODES();
  digestmap_free(router_properties, NULL);
  tor_free(status_one);
  tor_free(status_two);
  tor_free(first);
  tor_free(second);
}

static void
test_dirvote_compare_routerinfo_by_ipv6(void *arg)
{
  (void)arg;
  MOCK(router_digest_is_trusted_dir_type, mock_router_digest_is_trusted);
  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(dirserv_get_bandwidth_for_router_kb, mock_dirserv_get_bw);

  ALLOCATE_MOCK_NODES();
  router_properties = digestmap_new();
  char digest_one[DIGEST_LEN] = "aaaa";
  router_values_t *status_one = router_values_new(0, 0, 0, digest_one);
  digestmap_set(router_properties, status_one->digest, status_one);
  char digest_two[DIGEST_LEN] = "bbbb";
  router_values_t *status_two = router_values_new(0, 1, 0, digest_two);
  digestmap_set(router_properties, status_two->digest, status_two);

  // Both routers have an IPv6 address
  routerinfo_t *first = routerinfo_new(status_one, AF_INET6, 1);
  routerinfo_t *second = routerinfo_new(status_two, AF_INET6, 0xf);

  // The first argument's address precedes the seconds' one
  int a = compare_routerinfo_by_ipv6((const void **)&first,
                                     (const void **)&second);
  tt_assert(a < 0);
  // The second argument's address precedes the first' one
  a = compare_routerinfo_by_ipv6((const void **)&second,
                                 (const void **)&first);
  tt_assert(a > 0);
  tor_addr_copy(&(first->ipv6_addr), &(second->ipv6_addr));
  // The addresses are equal, they are compared by usefulness,
  // and first is less useful than second
  a = compare_routerinfo_by_ipv6((const void **)&first,
                                 (const void **)&second);
  tt_assert(a == 1);
done:
  UNMOCK(router_digest_is_trusted_dir_type);
  UNMOCK(node_get_by_id);
  UNMOCK(dirserv_get_bandwidth_for_router_kb);
  FREE_MOCK_NODES();
  digestmap_free(router_properties, NULL);
  tor_free(status_one);
  tor_free(status_two);
  tor_free(first);
  tor_free(second);
}

/** Create routers values and routerinfos that always have the same
 * characteristics, and add them to the global digestmap. This macro is here to
 * avoid duplicated code fragments.
 * The created name##_val pointer should be freed by the caller (and cannot
 * be freed in the macro as it causes a heap-after-free error)
 */
#define CREATE_ROUTER(digest, name, addr, ip_version)                    \
  sha1_digest_t name##_digest = digest;                                  \
  name##_val = router_values_new(1, 1, 1, name##_digest);               \
  digestmap_set(router_properties, name##_digest, name##_val);           \
  name##_ri = routerinfo_new(name##_val, ip_version, addr);

#define ROUTER_FREE(name) \
  tor_free(name##_val);   \
  tor_free(name##_ri);

/** Test to see if the returned routers are exactly the ones that should be
 * flagged as sybils : we test for inclusion then for number of elements
 */
#define TEST_SYBIL(true_sybil, possible_sybil)               \
  DIGESTMAP_FOREACH (true_sybil, sybil_id, void *, ignore) { \
    (void)ignore;                                            \
    tt_assert(digestmap_get(possible_sybil, sybil_id));      \
  }                                                          \
  DIGESTMAP_FOREACH_END;                                     \
  tt_assert(digestmap_size(true_sybil) == digestmap_size(possible_sybil));

static void
test_dirvote_get_sybil_by_ip_version_ipv4(void *arg)
{
  // It is assumed that global_dirauth_options.AuthDirMaxServersPerAddr == 2
  (void)arg;
  router_values_t *aaaa_val=NULL, *bbbb_val=NULL, *cccc_val=NULL,
    *dddd_val=NULL, *eeee_val=NULL, *ffff_val=NULL, *gggg_val=NULL,
    *hhhh_val=NULL;
  routerinfo_t *aaaa_ri=NULL, *bbbb_ri=NULL, *cccc_ri=NULL,
    *dddd_ri=NULL, *eeee_ri=NULL, *ffff_ri=NULL, *gggg_ri=NULL,
    *hhhh_ri=NULL;

  MOCK(router_digest_is_trusted_dir_type, mock_router_digest_is_trusted);
  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(dirserv_get_bandwidth_for_router_kb, mock_dirserv_get_bw);
  ALLOCATE_MOCK_NODES();
  router_properties = digestmap_new();
  smartlist_t *routers_ipv4;
  routers_ipv4 = smartlist_new();
  digestmap_t *true_sybil_routers = NULL;
  true_sybil_routers = digestmap_new();
  digestmap_t *omit_as_sybil;

  CREATE_ROUTER("aaaa", aaaa, 123, AF_INET);
  smartlist_add(routers_ipv4, aaaa_ri);
  CREATE_ROUTER("bbbb", bbbb, 123, AF_INET);
  smartlist_add(routers_ipv4, bbbb_ri);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv4, AF_INET);
  tt_assert(digestmap_isempty(omit_as_sybil) == 1);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("cccc", cccc, 123, AF_INET);
  smartlist_add(routers_ipv4, cccc_ri);
  digestmap_set(true_sybil_routers, cccc_digest, cccc_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv4, AF_INET);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("dddd", dddd, 123, AF_INET);
  smartlist_add(routers_ipv4, dddd_ri);
  digestmap_set(true_sybil_routers, dddd_digest, dddd_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv4, AF_INET);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("eeee", eeee, 456, AF_INET);
  smartlist_add(routers_ipv4, eeee_ri);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv4, AF_INET);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("ffff", ffff, 456, AF_INET);
  smartlist_add(routers_ipv4, ffff_ri);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv4, AF_INET);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("gggg", gggg, 456, AF_INET);
  smartlist_add(routers_ipv4, gggg_ri);
  digestmap_set(true_sybil_routers, gggg_digest, gggg_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv4, AF_INET);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("hhhh", hhhh, 456, AF_INET);
  smartlist_add(routers_ipv4, hhhh_ri);
  digestmap_set(true_sybil_routers, hhhh_digest, hhhh_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv4, AF_INET);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);

done:
  UNMOCK(router_digest_is_trusted_dir_type);
  UNMOCK(node_get_by_id);
  UNMOCK(dirserv_get_bandwidth_for_router_kb);
  FREE_MOCK_NODES();
  digestmap_free(router_properties, NULL);
  smartlist_free(routers_ipv4);
  digestmap_free(omit_as_sybil, NULL);
  digestmap_free(true_sybil_routers, NULL);
  ROUTER_FREE(aaaa);
  ROUTER_FREE(bbbb);
  ROUTER_FREE(cccc);
  ROUTER_FREE(dddd);
  ROUTER_FREE(eeee);
  ROUTER_FREE(ffff);
  ROUTER_FREE(gggg);
  ROUTER_FREE(hhhh);
}

static void
test_dirvote_get_sybil_by_ip_version_ipv6(void *arg)
{
  router_values_t *aaaa_val=NULL, *bbbb_val=NULL, *cccc_val=NULL,
    *dddd_val=NULL, *eeee_val=NULL, *ffff_val=NULL, *gggg_val=NULL,
    *hhhh_val=NULL;
  routerinfo_t *aaaa_ri=NULL, *bbbb_ri=NULL, *cccc_ri=NULL,
    *dddd_ri=NULL, *eeee_ri=NULL, *ffff_ri=NULL, *gggg_ri=NULL,
    *hhhh_ri=NULL;

  // It is assumed that global_dirauth_options.AuthDirMaxServersPerAddr == 2
  (void)arg;
  MOCK(router_digest_is_trusted_dir_type, mock_router_digest_is_trusted);
  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(dirserv_get_bandwidth_for_router_kb, mock_dirserv_get_bw);
  ALLOCATE_MOCK_NODES();
  router_properties = digestmap_new();
  smartlist_t *routers_ipv6;
  routers_ipv6 = smartlist_new();
  digestmap_t *true_sybil_routers = NULL;
  true_sybil_routers = digestmap_new();
  digestmap_t *omit_as_sybil;

  CREATE_ROUTER("aaaa", aaaa, 123, AF_INET6);
  smartlist_add(routers_ipv6, aaaa_ri);
  CREATE_ROUTER("bbbb", bbbb, 123, AF_INET6);
  smartlist_add(routers_ipv6, bbbb_ri);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv6, AF_INET6);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("cccc", cccc, 123, AF_INET6);
  smartlist_add(routers_ipv6, cccc_ri);
  digestmap_set(true_sybil_routers, cccc_digest, cccc_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv6, AF_INET6);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("dddd", dddd, 123, AF_INET6);
  smartlist_add(routers_ipv6, dddd_ri);
  digestmap_set(true_sybil_routers, dddd_digest, dddd_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv6, AF_INET6);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("eeee", eeee, 456, AF_INET6);
  smartlist_add(routers_ipv6, eeee_ri);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv6, AF_INET6);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("ffff", ffff, 456, AF_INET6);
  smartlist_add(routers_ipv6, ffff_ri);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv6, AF_INET6);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("gggg", gggg, 456, AF_INET6);
  smartlist_add(routers_ipv6, gggg_ri);
  digestmap_set(true_sybil_routers, gggg_digest, gggg_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv6, AF_INET6);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("hhhh", hhhh, 456, AF_INET6);
  smartlist_add(routers_ipv6, hhhh_ri);
  digestmap_set(true_sybil_routers, hhhh_digest, hhhh_digest);
  omit_as_sybil = get_sybil_list_by_ip_version(routers_ipv6, AF_INET6);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
done:
  UNMOCK(router_digest_is_trusted_dir_type);
  UNMOCK(node_get_by_id);
  UNMOCK(dirserv_get_bandwidth_for_router_kb);
  FREE_MOCK_NODES();
  digestmap_free(router_properties, NULL);
  digestmap_free(true_sybil_routers, NULL);
  smartlist_free(routers_ipv6);
  digestmap_free(omit_as_sybil, NULL);
  ROUTER_FREE(aaaa);
  ROUTER_FREE(bbbb);
  ROUTER_FREE(cccc);
  ROUTER_FREE(dddd);
  ROUTER_FREE(eeee);
  ROUTER_FREE(ffff);
  ROUTER_FREE(gggg);
  ROUTER_FREE(hhhh);
}

static void
test_dirvote_get_all_possible_sybil(void *arg)
{
  router_values_t *aaaa_val=NULL, *bbbb_val=NULL, *cccc_val=NULL,
    *dddd_val=NULL, *eeee_val=NULL, *ffff_val=NULL, *gggg_val=NULL,
    *hhhh_val=NULL, *iiii_val=NULL, *jjjj_val=NULL, *kkkk_val=NULL,
    *llll_val=NULL, *mmmm_val=NULL, *nnnn_val=NULL, *oooo_val=NULL,
    *pppp_val=NULL;
  routerinfo_t *aaaa_ri=NULL, *bbbb_ri=NULL, *cccc_ri=NULL,
    *dddd_ri=NULL, *eeee_ri=NULL, *ffff_ri=NULL, *gggg_ri=NULL,
    *hhhh_ri=NULL, *iiii_ri=NULL, *jjjj_ri=NULL, *kkkk_ri=NULL,
    *llll_ri=NULL, *mmmm_ri=NULL, *nnnn_ri=NULL, *oooo_ri=NULL,
    *pppp_ri=NULL;

  // It is assumed that global_dirauth_options.AuthDirMaxServersPerAddr == 2
  (void)arg;
  MOCK(router_digest_is_trusted_dir_type, mock_router_digest_is_trusted);
  MOCK(node_get_by_id, mock_node_get_by_id);
  MOCK(dirserv_get_bandwidth_for_router_kb, mock_dirserv_get_bw);
  ALLOCATE_MOCK_NODES();
  router_properties = digestmap_new();
  smartlist_t *routers;
  routers = smartlist_new();
  digestmap_t *true_sybil_routers = NULL;
  true_sybil_routers = digestmap_new();
  digestmap_t *omit_as_sybil;

  CREATE_ROUTER("aaaa", aaaa, 123, AF_INET);
  smartlist_add(routers, aaaa_ri);
  CREATE_ROUTER("bbbb", bbbb, 123, AF_INET);
  smartlist_add(routers, bbbb_ri);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("cccc", cccc, 123, AF_INET);
  smartlist_add(routers, cccc_ri);
  digestmap_set(true_sybil_routers, cccc_digest, cccc_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("dddd", dddd, 123, AF_INET);
  smartlist_add(routers, dddd_ri);
  digestmap_set(true_sybil_routers, dddd_digest, dddd_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("eeee", eeee, 456, AF_INET);
  smartlist_add(routers, eeee_ri);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("ffff", ffff, 456, AF_INET);
  smartlist_add(routers, ffff_ri);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("gggg", gggg, 456, AF_INET);
  smartlist_add(routers, gggg_ri);
  digestmap_set(true_sybil_routers, gggg_digest, gggg_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("hhhh", hhhh, 456, AF_INET);
  smartlist_add(routers, hhhh_ri);
  digestmap_set(true_sybil_routers, hhhh_digest, hhhh_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("iiii", iiii, 123, AF_INET6);
  smartlist_add(routers, iiii_ri);
  CREATE_ROUTER("jjjj", jjjj, 123, AF_INET6);
  smartlist_add(routers, jjjj_ri);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("kkkk", kkkk, 123, AF_INET6);
  smartlist_add(routers, kkkk_ri);
  digestmap_set(true_sybil_routers, kkkk_digest, kkkk_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil,NULL);

  CREATE_ROUTER("llll", llll, 123, AF_INET6);
  smartlist_add(routers, llll_ri);
  digestmap_set(true_sybil_routers, llll_digest, llll_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil,NULL);

  CREATE_ROUTER("mmmm", mmmm, 456, AF_INET6);
  smartlist_add(routers, mmmm_ri);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("nnnn", nnnn, 456, AF_INET6);
  smartlist_add(routers, nnnn_ri);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("oooo", oooo, 456, AF_INET6);
  smartlist_add(routers, oooo_ri);
  digestmap_set(true_sybil_routers, oooo_digest, oooo_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);
  digestmap_free(omit_as_sybil, NULL);

  CREATE_ROUTER("pppp", pppp, 456, AF_INET6);
  smartlist_add(routers, pppp_ri);
  digestmap_set(true_sybil_routers, pppp_digest, pppp_digest);
  omit_as_sybil = get_all_possible_sybil(routers);
  TEST_SYBIL(true_sybil_routers, omit_as_sybil);

done:
  UNMOCK(router_digest_is_trusted_dir_type);
  UNMOCK(node_get_by_id);
  UNMOCK(dirserv_get_bandwidth_for_router_kb);
  FREE_MOCK_NODES();
  digestmap_free(router_properties, NULL);
  smartlist_free(routers);
  digestmap_free(omit_as_sybil, NULL);
  digestmap_free(true_sybil_routers, NULL);
  ROUTER_FREE(aaaa);
  ROUTER_FREE(bbbb);
  ROUTER_FREE(cccc);
  ROUTER_FREE(dddd);
  ROUTER_FREE(eeee);
  ROUTER_FREE(ffff);
  ROUTER_FREE(gggg);
  ROUTER_FREE(hhhh);
  ROUTER_FREE(iiii);
  ROUTER_FREE(jjjj);
  ROUTER_FREE(kkkk);
  ROUTER_FREE(llll);
  ROUTER_FREE(mmmm);
  ROUTER_FREE(nnnn);
  ROUTER_FREE(oooo);
  ROUTER_FREE(pppp);
}

static void
test_dirvote_parse_param_buggy(void *arg)
{
  (void)arg;

  /* Tests for behavior with bug emulation to migrate away from bug 19011. */
  tt_i64_op(extract_param_buggy("blah blah", "bwweightscale", 10000),
            OP_EQ, 10000);
  tt_i64_op(extract_param_buggy("bwweightscale=7", "bwweightscale", 10000),
            OP_EQ, 7);
  tt_i64_op(extract_param_buggy("bwweightscale=7 foo=9",
                                "bwweightscale", 10000),
            OP_EQ, 10000);
  tt_i64_op(extract_param_buggy("foo=7 bwweightscale=777 bar=9",
                                "bwweightscale", 10000),
            OP_EQ, 10000);
  tt_i64_op(extract_param_buggy("foo=7 bwweightscale=1234",
                                "bwweightscale", 10000),
            OP_EQ, 1234);

 done:
  ;
}

#define NODE(name, flags)                           \
  {                                                 \
    #name, test_dirvote_##name, (flags), NULL, NULL \
  }

struct testcase_t dirvote_tests[] = {
    NODE(compare_routerinfo_usefulness, TT_FORK),
    NODE(compare_routerinfo_by_ipv6, TT_FORK),
    NODE(compare_routerinfo_by_ipv4, TT_FORK),
    NODE(get_sybil_by_ip_version_ipv4, TT_FORK),
    NODE(get_sybil_by_ip_version_ipv6, TT_FORK),
    NODE(get_all_possible_sybil, TT_FORK),
    NODE(parse_param_buggy, 0),
    END_OF_TESTCASES};
