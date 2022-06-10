/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file compat_string.c
 * \brief Useful string-processing functions that some platforms don't
 * provide.
 **/

#include "lib/string/compat_string.h"
#include "lib/err/torerr.h"

/* Inline the strl functions if the platform doesn't have them. */
#ifndef HAVE_STRLCPY
#include "ext/strlcpy.c"
#endif
#ifndef HAVE_STRLCAT
#include "ext/strlcat.c"
#endif

#include <stdlib.h>
#include <string.h>

/** Helper for tor_strtok_r_impl: Advances cp past all characters in
 * <b>sep</b>, and returns its new value. */
static char *
strtok_helper(char *cp, const char *sep)
{
  if (sep[1]) {
    while (*cp && strchr(sep, *cp))
      ++cp;
  } else {
    while (*cp && *cp == *sep)
      ++cp;
  }
  return cp;
}

/** Implementation of strtok_r for platforms whose coders haven't figured out
 * how to write one.  Hey, retrograde libc developers!  You can use this code
 * here for free! */
char *
tor_strtok_r_impl(char *str, const char *sep, char **lasts)
{
  char *cp, *start;
  raw_assert(*sep);
  if (str) {
    str = strtok_helper(str, sep);
    if (!*str)
      return NULL;
    start = cp = *lasts = str;
  } else if (!*lasts || !**lasts) {
    return NULL;
  } else {
    start = cp = *lasts;
  }

  if (sep[1]) {
    while (*cp && !strchr(sep, *cp))
      ++cp;
  } else {
    cp = strchr(cp, *sep);
  }

  if (!cp || !*cp) {
    *lasts = NULL;
  } else {
    *cp++ = '\0';
    *lasts = strtok_helper(cp, sep);
  }
  return start;
}
