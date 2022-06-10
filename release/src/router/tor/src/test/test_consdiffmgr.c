/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CONSDIFFMGR_PRIVATE

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/dircache/conscache.h"
#include "feature/dircommon/consdiff.h"
#include "feature/dircache/consdiffmgr.h"
#include "core/mainloop/cpuworker.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/dirparse/ns_parse.h"
#include "lib/evloop/workqueue.h"
#include "lib/compress/compress.h"
#include "lib/encoding/confline.h"

#include "feature/nodelist/networkstatus_st.h"

#include "test/test.h"
#include "test/log_test_helpers.h"

#define consdiffmgr_add_consensus consdiffmgr_add_consensus_nulterm

static char *
consensus_diff_apply_(const char *c, const char *d)
{
  size_t c_len = strlen(c);
  size_t d_len = strlen(d);
  // We use memdup here to ensure that the input is NOT nul-terminated.
  // This makes it likelier for us to spot bugs.
  char *c_tmp = tor_memdup(c, c_len);
  char *d_tmp = tor_memdup(d, d_len);
  char *result = consensus_diff_apply(c_tmp, c_len, d_tmp, d_len);
  tor_free(c_tmp);
  tor_free(d_tmp);
  return result;
}

// ============================== Setup/teardown the consdiffmgr
// These functions get run before/after each test in this module

static void *
consdiffmgr_test_setup(const struct testcase_t *arg)
{
  (void)arg;
  char *ddir_fname = tor_strdup(get_fname_rnd("datadir_cdm"));
  tor_free(get_options_mutable()->CacheDirectory);
  get_options_mutable()->CacheDirectory = ddir_fname; // now owns the pointer.
  check_private_dir(ddir_fname, CPD_CREATE, NULL);

  consdiff_cfg_t consdiff_cfg = { 300 };
  consdiffmgr_configure(&consdiff_cfg);
  return (void *)1; // must return something non-null.
}
static int
consdiffmgr_test_teardown(const struct testcase_t *arg, void *ignore)
{
  (void)arg;
  (void)ignore;
  consdiffmgr_free_all();
  return 1;
}
static struct testcase_setup_t setup_diffmgr = {
  consdiffmgr_test_setup,
  consdiffmgr_test_teardown
};

// ============================== NS faking functions
// These functions are for making quick fake consensus objects and
// strings that are just good enough for consdiff and consdiffmgr.

static networkstatus_t *
fake_ns_new(consensus_flavor_t flav, time_t valid_after)
{
  networkstatus_t *ns = tor_malloc_zero(sizeof(networkstatus_t));
  ns->type = NS_TYPE_CONSENSUS;
  ns->flavor = flav;
  ns->valid_after = valid_after;
  return ns;
}

static char *
fake_ns_body_new(consensus_flavor_t flav, time_t valid_after)
{
  const char *flavor_string = flav == FLAV_NS ? "" : " microdesc";
  char valid_after_string[ISO_TIME_LEN+1];

  format_iso_time(valid_after_string, valid_after);
  char *random_stuff = crypto_random_hostname(3, 25, "junk ", "");
  char *random_stuff2 = crypto_random_hostname(3, 10, "", "");

  char *consensus;
  tor_asprintf(&consensus,
               "network-status-version 3%s\n"
               "vote-status consensus\n"
               "valid-after %s\n"
               "r name ccccccccccccccccc etc\nsample\n"
               "r name eeeeeeeeeeeeeeeee etc\nbar\n"
               "%s\n"
               "directory-signature hello-there\n"
               "directory-signature %s\n",
               flavor_string,
               valid_after_string,
               random_stuff,
               random_stuff2);
  tor_free(random_stuff);
  tor_free(random_stuff2);
  return consensus;
}

// ============================== Cpuworker mocking code
// These mocking functions and types capture the cpuworker calls
// so we can inspect them and run them in the main thread.
static smartlist_t *fake_cpuworker_queue = NULL;
typedef struct fake_work_queue_ent_t {
  enum workqueue_reply_t (*fn)(void *, void *);
  void (*reply_fn)(void *);
  void *arg;
} fake_work_queue_ent_t;
static struct workqueue_entry_t *
mock_cpuworker_queue_work(workqueue_priority_t prio,
                          enum workqueue_reply_t (*fn)(void *, void *),
                          void (*reply_fn)(void *),
                          void *arg)
{
  (void) prio;

  if (! fake_cpuworker_queue)
    fake_cpuworker_queue = smartlist_new();

  fake_work_queue_ent_t *ent = tor_malloc_zero(sizeof(*ent));
  ent->fn = fn;
  ent->reply_fn = reply_fn;
  ent->arg = arg;
  smartlist_add(fake_cpuworker_queue, ent);
  return (struct workqueue_entry_t *)ent;
}
static int
mock_cpuworker_run_work(void)
{
  if (! fake_cpuworker_queue)
    return 0;
  SMARTLIST_FOREACH(fake_cpuworker_queue, fake_work_queue_ent_t *, ent, {
      enum workqueue_reply_t r = ent->fn(NULL, ent->arg);
      if (r != WQ_RPL_REPLY)
        return -1;
  });
  return 0;
}
static void
mock_cpuworker_handle_replies(void)
{
  if (! fake_cpuworker_queue)
    return;
  SMARTLIST_FOREACH(fake_cpuworker_queue, fake_work_queue_ent_t *, ent, {
      ent->reply_fn(ent->arg);
      tor_free(ent);
  });
  smartlist_free(fake_cpuworker_queue);
  fake_cpuworker_queue = NULL;
}

// ==============================  Other helpers

static consdiff_status_t
lookup_diff_from(consensus_cache_entry_t **out,
                 consensus_flavor_t flav,
                 const char *str1)
{
  uint8_t digest[DIGEST256_LEN];
  if (router_get_networkstatus_v3_sha3_as_signed(digest,
                                                 str1, strlen(str1))<0) {
    TT_FAIL(("Unable to compute sha3-as-signed"));
    return CONSDIFF_NOT_FOUND;
  }
  return consdiffmgr_find_diff_from(out, flav,
                                    DIGEST_SHA3_256, digest, sizeof(digest),
                                    NO_METHOD);
}

static int
lookup_apply_and_verify_diff(consensus_flavor_t flav,
                             const char *str1,
                             const char *str2)
{
  consensus_cache_entry_t *ent = NULL;
  consdiff_status_t status = lookup_diff_from(&ent, flav, str1);
  if (ent == NULL || status != CONSDIFF_AVAILABLE) {
    return -1;
  }

  consensus_cache_entry_incref(ent);
  size_t size;
  const char *diff_string = NULL;
  char *diff_owned = NULL;
  int r = uncompress_or_set_ptr(&diff_string, &size, &diff_owned, ent);
  consensus_cache_entry_decref(ent);
  if (diff_string == NULL || r < 0)
    return -1;

  char *applied = consensus_diff_apply(str1, strlen(str1), diff_string, size);
  tor_free(diff_owned);
  if (applied == NULL)
    return -1;

  int match = !strcmp(applied, str2);
  tor_free(applied);
  return match ? 0 : -1;
}

static void
cdm_reload(void)
{
  consdiffmgr_free_all();
  cdm_cache_get();
  consdiffmgr_rescan();
}

// ==============================  Beginning of tests

#if 0
static int got_failure = 0;
static void
got_assertion_failure(void)
{
  ++got_failure;
}

/* XXXX This test won't work, because there is currently no way to actually
 * XXXX capture a real assertion failure. */
static void
test_consdiffmgr_init_failure(void *arg)
{
  (void)arg;
  // Capture assertions and bugs.

  /* As in ...test_setup, but do not create the datadir. The missing directory
   * will cause a failure. */
  char *ddir_fname = tor_strdup(get_fname_rnd("datadir_cdm"));
  tor_free(get_options_mutable()->CacheDirectory);
  get_options_mutable()->CacheDirectory = ddir_fname; // now owns the pointer.

  consdiff_cfg_t consdiff_cfg = { 7200, 300 };

  tor_set_failed_assertion_callback(got_assertion_failure);
  tor_capture_bugs_(1);
  consdiffmgr_configure(&consdiff_cfg); // This should fail.
  tt_int_op(got_failure, OP_EQ, 1);
  const smartlist_t *bugs = tor_get_captured_bug_log_();
  tt_int_op(smartlist_len(bugs), OP_EQ, 1);

 done:
  tor_end_capture_bugs_();
}
#endif /* 0 */

static void
test_consdiffmgr_sha3_helper(void *arg)
{
  (void) arg;
  consensus_cache_t *cache = cdm_cache_get(); // violate abstraction barrier
  config_line_t *lines = NULL;
  char *mem_op_hex_tmp = NULL;
  config_line_prepend(&lines, "good-sha",
                      "F00DF00DF00DF00DF00DF00DF00DF00D"
                      "F00DF00DF00DF00DF00DF00DF00DF00D");
  config_line_prepend(&lines, "short-sha",
                      "F00DF00DF00DF00DF00DF00DF00DF00D"
                      "F00DF00DF00DF00DF00DF00DF00DF0");
  config_line_prepend(&lines, "long-sha",
                      "F00DF00DF00DF00DF00DF00DF00DF00D"
                      "F00DF00DF00DF00DF00DF00DF00DF00DF00D");
  config_line_prepend(&lines, "not-sha",
                      "F00DF00DF00DF00DF00DF00DF00DF00D"
                      "F00DF00DF00DF00DF00DF00DF00DXXXX");
  consensus_cache_entry_t *ent =
    consensus_cache_add(cache, lines, (const uint8_t *)"Hi there", 8);

  uint8_t buf[DIGEST256_LEN];
  tt_int_op(-1, OP_EQ, cdm_entry_get_sha3_value(buf, NULL, "good-sha"));
  tt_int_op(0, OP_EQ, cdm_entry_get_sha3_value(buf, ent, "good-sha"));
  test_memeq_hex(buf, "F00DF00DF00DF00DF00DF00DF00DF00D"
                      "F00DF00DF00DF00DF00DF00DF00DF00D");

  tt_int_op(-1, OP_EQ, cdm_entry_get_sha3_value(buf, ent, "missing-sha"));
  tt_int_op(-2, OP_EQ, cdm_entry_get_sha3_value(buf, ent, "short-sha"));
  tt_int_op(-2, OP_EQ, cdm_entry_get_sha3_value(buf, ent, "long-sha"));
  tt_int_op(-2, OP_EQ, cdm_entry_get_sha3_value(buf, ent, "not-sha"));

 done:
  consensus_cache_entry_decref(ent);
  config_free_lines(lines);
  tor_free(mem_op_hex_tmp);
}

static void
test_consdiffmgr_add(void *arg)
{
  (void) arg;
  time_t now = approx_time();

  const char *body = NULL;
  char *body_owned = NULL;

  consensus_cache_entry_t *ent = NULL;
  networkstatus_t *ns_tmp = fake_ns_new(FLAV_NS, now);
  const char *dummy = "foo";
  int r = consdiffmgr_add_consensus(dummy, ns_tmp);
  tt_int_op(r, OP_EQ, 0);

  /* If we add it again, it won't work */
  setup_capture_of_logs(LOG_INFO);
  dummy = "bar";
  r = consdiffmgr_add_consensus(dummy, ns_tmp);
  tt_int_op(r, OP_EQ, -1);
  expect_single_log_msg_containing("We already have a copy of that "
                                   "consensus");
  mock_clean_saved_logs();

  /* But it will work fine if the flavor is different */
  dummy = "baz";
  ns_tmp->flavor = FLAV_MICRODESC;
  r = consdiffmgr_add_consensus(dummy, ns_tmp);
  tt_int_op(r, OP_EQ, 0);

  /* And it will work fine if the time is different */
  dummy = "quux";
  ns_tmp->flavor = FLAV_NS;
  ns_tmp->valid_after = now - 60;
  r = consdiffmgr_add_consensus(dummy, ns_tmp);
  tt_int_op(r, OP_EQ, 0);

  /* If we add one a long long time ago, it will fail. */
  dummy = "xyzzy";
  ns_tmp->valid_after = 86400 * 100; /* A few months into 1970 */
  r = consdiffmgr_add_consensus(dummy, ns_tmp);
  tt_int_op(r, OP_EQ, -1);
  expect_log_msg_containing("it's too old.");

  /* Try looking up a consensuses. */
  ent = cdm_cache_lookup_consensus(FLAV_NS, now-60);
  tt_assert(ent);
  consensus_cache_entry_incref(ent);
  size_t s;
  r = uncompress_or_set_ptr(&body, &s, &body_owned, ent);
  tt_int_op(r, OP_EQ, 0);
  tt_int_op(s, OP_EQ, 4);
  tt_mem_op(body, OP_EQ, "quux", 4);

  /* Try looking up another entry, but fail */
  tt_ptr_op(cdm_cache_lookup_consensus(FLAV_MICRODESC, now - 60), OP_EQ, NULL);
  tt_ptr_op(cdm_cache_lookup_consensus(FLAV_NS, now - 61), OP_EQ, NULL);

 done:
  networkstatus_vote_free(ns_tmp);
  teardown_capture_of_logs();
  consensus_cache_entry_decref(ent);
  tor_free(body_owned);
}

static void
test_consdiffmgr_make_diffs(void *arg)
{
  (void)arg;
  networkstatus_t *ns = NULL;
  char *ns_body = NULL, *md_ns_body = NULL, *md_ns_body_2 = NULL;
  char *applied = NULL, *diff_text = NULL;
  time_t now = approx_time();
  int r;
  consensus_cache_entry_t *diff = NULL;
  uint8_t md_ns_sha3[DIGEST256_LEN];
  consdiff_status_t diff_status;

  MOCK(cpuworker_queue_work, mock_cpuworker_queue_work);

  // Try rescan with no consensuses: shouldn't crash or queue work.
  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_EQ, fake_cpuworker_queue);

  // Make two consensuses, 1 hour sec ago.
  ns = fake_ns_new(FLAV_NS, now-3600);
  ns_body = fake_ns_body_new(FLAV_NS, now-3600);
  r = consdiffmgr_add_consensus(ns_body, ns);
  networkstatus_vote_free(ns);
  tor_free(ns_body);
  tt_int_op(r, OP_EQ, 0);

  ns = fake_ns_new(FLAV_MICRODESC, now-3600);
  md_ns_body = fake_ns_body_new(FLAV_MICRODESC, now-3600);
  r = consdiffmgr_add_consensus(md_ns_body, ns);
  router_get_networkstatus_v3_sha3_as_signed(md_ns_sha3, md_ns_body,
                                             strlen(md_ns_body));
  networkstatus_vote_free(ns);
  tt_int_op(r, OP_EQ, 0);

  // No diffs will be generated.
  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_EQ, fake_cpuworker_queue);

  // Add a MD consensus from 45 minutes ago. This should cause one diff
  // worth of work to get queued.
  ns = fake_ns_new(FLAV_MICRODESC, now-45*60);
  md_ns_body_2 = fake_ns_body_new(FLAV_MICRODESC, now-45*60);
  r = consdiffmgr_add_consensus(md_ns_body_2, ns);
  networkstatus_vote_free(ns);
  tt_int_op(r, OP_EQ, 0);

  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_NE, fake_cpuworker_queue);
  tt_int_op(1, OP_EQ, smartlist_len(fake_cpuworker_queue));
  diff_status = consdiffmgr_find_diff_from(&diff, FLAV_MICRODESC,
                                           DIGEST_SHA3_256,
                                           md_ns_sha3, DIGEST256_LEN,
                                           NO_METHOD);
  tt_int_op(CONSDIFF_IN_PROGRESS, OP_EQ, diff_status);

  // Now run that process and get the diff.
  r = mock_cpuworker_run_work();
  tt_int_op(r, OP_EQ, 0);
  mock_cpuworker_handle_replies();

  // At this point we should be able to get that diff.
  diff_status = consdiffmgr_find_diff_from(&diff, FLAV_MICRODESC,
                                           DIGEST_SHA3_256,
                                           md_ns_sha3, DIGEST256_LEN,
                                           NO_METHOD);
  tt_int_op(CONSDIFF_AVAILABLE, OP_EQ, diff_status);
  tt_assert(diff);

  /* Make sure applying the diff actually works */
  const uint8_t *diff_body;
  size_t diff_size;
  r = consensus_cache_entry_get_body(diff, &diff_body, &diff_size);
  tt_int_op(r, OP_EQ, 0);
  diff_text = tor_memdup_nulterm(diff_body, diff_size);
  applied = consensus_diff_apply_(md_ns_body, diff_text);
  tt_assert(applied);
  tt_str_op(applied, OP_EQ, md_ns_body_2);

  /* Rescan again: no more work to do. */
  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_EQ, fake_cpuworker_queue);

 done:
  tor_free(md_ns_body);
  tor_free(md_ns_body_2);
  tor_free(diff_text);
  tor_free(applied);
}

static void
test_consdiffmgr_diff_rules(void *arg)
{
  (void)arg;
#define N 6
  char *md_body[N], *ns_body[N];
  networkstatus_t *md_ns[N], *ns_ns[N];
  int i;

  MOCK(cpuworker_queue_work, mock_cpuworker_queue_work);

  /* Create a bunch of consensus things at 15-second intervals. */
  time_t start = approx_time() - 120;
  for (i = 0; i < N; ++i) {
    time_t when = start + i * 15;
    md_body[i] = fake_ns_body_new(FLAV_MICRODESC, when);
    ns_body[i] = fake_ns_body_new(FLAV_NS, when);
    md_ns[i] = fake_ns_new(FLAV_MICRODESC, when);
    ns_ns[i] = fake_ns_new(FLAV_NS, when);
  }

  /* For the MD consensuses: add 4 of them, and make sure that
   * diffs are created to one consensus (the most recent) only. */
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[1], md_ns[1]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[2], md_ns[2]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[3], md_ns[3]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[4], md_ns[4]));
  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_NE, fake_cpuworker_queue);
  tt_int_op(3, OP_EQ, smartlist_len(fake_cpuworker_queue));
  tt_int_op(0, OP_EQ, mock_cpuworker_run_work());
  mock_cpuworker_handle_replies();
  tt_ptr_op(NULL, OP_EQ, fake_cpuworker_queue);

  /* For the NS consensuses: add 3, generate, and add one older one and
   * make sure that older one is the only one whose diff is generated */
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(ns_body[0], ns_ns[0]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(ns_body[1], ns_ns[1]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(ns_body[5], ns_ns[5]));
  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_NE, fake_cpuworker_queue);
  tt_int_op(2, OP_EQ, smartlist_len(fake_cpuworker_queue));
  tt_int_op(0, OP_EQ, mock_cpuworker_run_work());
  mock_cpuworker_handle_replies();

  /* At this point, we should actually have working diffs! */
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_NS, ns_body[0], ns_body[5]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_NS, ns_body[1], ns_body[5]));

  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[1], md_body[4]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[2], md_body[4]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[3], md_body[4]));

  /* Self-to-self diff won't be present */
  consensus_cache_entry_t *ent;
  tt_int_op(CONSDIFF_NOT_FOUND, OP_EQ,
       lookup_diff_from(&ent, FLAV_NS, ns_body[5]));
  /* No diff from 2 has been added yet */
  tt_int_op(CONSDIFF_NOT_FOUND, OP_EQ,
       lookup_diff_from(&ent, FLAV_NS, ns_body[2]));
  /* No diff arriving at old things. */
  tt_int_op(-1, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[1], md_body[2]));
  /* No backwards diff */
  tt_int_op(-1, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[4], md_body[3]));

  /* Now, an update: add number 2 and make sure it's the only one whose diff
   * is regenerated. */
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(ns_body[2], ns_ns[2]));
  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_NE, fake_cpuworker_queue);
  tt_int_op(1, OP_EQ, smartlist_len(fake_cpuworker_queue));
  tt_int_op(0, OP_EQ, mock_cpuworker_run_work());
  mock_cpuworker_handle_replies();

  tt_int_op(0, OP_EQ,
            lookup_apply_and_verify_diff(FLAV_NS, ns_body[2], ns_body[5]));

  /* Finally: reload, and make sure that the information is still indexed */
  cdm_reload();

  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_NS, ns_body[0], ns_body[5]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_NS, ns_body[2], ns_body[5]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_NS, ns_body[1], ns_body[5]));

  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[1], md_body[4]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[2], md_body[4]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[3], md_body[4]));

 done:
  for (i = 0; i < N; ++i) {
    tor_free(md_body[i]);
    tor_free(ns_body[i]);
    networkstatus_vote_free(md_ns[i]);
    networkstatus_vote_free(ns_ns[i]);
  }
  UNMOCK(cpuworker_queue_work);
#undef N
}

static void
test_consdiffmgr_diff_failure(void *arg)
{
  (void)arg;
  MOCK(cpuworker_queue_work, mock_cpuworker_queue_work);

  /* We're going to make sure that if we have a bogus request where
   * we can't actually compute a diff, the world must not end. */
  networkstatus_t *ns1 = NULL;
  networkstatus_t *ns2 = NULL;
  int r;

  ns1 = fake_ns_new(FLAV_NS, approx_time()-100);
  ns2 = fake_ns_new(FLAV_NS, approx_time()-50);
  r = consdiffmgr_add_consensus("foo bar baz\n", ns1);
  tt_int_op(r, OP_EQ, 0);
  // We refuse to compute a diff to or from a line holding only a single dot.
  // We can add it here, though.
  r = consdiffmgr_add_consensus("foo bar baz\n.\n.\n", ns2);
  tt_int_op(r, OP_EQ, 0);

  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_NE, fake_cpuworker_queue);
  setup_capture_of_logs(LOG_WARN);
  tt_int_op(1, OP_EQ, smartlist_len(fake_cpuworker_queue));
  tt_int_op(0, OP_EQ, mock_cpuworker_run_work());
  expect_single_log_msg_containing("one of the lines to be added is \".\".");
  mock_clean_saved_logs();
  mock_cpuworker_handle_replies();
  expect_single_log_msg_containing("Worker was unable to compute consensus "
                                   "diff from ");

  /* Make sure the diff is not present */
  consensus_cache_entry_t *ent;
  tt_int_op(CONSDIFF_NOT_FOUND, OP_EQ,
            lookup_diff_from(&ent, FLAV_NS, "foo bar baz\n"));

 done:
  teardown_capture_of_logs();
  UNMOCK(cpuworker_queue_work);
  networkstatus_vote_free(ns1);
  networkstatus_vote_free(ns2);
}

static void
test_consdiffmgr_diff_pending(void *arg)
{
#define N 3
  (void)arg;
  char *md_body[N];
  networkstatus_t *md_ns[N];
  time_t start = approx_time() - 120;
  int i;
  for (i = 0; i < N; ++i) {
    time_t when = start + i * 30;
    md_body[i] = fake_ns_body_new(FLAV_MICRODESC, when);
    md_ns[i] = fake_ns_new(FLAV_MICRODESC, when);
  }

  MOCK(cpuworker_queue_work, mock_cpuworker_queue_work);

  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[1], md_ns[1]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[2], md_ns[2]));
  /* Make a diff */
  consdiffmgr_rescan();
  tt_int_op(1, OP_EQ, smartlist_len(fake_cpuworker_queue));

  /* Look it up.  Is it pending? */
  consensus_cache_entry_t *ent = NULL;
  consdiff_status_t diff_status;
  diff_status = lookup_diff_from(&ent, FLAV_MICRODESC, md_body[1]);
  tt_int_op(CONSDIFF_IN_PROGRESS, OP_EQ, diff_status);
  tt_ptr_op(ent, OP_EQ, NULL);

  /* Add another old consensus.  only one new diff should launch! */
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[0], md_ns[0]));
  consdiffmgr_rescan();
  tt_int_op(2, OP_EQ, smartlist_len(fake_cpuworker_queue));

  tt_int_op(0, OP_EQ, mock_cpuworker_run_work());
  mock_cpuworker_handle_replies();

  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[0], md_body[2]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[1], md_body[2]));

 done:
  UNMOCK(cpuworker_queue_work);
  for (i = 0; i < N; ++i) {
    tor_free(md_body[i]);
    networkstatus_vote_free(md_ns[i]);
  }
#undef N
}

static void
test_consdiffmgr_cleanup_old(void *arg)
{
  (void)arg;
  config_line_t *labels = NULL;
  consensus_cache_entry_t *ent = NULL;
  consensus_cache_t *cache = cdm_cache_get(); // violate abstraction barrier

  /* This item will be will be cleanable because it has a valid-after
   * time far in the past. */
  config_line_prepend(&labels, "document-type", "confribble-blarg");
  config_line_prepend(&labels, "consensus-valid-after",
                      "1980-10-10T10:10:10");
  ent = consensus_cache_add(cache, labels, (const uint8_t*)"Foo", 3);
  tt_assert(ent);
  consensus_cache_entry_decref(ent);

  setup_capture_of_logs(LOG_DEBUG);
  tt_int_op(1, OP_EQ, consdiffmgr_cleanup());
  expect_log_msg_containing("Deleting entry because its consensus-valid-"
                            "after value (1980-10-10T10:10:10) was too old");

 done:
  teardown_capture_of_logs();
  config_free_lines(labels);
}

static void
test_consdiffmgr_cleanup_bad_valid_after(void *arg)
{
  /* This will seem cleanable, but isn't, because its valid-after time is
   * malformed. */

  (void)arg;
  config_line_t *labels = NULL;
  consensus_cache_entry_t *ent = NULL;
  consensus_cache_t *cache = cdm_cache_get(); // violate abstraction barrier

  config_line_prepend(&labels, "document-type", "consensus");
  config_line_prepend(&labels, "consensus-valid-after",
                      "whan that aprille with his shoures soote"); // (~1385?)
  ent = consensus_cache_add(cache, labels, (const uint8_t*)"Foo", 3);
  tt_assert(ent);
  consensus_cache_entry_decref(ent);

  setup_capture_of_logs(LOG_DEBUG);
  tt_int_op(0, OP_EQ, consdiffmgr_cleanup());
  expect_log_msg_containing("Ignoring entry because its consensus-valid-"
                            "after value (\"whan that aprille with his "
                            "shoures soote\") was unparseable");

 done:
  teardown_capture_of_logs();
  config_free_lines(labels);
}

static void
test_consdiffmgr_cleanup_no_valid_after(void *arg)
{
  (void)arg;
  config_line_t *labels = NULL;
  consensus_cache_entry_t *ent = NULL;
  consensus_cache_t *cache = cdm_cache_get(); // violate abstraction barrier

  /* This item will be will be uncleanable because it has no recognized
   * valid-after. */
  config_line_prepend(&labels, "document-type", "consensus");
  config_line_prepend(&labels, "confrooble-voolid-oofter",
                      "2010-10-10T09:08:07");
  ent = consensus_cache_add(cache, labels, (const uint8_t*)"Foo", 3);
  tt_assert(ent);
  consensus_cache_entry_decref(ent);

  setup_capture_of_logs(LOG_DEBUG);
  tt_int_op(0, OP_EQ, consdiffmgr_cleanup());
  expect_log_msg_containing("Ignoring entry because it had no consensus-"
                            "valid-after label");

 done:
  teardown_capture_of_logs();
  config_free_lines(labels);
}

static void
test_consdiffmgr_cleanup_old_diffs(void *arg)
{
  (void)arg;
#define N 4
  char *md_body[N];
  networkstatus_t *md_ns[N];
  int i;
  consensus_cache_entry_t *hold_ent = NULL, *ent;

  /* Make sure that the cleanup function removes diffs to the not-most-recent
   * consensus. */

  MOCK(cpuworker_queue_work, mock_cpuworker_queue_work);

  /* Create a bunch of consensus things at 15-second intervals. */
  time_t start = approx_time() - 120;
  for (i = 0; i < N; ++i) {
    time_t when = start + i * 15;
    md_body[i] = fake_ns_body_new(FLAV_MICRODESC, when);
    md_ns[i] = fake_ns_new(FLAV_MICRODESC, when);
  }

  /* add the first 3. */
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[0], md_ns[0]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[1], md_ns[1]));
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[2], md_ns[2]));
  /* Make diffs. */
  consdiffmgr_rescan();
  tt_ptr_op(NULL, OP_NE, fake_cpuworker_queue);
  tt_int_op(2, OP_EQ, smartlist_len(fake_cpuworker_queue));
  tt_int_op(0, OP_EQ, mock_cpuworker_run_work());
  mock_cpuworker_handle_replies();
  tt_ptr_op(NULL, OP_EQ, fake_cpuworker_queue);

  /* Nothing is deletable now */
  tt_int_op(0, OP_EQ, consdiffmgr_cleanup());
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[0], md_body[2]));
  tt_int_op(0, OP_EQ,
       lookup_apply_and_verify_diff(FLAV_MICRODESC, md_body[1], md_body[2]));

  tt_int_op(CONSDIFF_AVAILABLE, OP_EQ,
            lookup_diff_from(&hold_ent, FLAV_MICRODESC, md_body[1]));
  consensus_cache_entry_incref(hold_ent); // incref, so it is preserved.

  /* Now add an even-more-recent consensus; this should make all previous
   * diffs deletable, and make delete */
  tt_int_op(0, OP_EQ, consdiffmgr_add_consensus(md_body[3], md_ns[3]));
  tt_int_op(2 * n_diff_compression_methods() +
            (n_consensus_compression_methods() - 1) , OP_EQ,
            consdiffmgr_cleanup());

  tt_int_op(CONSDIFF_NOT_FOUND, OP_EQ,
            lookup_diff_from(&ent, FLAV_MICRODESC, md_body[0]));
  /* This one is marked deletable but still in the hashtable */
  tt_int_op(CONSDIFF_AVAILABLE, OP_EQ,
            lookup_diff_from(&ent, FLAV_MICRODESC, md_body[1]));
  tt_int_op(CONSDIFF_NOT_FOUND, OP_EQ,
            lookup_diff_from(&ent, FLAV_MICRODESC, md_body[2]));

  /* Everything should be valid at this point */
  tt_int_op(0, OP_EQ, consdiffmgr_validate());

  /* And if we recan NOW, we'll purge the hashtable of the entries,
   * and launch attempts to generate new ones */
  consdiffmgr_rescan();
  tt_int_op(CONSDIFF_IN_PROGRESS, OP_EQ,
            lookup_diff_from(&ent, FLAV_MICRODESC, md_body[0]));
  tt_int_op(CONSDIFF_IN_PROGRESS, OP_EQ,
            lookup_diff_from(&ent, FLAV_MICRODESC, md_body[1]));
  tt_int_op(CONSDIFF_IN_PROGRESS, OP_EQ,
            lookup_diff_from(&ent, FLAV_MICRODESC, md_body[2]));

  /* We're still holding on to this, though, so we can still map it! */
  const uint8_t *t1 = NULL;
  size_t s;
  int r = consensus_cache_entry_get_body(hold_ent, &t1, &s);
  tt_int_op(r, OP_EQ, 0);
  tt_assert(t1);

 done:
  for (i = 0; i < N; ++i) {
    tor_free(md_body[i]);
    networkstatus_vote_free(md_ns[i]);
  }
  consensus_cache_entry_decref(hold_ent);
  UNMOCK(cpuworker_queue_work);
#undef N
}

static void
test_consdiffmgr_validate(void *arg)
{
  (void)arg;
  config_line_t *lines = NULL;
  consensus_cache_entry_t *ent = NULL;
  consensus_cache_t *cache = cdm_cache_get(); // violate abstraction barrier
  smartlist_t *vals = smartlist_new();

  /* Put these: objects in the cache: one with a good sha3, one with bad sha3,
   * one with a wrong sha3, and one with no sha3. */
  config_line_prepend(&lines, "id", "wrong sha3");
  config_line_prepend(&lines, "sha3-digest",
                      "F00DF00DF00DF00DF00DF00DF00DF00D"
                      "F00DF00DF00DF00DF00DF00DF00DF00D");
  ent = consensus_cache_add(cache, lines, (const uint8_t *)"Hi there", 8);
  consensus_cache_entry_decref(ent);
  config_free_lines(lines);
  lines = NULL;

  config_line_prepend(&lines, "id", "bad sha3");
  config_line_prepend(&lines, "sha3-digest",
                      "now is the winter of our dicotheque");
  ent = consensus_cache_add(cache, lines, (const uint8_t *)"Hi there", 8);
  consensus_cache_entry_decref(ent);
  config_free_lines(lines);
  lines = NULL;

  config_line_prepend(&lines, "id", "no sha3");
  ent = consensus_cache_add(cache, lines, (const uint8_t *)"Hi there", 8);
  consensus_cache_entry_decref(ent);
  config_free_lines(lines);
  lines = NULL;

  config_line_prepend(&lines, "id", "good sha3");
  config_line_prepend(&lines, "sha3-digest",
                      "8d8b1998616cd6b4c4055da8d38728dc"
                      "93c758d4131a53c7d81aa6337dee1c05");
  ent = consensus_cache_add(cache, lines, (const uint8_t *)"Hi there", 8);
  consensus_cache_entry_decref(ent);
  config_free_lines(lines);
  lines = NULL;

  cdm_reload();
  cache = cdm_cache_get();
  tt_int_op(1, OP_EQ, consdiffmgr_validate());

  consensus_cache_find_all(vals, cache, "id", "good sha3");
  tt_int_op(smartlist_len(vals), OP_EQ, 1);
  smartlist_clear(vals);

  consensus_cache_find_all(vals, cache, "id", "no sha3");
  tt_int_op(smartlist_len(vals), OP_EQ, 1);
  smartlist_clear(vals);

  consensus_cache_find_all(vals, cache, "id", "wrong sha3");
  tt_int_op(smartlist_len(vals), OP_EQ, 0);
  consensus_cache_find_all(vals, cache, "id", "bad sha3");
  tt_int_op(smartlist_len(vals), OP_EQ, 0);

 done:
  smartlist_free(vals);
}

#define TEST(name)                                      \
  { #name, test_consdiffmgr_ ## name , TT_FORK, &setup_diffmgr, NULL }

struct testcase_t consdiffmgr_tests[] = {
#if 0
  { "init_failure", test_consdiffmgr_init_failure, TT_FORK, NULL, NULL },
#endif
  TEST(sha3_helper),
  TEST(add),
  TEST(make_diffs),
  TEST(diff_rules),
  TEST(diff_failure),
  TEST(diff_pending),
  TEST(cleanup_old),
  TEST(cleanup_bad_valid_after),
  TEST(cleanup_no_valid_after),
  TEST(cleanup_old_diffs),
  TEST(validate),

  // XXXX Test: non-cacheing cases of replyfn().

  END_OF_TESTCASES
};
