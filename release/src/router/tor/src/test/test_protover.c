/* Copyright (c) 2016-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define PROTOVER_PRIVATE

#include "orconfig.h"
#include "test/test.h"

#include "core/or/protover.h"

#include "core/or/or.h"
#include "core/or/connection_or.h"
#include "lib/tls/tortls.h"

static void
test_protover_parse(void *arg)
{
  (void) arg;
#ifdef HAVE_RUST
  /** This test is disabled on rust builds, because it only exists to test
   * internal C functions. */
  tt_skip();
 done:
  ;
#else
  char *re_encoded = NULL;

  const char *orig = "Foo=1,3 Bar=3 Baz= Quux=9-12,14,15-16,900";
  smartlist_t *elts = parse_protocol_list(orig);

  tt_assert(elts);
  tt_int_op(smartlist_len(elts), OP_EQ, 4);

  const proto_entry_t *e;
  const proto_range_t *r;
  e = smartlist_get(elts, 0);
  tt_str_op(e->name, OP_EQ, "Foo");
  tt_int_op(smartlist_len(e->ranges), OP_EQ, 2);
  {
    r = smartlist_get(e->ranges, 0);
    tt_int_op(r->low, OP_EQ, 1);
    tt_int_op(r->high, OP_EQ, 1);

    r = smartlist_get(e->ranges, 1);
    tt_int_op(r->low, OP_EQ, 3);
    tt_int_op(r->high, OP_EQ, 3);
  }

  e = smartlist_get(elts, 1);
  tt_str_op(e->name, OP_EQ, "Bar");
  tt_int_op(smartlist_len(e->ranges), OP_EQ, 1);
  {
    r = smartlist_get(e->ranges, 0);
    tt_int_op(r->low, OP_EQ, 3);
    tt_int_op(r->high, OP_EQ, 3);
  }

  e = smartlist_get(elts, 2);
  tt_str_op(e->name, OP_EQ, "Baz");
  tt_int_op(smartlist_len(e->ranges), OP_EQ, 0);

  e = smartlist_get(elts, 3);
  tt_str_op(e->name, OP_EQ, "Quux");
  tt_int_op(smartlist_len(e->ranges), OP_EQ, 4);
  {
    r = smartlist_get(e->ranges, 0);
    tt_int_op(r->low, OP_EQ, 9);
    tt_int_op(r->high, OP_EQ, 12);

    r = smartlist_get(e->ranges, 1);
    tt_int_op(r->low, OP_EQ, 14);
    tt_int_op(r->high, OP_EQ, 14);

    r = smartlist_get(e->ranges, 2);
    tt_int_op(r->low, OP_EQ, 15);
    tt_int_op(r->high, OP_EQ, 16);

    r = smartlist_get(e->ranges, 3);
    tt_int_op(r->low, OP_EQ, 900);
    tt_int_op(r->high, OP_EQ, 900);
  }

  re_encoded = encode_protocol_list(elts);
  tt_assert(re_encoded);
  tt_str_op(re_encoded, OP_EQ, orig);

 done:
  if (elts)
    SMARTLIST_FOREACH(elts, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(elts);
  tor_free(re_encoded);
#endif
}

static void
test_protover_parse_fail(void *arg)
{
  (void)arg;
#ifdef HAVE_RUST
  /** This test is disabled on rust builds, because it only exists to test
   * internal C functions. */
  tt_skip();
#else
  smartlist_t *elts;

  /* random junk */
  elts = parse_protocol_list("!!3@*");
  tt_ptr_op(elts, OP_EQ, NULL);

  /* Missing equals sign in an entry */
  elts = parse_protocol_list("Link=4 Haprauxymatyve Desc=9");
  tt_ptr_op(elts, OP_EQ, NULL);

  /* Missing word. */
  elts = parse_protocol_list("Link=4 =3 Desc=9");
  tt_ptr_op(elts, OP_EQ, NULL);

  /* Broken numbers */
  elts = parse_protocol_list("Link=fred");
  tt_ptr_op(elts, OP_EQ, NULL);
  elts = parse_protocol_list("Link=1,fred");
  tt_ptr_op(elts, OP_EQ, NULL);
  elts = parse_protocol_list("Link=1,fred,3");
  tt_ptr_op(elts, OP_EQ, NULL);

  /* Broken range */
  elts = parse_protocol_list("Link=1,9-8,3");
  tt_ptr_op(elts, OP_EQ, NULL);

  /* Protocol name too long */
  elts = parse_protocol_list("DoSaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                           "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  tt_ptr_op(elts, OP_EQ, NULL);

#endif
 done:
  ;
}

static void
test_protover_vote(void *arg)
{
  (void) arg;

  smartlist_t *lst = smartlist_new();
  char *result = protover_compute_vote(lst, 1);

  tt_str_op(result, OP_EQ, "");
  tor_free(result);

  smartlist_add(lst, (void*) "Foo=1-10,500 Bar=1,3-7,8");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Bar=1,3-8 Foo=1-10,500");
  tor_free(result);

  smartlist_add(lst, (void*) "Quux=123-456,78 Bar=2-6,8 Foo=9");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Bar=1-8 Foo=1-10,500 Quux=78,123-456");
  tor_free(result);

  result = protover_compute_vote(lst, 2);
  tt_str_op(result, OP_EQ, "Bar=3-6,8 Foo=9");
  tor_free(result);

  /* High threshold */
  result = protover_compute_vote(lst, 3);
  tt_str_op(result, OP_EQ, "");
  tor_free(result);

  /* Don't count double-voting. */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Foo=1 Foo=1");
  smartlist_add(lst, (void*) "Bar=1-2,2-3");
  result = protover_compute_vote(lst, 2);
  tt_str_op(result, OP_EQ, "");
  tor_free(result);

  /* Bad votes: the result must be empty */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Faux=10-5");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "");
  tor_free(result);

  /* This fails, since "-0" is not valid. */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Faux=-0");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "");
  tor_free(result);

  /* Vote large protover lists that are just below the threshold */

  /* Just below the threshold: Rust */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Sleen=1-500");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Sleen=1-500");
  tor_free(result);

  /* Just below the threshold: C */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Sleen=1-65536");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Sleen=1-65536");
  tor_free(result);

  /* Large protover lists that exceed the threshold */

  /* By adding two votes, C allows us to exceed the limit */
  smartlist_add(lst, (void*) "Sleen=1-65536");
  smartlist_add(lst, (void*) "Sleen=100000");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Sleen=1-65536,100000");
  tor_free(result);

  /* Large integers */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Sleen=4294967294");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Sleen=4294967294");
  tor_free(result);

  /* This parses, but fails at the vote stage */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Sleen=4294967295");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "");
  tor_free(result);

  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Sleen=4294967296");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "");
  tor_free(result);

  /* Protocol name too long */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "DoSaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "");
  tor_free(result);

 done:
  tor_free(result);
  smartlist_free(lst);
}

static void
test_protover_all_supported(void *arg)
{
  (void)arg;
  char *msg = NULL;

  tt_assert(protover_all_supported(NULL, &msg));
  tt_ptr_op(msg, OP_EQ, NULL);

  tt_assert(protover_all_supported("", &msg));
  tt_ptr_op(msg, OP_EQ, NULL);

  // Some things that we do support
  tt_assert(protover_all_supported("Link=3-4", &msg));
  tt_ptr_op(msg, OP_EQ, NULL);
  tt_assert(protover_all_supported("Link=3-4 Desc=2", &msg));
  tt_ptr_op(msg, OP_EQ, NULL);

  // Some things we don't support
  tt_assert(! protover_all_supported("Wombat=9", NULL));
  tt_assert(! protover_all_supported("Wombat=9", &msg));
  tt_str_op(msg, OP_EQ, "Wombat=9");
  tor_free(msg);
  tt_assert(! protover_all_supported("Link=999", &msg));
  tt_str_op(msg, OP_EQ, "Link=999");
  tor_free(msg);

  // Mix of things we support and things we don't
  tt_assert(! protover_all_supported("Link=3-4 Wombat=9", &msg));
  tt_str_op(msg, OP_EQ, "Wombat=9");
  tor_free(msg);

  /* Mix of things we support and don't support within a single protocol
   * which we do support */
  tt_assert(! protover_all_supported("Link=3-999", &msg));
  tt_str_op(msg, OP_EQ, "Link=6-999");
  tor_free(msg);
  tt_assert(! protover_all_supported("Link=1-3,345-666", &msg));
  tt_str_op(msg, OP_EQ, "Link=345-666");
  tor_free(msg);
  tt_assert(! protover_all_supported("Link=1-3,5-12", &msg));
  tt_str_op(msg, OP_EQ, "Link=6-12");
  tor_free(msg);

  /* Mix of protocols we do support and some we don't, where the protocols
   * we do support have some versions we don't support. */
  tt_assert(! protover_all_supported("Link=1-3,5-12 Quokka=9000-9001", &msg));
  tt_str_op(msg, OP_EQ, "Link=6-12 Quokka=9000-9001");
  tor_free(msg);

  /* We shouldn't be able to DoS ourselves parsing a large range. */
  tt_assert(! protover_all_supported("Sleen=1-2147483648", &msg));
  tt_str_op(msg, OP_EQ, "Sleen=1-2147483648");
  tor_free(msg);

  /* This case is allowed. */
  tt_assert(! protover_all_supported("Sleen=1-4294967294", &msg));
  tt_str_op(msg, OP_EQ, "Sleen=1-4294967294");
  tor_free(msg);

  /* If we get a (barely) valid (but unsupported list, we say "yes, that's
   * supported." */
  tt_assert(protover_all_supported("Fribble=", &msg));
  tt_ptr_op(msg, OP_EQ, NULL);

  /* If we get a completely unparseable list, protover_all_supported should
   * hit a fatal assertion for BUG(entries == NULL). */
  tor_capture_bugs_(1);
  tt_assert(protover_all_supported("Fribble", &msg));
  tor_end_capture_bugs_();

  /* If we get a completely unparseable list, protover_all_supported should
   * hit a fatal assertion for BUG(entries == NULL). */
  tor_capture_bugs_(1);
  tt_assert(protover_all_supported("Sleen=1-4294967295", &msg));
  tor_end_capture_bugs_();

  /* Protocol name too long */
#ifndef HAVE_RUST // XXXXXX ?????
  tor_capture_bugs_(1);
  tt_assert(protover_all_supported(
                 "DoSaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 "aaaaaaaaaaaa=1-65536", &msg));
  tor_end_capture_bugs_();
#endif

 done:
  tor_end_capture_bugs_();
  tor_free(msg);
}

static void
test_protover_list_supports_protocol_returns_true(void *arg)
{
  (void)arg;

  const char *protocols = "Link=1";
  int is_supported = protocol_list_supports_protocol(protocols, PRT_LINK, 1);
  tt_int_op(is_supported, OP_EQ, 1);

 done:
  ;
}

static void
test_protover_list_supports_protocol_for_unsupported_returns_false(void *arg)
{
  (void)arg;

  const char *protocols = "Link=1";
  int is_supported = protocol_list_supports_protocol(protocols, PRT_LINK, 10);
  tt_int_op(is_supported, OP_EQ, 0);

 done:
  ;
}

static void
test_protover_supports_version(void *arg)
{
  (void)arg;

  tt_assert(protocol_list_supports_protocol("Link=3-6", PRT_LINK, 3));
  tt_assert(protocol_list_supports_protocol("Link=3-6", PRT_LINK, 6));
  tt_assert(!protocol_list_supports_protocol("Link=3-6", PRT_LINK, 7));
  tt_assert(!protocol_list_supports_protocol("Link=3-6", PRT_LINKAUTH, 3));

  tt_assert(!protocol_list_supports_protocol("Link=4-6 LinkAuth=3",
                                            PRT_LINKAUTH, 2));
  tt_assert(protocol_list_supports_protocol("Link=4-6 LinkAuth=3",
                                            PRT_LINKAUTH, 3));
  tt_assert(!protocol_list_supports_protocol("Link=4-6 LinkAuth=3",
                                             PRT_LINKAUTH, 4));
  tt_assert(!protocol_list_supports_protocol_or_later("Link=4-6 LinkAuth=3",
                                             PRT_LINKAUTH, 4));
  tt_assert(protocol_list_supports_protocol_or_later("Link=4-6 LinkAuth=3",
                                             PRT_LINKAUTH, 3));
  tt_assert(protocol_list_supports_protocol_or_later("Link=4-6 LinkAuth=3",
                                             PRT_LINKAUTH, 2));

  tt_assert(!protocol_list_supports_protocol_or_later("Link=4-6 LinkAuth=3",
                                                      PRT_DESC, 2));
 done:
 ;
}

/* This could be MAX_PROTOCOLS_TO_EXPAND, but that's not exposed by protover */
#define MAX_PROTOCOLS_TO_TEST 1024

/* LinkAuth and Relay protocol versions.
 * Hard-coded here, because they are not in the code, or not exposed in the
 * headers. */
#define PROTOVER_LINKAUTH_V1 1
#define PROTOVER_LINKAUTH_V3 3

#define PROTOVER_RELAY_V1 1
#define PROTOVER_RELAY_V2 2

/* Highest supported HSv2 introduce protocol version.
 * Hard-coded here, because it does not appear anywhere in the code.
 * It's not clear if we actually support version 2, see #25068. */
#define PROTOVER_HSINTRO_V2 3

/* HSv2 Rend and HSDir protocol versions.
 * Hard-coded here, because they do not appear anywhere in the code. */
#define PROTOVER_HS_RENDEZVOUS_POINT_V2 1
#define PROTOVER_HSDIR_V2 1

/* DirCache, Desc, Microdesc, and Cons protocol versions.
 * Hard-coded here, because they do not appear anywhere in the code. */
#define PROTOVER_DIRCACHE_V1 1
#define PROTOVER_DIRCACHE_V2 2

#define PROTOVER_DESC_V1 1
#define PROTOVER_DESC_V2 2

#define PROTOVER_MICRODESC_V1 1
#define PROTOVER_MICRODESC_V2 2

#define PROTOVER_CONS_V1 1
#define PROTOVER_CONS_V2 2

/* Make sure we haven't forgotten any supported protocols */
static void
test_protover_supported_protocols(void *arg)
{
  (void)arg;

  const char *supported_protocols = protover_get_supported_protocols();

  /* Test for new Link in the code, that hasn't been added to supported
   * protocols */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_LINK,
                                            MAX_LINK_PROTO));
  for (uint16_t i = 0; i < MAX_PROTOCOLS_TO_TEST; i++) {
    if (is_or_protocol_version_known(i)) {
      tt_assert(protocol_list_supports_protocol(supported_protocols,
                                                PRT_LINK,
                                                i));
    }
  }

#ifdef HAVE_WORKING_TOR_TLS_GET_TLSSECRETS
  /* Legacy LinkAuth does not appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_LINKAUTH,
                                            PROTOVER_LINKAUTH_V1));
#endif
  /* Latest LinkAuth is not exposed in the headers. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_LINKAUTH,
                                            PROTOVER_LINKAUTH_V3));
  /* Is there any way to test for new LinkAuth? */

  /* Relay protovers do not appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_RELAY,
                                            PROTOVER_RELAY_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_RELAY,
                                            PROTOVER_RELAY_V2));
  /* Is there any way to test for new Relay? */

  /* We could test legacy HSIntro by calling rend_service_update_descriptor(),
   * and checking the protocols field. But that's unlikely to change, so
   * we just use a hard-coded value. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSINTRO,
                                            PROTOVER_HSINTRO_V2));
  /* Test for HSv3 HSIntro */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSINTRO,
                                            PROTOVER_HS_INTRO_V3));
  /* Is there any way to test for new HSIntro? */

  /* Legacy HSRend does not appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSREND,
                                            PROTOVER_HS_RENDEZVOUS_POINT_V2));
  /* Test for HSv3 HSRend */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSREND,
                                            PROTOVER_HS_RENDEZVOUS_POINT_V3));
  /* Is there any way to test for new HSRend? */

  /* Legacy HSDir does not appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSDIR,
                                            PROTOVER_HSDIR_V2));
  /* Test for HSv3 HSDir */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSDIR,
                                            PROTOVER_HSDIR_V3));
  /* Is there any way to test for new HSDir? */

  /* No DirCache versions appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_DIRCACHE,
                                            PROTOVER_DIRCACHE_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_DIRCACHE,
                                            PROTOVER_DIRCACHE_V2));
  /* Is there any way to test for new DirCache? */

  /* No Desc versions appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_DESC,
                                            PROTOVER_DESC_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_DESC,
                                            PROTOVER_DESC_V2));
  /* Is there any way to test for new Desc? */

  /* No Microdesc versions appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_MICRODESC,
                                            PROTOVER_MICRODESC_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_MICRODESC,
                                            PROTOVER_MICRODESC_V2));
  /* Is there any way to test for new Microdesc? */

  /* No Cons versions appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_CONS,
                                            PROTOVER_CONS_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_CONS,
                                            PROTOVER_CONS_V2));
  /* Is there any way to test for new Cons? */

 done:
 ;
}

static void
test_protover_vote_roundtrip(void *args)
{
  (void) args;
  static const struct {
    const char *input;
    const char *expected_output;
  } examples[] = {
    { "Risqu\u00e9=1", NULL },
    { ",,,=1", NULL },
    { "\xc1=1", NULL },
    { "Foo_Bar=1", NULL },
    { "Fkrkljdsf", NULL },
    { "Zn=4294967295", NULL },
    { "Zn=4294967295-1", NULL },
    { "Zn=4294967293-4294967295", NULL },
    /* Will fail because of 4294967295. */
    { "Foo=1,3 Bar=3 Baz= Quux=9-12,14,15-16,900 Zn=1,4294967295",
       NULL },
    { "Foo=1,3 Bar=3 Baz= Quux=9-12,14,15-16,900 Zn=1,4294967294",
      "Bar=3 Foo=1,3 Quux=9-12,14-16,900 Zn=1,4294967294" },
    { "Zu16=1,65536", "Zu16=1,65536" },
    { "N-1=1,2", "N-1=1-2" },
    { "-1=4294967295", NULL },
    { "-1=3", "-1=3" },
    /* junk. */
    { "!!3@*", NULL },
    /* Missing equals sign */
    { "Link=4 Haprauxymatyve Desc=9", NULL },
    { "Link=4 Haprauxymatyve=7 Desc=9",
      "Desc=9 Haprauxymatyve=7 Link=4" },
    { "=10-11", NULL },
    { "X=10-11", "X=10-11" },
    { "Link=4 =3 Desc=9", NULL },
    { "Link=4 Z=3 Desc=9", "Desc=9 Link=4 Z=3" },
    { "Link=fred", NULL },
    { "Link=1,fred", NULL },
    { "Link=1,fred,3", NULL },
    { "Link=1,9-8,3", NULL },
    { "Faux=-0", NULL },
    { "Faux=0--0", NULL },
    { "Faux=-1", NULL },
    { "Faux=-1-3", NULL },
    { "Faux=1--1", NULL },
    { "Link=1-2-", NULL },
    { "Link=1-2-3", NULL },
    { "Faux=1-2-", NULL },
    { "Faux=1-2-3", NULL },
    { "Link=\t1,3", NULL },
    { "Link=1\n,3", NULL },
    { "Faux=1,\r3", NULL },
    { "Faux=1,3\f", NULL },
    /* Large integers */
    { "Link=4294967296", NULL },
    /* Large range */
    { "Sleen=1-501", "Sleen=1-501" },
    { "Sleen=1-65537", NULL },
    /* Both C/Rust implementations should be able to handle this mild DoS. */
    { "Sleen=1-2147483648", NULL },
    /* Rust tests are built in debug mode, so ints are bounds-checked. */
    { "Sleen=1-4294967295", NULL },
  };
  unsigned u;
  smartlist_t *votes = smartlist_new();
  char *result = NULL;

  for (u = 0; u < ARRAY_LENGTH(examples); ++u) {
    const char *input = examples[u].input;
    const char *expected_output = examples[u].expected_output;

    smartlist_add(votes, (void*)input);
    result = protover_compute_vote(votes, 1);
    if (expected_output != NULL) {
      tt_str_op(result, OP_EQ, expected_output);
    } else {
      tt_str_op(result, OP_EQ, "");
    }

    smartlist_clear(votes);
    tor_free(result);
  }

 done:
  smartlist_free(votes);
  tor_free(result);
}

#define PV_TEST(name, flags)                       \
  { #name, test_protover_ ##name, (flags), NULL, NULL }

struct testcase_t protover_tests[] = {
  PV_TEST(parse, 0),
  PV_TEST(parse_fail, 0),
  PV_TEST(vote, 0),
  PV_TEST(all_supported, 0),
  PV_TEST(list_supports_protocol_for_unsupported_returns_false, 0),
  PV_TEST(list_supports_protocol_returns_true, 0),
  PV_TEST(supports_version, 0),
  PV_TEST(supported_protocols, 0),
  PV_TEST(vote_roundtrip, 0),
  END_OF_TESTCASES
};
