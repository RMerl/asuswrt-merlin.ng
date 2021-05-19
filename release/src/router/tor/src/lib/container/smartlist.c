/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file smartlist.c
 *
 * \brief Higher-level functions for the "smartlist" resizeable array
 * abstraction.
 *
 * The functions declared here use higher-level functionality than those in
 * smartlist_core.c, and handle things like smartlists of different types,
 * sorting, searching, heap-structured smartlists, and other convenience
 * functions.
 **/

#include "lib/container/smartlist.h"
#include "lib/err/torerr.h"
#include "lib/malloc/malloc.h"
#include "lib/defs/digest_sizes.h"
#include "lib/ctime/di_ops.h"
#include "lib/string/compat_ctype.h"
#include "lib/string/compat_string.h"
#include "lib/string/util_string.h"
#include "lib/string/printf.h"

#include "lib/log/util_bug.h"

#include <stdlib.h>
#include <string.h>

/** Append the string produced by tor_asprintf(<b>pattern</b>, <b>...</b>)
 * to <b>sl</b>. */
void
smartlist_add_asprintf(struct smartlist_t *sl, const char *pattern, ...)
{
  va_list ap;
  va_start(ap, pattern);
  smartlist_add_vasprintf(sl, pattern, ap);
  va_end(ap);
}

/** va_list-based backend of smartlist_add_asprintf. */
void
smartlist_add_vasprintf(struct smartlist_t *sl, const char *pattern,
                        va_list args)
{
  char *str = NULL;

  tor_vasprintf(&str, pattern, args);
  tor_assert(str != NULL);

  smartlist_add(sl, str);
}

/** Reverse the order of the items in <b>sl</b>. */
void
smartlist_reverse(smartlist_t *sl)
{
  int i, j;
  void *tmp;
  tor_assert(sl);
  for (i = 0, j = sl->num_used-1; i < j; ++i, --j) {
    tmp = sl->list[i];
    sl->list[i] = sl->list[j];
    sl->list[j] = tmp;
  }
}

/** If there are any strings in sl equal to element, remove and free them.
 * Does not preserve order. */
void
smartlist_string_remove(smartlist_t *sl, const char *element)
{
  int i;
  tor_assert(sl);
  tor_assert(element);
  for (i = 0; i < sl->num_used; ++i) {
    if (!strcmp(element, sl->list[i])) {
      tor_free(sl->list[i]);
      sl->list[i] = sl->list[--sl->num_used]; /* swap with the end */
      i--; /* so we process the new i'th element */
      sl->list[sl->num_used] = NULL;
    }
  }
}

/** Return true iff <b>sl</b> has some element E such that
 * !strcmp(E,<b>element</b>)
 */
int
smartlist_contains_string(const smartlist_t *sl, const char *element)
{
  int i;
  if (!sl) return 0;
  for (i=0; i < sl->num_used; i++)
    if (strcmp((const char*)sl->list[i],element)==0)
      return 1;
  return 0;
}

/** If <b>element</b> is equal to an element of <b>sl</b>, return that
 * element's index.  Otherwise, return -1. */
int
smartlist_string_pos(const smartlist_t *sl, const char *element)
{
  int i;
  if (!sl) return -1;
  for (i=0; i < sl->num_used; i++)
    if (strcmp((const char*)sl->list[i],element)==0)
      return i;
  return -1;
}

/** If <b>element</b> is the same pointer as an element of <b>sl</b>, return
 * that element's index.  Otherwise, return -1. */
int
smartlist_pos(const smartlist_t *sl, const void *element)
{
  int i;
  if (!sl) return -1;
  for (i=0; i < sl->num_used; i++)
    if (element == sl->list[i])
      return i;
  return -1;
}

/** Return true iff <b>sl</b> has some element E such that
 * !strcasecmp(E,<b>element</b>)
 */
int
smartlist_contains_string_case(const smartlist_t *sl, const char *element)
{
  int i;
  if (!sl) return 0;
  for (i=0; i < sl->num_used; i++)
    if (strcasecmp((const char*)sl->list[i],element)==0)
      return 1;
  return 0;
}

/** Return true iff <b>sl</b> has some element E such that E is equal
 * to the decimal encoding of <b>num</b>.
 */
int
smartlist_contains_int_as_string(const smartlist_t *sl, int num)
{
  char buf[32]; /* long enough for 64-bit int, and then some. */
  tor_snprintf(buf,sizeof(buf),"%d", num);
  return smartlist_contains_string(sl, buf);
}

/** Return true iff the two lists contain the same strings in the same
 * order, or if they are both NULL. */
int
smartlist_strings_eq(const smartlist_t *sl1, const smartlist_t *sl2)
{
  if (sl1 == NULL)
    return sl2 == NULL;
  if (sl2 == NULL)
    return 0;
  if (smartlist_len(sl1) != smartlist_len(sl2))
    return 0;
  SMARTLIST_FOREACH(sl1, const char *, cp1, {
      const char *cp2 = smartlist_get(sl2, cp1_sl_idx);
      if (strcmp(cp1, cp2))
        return 0;
    });
  return 1;
}

/** Return true iff the two lists contain the same int pointer values in
 * the same order, or if they are both NULL. */
int
smartlist_ints_eq(const smartlist_t *sl1, const smartlist_t *sl2)
{
  if (sl1 == NULL)
    return sl2 == NULL;
  if (sl2 == NULL)
    return 0;
  if (smartlist_len(sl1) != smartlist_len(sl2))
    return 0;
  SMARTLIST_FOREACH(sl1, int *, cp1, {
      int *cp2 = smartlist_get(sl2, cp1_sl_idx);
      if (*cp1 != *cp2)
        return 0;
    });
  return 1;
}

/**
 * Return true if there is shallow equality between smartlists -
 * i.e. all indices correspond to exactly same object (pointer
 * values are matching). Otherwise, return false.
 */
int
smartlist_ptrs_eq(const smartlist_t *s1, const smartlist_t *s2)
{
  if (s1 == s2)
    return 1;

  // Note: pointers cannot both be NULL at this point, because
  // above check.
  if (s1 == NULL || s2 == NULL)
    return 0;

  if (smartlist_len(s1) != smartlist_len(s2))
    return 0;

  for (int i = 0; i < smartlist_len(s1); i++) {
    if (smartlist_get(s1, i) != smartlist_get(s2, i))
      return 0;
  }

  return 1;
}

/** Return true iff <b>sl</b> has some element E such that
 * tor_memeq(E,<b>element</b>,DIGEST_LEN)
 */
int
smartlist_contains_digest(const smartlist_t *sl, const char *element)
{
  int i;
  if (!sl) return 0;
  for (i=0; i < sl->num_used; i++)
    if (tor_memeq((const char*)sl->list[i],element,DIGEST_LEN))
      return 1;
  return 0;
}

/** Return true iff some element E of sl2 has smartlist_contains(sl1,E).
 */
int
smartlist_overlap(const smartlist_t *sl1, const smartlist_t *sl2)
{
  int i;
  for (i=0; i < sl2->num_used; i++)
    if (smartlist_contains(sl1, sl2->list[i]))
      return 1;
  return 0;
}

/** Remove every element E of sl1 such that !smartlist_contains(sl2,E).
 * Does not preserve the order of sl1.
 */
void
smartlist_intersect(smartlist_t *sl1, const smartlist_t *sl2)
{
  int i;
  for (i=0; i < sl1->num_used; i++)
    if (!smartlist_contains(sl2, sl1->list[i])) {
      sl1->list[i] = sl1->list[--sl1->num_used]; /* swap with the end */
      i--; /* so we process the new i'th element */
      sl1->list[sl1->num_used] = NULL;
    }
}

/** Remove every element E of sl1 such that smartlist_contains(sl2,E).
 * Does not preserve the order of sl1.
 */
void
smartlist_subtract(smartlist_t *sl1, const smartlist_t *sl2)
{
  int i;
  for (i=0; i < sl2->num_used; i++)
    smartlist_remove(sl1, sl2->list[i]);
}

/** Allocate and return a new string containing the concatenation of
 * the elements of <b>sl</b>, in order, separated by <b>join</b>.  If
 * <b>terminate</b> is true, also terminate the string with <b>join</b>.
 * If <b>len_out</b> is not NULL, set <b>len_out</b> to the length of
 * the returned string. Requires that every element of <b>sl</b> is
 * NUL-terminated string.
 */
char *
smartlist_join_strings(smartlist_t *sl, const char *join,
                       int terminate, size_t *len_out)
{
  return smartlist_join_strings2(sl,join,strlen(join),terminate,len_out);
}

/** As smartlist_join_strings, but instead of separating/terminated with a
 * NUL-terminated string <b>join</b>, uses the <b>join_len</b>-byte sequence
 * at <b>join</b>.  (Useful for generating a sequence of NUL-terminated
 * strings.)
 */
char *
smartlist_join_strings2(smartlist_t *sl, const char *join,
                        size_t join_len, int terminate, size_t *len_out)
{
  int i;
  size_t n = 0;
  char *r = NULL, *dst, *src;

  tor_assert(sl);
  tor_assert(join);

  if (terminate)
    n = join_len;

  for (i = 0; i < sl->num_used; ++i) {
    n += strlen(sl->list[i]);
    if (i+1 < sl->num_used) /* avoid double-counting the last one */
      n += join_len;
  }
  dst = r = tor_malloc(n+1);
  for (i = 0; i < sl->num_used; ) {
    for (src = sl->list[i]; *src; )
      *dst++ = *src++;
    if (++i < sl->num_used) {
      memcpy(dst, join, join_len);
      dst += join_len;
    }
  }
  if (terminate) {
    memcpy(dst, join, join_len);
    dst += join_len;
  }
  *dst = '\0';

  if (len_out)
    *len_out = dst-r;
  return r;
}

/** Sort the members of <b>sl</b> into an order defined by
 * the ordering function <b>compare</b>, which returns less then 0 if a
 * precedes b, greater than 0 if b precedes a, and 0 if a 'equals' b.
 */
void
smartlist_sort(smartlist_t *sl, int (*compare)(const void **a, const void **b))
{
  if (!sl->num_used)
    return;
  qsort(sl->list, sl->num_used, sizeof(void*),
        (int (*)(const void *,const void*))compare);
}

/** Given a smartlist <b>sl</b> sorted with the function <b>compare</b>,
 * return the most frequent member in the list.  Break ties in favor of
 * later elements.  If the list is empty, return NULL.  If count_out is
 * non-null, set it to the count of the most frequent member.
 */
void *
smartlist_get_most_frequent_(const smartlist_t *sl,
                             int (*compare)(const void **a, const void **b),
                             int *count_out)
{
  const void *most_frequent = NULL;
  int most_frequent_count = 0;

  const void *cur = NULL;
  int i, count=0;

  if (!sl->num_used) {
    if (count_out)
      *count_out = 0;
    return NULL;
  }
  for (i = 0; i < sl->num_used; ++i) {
    const void *item = sl->list[i];
    if (cur && 0 == compare(&cur, &item)) {
      ++count;
    } else {
      if (cur && count >= most_frequent_count) {
        most_frequent = cur;
        most_frequent_count = count;
      }
      cur = item;
      count = 1;
    }
  }
  if (cur && count >= most_frequent_count) {
    most_frequent = cur;
    most_frequent_count = count;
  }
  if (count_out)
    *count_out = most_frequent_count;
  return (void*)most_frequent;
}

/** Given a sorted smartlist <b>sl</b> and the comparison function used to
 * sort it, remove all duplicate members.  If free_fn is provided, calls
 * free_fn on each duplicate.  Otherwise, just removes them.  Preserves order.
 */
void
smartlist_uniq(smartlist_t *sl,
               int (*compare)(const void **a, const void **b),
               void (*free_fn)(void *a))
{
  int i;
  for (i=1; i < sl->num_used; ++i) {
    if (compare((const void **)&(sl->list[i-1]),
                (const void **)&(sl->list[i])) == 0) {
      if (free_fn)
        free_fn(sl->list[i]);
      smartlist_del_keeporder(sl, i--);
    }
  }
}

/** Assuming the members of <b>sl</b> are in order, return a pointer to the
 * member that matches <b>key</b>.  Ordering and matching are defined by a
 * <b>compare</b> function that returns 0 on a match; less than 0 if key is
 * less than member, and greater than 0 if key is greater then member.
 */
void *
smartlist_bsearch(const smartlist_t *sl, const void *key,
                  int (*compare)(const void *key, const void **member))
{
  int found, idx;
  idx = smartlist_bsearch_idx(sl, key, compare, &found);
  return found ? smartlist_get(sl, idx) : NULL;
}

/** Assuming the members of <b>sl</b> are in order, return the index of the
 * member that matches <b>key</b>.  If no member matches, return the index of
 * the first member greater than <b>key</b>, or smartlist_len(sl) if no member
 * is greater than <b>key</b>.  Set <b>found_out</b> to true on a match, to
 * false otherwise.  Ordering and matching are defined by a <b>compare</b>
 * function that returns 0 on a match; less than 0 if key is less than member,
 * and greater than 0 if key is greater then member.
 */
int
smartlist_bsearch_idx(const smartlist_t *sl, const void *key,
                      int (*compare)(const void *key, const void **member),
                      int *found_out)
{
  int hi, lo, cmp, mid, len, diff;

  tor_assert(sl);
  tor_assert(compare);
  tor_assert(found_out);

  len = smartlist_len(sl);

  /* Check for the trivial case of a zero-length list */
  if (len == 0) {
    *found_out = 0;
    /* We already know smartlist_len(sl) is 0 in this case */
    return 0;
  }

  /* Okay, we have a real search to do */
  tor_assert(len > 0);
  lo = 0;
  hi = len - 1;

  /*
   * These invariants are always true:
   *
   * For all i such that 0 <= i < lo, sl[i] < key
   * For all i such that hi < i <= len, sl[i] > key
   */

  while (lo <= hi) {
    diff = hi - lo;
    /*
     * We want mid = (lo + hi) / 2, but that could lead to overflow, so
     * instead diff = hi - lo (non-negative because of loop condition), and
     * then hi = lo + diff, mid = (lo + lo + diff) / 2 = lo + (diff / 2).
     */
    mid = lo + (diff / 2);
    cmp = compare(key, (const void**) &(sl->list[mid]));
    if (cmp == 0) {
      /* sl[mid] == key; we found it */
      *found_out = 1;
      return mid;
    } else if (cmp > 0) {
      /*
       * key > sl[mid] and an index i such that sl[i] == key must
       * have i > mid if it exists.
       */

      /*
       * Since lo <= mid <= hi, hi can only decrease on each iteration (by
       * being set to mid - 1) and hi is initially len - 1, mid < len should
       * always hold, and this is not symmetric with the left end of list
       * mid > 0 test below.  A key greater than the right end of the list
       * should eventually lead to lo == hi == mid == len - 1, and then
       * we set lo to len below and fall out to the same exit we hit for
       * a key in the middle of the list but not matching.  Thus, we just
       * assert for consistency here rather than handle a mid == len case.
       */
      tor_assert(mid < len);
      /* Move lo to the element immediately after sl[mid] */
      lo = mid + 1;
    } else {
      /* This should always be true in this case */
      tor_assert(cmp < 0);

      /*
       * key < sl[mid] and an index i such that sl[i] == key must
       * have i < mid if it exists.
       */

      if (mid > 0) {
        /* Normal case, move hi to the element immediately before sl[mid] */
        hi = mid - 1;
      } else {
        /* These should always be true in this case */
        tor_assert(mid == lo);
        tor_assert(mid == 0);
        /*
         * We were at the beginning of the list and concluded that every
         * element e compares e > key.
         */
        *found_out = 0;
        return 0;
      }
    }
  }

  /*
   * lo > hi; we have no element matching key but we have elements falling
   * on both sides of it.  The lo index points to the first element > key.
   */
  tor_assert(lo == hi + 1); /* All other cases should have been handled */
  tor_assert(lo >= 0);
  tor_assert(lo <= len);
  tor_assert(hi >= 0);
  tor_assert(hi <= len);

  if (lo < len) {
    cmp = compare(key, (const void **) &(sl->list[lo]));
    tor_assert(cmp < 0);
  } else {
    cmp = compare(key, (const void **) &(sl->list[len-1]));
    tor_assert(cmp > 0);
  }

  *found_out = 0;
  return lo;
}

/** Helper: compare two const char **s. */
static int
compare_string_ptrs_(const void **_a, const void **_b)
{
  return strcmp((const char*)*_a, (const char*)*_b);
}

/** Sort a smartlist <b>sl</b> containing strings into lexically ascending
 * order. */
void
smartlist_sort_strings(smartlist_t *sl)
{
  smartlist_sort(sl, compare_string_ptrs_);
}

/** Return the most frequent string in the sorted list <b>sl</b> */
const char *
smartlist_get_most_frequent_string(smartlist_t *sl)
{
  return smartlist_get_most_frequent(sl, compare_string_ptrs_);
}

/** Return the most frequent string in the sorted list <b>sl</b>.
 * If <b>count_out</b> is provided, set <b>count_out</b> to the
 * number of times that string appears.
 */
const char *
smartlist_get_most_frequent_string_(smartlist_t *sl, int *count_out)
{
  return smartlist_get_most_frequent_(sl, compare_string_ptrs_, count_out);
}

/** Remove duplicate strings from a sorted list, and free them with tor_free().
 */
void
smartlist_uniq_strings(smartlist_t *sl)
{
  smartlist_uniq(sl, compare_string_ptrs_, tor_free_);
}

/** Helper: compare two pointers. */
static int
compare_ptrs_(const void **_a, const void **_b)
{
  const void *a = *_a, *b = *_b;
  if (a<b)
    return -1;
  else if (a==b)
    return 0;
  else
    return 1;
}

/** Sort <b>sl</b> in ascending order of the pointers it contains. */
void
smartlist_sort_pointers(smartlist_t *sl)
{
  smartlist_sort(sl, compare_ptrs_);
}

/* Heap-based priority queue implementation for O(lg N) insert and remove.
 * Recall that the heap property is that, for every index I, h[I] <
 * H[LEFT_CHILD[I]] and h[I] < H[RIGHT_CHILD[I]].
 *
 * For us to remove items other than the topmost item, each item must store
 * its own index within the heap.  When calling the pqueue functions, tell
 * them about the offset of the field that stores the index within the item.
 *
 * Example:
 *
 *   typedef struct timer_t {
 *     struct timeval tv;
 *     int heap_index;
 *   } timer_t;
 *
 *   static int compare(const void *p1, const void *p2) {
 *     const timer_t *t1 = p1, *t2 = p2;
 *     if (t1->tv.tv_sec < t2->tv.tv_sec) {
 *        return -1;
 *     } else if (t1->tv.tv_sec > t2->tv.tv_sec) {
 *        return 1;
 *     } else {
 *        return t1->tv.tv_usec - t2->tv_usec;
 *     }
 *   }
 *
 *   void timer_heap_insert(smartlist_t *heap, timer_t *timer) {
 *      smartlist_pqueue_add(heap, compare, offsetof(timer_t, heap_index),
 *         timer);
 *   }
 *
 *   void timer_heap_pop(smartlist_t *heap) {
 *      return smartlist_pqueue_pop(heap, compare,
 *         offsetof(timer_t, heap_index));
 *   }
 */

/** @{ */
/** Functions to manipulate heap indices to find a node's parent and children.
 *
 * For a 1-indexed array, we would use LEFT_CHILD[x] = 2*x and RIGHT_CHILD[x]
 *   = 2*x + 1.  But this is C, so we have to adjust a little. */

/* MAX_PARENT_IDX is the largest IDX in the smartlist which might have
 * children whose indices fit inside an int.
 * LEFT_CHILD(MAX_PARENT_IDX) == INT_MAX-2;
 * RIGHT_CHILD(MAX_PARENT_IDX) == INT_MAX-1;
 * LEFT_CHILD(MAX_PARENT_IDX + 1) == INT_MAX // impossible, see max list size.
 */
#define MAX_PARENT_IDX ((INT_MAX - 2) / 2)
/* If this is true, then i is small enough to potentially have children
 * in the smartlist, and it is save to use LEFT_CHILD/RIGHT_CHILD on it. */
#define IDX_MAY_HAVE_CHILDREN(i) ((i) <= MAX_PARENT_IDX)
#define LEFT_CHILD(i)  ( 2*(i) + 1 )
#define RIGHT_CHILD(i) ( 2*(i) + 2 )
#define PARENT(i)      ( ((i)-1) / 2 )
/** @} */

/** @{ */
/** Helper macros for heaps: Given a local variable <b>idx_field_offset</b>
 * set to the offset of an integer index within the heap element structure,
 * IDX_OF_ITEM(p) gives you the index of p, and IDXP(p) gives you a pointer to
 * where p's index is stored.  Given additionally a local smartlist <b>sl</b>,
 * UPDATE_IDX(i) sets the index of the element at <b>i</b> to the correct
 * value (that is, to <b>i</b>).
 */
#define IDXP(p) ((int*)STRUCT_VAR_P(p, idx_field_offset))

#define UPDATE_IDX(i)  do {                            \
    void *updated = sl->list[i];                       \
    *IDXP(updated) = i;                                \
  } while (0)

#define IDX_OF_ITEM(p) (*IDXP(p))
/** @} */

/** Helper. <b>sl</b> may have at most one violation of the heap property:
 * the item at <b>idx</b> may be greater than one or both of its children.
 * Restore the heap property. */
static inline void
smartlist_heapify(smartlist_t *sl,
                  int (*compare)(const void *a, const void *b),
                  ptrdiff_t idx_field_offset,
                  int idx)
{
  while (1) {
    if (! IDX_MAY_HAVE_CHILDREN(idx)) {
      /* idx is so large that it cannot have any children, since doing so
       * would mean the smartlist was over-capacity. Therefore it cannot
       * violate the heap property by being greater than a child (since it
       * doesn't have any). */
      return;
    }

    int left_idx = LEFT_CHILD(idx);
    int best_idx;

    if (left_idx >= sl->num_used)
      return;
    if (compare(sl->list[idx],sl->list[left_idx]) < 0)
      best_idx = idx;
    else
      best_idx = left_idx;
    if (left_idx+1 < sl->num_used &&
        compare(sl->list[left_idx+1],sl->list[best_idx]) < 0)
      best_idx = left_idx + 1;

    if (best_idx == idx) {
      return;
    } else {
      void *tmp = sl->list[idx];
      sl->list[idx] = sl->list[best_idx];
      sl->list[best_idx] = tmp;
      UPDATE_IDX(idx);
      UPDATE_IDX(best_idx);

      idx = best_idx;
    }
  }
}

/** Insert <b>item</b> into the heap stored in <b>sl</b>, where order is
 * determined by <b>compare</b> and the offset of the item in the heap is
 * stored in an int-typed field at position <b>idx_field_offset</b> within
 * item.
 */
void
smartlist_pqueue_add(smartlist_t *sl,
                     int (*compare)(const void *a, const void *b),
                     ptrdiff_t idx_field_offset,
                     void *item)
{
  int idx;
  smartlist_add(sl,item);
  UPDATE_IDX(sl->num_used-1);

  for (idx = sl->num_used - 1; idx; ) {
    int parent = PARENT(idx);
    if (compare(sl->list[idx], sl->list[parent]) < 0) {
      void *tmp = sl->list[parent];
      sl->list[parent] = sl->list[idx];
      sl->list[idx] = tmp;
      UPDATE_IDX(parent);
      UPDATE_IDX(idx);
      idx = parent;
    } else {
      return;
    }
  }
}

/** Remove and return the top-priority item from the heap stored in <b>sl</b>,
 * where order is determined by <b>compare</b> and the item's position is
 * stored at position <b>idx_field_offset</b> within the item.  <b>sl</b> must
 * not be empty. */
void *
smartlist_pqueue_pop(smartlist_t *sl,
                     int (*compare)(const void *a, const void *b),
                     ptrdiff_t idx_field_offset)
{
  void *top;
  tor_assert(sl->num_used);

  top = sl->list[0];
  *IDXP(top)=-1;
  if (--sl->num_used) {
    sl->list[0] = sl->list[sl->num_used];
    sl->list[sl->num_used] = NULL;
    UPDATE_IDX(0);
    smartlist_heapify(sl, compare, idx_field_offset, 0);
  }
  sl->list[sl->num_used] = NULL;
  return top;
}

/** Remove the item <b>item</b> from the heap stored in <b>sl</b>,
 * where order is determined by <b>compare</b> and the item's position is
 * stored at position <b>idx_field_offset</b> within the item.  <b>sl</b> must
 * not be empty. */
void
smartlist_pqueue_remove(smartlist_t *sl,
                        int (*compare)(const void *a, const void *b),
                        ptrdiff_t idx_field_offset,
                        void *item)
{
  int idx = IDX_OF_ITEM(item);
  tor_assert(idx >= 0);
  tor_assert(sl->list[idx] == item);
  --sl->num_used;
  *IDXP(item) = -1;
  if (idx == sl->num_used) {
    sl->list[sl->num_used] = NULL;
    return;
  } else {
    sl->list[idx] = sl->list[sl->num_used];
    sl->list[sl->num_used] = NULL;
    UPDATE_IDX(idx);
    smartlist_heapify(sl, compare, idx_field_offset, idx);
  }
}

/** Assert that the heap property is correctly maintained by the heap stored
 * in <b>sl</b>, where order is determined by <b>compare</b>. */
void
smartlist_pqueue_assert_ok(smartlist_t *sl,
                           int (*compare)(const void *a, const void *b),
                           ptrdiff_t idx_field_offset)
{
  int i;
  for (i = sl->num_used - 1; i >= 0; --i) {
    if (i>0)
      tor_assert(compare(sl->list[PARENT(i)], sl->list[i]) <= 0);
    tor_assert(IDX_OF_ITEM(sl->list[i]) == i);
  }
}

/** Helper: compare two DIGEST_LEN digests. */
static int
compare_digests_(const void **_a, const void **_b)
{
  return tor_memcmp((const char*)*_a, (const char*)*_b, DIGEST_LEN);
}

/** Sort the list of DIGEST_LEN-byte digests into ascending order. */
void
smartlist_sort_digests(smartlist_t *sl)
{
  smartlist_sort(sl, compare_digests_);
}

/** Remove duplicate digests from a sorted list, and free them with tor_free().
 */
void
smartlist_uniq_digests(smartlist_t *sl)
{
  smartlist_uniq(sl, compare_digests_, tor_free_);
}

/** Helper: compare two DIGEST256_LEN digests. */
static int
compare_digests256_(const void **_a, const void **_b)
{
  return tor_memcmp((const char*)*_a, (const char*)*_b, DIGEST256_LEN);
}

/** Sort the list of DIGEST256_LEN-byte digests into ascending order. */
void
smartlist_sort_digests256(smartlist_t *sl)
{
  smartlist_sort(sl, compare_digests256_);
}

/** Return the most frequent member of the sorted list of DIGEST256_LEN
 * digests in <b>sl</b> */
const uint8_t *
smartlist_get_most_frequent_digest256(smartlist_t *sl)
{
  return smartlist_get_most_frequent(sl, compare_digests256_);
}

/** Remove duplicate 256-bit digests from a sorted list, and free them with
 * tor_free().
 */
void
smartlist_uniq_digests256(smartlist_t *sl)
{
  smartlist_uniq(sl, compare_digests256_, tor_free_);
}
