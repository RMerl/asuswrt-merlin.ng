/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file smartlist_core.c
 * \brief Implements the core functionality of a smartlist (a resizeable
 * dynamic array).  For more functionality and helper functions, see the
 * container library.
 **/

#include "lib/err/torerr.h"
#include "lib/malloc/malloc.h"
#include "lib/smartlist_core/smartlist_core.h"

#include <stdlib.h>
#include <string.h>

/** All newly allocated smartlists have this capacity. */
#define SMARTLIST_DEFAULT_CAPACITY 16

/** Allocate and return an empty smartlist.
 */
MOCK_IMPL(smartlist_t *,
smartlist_new,(void))
{
  smartlist_t *sl = tor_malloc(sizeof(smartlist_t));
  sl->num_used = 0;
  sl->capacity = SMARTLIST_DEFAULT_CAPACITY;
  sl->list = tor_calloc(sizeof(void *), sl->capacity);
  return sl;
}

/** Deallocate a smartlist.  Does not release storage associated with the
 * list's elements.
 */
MOCK_IMPL(void,
smartlist_free_,(smartlist_t *sl))
{
  if (!sl)
    return;
  tor_free(sl->list);
  tor_free(sl);
}

/** Remove all elements from the list.
 */
void
smartlist_clear(smartlist_t *sl)
{
  memset(sl->list, 0, sizeof(void *) * sl->num_used);
  sl->num_used = 0;
}

#if SIZE_MAX < INT_MAX
#error "We don't support systems where size_t is smaller than int."
#endif

/** Make sure that <b>sl</b> can hold at least <b>size</b> entries. */
static inline void
smartlist_ensure_capacity(smartlist_t *sl, size_t size)
{
  /* Set MAX_CAPACITY to MIN(INT_MAX, SIZE_MAX / sizeof(void*)) */
#if (SIZE_MAX/SIZEOF_VOID_P) > INT_MAX
#define MAX_CAPACITY (INT_MAX)
#else
#define MAX_CAPACITY (int)((SIZE_MAX / (sizeof(void*))))
#endif

  raw_assert(size <= MAX_CAPACITY);

  if (size > (size_t) sl->capacity) {
    size_t higher = (size_t) sl->capacity;
    if (PREDICT_UNLIKELY(size > MAX_CAPACITY/2)) {
      higher = MAX_CAPACITY;
    } else {
      while (size > higher)
        higher *= 2;
    }
    sl->list = tor_reallocarray(sl->list, sizeof(void *),
                                ((size_t)higher));
    memset(sl->list + sl->capacity, 0,
           sizeof(void *) * (higher - sl->capacity));
    sl->capacity = (int) higher;
  }
#undef ASSERT_CAPACITY
#undef MAX_CAPACITY
}

/** Expand <b>sl</b> so that its length is at least <b>new_size</b>,
 * filling in previously unused entries with NULL>
 *
 * Do nothing if <b>sl</b> already had at least <b>new_size</b> elements.
 */
void
smartlist_grow(smartlist_t *sl, size_t new_size)
{
  smartlist_ensure_capacity(sl, new_size);

  if (new_size > (size_t)sl->num_used) {
    /* This memset() should be a no-op: everything else in the smartlist code
     * tries to make sure that unused entries are always NULL.  Still, that is
     * meant as a safety mechanism, so let's clear the memory here.
     */
    memset(sl->list + sl->num_used, 0,
           sizeof(void *) * (new_size - sl->num_used));

    /* This cast is safe, since we already asserted that we were below
     * MAX_CAPACITY in smartlist_ensure_capacity(). */
    sl->num_used = (int)new_size;
  }
}

/** Append element to the end of the list. */
void
smartlist_add(smartlist_t *sl, void *element)
{
  smartlist_ensure_capacity(sl, ((size_t) sl->num_used)+1);
  sl->list[sl->num_used++] = element;
}

/** Append each element from S2 to the end of S1. */
void
smartlist_add_all(smartlist_t *s1, const smartlist_t *s2)
{
  size_t new_size = (size_t)s1->num_used + (size_t)s2->num_used;
  raw_assert(new_size >= (size_t) s1->num_used); /* check for overflow. */
  smartlist_ensure_capacity(s1, new_size);
  memcpy(s1->list + s1->num_used, s2->list, s2->num_used*sizeof(void*));
  raw_assert(new_size <= INT_MAX); /* redundant. */
  s1->num_used = (int) new_size;
}

/** Append a copy of string to sl */
void
smartlist_add_strdup(struct smartlist_t *sl, const char *string)
{
  char *copy;

  copy = tor_strdup(string);

  smartlist_add(sl, copy);
}

/** Remove all elements E from sl such that E==element.  Preserve
 * the order of any elements before E, but elements after E can be
 * rearranged.
 */
void
smartlist_remove(smartlist_t *sl, const void *element)
{
  int i;
  if (element == NULL)
    return;
  for (i=0; i < sl->num_used; i++)
    if (sl->list[i] == element) {
      sl->list[i] = sl->list[--sl->num_used]; /* swap with the end */
      i--; /* so we process the new i'th element */
      sl->list[sl->num_used] = NULL;
    }
}

/** As <b>smartlist_remove</b>, but do not change the order of
 * any elements not removed */
void
smartlist_remove_keeporder(smartlist_t *sl, const void *element)
{
  int i, j, num_used_orig = sl->num_used;
  if (element == NULL)
    return;

  for (i=j=0; j < num_used_orig; ++j) {
    if (sl->list[j] == element) {
      --sl->num_used;
    } else {
      sl->list[i++] = sl->list[j];
    }
  }
  memset(sl->list + sl->num_used, 0,
         sizeof(void *) * (num_used_orig - sl->num_used));
}

/** If <b>sl</b> is nonempty, remove and return the final element.  Otherwise,
 * return NULL. */
void *
smartlist_pop_last(smartlist_t *sl)
{
  raw_assert(sl);
  if (sl->num_used) {
    void *tmp = sl->list[--sl->num_used];
    sl->list[sl->num_used] = NULL;
    return tmp;
  } else
    return NULL;
}

/** Return true iff some element E of sl has E==element.
 */
int
smartlist_contains(const smartlist_t *sl, const void *element)
{
  int i;
  for (i=0; i < sl->num_used; i++)
    if (sl->list[i] == element)
      return 1;
  return 0;
}

/** Remove the <b>idx</b>th element of sl; if idx is not the last
 * element, swap the last element of sl into the <b>idx</b>th space.
 */
void
smartlist_del(smartlist_t *sl, int idx)
{
  raw_assert(sl);
  raw_assert(idx>=0);
  raw_assert(idx < sl->num_used);
  sl->list[idx] = sl->list[--sl->num_used];
  sl->list[sl->num_used] = NULL;
}

/** Remove the <b>idx</b>th element of sl; if idx is not the last element,
 * moving all subsequent elements back one space. Return the old value
 * of the <b>idx</b>th element.
 */
void
smartlist_del_keeporder(smartlist_t *sl, int idx)
{
  raw_assert(sl);
  raw_assert(idx>=0);
  raw_assert(idx < sl->num_used);
  --sl->num_used;
  if (idx < sl->num_used)
    memmove(sl->list+idx, sl->list+idx+1, sizeof(void*)*(sl->num_used-idx));
  sl->list[sl->num_used] = NULL;
}

/** Insert the value <b>val</b> as the new <b>idx</b>th element of
 * <b>sl</b>, moving all items previously at <b>idx</b> or later
 * forward one space.
 */
void
smartlist_insert(smartlist_t *sl, int idx, void *val)
{
  raw_assert(sl);
  raw_assert(idx>=0);
  raw_assert(idx <= sl->num_used);
  if (idx == sl->num_used) {
    smartlist_add(sl, val);
  } else {
    smartlist_ensure_capacity(sl, ((size_t) sl->num_used)+1);
    /* Move other elements away */
    if (idx < sl->num_used)
      memmove(sl->list + idx + 1, sl->list + idx,
              sizeof(void*)*(sl->num_used-idx));
    sl->num_used++;
    sl->list[idx] = val;
  }
}
