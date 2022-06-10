/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define PROTOVER_PRIVATE
#define DIRVOTE_PRIVATE

#include "orconfig.h"
#include "test/test.h"

#include "lib/tls/tortls.h"

#include "core/or/or.h"

#include "core/or/connection_or.h"
#include "core/or/protover.h"
#include "core/or/versions.h"

#include "feature/dirauth/dirvote.h"

#include "feature/relay/relay_handshake.h"

static void
test_protover_parse(void *arg)
{
  (void) arg;
  char *re_encoded = NULL;

  const char *orig = "Foo=1,3 Bar=3 Baz= Quux=9-12,14,15-16";
  smartlist_t *elts = parse_protocol_list(orig);

  tt_assert(elts);
  tt_int_op(smartlist_len(elts), OP_EQ, 4);

  const proto_entry_t *e;
  e = smartlist_get(elts, 0);
  tt_str_op(e->name, OP_EQ, "Foo");
  tt_int_op(e->bitmask, OP_EQ, 0x0a);

  e = smartlist_get(elts, 1);
  tt_str_op(e->name, OP_EQ, "Bar");
  tt_int_op(e->bitmask, OP_EQ, 0x08);

  e = smartlist_get(elts, 2);
  tt_str_op(e->name, OP_EQ, "Baz");
  tt_int_op(e->bitmask, OP_EQ, 0x00);

  e = smartlist_get(elts, 3);
  tt_str_op(e->name, OP_EQ, "Quux");
  tt_int_op(e->bitmask, OP_EQ, 0x1de00);

  re_encoded = encode_protocol_list(elts);
  tt_assert(re_encoded);
  tt_str_op(re_encoded, OP_EQ, "Foo=1,3 Bar=3 Baz= Quux=9-12,14-16");

 done:
  if (elts)
    SMARTLIST_FOREACH(elts, proto_entry_t *, ent, proto_entry_free(ent));
  smartlist_free(elts);
  tor_free(re_encoded);
}

static void
test_protover_parse_fail(void *arg)
{
  (void)arg;
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

  smartlist_add(lst, (void*) "Foo=1-10,63 Bar=1,3-7,8");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Bar=1,3-8 Foo=1-10,63");
  tor_free(result);

  smartlist_add(lst, (void*) "Quux=12-45 Bar=2-6,8 Foo=9");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Bar=1-8 Foo=1-10,63 Quux=12-45");
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
  smartlist_add(lst, (void*) "Sleen=1-50");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Sleen=1-50");
  tor_free(result);

  /* Just below the threshold: C */
  smartlist_clear(lst);
  smartlist_add(lst, (void*) "Sleen=1-63");
  result = protover_compute_vote(lst, 1);
  tt_str_op(result, OP_EQ, "Sleen=1-63");
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
  tt_assert(! protover_all_supported("Link=60", &msg));
  tt_str_op(msg, OP_EQ, "Link=60");
  tor_free(msg);

  // Mix of things we support and things we don't
  tt_assert(! protover_all_supported("Link=3-4 Wombat=9", &msg));
  tt_str_op(msg, OP_EQ, "Wombat=9");
  tor_free(msg);

  /* Mix of things we support and don't support within a single protocol
   * which we do support */
  tt_assert(! protover_all_supported("Link=3-60", &msg));
  tt_str_op(msg, OP_EQ, "Link=6-60");
  tor_free(msg);
  tt_assert(! protover_all_supported("Link=1-3,50-63", &msg));
  tt_str_op(msg, OP_EQ, "Link=50-63");
  tor_free(msg);
  tt_assert(! protover_all_supported("Link=1-3,5-12", &msg));
  tt_str_op(msg, OP_EQ, "Link=6-12");
  tor_free(msg);

  /* Mix of protocols we do support and some we don't, where the protocols
   * we do support have some versions we don't support. */
  tt_assert(! protover_all_supported("Link=1-3,5-12 Quokka=40-41", &msg));
  tt_str_op(msg, OP_EQ, "Link=6-12 Quokka=40-41");
  tor_free(msg);

  /* If we get a (barely) valid (but unsupported list, we say "yes, that's
   * supported." */
  tt_assert(protover_all_supported("Fribble=", &msg));
  tt_ptr_op(msg, OP_EQ, NULL);

#ifndef ALL_BUGS_ARE_FATAL
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
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

  /* Protocol name too long */
#if !defined(ALL_BUGS_ARE_FATAL)
  tor_capture_bugs_(1);
  tt_assert(protover_all_supported(
                 "DoSaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 "aaaaaaaaaaaa=1-65536", &msg));
  tor_end_capture_bugs_();
#endif /* !defined(ALL_BUGS_ARE_FATAL) */

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
#define PROTOVER_LINKAUTH_V2 2
#define PROTOVER_RELAY_V1 1

/* Deprecated HSIntro versions */
#define PROTOVER_HS_INTRO_DEPRECATED_1 1
#define PROTOVER_HS_INTRO_DEPRECATED_2 2

/* HSv2 Rend and HSDir protocol versions. */
#define PROTOVER_HS_RENDEZVOUS_POINT_V2 1
#define PROTOVER_HSDIR_V2 2

/* DirCache, Desc, Microdesc, and Cons protocol versions. */
#define PROTOVER_DIRCACHE_V1 1
#define PROTOVER_DIRCACHE_V2 2

#define PROTOVER_DESC_V1 1
#define PROTOVER_DESC_V2 2

#define PROTOVER_MICRODESC_V1 1
#define PROTOVER_MICRODESC_V2 2

#define PROTOVER_CONS_V1 1
#define PROTOVER_CONS_V2 2

#define PROTOVER_PADDING_V1 1

#define PROTOVER_FLOWCTRL_V1 1

#define PROTOVER_RELAY_NTOR_V3 4

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
      tt_int_op(protocol_list_supports_protocol(supported_protocols,
                                                PRT_LINK,
                                                i),
                OP_EQ,
                is_or_protocol_version_known(i));
  }

  /* Legacy LinkAuth is only supported on OpenSSL and similar. */
  tt_int_op(protocol_list_supports_protocol(supported_protocols,
                                            PRT_LINKAUTH,
                                            PROTOVER_LINKAUTH_V1),
            OP_EQ,
            authchallenge_type_is_supported(AUTHTYPE_RSA_SHA256_TLSSECRET));
  /* LinkAuth=2 is unused */
  tt_assert(!protocol_list_supports_protocol(supported_protocols,
                                             PRT_LINKAUTH,
                                             PROTOVER_LINKAUTH_V2));
  tt_assert(
      protocol_list_supports_protocol(supported_protocols,
                                     PRT_LINKAUTH,
                                     PROTOVER_LINKAUTH_ED25519_HANDSHAKE));

  /* Relay protovers do not appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_RELAY,
                                            PROTOVER_RELAY_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_RELAY,
                                            PROTOVER_RELAY_EXTEND2));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_RELAY,
                                            PROTOVER_RELAY_ACCEPT_IPV6));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_RELAY,
                                            PROTOVER_RELAY_EXTEND_IPV6));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_RELAY,
                                            PROTOVER_RELAY_CANONICAL_IPV6));

  /* These HSIntro versions are deprecated */
  tt_assert(!protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSINTRO,
                                            PROTOVER_HS_INTRO_DEPRECATED_1));
  tt_assert(!protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSINTRO,
                                            PROTOVER_HS_INTRO_DEPRECATED_2));
  /* Test for HSv3 HSIntro */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSINTRO,
                                            PROTOVER_HS_INTRO_V3));
  /* Test for HSIntro DoS */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSINTRO,
                                            PROTOVER_HS_INTRO_DOS));

  /* Legacy HSRend does not appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSREND,
                                            PROTOVER_HS_RENDEZVOUS_POINT_V2));
  /* Test for HSv3 HSRend */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSREND,
                                            PROTOVER_HS_RENDEZVOUS_POINT_V3));

  /* Legacy HSDir does not appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSDIR,
                                            PROTOVER_HSDIR_V2));
  /* Test for HSv3 HSDir */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_HSDIR,
                                            PROTOVER_HSDIR_V3));

  /* No DirCache versions appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_DIRCACHE,
                                            PROTOVER_DIRCACHE_V2));

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

  /* No Cons versions appear anywhere in the code. */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_CONS,
                                            PROTOVER_CONS_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_CONS,
                                            PROTOVER_CONS_V2));

  /* Padding=1 is deprecated. */
  tt_assert(!protocol_list_supports_protocol(supported_protocols,
                                             PRT_PADDING,
                                             PROTOVER_PADDING_V1));
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_PADDING,
                                            PROTOVER_HS_SETUP_PADDING));

  /* FlowCtrl */
  tt_assert(protocol_list_supports_protocol(supported_protocols,
                                            PRT_FLOWCTRL,
                                            PROTOVER_FLOWCTRL_V1));

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
    { "Foo=1,3 Bar=3 Baz= Quux=9-12,14,15-16,50 Zn=1,42",
      "Bar=3 Foo=1,3 Quux=9-12,14-16,50 Zn=1,42" },
    { "Zu16=1,63", "Zu16=1,63" },
    { "N-1=1,2", "N-1=1-2" },
    { "-1=4294967295", NULL },
    { "-1=3", "-1=3" },
    { "Foo=,", NULL },
    { "Foo=,1", NULL },
    { "Foo=1,,3", NULL },
    { "Foo=1,3,", NULL },
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
    { "Sleen=1-63", "Sleen=1-63" },
    { "Sleen=1-65537", NULL },
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

static void
test_protover_vote_roundtrip_ours(void *args)
{
  (void) args;
  const char *examples[] = {
    protover_get_supported_protocols(),
    protover_get_recommended_client_protocols(),
    protover_get_recommended_relay_protocols(),
    protover_get_required_client_protocols(),
    protover_get_required_relay_protocols(),
  };
  unsigned u;
  smartlist_t *votes = smartlist_new();
  char *result = NULL;

  for (u = 0; u < ARRAY_LENGTH(examples); ++u) {
    tt_assert(examples[u]);
    const char *input = examples[u];
    const char *expected_output = examples[u];

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

/* Stringifies its argument.
 * 4 -> "4" */
#define STR(x) #x

#ifdef COCCI
#define PROTOVER(proto_string, version_macro)
#else
/* Generate a protocol version string using proto_string and version_macro.
 * PROTOVER("HSIntro", PROTOVER_HS_INTRO_DOS) -> "HSIntro" "=" "5"
 * Uses two levels of macros to turn PROTOVER_HS_INTRO_DOS into "5".
 */
#define PROTOVER(proto_string, version_macro) \
  (proto_string "=" STR(version_macro))
#endif /* defined(COCCI) */

#define DEBUG_PROTOVER(flags) \
  STMT_BEGIN \
  log_debug(LD_GENERAL, \
            "protovers:\n" \
            "protocols_known: %d,\n" \
            "supports_extend2_cells: %d,\n" \
            "supports_accepting_ipv6_extends: %d,\n" \
            "supports_initiating_ipv6_extends: %d,\n" \
            "supports_canonical_ipv6_conns: %d,\n" \
            "supports_ed25519_link_handshake_compat: %d,\n" \
            "supports_ed25519_link_handshake_any: %d,\n" \
            "supports_ed25519_hs_intro: %d,\n" \
            "supports_establish_intro_dos_extension: %d,\n" \
            "supports_v3_hsdir: %d,\n" \
            "supports_v3_rendezvous_point: %d,\n" \
            "supports_hs_setup_padding: %d,\n" \
            "supports_congestion_control: %d.", \
            (flags).protocols_known, \
            (flags).supports_extend2_cells, \
            (flags).supports_accepting_ipv6_extends, \
            (flags).supports_initiating_ipv6_extends, \
            (flags).supports_canonical_ipv6_conns, \
            (flags).supports_ed25519_link_handshake_compat, \
            (flags).supports_ed25519_link_handshake_any, \
            (flags).supports_ed25519_hs_intro, \
            (flags).supports_establish_intro_dos_extension, \
            (flags).supports_v3_hsdir, \
            (flags).supports_v3_rendezvous_point, \
            (flags).supports_hs_setup_padding, \
            (flags).supports_congestion_control); \
    STMT_END

/* Test that the proto_string version version_macro sets summary_flag. */
#define TEST_PROTOVER(proto_string, version_macro, summary_flag) \
  STMT_BEGIN \
  memset(&flags, 0, sizeof(flags)); \
  summarize_protover_flags(&flags, \
                           PROTOVER(proto_string, version_macro), \
                           NULL); \
  DEBUG_PROTOVER(flags); \
  tt_int_op(flags.protocols_known, OP_EQ, 1); \
  tt_int_op(flags.summary_flag, OP_EQ, 1); \
  flags.protocols_known = 0; \
  flags.summary_flag = 0; \
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags)); \
  STMT_END

static void
test_protover_summarize_flags(void *args)
{
  (void) args;
  char pv[30];
  memset(&pv, 0, sizeof(pv));

  protover_summary_cache_free_all();

  protover_summary_flags_t zero_flags;
  memset(&zero_flags, 0, sizeof(zero_flags));
  protover_summary_flags_t flags;

  memset(&flags, 0, sizeof(flags));
  summarize_protover_flags(&flags, NULL, NULL);
  DEBUG_PROTOVER(flags);
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  memset(&flags, 0, sizeof(flags));
  summarize_protover_flags(&flags, "", "");
  DEBUG_PROTOVER(flags);
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  /* Now check version exceptions */

  /* EXTEND2 cell support */
  memset(&flags, 0, sizeof(flags));
  summarize_protover_flags(&flags, NULL, "Tor 0.2.4.8-alpha");
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  tt_int_op(flags.supports_extend2_cells, OP_EQ, 1);
  /* Now clear those flags, and check the rest are zero */
  flags.protocols_known = 0;
  flags.supports_extend2_cells = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  /* disabling HSDir v3 support for buggy versions */
  memset(&flags, 0, sizeof(flags));
  summarize_protover_flags(&flags,
                           PROTOVER("HSDir", PROTOVER_HSDIR_V3),
                           NULL);
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  tt_int_op(flags.supports_v3_hsdir, OP_EQ, 1);
  /* Now clear those flags, and check the rest are zero */
  flags.protocols_known = 0;
  flags.supports_v3_hsdir = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  memset(&flags, 0, sizeof(flags));
  summarize_protover_flags(&flags,
                           PROTOVER("HSDir", PROTOVER_HSDIR_V3),
                           "Tor 0.3.0.7");
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  /* Now clear that flag, and check the rest are zero */
  flags.protocols_known = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  /* Now check standard summaries */

  /* LinkAuth */
  memset(&flags, 0, sizeof(flags));
  summarize_protover_flags(&flags,
                           PROTOVER("LinkAuth",
                                    PROTOVER_LINKAUTH_ED25519_HANDSHAKE),
                           NULL);
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  tt_int_op(flags.supports_ed25519_link_handshake_compat, OP_EQ, 1);
  tt_int_op(flags.supports_ed25519_link_handshake_any, OP_EQ, 1);
  /* Now clear those flags, and check the rest are zero */
  flags.protocols_known = 0;
  flags.supports_ed25519_link_handshake_compat = 0;
  flags.supports_ed25519_link_handshake_any = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  /* Test one greater */
  memset(&flags, 0, sizeof(flags));
  snprintf(pv, sizeof(pv),
           "%s=%d", "LinkAuth", PROTOVER_LINKAUTH_ED25519_HANDSHAKE + 1);
  summarize_protover_flags(&flags, pv, NULL);
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  tt_int_op(flags.supports_ed25519_link_handshake_compat, OP_EQ, 0);
  tt_int_op(flags.supports_ed25519_link_handshake_any, OP_EQ, 1);
  /* Now clear those flags, and check the rest are zero */
  flags.protocols_known = 0;
  flags.supports_ed25519_link_handshake_compat = 0;
  flags.supports_ed25519_link_handshake_any = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  /* Test one less */
  memset(&flags, 0, sizeof(flags));
  snprintf(pv, sizeof(pv),
           "%s=%d", "LinkAuth", PROTOVER_LINKAUTH_ED25519_HANDSHAKE - 1);
  summarize_protover_flags(&flags, pv, NULL);
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  tt_int_op(flags.supports_ed25519_link_handshake_compat, OP_EQ, 0);
  tt_int_op(flags.supports_ed25519_link_handshake_any, OP_EQ, 0);
  /* Now clear those flags, and check the rest are zero */
  flags.protocols_known = 0;
  flags.supports_ed25519_link_handshake_compat = 0;
  flags.supports_ed25519_link_handshake_any = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  /* We don't test "one more" and "one less" for each protocol version.
   * But that could be a useful thing to add. */

  /* Relay */
  memset(&flags, 0, sizeof(flags));
  /* This test relies on these versions being equal */
  tt_int_op(PROTOVER_RELAY_EXTEND2, OP_EQ, PROTOVER_RELAY_ACCEPT_IPV6);
  summarize_protover_flags(&flags,
                           PROTOVER("Relay", PROTOVER_RELAY_EXTEND2), NULL);
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  tt_int_op(flags.supports_extend2_cells, OP_EQ, 1);
  tt_int_op(flags.supports_accepting_ipv6_extends, OP_EQ, 1);
  /* Now clear those flags, and check the rest are zero */
  flags.protocols_known = 0;
  flags.supports_extend2_cells = 0;
  flags.supports_accepting_ipv6_extends = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  memset(&flags, 0, sizeof(flags));
  /* This test relies on these versions being equal */
  tt_int_op(PROTOVER_RELAY_EXTEND_IPV6, OP_EQ, PROTOVER_RELAY_CANONICAL_IPV6);
  summarize_protover_flags(&flags,
                           PROTOVER("Relay", PROTOVER_RELAY_EXTEND_IPV6),
                           NULL);
  DEBUG_PROTOVER(flags);
  tt_int_op(flags.protocols_known, OP_EQ, 1);
  tt_int_op(flags.supports_accepting_ipv6_extends, OP_EQ, 1);
  tt_int_op(flags.supports_initiating_ipv6_extends, OP_EQ, 1);
  tt_int_op(flags.supports_canonical_ipv6_conns, OP_EQ, 1);
  /* Now clear those flags, and check the rest are zero */
  flags.protocols_known = 0;
  flags.supports_accepting_ipv6_extends = 0;
  flags.supports_initiating_ipv6_extends = 0;
  flags.supports_canonical_ipv6_conns = 0;
  tt_mem_op(&flags, OP_EQ, &zero_flags, sizeof(flags));

  TEST_PROTOVER("HSIntro", PROTOVER_HS_INTRO_V3,
                supports_ed25519_hs_intro);
  TEST_PROTOVER("HSIntro", PROTOVER_HS_INTRO_DOS,
                supports_establish_intro_dos_extension);

  TEST_PROTOVER("HSRend", PROTOVER_HS_RENDEZVOUS_POINT_V3,
                supports_v3_rendezvous_point);

  TEST_PROTOVER("HSDir", PROTOVER_HSDIR_V3,
                supports_v3_hsdir);

  TEST_PROTOVER("Padding", PROTOVER_HS_SETUP_PADDING,
                supports_hs_setup_padding);

 done:
  ;
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
  PV_TEST(vote_roundtrip_ours, 0),
  /* fork, because we memoize flags internally */
  PV_TEST(summarize_flags, TT_FORK),
  END_OF_TESTCASES
};
