/* Copyright (c) 2003, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file malloc.c
 * \brief Wrappers for C malloc code, and replacements for items that
 *   may be missing.
 **/

#include "orconfig.h"

#include <stdlib.h>
#include <string.h>

#include "lib/testsupport/testsupport.h"
#define UTIL_MALLOC_PRIVATE
#include "lib/malloc/malloc.h"
#include "lib/cc/torint.h"
#include "lib/err/torerr.h"

#ifdef __clang_analyzer__
#undef MALLOC_ZERO_WORKS
#endif

/** Allocate a chunk of <b>size</b> bytes of memory, and return a pointer to
 * result.  On error, log and terminate the process.  (Same as malloc(size),
 * but never returns NULL.)
 */
void *
tor_malloc_(size_t size)
{
  void *result;

  raw_assert(size < SIZE_T_CEILING);

#ifndef MALLOC_ZERO_WORKS
  /* Some libc mallocs don't work when size==0. Override them. */
  if (size==0) {
    size=1;
  }
#endif /* !defined(MALLOC_ZERO_WORKS) */

  result = raw_malloc(size);

  if (PREDICT_UNLIKELY(result == NULL)) {
    /* LCOV_EXCL_START */
    /* If these functions die within a worker process, they won't call
     * spawn_exit, but that's ok, since the parent will run out of memory soon
     * anyway. */
    raw_assert_unreached_msg("Out of memory on malloc(). Dying.");
    /* LCOV_EXCL_STOP */
  }
  return result;
}

/** Allocate a chunk of <b>size</b> bytes of memory, fill the memory with
 * zero bytes, and return a pointer to the result.  Log and terminate
 * the process on error.  (Same as calloc(size,1), but never returns NULL.)
 */
void *
tor_malloc_zero_(size_t size)
{
  /* You may ask yourself, "wouldn't it be smart to use calloc instead of
   * malloc+memset?  Perhaps libc's calloc knows some nifty optimization trick
   * we don't!"  Indeed it does, but its optimizations are only a big win when
   * we're allocating something very big (it knows if it just got the memory
   * from the OS in a pre-zeroed state).  We don't want to use tor_malloc_zero
   * for big stuff, so we don't bother with calloc. */
  void *result = tor_malloc_(size);
  memset(result, 0, size);
  return result;
}

/* The square root of SIZE_MAX + 1.  If a is less than this, and b is less
 * than this, then a*b is less than SIZE_MAX.  (For example, if size_t is
 * 32 bits, then SIZE_MAX is 0xffffffff and this value is 0x10000.  If a and
 * b are less than this, then their product is at most (65535*65535) ==
 * 0xfffe0001. */
#define SQRT_SIZE_MAX_P1 (((size_t)1) << (sizeof(size_t)*4))

/** Return non-zero if and only if the product of the arguments is exact,
 * and cannot overflow. */
STATIC int
size_mul_check(const size_t x, const size_t y)
{
  /* This first check is equivalent to
     (x < SQRT_SIZE_MAX_P1 && y < SQRT_SIZE_MAX_P1)

     Rationale: if either one of x or y is >= SQRT_SIZE_MAX_P1, then it
     will have some bit set in its most significant half.
   */
  return ((x|y) < SQRT_SIZE_MAX_P1 ||
          y == 0 ||
          x <= SIZE_MAX / y);
}

/** Allocate a chunk of <b>nmemb</b>*<b>size</b> bytes of memory, fill
 * the memory with zero bytes, and return a pointer to the result.
 * Log and terminate the process on error.  (Same as
 * calloc(<b>nmemb</b>,<b>size</b>), but never returns NULL.)
 * The second argument (<b>size</b>) should preferably be non-zero
 * and a compile-time constant.
 */
void *
tor_calloc_(size_t nmemb, size_t size)
{
  raw_assert(size_mul_check(nmemb, size));
  return tor_malloc_zero_((nmemb * size));
}

/** Change the size of the memory block pointed to by <b>ptr</b> to <b>size</b>
 * bytes long; return the new memory block.  On error, log and
 * terminate. (Like realloc(ptr,size), but never returns NULL.)
 */
void *
tor_realloc_(void *ptr, size_t size)
{
  void *result;

  raw_assert(size < SIZE_T_CEILING);

#ifndef MALLOC_ZERO_WORKS
  /* Some libc mallocs don't work when size==0. Override them. */
  if (size==0) {
    size=1;
  }
#endif /* !defined(MALLOC_ZERO_WORKS) */

  result = raw_realloc(ptr, size);

  if (PREDICT_UNLIKELY(result == NULL)) {
    /* LCOV_EXCL_START */
    raw_assert_unreached_msg("Out of memory on realloc(). Dying.");
    /* LCOV_EXCL_STOP */
  }
  return result;
}

/**
 * Try to realloc <b>ptr</b> so that it takes up sz1 * sz2 bytes.  Check for
 * overflow. Unlike other allocation functions, return NULL on overflow.
 */
void *
tor_reallocarray_(void *ptr, size_t sz1, size_t sz2)
{
  /* XXXX we can make this return 0, but we would need to check all the
   * reallocarray users. */
  raw_assert(size_mul_check(sz1, sz2));

  return tor_realloc(ptr, (sz1 * sz2));
}

/** Return a newly allocated copy of the NUL-terminated string s. On
 * error, log and terminate.  (Like strdup(s), but never returns
 * NULL.)
 */
char *
tor_strdup_(const char *s)
{
  char *duplicate;
  raw_assert(s);

  duplicate = raw_strdup(s);

  if (PREDICT_UNLIKELY(duplicate == NULL)) {
    /* LCOV_EXCL_START */
    raw_assert_unreached_msg("Out of memory on strdup(). Dying.");
    /* LCOV_EXCL_STOP */
  }
  return duplicate;
}

/** Allocate and return a new string containing the first <b>n</b>
 * characters of <b>s</b>.  If <b>s</b> is longer than <b>n</b>
 * characters, only the first <b>n</b> are copied.  The result is
 * always NUL-terminated.  (Like strndup(s,n), but never returns
 * NULL.)
 */
char *
tor_strndup_(const char *s, size_t n)
{
  char *duplicate;
  raw_assert(s);
  raw_assert(n < SIZE_T_CEILING);
  duplicate = tor_malloc_((n+1));
  /* Performance note: Ordinarily we prefer strlcpy to strncpy.  But
   * this function gets called a whole lot, and platform strncpy is
   * much faster than strlcpy when strlen(s) is much longer than n.
   */
  strncpy(duplicate, s, n);
  duplicate[n]='\0';
  return duplicate;
}

/** Allocate a chunk of <b>len</b> bytes, with the same contents as the
 * <b>len</b> bytes starting at <b>mem</b>. */
void *
tor_memdup_(const void *mem, size_t len)
{
  char *duplicate;
  raw_assert(len < SIZE_T_CEILING);
  raw_assert(mem);
  duplicate = tor_malloc_(len);
  memcpy(duplicate, mem, len);
  return duplicate;
}

/** As tor_memdup(), but add an extra 0 byte at the end of the resulting
 * memory. */
void *
tor_memdup_nulterm_(const void *mem, size_t len)
{
  char *duplicate;
  raw_assert(len < SIZE_T_CEILING+1);
  raw_assert(mem);
  duplicate = tor_malloc_(len+1);
  memcpy(duplicate, mem, len);
  duplicate[len] = '\0';
  return duplicate;
}

/** Helper for places that need to take a function pointer to the right
 * spelling of "free()". */
void
tor_free_(void *mem)
{
  tor_free(mem);
}
