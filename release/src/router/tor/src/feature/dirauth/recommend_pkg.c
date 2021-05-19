/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file recommend_pkg.c
 * \brief Code related to the recommended-packages subsystem.
 *
 * Currently unused.
 **/

#include "core/or/or.h"
#include "feature/dirauth/recommend_pkg.h"

/** Return true iff <b>line</b> is a valid RecommendedPackages line.
 */
/*
  The grammar is:

    "package" SP PACKAGENAME SP VERSION SP URL SP DIGESTS NL

      PACKAGENAME = NONSPACE
      VERSION = NONSPACE
      URL = NONSPACE
      DIGESTS = DIGEST | DIGESTS SP DIGEST
      DIGEST = DIGESTTYPE "=" DIGESTVAL

      NONSPACE = one or more non-space printing characters

      DIGESTVAL = DIGESTTYPE = one or more non-=, non-" " characters.

      SP = " "
      NL = a newline

 */
int
validate_recommended_package_line(const char *line)
{
  const char *cp = line;

#define WORD()                                  \
  do {                                          \
    if (*cp == ' ')                             \
      return 0;                                 \
    cp = strchr(cp, ' ');                       \
    if (!cp)                                    \
      return 0;                                 \
  } while (0)

  WORD(); /* skip packagename */
  ++cp;
  WORD(); /* skip version */
  ++cp;
  WORD(); /* Skip URL */
  ++cp;

  /* Skip digesttype=digestval + */
  int n_entries = 0;
  while (1) {
    const char *start_of_word = cp;
    const char *end_of_word = strchr(cp, ' ');
    if (! end_of_word)
      end_of_word = cp + strlen(cp);

    if (start_of_word == end_of_word)
      return 0;

    const char *eq = memchr(start_of_word, '=', end_of_word - start_of_word);

    if (!eq)
      return 0;
    if (eq == start_of_word)
      return 0;
    if (eq == end_of_word - 1)
      return 0;
    if (memchr(eq+1, '=', end_of_word - (eq+1)))
      return 0;

    ++n_entries;
    if (0 == *end_of_word)
      break;

    cp = end_of_word + 1;
  }

  /* If we reach this point, we have at least 1 entry. */
  tor_assert(n_entries > 0);
  return 1;
}
