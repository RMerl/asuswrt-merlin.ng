/* Copyright (c) 2014, Daniel Martí
 * Copyright (c) 2014-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define CONSDIFF_PRIVATE

#include "core/or/or.h"
#include "test/test.h"

#include "feature/dircommon/consdiff.h"
#include "lib/memarea/memarea.h"
#include "test/log_test_helpers.h"

#define tt_str_eq_line(a,b) \
  tt_assert(line_str_eq((b),(a)))

static int
consensus_split_lines_(smartlist_t *out, const char *s, memarea_t *area)
{
  size_t len = strlen(s);
  return consensus_split_lines(out, s, len, area);
}

static int
consensus_compute_digest_(const char *cons,
                          consensus_digest_t *digest_out)
{
  size_t len = strlen(cons);
  char *tmp = tor_memdup(cons, len);
  // We use memdup here to ensure that the input is NOT nul-terminated.
  // This makes it likelier for us to spot bugs.
  int r = consensus_compute_digest(tmp, len, digest_out);
  tor_free(tmp);
  return r;
}

static int
consensus_compute_digest_as_signed_(const char *cons,
                                    consensus_digest_t *digest_out)
{
  size_t len = strlen(cons);
  char *tmp = tor_memdup(cons, len);
  // We use memdup here to ensure that the input is NOT nul-terminated.
  // This makes it likelier for us to spot bugs.
  int r = consensus_compute_digest_as_signed(tmp, len, digest_out);
  tor_free(tmp);
  return r;
}

static void
test_consdiff_smartlist_slice(void *arg)
{
  smartlist_t *sl = smartlist_new();
  smartlist_slice_t *sls;
  int items[6] = {0,0,0,0,0,0};

  /* Create a regular smartlist. */
  (void)arg;
  smartlist_add(sl, &items[1]);
  smartlist_add(sl, &items[2]);
  smartlist_add(sl, &items[3]);
  smartlist_add(sl, &items[4]);
  smartlist_add(sl, &items[5]);

  /* See if the slice was done correctly. */
  sls = smartlist_slice(sl, 2, 5);
  tt_ptr_op(sl, OP_EQ, sls->list);
  tt_ptr_op(&items[3], OP_EQ, smartlist_get(sls->list, sls->offset));
  tt_ptr_op(&items[5], OP_EQ,
      smartlist_get(sls->list, sls->offset + (sls->len-1)));
  tor_free(sls);

  /* See that using -1 as the end does get to the last element. */
  sls = smartlist_slice(sl, 2, -1);
  tt_ptr_op(sl, OP_EQ, sls->list);
  tt_ptr_op(&items[3], OP_EQ, smartlist_get(sls->list, sls->offset));
  tt_ptr_op(&items[5], OP_EQ,
      smartlist_get(sls->list, sls->offset + (sls->len-1)));

 done:
  tor_free(sls);
  smartlist_free(sl);
}

static void
test_consdiff_smartlist_slice_string_pos(void *arg)
{
  smartlist_t *sl = smartlist_new();
  smartlist_slice_t *sls;
  memarea_t *area = memarea_new();

  /* Create a regular smartlist. */
  (void)arg;
  consensus_split_lines_(sl, "a\nd\nc\na\nb\n", area);

  /* See that smartlist_slice_string_pos respects the bounds of the slice. */
  sls = smartlist_slice(sl, 2, 5);
  cdline_t a_line = { "a", 1 };
  tt_int_op(3, OP_EQ, smartlist_slice_string_pos(sls, &a_line));
  cdline_t d_line = { "d", 1 };
  tt_int_op(-1, OP_EQ, smartlist_slice_string_pos(sls, &d_line));

 done:
  tor_free(sls);
  smartlist_free(sl);
  memarea_drop_all(area);
}

static void
test_consdiff_lcs_lengths(void *arg)
{
  smartlist_t *sl1 = smartlist_new();
  smartlist_t *sl2 = smartlist_new();
  smartlist_slice_t *sls1, *sls2;
  int *lengths1, *lengths2;
  memarea_t *area = memarea_new();

  /* Expected lcs lengths in regular and reverse order. */
  int e_lengths1[] = { 0, 1, 2, 3, 3, 4 };
  int e_lengths2[] = { 0, 1, 1, 2, 3, 4 };

  (void)arg;
  consensus_split_lines_(sl1, "a\nb\nc\nd\ne\n", area);
  consensus_split_lines_(sl2, "a\nc\nd\ni\ne\n", area);

  sls1 = smartlist_slice(sl1, 0, -1);
  sls2 = smartlist_slice(sl2, 0, -1);

  lengths1 = lcs_lengths(sls1, sls2, 1);
  lengths2 = lcs_lengths(sls1, sls2, -1);
  tt_mem_op(e_lengths1, OP_EQ, lengths1, sizeof(int) * 6);
  tt_mem_op(e_lengths2, OP_EQ, lengths2, sizeof(int) * 6);

 done:
  tor_free(lengths1);
  tor_free(lengths2);
  tor_free(sls1);
  tor_free(sls2);
  smartlist_free(sl1);
  smartlist_free(sl2);
  memarea_drop_all(area);
}

static void
test_consdiff_trim_slices(void *arg)
{
  smartlist_t *sl1 = smartlist_new();
  smartlist_t *sl2 = smartlist_new();
  smartlist_t *sl3 = smartlist_new();
  smartlist_t *sl4 = smartlist_new();
  smartlist_slice_t *sls1, *sls2, *sls3, *sls4;
  memarea_t *area = memarea_new();

  (void)arg;
  consensus_split_lines_(sl1, "a\nb\nb\nb\nd\n", area);
  consensus_split_lines_(sl2, "a\nc\nc\nc\nd\n", area);
  consensus_split_lines_(sl3, "a\nb\nb\nb\na\n", area);
  consensus_split_lines_(sl4, "c\nb\nb\nb\nc\n", area);
  sls1 = smartlist_slice(sl1, 0, -1);
  sls2 = smartlist_slice(sl2, 0, -1);
  sls3 = smartlist_slice(sl3, 0, -1);
  sls4 = smartlist_slice(sl4, 0, -1);

  /* They should be trimmed by one line at each end. */
  tt_int_op(5, OP_EQ, sls1->len);
  tt_int_op(5, OP_EQ, sls2->len);
  trim_slices(sls1, sls2);
  tt_int_op(3, OP_EQ, sls1->len);
  tt_int_op(3, OP_EQ, sls2->len);

  /* They should not be trimmed at all. */
  tt_int_op(5, OP_EQ, sls3->len);
  tt_int_op(5, OP_EQ, sls4->len);
  trim_slices(sls3, sls4);
  tt_int_op(5, OP_EQ, sls3->len);
  tt_int_op(5, OP_EQ, sls4->len);

 done:
  tor_free(sls1);
  tor_free(sls2);
  tor_free(sls3);
  tor_free(sls4);
  smartlist_free(sl1);
  smartlist_free(sl2);
  smartlist_free(sl3);
  smartlist_free(sl4);
  memarea_drop_all(area);
}

static void
test_consdiff_set_changed(void *arg)
{
  smartlist_t *sl1 = smartlist_new();
  smartlist_t *sl2 = smartlist_new();
  bitarray_t *changed1 = bitarray_init_zero(4);
  bitarray_t *changed2 = bitarray_init_zero(4);
  smartlist_slice_t *sls1, *sls2;
  memarea_t *area = memarea_new();

  (void)arg;
  consensus_split_lines_(sl1, "a\nb\na\na\n", area);
  consensus_split_lines_(sl2, "a\na\na\na\n", area);

  /* Length of sls1 is 0. */
  sls1 = smartlist_slice(sl1, 0, 0);
  sls2 = smartlist_slice(sl2, 1, 3);
  set_changed(changed1, changed2, sls1, sls2);

  /* The former is not changed, the latter changes all of its elements. */
  tt_assert(!bitarray_is_set(changed1, 0));
  tt_assert(!bitarray_is_set(changed1, 1));
  tt_assert(!bitarray_is_set(changed1, 2));
  tt_assert(!bitarray_is_set(changed1, 3));

  tt_assert(!bitarray_is_set(changed2, 0));
  tt_assert(bitarray_is_set(changed2, 1));
  tt_assert(bitarray_is_set(changed2, 2));
  tt_assert(!bitarray_is_set(changed2, 3));
  bitarray_clear(changed2, 1);
  bitarray_clear(changed2, 2);

  /* Length of sls1 is 1 and its element is in sls2. */
  tor_free(sls1);
  sls1 = smartlist_slice(sl1, 0, 1);
  set_changed(changed1, changed2, sls1, sls2);

  /* The latter changes all elements but the (first) common one. */
  tt_assert(!bitarray_is_set(changed1, 0));
  tt_assert(!bitarray_is_set(changed1, 1));
  tt_assert(!bitarray_is_set(changed1, 2));
  tt_assert(!bitarray_is_set(changed1, 3));

  tt_assert(!bitarray_is_set(changed2, 0));
  tt_assert(!bitarray_is_set(changed2, 1));
  tt_assert(bitarray_is_set(changed2, 2));
  tt_assert(!bitarray_is_set(changed2, 3));
  bitarray_clear(changed2, 2);

  /* Length of sls1 is 1 and its element is not in sls2. */
  tor_free(sls1);
  sls1 = smartlist_slice(sl1, 1, 2);
  set_changed(changed1, changed2, sls1, sls2);

  /* The former changes its element, the latter changes all elements. */
  tt_assert(!bitarray_is_set(changed1, 0));
  tt_assert(bitarray_is_set(changed1, 1));
  tt_assert(!bitarray_is_set(changed1, 2));
  tt_assert(!bitarray_is_set(changed1, 3));

  tt_assert(!bitarray_is_set(changed2, 0));
  tt_assert(bitarray_is_set(changed2, 1));
  tt_assert(bitarray_is_set(changed2, 2));
  tt_assert(!bitarray_is_set(changed2, 3));

 done:
  bitarray_free(changed1);
  bitarray_free(changed2);
  smartlist_free(sl1);
  smartlist_free(sl2);
  tor_free(sls1);
  tor_free(sls2);
  memarea_drop_all(area);
}

static void
test_consdiff_calc_changes(void *arg)
{
  smartlist_t *sl1 = smartlist_new();
  smartlist_t *sl2 = smartlist_new();
  smartlist_slice_t *sls1, *sls2;
  bitarray_t *changed1 = bitarray_init_zero(4);
  bitarray_t *changed2 = bitarray_init_zero(4);
  memarea_t *area = memarea_new();

  (void)arg;
  consensus_split_lines_(sl1, "a\na\na\na\n", area);
  consensus_split_lines_(sl2, "a\na\na\na\n", area);

  sls1 = smartlist_slice(sl1, 0, -1);
  sls2 = smartlist_slice(sl2, 0, -1);
  calc_changes(sls1, sls2, changed1, changed2);

  /* Nothing should be set to changed. */
  tt_assert(!bitarray_is_set(changed1, 0));
  tt_assert(!bitarray_is_set(changed1, 1));
  tt_assert(!bitarray_is_set(changed1, 2));
  tt_assert(!bitarray_is_set(changed1, 3));

  tt_assert(!bitarray_is_set(changed2, 0));
  tt_assert(!bitarray_is_set(changed2, 1));
  tt_assert(!bitarray_is_set(changed2, 2));
  tt_assert(!bitarray_is_set(changed2, 3));

  smartlist_clear(sl2);
  consensus_split_lines_(sl2, "a\nb\na\nb\n", area);
  tor_free(sls1);
  tor_free(sls2);
  sls1 = smartlist_slice(sl1, 0, -1);
  sls2 = smartlist_slice(sl2, 0, -1);
  calc_changes(sls1, sls2, changed1, changed2);

  /* Two elements are changed. */
  tt_assert(!bitarray_is_set(changed1, 0));
  tt_assert(bitarray_is_set(changed1, 1));
  tt_assert(bitarray_is_set(changed1, 2));
  tt_assert(!bitarray_is_set(changed1, 3));
  bitarray_clear(changed1, 1);
  bitarray_clear(changed1, 2);

  tt_assert(!bitarray_is_set(changed2, 0));
  tt_assert(bitarray_is_set(changed2, 1));
  tt_assert(!bitarray_is_set(changed2, 2));
  tt_assert(bitarray_is_set(changed2, 3));
  bitarray_clear(changed1, 1);
  bitarray_clear(changed1, 3);

  smartlist_clear(sl2);
  consensus_split_lines_(sl2, "b\nb\nb\nb\n", area);
  tor_free(sls1);
  tor_free(sls2);
  sls1 = smartlist_slice(sl1, 0, -1);
  sls2 = smartlist_slice(sl2, 0, -1);
  calc_changes(sls1, sls2, changed1, changed2);

  /* All elements are changed. */
  tt_assert(bitarray_is_set(changed1, 0));
  tt_assert(bitarray_is_set(changed1, 1));
  tt_assert(bitarray_is_set(changed1, 2));
  tt_assert(bitarray_is_set(changed1, 3));

  tt_assert(bitarray_is_set(changed2, 0));
  tt_assert(bitarray_is_set(changed2, 1));
  tt_assert(bitarray_is_set(changed2, 2));
  tt_assert(bitarray_is_set(changed2, 3));

 done:
  bitarray_free(changed1);
  bitarray_free(changed2);
  smartlist_free(sl1);
  smartlist_free(sl2);
  tor_free(sls1);
  tor_free(sls2);
  memarea_drop_all(area);
}

static void
test_consdiff_get_id_hash(void *arg)
{
  (void)arg;

  cdline_t line1 = { "r name", 6 };
  cdline_t line2 = { "r name _hash_isnt_base64 etc", 28 };
  cdline_t line3 = { "r name hash+valid+base64 etc", 28 };
  cdline_t tmp;

  /* No hash. */
  tt_int_op(-1, OP_EQ, get_id_hash(&line1, &tmp));
  /* The hash contains characters that are not base64. */
  tt_int_op(-1, OP_EQ, get_id_hash(&line2, &tmp));

  /* valid hash. */
  tt_int_op(0, OP_EQ, get_id_hash(&line3, &tmp));
  tt_ptr_op(tmp.s, OP_EQ, line3.s + 7);
  tt_uint_op(tmp.len, OP_EQ, line3.len - 11);

 done:
  ;
}

static void
test_consdiff_is_valid_router_entry(void *arg)
{
  /* Doesn't start with "r ". */
  (void)arg;
  cdline_t line0 = { "foo", 3 };
  tt_int_op(0, OP_EQ, is_valid_router_entry(&line0));

  /* These are already tested with get_id_hash, but make sure it's run
   * properly. */

  cdline_t line1 = { "r name", 6 };
  cdline_t line2 = { "r name _hash_isnt_base64 etc", 28 };
  cdline_t line3 = { "r name hash+valid+base64 etc", 28 };
  tt_int_op(0, OP_EQ, is_valid_router_entry(&line1));
  tt_int_op(0, OP_EQ, is_valid_router_entry(&line2));
  tt_int_op(1, OP_EQ, is_valid_router_entry(&line3));

 done:
  ;
}

static void
test_consdiff_next_router(void *arg)
{
  smartlist_t *sl = smartlist_new();
  memarea_t *area = memarea_new();
  (void)arg;
  smartlist_add_linecpy(sl, area, "foo");
  smartlist_add_linecpy(sl, area,
      "r name hash+longer+than+27+chars+and+valid+base64 etc");
  smartlist_add_linecpy(sl, area, "foo");
  smartlist_add_linecpy(sl, area, "foo");
  smartlist_add_linecpy(sl, area,
      "r name hash+longer+than+27+chars+and+valid+base64 etc");
  smartlist_add_linecpy(sl, area, "foo");

  /* Not currently on a router entry line, finding the next one. */
  tt_int_op(1, OP_EQ, next_router(sl, 0));
  tt_int_op(4, OP_EQ, next_router(sl, 2));

  /* Already at the beginning of a router entry line, ignore it. */
  tt_int_op(4, OP_EQ, next_router(sl, 1));

  /* There are no more router entries, so return the line after the last. */
  tt_int_op(6, OP_EQ, next_router(sl, 4));
  tt_int_op(6, OP_EQ, next_router(sl, 5));

 done:
  smartlist_free(sl);
  memarea_drop_all(area);
}

static int
base64cmp_wrapper(const char *a, const char *b)
{
  cdline_t aa = { a, a ? (uint32_t) strlen(a) : 0 };
  cdline_t bb = { b, b ? (uint32_t) strlen(b) : 0 };
  return base64cmp(&aa, &bb);
}

static void
test_consdiff_base64cmp(void *arg)
{
  /* NULL arguments. */
  (void)arg;
  tt_int_op(0, OP_EQ, base64cmp_wrapper(NULL, NULL));
  tt_int_op(-1, OP_EQ, base64cmp_wrapper(NULL, "foo"));
  tt_int_op(1, OP_EQ, base64cmp_wrapper("bar", NULL));

  /* Nil base64 values. */
  tt_int_op(0, OP_EQ, base64cmp_wrapper("", ""));
  tt_int_op(0, OP_EQ, base64cmp_wrapper("_", "&"));

  /* Exact same valid strings. */
  tt_int_op(0, OP_EQ, base64cmp_wrapper("abcABC/+", "abcABC/+"));
  /* Both end with an invalid base64 char other than '\0'. */
  tt_int_op(0, OP_EQ, base64cmp_wrapper("abcABC/+ ", "abcABC/+ "));
  /* Only one ends with an invalid base64 char other than '\0'. */
  tt_int_op(-1, OP_EQ, base64cmp_wrapper("abcABC/+ ", "abcABC/+a"));

  /* Comparisons that would return differently with strcmp(). */
  tt_int_op(strcmp("/foo", "Afoo"), OP_LT, 0);
  tt_int_op(base64cmp_wrapper("/foo", "Afoo"), OP_GT, 0);
  tt_int_op(strcmp("Afoo", "0foo"), OP_GT, 0);
  tt_int_op(base64cmp_wrapper("Afoo", "0foo"), OP_LT, 0);

  /* Comparisons that would return the same as with strcmp(). */
  tt_int_op(strcmp("afoo", "Afoo"), OP_GT, 0);
  tt_int_op(base64cmp_wrapper("afoo", "Afoo"), OP_GT, 0);

  /* Different lengths */
  tt_int_op(base64cmp_wrapper("afoo", "afooo"), OP_LT, 0);
  tt_int_op(base64cmp_wrapper("afooo", "afoo"), OP_GT, 0);

 done:
  ;
}

static void
test_consdiff_gen_ed_diff(void *arg)
{
  smartlist_t *cons1=NULL, *cons2=NULL, *diff=NULL;
  int i;
  memarea_t *area = memarea_new();
  setup_capture_of_logs(LOG_WARN);

  (void)arg;
  cons1 = smartlist_new();
  cons2 = smartlist_new();

  /* Identity hashes are not sorted properly, return NULL. */
  smartlist_add_linecpy(cons1, area, "r name bbbbbbbbbbbbbbbbbbbbbbbbbbb etc");
  smartlist_add_linecpy(cons1, area, "foo");
  smartlist_add_linecpy(cons1, area, "r name aaaaaaaaaaaaaaaaaaaaaaaaaaa etc");
  smartlist_add_linecpy(cons1, area, "bar");

  smartlist_add_linecpy(cons2, area, "r name aaaaaaaaaaaaaaaaaaaaaaaaaaa etc");
  smartlist_add_linecpy(cons2, area, "foo");
  smartlist_add_linecpy(cons2, area, "r name ccccccccccccccccccccccccccc etc");
  smartlist_add_linecpy(cons2, area, "bar");

  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_EQ, diff);
  expect_single_log_msg_containing("Refusing to generate consensus diff "
         "because the base consensus doesn't have its router entries sorted "
         "properly.");

  /* Same, but now with the second consensus. */
  mock_clean_saved_logs();
  diff = gen_ed_diff(cons2, cons1, area);
  tt_ptr_op(NULL, OP_EQ, diff);
  expect_single_log_msg_containing("Refusing to generate consensus diff "
         "because the target consensus doesn't have its router entries sorted "
         "properly.");

  /* Same as the two above, but with the reversed thing immediately after a
     match. (The code handles this differently) */
  smartlist_del(cons1, 0);
  smartlist_add_linecpy(cons1, area, "r name aaaaaaaaaaaaaaaaaaaaaaaaaaa etc");

  mock_clean_saved_logs();
  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_EQ, diff);
  expect_single_log_msg_containing("Refusing to generate consensus diff "
         "because the base consensus doesn't have its router entries sorted "
         "properly.");

  mock_clean_saved_logs();
  diff = gen_ed_diff(cons2, cons1, area);
  tt_ptr_op(NULL, OP_EQ, diff);
  expect_single_log_msg_containing("Refusing to generate consensus diff "
         "because the target consensus doesn't have its router entries sorted "
         "properly.");

  /* Identity hashes are repeated, return NULL. */
  smartlist_clear(cons1);

  smartlist_add_linecpy(cons1, area, "r name bbbbbbbbbbbbbbbbbbbbbbbbbbb etc");
  smartlist_add_linecpy(cons1, area, "foo");
  smartlist_add_linecpy(cons1, area, "r name bbbbbbbbbbbbbbbbbbbbbbbbbbb etc");
  smartlist_add_linecpy(cons1, area, "bar");

  mock_clean_saved_logs();
  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_EQ, diff);
  expect_single_log_msg_containing("Refusing to generate consensus diff "
         "because the base consensus doesn't have its router entries sorted "
         "properly.");

  /* We have to add a line that is just a dot, return NULL. */
  smartlist_clear(cons1);
  smartlist_clear(cons2);

  smartlist_add_linecpy(cons1, area, "foo1");
  smartlist_add_linecpy(cons1, area, "foo2");

  smartlist_add_linecpy(cons2, area, "foo1");
  smartlist_add_linecpy(cons2, area, ".");
  smartlist_add_linecpy(cons2, area, "foo2");

  mock_clean_saved_logs();
  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_EQ, diff);
  expect_single_log_msg_containing("Cannot generate consensus diff "
         "because one of the lines to be added is \".\".");

#define MAX_LINE_COUNT (10000)
  /* Too many lines to be fed to the quadratic-time function. */
  smartlist_clear(cons1);
  smartlist_clear(cons2);

  for (i=0; i < MAX_LINE_COUNT; ++i) smartlist_add_linecpy(cons1, area, "a");
  for (i=0; i < MAX_LINE_COUNT; ++i) smartlist_add_linecpy(cons1, area, "b");

  mock_clean_saved_logs();
  diff = gen_ed_diff(cons1, cons2, area);

  tt_ptr_op(NULL, OP_EQ, diff);
  expect_single_log_msg_containing("Refusing to generate consensus diff "
         "because we found too few common router ids.");

  /* We have dot lines, but they don't interfere with the script format. */
  smartlist_clear(cons1);
  smartlist_clear(cons2);

  smartlist_add_linecpy(cons1, area, "foo1");
  smartlist_add_linecpy(cons1, area, ".");
  smartlist_add_linecpy(cons1, area, ".");
  smartlist_add_linecpy(cons1, area, "foo2");

  smartlist_add_linecpy(cons2, area, "foo1");
  smartlist_add_linecpy(cons2, area, ".");
  smartlist_add_linecpy(cons2, area, "foo2");

  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  smartlist_free(diff);

  /* Empty diff tests. */
  smartlist_clear(cons1);
  smartlist_clear(cons2);

  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(0, OP_EQ, smartlist_len(diff));
  smartlist_free(diff);

  smartlist_add_linecpy(cons1, area, "foo");
  smartlist_add_linecpy(cons1, area, "bar");

  smartlist_add_linecpy(cons2, area, "foo");
  smartlist_add_linecpy(cons2, area, "bar");

  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(0, OP_EQ, smartlist_len(diff));
  smartlist_free(diff);

  /* Everything is deleted. */
  smartlist_clear(cons2);

  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(1, OP_EQ, smartlist_len(diff));
  tt_str_eq_line("1,2d", smartlist_get(diff, 0));

  smartlist_free(diff);

  /* Everything is added. */
  diff = gen_ed_diff(cons2, cons1, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(4, OP_EQ, smartlist_len(diff));
  tt_str_eq_line("0a", smartlist_get(diff, 0));
  tt_str_eq_line("foo", smartlist_get(diff, 1));
  tt_str_eq_line("bar", smartlist_get(diff, 2));
  tt_str_eq_line(".", smartlist_get(diff, 3));

  smartlist_free(diff);

  /* Everything is changed. */
  smartlist_add_linecpy(cons2, area, "foo2");
  smartlist_add_linecpy(cons2, area, "bar2");
  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(4, OP_EQ, smartlist_len(diff));
  tt_str_eq_line("1,2c", smartlist_get(diff, 0));
  tt_str_eq_line("foo2", smartlist_get(diff, 1));
  tt_str_eq_line("bar2", smartlist_get(diff, 2));
  tt_str_eq_line(".", smartlist_get(diff, 3));

  smartlist_free(diff);

  /* Test 'a', 'c' and 'd' together. See that it is done in reverse order. */
  smartlist_clear(cons1);
  smartlist_clear(cons2);
  consensus_split_lines_(cons1, "A\nB\nC\nD\nE\n", area);
  consensus_split_lines_(cons2, "A\nC\nO\nE\nU\n", area);
  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(7, OP_EQ, smartlist_len(diff));
  tt_str_eq_line("5a", smartlist_get(diff, 0));
  tt_str_eq_line("U", smartlist_get(diff, 1));
  tt_str_eq_line(".", smartlist_get(diff, 2));
  tt_str_eq_line("4c", smartlist_get(diff, 3));
  tt_str_eq_line("O", smartlist_get(diff, 4));
  tt_str_eq_line(".", smartlist_get(diff, 5));
  tt_str_eq_line("2d", smartlist_get(diff, 6));

  smartlist_free(diff);

  smartlist_clear(cons1);
  smartlist_clear(cons2);
  consensus_split_lines_(cons1, "B\n", area);
  consensus_split_lines_(cons2, "A\nB\n", area);
  diff = gen_ed_diff(cons1, cons2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(3, OP_EQ, smartlist_len(diff));
  tt_str_eq_line("0a", smartlist_get(diff, 0));
  tt_str_eq_line("A", smartlist_get(diff, 1));
  tt_str_eq_line(".", smartlist_get(diff, 2));

  /* TODO: small real use-cases, i.e. consensuses. */

 done:
  teardown_capture_of_logs();
  smartlist_free(cons1);
  smartlist_free(cons2);
  smartlist_free(diff);
  memarea_drop_all(area);
}

static void
test_consdiff_apply_ed_diff(void *arg)
{
  smartlist_t *cons1=NULL, *cons2=NULL, *diff=NULL;
  memarea_t *area = memarea_new();
  (void)arg;
  cons1 = smartlist_new();
  diff = smartlist_new();
  setup_capture_of_logs(LOG_WARN);

  consensus_split_lines_(cons1, "A\nB\nC\nD\nE\n", area);

  /* Command without range. */
  smartlist_add_linecpy(diff, area, "a");
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  smartlist_clear(diff);
  expect_single_log_msg_containing("an ed command was missing a line number");

  /* Range without command. */
  smartlist_add_linecpy(diff, area, "1");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("a line with no ed command was found");

  smartlist_clear(diff);

  /* Range without end. */
  smartlist_add_linecpy(diff, area, "1,");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an ed command was missing a range "
                                   "end line number.");

  smartlist_clear(diff);

  /* Incoherent ranges. */
  smartlist_add_linecpy(diff, area, "1,1");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an invalid range was found");

  smartlist_clear(diff);

  smartlist_add_linecpy(diff, area, "3,2");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an invalid range was found");

  smartlist_clear(diff);

  /* Unexpected range for add command. */
  smartlist_add_linecpy(diff, area, "1,2a");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("add lines after a range");

  smartlist_clear(diff);

  /* $ for a non-delete command. */
  smartlist_add_linecpy(diff, area, "1,$c");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("it wanted to use $ with a command "
                                   "other than delete");

  smartlist_clear(diff);

  /* Script is not in reverse order. */
  smartlist_add_linecpy(diff, area, "1d");
  smartlist_add_linecpy(diff, area, "3d");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("its commands are not properly sorted");

  smartlist_clear(diff);

  /* Script contains unrecognised commands longer than one char. */
  smartlist_add_linecpy(diff, area, "1foo");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an ed command longer than one char was "
                                   "found");

  smartlist_clear(diff);

  /* Script contains unrecognised commands. */
  smartlist_add_linecpy(diff, area, "1e");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an unrecognised ed command was found");

  smartlist_clear(diff);

  /* Command that should be followed by at least one line and a ".", but
   * isn't. */
  smartlist_add_linecpy(diff, area, "0a");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("it has an ed command that tries to "
                                   "insert zero lines.");

  /* Now it is followed by a ".", but it inserts zero lines. */
  smartlist_add_linecpy(diff, area, ".");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("it has an ed command that tries to "
                                   "insert zero lines.");

  smartlist_clear(diff);

  /* Now it it inserts something, but has no terminator. */
  smartlist_add_linecpy(diff, area, "0a");
  smartlist_add_linecpy(diff, area, "hello");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("lines to be inserted that don't end with "
                                   "a \".\".");

  smartlist_clear(diff);

  /* Ranges must be numeric only and cannot contain spaces. */
  smartlist_add_linecpy(diff, area, "0, 4d");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an ed command was missing a range "
                                   "end line number.");

  smartlist_clear(diff);

  /* '+' is not a number. */
  smartlist_add_linecpy(diff, area, "+0,4d");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an ed command was missing a line number");

  smartlist_clear(diff);

  /* range duplication */
  smartlist_add_linecpy(diff, area, "0,4d,5d");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an ed command longer than one char was "
                                   "found");

  smartlist_clear(diff);

  /* space before command */
  smartlist_add_linecpy(diff, area, "0,4 d");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an ed command longer than one char was "
                                   "found");

  smartlist_clear(diff);

  /* space inside number */
  smartlist_add_linecpy(diff, area, "0,4 5d");
  mock_clean_saved_logs();
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("an ed command longer than one char was "
                                   "found");

  smartlist_clear(diff);

  /* Test appending text, 'a'. */
  consensus_split_lines_(diff, "3a\nU\nO\n.\n0a\nV\n.\n", area);
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_NE, cons2);
  tt_int_op(8, OP_EQ, smartlist_len(cons2));
  tt_str_eq_line("V", smartlist_get(cons2, 0));
  tt_str_eq_line("A", smartlist_get(cons2, 1));
  tt_str_eq_line("B", smartlist_get(cons2, 2));
  tt_str_eq_line("C", smartlist_get(cons2, 3));
  tt_str_eq_line("U", smartlist_get(cons2, 4));
  tt_str_eq_line("O", smartlist_get(cons2, 5));
  tt_str_eq_line("D", smartlist_get(cons2, 6));
  tt_str_eq_line("E", smartlist_get(cons2, 7));

  smartlist_clear(diff);
  smartlist_free(cons2);

  /* Test deleting text, 'd'. */
  consensus_split_lines_(diff, "4d\n1,2d\n", area);
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_NE, cons2);
  tt_int_op(2, OP_EQ, smartlist_len(cons2));
  tt_str_eq_line("C", smartlist_get(cons2, 0));
  tt_str_eq_line("E", smartlist_get(cons2, 1));

  smartlist_clear(diff);
  smartlist_free(cons2);

  /* Test changing text, 'c'. */
  consensus_split_lines_(diff, "4c\nT\nX\n.\n1,2c\nM\n.\n", area);
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_NE, cons2);
  tt_int_op(5, OP_EQ, smartlist_len(cons2));
  tt_str_eq_line("M", smartlist_get(cons2, 0));
  tt_str_eq_line("C", smartlist_get(cons2, 1));
  tt_str_eq_line("T", smartlist_get(cons2, 2));
  tt_str_eq_line("X", smartlist_get(cons2, 3));
  tt_str_eq_line("E", smartlist_get(cons2, 4));

  smartlist_clear(diff);
  smartlist_free(cons2);

  /* Test 'a', 'd' and 'c' together. */
  consensus_split_lines_(diff, "4c\nT\nX\n.\n2d\n0a\nM\n.\n", area);
  cons2 = apply_ed_diff(cons1, diff, 0);
  tt_ptr_op(NULL, OP_NE, cons2);
  tt_int_op(6, OP_EQ, smartlist_len(cons2));
  tt_str_eq_line("M", smartlist_get(cons2, 0));
  tt_str_eq_line("A", smartlist_get(cons2, 1));
  tt_str_eq_line("C", smartlist_get(cons2, 2));
  tt_str_eq_line("T", smartlist_get(cons2, 3));
  tt_str_eq_line("X", smartlist_get(cons2, 4));
  tt_str_eq_line("E", smartlist_get(cons2, 5));

 done:
  teardown_capture_of_logs();
  smartlist_free(cons1);
  smartlist_free(cons2);
  smartlist_free(diff);
  memarea_drop_all(area);
}

static void
test_consdiff_gen_diff(void *arg)
{
  char *cons1_str=NULL, *cons2_str=NULL;
  smartlist_t *cons1=NULL, *cons2=NULL, *diff=NULL;
  consensus_digest_t digests1, digests2;
  memarea_t *area = memarea_new();
  (void)arg;
  cons1 = smartlist_new();
  cons2 = smartlist_new();

  /* Identity hashes are not sorted properly, return NULL.
   * Already tested in gen_ed_diff, but see that a NULL ed diff also makes
   * gen_diff return NULL. */
  cons1_str = tor_strdup(
      "network-status-version foo\n"
      "r name bbbbbbbbbbbbbbbbb etc\nfoo\n"
      "r name aaaaaaaaaaaaaaaaa etc\nbar\n"
      "directory-signature foo bar\nbar\n"
      );
  cons2_str = tor_strdup(
      "network-status-version foo\n"
      "r name aaaaaaaaaaaaaaaaa etc\nfoo\n"
      "r name ccccccccccccccccc etc\nbar\n"
      "directory-signature foo bar\nbar\n"
      );

  tt_int_op(0, OP_EQ,
      consensus_compute_digest_as_signed_(cons1_str, &digests1));
  tt_int_op(0, OP_EQ,
      consensus_compute_digest_(cons2_str, &digests2));

  consensus_split_lines_(cons1, cons1_str, area);
  consensus_split_lines_(cons2, cons2_str, area);

  diff = consdiff_gen_diff(cons1, cons2, &digests1, &digests2, area);
  tt_ptr_op(NULL, OP_EQ, diff);

  /* Check that the headers are done properly. */
  tor_free(cons1_str);
  cons1_str = tor_strdup(
      "network-status-version foo\n"
      "r name ccccccccccccccccc etc\nfoo\n"
      "r name eeeeeeeeeeeeeeeee etc\nbar\n"
      "directory-signature foo bar\nbar\n"
      );
  tt_int_op(0, OP_EQ,
      consensus_compute_digest_as_signed_(cons1_str, &digests1));
  smartlist_clear(cons1);
  consensus_split_lines_(cons1, cons1_str, area);
  diff = consdiff_gen_diff(cons1, cons2, &digests1, &digests2, area);
  tt_ptr_op(NULL, OP_NE, diff);
  tt_int_op(11, OP_EQ, smartlist_len(diff));
  tt_assert(line_str_eq(smartlist_get(diff, 0),
                        "network-status-diff-version 1"));
  tt_assert(line_str_eq(smartlist_get(diff, 1), "hash "
      "95D70F5A3CC65F920AA8B44C4563D7781A082674329661884E19E94B79D539C2 "
      "7AFECEFA4599BA33D603653E3D2368F648DF4AC4723929B0F7CF39281596B0C1"));
  tt_assert(line_str_eq(smartlist_get(diff, 2), "6,$d"));
  tt_assert(line_str_eq(smartlist_get(diff, 3), "3,4c"));
  tt_assert(line_str_eq(smartlist_get(diff, 4), "bar"));
  tt_assert(line_str_eq(smartlist_get(diff, 5),
                        "directory-signature foo bar"));
  tt_assert(line_str_eq(smartlist_get(diff, 6),
                        "."));
  tt_assert(line_str_eq(smartlist_get(diff, 7), "1a"));
  tt_assert(line_str_eq(smartlist_get(diff, 8),
                        "r name aaaaaaaaaaaaaaaaa etc"));
  tt_assert(line_str_eq(smartlist_get(diff, 9), "foo"));
  tt_assert(line_str_eq(smartlist_get(diff, 10), "."));

  /* TODO: small real use-cases, i.e. consensuses. */

 done:
  tor_free(cons1_str);
  tor_free(cons2_str);
  smartlist_free(cons1);
  smartlist_free(cons2);
  smartlist_free(diff);
  memarea_drop_all(area);
}

static void
test_consdiff_apply_diff(void *arg)
{
  smartlist_t *cons1=NULL, *diff=NULL;
  char *cons1_str=NULL, *cons2 = NULL;
  consensus_digest_t digests1;
  (void)arg;
  memarea_t *area = memarea_new();
  cons1 = smartlist_new();
  diff = smartlist_new();
  setup_capture_of_logs(LOG_INFO);

  cons1_str = tor_strdup(
      "network-status-version foo\n"
      "r name ccccccccccccccccc etc\nfoo\n"
      "r name eeeeeeeeeeeeeeeee etc\nbar\n"
      "directory-signature foo bar\nbar\n"
      );
  tt_int_op(0, OP_EQ,
      consensus_compute_digest_(cons1_str, &digests1));
  consensus_split_lines_(cons1, cons1_str, area);

  /* diff doesn't have enough lines. */
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("too short");

  /* first line doesn't match format-version string. */
  smartlist_add_linecpy(diff, area, "foo-bar");
  smartlist_add_linecpy(diff, area, "header-line");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("format is not known");

  /* The first word of the second header line is not "hash". */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "word a b");
  smartlist_add_linecpy(diff, area, "x");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("does not include the necessary digests");

  /* Wrong number of words after "hash". */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash a b c");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("does not include the necessary digests");

  /* base16 digests do not have the expected length. */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash aaa bbb");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("includes base16-encoded digests of "
                                   "incorrect size");

  /* base16 digests contain non-base16 characters. */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash"
      " ????????????????????????????????????????????????????????????????"
      " ----------------------------------------------------------------");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("includes malformed digests");

  /* Invalid ed diff.
   * As tested in apply_ed_diff, but check that apply_diff does return NULL if
   * the ed diff can't be applied. */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash"
      /* sha3 of cons1. */
      " 06646D6CF563A41869D3B02E73254372AE3140046C5E7D83C9F71E54976AF9B4"
      /* sha256 of cons2. */
      " 635D34593020C08E5ECD865F9986E29D50028EFA62843766A8197AD228A7F6AA");
  smartlist_add_linecpy(diff, area, "foobar");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_single_log_msg_containing("because an ed command was missing a line "
                                   "number");

  /* Base consensus doesn't match its digest as found in the diff. */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash"
      /* bogus sha256. */
      " 3333333333333333333333333333333333333333333333333333333333333333"
      /* sha256 of cons2. */
      " 635D34593020C08E5ECD865F9986E29D50028EFA62843766A8197AD228A7F6AA");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_log_msg_containing("base consensus doesn't match the digest "
                            "as found");

  /* Resulting consensus doesn't match its digest as found in the diff. */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash"
      /* sha3 of cons1. */
      " 06646D6CF563A41869D3B02E73254372AE3140046C5E7D83C9F71E54976AF9B4"
      /* bogus sha3. */
      " 3333333333333333333333333333333333333333333333333333333333333333");
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_log_msg_containing("resulting consensus doesn't match the "
                            "digest as found");

#if 0
  /* XXXX No longer possible, since we aren't using the other algorithm. */
  /* Resulting consensus digest cannot be computed */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash"
      /* sha3 of cons1. */
      " 06646D6CF563A41869D3B02E73254372AE3140046C5E7D83C9F71E54976AF9B4"
      /* bogus sha3. */
      " 3333333333333333333333333333333333333333333333333333333333333333");
  smartlist_add_linecpy(diff, area, "1,2d"); // remove starting line
  mock_clean_saved_logs();
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_EQ, cons2);
  expect_log_msg_containing("Could not compute digests of the consensus "
                            "resulting from applying a consensus diff.");
#endif /* 0 */

  /* Very simple test, only to see that nothing errors. */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash"
      /* sha3 of cons1. */
      " 06646D6CF563A41869D3B02E73254372AE3140046C5E7D83C9F71E54976AF9B4"
      /* sha3 of cons2. */
      " 90A418881B2FCAB3D9E60EE02E4D666D56CFA38F8A3B7AA3E0ADBA530DDA9353");
  smartlist_add_linecpy(diff, area, "3c");
  smartlist_add_linecpy(diff, area, "sample");
  smartlist_add_linecpy(diff, area, ".");
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_NE, cons2);
  tt_str_op(
      "network-status-version foo\n"
      "r name ccccccccccccccccc etc\nsample\n"
      "r name eeeeeeeeeeeeeeeee etc\nbar\n"
      "directory-signature foo bar\nbar\n", OP_EQ,
      cons2);
  tor_free(cons2);

  /* Check that lowercase letters in base16-encoded digests work too. */
  smartlist_clear(diff);
  smartlist_add_linecpy(diff, area, "network-status-diff-version 1");
  smartlist_add_linecpy(diff, area, "hash"
      /* sha3 of cons1. */
      " 06646d6cf563a41869d3b02e73254372ae3140046c5e7d83c9f71e54976af9b4"
      /* sha3 of cons2. */
      " 90a418881b2fcab3d9e60ee02e4d666d56cfa38f8a3b7aa3e0adba530dda9353");
  smartlist_add_linecpy(diff, area, "3c");
  smartlist_add_linecpy(diff, area, "sample");
  smartlist_add_linecpy(diff, area, ".");
  cons2 = consdiff_apply_diff(cons1, diff, &digests1);
  tt_ptr_op(NULL, OP_NE, cons2);
  tt_str_op(
      "network-status-version foo\n"
      "r name ccccccccccccccccc etc\nsample\n"
      "r name eeeeeeeeeeeeeeeee etc\nbar\n"
      "directory-signature foo bar\nbar\n", OP_EQ,
      cons2);
  tor_free(cons2);

  smartlist_clear(diff);

 done:
  teardown_capture_of_logs();
  tor_free(cons1_str);
  smartlist_free(cons1);
  smartlist_free(diff);
  memarea_drop_all(area);
}

#define CONSDIFF_LEGACY(name)                                          \
  { #name, test_consdiff_ ## name , 0, NULL, NULL }

struct testcase_t consdiff_tests[] = {
  CONSDIFF_LEGACY(smartlist_slice),
  CONSDIFF_LEGACY(smartlist_slice_string_pos),
  CONSDIFF_LEGACY(lcs_lengths),
  CONSDIFF_LEGACY(trim_slices),
  CONSDIFF_LEGACY(set_changed),
  CONSDIFF_LEGACY(calc_changes),
  CONSDIFF_LEGACY(get_id_hash),
  CONSDIFF_LEGACY(is_valid_router_entry),
  CONSDIFF_LEGACY(next_router),
  CONSDIFF_LEGACY(base64cmp),
  CONSDIFF_LEGACY(gen_ed_diff),
  CONSDIFF_LEGACY(apply_ed_diff),
  CONSDIFF_LEGACY(gen_diff),
  CONSDIFF_LEGACY(apply_diff),
  END_OF_TESTCASES
};
