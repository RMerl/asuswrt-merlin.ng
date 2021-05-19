/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file smartlist_split.c
 * \brief Split a string into a smartlist_t of substrings.
 **/

#include "lib/smartlist_core/smartlist_core.h"
#include "lib/smartlist_core/smartlist_split.h"

#include "lib/err/torerr.h"
#include "lib/string/util_string.h"
#include "lib/string/compat_ctype.h"
#include "lib/malloc/malloc.h"

#include <string.h>

/**
 * Split a string <b>str</b> along all occurrences of <b>sep</b>,
 * appending the (newly allocated) split strings, in order, to
 * <b>sl</b>.  Return the number of strings added to <b>sl</b>.
 *
 * If <b>flags</b>&amp;SPLIT_SKIP_SPACE is true, remove initial and
 * trailing space from each entry.
 * If <b>flags</b>&amp;SPLIT_IGNORE_BLANK is true, remove any entries
 * of length 0.
 * If <b>flags</b>&amp;SPLIT_STRIP_SPACE is true, strip spaces from each
 * split string.
 *
 * If <b>max</b>\>0, divide the string into no more than <b>max</b> pieces. If
 * <b>sep</b> is NULL, split on any sequence of horizontal space.
 */
int
smartlist_split_string(smartlist_t *sl, const char *str, const char *sep,
                       int flags, int max)
{
  const char *cp, *end, *next;
  int n = 0;

  raw_assert(sl);
  raw_assert(str);

  cp = str;
  while (1) {
    if (flags&SPLIT_SKIP_SPACE) {
      while (TOR_ISSPACE(*cp)) ++cp;
    }

    if (max>0 && n == max-1) {
      end = strchr(cp,'\0');
    } else if (sep) {
      end = strstr(cp,sep);
      if (!end)
        end = strchr(cp,'\0');
    } else {
      for (end = cp; *end && *end != '\t' && *end != ' '; ++end)
        ;
    }

    raw_assert(end);

    if (!*end) {
      next = NULL;
    } else if (sep) {
      next = end+strlen(sep);
    } else {
      next = end+1;
      while (*next == '\t' || *next == ' ')
        ++next;
    }

    if (flags&SPLIT_SKIP_SPACE) {
      while (end > cp && TOR_ISSPACE(*(end-1)))
        --end;
    }
    if (end != cp || !(flags&SPLIT_IGNORE_BLANK)) {
      char *string = tor_strndup(cp, end-cp);
      if (flags&SPLIT_STRIP_SPACE)
        tor_strstrip(string, " ");
      smartlist_add(sl, string);
      ++n;
    }
    if (!next)
      break;
    cp = next;
  }

  return n;
}
