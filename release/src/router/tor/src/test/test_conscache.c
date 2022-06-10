/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "app/config/config.h"
#include "feature/dircache/conscache.h"
#include "lib/encoding/confline.h"
#include "test/test.h"

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

static void
test_conscache_open_failure(void *arg)
{
  (void) arg;
  /* Try opening a directory that doesn't exist and which we shouldn't be
   * able to create. */
  consensus_cache_t *cache = consensus_cache_open("a/b/c/d/e/f/g", 128);
  tt_ptr_op(cache, OP_EQ, NULL);

 done:
  ;
}

static void
test_conscache_simple_usage(void *arg)
{
  (void)arg;
  consensus_cache_entry_t *ent = NULL, *ent2 = NULL;

  /* Make a temporary datadir for these tests */
  char *ddir_fname = tor_strdup(get_fname_rnd("datadir_cache"));
  tor_free(get_options_mutable()->CacheDirectory);
  get_options_mutable()->CacheDirectory = tor_strdup(ddir_fname);
  check_private_dir(ddir_fname, CPD_CREATE, NULL);
  consensus_cache_t *cache = consensus_cache_open("cons", 128);

  tt_assert(cache);

  /* Create object; make sure it exists. */
  config_line_t *labels = NULL;
  config_line_append(&labels, "Hello", "world");
  config_line_append(&labels, "Adios", "planetas");
  ent = consensus_cache_add(cache,
                            labels, (const uint8_t *)"A\0B\0C", 5);
  config_free_lines(labels);
  labels = NULL;
  tt_assert(ent);

  /* Make a second object */
  config_line_append(&labels, "Hello", "mundo");
  config_line_append(&labels, "Adios", "planets");
  ent2 = consensus_cache_add(cache,
                             labels, (const uint8_t *)"xyzzy", 5);
  config_free_lines(labels);
  labels = NULL;
  tt_assert(ent2);
  tt_assert(! consensus_cache_entry_is_mapped(ent2));
  consensus_cache_entry_decref(ent2);
  ent2 = NULL;

  /* Check get_value */
  tt_ptr_op(NULL, OP_EQ, consensus_cache_entry_get_value(ent, "hebbo"));
  tt_str_op("world", OP_EQ, consensus_cache_entry_get_value(ent, "Hello"));

  /* Check find_first */
  ent2 = consensus_cache_find_first(cache, "Hello", "world!");
  tt_ptr_op(ent2, OP_EQ, NULL);
  ent2 = consensus_cache_find_first(cache, "Hello", "world");
  tt_ptr_op(ent2, OP_EQ, ent);
  ent2 = consensus_cache_find_first(cache, "Hello", "mundo");
  tt_ptr_op(ent2, OP_NE, ent);

  tt_assert(! consensus_cache_entry_is_mapped(ent));

  /* Check get_body */
  const uint8_t *bp = NULL;
  size_t sz = 0;
  int r = consensus_cache_entry_get_body(ent, &bp, &sz);
  tt_int_op(r, OP_EQ, 0);
  tt_u64_op(sz, OP_EQ, 5);
  tt_mem_op(bp, OP_EQ, "A\0B\0C", 5);
  tt_assert(consensus_cache_entry_is_mapped(ent));

  /* Free and re-create the cache, to rescan the directory. */
  consensus_cache_free(cache);
  consensus_cache_entry_decref(ent);
  cache = consensus_cache_open("cons", 128);

  /* Make sure the entry is still there */
  ent = consensus_cache_find_first(cache, "Hello", "mundo");
  tt_assert(ent);
  ent2 = consensus_cache_find_first(cache, "Adios", "planets");
  tt_ptr_op(ent, OP_EQ, ent2);
  consensus_cache_entry_incref(ent);
  tt_assert(! consensus_cache_entry_is_mapped(ent));
  r = consensus_cache_entry_get_body(ent, &bp, &sz);
  tt_int_op(r, OP_EQ, 0);
  tt_u64_op(sz, OP_EQ, 5);
  tt_mem_op(bp, OP_EQ, "xyzzy", 5);
  tt_assert(consensus_cache_entry_is_mapped(ent));

  /* There should be two entries total. */
  smartlist_t *entries = smartlist_new();
  consensus_cache_find_all(entries, cache, NULL, NULL);
  int n = smartlist_len(entries);
  smartlist_free(entries);
  tt_int_op(n, OP_EQ, 2);

 done:
  consensus_cache_entry_decref(ent);
  tor_free(ddir_fname);
  consensus_cache_free(cache);
}

static void
test_conscache_cleanup(void *arg)
{
  (void)arg;
  const int N = 20;
  consensus_cache_entry_t **ents =
    tor_calloc(N, sizeof(consensus_cache_entry_t*));

  /* Make a temporary datadir for these tests */
  char *ddir_fname = tor_strdup(get_fname_rnd("datadir_cache"));
  tor_free(get_options_mutable()->CacheDirectory);
  get_options_mutable()->CacheDirectory = tor_strdup(ddir_fname);
  check_private_dir(ddir_fname, CPD_CREATE, NULL);
  consensus_cache_t *cache = consensus_cache_open("cons", 128);

  tt_assert(cache);

  /* Create a bunch of entries. */
  int i;
  for (i = 0; i < N; ++i) {
    config_line_t *labels = NULL;
    char num[8];
    tor_snprintf(num, sizeof(num), "%d", i);
    config_line_append(&labels, "test-id", "cleanup");
    config_line_append(&labels, "index", num);
    size_t bodylen = i * 3;
    uint8_t *body = tor_malloc(bodylen);
    memset(body, i, bodylen);
    ents[i] = consensus_cache_add(cache, labels, body, bodylen);
    tor_free(body);
    config_free_lines(labels);
    tt_assert(ents[i]);
    /* We're still holding a reference to each entry at this point. */
  }

  /* Page all of the entries into RAM */
  for (i = 0; i < N; ++i) {
    const uint8_t *bp;
    size_t sz;
    tt_assert(! consensus_cache_entry_is_mapped(ents[i]));
    consensus_cache_entry_get_body(ents[i], &bp, &sz);
    tt_assert(consensus_cache_entry_is_mapped(ents[i]));
  }

  /* Mark some of the entries as deletable. */
  for (i = 7; i < N; i += 7) {
    consensus_cache_entry_mark_for_removal(ents[i]);
    tt_assert(consensus_cache_entry_is_mapped(ents[i]));
  }

  /* Mark some of the entries as aggressively unpaged. */
  for (i = 3; i < N; i += 3) {
    consensus_cache_entry_mark_for_aggressive_release(ents[i]);
    tt_assert(consensus_cache_entry_is_mapped(ents[i]));
  }

  /* Incref some of the entries again */
  for (i = 0; i < N; i += 2) {
    consensus_cache_entry_incref(ents[i]);
  }

  /* Now we're going to decref everything. We do so at a specific time.  I'm
   * picking the moment when I was writing this test, at 2017-04-05 12:16:48
   * UTC. */
  const time_t example_time = 1491394608;
  update_approx_time(example_time);
  for (i = 0; i < N; ++i) {
    consensus_cache_entry_decref(ents[i]);
    if (i % 2) {
      ents[i] = NULL; /* We're no longer holding any reference here. */
    }
  }

  /* At this point, the aggressively-released items with refcount 1 should
   * be unmapped. Nothing should be deleted. */
  consensus_cache_entry_t *e_tmp;
  e_tmp = consensus_cache_find_first(cache, "index", "3");
  tt_assert(e_tmp);
  tt_assert(! consensus_cache_entry_is_mapped(e_tmp));
  e_tmp = consensus_cache_find_first(cache, "index", "5");
  tt_assert(e_tmp);
  tt_assert(consensus_cache_entry_is_mapped(e_tmp));
  e_tmp = consensus_cache_find_first(cache, "index", "6");
  tt_assert(e_tmp);
  tt_assert(consensus_cache_entry_is_mapped(e_tmp));
  e_tmp = consensus_cache_find_first(cache, "index", "7");
  tt_ptr_op(e_tmp, OP_EQ, NULL); // not found because pending deletion.

  /* Delete the pending-deletion items. */
  consensus_cache_delete_pending(cache, 0);
  {
    smartlist_t *entries = smartlist_new();
    consensus_cache_find_all(entries, cache, NULL, NULL);
    int n = smartlist_len(entries);
    smartlist_free(entries);
    tt_int_op(n, OP_EQ, 20 - 2); /* 1 entry was deleted; 1 is not-found. */
  }
  e_tmp = consensus_cache_find_first(cache, "index", "7"); // refcnt == 1...
  tt_ptr_op(e_tmp, OP_EQ, NULL); // so deleted.
  e_tmp = consensus_cache_find_first(cache, "index", "14"); // refcnt == 2
  tt_ptr_op(e_tmp, OP_EQ, NULL); // not deleted; but not found.

  /* Now do lazy unmapping. */
  // should do nothing.
  consensus_cache_unmap_lazy(cache, example_time - 10);
  e_tmp = consensus_cache_find_first(cache, "index", "11");
  tt_assert(e_tmp);
  tt_assert(consensus_cache_entry_is_mapped(e_tmp));
  // should actually unmap
  consensus_cache_unmap_lazy(cache, example_time + 10);
  e_tmp = consensus_cache_find_first(cache, "index", "11");
  tt_assert(e_tmp);
  tt_assert(! consensus_cache_entry_is_mapped(e_tmp));
  // This one will still be mapped, since it has a reference.
  e_tmp = consensus_cache_find_first(cache, "index", "16");
  tt_assert(e_tmp);
  tt_assert(consensus_cache_entry_is_mapped(e_tmp));

  for (i = 0; i < N; ++i) {
    consensus_cache_entry_decref(ents[i]);
    ents[i] = NULL;
  }

  /* Free and re-create the cache, to rescan the directory. Make sure the
   * deleted thing is still deleted, along with the other deleted thing. */
  consensus_cache_free(cache);
  cache = consensus_cache_open("cons", 128);
  {
    smartlist_t *entries = smartlist_new();
    consensus_cache_find_all(entries, cache, NULL, NULL);
    int n = smartlist_len(entries);
    smartlist_free(entries);
    tt_int_op(n, OP_EQ, 18);
  }

 done:
  for (i = 0; i < N; ++i) {
    consensus_cache_entry_decref(ents[i]);
  }
  tor_free(ents);
  tor_free(ddir_fname);
  consensus_cache_free(cache);
}

static void
test_conscache_filter(void *arg)
{
  (void)arg;
  const int N = 30;
  smartlist_t *lst = NULL;

  /* Make a temporary datadir for these tests */
  char *ddir_fname = tor_strdup(get_fname_rnd("datadir_cache"));
  tor_free(get_options_mutable()->CacheDirectory);
  get_options_mutable()->CacheDirectory = tor_strdup(ddir_fname);
  check_private_dir(ddir_fname, CPD_CREATE, NULL);
  consensus_cache_t *cache = consensus_cache_open("cons", 128);

  tt_assert(cache);

  /* Create a bunch of entries with different labels */
  int i;
  for (i = 0; i < N; ++i) {
    config_line_t *labels = NULL;
    char num[8];
    tor_snprintf(num, sizeof(num), "%d", i);
    config_line_append(&labels, "test-id", "filter");
    config_line_append(&labels, "index", num);
    tor_snprintf(num, sizeof(num), "%d", i % 3);
    config_line_append(&labels, "mod3", num);
    tor_snprintf(num, sizeof(num), "%d", i % 5);
    config_line_append(&labels, "mod5", num);

    size_t bodylen = i * 3;
    uint8_t *body = tor_malloc(bodylen);
    memset(body, i, bodylen);
    consensus_cache_entry_t *ent =
      consensus_cache_add(cache, labels, body, bodylen);
    tor_free(body);
    config_free_lines(labels);
    tt_assert(ent);
    consensus_cache_entry_decref(ent);
  }

  lst = smartlist_new();
  /* Find nothing. */
  consensus_cache_find_all(lst, cache, "mod5", "5");
  tt_int_op(smartlist_len(lst), OP_EQ, 0);
  /* Find everything. */
  consensus_cache_find_all(lst, cache, "test-id", "filter");
  tt_int_op(smartlist_len(lst), OP_EQ, N);

  /* Now filter to find the entries that have i%3 == 1 */
  consensus_cache_filter_list(lst, "mod3", "1");
  tt_int_op(smartlist_len(lst), OP_EQ, 10);
  /* Now filter to find the entries that also have i%5 == 3 */
  consensus_cache_filter_list(lst, "mod5", "3");
  tt_int_op(smartlist_len(lst), OP_EQ, 2);
  /* So now we have those entries for which i%15 == 13. */

  consensus_cache_entry_t *ent1 = smartlist_get(lst, 0);
  consensus_cache_entry_t *ent2 = smartlist_get(lst, 1);
  const char *idx1 = consensus_cache_entry_get_value(ent1, "index");
  const char *idx2 = consensus_cache_entry_get_value(ent2, "index");
  tt_assert( (!strcmp(idx1, "28") && !strcmp(idx2, "13")) ||
             (!strcmp(idx1, "13") && !strcmp(idx2, "28")) );

 done:
  tor_free(ddir_fname);
  consensus_cache_free(cache);
  smartlist_free(lst);
}

#define ENT(name)                                               \
  { #name, test_conscache_ ## name, TT_FORK, NULL, NULL }

struct testcase_t conscache_tests[] = {
  ENT(open_failure),
  ENT(simple_usage),
  ENT(cleanup),
  ENT(filter),
  END_OF_TESTCASES
};
