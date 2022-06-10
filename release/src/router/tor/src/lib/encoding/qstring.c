/* Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file qstring.c
 * \brief Implement QuotedString parsing.
 *
 * Note that this is only used for controller authentication; do not
 * create new users for this.  Instead, prefer the cstring.c functions.
 **/

#include "orconfig.h"
#include "lib/encoding/qstring.h"
#include "lib/malloc/malloc.h"
#include "lib/log/util_bug.h"

/** If the first <b>in_len_max</b> characters in <b>start</b> contain a
 * QuotedString, return the length of that
 * string (as encoded, including quotes).  Otherwise return -1. */
static inline int
get_qstring_length(const char *start, size_t in_len_max,
                          int *chars_out)
{
  const char *cp, *end;
  int chars = 0;

  if (*start != '\"')
    return -1;

  cp = start+1;
  end = start+in_len_max;

  /* Calculate length. */
  while (1) {
    if (cp >= end) {
      return -1; /* Too long. */
    } else if (*cp == '\\') {
      if (++cp == end)
        return -1; /* Can't escape EOS. */
      ++cp;
      ++chars;
    } else if (*cp == '\"') {
      break;
    } else {
      ++cp;
      ++chars;
    }
  }
  if (chars_out)
    *chars_out = chars;
  return (int)(cp - start+1);
}

/** Given a pointer to a string starting at <b>start</b> containing
 * <b>in_len_max</b> characters, decode a string beginning with one double
 * quote, containing any number of non-quote characters or characters escaped
 * with a backslash, and ending with a final double quote.  Place the resulting
 * string (unquoted, unescaped) into a newly allocated string in *<b>out</b>;
 * store its length in <b>out_len</b>.  On success, return a pointer to the
 * character immediately following the escaped string.  On failure, return
 * NULL. */
const char *
decode_qstring(const char *start, size_t in_len_max,
               char **out, size_t *out_len)
{
  const char *cp, *end;
  char *outp;
  int len, n_chars = 0;

  len = get_qstring_length(start, in_len_max, &n_chars);
  if (len<0)
    return NULL;

  end = start+len-1; /* Index of last quote. */
  tor_assert(*end == '\"');
  outp = *out = tor_malloc(len+1);
  *out_len = n_chars;

  cp = start+1;
  while (cp < end) {
    if (*cp == '\\')
      ++cp;
    *outp++ = *cp++;
  }
  *outp = '\0';
  tor_assert((outp - *out) == (int)*out_len);

  return end+1;
}
