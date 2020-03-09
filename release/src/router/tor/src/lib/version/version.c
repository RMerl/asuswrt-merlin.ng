/* Copyright 2001-2004 Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include "lib/version/torversion.h"
#include "lib/version/git_revision.h"

#include <stdio.h>
#include <string.h>

/** A shorter version of this Tor process's version, for export in our router
 *  descriptor.  (Does not include the git version, if any.) */
static const char the_short_tor_version[] =
  VERSION
#ifdef TOR_BUILD_TAG
  " ("TOR_BUILD_TAG")"
#endif
  "";

#define MAX_VERSION_LEN 128

/** The version of this Tor process, possibly including git version */
static char the_tor_version[MAX_VERSION_LEN] = "";

/** Return the current Tor version. */
const char *
get_version(void)
{
  if (the_tor_version[0] == 0) {
    if (strlen(tor_git_revision)) {
      snprintf(the_tor_version, sizeof(the_tor_version),
               "%s (git-%s)", the_short_tor_version, tor_git_revision);
    } else {
      snprintf(the_tor_version, sizeof(the_tor_version),
               "%s", the_short_tor_version);
    }
    the_tor_version[sizeof(the_tor_version)-1] = 0;
  }

  return the_tor_version;
}

/** Return the current Tor version, without any git tag. */
const char *
get_short_version(void)
{
  return the_short_tor_version;
}
