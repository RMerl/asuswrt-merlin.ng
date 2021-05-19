/* Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define ROUTERSET_PRIVATE

#include "core/or/or.h"
#include "core/or/policies.h"
#include "feature/dirparse/policy_parse.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerset.h"
#include "lib/geoip/geoip.h"

#include "core/or/addr_policy_st.h"
#include "core/or/extend_info_st.h"
#include "feature/nodelist/node_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

#include "test/test.h"

/*
 * Functional (blackbox) test to determine that each member of the routerset
 * is non-NULL
 */

static void
test_rset_new(void *arg)
{
  routerset_t *rs;
  (void)arg;

  rs = routerset_new();

  tt_ptr_op(rs, OP_NE, NULL);
  tt_ptr_op(rs->list, OP_NE, NULL);
  tt_ptr_op(rs->names, OP_NE, NULL);
  tt_ptr_op(rs->digests, OP_NE, NULL);
  tt_ptr_op(rs->policies, OP_NE, NULL);
  tt_ptr_op(rs->country_names, OP_NE, NULL);

  done:
    routerset_free(rs);
}

/*
 * Functional test to strip the braces from a "{xx}" country code string.
 */

static void
test_rset_get_countryname(void *arg)
{
  const char *input;
  char *name;
  (void)arg;

  /* strlen(c) < 4 */
  input = "xxx";
  name = routerset_get_countryname(input);
  tt_ptr_op(name, OP_EQ, NULL);
  tor_free(name);

  /* c[0] != '{' */
  input = "xxx}";
  name = routerset_get_countryname(input);
  tt_ptr_op(name, OP_EQ, NULL);
  tor_free(name);

  /* c[3] != '}' */
  input = "{xxx";
  name = routerset_get_countryname(input);
  tt_ptr_op(name, OP_EQ, NULL);
  tor_free(name);

  /* tor_strlower */
  input = "{XX}";
  name = routerset_get_countryname(input);
  tt_str_op(name, OP_EQ, "xx");
  tor_free(name);

  input = "{xx}";
  name = routerset_get_countryname(input);
  tt_str_op(name, OP_EQ, "xx");
  done:
    tor_free(name);
}

/*
 * Structural (whitebox) test for routerset_refresh_counties, when the GeoIP DB
 * is not loaded.
 */

static int rset_refresh_geoip_not_loaded_geoip_is_loaded(sa_family_t family);
static int rset_refresh_geoip_not_loaded_geoip_is_loaded_called = 0;
static int rset_refresh_geoip_not_loaded_geoip_get_n_countries(void);
static int rset_refresh_geoip_not_loaded_geoip_get_n_countries_called = 0;

static void
test_rset_refresh_geoip_not_loaded(void *arg)
{
  routerset_t *set = routerset_new();
  (void)arg;

  MOCK(geoip_is_loaded,
       rset_refresh_geoip_not_loaded_geoip_is_loaded);
  MOCK(geoip_get_n_countries,
       rset_refresh_geoip_not_loaded_geoip_get_n_countries);

  routerset_refresh_countries(set);

  tt_ptr_op(set->countries, OP_EQ, NULL);
  tt_int_op(set->n_countries, OP_EQ, 0);
  tt_int_op(rset_refresh_geoip_not_loaded_geoip_is_loaded_called, OP_EQ, 1);
  tt_int_op(rset_refresh_geoip_not_loaded_geoip_get_n_countries_called,
            OP_EQ, 0);

  done:
    UNMOCK(geoip_is_loaded);
    UNMOCK(geoip_get_n_countries);
    routerset_free(set);
}

static int
rset_refresh_geoip_not_loaded_geoip_is_loaded(sa_family_t family)
{
  (void)family;
  rset_refresh_geoip_not_loaded_geoip_is_loaded_called++;

  return 0;
}

static int
rset_refresh_geoip_not_loaded_geoip_get_n_countries(void)
{
  rset_refresh_geoip_not_loaded_geoip_get_n_countries_called++;

  return 0;
}

/*
 * Structural test for routerset_refresh_counties, when there are no countries.
 */

static int rset_refresh_no_countries_geoip_is_loaded(sa_family_t family);
static int rset_refresh_no_countries_geoip_is_loaded_called = 0;
static int rset_refresh_no_countries_geoip_get_n_countries(void);
static int rset_refresh_no_countries_geoip_get_n_countries_called = 0;
static country_t rset_refresh_no_countries_geoip_get_country(
                                                    const char *country);
static int rset_refresh_no_countries_geoip_get_country_called = 0;

static void
test_rset_refresh_no_countries(void *arg)
{
  routerset_t *set = routerset_new();
  (void)arg;

  MOCK(geoip_is_loaded,
       rset_refresh_no_countries_geoip_is_loaded);
  MOCK(geoip_get_n_countries,
       rset_refresh_no_countries_geoip_get_n_countries);
  MOCK(geoip_get_country,
       rset_refresh_no_countries_geoip_get_country);

  routerset_refresh_countries(set);

  tt_ptr_op(set->countries, OP_NE, NULL);
  tt_int_op(set->n_countries, OP_EQ, 1);
  tt_int_op((unsigned int)(*set->countries), OP_EQ, 0);
  tt_int_op(rset_refresh_no_countries_geoip_is_loaded_called, OP_EQ, 1);
  tt_int_op(rset_refresh_no_countries_geoip_get_n_countries_called, OP_EQ, 1);
  tt_int_op(rset_refresh_no_countries_geoip_get_country_called, OP_EQ, 0);

  done:
    UNMOCK(geoip_is_loaded);
    UNMOCK(geoip_get_n_countries);
    UNMOCK(geoip_get_country);
    routerset_free(set);
}

static int
rset_refresh_no_countries_geoip_is_loaded(sa_family_t family)
{
  (void)family;
  rset_refresh_no_countries_geoip_is_loaded_called++;

  return 1;
}

static int
rset_refresh_no_countries_geoip_get_n_countries(void)
{
  rset_refresh_no_countries_geoip_get_n_countries_called++;

  return 1;
}

static country_t
rset_refresh_no_countries_geoip_get_country(const char *countrycode)
{
  (void)countrycode;
  rset_refresh_no_countries_geoip_get_country_called++;

  return 1;
}

/*
 * Structural test for routerset_refresh_counties, with one valid country.
 */

static int rset_refresh_one_valid_country_geoip_is_loaded(sa_family_t family);
static int rset_refresh_one_valid_country_geoip_is_loaded_called = 0;
static int rset_refresh_one_valid_country_geoip_get_n_countries(void);
static int rset_refresh_one_valid_country_geoip_get_n_countries_called = 0;
static country_t rset_refresh_one_valid_country_geoip_get_country(
                                                    const char *country);
static int rset_refresh_one_valid_country_geoip_get_country_called = 0;

static void
test_rset_refresh_one_valid_country(void *arg)
{
  routerset_t *set = routerset_new();
  (void)arg;

  MOCK(geoip_is_loaded,
       rset_refresh_one_valid_country_geoip_is_loaded);
  MOCK(geoip_get_n_countries,
       rset_refresh_one_valid_country_geoip_get_n_countries);
  MOCK(geoip_get_country,
       rset_refresh_one_valid_country_geoip_get_country);
  smartlist_add(set->country_names, tor_strndup("foo", 3));

  routerset_refresh_countries(set);

  tt_ptr_op(set->countries, OP_NE, NULL);
  tt_int_op(set->n_countries, OP_EQ, 2);
  tt_int_op(rset_refresh_one_valid_country_geoip_is_loaded_called, OP_EQ, 1);
  tt_int_op(rset_refresh_one_valid_country_geoip_get_n_countries_called,
            OP_EQ, 1);
  tt_int_op(rset_refresh_one_valid_country_geoip_get_country_called, OP_EQ, 1);
  tt_int_op((unsigned int)(*set->countries), OP_NE, 0);

  done:
    UNMOCK(geoip_is_loaded);
    UNMOCK(geoip_get_n_countries);
    UNMOCK(geoip_get_country);
    routerset_free(set);
}

static int
rset_refresh_one_valid_country_geoip_is_loaded(sa_family_t family)
{
  (void)family;
  rset_refresh_one_valid_country_geoip_is_loaded_called++;

  return 1;
}

static int
rset_refresh_one_valid_country_geoip_get_n_countries(void)
{
  rset_refresh_one_valid_country_geoip_get_n_countries_called++;

  return 2;
}

static country_t
rset_refresh_one_valid_country_geoip_get_country(const char *countrycode)
{
  (void)countrycode;
  rset_refresh_one_valid_country_geoip_get_country_called++;

  return 1;
}

/*
 * Structural test for routerset_refresh_counties, with one invalid
 * country code..
 */

static int rset_refresh_one_invalid_country_geoip_is_loaded(
                                               sa_family_t family);
static int rset_refresh_one_invalid_country_geoip_is_loaded_called = 0;
static int rset_refresh_one_invalid_country_geoip_get_n_countries(void);
static int rset_refresh_one_invalid_country_geoip_get_n_countries_called = 0;
static country_t rset_refresh_one_invalid_country_geoip_get_country(
                                               const char *country);
static int rset_refresh_one_invalid_country_geoip_get_country_called = 0;

static void
test_rset_refresh_one_invalid_country(void *arg)
{
  routerset_t *set = routerset_new();
  (void)arg;

  MOCK(geoip_is_loaded,
       rset_refresh_one_invalid_country_geoip_is_loaded);
  MOCK(geoip_get_n_countries,
       rset_refresh_one_invalid_country_geoip_get_n_countries);
  MOCK(geoip_get_country,
       rset_refresh_one_invalid_country_geoip_get_country);
  smartlist_add(set->country_names, tor_strndup("foo", 3));

  routerset_refresh_countries(set);

  tt_ptr_op(set->countries, OP_NE, NULL);
  tt_int_op(set->n_countries, OP_EQ, 2);
  tt_int_op(rset_refresh_one_invalid_country_geoip_is_loaded_called, OP_EQ, 1);
  tt_int_op(rset_refresh_one_invalid_country_geoip_get_n_countries_called,
            OP_EQ, 1);
  tt_int_op(rset_refresh_one_invalid_country_geoip_get_country_called,
            OP_EQ, 1);
  tt_int_op((unsigned int)(*set->countries), OP_EQ, 0);

  done:
    UNMOCK(geoip_is_loaded);
    UNMOCK(geoip_get_n_countries);
    UNMOCK(geoip_get_country);
    routerset_free(set);
}

static int
rset_refresh_one_invalid_country_geoip_is_loaded(sa_family_t family)
{
  (void)family;
  rset_refresh_one_invalid_country_geoip_is_loaded_called++;

  return 1;
}

static int
rset_refresh_one_invalid_country_geoip_get_n_countries(void)
{
  rset_refresh_one_invalid_country_geoip_get_n_countries_called++;

  return 2;
}

static country_t
rset_refresh_one_invalid_country_geoip_get_country(const char *countrycode)
{
  (void)countrycode;
  rset_refresh_one_invalid_country_geoip_get_country_called++;

  return -1;
}

/*
 * Functional test, with a malformed string to parse.
 */

static void
test_rset_parse_malformed(void *arg)
{
  routerset_t *set = routerset_new();
  const char *s = "_";
  int r;
  (void)arg;

  r = routerset_parse(set, s, "");

  tt_int_op(r, OP_EQ, -1);

  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_parse, that routerset_parse returns 0
 * on a valid hexdigest entry.
 */

static void
test_rset_parse_valid_hexdigest(void *arg)
{
  routerset_t *set;
  const char *s;
  int r;
  (void)arg;

  set = routerset_new();
  s = "$0000000000000000000000000000000000000000";
  r = routerset_parse(set, s, "");
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(digestmap_isempty(set->digests), OP_NE, 1);

  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_parse, when given a valid nickname as input.
 */

static void
test_rset_parse_valid_nickname(void *arg)
{
  routerset_t *set;
  const char *s;
  int r;
  (void)arg;

  set = routerset_new();
  s = "fred";
  r = routerset_parse(set, s, "");
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(strmap_isempty(set->names), OP_NE, 1);

  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_parse, when given a valid countryname.
 */

static void
test_rset_parse_get_countryname(void *arg)
{
  routerset_t *set;
  const char *s;
  int r;
  (void)arg;

  set = routerset_new();
  s = "{cc}";
  r = routerset_parse(set, s, "");
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(set->country_names), OP_NE, 0);

  done:
    routerset_free(set);
}

/*
 * Structural test for routerset_parse, when given a valid wildcard policy.
 */

static addr_policy_t * rset_parse_policy_wildcard_parse_item_from_string(
               const char *s, int assume_action, int *malformed_list);
static int rset_parse_policy_wildcard_parse_item_from_string_called = 0;

static addr_policy_t *rset_parse_policy_wildcard_mock_addr_policy;

static void
test_rset_parse_policy_wildcard(void *arg)
{
  routerset_t *set;
  const char *s;
  int r;
  (void)arg;

  MOCK(router_parse_addr_policy_item_from_string,
       rset_parse_policy_wildcard_parse_item_from_string);
  rset_parse_policy_wildcard_mock_addr_policy =
    tor_malloc_zero(sizeof(addr_policy_t));

  set = routerset_new();
  s = "*";
  r = routerset_parse(set, s, "");
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(set->policies), OP_NE, 0);
  tt_int_op(rset_parse_policy_wildcard_parse_item_from_string_called,
            OP_EQ, 1);

  done:
    routerset_free(set);
}

addr_policy_t *
rset_parse_policy_wildcard_parse_item_from_string(const char *s,
                                              int assume_action,
                                              int *malformed_list)
{
  (void)s;
  (void)assume_action;
  (void)malformed_list;
  rset_parse_policy_wildcard_parse_item_from_string_called++;

  return rset_parse_policy_wildcard_mock_addr_policy;
}

/*
 * Structural test for routerset_parse, when given a valid IPv4 address
 * literal policy.
 */

static addr_policy_t * rset_parse_policy_ipv4_parse_item_from_string(
              const char *s, int assume_action, int *bogus);
static int rset_parse_policy_ipv4_parse_item_from_string_called = 0;

static addr_policy_t *rset_parse_policy_ipv4_mock_addr_policy;

static void
test_rset_parse_policy_ipv4(void *arg)
{
  routerset_t *set;
  const char *s;
  int r;
  (void)arg;

  MOCK(router_parse_addr_policy_item_from_string,
       rset_parse_policy_ipv4_parse_item_from_string);
  rset_parse_policy_ipv4_mock_addr_policy =
    tor_malloc_zero(sizeof(addr_policy_t));

  set = routerset_new();
  s = "127.0.0.1";
  r = routerset_parse(set, s, "");
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(set->policies), OP_NE, 0);
  tt_int_op(rset_parse_policy_ipv4_parse_item_from_string_called, OP_EQ, 1);

 done:
  routerset_free(set);
}

addr_policy_t *
rset_parse_policy_ipv4_parse_item_from_string(
                                      const char *s, int assume_action,
                                      int *bogus)
{
  (void)s;
  (void)assume_action;
  rset_parse_policy_ipv4_parse_item_from_string_called++;
  *bogus = 0;

  return rset_parse_policy_ipv4_mock_addr_policy;
}

/*
 * Structural test for routerset_parse, when given a valid IPv6 address
 * literal policy.
 */

static addr_policy_t * rset_parse_policy_ipv6_parse_item_from_string(
               const char *s, int assume_action, int *bad);
static int rset_parse_policy_ipv6_parse_item_from_string_called = 0;

static addr_policy_t *rset_parse_policy_ipv6_mock_addr_policy;

static void
test_rset_parse_policy_ipv6(void *arg)
{
  routerset_t *set;
  const char *s;
  int r;
  (void)arg;

  MOCK(router_parse_addr_policy_item_from_string,
       rset_parse_policy_ipv6_parse_item_from_string);
  rset_parse_policy_ipv6_mock_addr_policy =
    tor_malloc_zero(sizeof(addr_policy_t));

  set = routerset_new();
  s = "::1";
  r = routerset_parse(set, s, "");
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(smartlist_len(set->policies), OP_NE, 0);
  tt_int_op(rset_parse_policy_ipv6_parse_item_from_string_called, OP_EQ, 1);

 done:
  routerset_free(set);
}

addr_policy_t *
rset_parse_policy_ipv6_parse_item_from_string(const char *s,
                                              int assume_action, int *bad)
{
  (void)s;
  (void)assume_action;
  rset_parse_policy_ipv6_parse_item_from_string_called++;
  *bad = 0;

  return rset_parse_policy_ipv6_mock_addr_policy;
}

/*
 * Structural test for routerset_union, when given a bad source argument.
 */

static smartlist_t * rset_union_source_bad_smartlist_new(void);
static int rset_union_source_bad_smartlist_new_called = 0;

static void
test_rset_union_source_bad(void *arg)
{
  routerset_t *set, *bad_set;
  (void)arg;

  set = routerset_new();
  bad_set = routerset_new();
  smartlist_free(bad_set->list);
  bad_set->list = NULL;

  MOCK(smartlist_new,
       rset_union_source_bad_smartlist_new);

  routerset_union(set, NULL);
  tt_int_op(rset_union_source_bad_smartlist_new_called, OP_EQ, 0);

  routerset_union(set, bad_set);
  tt_int_op(rset_union_source_bad_smartlist_new_called, OP_EQ, 0);

  done:
    UNMOCK(smartlist_new);
    routerset_free(set);

    /* Just recreate list, so we can simply use routerset_free. */
    bad_set->list = smartlist_new();
    routerset_free(bad_set);
}

static smartlist_t *
rset_union_source_bad_smartlist_new(void)
{
  rset_union_source_bad_smartlist_new_called++;

  return NULL;
}

/*
 * Functional test for routerset_union.
 */

static void
test_rset_union_one(void *arg)
{
  routerset_t *src = routerset_new();
  routerset_t *tgt;
  (void)arg;

  tgt = routerset_new();
  smartlist_add_strdup(src->list, "{xx}");
  routerset_union(tgt, src);

  tt_int_op(smartlist_len(tgt->list), OP_NE, 0);

  done:
    routerset_free(src);
    routerset_free(tgt);
}

/*
 * Functional tests for routerset_is_list.
 */

static void
test_rset_is_list(void *arg)
{
  routerset_t *set;
  addr_policy_t *policy;
  int is_list;
  (void)arg;

  /* len(set->country_names) == 0, len(set->policies) == 0 */
  set = routerset_new();
  is_list = routerset_is_list(set);
  routerset_free(set);
  set = NULL;
  tt_int_op(is_list, OP_NE, 0);

  /* len(set->country_names) != 0, len(set->policies) == 0 */
  set = routerset_new();
  smartlist_add(set->country_names, tor_strndup("foo", 3));
  is_list = routerset_is_list(set);
  routerset_free(set);
  set = NULL;
  tt_int_op(is_list, OP_EQ, 0);

  /* len(set->country_names) == 0, len(set->policies) != 0 */
  set = routerset_new();
  policy = tor_malloc_zero(sizeof(addr_policy_t));
  smartlist_add(set->policies, (void *)policy);
  is_list = routerset_is_list(set);
  routerset_free(set);
  set = NULL;
  tt_int_op(is_list, OP_EQ, 0);

  /* len(set->country_names) != 0, len(set->policies) != 0 */
  set = routerset_new();
  smartlist_add(set->country_names, tor_strndup("foo", 3));
  policy = tor_malloc_zero(sizeof(addr_policy_t));
  smartlist_add(set->policies, (void *)policy);
  is_list = routerset_is_list(set);
  routerset_free(set);
  set = NULL;
  tt_int_op(is_list, OP_EQ, 0);

  done:
    ;
}

/*
 * Functional tests for routerset_needs_geoip.
 */

static void
test_rset_needs_geoip(void *arg)
{
  routerset_t *set;
  int needs_geoip;
  (void)arg;

  set = NULL;
  needs_geoip = routerset_needs_geoip(set);
  tt_int_op(needs_geoip, OP_EQ, 0);

  set = routerset_new();
  needs_geoip = routerset_needs_geoip(set);
  routerset_free(set);
  tt_int_op(needs_geoip, OP_EQ, 0);
  set = NULL;

  set = routerset_new();
  smartlist_add(set->country_names, tor_strndup("xx", 2));
  needs_geoip = routerset_needs_geoip(set);
  routerset_free(set);
  set = NULL;
  tt_int_op(needs_geoip, OP_NE, 0);

  done:
    ;
}

/*
 * Functional tests for routerset_is_empty.
 */

static void
test_rset_is_empty(void *arg)
{
  routerset_t *set = NULL;
  int is_empty;
  (void)arg;

  is_empty = routerset_is_empty(set);
  tt_int_op(is_empty, OP_NE, 0);

  set = routerset_new();
  is_empty = routerset_is_empty(set);
  routerset_free(set);
  set = NULL;
  tt_int_op(is_empty, OP_NE, 0);

  set = routerset_new();
  smartlist_add_strdup(set->list, "{xx}");
  is_empty = routerset_is_empty(set);
  routerset_free(set);
  set = NULL;
  tt_int_op(is_empty, OP_EQ, 0);

  done:
    ;
}

/*
 * Functional test for routerset_contains, when given a NULL set or the
 * set has a NULL list.
 */

static void
test_rset_contains_null_set_or_list(void *arg)
{
  routerset_t *set = NULL;
  int contains;
  (void)arg;

  contains = routerset_contains(set, NULL, 0, NULL, NULL, 0);

  tt_int_op(contains, OP_EQ, 0);

  set = tor_malloc_zero(sizeof(routerset_t));
  set->list = NULL;
  contains = routerset_contains(set, NULL, 0, NULL, NULL, 0);
  tor_free(set);
  tt_int_op(contains, OP_EQ, 0);

  done:
    ;
}

/*
 * Functional test for routerset_contains, when given a valid routerset but a
 * NULL nickname.
 */

static void
test_rset_contains_null_nickname(void *arg)
{
  routerset_t *set = routerset_new();
  char *nickname = NULL;
  int contains;
  (void)arg;

  contains = routerset_contains(set, NULL, 0, nickname, NULL, 0);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 0);

  done:
    ;
}

/*
 * Functional test for routerset_contains, when given a valid routerset
 * and the nickname is in the routerset.
 */

static void
test_rset_contains_nickname(void *arg)
{
  routerset_t *set = routerset_new();
  const char *nickname;
  int contains;
  (void)arg;

  nickname = "Foo";  /* This tests the lowercase comparison as well. */
  strmap_set_lc(set->names, nickname, (void *)1);
  contains = routerset_contains(set, NULL, 0, nickname, NULL, 0);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 4);
  done:
    ;
}

/*
 * Functional test for routerset_contains, when given a valid routerset
 * and the nickname is not in the routerset.
 */

static void
test_rset_contains_no_nickname(void *arg)
{
  routerset_t *set = routerset_new();
  int contains;
  (void)arg;

  strmap_set_lc(set->names, "bar", (void *)1);
  contains = routerset_contains(set, NULL, 0, "foo", NULL, 0);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 0);
  done:
    ;
}

/*
 * Functional test for routerset_contains, when given a valid routerset
 * and the digest is contained in the routerset.
 */

static void
test_rset_contains_digest(void *arg)
{
  routerset_t *set = routerset_new();
  int contains;
  uint8_t foo[20] = { 2, 3, 4 };
  (void)arg;

  digestmap_set(set->digests, (const char*)foo, (void *)1);
  contains = routerset_contains(set, NULL, 0, NULL, (const char*)foo, 0);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 4);
  done:
    ;
}

/*
 * Functional test for routerset_contains, when given a valid routerset
 * and the digest is not contained in the routerset.
 */

static void
test_rset_contains_no_digest(void *arg)
{
  routerset_t *set = routerset_new();
  int contains;
  uint8_t bar[20] = { 9, 10, 11, 55 };
  uint8_t foo[20] = { 1, 2, 3, 4};
  (void)arg;

  digestmap_set(set->digests, (const char*)bar, (void *)1);
  contains = routerset_contains(set, NULL, 0, NULL, (const char*)foo, 0);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 0);
  done:
    ;
}

/*
 * Functional test for routerset_contains, when given a valid routerset
 * and the digest is NULL.
 */

static void
test_rset_contains_null_digest(void *arg)
{
  routerset_t *set = routerset_new();
  int contains;
  uint8_t bar[20] = { 9, 10, 11, 55 };
  (void)arg;

  digestmap_set(set->digests, (const char*)bar, (void *)1);
  contains = routerset_contains(set, NULL, 0, NULL, NULL, 0);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 0);
  done:
    ;
}

/*
 * Structural test for routerset_contains, when given a valid routerset
 * and the address is rejected by policy.
 */

static addr_policy_result_t rset_contains_addr_cmp_addr_to_policy(
              const tor_addr_t *addr, uint16_t port,
              const smartlist_t *policy);
static int rset_contains_addr_cmp_addr_to_policy_called = 0;

static tor_addr_t MOCK_TOR_ADDR;
#define MOCK_TOR_ADDR_PTR (&MOCK_TOR_ADDR)

static void
test_rset_contains_addr(void *arg)
{
  routerset_t *set = routerset_new();
  tor_addr_t *addr = MOCK_TOR_ADDR_PTR;
  int contains;
  (void)arg;

  MOCK(compare_tor_addr_to_addr_policy,
       rset_contains_addr_cmp_addr_to_policy);

  contains = routerset_contains(set, addr, 0, NULL, NULL, 0);
  routerset_free(set);

  tt_int_op(rset_contains_addr_cmp_addr_to_policy_called, OP_EQ, 1);
  tt_int_op(contains, OP_EQ, 3);

  done:
    ;
}

addr_policy_result_t
rset_contains_addr_cmp_addr_to_policy(const tor_addr_t *addr, uint16_t port,
    const smartlist_t *policy)
{
  (void)port;
  (void)policy;
  rset_contains_addr_cmp_addr_to_policy_called++;
  tt_ptr_op(addr, OP_EQ, MOCK_TOR_ADDR_PTR);
  return ADDR_POLICY_REJECTED;

  done:
    return 0;
}

/*
 * Structural test for routerset_contains, when given a valid routerset
 * and the address is not rejected by policy.
 */

static addr_policy_result_t rset_contains_no_addr_cmp_addr_to_policy(
              const tor_addr_t *addr, uint16_t port,
              const smartlist_t *policy);
static int rset_contains_no_addr_cmp_addr_to_policy_called = 0;

static void
test_rset_contains_no_addr(void *arg)
{
  routerset_t *set = routerset_new();
  tor_addr_t *addr = MOCK_TOR_ADDR_PTR;
  int contains;
  (void)arg;

  MOCK(compare_tor_addr_to_addr_policy,
       rset_contains_no_addr_cmp_addr_to_policy);

  contains = routerset_contains(set, addr, 0, NULL, NULL, 0);
  routerset_free(set);

  tt_int_op(rset_contains_no_addr_cmp_addr_to_policy_called, OP_EQ, 1);
  tt_int_op(contains, OP_EQ, 0);

  done:
    ;
}

addr_policy_result_t
rset_contains_no_addr_cmp_addr_to_policy(const tor_addr_t *addr, uint16_t port,
    const smartlist_t *policy)
{
  (void)port;
  (void)policy;
  rset_contains_no_addr_cmp_addr_to_policy_called++;
  tt_ptr_op(addr, OP_EQ, MOCK_TOR_ADDR_PTR);

  return ADDR_POLICY_ACCEPTED;

  done:
      return 0;
}

/*
 * Structural test for routerset_contains, when given a valid routerset
 * and the address is NULL.
 */

static addr_policy_result_t rset_contains_null_addr_cmp_addr_to_policy(
                 const tor_addr_t *addr, uint16_t port,
                 const smartlist_t *policy);
static int rset_contains_null_addr_cmp_addr_to_policy_called = 0;

static void
test_rset_contains_null_addr(void *arg)
{
  routerset_t *set = routerset_new();
  int contains;
  (void)arg;

  MOCK(compare_tor_addr_to_addr_policy,
       rset_contains_null_addr_cmp_addr_to_policy);

  contains = routerset_contains(set, NULL, 0, NULL, NULL, 0);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 0);

  done:
    ;
}

addr_policy_result_t
rset_contains_null_addr_cmp_addr_to_policy(
    const tor_addr_t *addr, uint16_t port,
    const smartlist_t *policy)
{
  (void)port;
  (void)policy;
  rset_contains_null_addr_cmp_addr_to_policy_called++;
  tt_ptr_op(addr, OP_EQ, MOCK_TOR_ADDR_PTR);

  return ADDR_POLICY_ACCEPTED;

  done:
    return 0;
}

/*
 * Structural test for routerset_contains, when there is no matching country
 * for the address.
 */

static addr_policy_result_t rset_countries_no_geoip_cmp_addr_to_policy(
                  const tor_addr_t *addr, uint16_t port,
                  const smartlist_t *policy);
static int rset_countries_no_geoip_cmp_addr_to_policy_called = 0;
static int rset_countries_no_geoip_geoip_get_country_by_addr(
                  const tor_addr_t *addr);
static int rset_countries_no_geoip_geoip_get_country_by_addr_called = 0;

static void
test_rset_countries_no_geoip(void *arg)
{
  routerset_t *set = routerset_new();
  int contains = 1;
  (void)arg;

  MOCK(compare_tor_addr_to_addr_policy,
       rset_countries_no_geoip_cmp_addr_to_policy);
  MOCK(geoip_get_country_by_addr,
       rset_countries_no_geoip_geoip_get_country_by_addr);

  set->countries = bitarray_init_zero(1);
  bitarray_set(set->countries, 1);
  contains = routerset_contains(set, MOCK_TOR_ADDR_PTR, 0, NULL, NULL, -1);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 0);
  tt_int_op(rset_countries_no_geoip_cmp_addr_to_policy_called,
            OP_EQ, 1);
  tt_int_op(rset_countries_no_geoip_geoip_get_country_by_addr_called,
            OP_EQ, 1);

  done:
    ;
}

addr_policy_result_t
rset_countries_no_geoip_cmp_addr_to_policy(
    const tor_addr_t *addr, uint16_t port,
    const smartlist_t *policy)
{
  (void)port;
  (void)policy;
  rset_countries_no_geoip_cmp_addr_to_policy_called++;
  tt_ptr_op(addr, OP_EQ, MOCK_TOR_ADDR_PTR);

  done:
    return ADDR_POLICY_ACCEPTED;
}

int
rset_countries_no_geoip_geoip_get_country_by_addr(const tor_addr_t *addr)
{
  rset_countries_no_geoip_geoip_get_country_by_addr_called++;
  tt_ptr_op(addr, OP_EQ, MOCK_TOR_ADDR_PTR);

  done:
    return -1;
}

/*
 * Structural test for routerset_contains, when there a matching country
 * for the address.
 */

static addr_policy_result_t rset_countries_geoip_cmp_addr_to_policy(
              const tor_addr_t *addr, uint16_t port,
              const smartlist_t *policy);
static int rset_countries_geoip_cmp_addr_to_policy_called = 0;
static int rset_countries_geoip_geoip_get_country_by_addr(
                                          const tor_addr_t *addr);
static int rset_countries_geoip_geoip_get_country_by_addr_called = 0;

static void
test_rset_countries_geoip(void *arg)
{
  routerset_t *set = routerset_new();
  int contains = 1;
  (void)arg;

  MOCK(compare_tor_addr_to_addr_policy,
       rset_countries_geoip_cmp_addr_to_policy);
  MOCK(geoip_get_country_by_addr,
       rset_countries_geoip_geoip_get_country_by_addr);

  set->n_countries = 2;
  set->countries = bitarray_init_zero(1);
  bitarray_set(set->countries, 1);
  contains = routerset_contains(set, MOCK_TOR_ADDR_PTR, 0, NULL, NULL, -1);
  routerset_free(set);

  tt_int_op(contains, OP_EQ, 2);
  tt_int_op(
     rset_countries_geoip_cmp_addr_to_policy_called,
     OP_EQ, 1);
  tt_int_op(rset_countries_geoip_geoip_get_country_by_addr_called,
            OP_EQ, 1);

  done:
    ;
}

addr_policy_result_t
rset_countries_geoip_cmp_addr_to_policy(
    const tor_addr_t *addr, uint16_t port,
    const smartlist_t *policy)
{
  (void)port;
  (void)policy;
  rset_countries_geoip_cmp_addr_to_policy_called++;
  tt_ptr_op(addr, OP_EQ, MOCK_TOR_ADDR_PTR);

  done:
    return ADDR_POLICY_ACCEPTED;
}

int
rset_countries_geoip_geoip_get_country_by_addr(const tor_addr_t *addr)
{
  rset_countries_geoip_geoip_get_country_by_addr_called++;
  tt_ptr_op(addr, OP_EQ, MOCK_TOR_ADDR_PTR);

  done:
    return 1;
}

/*
 * Functional test for routerset_add_unknown_ccs, where only_if_some_cc_set
 * is set and there are no country names.
 */

static void
test_rset_add_unknown_ccs_only_flag(void *arg)
{
  routerset_t *set = routerset_new();
  routerset_t **setp = &set;
  int r;
  (void)arg;

  r = routerset_add_unknown_ccs(setp, 1);

  tt_int_op(r, OP_EQ, 0);

  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_add_unknown_ccs, where the set argument
 * is created if passed in as NULL.
 */

/* The mock is only used to stop the test from asserting erroneously. */
static country_t rset_add_unknown_ccs_creates_set_geoip_get_country(
                                                     const char *country);
static int rset_add_unknown_ccs_creates_set_geoip_get_country_called = 0;

static void
test_rset_add_unknown_ccs_creates_set(void *arg)
{
  routerset_t *set = NULL;
  routerset_t **setp = &set;
  int r;
  (void)arg;

  MOCK(geoip_get_country,
       rset_add_unknown_ccs_creates_set_geoip_get_country);

  r = routerset_add_unknown_ccs(setp, 0);

  tt_ptr_op(*setp, OP_NE, NULL);
  tt_int_op(r, OP_EQ, 0);

  done:
    if (set != NULL)
      routerset_free(set);
}

country_t
rset_add_unknown_ccs_creates_set_geoip_get_country(const char *country)
{
  (void)country;
  rset_add_unknown_ccs_creates_set_geoip_get_country_called++;

  return -1;
}

/*
 * Structural test for routerset_add_unknown_ccs, that the "{??}"
 * country code is added to the list.
 */

static country_t rset_add_unknown_ccs_add_unknown_geoip_get_country(
                                                 const char *country);
static int rset_add_unknown_ccs_add_unknown_geoip_get_country_called = 0;
static int rset_add_unknown_ccs_add_unknown_geoip_is_loaded(
                                                 sa_family_t family);
static int rset_add_unknown_ccs_add_unknown_geoip_is_loaded_called = 0;

static void
test_rset_add_unknown_ccs_add_unknown(void *arg)
{
  routerset_t *set = routerset_new();
  routerset_t **setp = &set;
  int r;
  (void)arg;

  MOCK(geoip_get_country,
       rset_add_unknown_ccs_add_unknown_geoip_get_country);
  MOCK(geoip_is_loaded,
       rset_add_unknown_ccs_add_unknown_geoip_is_loaded);

  r = routerset_add_unknown_ccs(setp, 0);

  tt_int_op(r, OP_EQ, 1);
  tt_int_op(smartlist_contains_string(set->country_names, "??"), OP_EQ, 1);
  tt_int_op(smartlist_contains_string(set->list, "{??}"), OP_EQ, 1);

  done:
    if (set != NULL)
      routerset_free(set);
}

country_t
rset_add_unknown_ccs_add_unknown_geoip_get_country(const char *country)
{
  int arg_is_qq, arg_is_a1;

  rset_add_unknown_ccs_add_unknown_geoip_get_country_called++;

  arg_is_qq = !strcmp(country, "??");
  arg_is_a1 = !strcmp(country, "A1");

  tt_int_op(arg_is_qq || arg_is_a1, OP_EQ, 1);

  if (arg_is_qq)
    return 1;

  done:
    return -1;
}

int
rset_add_unknown_ccs_add_unknown_geoip_is_loaded(sa_family_t family)
{
  rset_add_unknown_ccs_add_unknown_geoip_is_loaded_called++;

  tt_int_op(family, OP_EQ, AF_INET);

  done:
    return 0;
}

/*
 * Structural test for routerset_add_unknown_ccs, that the "{a1}"
 * country code is added to the list.
 */

static country_t rset_add_unknown_ccs_add_a1_geoip_get_country(
                                           const char *country);
static int rset_add_unknown_ccs_add_a1_geoip_get_country_called = 0;
static int rset_add_unknown_ccs_add_a1_geoip_is_loaded(sa_family_t family);
static int rset_add_unknown_ccs_add_a1_geoip_is_loaded_called = 0;

static void
test_rset_add_unknown_ccs_add_a1(void *arg)
{
  routerset_t *set = routerset_new();
  routerset_t **setp = &set;
  int r;
  (void)arg;

  MOCK(geoip_get_country,
       rset_add_unknown_ccs_add_a1_geoip_get_country);
  MOCK(geoip_is_loaded,
       rset_add_unknown_ccs_add_a1_geoip_is_loaded);

  r = routerset_add_unknown_ccs(setp, 0);

  tt_int_op(r, OP_EQ, 1);
  tt_int_op(smartlist_contains_string(set->country_names, "a1"), OP_EQ, 1);
  tt_int_op(smartlist_contains_string(set->list, "{a1}"), OP_EQ, 1);

  done:
    if (set != NULL)
      routerset_free(set);
}

country_t
rset_add_unknown_ccs_add_a1_geoip_get_country(const char *country)
{
  int arg_is_qq, arg_is_a1;

  rset_add_unknown_ccs_add_a1_geoip_get_country_called++;

  arg_is_qq = !strcmp(country, "??");
  arg_is_a1 = !strcmp(country, "A1");

  tt_int_op(arg_is_qq || arg_is_a1, OP_EQ, 1);

  if (arg_is_a1)
    return 1;

  done:
    return -1;
}

int
rset_add_unknown_ccs_add_a1_geoip_is_loaded(sa_family_t family)
{
  rset_add_unknown_ccs_add_a1_geoip_is_loaded_called++;

  tt_int_op(family, OP_EQ, AF_INET);

  done:
    return 0;
}

/*
 * Functional test for routerset_contains_extendinfo.
 */

static void
test_rset_contains_extendinfo(void *arg)
{
  routerset_t *set = routerset_new();
  extend_info_t ei;
  int r;
  const char *nickname = "foo";
  (void)arg;

  memset(&ei, 0, sizeof(ei));
  strmap_set_lc(set->names, nickname, (void *)1);
  strncpy(ei.nickname, nickname, sizeof(ei.nickname) - 1);
  ei.nickname[sizeof(ei.nickname) - 1] = '\0';

  r = routerset_contains_extendinfo(set, &ei);

  tt_int_op(r, OP_EQ, 4);
  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_contains_router.
 */

static void
test_rset_contains_router(void *arg)
{
  routerset_t *set = routerset_new();
  routerinfo_t ri;
  country_t country = 1;
  int r;
  const char *nickname = "foo";
  (void)arg;

  memset(&ri, 0, sizeof(ri));
  strmap_set_lc(set->names, nickname, (void *)1);
  ri.nickname = (char *)nickname;

  r = routerset_contains_router(set, &ri, country);
  tt_int_op(r, OP_EQ, 4);

  done:
    routerset_free(set);
}

static void
test_rset_contains_router_ipv4(void *arg)
{
  routerset_t *set;
  routerinfo_t ri;
  country_t country = 1;
  int r;
  const char *s;
  (void) arg;

  /* IPv4 address test. */
  memset(&ri, 0, sizeof(ri));
  set = routerset_new();
  s = "10.0.0.1";
  r = routerset_parse(set, s, "");
  tor_addr_from_ipv4h(&ri.ipv4_addr, 0x0a000001);
  ri.ipv4_orport = 1234;

  r = routerset_contains_router(set, &ri, country);
  tt_int_op(r, OP_EQ, 3);

 done:
  routerset_free(set);
}

static void
test_rset_contains_router_ipv6(void *arg)
{
  routerset_t *set;
  routerinfo_t ri;
  country_t country = 1;
  int r;
  const char *s;
  (void) arg;

  /* IPv6 address test. */
  memset(&ri, 0, sizeof(ri));
  set = routerset_new();
  s = "2600::1";
  r = routerset_parse(set, s, "");
  tor_addr_parse(&ri.ipv6_addr, "2600::1");
  ri.ipv6_orport = 12345;

  r = routerset_contains_router(set, &ri, country);
  tt_int_op(r, OP_EQ, 3);

 done:
  routerset_free(set);
}

/*
 * Functional test for routerset_contains_routerstatus.
 */

// XXX: This is a bit brief. It only populates and tests the nickname fields
// ie., enough to make the containment check succeed. Perhaps it should do
// a bit more or test a bit more.

static void
test_rset_contains_routerstatus(void *arg)
{
  routerset_t *set = routerset_new();
  routerstatus_t rs;
  country_t country = 1;
  int r;
  const char *nickname = "foo";
  (void)arg;

  memset(&rs, 0, sizeof(rs));
  strmap_set_lc(set->names, nickname, (void *)1);
  strncpy(rs.nickname, nickname, sizeof(rs.nickname) - 1);
  rs.nickname[sizeof(rs.nickname) - 1] = '\0';

  r = routerset_contains_routerstatus(set, &rs, country);

  tt_int_op(r, OP_EQ, 4);
  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_contains_node, when the node has no
 * routerset or routerinfo.
 */

static node_t rset_contains_none_mock_node;

static void
test_rset_contains_none(void *arg)
{
  routerset_t *set = routerset_new();
  int r;
  (void)arg;

  memset(&rset_contains_none_mock_node, 0,
         sizeof(rset_contains_none_mock_node));
  rset_contains_none_mock_node.ri = NULL;
  rset_contains_none_mock_node.rs = NULL;

  r = routerset_contains_node(set, &rset_contains_none_mock_node);
  tt_int_op(r, OP_EQ, 0);

  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_contains_node, when the node has a
 * routerset and no routerinfo.
 */

static node_t rset_contains_rs_mock_node;

static void
test_rset_contains_rs(void *arg)
{
  routerset_t *set = routerset_new();
  int r;
  const char *nickname = "foo";
  routerstatus_t rs;
  (void)arg;

  strmap_set_lc(set->names, nickname, (void *)1);

  strncpy(rs.nickname, nickname, sizeof(rs.nickname) - 1);
  rs.nickname[sizeof(rs.nickname) - 1] = '\0';
  memset(&rset_contains_rs_mock_node, 0, sizeof(rset_contains_rs_mock_node));
  rset_contains_rs_mock_node.ri = NULL;
  rset_contains_rs_mock_node.rs = &rs;

  r = routerset_contains_node(set, &rset_contains_rs_mock_node);

  tt_int_op(r, OP_EQ, 4);
  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_contains_node, when the node has no
 * routerset and a routerinfo.
 */

static void
test_rset_contains_routerinfo(void *arg)
{
  routerset_t *set = routerset_new();
  int r;
  const char *nickname = "foo";
  routerinfo_t ri;
  node_t mock_node;
  (void)arg;

  strmap_set_lc(set->names, nickname, (void *)1);

  ri.nickname = (char *)nickname;
  memset(&mock_node, 0, sizeof(mock_node));
  mock_node.ri = &ri;
  mock_node.rs = NULL;

  r = routerset_contains_node(set, &mock_node);

  tt_int_op(r, OP_EQ, 4);
  done:
    routerset_free(set);
}

/*
 * Functional test for routerset_get_all_nodes, when routerset is NULL or
 * the routerset list is NULL.
 */

static void
test_rset_get_all_no_routerset(void *arg)
{
  smartlist_t *out = smartlist_new();
  routerset_t *set = NULL;
  (void)arg;

  tt_int_op(smartlist_len(out), OP_EQ, 0);
  routerset_get_all_nodes(out, NULL, NULL, 0);

  tt_int_op(smartlist_len(out), OP_EQ, 0);

  set = routerset_new();
  smartlist_free(set->list);
  routerset_get_all_nodes(out, NULL, NULL, 0);
  tt_int_op(smartlist_len(out), OP_EQ, 0);

  /* Just recreate list, so we can simply use routerset_free. */
  set->list = smartlist_new();

 done:
  routerset_free(set);
  smartlist_free(out);
}

/*
 * Structural test for routerset_get_all_nodes, when the routerset list
 * is empty.
 */

static const node_t * rset_get_all_l_no_nodes_node_get_by_nickname(
                                       const char *nickname, unsigned flags);
static int rset_get_all_l_no_nodes_node_get_by_nickname_called = 0;
static const char *rset_get_all_l_no_nodes_mock_nickname;

static void
test_rset_get_all_l_no_nodes(void *arg)
{
  smartlist_t *out = smartlist_new();
  routerset_t *set = routerset_new();
  int out_len;
  (void)arg;

  MOCK(node_get_by_nickname,
       rset_get_all_l_no_nodes_node_get_by_nickname);

  rset_get_all_l_no_nodes_mock_nickname = "foo";
  smartlist_add_strdup(set->list, rset_get_all_l_no_nodes_mock_nickname);

  routerset_get_all_nodes(out, set, NULL, 0);
  out_len = smartlist_len(out);

  smartlist_free(out);
  routerset_free(set);

  tt_int_op(out_len, OP_EQ, 0);
  tt_int_op(rset_get_all_l_no_nodes_node_get_by_nickname_called, OP_EQ, 1);

  done:
    ;
}

const node_t *
rset_get_all_l_no_nodes_node_get_by_nickname(const char *nickname,
                                             unsigned flags)
{
  rset_get_all_l_no_nodes_node_get_by_nickname_called++;
  tt_str_op(nickname, OP_EQ, rset_get_all_l_no_nodes_mock_nickname);
  tt_uint_op(flags, OP_EQ, 0);

  done:
    return NULL;
}

/*
 * Structural test for routerset_get_all_nodes, with the running_only flag
 * is set but the nodes are not running.
 */

static const node_t * rset_get_all_l_not_running_node_get_by_nickname(
                                       const char *nickname, unsigned flags);
static int rset_get_all_l_not_running_node_get_by_nickname_called = 0;
static const char *rset_get_all_l_not_running_mock_nickname;
static node_t rset_get_all_l_not_running_mock_node;

static void
test_rset_get_all_l_not_running(void *arg)
{
  smartlist_t *out = smartlist_new();
  routerset_t *set = routerset_new();
  int out_len;
  (void)arg;

  MOCK(node_get_by_nickname,
       rset_get_all_l_not_running_node_get_by_nickname);

  rset_get_all_l_not_running_mock_node.is_running = 0;
  rset_get_all_l_not_running_mock_nickname = "foo";
  smartlist_add_strdup(set->list, rset_get_all_l_not_running_mock_nickname);

  routerset_get_all_nodes(out, set, NULL, 1);
  out_len = smartlist_len(out);

  smartlist_free(out);
  routerset_free(set);

  tt_int_op(out_len, OP_EQ, 0);
  tt_int_op(rset_get_all_l_not_running_node_get_by_nickname_called, OP_EQ, 1);

  done:
    ;
}

const node_t *
rset_get_all_l_not_running_node_get_by_nickname(const char *nickname,
                                                unsigned flags)
{
  rset_get_all_l_not_running_node_get_by_nickname_called++;
  tt_str_op(nickname, OP_EQ, rset_get_all_l_not_running_mock_nickname);
  tt_int_op(flags, OP_EQ, 0);

  done:
    return &rset_get_all_l_not_running_mock_node;
}

/*
 * Structural test for routerset_get_all_nodes.
 */

static const node_t * rset_get_all_list_node_get_by_nickname(
                             const char *nickname, unsigned flags);
static int rset_get_all_list_node_get_by_nickname_called = 0;
static char *rset_get_all_list_mock_nickname;
static node_t rset_get_all_list_mock_node;

static void
test_rset_get_all_list(void *arg)
{
  smartlist_t *out = smartlist_new();
  routerset_t *set = routerset_new();
  int out_len;
  node_t *ent;
  (void)arg;

  MOCK(node_get_by_nickname,
       rset_get_all_list_node_get_by_nickname);

  rset_get_all_list_mock_nickname = tor_strdup("foo");
  smartlist_add(set->list, rset_get_all_list_mock_nickname);

  routerset_get_all_nodes(out, set, NULL, 0);
  out_len = smartlist_len(out);
  ent = (node_t *)smartlist_get(out, 0);

  smartlist_free(out);
  routerset_free(set);

  tt_int_op(out_len, OP_EQ, 1);
  tt_ptr_op(ent, OP_EQ, &rset_get_all_list_mock_node);
  tt_int_op(rset_get_all_list_node_get_by_nickname_called, OP_EQ, 1);

  done:
    ;
}

const node_t *
rset_get_all_list_node_get_by_nickname(const char *nickname, unsigned flags)
{
  rset_get_all_list_node_get_by_nickname_called++;
  tt_str_op(nickname, OP_EQ, rset_get_all_list_mock_nickname);
  tt_int_op(flags, OP_EQ, 0);

  done:
    return &rset_get_all_list_mock_node;
}

/*
 * Structural test for routerset_get_all_nodes, when the nodelist has no nodes.
 */

static const smartlist_t * rset_get_all_n_no_nodes_nodelist_get_list(void);
static int rset_get_all_n_no_nodes_nodelist_get_list_called = 0;

static smartlist_t *rset_get_all_n_no_nodes_mock_smartlist;
static void
test_rset_get_all_n_no_nodes(void *arg)
{
  routerset_t *set = routerset_new();
  smartlist_t *out = smartlist_new();
  int r;
  (void)arg;

  MOCK(nodelist_get_list,
       rset_get_all_n_no_nodes_nodelist_get_list);

  smartlist_add_strdup(set->country_names, "{xx}");
  rset_get_all_n_no_nodes_mock_smartlist = smartlist_new();

  routerset_get_all_nodes(out, set, NULL, 1);
  r = smartlist_len(out);
  routerset_free(set);
  smartlist_free(out);
  smartlist_free(rset_get_all_n_no_nodes_mock_smartlist);

  tt_int_op(r, OP_EQ, 0);
  tt_int_op(rset_get_all_n_no_nodes_nodelist_get_list_called, OP_EQ, 1);

  done:
    ;
}

const smartlist_t *
rset_get_all_n_no_nodes_nodelist_get_list(void)
{
  rset_get_all_n_no_nodes_nodelist_get_list_called++;

  return rset_get_all_n_no_nodes_mock_smartlist;
}

/*
 * Structural test for routerset_get_all_nodes, with a non-list routerset
 * the running_only flag is set, but the nodes are not running.
 */

static const smartlist_t * rset_get_all_n_not_running_nodelist_get_list(void);
static int rset_get_all_n_not_running_nodelist_get_list_called = 0;

static smartlist_t *rset_get_all_n_not_running_mock_smartlist;
static node_t rset_get_all_n_not_running_mock_node;

static void
test_rset_get_all_n_not_running(void *arg)
{
  routerset_t *set = routerset_new();
  smartlist_t *out = smartlist_new();
  int r;
  (void)arg;

  MOCK(nodelist_get_list,
       rset_get_all_n_not_running_nodelist_get_list);

  smartlist_add_strdup(set->country_names, "{xx}");
  rset_get_all_n_not_running_mock_smartlist = smartlist_new();
  rset_get_all_n_not_running_mock_node.is_running = 0;
  smartlist_add(rset_get_all_n_not_running_mock_smartlist,
                (void *)&rset_get_all_n_not_running_mock_node);

  routerset_get_all_nodes(out, set, NULL, 1);
  r = smartlist_len(out);
  routerset_free(set);
  smartlist_free(out);
  smartlist_free(rset_get_all_n_not_running_mock_smartlist);

  tt_int_op(r, OP_EQ, 0);
  tt_int_op(rset_get_all_n_not_running_nodelist_get_list_called, OP_EQ, 1);

  done:
    ;
}

const smartlist_t *
rset_get_all_n_not_running_nodelist_get_list(void)
{
  rset_get_all_n_not_running_nodelist_get_list_called++;

  return rset_get_all_n_not_running_mock_smartlist;
}

/*
 * Functional test for routerset_subtract_nodes.
 */

static void
test_rset_subtract_nodes(void *arg)
{
  routerset_t *set = routerset_new();
  smartlist_t *list = smartlist_new();
  const char *nickname = "foo";
  routerinfo_t ri;
  node_t mock_node;
  (void)arg;

  strmap_set_lc(set->names, nickname, (void *)1);

  ri.nickname = (char *)nickname;
  mock_node.rs = NULL;
  mock_node.ri = &ri;
  smartlist_add(list, (void *)&mock_node);

  tt_int_op(smartlist_len(list), OP_NE, 0);
  routerset_subtract_nodes(list, set);

  tt_int_op(smartlist_len(list), OP_EQ, 0);
  done:
    routerset_free(set);
    smartlist_free(list);
}

/*
 * Functional test for routerset_subtract_nodes, with a NULL routerset.
 */

static void
test_rset_subtract_nodes_null_routerset(void *arg)
{
  routerset_t *set = NULL;
  smartlist_t *list = smartlist_new();
  const char *nickname = "foo";
  routerinfo_t ri;
  node_t mock_node;
  (void)arg;

  ri.nickname = (char *)nickname;
  mock_node.ri = &ri;
  smartlist_add(list, (void *)&mock_node);

  tt_int_op(smartlist_len(list), OP_NE, 0);
  routerset_subtract_nodes(list, set);

  tt_int_op(smartlist_len(list), OP_NE, 0);
  done:
    routerset_free(set);
    smartlist_free(list);
}

/*
 * Functional test for routerset_to_string.
 */

static void
test_rset_to_string(void *arg)
{
  routerset_t *set = NULL;
  char *s = NULL;
  (void)arg;

  set = NULL;
  s = routerset_to_string(set);
  tt_str_op(s, OP_EQ, "");
  tor_free(s);

  set = routerset_new();
  s = routerset_to_string(set);
  tt_str_op(s, OP_EQ, "");
  tor_free(s);
  routerset_free(set); set = NULL;

  set = routerset_new();
  smartlist_add(set->list, tor_strndup("a", 1));
  s = routerset_to_string(set);
  tt_str_op(s, OP_EQ, "a");
  tor_free(s);
  routerset_free(set); set = NULL;

  set = routerset_new();
  smartlist_add(set->list, tor_strndup("a", 1));
  smartlist_add(set->list, tor_strndup("b", 1));
  s = routerset_to_string(set);
  tt_str_op(s, OP_EQ, "a,b");
  tor_free(s);
  routerset_free(set); set = NULL;

 done:
  tor_free(s);
  routerset_free(set);
}

/*
 * Functional test for routerset_equal, with both routersets empty.
 */

static void
test_rset_equal_empty_empty(void *arg)
{
  routerset_t *a = routerset_new(), *b = routerset_new();
  int r;
  (void)arg;

  r = routerset_equal(a, b);
  routerset_free(a);
  routerset_free(b);

  tt_int_op(r, OP_EQ, 1);

  done:
    ;
}

/*
 * Functional test for routerset_equal, with one routersets empty.
 */

static void
test_rset_equal_empty_not_empty(void *arg)
{
  routerset_t *a = routerset_new(), *b = routerset_new();
  int r;
  (void)arg;

  smartlist_add_strdup(b->list, "{xx}");
  r = routerset_equal(a, b);
  routerset_free(a);
  routerset_free(b);

  tt_int_op(r, OP_EQ, 0);
  done:
    ;
}

/*
 * Functional test for routerset_equal, with the routersets having
 * differing lengths.
 */

static void
test_rset_equal_differing_lengths(void *arg)
{
  routerset_t *a = routerset_new(), *b = routerset_new();
  int r;
  (void)arg;

  smartlist_add_strdup(a->list, "{aa}");
  smartlist_add_strdup(b->list, "{b1}");
  smartlist_add_strdup(b->list, "{b2}");
  r = routerset_equal(a, b);
  routerset_free(a);
  routerset_free(b);

  tt_int_op(r, OP_EQ, 0);
  done:
    ;
}

/*
 * Functional test for routerset_equal, with the routersets being
 * different.
 */

static void
test_rset_equal_unequal(void *arg)
{
  routerset_t *a = routerset_new(), *b = routerset_new();
  int r;
  (void)arg;

  smartlist_add_strdup(a->list, "foo");
  smartlist_add_strdup(b->list, "bar");
  r = routerset_equal(a, b);
  routerset_free(a);
  routerset_free(b);

  tt_int_op(r, OP_EQ, 0);
  done:
    ;
}

/*
 * Functional test for routerset_equal, with the routersets being
 * equal.
 */

static void
test_rset_equal_equal(void *arg)
{
  routerset_t *a = routerset_new(), *b = routerset_new();
  int r;
  (void)arg;

  smartlist_add_strdup(a->list, "foo");
  smartlist_add_strdup(b->list, "foo");
  r = routerset_equal(a, b);
  routerset_free(a);
  routerset_free(b);

  tt_int_op(r, OP_EQ, 1);
  done:
    ;
}

/*
 * Structural test for routerset_free, where the routerset is NULL.
 */

static void rset_free_null_routerset_smartlist_free_(smartlist_t *sl);
static int rset_free_null_routerset_smartlist_free__called = 0;

static void
test_rset_free_null_routerset(void *arg)
{
  (void)arg;

  MOCK(smartlist_free_,
       rset_free_null_routerset_smartlist_free_);

  routerset_free_(NULL);

  tt_int_op(rset_free_null_routerset_smartlist_free__called, OP_EQ, 0);

  done:
    ;
}

void
rset_free_null_routerset_smartlist_free_(smartlist_t *s)
{
  (void)s;
  rset_free_null_routerset_smartlist_free__called++;
}

/*
 * Structural test for routerset_free.
 */

static void rset_free_smartlist_free_(smartlist_t *sl);
static int rset_free_smartlist_free__called = 0;
static void rset_free_strmap_free_(strmap_t *map, void (*free_val)(void*));
static int rset_free_strmap_free__called = 0;
static void rset_free_digestmap_free_(digestmap_t *map,
                                      void (*free_val)(void*));
static int rset_free_digestmap_free__called = 0;

static void
test_rset_free(void *arg)
{
  routerset_t *routerset = routerset_new();
  (void)arg;

  MOCK(smartlist_free_,
       rset_free_smartlist_free_);
  MOCK(strmap_free_,
       rset_free_strmap_free_);
  MOCK(digestmap_free_,
       rset_free_digestmap_free_);

  routerset_free(routerset);

  tt_int_op(rset_free_smartlist_free__called, OP_NE, 0);
  tt_int_op(rset_free_strmap_free__called, OP_NE, 0);
  tt_int_op(rset_free_digestmap_free__called, OP_NE, 0);

  done:
    ;
}

void
rset_free_smartlist_free_(smartlist_t *s)
{
  rset_free_smartlist_free__called++;
  smartlist_free___real(s);
}

void
rset_free_strmap_free_(strmap_t *map, void (*free_val)(void*))
{
  rset_free_strmap_free__called++;
  strmap_free___real(map, free_val);
}

void
rset_free_digestmap_free_(digestmap_t *map, void (*free_val)(void*))
{
  rset_free_digestmap_free__called++;
  digestmap_free___real(map, free_val);
}

struct testcase_t routerset_tests[] = {
  { "new", test_rset_new, TT_FORK, NULL, NULL },
  { "get_countryname", test_rset_get_countryname, TT_FORK, NULL, NULL },
  { "is_list", test_rset_is_list, TT_FORK, NULL, NULL },
  { "needs_geoip", test_rset_needs_geoip, TT_FORK, NULL, NULL },
  { "is_empty", test_rset_is_empty, TT_FORK, NULL, NULL },
  { "contains_null_set_or_list", test_rset_contains_null_set_or_list,
    TT_FORK, NULL, NULL },
  { "contains_nickname", test_rset_contains_nickname, TT_FORK, NULL, NULL },
  { "contains_null_nickname", test_rset_contains_null_nickname,
    TT_FORK, NULL, NULL },
  { "contains_no_nickname", test_rset_contains_no_nickname,
    TT_FORK, NULL, NULL },
  { "contains_digest", test_rset_contains_digest, TT_FORK, NULL, NULL },
  { "contains_no_digest", test_rset_contains_no_digest, TT_FORK, NULL, NULL },
  { "contains_null_digest", test_rset_contains_null_digest,
    TT_FORK, NULL, NULL },
  { "contains_addr", test_rset_contains_addr, TT_FORK, NULL, NULL },
  { "contains_no_addr", test_rset_contains_no_addr, TT_FORK, NULL, NULL },
  { "contains_null_addr", test_rset_contains_null_addr, TT_FORK, NULL, NULL },
  { "contains_countries_no_geoip", test_rset_countries_no_geoip,
    TT_FORK, NULL, NULL },
  { "contains_countries_geoip", test_rset_countries_geoip,
    TT_FORK, NULL, NULL },
  { "add_unknown_ccs_only_flag", test_rset_add_unknown_ccs_only_flag,
    TT_FORK, NULL, NULL },
  { "add_unknown_ccs_creates_set", test_rset_add_unknown_ccs_creates_set,
    TT_FORK, NULL, NULL },
  { "add_unknown_ccs_add_unknown", test_rset_add_unknown_ccs_add_unknown,
    TT_FORK, NULL, NULL },
  { "add_unknown_ccs_add_a1", test_rset_add_unknown_ccs_add_a1,
    TT_FORK, NULL, NULL },
  { "contains_extendinfo", test_rset_contains_extendinfo,
    TT_FORK, NULL, NULL },
  { "contains_router", test_rset_contains_router, TT_FORK, NULL, NULL },
  { "contains_router_ipv4", test_rset_contains_router_ipv4,
    TT_FORK, NULL, NULL },
  { "contains_router_ipv6", test_rset_contains_router_ipv6,
    TT_FORK, NULL, NULL },
  { "contains_routerstatus", test_rset_contains_routerstatus,
    TT_FORK, NULL, NULL },
  { "contains_none", test_rset_contains_none, TT_FORK, NULL, NULL },
  { "contains_routerinfo", test_rset_contains_routerinfo,
    TT_FORK, NULL, NULL },
  { "contains_rs", test_rset_contains_rs, TT_FORK, NULL, NULL },
  { "get_all_no_routerset", test_rset_get_all_no_routerset,
    TT_FORK, NULL, NULL },
  { "get_all_l_no_nodes", test_rset_get_all_l_no_nodes, TT_FORK, NULL, NULL },
  { "get_all_l_not_running", test_rset_get_all_l_not_running,
    TT_FORK, NULL, NULL },
  { "get_all_list", test_rset_get_all_list, TT_FORK, NULL, NULL },
  { "get_all_n_no_nodes", test_rset_get_all_n_no_nodes, TT_FORK, NULL, NULL },
  { "get_all_n_not_running", test_rset_get_all_n_not_running,
    TT_FORK, NULL, NULL },
  { "refresh_geoip_not_loaded", test_rset_refresh_geoip_not_loaded,
    TT_FORK, NULL, NULL },
  { "refresh_no_countries", test_rset_refresh_no_countries,
    TT_FORK, NULL, NULL },
  { "refresh_one_valid_country", test_rset_refresh_one_valid_country,
    TT_FORK, NULL, NULL },
  { "refresh_one_invalid_country", test_rset_refresh_one_invalid_country,
    TT_FORK, NULL, NULL },
  { "union_source_bad", test_rset_union_source_bad, TT_FORK, NULL, NULL },
  { "union_one", test_rset_union_one, TT_FORK, NULL, NULL },
  { "parse_malformed", test_rset_parse_malformed, TT_FORK, NULL, NULL },
  { "parse_valid_hexdigest", test_rset_parse_valid_hexdigest,
    TT_FORK, NULL, NULL },
  { "parse_valid_nickname", test_rset_parse_valid_nickname,
    TT_FORK, NULL, NULL },
  { "parse_get_countryname", test_rset_parse_get_countryname,
    TT_FORK, NULL, NULL },
  { "parse_policy_wildcard", test_rset_parse_policy_wildcard,
    TT_FORK, NULL, NULL },
  { "parse_policy_ipv4", test_rset_parse_policy_ipv4, TT_FORK, NULL, NULL },
  { "parse_policy_ipv6", test_rset_parse_policy_ipv6, TT_FORK, NULL, NULL },
  { "subtract_nodes", test_rset_subtract_nodes, TT_FORK, NULL, NULL },
  { "subtract_nodes_null_routerset", test_rset_subtract_nodes_null_routerset,
    TT_FORK, NULL, NULL },
  { "to_string", test_rset_to_string, TT_FORK, NULL, NULL },
  { "equal_empty_empty", test_rset_equal_empty_empty, TT_FORK, NULL, NULL },
  { "equal_empty_not_empty", test_rset_equal_empty_not_empty,
    TT_FORK, NULL, NULL },
  { "equal_differing_lengths", test_rset_equal_differing_lengths,
    TT_FORK, NULL, NULL },
  { "equal_unequal", test_rset_equal_unequal, TT_FORK, NULL, NULL },
  { "equal_equal", test_rset_equal_equal, TT_FORK, NULL, NULL },
  { "free_null_routerset", test_rset_free_null_routerset,
    TT_FORK, NULL, NULL },
  { "free", test_rset_free, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
