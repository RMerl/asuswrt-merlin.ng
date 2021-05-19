/* Copyright (c) 2010-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "core/or/or.h"

#define DIRVOTE_PRIVATE
#include "app/config/config.h"
#include "feature/dirauth/dirvote.h"
#include "feature/dirparse/microdesc_parse.h"
#include "feature/dirparse/routerparse.h"
#include "feature/nodelist/microdesc.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nodefamily.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/torcert.h"

#include "feature/nodelist/microdesc_st.h"
#include "feature/nodelist/networkstatus_st.h"
#include "feature/nodelist/routerinfo_st.h"
#include "feature/nodelist/routerstatus_st.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef _WIN32
/* For mkdir() */
#include <direct.h>
#else
#include <dirent.h>
#endif /* defined(_WIN32) */

static const char test_md1[] =
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMjlHH/daN43cSVRaHBwgUfnszzAhg98EvivJ9Qxfv51mvQUxPjQ07es\n"
  "gV/3n8fyh3Kqr/ehi9jxkdgSRfSnmF7giaHL1SLZ29kA7KtST+pBvmTpDtHa3ykX\n"
  "Xorc7hJvIyTZoc1HU+5XSynj3gsBE5IGK1ZRzrNS688LnuZMVp1tAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs=\n";

static const char test_md2[] =
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMIixIowh2DyPmDNMDwBX2DHcYcqdcH1zdIQJZkyV6c6rQHnvbcaDoSg\n"
  "jgFSLJKpnGmh71FVRqep+yVB0zI1JY43kuEnXry2HbZCD9UDo3d3n7t015X5S7ON\n"
  "bSSYtQGPwOr6Epf96IF6DoQxy4iDnPUAlejuhAG51s1y6/rZQ3zxAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs=\n";

static const char test_md3[] =
  "@last-listed 2009-06-22\n"
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMH3340d4ENNGrqx7UxT+lB7x6DNUKOdPEOn4teceE11xlMyZ9TPv41c\n"
  "qj2fRZzfxlc88G/tmiaHshmdtEpklZ740OFqaaJVj4LjPMKFNE+J7Xc1142BE9Ci\n"
  "KgsbjGYe2RY261aADRWLetJ8T9QDMm+JngL4288hc8pq1uB/3TAbAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs=\n"
  "p accept 1-700,800-1000\n"
  "family nodeX nodeY nodeZ\n";

static void
test_md_cache(void *data)
{
  or_options_t *options = NULL;
  microdesc_cache_t *mc = NULL ;
  smartlist_t *added = NULL, *wanted = NULL;
  microdesc_t *md1, *md2, *md3;
  char d1[DIGEST256_LEN], d2[DIGEST256_LEN], d3[DIGEST256_LEN];
  const char *test_md3_noannotation = strchr(test_md3, '\n')+1;
  time_t time1, time2, time3;
  char *fn = NULL, *s = NULL;
  char *encoded_family = NULL;
  (void)data;

  options = get_options_mutable();
  tt_assert(options);

  time1 = time(NULL);
  time2 = time(NULL) - 2*24*60*60;
  time3 = time(NULL) - 15*24*60*60;

  /* Possibly, turn this into a test setup/cleanup pair */
  tor_free(options->CacheDirectory);
  options->CacheDirectory = tor_strdup(get_fname("md_datadir_test"));
#ifdef _WIN32
  tt_int_op(0, OP_EQ, mkdir(options->CacheDirectory));
#else
  tt_int_op(0, OP_EQ, mkdir(options->CacheDirectory, 0700));
#endif

  tt_assert(!strcmpstart(test_md3_noannotation, "onion-key"));

  crypto_digest256(d1, test_md1, strlen(test_md1), DIGEST_SHA256);
  crypto_digest256(d2, test_md2, strlen(test_md1), DIGEST_SHA256);
  crypto_digest256(d3, test_md3_noannotation, strlen(test_md3_noannotation),
                   DIGEST_SHA256);

  mc = get_microdesc_cache();

  added = microdescs_add_to_cache(mc, test_md1, NULL, SAVED_NOWHERE, 0,
                                  time1, NULL);
  tt_int_op(1, OP_EQ, smartlist_len(added));
  md1 = smartlist_get(added, 0);
  smartlist_free(added);
  added = NULL;

  wanted = smartlist_new();
  added = microdescs_add_to_cache(mc, test_md2, NULL, SAVED_NOWHERE, 0,
                                  time2, wanted);
  /* Should fail, since we didn't list test_md2's digest in wanted */
  tt_int_op(0, OP_EQ, smartlist_len(added));
  smartlist_free(added);
  added = NULL;

  smartlist_add(wanted, tor_memdup(d2, DIGEST256_LEN));
  smartlist_add(wanted, tor_memdup(d3, DIGEST256_LEN));
  added = microdescs_add_to_cache(mc, test_md2, NULL, SAVED_NOWHERE, 0,
                                  time2, wanted);
  /* Now it can work. md2 should have been added */
  tt_int_op(1, OP_EQ, smartlist_len(added));
  md2 = smartlist_get(added, 0);
  /* And it should have gotten removed from 'wanted' */
  tt_int_op(smartlist_len(wanted), OP_EQ, 1);
  tt_mem_op(smartlist_get(wanted, 0), OP_EQ, d3, DIGEST256_LEN);
  smartlist_free(added);
  added = NULL;

  added = microdescs_add_to_cache(mc, test_md3, NULL,
                                  SAVED_NOWHERE, 0, -1, NULL);
  /* Must fail, since SAVED_NOWHERE precludes annotations */
  tt_int_op(0, OP_EQ, smartlist_len(added));
  smartlist_free(added);
  added = NULL;

  added = microdescs_add_to_cache(mc, test_md3_noannotation, NULL,
                                  SAVED_NOWHERE, 0, time3, NULL);
  /* Now it can work */
  tt_int_op(1, OP_EQ, smartlist_len(added));
  md3 = smartlist_get(added, 0);
  smartlist_free(added);
  added = NULL;

  /* Okay.  We added 1...3.  Let's poke them to see how they look, and make
   * sure they're really in the journal. */
  tt_ptr_op(md1, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d1));
  tt_ptr_op(md2, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d2));
  tt_ptr_op(md3, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d3));

  tt_int_op(md1->last_listed, OP_EQ, time1);
  tt_int_op(md2->last_listed, OP_EQ, time2);
  tt_int_op(md3->last_listed, OP_EQ, time3);

  tt_int_op(md1->saved_location, OP_EQ, SAVED_IN_JOURNAL);
  tt_int_op(md2->saved_location, OP_EQ, SAVED_IN_JOURNAL);
  tt_int_op(md3->saved_location, OP_EQ, SAVED_IN_JOURNAL);

  tt_int_op(md1->bodylen, OP_EQ, strlen(test_md1));
  tt_int_op(md2->bodylen, OP_EQ, strlen(test_md2));
  tt_int_op(md3->bodylen, OP_EQ, strlen(test_md3_noannotation));
  tt_mem_op(md1->body, OP_EQ, test_md1, strlen(test_md1));
  tt_mem_op(md2->body, OP_EQ, test_md2, strlen(test_md2));
  tt_mem_op(md3->body, OP_EQ, test_md3_noannotation,
              strlen(test_md3_noannotation));

  tor_asprintf(&fn, "%s"PATH_SEPARATOR"cached-microdescs.new",
               options->CacheDirectory);
  s = read_file_to_str(fn, RFTS_BIN, NULL);
  tt_assert(s);
  tt_mem_op(md1->body, OP_EQ, s + md1->off, md1->bodylen);
  tt_mem_op(md2->body, OP_EQ, s + md2->off, md2->bodylen);
  tt_mem_op(md3->body, OP_EQ, s + md3->off, md3->bodylen);

  tt_ptr_op(md1->family, OP_EQ, NULL);
  tt_ptr_op(md3->family, OP_NE, NULL);

  encoded_family = nodefamily_format(md3->family);
  tt_str_op(encoded_family, OP_EQ, "nodex nodey nodez");

  /* Now rebuild the cache! */
  tt_int_op(microdesc_cache_rebuild(mc, 1), OP_EQ, 0);

  tt_int_op(md1->saved_location, OP_EQ, SAVED_IN_CACHE);
  tt_int_op(md2->saved_location, OP_EQ, SAVED_IN_CACHE);
  tt_int_op(md3->saved_location, OP_EQ, SAVED_IN_CACHE);

  /* The journal should be empty now */
  tor_free(s);
  s = read_file_to_str(fn, RFTS_BIN, NULL);
  tt_str_op(s, OP_EQ, "");
  tor_free(s);
  tor_free(fn);

  /* read the cache. */
  tor_asprintf(&fn, "%s"PATH_SEPARATOR"cached-microdescs",
               options->CacheDirectory);
  s = read_file_to_str(fn, RFTS_BIN, NULL);
  tt_mem_op(md1->body, OP_EQ, s + md1->off, strlen(test_md1));
  tt_mem_op(md2->body, OP_EQ, s + md2->off, strlen(test_md2));
  tt_mem_op(md3->body, OP_EQ, s + md3->off, strlen(test_md3_noannotation));

  /* Okay, now we are going to forget about the cache entirely, and reload it
   * from the disk. */
  microdesc_free_all();
  mc = get_microdesc_cache();
  md1 = microdesc_cache_lookup_by_digest256(mc, d1);
  md2 = microdesc_cache_lookup_by_digest256(mc, d2);
  md3 = microdesc_cache_lookup_by_digest256(mc, d3);
  tt_assert(md1);
  tt_assert(md2);
  tt_assert(md3);
  tt_mem_op(md1->body, OP_EQ, s + md1->off, strlen(test_md1));
  tt_mem_op(md2->body, OP_EQ, s + md2->off, strlen(test_md2));
  tt_mem_op(md3->body, OP_EQ, s + md3->off, strlen(test_md3_noannotation));

  tt_int_op(md1->last_listed, OP_EQ, time1);
  tt_int_op(md2->last_listed, OP_EQ, time2);
  tt_int_op(md3->last_listed, OP_EQ, time3);

  /* Okay, now we are going to clear out everything older than a week old.
   * In practice, that means md3 */
  microdesc_cache_clean(mc, time(NULL)-7*24*60*60, 1/*force*/);
  tt_ptr_op(md1, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d1));
  tt_ptr_op(md2, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d2));
  tt_ptr_op(NULL, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d3));
  md3 = NULL; /* it's history now! */

  /* rebuild again, make sure it stays gone. */
  tt_int_op(microdesc_cache_rebuild(mc, 1), OP_EQ, 0);
  tt_ptr_op(md1, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d1));
  tt_ptr_op(md2, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d2));
  tt_ptr_op(NULL, OP_EQ, microdesc_cache_lookup_by_digest256(mc, d3));

  /* Re-add md3, and make sure we can rebuild the cache. */
  added = microdescs_add_to_cache(mc, test_md3_noannotation, NULL,
                                  SAVED_NOWHERE, 0, time3, NULL);
  tt_int_op(1, OP_EQ, smartlist_len(added));
  md3 = smartlist_get(added, 0);
  smartlist_free(added);
  added = NULL;
  tt_int_op(md1->saved_location, OP_EQ, SAVED_IN_CACHE);
  tt_int_op(md2->saved_location, OP_EQ, SAVED_IN_CACHE);
  tt_int_op(md3->saved_location, OP_EQ, SAVED_IN_JOURNAL);

  tt_int_op(microdesc_cache_rebuild(mc, 1), OP_EQ, 0);
  tt_int_op(md3->saved_location, OP_EQ, SAVED_IN_CACHE);

 done:
  if (options)
    tor_free(options->CacheDirectory);
  microdesc_free_all();

  smartlist_free(added);
  if (wanted)
    SMARTLIST_FOREACH(wanted, char *, cp, tor_free(cp));
  smartlist_free(wanted);
  tor_free(s);
  tor_free(fn);
  tor_free(encoded_family);
}

static const char truncated_md[] =
  "@last-listed 2013-08-08 19:02:59\n"
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAM91vLFNaM+gGhnRIdz2Cm/Kl7Xz0cOobIdVzhS3cKUJfk867hCuTipS\n"
  "NveLBzNopvgXKruAAzEj3cACxk6Q8lv5UWOGCD1UolkgsWSE62RBjap44g+oc9J1\n"
  "RI9968xOTZw0VaBQg9giEILNXl0djoikQ+5tQRUvLDDa67gpa5Q1AgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "family @\n";

static void
test_md_cache_broken(void *data)
{
  or_options_t *options;
  char *fn=NULL;
  microdesc_cache_t *mc = NULL;

  (void)data;

  options = get_options_mutable();
  tt_assert(options);
  tor_free(options->CacheDirectory);
  options->CacheDirectory = tor_strdup(get_fname("md_datadir_test2"));

#ifdef _WIN32
  tt_int_op(0, OP_EQ, mkdir(options->CacheDirectory));
#else
  tt_int_op(0, OP_EQ, mkdir(options->CacheDirectory, 0700));
#endif

  tor_asprintf(&fn, "%s"PATH_SEPARATOR"cached-microdescs",
               options->CacheDirectory);

  write_str_to_file(fn, truncated_md, 1);

  mc = get_microdesc_cache();
  tt_assert(mc);

 done:
  if (options)
    tor_free(options->CacheDirectory);
  tor_free(fn);
  microdesc_free_all();
}

/* Generated by chutney. */
static const char test_ri[] =
  "router test005r 127.0.0.1 5005 0 7005\n"
  "identity-ed25519\n"
  "-----BEGIN ED25519 CERT-----\n"
  "AQQABs1eAfTuBhu6ypB5/9avDiY3qBzulkCvfYqbFN/ABk/o4xFcAQAgBAAnmWRG\n"
  "rIvqpb4Kk3cThEiWAll4uDCO2Y46uNm9WG7AtPt4LG+XfktG3GAxv6aVQimwlyHc\n"
  "1x2Lfm9KG3mWWj+hxnum4Z7873OE0B9l2Hg0YQZCW/PuHSWN0rspTvY5SgA=\n"
  "-----END ED25519 CERT-----\n"
  "master-key-ed25519 J5lkRqyL6qW+CpN3E4RIlgJZeLgwjtmOOrjZvVhuwLQ\n"
  "or-address [::]:5005\n"
  "platform Tor 0.4.5.0-alpha-dev on Linux\n"
  "proto Cons=1-2 Desc=1-2 DirCache=1-2 FlowCtrl=1 HSDir=1-2 HSIntro=3-5 "
      "HSRend=1-2 Link=1-5 LinkAuth=1,3 Microdesc=1-2 Padding=2 Relay=1-3\n"
  "published 2020-10-13 13:27:34\n"
  "fingerprint D219 590A C951 3BCD EBBA 9AB7 2100 7A4C C01B BAE3\n"
  "uptime 324451\n"
  "bandwidth 1073741824 1073741824 637796\n"
  "extra-info-digest 78E6D382BC826B95B4111554EEE7D541A32AAAA3 "
       "c61Onjpq+1S0TrdvoaOvGAxew6yfO+uHNhipbemQmgA\n"
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMvEJ/JVNK7I38PPWhQMuCgkET/ki4WIas4tj5Kmqfb9kHqxMR+EunRD\n"
  "83k4pel1yB7QdV+iTd/4SZOI8RpZP+BO1KnOTWfpztAU1lDGr19/PwdwcHaILpBD\n"
  "nNzm6otk4/bKUQ0vqpOfJljtg0DfAm4uMAQ6BMFy6uEAF7+JupuPAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "signing-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBANBzejGAwyPTPq2Gm03wpg3qICo0uDQau8opude2mW3eyxAqOqHzC8De\n"
  "gRgbmn040vqe9gwvH4iaHpVeTxyDwQefbfULdq6bETmX3aSUj6LKBCqqcyuOJFQu\n"
  "7M2QfNSfHtldUABpIaqFvEA3AV8qjOoUtauoFNJKMy7Wj2//S70VAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "onion-key-crosscert\n"
  "-----BEGIN CROSSCERT-----\n"
  "pD3Nkkunt8zP6PO6H3uHT0t7xnorC7cY/KfF75mFB+90pHCD9f0Xdu3Pjrur/q23\n"
  "PIKV3hdtdsODoJuoh8LPGNAjS5rO6HMCtHNDNunNOs69bvfaO0jThnurXmOpY0sW\n"
  "eRfBeYN2KNgrN0B1eDejfPSr03dkFY48yoUDROv9EJQ=\n"
  "-----END CROSSCERT-----\n"
  "ntor-onion-key-crosscert 0\n"
  "-----BEGIN ED25519 CERT-----\n"
  "AQoABs2OASeZZEasi+qlvgqTdxOESJYCWXi4MI7Zjjq42b1YbsC0AKc5y5qYUYvw\n"
  "VATtWkV9DVIZbZSb9mQP5pmNaqmX+DbmINCYt8j7l+U7g3ftUyh0Wlrgevx0pFUI\n"
  "RcIU0HKHZQA=\n"
  "-----END ED25519 CERT-----\n"
  "hidden-service-dir\n"
  "ntor-onion-key FChIfm77vrWB7JsxQ+jMbN6VSSp1P0DYbw/2aqey4iA\n"
  "accept *:*\n"
  "tunnelled-dir-server\n"
  "router-sig-ed25519 Xm56dYbo/hCHWyzcdUPmfTeZ4qly2TYf1/2Q1lXKQDMJyBti"
      "8ZE8R2TTYsYimr+UtAapbzBItccZLze505nhBw\n"
  "router-signature\n"
  "-----BEGIN SIGNATURE-----\n"
  "bbeN0lq6nCfJQXGcKa1M9TQ6b2upig7clrlVXuzKeR0JhGwnDCXUAFxDtrw3vkVo\n"
  "ExBXXvJeBPyustFOQkdiAEWHHSW5CwEgeVCBYZeEnaiySIgDVKuu+9B53ezFdC0Y\n"
  "iFJkKxxDx7ksxX0zdl7aPT4ORFEuRhCYS6el7YJmoyg=\n"
  "-----END SIGNATURE-----\n";

static const char test_md2_25[] =
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMvEJ/JVNK7I38PPWhQMuCgkET/ki4WIas4tj5Kmqfb9kHqxMR+EunRD\n"
  "83k4pel1yB7QdV+iTd/4SZOI8RpZP+BO1KnOTWfpztAU1lDGr19/PwdwcHaILpBD\n"
  "nNzm6otk4/bKUQ0vqpOfJljtg0DfAm4uMAQ6BMFy6uEAF7+JupuPAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key FChIfm77vrWB7JsxQ+jMbN6VSSp1P0DYbw/2aqey4iA=\n"
  "p accept 1-65535\n"
  "id ed25519 J5lkRqyL6qW+CpN3E4RIlgJZeLgwjtmOOrjZvVhuwLQ\n";

static const char test_md2_withfamily_28[] =
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMvEJ/JVNK7I38PPWhQMuCgkET/ki4WIas4tj5Kmqfb9kHqxMR+EunRD\n"
  "83k4pel1yB7QdV+iTd/4SZOI8RpZP+BO1KnOTWfpztAU1lDGr19/PwdwcHaILpBD\n"
  "nNzm6otk4/bKUQ0vqpOfJljtg0DfAm4uMAQ6BMFy6uEAF7+JupuPAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key FChIfm77vrWB7JsxQ+jMbN6VSSp1P0DYbw/2aqey4iA=\n"
  "family OtherNode !Strange\n"
  "p accept 1-65535\n"
  "id ed25519 J5lkRqyL6qW+CpN3E4RIlgJZeLgwjtmOOrjZvVhuwLQ\n";

static const char test_md2_withfamily_29[] =
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMvEJ/JVNK7I38PPWhQMuCgkET/ki4WIas4tj5Kmqfb9kHqxMR+EunRD\n"
  "83k4pel1yB7QdV+iTd/4SZOI8RpZP+BO1KnOTWfpztAU1lDGr19/PwdwcHaILpBD\n"
  "nNzm6otk4/bKUQ0vqpOfJljtg0DfAm4uMAQ6BMFy6uEAF7+JupuPAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key FChIfm77vrWB7JsxQ+jMbN6VSSp1P0DYbw/2aqey4iA=\n"
  "family !Strange $D219590AC9513BCDEBBA9AB721007A4CC01BBAE3 othernode\n"
  "p accept 1-65535\n"
  "id ed25519 J5lkRqyL6qW+CpN3E4RIlgJZeLgwjtmOOrjZvVhuwLQ\n";

static void
test_md_generate(void *arg)
{
  routerinfo_t *ri;
  microdesc_t *md = NULL;
  (void)arg;

  ri = router_parse_entry_from_string(test_ri, NULL, 0, 0, NULL, NULL);
  tt_assert(ri);

  md = dirvote_create_microdescriptor(ri, 25);
  tt_str_op(md->body, OP_EQ, test_md2_25);
  tt_assert(ed25519_pubkey_eq(md->ed25519_identity_pkey,
                              &ri->cache_info.signing_key_cert->signing_key));

  // Try family encoding.
  microdesc_free(md);
  ri->declared_family = smartlist_new();
  smartlist_add_strdup(ri->declared_family, "OtherNode !Strange");
  md = dirvote_create_microdescriptor(ri, 28);
  tt_str_op(md->body, OP_EQ, test_md2_withfamily_28);

  microdesc_free(md);
  md = dirvote_create_microdescriptor(ri, 29);
  tt_str_op(md->body, OP_EQ, test_md2_withfamily_29);

 done:
  microdesc_free(md);
  routerinfo_free(ri);
}

#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
DISABLE_GCC_WARNING("-Woverlength-strings")
/* We allow huge string constants in the unit tests, but not in the code
 * at large. */
#endif
/* Taken at random from my ~/.tor/cached-microdescs file and then
 * hand-munged */
static const char MD_PARSE_TEST_DATA[] =
  /* Good 0 */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBANsKd1GRfOuSR1MkcwKqs6SVy4Gi/JXplt/bHDkIGm6Q96TeJ5uyVgUL\n"
  "DBr/ij6+JqgVFeriuiMzHKREytzjdaTuKsKBFFpLwb+Ppcjr5nMIH/AR6/aHO8hW\n"
  "T3B9lx5T6Kl7CqZ4yqXxYRHzn50EPTIZuz0y9se4J4gi9mLmL+pHAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs=\n"
  "p accept 20-23,43,53,79-81,88,110,143,194,220,443,464,531,543-544\n"
  "id rsa1024 GEo59/iR1GWSIWZDzXTd5QxtqnU\n"
  /* Bad 0: I've messed with the onion-key in the second one. */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMr4o/pflVwscx11vC1AKEADlKEqnhpvCIjAEzNEenMhvGQHRlA0EXLC\n"
  "7G7O5bhnCwEHqK8Pvg8cuX/fD8v08TF1EVPhwPa0UI6ab8KnPP2F!!!!!!b92DG7EQIk3q\n"
  "d68Uxp7E9/t3v1WWZjzDqvEe0par6ul+DKW6HMlTGebFo5Q4e8R1AgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key 761Dmm27via7lXygNHM3l+oJLrYU2Nye0Uz4pkpipyY=\n"
  "p accept 53\n"
  "id rsa1024 3Y4fwXhtgkdGDZ5ef5mtb6TJRQQ\n"
  /* Good 1 */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBANsMSjVi3EX8ZHfm/dvPF6KdVR66k1tVul7Jp+dDbDajBYNhgKRzVCxy\n"
  "Yac1CBuQjOqK89tKap9PQBnhF087eDrfaZDqYTLwB2W2sBJncVej15WEPXPRBifo\n"
  "iFZ8337kgczkaY+IOfSuhtbOUyDOoDpRJheIKBNq0ZiTqtLbbadVAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key ncfiHJjSgdDEW/gc6q6/7idac7j+x7ejQrRm6i75pGA=\n"
  "p accept 443,6660-6669,6697,7000-7001\n"
  "id rsa1024 XXuLzw3mfBELEq3veXoNhdehwD4\n"
  /* Good 2 */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBANQfBlrHrh9F/CAOytrNFgi0ikWMW/HZxuoszF9X+AQ+MudR8bcxxOGl\n"
  "1RFwb74s8E3uuzrCkNFvSw9Ar1L02F2DOX0gLsxEGuYC4Ave9NUteGqSqDyEJQUJ\n"
  "KlfxCPn2qC9nvNT7wR/Dg2WRvAEKnJmkpb57N3+WSAOPLjKOFEz3AgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs=\n"
  "id rsa1024 6y60AEI9a1PUUlRPO0YQT9WzrjI\n"
  /* Bad 1: Here I've messed with the ntor key */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAPjy2HacU3jDNO5nTOFGSwNa0qKCNn4yhtrDVcAJ5alIQeBWZZGJLZ0q\n"
  "Cqylw1vYqxu8E09g+QXXFbAgBv1U9TICaATxrIJhIJzc8TJPhqJemp1kq0DvHLDx\n"
  "mxwlkNnCD/P5NS+JYB3EjOlU9EnSKUWNU61+Co344m2JqhEau40vAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key 4i2Fp9JHTUr1uQs0pxD5j5spl4/RG56S2P0gQxU=\n"
  "id rsa1024 nMRmNEGysA0NmlALVaUmI7D5jLU\n"
  /* Good 3: I've added a weird token in this one. This shouldn't prevent
   * it parsing */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAKmosxudyNA/yJNz3S890VqV/ebylzoD11Sc0b/d5tyNNaNZjcYy5vRD\n"
  "kwyxFRMbP2TLZQ1zRfNwY7IDnYjU2SbW0pxuM6M8WRtsmx/YOE3kHMVAFJNrTUqU\n"
  "6D1zB3IiRDS5q5+NoRxwqo+hYUck60O3WTwEoqb+l3lvXeu7z9rFAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "flux-capacitor 1.21 GW\n"
  "ntor-onion-key MWBoEkl+RlBiGX44XKIvTSqbznTNZStOmUYtcYRQQyY=\n"
  "id rsa1024 R+A5O9qRvRac4FT3C4L2QnFyxsc\n"
  /* Good 4: Here I've made the 'id rsa' token odd.  It should still parse
   * just fine. */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAOh+WMkdNe/Pkjb8UjQyfLOlFgpuVFrxAIGnJsmWWx0yBE97DQxGyh2n\n"
  "h8G5OJZHRarJQyCIf7vpZQAi0oP0OkGGaCaDQsM+D8TnqhnU++RWGnMqY/cXxPrL\n"
  "MEq+n6aGiLmzkO7ah8yorZpoREk4GqLUIN89/tHHGOhJL3c4CPGjAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs=\n"
  "p reject 25,119,135-139,445,563,1214,4661-4666,6346-6429,6699,6881-6999\n"
  "id rsa1234 jlqAKFD2E7uMKv+8TmKSeo7NBho\n"
  /* Good 5: Extra id type. */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAMdgPPc5uaw4y/q+SUTN/I8Y+Gvdx9kKgWV4dmDGJ0mxsVZmo1v6+v3F\n"
  "12M2f9m99G3WB8F8now29C+9XyEv8MBHj1lHRdUFHSQes3YTFvDNlgj+FjLqO5TJ\n"
  "adOOmfu4DCUUtUEDyQKbNVL4EkMTXY73omTVsjcH3xxFjTx5wixhAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key AAVnWZcnDbxasdZwKqb4fL6O9sZV+XsRNHTpNd1YMz8=\n"
  "id rsa1024 72EfBL11QuwX2vU8y+p9ExGfGEg\n"
  "id expolding hedgehog 0+A5O9qRvRac4FT3C4L2QnFyxsc\n"
  /* Good 6: I've given this a bogus policy. It should parse. */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBALNuufwhPMF8BooxYMNvhYJMPqUB8hQDt8wGmPKphJcD1sVD1i4gAZM2\n"
  "HIo+zUBlljDrRWL5NzVzd1yxUJAiQxvXS5dRRFY3B70M7wTVpXw53xe0/BM5t1AX\n"
  "n0MFk7Jl6XIKMlzRalZvmMvE/odtyWXkP4Nd1MyZ1QcIwrQ2iwyrAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "p condone 1-10\n"
  "ntor-onion-key 2/nMJ+L4dd/2GpMyTYjz3zC59MvQy4MIzJZhdzKHekg=\n"
  "id rsa1024 FHyh10glEMA6MCmBb5R9Y+X/MhQ\n"
  /* Good 7: I've given this one another sort of odd policy. Should parse. */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAKcd3FmQ8iAADghyvX8eca0ePqtJ2w1IDdUdTlf5Y/8+OMdp//sD01yC\n"
  "YmiX45LK5ge1O3AzcakYCO6fb3pyIqvXdvm24OjyYZELQ40cmKSLjdhcSf4Fr/N9\n"
  "uR/CkknR9cEePu1wZ5WBIGmGdXI6s7t3LB+e7XFyBYAx6wMGlnX7AgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "p accept frogs-mice\n"
  "ntor-onion-key AMxvhaQ1Qg7jBJFoyHuPRgETvLbFmJ194hExV24FuAI=\n"
  "family $D8CFEA0D996F5D1473D2063C041B7910DB23981E\n"
  "id rsa1024 d0VVZC/cHh1P3y4MMbfKlQHFycc\n"
  /* Good 8: This one has the ntor-onion-key without terminating =. That's
   * allowed. */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAL438YfjrJE2SPqkkXeQwICygu8KNO54Juj6sjqk5hgsiazIWMOBgbaX\n"
  "LIRqPNGaiSq01xSqwjwCBCfwZYT/nSdDBqj1h9aoR8rnjxZjyQ+m3rWpdDqeCDMx\n"
  "I3NgZ5w4bNX4poRb42lrV6NmQiFdjzpqszVbv5Lpn2CSKu32CwKVAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key UKL6Dnj2KwYsFlkCvOkXVatxvOPB4MaxqwPQQgZMTwI\n"
  "id rsa1024 FPIXc6k++JnKCtSKWUxaR6oXEKs\n"
  /* Good 9: Another totally normal one.*/
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBANNGIKRd8PFNXkJ2JPV1ohDMFNbJwKbwybeieaQFjtU9KWedHCbr+QD4\n"
  "B6zNY5ysguNjHNnlq2f6D09+uhnfDBON8tAz0mPQH/6JqnOXm+EiUn+8bN0E8Nke\n"
  "/i3GEgDeaxJJMNQcpsJvmmSmKFOlYy9Fy7ejAjTGqtAnqOte7BnTAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key gUsq3e5iYgsQQvyxINtLzBpHxmIt5rtuFlEbKfI4gFk=\n"
  "id rsa1024 jv+LdatDzsMfEW6pLBeL/5uzwCc\n"
  /* Bad 2: RSA key has bad exponent of 3. */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGHAoGBAMMTWtvPxYnUNJ5Y7B+XENcpxzPoGstrdiUszCBS+/42xvluLJ+JDSdR\n"
  "qJaMD6ax8vKAeLS5C6O17MNdG2VldlPRbtgl41MXsOoUqEJ+nY9e3WG9Snjp47xC\n"
  "zmWIfeduXSavIsb3a43/MLIz/9qO0TkgAAiuQr79JlwKhLdzCqTLAgED\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key NkRB4wTUFogiVp5jYmjGORe2ffb/y5Kk8Itw8jdzMjA=\n"
  "p reject 25,119,135-139,445,563,1214,4661-4666,6346-6429,6699,6881-6999\n"
  "id rsa1024 fKvYjP7TAjCC1FzYee5bYAwYkoDg\n"
  /* Bad 3: Bogus annotation */
  "@last-listed with strange aeons\n"
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBALcRBFNCZtpd2TFJysU77/fJMFzKisRQEBOtDGtTZ2Bg4aEGosssa0Id\n"
  "YtUagRLYle08QVGvGB+EHBI5qf6Ah2yPH7k5QiN2a3Sq+nyh85dXKPazBGBBbM+C\n"
  "DOfDauV02CAnADNMLJEf1voY3oBVvYyIsmHxn5i1R19ZYIiR8NX5AgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "ntor-onion-key m4xcFXMWMjCvZDXq8FT3XmS0EHYseGOeu+fV+6FYDlk=\n"
  "p accept 20-23,43,53,79-81,88,110,143,194,220,389,443,464,531,543-544\n"
  "id rsa1024 SSbfNE9vmaiwRKH+eqNAkiKQhds\n"
  /* Good 10: Normal, with added ipv6 address and added other address */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAM7uUtq5F6h63QNYIvC+4NcWaD0DjtnrOORZMkdpJhinXUOwce3cD5Dj\n"
  "sgdN1wJpWpTQMXJ2DssfSgmOVXETP7qJuZyRprxalQhaEATMDNJA/66Ml1jSO9mZ\n"
  "+8Xb7m/4q778lNtkSbsvMaYD2Dq6k2QQ3kMhr9z8oUtX0XA23+pfAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "a [::1:2:3:4]:9090\n"
  "a 18.0.0.1:9999\n"
  "ntor-onion-key k2yFqTU2vzMCQDEiE/j9UcEHxKrXMLpB3IL0or09sik=\n"
  "id rsa1024 2A8wYpHxnkKJ92orocvIQBzeHlE\n"
  "p6 allow 80\n"
  /* Good 11: Normal, non-exit relay with ipv6 address */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAM7uUtq5F6h63QNYIvC+4NcWaD0DjtnrOORZMkdpJhinXUOwce3cD5Dj\n"
  "sgdN1wJpWpTQMXJ2DssfSgmOVXETP7qJuZyRprxalQhaEATMDNJA/66Ml1jSO9mZ\n"
  "+8Xb7m/4q778lNtkSbsvMaYD2Dq6k2QQ3kMhr9z8oUtX0XA23+pfAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "a [::1:2:3:4]:9090\n"
  "a 18.0.0.1:9999\n"
  "ntor-onion-key k2yFqTU2vzMCQDEiE/j9UcEHxKrXMLpB3IL0or09sik=\n"
  "id rsa1024 2A8wYpHxnkKJ92orocvIQBzeHlE\n"
  /* Good 12: Normal, exit relay with ipv6 address */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAM7uUtq5F6h63QNYIvC+4NcWaD0DjtnrOORZMkdpJhinXUOwce3cD5Dj\n"
  "sgdN1wJpWpTQMXJ2DssfSgmOVXETP7qJuZyRprxalQhaEATMDNJA/66Ml1jSO9mZ\n"
  "+8Xb7m/4q778lNtkSbsvMaYD2Dq6k2QQ3kMhr9z8oUtX0XA23+pfAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "a [::1:2:3:4]:9090\n"
  "a 18.0.0.1:9999\n"
  "ntor-onion-key k2yFqTU2vzMCQDEiE/j9UcEHxKrXMLpB3IL0or09sik=\n"
  "p accept 20-23,43,53,79-81,88,110,143,194,220,389,443,464,531,543-544\n"
  "id rsa1024 2A8wYpHxnkKJ92orocvIQBzeHlE\n"
  /* Good 13: Normal, exit relay with only ipv6 exit policy */
  "onion-key\n"
  "-----BEGIN RSA PUBLIC KEY-----\n"
  "MIGJAoGBAM7uUtq5F6h63QNYIvC+4NcWaD0DjtnrOORZMkdpJhinXUOwce3cD5Dj\n"
  "sgdN1wJpWpTQMXJ2DssfSgmOVXETP7qJuZyRprxalQhaEATMDNJA/66Ml1jSO9mZ\n"
  "+8Xb7m/4q778lNtkSbsvMaYD2Dq6k2QQ3kMhr9z8oUtX0XA23+pfAgMBAAE=\n"
  "-----END RSA PUBLIC KEY-----\n"
  "a [::1:2:3:4]:9090\n"
  "a 18.0.0.1:9999\n"
  "ntor-onion-key k2yFqTU2vzMCQDEiE/j9UcEHxKrXMLpB3IL0or09sik=\n"
  "p6 accept 20-23,43,53,79-81,88,110,143,194,220,389,443,464,531,543-544\n"
  "id rsa1024 2A8wYpHxnkKJ92orocvIQBzeHlE\n"
  ;
#ifdef HAVE_CFLAG_WOVERLENGTH_STRINGS
ENABLE_GCC_WARNING("-Woverlength-strings")
#endif

/** More tests for parsing different kinds of microdescriptors, and getting
 * invalid digests trackd from them. */
static void
test_md_parse(void *arg)
{
  (void) arg;
  char *mem_op_hex_tmp = NULL;
  smartlist_t *invalid = smartlist_new();

  smartlist_t *mds = microdescs_parse_from_string(MD_PARSE_TEST_DATA,
                                                  NULL, 1, SAVED_NOWHERE,
                                                  invalid);
  tt_int_op(smartlist_len(mds), OP_EQ, 14);
  tt_int_op(smartlist_len(invalid), OP_EQ, 4);

  test_memeq_hex(smartlist_get(invalid,0),
                 "5d76bf1c6614e885614a1e0ad074e1ab"
                 "4ea14655ebeefb1736a71b5ed8a15a51");
  test_memeq_hex(smartlist_get(invalid,1),
                 "2fde0ee3343669c2444cd9d53cbd39c6"
                 "a7d1fc0513513e840ca7f6e68864b36c");
  test_memeq_hex(smartlist_get(invalid,2),
                 "20d1576c5ab11bbcff0dedb1db4a3cfc"
                 "c8bc8dd839d8cbfef92d00a1a7d7b294");
  test_memeq_hex(smartlist_get(invalid,3),
                 "074770f394c73dbde7b44412e9692add"
                 "691a478d4727f9804b77646c95420a96");

  /* Spot-check the valid ones. */
  const microdesc_t *md = smartlist_get(mds, 5);
  test_memeq_hex(md->digest,
                 "54bb6d733ddeb375d2456c79ae103961"
                 "da0cae29620375ac4cf13d54da4d92b3");
  tt_int_op(md->last_listed, OP_EQ, 0);
  tt_int_op(md->saved_location, OP_EQ, SAVED_NOWHERE);
  tt_int_op(md->no_save, OP_EQ, 0);
  tt_uint_op(md->held_in_map, OP_EQ, 0);
  tt_uint_op(md->held_by_nodes, OP_EQ, 0);
  tt_assert(md->onion_curve25519_pkey);

  md = smartlist_get(mds, 6);
  test_memeq_hex(md->digest,
                 "53f740bd222ab37f19f604b1d3759aa6"
                 "5eff1fbce9ac254bd0fa50d4af9b1bae");
  tt_assert(! md->exit_policy);

  md = smartlist_get(mds, 8);
  test_memeq_hex(md->digest,
                 "a0a155562d8093d8fd0feb7b93b7226e"
                 "17f056c2142aab7a4ea8c5867a0376d5");
  tt_assert(md->onion_curve25519_pkey);

  md = smartlist_get(mds, 10);
  test_memeq_hex(md->digest,
                 "409ebd87d23925a2732bd467a92813c9"
                 "21ca378fcb9ca193d354c51550b6d5e9");
  tt_assert(tor_addr_family(&md->ipv6_addr) == AF_INET6);
  tt_int_op(md->ipv6_orport, OP_EQ, 9090);

  md = smartlist_get(mds, 11);
  tt_assert(tor_addr_family(&md->ipv6_addr) == AF_INET6);
  tt_int_op(md->ipv6_orport, OP_EQ, 9090);
  tt_int_op(md->policy_is_reject_star, OP_EQ, 1);

  md = smartlist_get(mds, 12);
  tt_assert(tor_addr_family(&md->ipv6_addr) == AF_INET6);
  tt_int_op(md->ipv6_orport, OP_EQ, 9090);
  tt_int_op(md->policy_is_reject_star, OP_EQ, 0);

  md = smartlist_get(mds, 13);
  tt_assert(tor_addr_family(&md->ipv6_addr) == AF_INET6);
  tt_int_op(md->ipv6_orport, OP_EQ, 9090);
  tt_int_op(md->policy_is_reject_star, OP_EQ, 0);

 done:
  SMARTLIST_FOREACH(mds, microdesc_t *, mdsc, microdesc_free(mdsc));
  smartlist_free(mds);
  SMARTLIST_FOREACH(invalid, char *, cp, tor_free(cp));
  smartlist_free(invalid);
  tor_free(mem_op_hex_tmp);
}

static void
test_md_parse_id_ed25519(void *arg)
{
  (void)arg;

  /* A correct MD with an ed25519 ID ... and an unspecified ID type,
   * which is permitted. */
  const char GOOD_MD[] =
    "onion-key\n"
    "-----BEGIN RSA PUBLIC KEY-----\n"
    "MIGJAoGBAM7uUtq5F6h63QNYIvC+4NcWaD0DjtnrOORZMkdpJhinXUOwce3cD5Dj\n"
    "sgdN1wJpWpTQMXJ2DssfSgmOVXETP7qJuZyRprxalQhaEATMDNJA/66Ml1jSO9mZ\n"
    "+8Xb7m/4q778lNtkSbsvMaYD2Dq6k2QQ3kMhr9z8oUtX0XA23+pfAgMBAAE=\n"
    "-----END RSA PUBLIC KEY-----\n"
    "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs=\n"
    "id ed25519 VGhpcyBpc24ndCBhY3R1YWxseSBhIHB1YmxpYyBrZXk\n"
    "id wumpus dodecahedron\n";

  smartlist_t *mds = NULL;
  const microdesc_t *md;

  mds = microdescs_parse_from_string(GOOD_MD,
                                     NULL, 1, SAVED_NOWHERE, NULL);
  tt_assert(mds);
  tt_int_op(smartlist_len(mds), OP_EQ, 1);
  md = smartlist_get(mds, 0);
  tt_mem_op(md->ed25519_identity_pkey, OP_EQ,
            "This isn't actually a public key", ED25519_PUBKEY_LEN);
  SMARTLIST_FOREACH(mds, microdesc_t *, m, microdesc_free(m));
  smartlist_free(mds);

  /* As above, but ed25519 ID key appears twice. */
  const char DUPLICATE_KEY[] =
    "onion-key\n"
    "-----BEGIN RSA PUBLIC KEY-----\n"
    "MIGJAoGBAM7uUtq5F6h63QNYIvC+4NcWaD0DjtnrOORZMkdpJhinXUOwce3cD5Dj\n"
    "sgdN1wJpWpTQMXJ2DssfSgmOVXETP7qJuZyRprxalQhaEATMDNJA/66Ml1jSO9mZ\n"
    "+8Xb7m/4q778lNtkSbsvMaYD2Dq6k2QQ3kMhr9z8oUtX0XA23+pfAgMBAAE=\n"
    "-----END RSA PUBLIC KEY-----\n"
    "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs\n"
    "id ed25519 VGhpcyBpc24ndCBhY3R1YWxseSBhIHB1YmxpYyBrZXk\n"
    "id ed25519 VGhpcyBpc24ndCBhY3R1YWxseSBhIHB1YmxpYyBrZXk\n";

  setup_capture_of_logs(LOG_WARN);
  mds = microdescs_parse_from_string(DUPLICATE_KEY,
                                     NULL, 1, SAVED_NOWHERE, NULL);
  tt_assert(mds);
  tt_int_op(smartlist_len(mds), OP_EQ, 0); // no entries.
  expect_single_log_msg_containing("Extra ed25519 key");
  mock_clean_saved_logs();
  smartlist_free(mds);

  /* As above, but ed25519 ID key is invalid. */
  const char BOGUS_KEY[] =
    "onion-key\n"
    "-----BEGIN RSA PUBLIC KEY-----\n"
    "MIGJAoGBAM7uUtq5F6h63QNYIvC+4NcWaD0DjtnrOORZMkdpJhinXUOwce3cD5Dj\n"
    "sgdN1wJpWpTQMXJ2DssfSgmOVXETP7qJuZyRprxalQhaEATMDNJA/66Ml1jSO9mZ\n"
    "+8Xb7m/4q778lNtkSbsvMaYD2Dq6k2QQ3kMhr9z8oUtX0XA23+pfAgMBAAE=\n"
    "-----END RSA PUBLIC KEY-----\n"
    "ntor-onion-key AppBt6CSeb1kKid/36ototmFA24ddfW5JpjWPLuoJgs\n"
    "id ed25519 VGhpcyBpc24ndCBhY3R1YWxseSBhIHB1YmxpYyZZZZZZZZZZZ\n";

  mds = microdescs_parse_from_string(BOGUS_KEY,
                                     NULL, 1, SAVED_NOWHERE, NULL);
  tt_assert(mds);
  tt_int_op(smartlist_len(mds), OP_EQ, 0); // no entries.
  expect_single_log_msg_containing("Bogus ed25519 key");

 done:
  if (mds) {
    SMARTLIST_FOREACH(mds, microdesc_t *, m, microdesc_free(m));
    smartlist_free(mds);
  }
  teardown_capture_of_logs();
}

static int mock_rgsbd_called = 0;
static routerstatus_t *mock_rgsbd_val_a = NULL;
static routerstatus_t *mock_rgsbd_val_b = NULL;
static routerstatus_t *
mock_router_get_status_by_digest(networkstatus_t *c, const char *d)
{
  (void) c;
  ++mock_rgsbd_called;

  if (fast_memeq(d, "\x5d\x76", 2)) {
    memcpy(mock_rgsbd_val_a->descriptor_digest, d, 32);
    return mock_rgsbd_val_a;
  } else if (fast_memeq(d, "\x20\xd1", 2)) {
    memcpy(mock_rgsbd_val_b->descriptor_digest, d, 32);
    return mock_rgsbd_val_b;
  } else {
    return NULL;
  }
}

static networkstatus_t *mock_ns_val = NULL;
static networkstatus_t *
mock_ns_get_by_flavor(consensus_flavor_t f)
{
  (void)f;
  return mock_ns_val;
}

static void
test_md_reject_cache(void *arg)
{
  (void) arg;
  microdesc_cache_t *mc = NULL ;
  smartlist_t *added = NULL, *wanted = smartlist_new();
  or_options_t *options = get_options_mutable();
  char buf[DIGEST256_LEN];

  tor_free(options->CacheDirectory);
  options->CacheDirectory = tor_strdup(get_fname("md_datadir_test_rej"));
  mock_rgsbd_val_a = tor_malloc_zero(sizeof(routerstatus_t));
  mock_rgsbd_val_b = tor_malloc_zero(sizeof(routerstatus_t));
  mock_ns_val = tor_malloc_zero(sizeof(networkstatus_t));

  mock_ns_val->valid_after = time(NULL) - 86400;
  mock_ns_val->valid_until = time(NULL) + 86400;
  mock_ns_val->flavor = FLAV_MICRODESC;

#ifdef _WIN32
  tt_int_op(0, OP_EQ, mkdir(options->CacheDirectory));
#else
  tt_int_op(0, OP_EQ, mkdir(options->CacheDirectory, 0700));
#endif

  MOCK(router_get_mutable_consensus_status_by_descriptor_digest,
       mock_router_get_status_by_digest);
  MOCK(networkstatus_get_latest_consensus_by_flavor, mock_ns_get_by_flavor);

  mc = get_microdesc_cache();
#define ADD(hex)                                                        \
  do {                                                                  \
    tt_int_op(sizeof(buf),OP_EQ,base16_decode(buf,sizeof(buf),          \
                hex,strlen(hex)));\
    smartlist_add(wanted, tor_memdup(buf, DIGEST256_LEN));              \
  } while (0)

  /* invalid,0 */
  ADD("5d76bf1c6614e885614a1e0ad074e1ab4ea14655ebeefb1736a71b5ed8a15a51");
  /* invalid,2 */
  ADD("20d1576c5ab11bbcff0dedb1db4a3cfcc8bc8dd839d8cbfef92d00a1a7d7b294");
  /* valid, 6 */
  ADD("53f740bd222ab37f19f604b1d3759aa65eff1fbce9ac254bd0fa50d4af9b1bae");
  /* valid, 8 */
  ADD("a0a155562d8093d8fd0feb7b93b7226e17f056c2142aab7a4ea8c5867a0376d5");

  added = microdescs_add_to_cache(mc, MD_PARSE_TEST_DATA, NULL,
                                  SAVED_NOWHERE, 0, time(NULL), wanted);

  tt_int_op(smartlist_len(added), OP_EQ, 2);
  tt_int_op(mock_rgsbd_called, OP_EQ, 2);
  tt_int_op(mock_rgsbd_val_a->dl_status.n_download_failures, OP_EQ, 255);
  tt_int_op(mock_rgsbd_val_b->dl_status.n_download_failures, OP_EQ, 255);

 done:
  UNMOCK(networkstatus_get_latest_consensus_by_flavor);
  UNMOCK(router_get_mutable_consensus_status_by_descriptor_digest);
  tor_free(options->CacheDirectory);
  microdesc_free_all();
  smartlist_free(added);
  SMARTLIST_FOREACH(wanted, char *, cp, tor_free(cp));
  smartlist_free(wanted);
  tor_free(mock_rgsbd_val_a);
  tor_free(mock_rgsbd_val_b);
  tor_free(mock_ns_val);
}

static void
test_md_corrupt_desc(void *arg)
{
  char *cp = NULL;
  smartlist_t *sl = NULL;
  (void) arg;

  sl = microdescs_add_to_cache(get_microdesc_cache(),
                               "@last-listed 2015-06-22 10:00:00\n"
                               "onion-k\n",
                               NULL, SAVED_IN_JOURNAL, 0, time(NULL), NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 0);
  smartlist_free(sl);

  sl = microdescs_add_to_cache(get_microdesc_cache(),
                               "@last-listed 2015-06-22 10:00:00\n"
                               "wiggly\n",
                               NULL, SAVED_IN_JOURNAL, 0, time(NULL), NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 0);
  smartlist_free(sl);

  tor_asprintf(&cp, "%s\n%s", test_md1, "@foobar\nonion-wobble\n");

  sl = microdescs_add_to_cache(get_microdesc_cache(),
                               cp, cp+strlen(cp),
                               SAVED_IN_JOURNAL, 0, time(NULL), NULL);
  tt_int_op(smartlist_len(sl), OP_EQ, 0);

 done:
  tor_free(cp);
  smartlist_free(sl);
}

struct testcase_t microdesc_tests[] = {
  { "cache", test_md_cache, TT_FORK, NULL, NULL },
  { "broken_cache", test_md_cache_broken, TT_FORK, NULL, NULL },
  { "generate", test_md_generate, 0, NULL, NULL },
  { "parse", test_md_parse, 0, NULL, NULL },
  { "parse_id_ed25519", test_md_parse_id_ed25519, 0, NULL, NULL },
  { "reject_cache", test_md_reject_cache, TT_FORK, NULL, NULL },
  { "corrupt_desc", test_md_corrupt_desc, TT_FORK, NULL, NULL },
  END_OF_TESTCASES
};
