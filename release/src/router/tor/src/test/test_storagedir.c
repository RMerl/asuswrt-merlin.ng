/* Copyright (c) 2017-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "core/or/or.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/fs/storagedir.h"
#include "lib/encoding/confline.h"
#include "test/test.h"

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

static void
test_storagedir_empty(void *arg)
{
  char *dirname = tor_strdup(get_fname_rnd("store_dir"));
  storage_dir_t *d = NULL;
  (void)arg;

  tt_int_op(FN_NOENT, OP_EQ, file_status(dirname));

  d = storage_dir_new(dirname, 10);
  tt_assert(d);

  tt_int_op(FN_DIR, OP_EQ, file_status(dirname));

  tt_int_op(0, OP_EQ, smartlist_len(storage_dir_list(d)));
  tt_u64_op(0, OP_EQ, storage_dir_get_usage(d));

  storage_dir_free(d);
  d = storage_dir_new(dirname, 10);
  tt_assert(d);

  tt_int_op(FN_DIR, OP_EQ, file_status(dirname));

  tt_int_op(0, OP_EQ, smartlist_len(storage_dir_list(d)));
  tt_u64_op(0, OP_EQ, storage_dir_get_usage(d));

 done:
  storage_dir_free(d);
  tor_free(dirname);
}

static void
test_storagedir_basic(void *arg)
{
  char *dirname = tor_strdup(get_fname_rnd("store_dir"));
  storage_dir_t *d = NULL;
  uint8_t *junk = NULL, *bytes = NULL;
  const size_t junklen = 1024;
  char *fname1 = NULL, *fname2 = NULL;
  const char hello_str[] = "then what are we but cold, alone ... ?";
  tor_mmap_t *mapping = NULL;
  (void)arg;

  junk = tor_malloc(junklen);
  crypto_rand((void*)junk, junklen);

  d = storage_dir_new(dirname, 10);
  tt_assert(d);
  tt_u64_op(0, OP_EQ, storage_dir_get_usage(d));

  int r;
  r = storage_dir_save_string_to_file(d, hello_str, 1, &fname1);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(fname1, OP_NE, NULL);
  tt_u64_op(strlen(hello_str), OP_EQ, storage_dir_get_usage(d));

  r = storage_dir_save_bytes_to_file(d, junk, junklen, 1, &fname2);
  tt_int_op(r, OP_EQ, 0);
  tt_ptr_op(fname2, OP_NE, NULL);

  tt_str_op(fname1, OP_NE, fname2);

  tt_int_op(2, OP_EQ, smartlist_len(storage_dir_list(d)));
  tt_u64_op(junklen + strlen(hello_str), OP_EQ, storage_dir_get_usage(d));
  tt_assert(smartlist_contains_string(storage_dir_list(d), fname1));
  tt_assert(smartlist_contains_string(storage_dir_list(d), fname2));

  storage_dir_free(d);
  d = storage_dir_new(dirname, 10);
  tt_assert(d);
  tt_int_op(2, OP_EQ, smartlist_len(storage_dir_list(d)));
  tt_u64_op(junklen + strlen(hello_str), OP_EQ, storage_dir_get_usage(d));
  tt_assert(smartlist_contains_string(storage_dir_list(d), fname1));
  tt_assert(smartlist_contains_string(storage_dir_list(d), fname2));

  size_t n;
  bytes = storage_dir_read(d, fname2, 1, &n);
  tt_assert(bytes);
  tt_u64_op(n, OP_EQ, junklen);
  tt_mem_op(bytes, OP_EQ, junk, junklen);

  mapping = storage_dir_map(d, fname1);
  tt_assert(mapping);
  tt_u64_op(mapping->size, OP_EQ, strlen(hello_str));
  tt_mem_op(mapping->data, OP_EQ, hello_str, strlen(hello_str));

 done:
  tor_free(dirname);
  tor_free(junk);
  tor_free(bytes);
  tor_munmap_file(mapping);
  storage_dir_free(d);
  tor_free(fname1);
  tor_free(fname2);
}

static void
test_storagedir_deletion(void *arg)
{
  (void)arg;
  char *dirname = tor_strdup(get_fname_rnd("store_dir"));
  storage_dir_t *d = NULL;
  char *fn1 = NULL, *fn2 = NULL;
  char *bytes = NULL;
  int r;
  const char str1[] = "There are nine and sixty ways to disguise communiques";
  const char str2[] = "And rather more than one of them is right";

  // Make sure the directory is there. */
  d = storage_dir_new(dirname, 10);
  storage_dir_free(d);
  d = NULL;

  tor_asprintf(&fn1, "%s/1007", dirname);
  r = write_str_to_file(fn1, str1, 0);
  tt_int_op(r, OP_EQ, 0);

  tor_asprintf(&fn2, "%s/1003.tmp", dirname);
  r = write_str_to_file(fn2, str2, 0);
  tt_int_op(r, OP_EQ, 0);

  // The tempfile should be deleted the next time we list the directory.
  d = storage_dir_new(dirname, 10);
  tt_int_op(1, OP_EQ, smartlist_len(storage_dir_list(d)));
  tt_u64_op(strlen(str1), OP_EQ, storage_dir_get_usage(d));
  tt_int_op(FN_FILE, OP_EQ, file_status(fn1));
  tt_int_op(FN_NOENT, OP_EQ, file_status(fn2));

  bytes = (char*) storage_dir_read(d, "1007", 1, NULL);
  tt_str_op(bytes, OP_EQ, str1);

  // Should have no effect; file already gone.
  storage_dir_remove_file(d, "1003.tmp");
  tt_int_op(1, OP_EQ, smartlist_len(storage_dir_list(d)));
  tt_u64_op(strlen(str1), OP_EQ, storage_dir_get_usage(d));

  // Actually remove a file.
  storage_dir_remove_file(d, "1007");
  tt_int_op(FN_NOENT, OP_EQ, file_status(fn1));
  tt_int_op(0, OP_EQ, smartlist_len(storage_dir_list(d)));
  tt_u64_op(0, OP_EQ, storage_dir_get_usage(d));

 done:
  tor_free(dirname);
  tor_free(fn1);
  tor_free(fn2);
  storage_dir_free(d);
  tor_free(bytes);
}

static void
test_storagedir_full(void *arg)
{
  (void)arg;

  char *dirname = tor_strdup(get_fname_rnd("store_dir"));
  storage_dir_t *d = NULL;
  const char str[] = "enemies of the peephole";
  int r;

  d = storage_dir_new(dirname, 3);
  tt_assert(d);

  r = storage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, 0);
  r = storage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, 0);
  r = storage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, 0);

  // These should fail!
  r = storage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, -1);
  r = storage_dir_save_string_to_file(d, str, 1, NULL);
  tt_int_op(r, OP_EQ, -1);

  tt_u64_op(strlen(str) * 3, OP_EQ, storage_dir_get_usage(d));

 done:
  tor_free(dirname);
  storage_dir_free(d);
}

static void
test_storagedir_cleaning(void *arg)
{
  (void)arg;

  char *dirname = tor_strdup(get_fname_rnd("store_dir"));
  storage_dir_t *d = NULL;
  const char str[] =
    "On a mountain halfway between Reno and Rome / "
    "We have a machine in a plexiglass dome / "
    "Which listens and looks into everyone's home."
    " -- Dr. Seuss";
  char *fns[8];
  int r, i;

  memset(fns, 0, sizeof(fns));
  d = storage_dir_new(dirname, 10);
  tt_assert(d);

  for (i = 0; i < 8; ++i) {
    r = storage_dir_save_string_to_file(d, str+i*2, 1, &fns[i]);
    tt_int_op(r, OP_EQ, 0);
  }

  /* Now we're going to make sure all the files have distinct mtimes. */
  time_t now = time(NULL);
  struct utimbuf ub;
  ub.actime = now;
  ub.modtime = now - 1000;
  for (i = 0; i < 8; ++i) {
    char *f = NULL;
    tor_asprintf(&f, "%s/%s", dirname, fns[i]);
    r = utime(f, &ub);
    tor_free(f);
    tt_int_op(r, OP_EQ, 0);
    ub.modtime += 5;
  }

  const uint64_t usage_orig = storage_dir_get_usage(d);
  /* No changes needed if we are already under target. */
  storage_dir_shrink(d, 1024*1024, 0);
  tt_u64_op(usage_orig, OP_EQ, storage_dir_get_usage(d));

  /* Get rid of at least one byte.  This will delete fns[0]. */
  storage_dir_shrink(d, usage_orig - 1, 0);
  tt_u64_op(usage_orig, OP_GT, storage_dir_get_usage(d));
  tt_u64_op(usage_orig - strlen(str), OP_EQ, storage_dir_get_usage(d));

  /* Get rid of at least two files.  This will delete fns[1] and fns[2]. */
  storage_dir_shrink(d, 1024*1024, 2);
  tt_u64_op(usage_orig - strlen(str)*3 + 6, OP_EQ, storage_dir_get_usage(d));

  /* Get rid of everything. */
  storage_dir_remove_all(d);
  tt_u64_op(0, OP_EQ, storage_dir_get_usage(d));

 done:
  tor_free(dirname);
  storage_dir_free(d);
  for (i = 0; i < 8; ++i) {
    tor_free(fns[i]);
  }
}

static void
test_storagedir_save_labeled(void *arg)
{
  (void)arg;
  char *dirname = tor_strdup(get_fname_rnd("store_dir"));
  storage_dir_t *d = NULL;
  uint8_t *inp = tor_malloc_zero(8192);
  config_line_t *labels = NULL;
  char *fname = NULL;
  uint8_t *saved = NULL;

  d = storage_dir_new(dirname, 10);
  tt_assert(d);

  crypto_rand((char *)inp, 8192);

  config_line_append(&labels, "Foo", "bar baz");
  config_line_append(&labels, "quux", "quuzXxz");
  const char expected[] =
    "Foo bar baz\n"
    "quux quuzXxz\n";

  int r = storage_dir_save_labeled_to_file(d, labels, inp, 8192, &fname);
  tt_int_op(r, OP_EQ, 0);

  size_t n = 0;
  saved = storage_dir_read(d, fname, 1, &n);
  tt_assert(memchr(saved, '\0', n));
  tt_str_op((char*)saved, OP_EQ, expected); /* NUL guarantees strcmp works */
  tt_mem_op(saved+strlen(expected)+1, OP_EQ, inp, 8192);

 done:
  storage_dir_free(d);
  tor_free(dirname);
  tor_free(inp);
  tor_free(fname);
  config_free_lines(labels);
  tor_free(saved);
}

static void
test_storagedir_read_labeled(void *arg)
{
  (void)arg;
  char *dirname = tor_strdup(get_fname_rnd("store_dir"));
  storage_dir_t *d = NULL;
  uint8_t *inp = tor_malloc_zero(8192);
  config_line_t *labels = NULL, *labels2 = NULL;
  char *fname = NULL;
  tor_mmap_t *map = NULL;
  uint8_t *as_read = NULL;

  d = storage_dir_new(dirname, 10);
  tt_assert(d);

  tor_snprintf((char*)inp, 8192,
               "Hello world\n"
               "This is a test\n"
               "Yadda yadda.\n");
  size_t bodylen = 8192 - strlen((char*)inp) - 1;
  crypto_rand((char *)inp+strlen((char*)inp)+1, bodylen);

  int r = storage_dir_save_bytes_to_file(d, inp, 8192, 1, &fname);
  tt_int_op(r, OP_EQ, 0);

  /* Try mapping */
  const uint8_t *datap = NULL;
  size_t sz = 0;
  map = storage_dir_map_labeled(d, fname, &labels, &datap, &sz);
  tt_assert(map);
  tt_assert(datap);
  tt_u64_op(sz, OP_EQ, bodylen);
  tt_mem_op(datap, OP_EQ, inp+strlen((char*)inp)+1, bodylen);
  tt_assert(labels);
  tt_str_op(labels->key, OP_EQ, "Hello");
  tt_str_op(labels->value, OP_EQ, "world");
  tt_assert(labels->next);
  tt_str_op(labels->next->key, OP_EQ, "This");
  tt_str_op(labels->next->value, OP_EQ, "is a test");
  tt_assert(labels->next->next);
  tt_str_op(labels->next->next->key, OP_EQ, "Yadda");
  tt_str_op(labels->next->next->value, OP_EQ, "yadda.");
  tt_ptr_op(labels->next->next->next, OP_EQ, NULL);

  /* Try reading this time. */
  sz = 0;
  as_read = storage_dir_read_labeled(d, fname, &labels2, &sz);
  tt_assert(as_read);
  tt_u64_op(sz, OP_EQ, bodylen);
  tt_mem_op(as_read, OP_EQ, inp+strlen((char*)inp)+1, bodylen);
  tt_assert(config_lines_eq(labels, labels2));

 done:
  storage_dir_free(d);
  tor_free(dirname);
  tor_free(inp);
  tor_free(fname);
  config_free_lines(labels);
  config_free_lines(labels2);
  tor_munmap_file(map);
  tor_free(as_read);
}

#define ENT(name)                                               \
  { #name, test_storagedir_ ## name, TT_FORK, NULL, NULL }

struct testcase_t storagedir_tests[] = {
  ENT(empty),
  ENT(basic),
  ENT(deletion),
  ENT(full),
  ENT(cleaning),
  ENT(save_labeled),
  ENT(read_labeled),
  END_OF_TESTCASES
};
